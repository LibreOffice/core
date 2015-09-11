/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SAL_OSL_UNX_SYSTEM_HXX
#define INCLUDED_SAL_OSL_UNX_SYSTEM_HXX

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
#if defined(SOLARIS) && !defined(_XOPEN_SOURCE)
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
#   define  IORESOURCE_TRANSFER_BSD
#   define  IOCHANNEL_TRANSFER_BSD_RENO
#   define  pthread_testcancel()
#   define  NO_PTHREAD_PRIORITY
#   define  PTHREAD_SIGACTION           pthread_sigaction

#   ifndef ETIME
#       define ETIME ETIMEDOUT
#   endif

#endif

#ifdef ANDROID
#   include <pthread.h>
#   include <sys/file.h>
#   include <sys/ioctl.h>
#   include <sys/uio.h>
#   include <sys/un.h>
#   include <netinet/tcp.h>
#   include <dlfcn.h>
#   include <endian.h>
#   include <sys/time.h>
#   define  IORESOURCE_TRANSFER_BSD
#   define  IOCHANNEL_TRANSFER_BSD_RENO
#   define  pthread_testcancel()
#   define  NO_PTHREAD_PRIORITY
#endif

#ifdef NETBSD
#   define  NO_PTHREAD_RTL
#endif

#ifdef FREEBSD
#   define  ETIME ETIMEDOUT
#   include <pthread.h>
#   include <sys/sem.h>
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
#   define  NO_PTHREAD_RTL
#endif

#ifdef OPENBSD
#   define  ETIME ETIMEDOUT
#   include <pthread.h>
#   include <sys/sem.h>
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

#if defined(DRAGONFLY) || defined(NETBSD)
#   define  ETIME ETIMEDOUT
#   include <pthread.h>
#   include <sys/sem.h>
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
#   define  IOCHANNEL_TRANSFER_BSD_RENO
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
#   define  SLEEP_TIMESPEC(timespec)    nsleep(&timespec, 0)
#   define  LIBPATH "LIBPATH"
#endif

#ifdef SOLARIS
#   include <shadow.h>
#   include <sys/un.h>
#   include <stropts.h>
#   include <pthread.h>
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
#   include <mach-o/dyld.h>
#   define  IOCHANNEL_TRANSFER_BSD_RENO
#   define  NO_PTHREAD_RTL
/* for NSGetArgc/Argv/Environ */
#       include <crt_externs.h>
int macxp_resolveAlias(char *path, int buflen);
#endif

#ifdef IOS
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
#   define  IOCHANNEL_TRANSFER_BSD_RENO
#   define  NO_PTHREAD_RTL
#endif

#if !defined(_WIN32)  && \
    !defined(LINUX)   && !defined(NETBSD) && !defined(FREEBSD) && \
    !defined(AIX)     && \
    !defined(SOLARIS) && !defined(MACOSX) && \
    !defined(OPENBSD) && !defined(DRAGONFLY) && \
    !defined(IOS) && !defined(ANDROID)
#   error "Target platform not specified!"
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

#if defined MACOSX
#define PTHREAD_VALUE(t) reinterpret_cast<unsigned long>(t)
#else
#define PTHREAD_VALUE(t) (t)
#endif

#ifndef PTHREAD_NONE
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

#ifdef NO_PTHREAD_RTL
#if !defined FREEBSD
#if !defined NETBSD
struct passwd *getpwent_r(struct passwd *pwd, char *buffer,  int buflen);
#endif
extern struct spwd *getspnam_r(const char *name, struct spwd *result,
                               char *buffer, int buflen);

#if !defined MACOSX
struct tm *localtime_r(const time_t *timep, struct tm *buffer);
struct tm *gmtime_r(const time_t *timep, struct tm *buffer);
#endif
struct hostent *gethostbyname_r(const char *name, struct hostent *result,
                                char *buffer, size_t buflen, int *h_errnop);
#endif /* !defined(FREEBSD) */
#endif

#endif // INCLUDED_SAL_OSL_UNX_SYSTEM_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
