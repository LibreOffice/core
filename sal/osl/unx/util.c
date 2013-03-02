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


#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>

#ifdef SOLARIS
#include <sys/sockio.h>
#endif

#include "osl/util.h"
#include "osl/diagnose.h"


/*****************************************************************************/
/* Static Module Functions */
/*****************************************************************************/

static int   osl_getHWAddr(const char *ifname, char* hard_addr);
static int   osl_checkAddr(const char* addr);


/*****************************************************************************/
/* osl_getEthernetAddress */
/*****************************************************************************/

sal_Bool SAL_CALL osl_getEthernetAddress( sal_uInt8 * pAddr )
{
#ifdef SOLARIS
    /** algorithm doesn't work on solaris */
    return sal_False;
#else
    char buff[1024];
    char hard_addr[64];
    struct ifconf ifc;
    struct ifreq *ifr;
    int i;
    int so;

    if ( pAddr == 0 )
    {
        return sal_False;
    }


    /*
     * All we need is ... a network file descriptor.
     * Normally, this is a very socket.
     */

    so = socket(AF_INET, SOCK_DGRAM, 0);


    /*
     * The first thing we have to do, get the interface configuration.
     * It is a list of attached/configured interfaces
     */

    ifc.ifc_len = sizeof(buff);
    ifc.ifc_buf = buff;
    if ( ioctl(so, SIOCGIFCONF, &ifc) < 0 )
    {
        OSL_TRACE( "SIOCGIFCONF: %s", strerror(errno) );
        close(so);
        return sal_False;
    }

    close(so);

    /*
     *  For each of the interfaces in the interface list,
     *  try to get the hardware address
     */

    ifr = ifc.ifc_req;
    for ( i = ifc.ifc_len / sizeof(struct ifreq) ; --i >= 0 ; ifr++ )
    {
        int nRet=0;
        nRet = osl_getHWAddr(ifr->ifr_name,hard_addr);
        if ( nRet  > 0 )
        {
            memcpy( pAddr , hard_addr, 6 );
            return sal_True;
        }
    }

    return sal_False;
#endif
}


/*****************************************************************************/
/* osl_getHWAddr */
/*****************************************************************************/

static int osl_getHWAddr(const char *ifname, char* hard_addr)
{
    int ret=0;
    struct ifreq ifr;
    int so = socket(AF_INET, SOCK_DGRAM, 0);

    if (strlen(ifname) >= sizeof(ifr.ifr_name))
    {
        close(so);
        return 0;
    }

    strcpy(ifr.ifr_name, ifname);

    /*
     *   First, get the Interface-FLAGS
     */

    ret=ioctl(so, SIOCGIFFLAGS, &ifr) ;

    if ( ret < 0 )
    {
        OSL_TRACE( "SIOCGIFFLAGS: %s", strerror(errno) );
        close(so);
        return ret;
    }


    /*
     *  If it is the loopback device, do not consider it any further
     */

    if (ifr.ifr_flags & IFF_LOOPBACK)
    {
        OSL_TRACE( "SIOCGIFFLAGS : is LOOPBACK : %s", strerror(errno) );
        close(so);
        return 0;
    }


    /*
     *  And now, the real thing: the get address
     */

#if defined(SIOCGIFHWADDR) && !defined(SOLARIS)
    ret=ioctl(so, SIOCGIFHWADDR, &ifr);
#else
    ret=ioctl(so, SIOCGIFADDR, &ifr);
#endif

    if (ret < 0) {
        OSL_TRACE( "SIOCGIFADDR: %s", strerror(errno) );
        memset(hard_addr, 0, 32);
        close(so);
        return ret;
    }

    close(so);

#if defined(SIOCGIFHWADDR) && !defined(SOLARIS)
    memcpy(hard_addr,ifr.ifr_hwaddr.sa_data,8);
#else
    memcpy(hard_addr,ifr.ifr_ifru.ifru_addr.sa_data,8);
#endif


    /*
     *  Check, if no real, i.e. 00:00:00:00:00:00, address was retrieved.
     *  The Linux dummy device has this kind of behaviour
     */

    ret=osl_checkAddr(hard_addr);

    if (ret < 0) {
        OSL_TRACE( "SIOCGIFADDR got '00:00:00:00:00:00'" );
        return ret;
    }

    return 1;
}


/*****************************************************************************/
/* osl_checkAddr */
/*****************************************************************************/

static int osl_checkAddr(const char* addr)
{
    if (addr[0]==0 && addr[1]==0 &&
        addr[2]==0 && addr[3]==0 &&
        addr[4]==0 && addr[5]==0)
    {
        return -1;
    }
    return 0;
}


#if defined (SPARC)

#if defined (SOLARIS) && !defined(__sparcv8plus) && !defined(__sparcv9)
#include <sys/types.h>
#include <sys/processor.h>

/*****************************************************************************/
/* osl_InitSparcV9 */
/*****************************************************************************/

void osl_InterlockedCountSetV9(sal_Bool bV9);

/*
 * osl_InitSparcV9() should be executed as early as possible. We place it in the
 * .init section of sal
 */
#if defined ( __SUNPRO_C ) || defined ( __SUNPRO_CC )
void osl_InitSparcV9(void);
#pragma init (osl_InitSparcV9)
#elif defined ( __GNUC__ )
void osl_InitSparcV9(void)  __attribute__((constructor));
#endif

void osl_InitSparcV9(void)
{
    /* processor_info() identifies SPARCV8 (ie sun4c machines) simply as "sparc"
     * and SPARCV9 (ie ultra sparcs, sun4u) as "sparcv9". Since we know that we
     * run at least on a SPARCV8 architecture or better, any processor type != "sparc"
     * and != "i386" is considered to be SPARCV9 or better
     *
     * This way we are certain that this will still work if someone names SPARCV10
     * "foobar"
     */
    processor_info_t aInfo;
    int rc;

    rc = processor_info(0, &aInfo);

    if ( rc != -1 ) {
        if ( !strcmp( "sparc", aInfo.pi_processor_type )    /* SPARCV8 */
            || !strcmp( "i386", aInfo.pi_processor_type ) ) /* can't happen, but ... */
            return;
        /* we are reasonably certain to be on sparcv9/sparcv8plus or better */
        osl_InterlockedCountSetV9(sal_True);
    }
}

#endif /* SOLARIS */

#if defined(NETBSD) && defined(__GNUC__) && !defined(__sparcv9) && !defined(__sparc_v9__)

#include <sys/param.h>
#include <sys/sysctl.h>
void osl_InitSparcV9(void)  __attribute__((constructor));
void osl_InterlockedCountSetV9(sal_Bool bV9);

/* Determine which processor we are running on (sparc v8 or v9)
 * The approach is very similar to Solaris.
 */

void osl_InitSparcV9(void)
{
    int mib[2]={CTL_HW,HW_MACHINE};
    char processorname[256];
    size_t len=256;

    /* get the machine name */
    sysctl(mib, 2, processorname, &len, NULL, 0);
    if (!strncmp("sparc64",processorname, len)) {
        osl_InterlockedCountSetV9(sal_True);
    }
}

#endif /* NETBSD */

#endif /* SPARC */

#if defined ( LINUX ) && defined ( SPARC )
#include <sys/utsname.h>
void osl_InitSparcV9(void)  __attribute__((constructor));
void osl_InterlockedCountSetV9(sal_Bool bV9);
/* Determine which processor we are running on (sparc v8 or v9)
 * The approach is very similar to Solaris.
 */
void osl_InitSparcV9(void)
{
    struct utsname name;
    int rc;
    rc = uname(&name);
    if ( rc != -1 ) {
        if ( !strcmp( "sparc", name.machine ))
        return;
    osl_InterlockedCountSetV9(sal_True);
    }
}
#endif

#if    ( defined(__GNUC__) && (defined(X86) || defined(X86_64)) )\
    || ( defined(SOLARIS) && defined(__i386) )

/* Safe default */
int osl_isSingleCPU = 0;

/* Determine if we are on a multiprocessor/multicore/HT x86/x64 system
 *
 * The lock prefix for atomic operations in osl_[inc|de]crementInterlockedCount()
 * comes with a cost and is especially expensive on pre HT x86 single processor
 * systems, where it isn't needed at all.
 *
 * This should be run as early as possible, thus it's placed in the init section
 */
#if defined(_SC_NPROCESSORS_CONF) /* i.e. MACOSX for Intel doesn't have this */
#if defined(__GNUC__)
void osl_interlockedCountCheckForSingleCPU(void)  __attribute__((constructor));
#elif defined(__SUNPRO_C)
void osl_interlockedCountCheckForSingleCPU(void);
#pragma init (osl_interlockedCountCheckForSingleCPU)
#endif

void osl_interlockedCountCheckForSingleCPU(void)
{
    /* In case sysconfig fails be on the safe side,
     * consider it a multiprocessor/multicore/HT system */
    if ( sysconf(_SC_NPROCESSORS_CONF) == 1 ) {
        osl_isSingleCPU = 1;
    }
}
#endif /* defined(_SC_NPROCESSORS_CONF) */
#endif

//might be useful on other platforms, but doesn't compiler under MACOSX anyway
#if defined(__GNUC__) && defined(LINUX)
//force the __data_start symbol to exist in any executables that link against
//libuno_sal so that dlopening of the libgcj provided libjvm.so on some
//platforms where it needs that symbol will succeed. e.g. Debian mips/lenny
//with gcc 4.3. With this in place the smoketest succeeds with libgcj provided
//java. Quite possibly also required/helpful for s390x/s390 and maybe some
//others. Without it the dlopen of libjvm.so will fail with __data_start
//not found
extern int __data_start[] __attribute__((weak));
extern int data_start[] __attribute__((weak));
extern int _end[] __attribute__((weak));
static void *dummy[] __attribute__((used)) = {__data_start, data_start, _end};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
