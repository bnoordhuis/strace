#include "defs.h"
#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif
#include "xlat/fsmagic.h"

static const char *
sprintfstype(const unsigned int magic)
{
	static char buf[32];
	const char *s;

	s = xlat_search(fsmagic, ARRAY_SIZE(fsmagic), magic);
	if (s) {
		sprintf(buf, "\"%s\"", s);
		return buf;
	}
	sprintf(buf, "%#x", magic);
	return buf;
}

static void
printstatfs(struct tcb *tcp, const long addr)
{
	struct statfs statbuf;

	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}
	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}
#ifdef ALPHA
	tprintf("{f_type=%s, f_fbsize=%u, f_blocks=%u, f_bfree=%u, ",
		sprintfstype(statbuf.f_type),
		statbuf.f_bsize, statbuf.f_blocks, statbuf.f_bfree);
	tprintf("f_bavail=%u, f_files=%u, f_ffree=%u, f_fsid={%d, %d}, f_namelen=%u",
		statbuf.f_bavail, statbuf.f_files, statbuf.f_ffree,
		statbuf.f_fsid.__val[0], statbuf.f_fsid.__val[1],
		statbuf.f_namelen);
#else /* !ALPHA */
	tprintf("{f_type=%s, f_bsize=%lu, f_blocks=%lu, f_bfree=%lu, ",
		sprintfstype(statbuf.f_type),
		(unsigned long)statbuf.f_bsize,
		(unsigned long)statbuf.f_blocks,
		(unsigned long)statbuf.f_bfree);
	tprintf("f_bavail=%lu, f_files=%lu, f_ffree=%lu, f_fsid={%d, %d}",
		(unsigned long)statbuf.f_bavail,
		(unsigned long)statbuf.f_files,
		(unsigned long)statbuf.f_ffree,
		statbuf.f_fsid.__val[0], statbuf.f_fsid.__val[1]);
	tprintf(", f_namelen=%lu", (unsigned long)statbuf.f_namelen);
#endif /* !ALPHA */
#ifdef _STATFS_F_FRSIZE
	tprintf(", f_frsize=%lu", (unsigned long)statbuf.f_frsize);
#endif
	tprints("}");
}

int
sys_statfs(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printstatfs(tcp, tcp->u_arg[1]);
	}
	return 0;
}

int
sys_fstatfs(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printstatfs(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#ifdef HAVE_STRUCT_STATFS64
static void
printstatfs64(struct tcb *tcp, long addr)
{
	struct statfs64 statbuf;

	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}
	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}
	tprintf("{f_type=%s, f_bsize=%llu, f_blocks=%llu, f_bfree=%llu, ",
		sprintfstype(statbuf.f_type),
		(unsigned long long)statbuf.f_bsize,
		(unsigned long long)statbuf.f_blocks,
		(unsigned long long)statbuf.f_bfree);
	tprintf("f_bavail=%llu, f_files=%llu, f_ffree=%llu, f_fsid={%d, %d}",
		(unsigned long long)statbuf.f_bavail,
		(unsigned long long)statbuf.f_files,
		(unsigned long long)statbuf.f_ffree,
		statbuf.f_fsid.__val[0], statbuf.f_fsid.__val[1]);
	tprintf(", f_namelen=%lu", (unsigned long)statbuf.f_namelen);
#ifdef _STATFS_F_FRSIZE
	tprintf(", f_frsize=%llu", (unsigned long long)statbuf.f_frsize);
#endif
#ifdef _STATFS_F_FLAGS
	tprintf(", f_flags=%llu", (unsigned long long)statbuf.f_flags);
#endif
	tprints("}");
}

struct compat_statfs64 {
	uint32_t f_type;
	uint32_t f_bsize;
	uint64_t f_blocks;
	uint64_t f_bfree;
	uint64_t f_bavail;
	uint64_t f_files;
	uint64_t f_ffree;
	fsid_t f_fsid;
	uint32_t f_namelen;
	uint32_t f_frsize;
	uint32_t f_flags;
	uint32_t f_spare[4];
}
#if defined(X86_64) || defined(IA64)
  __attribute__ ((packed, aligned(4)))
#endif
;

static void
printcompat_statfs64(struct tcb *tcp, const long addr)
{
	struct compat_statfs64 statbuf;

	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}
	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}
	tprintf("{f_type=%s, f_bsize=%lu, f_blocks=%llu, f_bfree=%llu, ",
		sprintfstype(statbuf.f_type),
		(unsigned long)statbuf.f_bsize,
		(unsigned long long)statbuf.f_blocks,
		(unsigned long long)statbuf.f_bfree);
	tprintf("f_bavail=%llu, f_files=%llu, f_ffree=%llu, f_fsid={%d, %d}",
		(unsigned long long)statbuf.f_bavail,
		(unsigned long long)statbuf.f_files,
		(unsigned long long)statbuf.f_ffree,
		statbuf.f_fsid.__val[0], statbuf.f_fsid.__val[1]);
	tprintf(", f_namelen=%lu", (unsigned long)statbuf.f_namelen);
	tprintf(", f_frsize=%lu", (unsigned long)statbuf.f_frsize);
	tprintf(", f_flags=%lu}", (unsigned long)statbuf.f_frsize);
}

int
sys_statfs64(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %lu, ", tcp->u_arg[1]);
	} else {
		if (tcp->u_arg[1] == sizeof(struct statfs64))
			printstatfs64(tcp, tcp->u_arg[2]);
		else if (tcp->u_arg[1] == sizeof(struct compat_statfs64))
			printcompat_statfs64(tcp, tcp->u_arg[2]);
		else
			tprints("{???}");
	}
	return 0;
}

int
sys_fstatfs64(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprintf(", %lu, ", tcp->u_arg[1]);
	} else {
		if (tcp->u_arg[1] == sizeof(struct statfs64))
			printstatfs64(tcp, tcp->u_arg[2]);
		else if (tcp->u_arg[1] == sizeof(struct compat_statfs64))
			printcompat_statfs64(tcp, tcp->u_arg[2]);
		else
			tprints("{???}");
	}
	return 0;
}
#endif /* HAVE_STRUCT_STATFS64 */

#ifdef ALPHA
int
osf_statfs(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printstatfs(tcp, tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

int
osf_fstatfs(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%lu, ", tcp->u_arg[0]);
	} else {
		printstatfs(tcp, tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}
#endif /* ALPHA */
