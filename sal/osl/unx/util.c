/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: util.c,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: rt $ $Date: 2006-10-27 11:59:44 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


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



/*****************************************************************************/
/* Static Module Functions */
/*****************************************************************************/

static int   osl_getHWAddr(const char *ifname, char* hard_addr);
static int   osl_checkAddr(const char* addr);
static char* osl_decodeEtherAddr(const char *ptr, char* buff);


/*****************************************************************************/
/* osl_getEtherAddr */
/*****************************************************************************/

sal_Bool SAL_CALL osl_getEthernetAddress( sal_uInt8 * pAddr )
{
    char buff[1024];
    char hard_addr[64];
    struct ifconf ifc;
    struct ifreq *ifr;
    int i;
    int so;

#ifdef SOLARIS
    /** algorithm doesn't work on solaris */
    return sal_False;
#else

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
/*      fprintf(stderr, "SIOCGIFCONF: %s\n", strerror(errno));*/
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


extern sal_Bool osl_getEtherAddr(sal_Char* pszAddr, sal_uInt16 BufferSize)
{
    char buff[1024];
    char hard_addr[64];
    struct ifconf ifc;
    struct ifreq *ifr;
    int i;
    int so;

    (void) BufferSize; /* unused */

    if ( pszAddr == 0 )
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
/*      fprintf(stderr, "SIOCGIFCONF: %s\n", strerror(errno));*/
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
            osl_decodeEtherAddr(hard_addr,pszAddr);
            return sal_True;
        }
    }

    return sal_False;
}


/*****************************************************************************/
/* osl_getHWAddr */
/*****************************************************************************/

static int osl_getHWAddr(const char *ifname, char* hard_addr)
{
    int ret=0;
    struct ifreq ifr;
    int so = socket(AF_INET, SOCK_DGRAM, 0);

    strcpy(ifr.ifr_name, ifname);

    /*
     *   First, get the Interface-FLAGS
     */

    ret=ioctl(so, SIOCGIFFLAGS, &ifr) ;

    if ( ret < 0 )
    {
/*      fprintf(stderr, "SIOCGIFFLAGS: %s\n", strerror(errno)); */
        close(so);
        return ret;
    }


    /*
     *  If it is the loopback device, do not consider it any further
     */

    if (ifr.ifr_flags & IFF_LOOPBACK)
    {
/*      fprintf(stderr, "SIOCGIFFLAGS : is LOOPBACK : %s\n", strerror(errno));*/
        close(so);
        return 0;
    }


    /*
     *  And now, the real thing: the get address
     */

#ifdef LINUX
    ret=ioctl(so, SIOCGIFHWADDR, &ifr);
#else
    ret=ioctl(so, SIOCGIFADDR, &ifr);
#endif

    if (ret < 0) {
/*      fprintf(stderr, "SIOCGIFADDR: %s\n", strerror(errno));*/
        memset(hard_addr, 0, 32);
        close(so);
        return ret;
    }

    close(so);

#ifdef LINUX
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
/*      fprintf(stderr, "SIOCGIFADDR got '00:00:00:00:00:00'\n"); */
        return ret;
    }

/*  fprintf(stderr,"interface : %s -- ",ifname);*/
/*  fprintf(stderr,"HWaddr : %s\n", print_ether(hard_addr));*/

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


/*****************************************************************************/
/* osl_decodeEtherAddr */
/*****************************************************************************/

static char* osl_decodeEtherAddr(const char *ptr, char* buff)
{
    sprintf(buff, "%02X:%02X:%02X:%02X:%02X:%02X",
            (ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
            (ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377));
    return(buff);
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

#if defined(NETBSD) && defined(GCC) && !defined(__sparcv9) && !defined(__sparc_v9__)

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

#if    ( defined(LINUX) && defined(__GNUC__) && (defined(X86) || defined(X86_64)) )\
    || ( defined(SOLARIS) && defined (__SUNPRO_C) && defined(__i386) )
/* Determine if we are on a multiprocessor/multicore/HT x86/x64 system
 *
 * The lock prefix for atomic operations in osl_[inc|de]crementInterlockedCount()
 * comes with a cost and is especially expensive on pre HT x86 single processor
 * systems, where it isn't needed at all.
 *
 * This should be run as early as possible, thus it's placed in the init section
 */
#if defined(__GNUC__)
void osl_interlockedCountCheckForSingleCPU(void)  __attribute__((constructor));
#elif defined(__SUNPRO_C)
void osl_interlockedCountCheckForSingleCPU(void);
#pragma init (osl_interlockedCountCheckForSingleCPU)
#endif

int osl_isSingleCPU = 0;
void osl_interlockedCountCheckForSingleCPU(void)
{
    /* In case sysconfig fails be on the safe side,
     * consider it a multiprocessor/multicore/HT system */
    if ( sysconf(_SC_NPROCESSORS_CONF) == 1 ) {
        osl_isSingleCPU = 1;
    }
}
#endif
