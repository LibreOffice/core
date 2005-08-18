#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dlfcn.h>

#ifdef _cplusplus
extern "C" {
#endif

#ifdef SOLARIS

#include <sys/systeminfo.h>
#include <strings.h>

int   chown  (const char *path, uid_t owner, gid_t group) {return 0;}
int   lchown (const char *path, uid_t owner, gid_t group) {return 0;}
int   fchown (int fildes, uid_t owner, gid_t group)       {return 0;}

uid_t getuid  (void) {return 0;}
#ifdef __notdef__
uid_t geteuid (void) {return 0;}
gid_t getgid  (void) {return 0;}
gid_t getegid (void) {return 0;}
#endif

int   setuid  (uid_t p)  {return 0;}
int   setgid  (gid_t p)  {return 0;}

/* This is to fool cpio and pkgmk */
int fstat(int fildes, struct stat *buf)
{
    int ret = 0;
    static int (*p_fstat) (int fildes, struct stat *buf) = NULL;
    if (p_fstat == NULL)
        p_fstat = (int (*)(int fildes, struct stat *buf))
            dlsym (RTLD_NEXT, "fstat");
    ret = (*p_fstat)(fildes, buf);
    if (buf != NULL)
    {
        buf->st_uid = 0; /* root */
        buf->st_gid = 2; /* bin */
    }

    return ret;
}

/* This is to fool tar */
int fstatat64(int fildes, const char *path, struct stat64  *buf, int flag)
{
    int ret = 0;
    static int (*p_fstatat) (int fildes, const char *path, struct stat64 *buf, int flag) = NULL;
    if (p_fstatat == NULL)
        p_fstatat = (int (*)(int fildes, const char *path, struct stat64 *buf, int flag))
            dlsym (RTLD_NEXT, "fstatat64");
    ret = (*p_fstatat)(fildes, path, buf, flag);
    if (buf != NULL)
    {
        buf->st_uid = 0; /* root */
        buf->st_gid = 2; /* bin */
    }

    return ret;
}

#elif defined LINUX

uid_t getuid  (void) {return 0;}
uid_t geteuid (void) {return 0;}

/* This is to fool tar */
int __lxstat64(int n, const char *path, struct stat64 *buf)
{
    int ret = 0;
    static int (*p_lstat) (int n, const char *path, struct stat64 *buf) = NULL;
    if (p_lstat == NULL)
        p_lstat = (int (*)(int n, const char *path, struct stat64 *buf))
            dlsym (RTLD_NEXT, "__lxstat64");
    ret = (*p_lstat)(n, path, buf);
    if (buf != NULL)
    {
        buf->st_uid = 0; /* root */
        buf->st_gid = 0; /* root */
    }

    return ret;
}

#endif

#ifdef _cplusplus
}
#endif

