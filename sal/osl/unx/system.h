/*************************************************************************
 *
 *  $RCSfile: system.h,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: hjs $ $Date: 2003-08-18 15:18:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef __OSL_SYSTEM_H__
#define __OSL_SYSTEM_H__

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <signal.h>
#include <utime.h>

#include <pwd.h>

#include <netdb.h>

#include <sys/stat.h>
#include <sys/wait.h>

#include <sys/types.h>

/* Make sockets of type AF_UNIX use underlying FS rights */
#ifdef SOLARIS
#   define _XOPEN_SOURCE 500
#   include <sys/socket.h>
#   undef _XOPEN_SOURCE
#else
#   include <sys/socket.h>
#endif

#include <netinet/in.h>
#include <arpa/inet.h>

#define max(a, b)   ((a) < (b) ? (b) : (a))
#define min(a, b)   ((a) > (b) ? (b) : (a))
#ifndef abs
#define abs(x)      ((x) >= 0 ? (x) : -(x))
#endif

#ifdef SYSV
#   include <sys/utsname.h>
#endif

#ifdef LINUX
#define __USE_GNU
#if GLIBC >= 2
#   include <shadow.h>
#   if !defined(SPARC)
#       include <asm/sigcontext.h>
#   endif
#   include <pthread.h>
#   include <sys/file.h>
#   include <sys/ioctl.h>
#   include <sys/uio.h>
#   include <sys/un.h>
#   include <netinet/tcp.h>
#   include <dlfcn.h>
#   include <endian.h>
#   include <sys/time.h>
#   include <semaphore.h>
#   if __BYTE_ORDER == __LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#   elif __BYTE_ORDER == __BIG_ENDIAN
#       define _BIG_ENDIAN
#   elif __BYTE_ORDER == __PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#   define  PTR_SIZE_T(s)               ((size_t *)&(s))
#   define  IORESOURCE_TRANSFER_BSD
#   define  IOCHANNEL_TRANSFER_BSD_RENO
#   define  pthread_testcancel()
#   define  NO_PTHREAD_PRIORITY
#   define  CMD_ARG_PROC_STREAM
#   define  CMD_ARG_PROC_NAME           "/proc/%u/cmdline"
#   define  PTHREAD_SIGACTION           pthread_sigaction
#else
#   include <shadow.h>
#   include <asm/sigcontext.h>
#   include <pthread.h>
#   include <sys/file.h>
#   include <sys/ioctl.h>
#   include <linux/net.h>
#   include <sys/un.h>
#   include <netinet/tcp.h>
#   include <linux/elfcore.h>
#   include <dlfcn.h>
#   include <endian.h>
#   if __BYTE_ORDER == __LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#   elif __BYTE_ORDER == __BIG_ENDIAN
#       define _BIG_ENDIAN
#   elif __BYTE_ORDER == __PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#   define  IORESOURCE_TRANSFER_BSD
#   define  IOCHANNEL_TRANSFER_BSD_RENO
#   define  pthread_testcancel()
#   define  NO_PTHREAD_RTL
#   define  NO_PTHREAD_PRIORITY
#   define  CMD_ARG_PROC_STREAM
#   define  CMD_ARG_PROC_NAME           "/proc/%u/cmdline"
#   define  PTHREAD_SIGACTION           pthread_sigaction
#endif
#endif

#ifdef NETBSD
#   define  ETIME ETIMEDOUT
#   define _POSIX_THREAD_SYSCALL_SOFT 1
#   include <pthread.h>
#   include <netdb.h>
#   include <sys/sem.h>
#   include <sys/exec.h>
#   include <sys/filio.h>
#   include <sys/ioctl.h>
#   include <sys/time.h>
#   include <sys/un.h>
#   include <netinet/tcp.h>
#   include <dlfcn.h>
#   include <machine/endian.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#       define _LITTLE_ENDIAN_OO
#   elif BYTE_ORDER == BIG_ENDIAN
#       define _BIG_ENDIAN_OO
#   elif BYTE_ORDER == PDP_ENDIAN
#       define _PDP_ENDIAN_OO
#   endif
#   define  PTR_SIZE_T(s)               ((size_t *)&(s))
#   define  IORESOURCE_TRANSFER_BSD
#   define  IOCHANNEL_TRANSFER_BSD_RENO
#   define  pthread_testcancel()
#   define  NO_PTHREAD_PRIORITY
#     define  NO_PTHREAD_SEMAPHORES
#   define  NO_PTHREAD_RTL
/* __progname isn't sufficient here. We need the full path as well
 * for e.g. setup and __progname only points to the binary name.
 */
extern struct ps_strings *__ps_strings;
#   define  CMD_ARG_PRG         *(__ps_strings->ps_argvstr)
#   define  CMD_ARG_PRG_IS_DEFINED
#   define  CMD_ARG_ENV         environ
#   define  PTHREAD_SIGACTION           pthread_sigaction
#endif

#ifdef FREEBSD
#   define  ETIME ETIMEDOUT
#   include <pthread.h>
#   include <sys/sem.h>
#   include <semaphore.h>
#   include <dlfcn.h>
#   include <sys/filio.h>
#   include <sys/ioctl.h>
#   include <sys/param.h>
#   include <sys/time.h>
#   include <sys/uio.h>
#   include <sys/exec.h>
#   include <vm/vm.h>
#   include <vm/vm_param.h>
#   include <vm/pmap.h>
#   include <vm/swap_pager.h>
#   include <sys/un.h>
#   include <netinet/tcp.h>
#   define  IORESOURCE_TRANSFER_BSD
#   include <machine/endian.h>
#if __FreeBSD_version < 500000
#   if BYTE_ORDER == LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#   elif BYTE_ORDER == BIG_ENDIAN
#       define _BIG_ENDIAN
#   elif BYTE_ORDER == PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#endif
#   define  NO_PTHREAD_RTL
/* __progname isn't sufficient here. We need the full path as well
 * for e.g. setup and __progname only points to the binary name.
 */
#   define  CMD_ARG_PRG_IS_DEFINED
#   define  CMD_ARG_PRG         *((struct ps_strings *)PS_STRINGS)->ps_argvstr
#   define  CMD_ARG_ENV         environ
#endif

#ifdef SCO
#   define AF_IPX -1
#   include <strings.h>
#   include <pthread.h>
#   include <shadow.h>
#   include <netdb.h>
#   include <sys/un.h>
#   include <sys/netinet/tcp.h>
#   include <sys/types.h>
#   include <sys/byteorder.h>
#   include <dlfcn.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#   elif BYTE_ORDER == BIG_ENDIAN
#       define _BIG_ENDIAN
#   elif BYTE_ORDER == PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#   define  sched_yield()               pthread_yield()
#   define  pthread_testcancel()
#   define  NO_PTHREAD_RTL
#   define  NO_PTHREAD_PRIORITY
extern int pthread_cancel(pthread_t);
extern unsigned int nanosleep(unsigned int);
#   define  SLEEP_TIMESPEC(timespec)    (timespec .tv_sec > 0) ? sleep(timespec .tv_sec), nanosleep(timespec .tv_nsec) : nanosleep(timespec .tv_nsec)
#   define  PATH_MAX                    _POSIX_PATH_MAX
#   define  S_ISSOCK                    S_ISFIFO
#   define  PTHREAD_SIGACTION           pthread_sigaction
#   define  CMD_ARG_ENV                 _environ
#   define  STAT_PARENT                 stat
#endif

#ifdef AIX
#   define AF_IPX -1
#   include <strings.h>
#   include <pthread.h>
#   include <sys/time.h>
#   include <sys/un.h>
#   include <netinet/tcp.h>
#   include <sys/machine.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#   elif BYTE_ORDER == BIG_ENDIAN
#       define _BIG_ENDIAN
#   elif BYTE_ORDER == PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#   define  sched_yield()               pthread_yield()
#   define  SLEEP_TIMESPEC(timespec)    nsleep(&timespec, 0)
#   define  LIBPATH "LIBPATH"
#   define  PTR_SIZE_T(s)               ((size_t *)&(s))
#   define  NO_PTHREAD_SEMAPHORES
#   define  NO_DL_FUNCTIONS
#   define  CMD_ARG_PS                  "ps -p %u -o args=\"\""
#endif

#ifdef HPUX
#   define  AF_IPX -1
#   undef   howmany
#   undef   MAXINT
#   include <pthread.h>
#   include <sys/un.h>
#   include <sys/sched.h>
#   include <sys/xti.h>
#   include <sys/pstat.h>
#   include <shadow.h>
#   include <crypt.h>
#   include <machine/param.h>
#   define  LIBPATH "SHLIB_PATH"
#   define  PTR_SIZE_T(s)               ((int *)&(s))
#   define  PTR_FD_SET(s)               ((int *)&(s))
#   define  PTHREAD_VALUE(t)            ((t).field2)
#   define  PTHREAD_NONE_INIT           { 0, -1 }
#   define  PTHREAD_ATTR_DEFAULT        pthread_attr_default
#   define  PTHREAD_MUTEXATTR_DEFAULT   pthread_mutexattr_default
#   define  PTHREAD_CONDATTR_DEFAULT    pthread_condattr_default
#   define  pthread_detach(t)           pthread_detach(&(t))
#   define  NO_PTHREAD_PRIORITY
#   define  NO_PTHREAD_SEMAPHORES
#   define  NO_DL_FUNCTIONS
#   define  CMD_ARG_PRG                 $ARGV
#   define  CMD_ARG_ENV                 environ
#   undef   sigaction
#   define  PTHREAD_SIGACTION           cma_sigaction
#endif

#ifdef IRIX
#   define  AF_IPX -1
#   include <pthread.h>
#   include <semaphore.h>
#   include <sched.h>
#   include <sys/socket.h>
#   include <sys/un.h>
#   include <sys/stropts.h>
#   include <netinet/tcp.h>
#   include <procfs/procfs.h>
#   include <sys/endian.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#   undef _BIG_ENDIAN
#   undef _PDP_ENDIAN
#   elif BYTE_ORDER == BIG_ENDIAN
#   undef _LITTLE_ENDIAN
#   undef _PDP_ENDIAN
#   elif BYTE_ORDER == PDP_ENDIAN
#   undef _LITTLE_ENDIAN
#   undef _BIG_ENDIAN
#   endif
#   define  SA_FAMILY_DECL \
        union { struct { short sa_family2; } sa_generic; } sa_union
#   define  CMD_ARG_ENV _environ
#   define  PTR_SIZE_T(s)               ((int *)&(s))
#   define  NO_PTHREAD_PRIORITY
#   include <dlfcn.h>
#   define  IOCHANNEL_TRANSFER_BSD
extern char *strdup(const char *);
#endif

#ifdef SOLARIS
#   include <shadow.h>
#   include <sys/procfs.h>
#   include <sys/un.h>
#   include <stropts.h>
#   include <pthread.h>
#   include <semaphore.h>
#   include <netinet/tcp.h>
#   include <sys/filio.h>
#   include <dlfcn.h>
#   include <sys/isa_defs.h>
#   define  IORESOURCE_TRANSFER_SYSV
#   define  IOCHANNEL_TRANSFER_BSD
#   define  LIBPATH "LD_LIBRARY_PATH"
#   define  PTR_SIZE_T(s)               ((int *)&(s))
#   define  CMD_ARG_PROC_IOCTL          PIOCPSINFO
#   define  CMD_ARG_PROC_NAME           "/proc/%u"
#   define  CMD_ARG_PROC_TYPE           prpsinfo_t
#   define  CMD_ARG_PROC_ARGC(t)        t.pr_argc
#   define  CMD_ARG_PROC_ARGV(t)        t.pr_argv
#endif

#ifdef MACOSX
#   define  ETIME ETIMEDOUT
#   include <pthread.h>
#   include <sys/file.h>
#   include <sys/ioctl.h>
#   include <sys/uio.h>
#   include <sys/un.h>
#   include <netinet/tcp.h>
#   include <machine/endian.h>
#   include <sys/time.h>
#   include <sys/semaphore.h>
/* fixme are premac and postmac still needed here? */
#   include <premac.h>
#   include <mach-o/dyld.h>
#   include <postmac.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#   elif BYTE_ORDER == BIG_ENDIAN
#       define _BIG_ENDIAN
#   elif BYTE_ORDER == PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#   define  IOCHANNEL_TRANSFER_BSD_RENO
#   define  NO_PTHREAD_RTL
/* for NSGetArgc/Argv/Environ */
#       include <crt_externs.h>
int  readdir_r( DIR *dirp, struct dirent *entry, struct dirent **result );
char *asctime_r( const struct tm *tm, char *buffer );
char *macxp_tempnam( const char *tmpdir, const char *prefix );
void macxp_getSystemVersion( unsigned int *isDarwin, unsigned int *majorVersion, unsigned int *minorVersion, unsigned int *minorMinorVersion );
#endif

#if !defined(_WIN32)  && !defined(_WIN16) && !defined(OS2)  && \
    !defined(LINUX)   && !defined(NETBSD) && !defined(FREEBSD) && !defined(SCO)  && \
    !defined(AIX)     && !defined(HPUX)   && \
    !defined(SOLARIS) && !defined(IRIX)   && !defined(MAC) && \
    !defined(MACOSX)
#   error "Target plattform not specified !"
#endif

#if defined(NETBSD)
#if defined _LITTLE_ENDIAN_OO
#   define _OSL_BIGENDIAN
#elif defined _BIG_ENDIAN_OO
#   define _OSL_LITENDIAN
#else
#   error undetermined endianess
#endif
#else
#if defined _LITTLE_ENDIAN
#   define _OSL_BIGENDIAN
#elif defined _BIG_ENDIAN
#   define _OSL_LITENDIAN
#else
#   error undetermined endianess
#endif
#endif

#ifndef CMD_ARG_MAX
#   define CMD_ARG_MAX                  4096
#endif

#ifndef ENV_VAR_MAX
#   define ENV_VAR_MAX                  4096
#endif

#ifndef PTR_SIZE_T
#   define PTR_SIZE_T(s)                (&(s))
#endif

#ifndef PTR_FD_SET
#   define PTR_FD_SET(s)                (&(s))
#endif

#ifndef NORMALIZE_TIMESPEC
#   define NORMALIZE_TIMESPEC(timespec) \
          timespec . tv_sec  += timespec . tv_nsec / 1000000000; \
          timespec . tv_nsec %= 1000000000;
#endif

#ifndef SET_TIMESPEC
#   define SET_TIMESPEC(timespec, sec, nsec) \
          timespec . tv_sec  = (sec);  \
          timespec . tv_nsec = (nsec); \
        NORMALIZE_TIMESPEC(timespec);
#endif

#ifndef SLEEP_TIMESPEC
#   define SLEEP_TIMESPEC(timespec) nanosleep(&timespec, 0)
#endif

#ifndef INIT_GROUPS
#   define  INIT_GROUPS(name, gid)  ((setgid((gid)) == 0) && (initgroups((name), (gid)) == 0))
#endif

#ifndef PTHREAD_VALUE
#   define PTHREAD_VALUE(t)             (t)
#endif
#ifndef PTHREAD_NONE
extern pthread_t _pthread_none_;
#   define PTHREAD_NONE                 _pthread_none_
#   ifndef PTHREAD_NONE_INIT
#       define PTHREAD_NONE_INIT        ((pthread_t)-1)
#   endif
#endif

#ifndef PTHREAD_ATTR_DEFAULT
#   define PTHREAD_ATTR_DEFAULT         NULL
#endif
#ifndef PTHREAD_MUTEXATTR_DEFAULT
#   define PTHREAD_MUTEXATTR_DEFAULT    NULL
#endif
#ifndef PTHREAD_CONDATTR_DEFAULT
#   define PTHREAD_CONDATTR_DEFAULT     NULL
#endif

#ifndef PTHREAD_SIGACTION
#   define PTHREAD_SIGACTION sigaction
#endif

#ifndef STAT_PARENT
#   define STAT_PARENT                  lstat
#endif

/* socket options which might not be defined on all unx flavors */
#ifndef SO_ACCEPTCONN
#   define SO_ACCEPTCONN    0
#endif
#ifndef SO_SNDLOWAT
#   define SO_SNDLOWAT      0
#endif
#ifndef SO_RCVLOWAT
#   define SO_RCVLOWAT      0
#endif
#ifndef SO_SNDTIMEO
#   define  SO_SNDTIMEO     0
#endif
#ifndef SO_RCVTIMEO
#   define SO_RCVTIMEO      0
#endif
#ifndef SO_USELOOPBACK
#   define SO_USELOOPBACK   0
#endif
#ifndef MSG_MAXIOVLEN
#   define MSG_MAXIOVLEN    0
#endif

/* BEGIN HACK */
/* dummy define and declarations for IPX should be replaced by */
/* original ipx headers when these are available for this platform */

#ifndef SA_FAMILY_DECL
#   define SA_FAMILY_DECL short sa_family
#endif

typedef struct sockaddr_ipx {
    SA_FAMILY_DECL;
    char  sa_netnum[4];
    char  sa_nodenum[6];
    unsigned short sa_socket;
} SOCKADDR_IPX;

#define NSPROTO_IPX      1000
#define NSPROTO_SPX      1256
#define NSPROTO_SPXII    1257

/* END HACK */

#ifdef NO_PTHREAD_SEMAPHORES

typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t  increased;
    int             value;
} sem_t;
extern int sem_init(sem_t* sem, int pshared, unsigned int value);
extern int sem_destroy(sem_t* sem);
extern int sem_wait(sem_t* sem);
extern int sem_trywait(sem_t* sem);
extern int sem_post(sem_t* sem);

#endif

#ifdef NO_PTHREAD_RTL
#if !defined FREEBSD || (__FreeBSD_version < 500112)
struct passwd *getpwent_r(struct passwd *pwd, char *buffer,  int buflen);
extern struct spwd *getspnam_r(const char *name, struct spwd *result,
                               char *buffer, int buflen);

struct tm *localtime_r(const time_t *timep, struct tm *buffer);
struct tm *gmtime_r(const time_t *timep, struct tm *buffer);
#endif /* !defined FREEBSD || (__FreeBSD_version < 500112) */
struct hostent *gethostbyname_r(const char *name, struct hostent *result,
                                char *buffer, int buflen, int *h_errnop);
#endif

#endif /* __OSL_SYSTEM_H__ */

