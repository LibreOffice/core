/*************************************************************************
 *
 *  $RCSfile: util.c,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:21 $
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
#endif

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

    close(so);
    return sal_False;
}


extern sal_Bool osl_getEtherAddr(sal_Char* pszAddr, sal_uInt16 BufferSize)
{
    char buff[1024];
    char hard_addr[64];
    struct ifconf ifc;
    struct ifreq *ifr;
    int i;
    int so;

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

    close(so);
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
        return ret;
    }


    /*
     *  If it is the loopback device, do not consider it any further
     */

    if (ifr.ifr_flags & IFF_LOOPBACK)
    {
/*      fprintf(stderr, "SIOCGIFFLAGS : is LOOPBACK : %s\n", strerror(errno));*/
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
        return ret;
    }

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
        fprintf(stderr, "SIOCGIFADDR got '00:00:00:00:00:00'\n");
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


