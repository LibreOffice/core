/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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

#ifdef SYSV
#   include <sys/utsname.h>
#endif

#ifdef LINUX
#   ifndef __USE_GNU
#   define __USE_GNU
#   endif

#if GLIBC >= 2
#   include <shadow.h>
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
#               ifndef _BIG_ENDIAN
#               define _BIG_ENDIAN
#               endif
#   elif __BYTE_ORDER == __PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#   define  IORESOURCE_TRANSFER_BSD
#   define  IOCHANNEL_TRANSFER_BSD_RENO
#   define  pthread_testcancel()
#   define  NO_PTHREAD_PRIORITY
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
#       ifndef _LITTLE_ENDIAN
#           define _LITTLE_ENDIAN
#       endif
#   elif __BYTE_ORDER == __BIG_ENDIAN
#       ifndef _BIG_ENDIAN
#           define _BIG_ENDIAN
#       endif
#   elif __BYTE_ORDER == __PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#   define  IORESOURCE_TRANSFER_BSD
#   define  IOCHANNEL_TRANSFER_BSD_RENO
#   define  pthread_testcancel()
#   define  NO_PTHREAD_RTL
#   define  NO_PTHREAD_PRIORITY
#   define  PTHREAD_SIGACTION           pthread_sigaction
#endif

#   ifndef ETIME
#       define ETIME ETIMEDOUT
#   endif

#endif

#ifdef NETBSD
#   include <sys/param.h>
#       ifndef ETIME
#     define  ETIME ETIMEDOUT
#       endif
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
#   define  IORESOURCE_TRANSFER_BSD
#   define  IOCHANNEL_TRANSFER_BSD_RENO
#   define  pthread_testcancel()
#   define  NO_PTHREAD_PRIORITY
#     define  NO_PTHREAD_SEMAPHORES
#   define  NO_PTHREAD_RTL
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
#endif

#ifdef OPENBSD
#   define  ETIME ETIMEDOUT
#   define _POSIX_THREAD_SYSCALL_SOFT 1
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
#       include <sys/un.h>
#   include <netinet/tcp.h>
#       define  IORESOURCE_TRANSFER_BSD
#   include <machine/endian.h>
#      define  PTR_SIZE_T(s)   ((size_t *)&(s))
#       define  IORESOURCE_TRANSFER_BSD
#       define  IOCHANNEL_TRANSFER_BSD_RENO
#       define  pthread_testcancel()
#       define  NO_PTHREAD_PRIORITY
#       define  NO_PTHREAD_RTL
#       define  PTHREAD_SIGACTION                       pthread_sigaction
#endif

#ifdef DRAGONFLY
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
#   include <sys/un.h>
#   include <netinet/tcp.h>
#   include <machine/endian.h>
#   define  IORESOURCE_TRANSFER_BSD
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
#   define  STAT_PARENT                 stat
#endif

#ifdef AIX
#   define AF_IPX -1
#   include <strings.h>
#   include <pthread.h>
#   include <dlfcn.h>
#   include <sys/time.h>
#   include <sys/un.h>
#   include <netinet/tcp.h>
#   include <sys/machine.h>
#   if BYTE_ORDER == LITTLE_ENDIAN
#       ifndef _LITTLE_ENDIAN
#           define _LITTLE_ENDIAN
#       endif
#   elif BYTE_ORDER == BIG_ENDIAN
#       ifndef _BIG_ENDIAN
#           define _BIG_ENDIAN
#       endif
#   elif BYTE_ORDER == PDP_ENDIAN
#       define _PDP_ENDIAN
#   endif
#   define  SLEEP_TIMESPEC(timespec)    nsleep(&timespec, 0)
#   define  LIBPATH "LIBPATH"
#   define  NO_PTHREAD_SEMAPHORES
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
#   undef   sigaction
#   define  PTHREAD_SIGACTION           cma_sigaction
#endif

#ifdef SOLARIS
#   include <shadow.h>
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
#endif

#ifdef MACOSX
#define __OPENTRANSPORTPROVIDERS__ // these are already defined
#define TimeValue CFTimeValue      // Do not conflict with TimeValue in sal/inc/osl/time.h
#include <Carbon/Carbon.h>
#undef TimeValue
#   ifndef ETIME
#       define  ETIME ETIMEDOUT
#   endif
#   include <dlfcn.h>
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
#       ifndef _LITTLE_ENDIAN
#       define _LITTLE_ENDIAN
#       endif
#   elif BYTE_ORDER == BIG_ENDIAN
#       ifndef _BIG_ENDIAN
#       define _BIG_ENDIAN
#       endif
#   elif BYTE_ORDER == PDP_ENDIAN
#       ifndef _PDP_ENDIAN
#       define _PDP_ENDIAN
#       endif
#   endif
#   define  IOCHANNEL_TRANSFER_BSD_RENO
#   define  NO_PTHREAD_RTL
/* for NSGetArgc/Argv/Environ */
#       include <crt_externs.h>
#ifdef __cplusplus
extern "C" {
#endif
int macxp_resolveAlias(char *path, int buflen);
#ifdef __cplusplus
}
#endif
#endif

#if !defined(_WIN32)  && !defined(_WIN16) && !defined(OS2)  && \
    !defined(LINUX)   && !defined(NETBSD) && !defined(FREEBSD) && !defined(SCO)  && \
    !defined(AIX)     && !defined(HPUX)   && \
    !defined(SOLARIS) && !defined(MACOSX) && \
    !defined(OPENBSD) && !defined(DRAGONFLY)
#   error "Target platform not specified!"
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
# if (__GNUC__ < 4) && !defined(MACOSX)
extern pthread_t _pthread_none_;
# endif
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
#if !defined NETBSD
struct passwd *getpwent_r(struct passwd *pwd, char *buffer,  int buflen);
#endif
extern struct spwd *getspnam_r(const char *name, struct spwd *result,
                               char *buffer, int buflen);

struct tm *localtime_r(const time_t *timep, struct tm *buffer);
struct tm *gmtime_r(const time_t *timep, struct tm *buffer);
#endif /* !defined FREEBSD || (__FreeBSD_version < 500112) */
#if !defined(FREEBSD) || (__FreeBSD_version < 601103)
struct hostent *gethostbyname_r(const char *name, struct hostent *result,
                                char *buffer, int buflen, int *h_errnop);
#endif /* !defined(FREEBSD) || (__FreeBSD_version < 601103) */
#endif

#endif /* __OSL_SYSTEM_H__ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
