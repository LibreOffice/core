/*************************************************************************
 *
 *  $RCSfile: system.h,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:17:20 $
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

/* toolkit header */

#define BSD_SELECT

#ifdef GCC
#include <sys/types.h>
#else
#include <types.h>
#endif
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netdb.h>
/*
#include <s:/solenv/os2icci3/inc/types.h>
#include <s:/solenv/os2icci3/inc/netinet/in.h>
#include <s:/solenv/os2icci3/inc/sys/socket.h>
#include <s:/solenv/os2icci3/inc/tcpip/sys/select.h>
#include <s:/solenv/os2icci3/inc/tcpip/sys/ioctl.h>
#include <s:/solenv/os2icci3/inc/netdb.h>
*/
/* OS/2 API header */

#define INCL_WINPROGRAMLIST
#define INCL_WINSHELLDATA
#define INCL_BASE
#define INCL_DOSSIGNALS
#define INCL_DOSSEMAPHORES
#define INCL_DOSMODULEMGR
#define INCL_DOSERRORS
#define INCL_DOSSESMGR
#define INCL_DOSPROCESS
#define INCL_DOSNMPIPES
#define INCL_DOSMISC
#include <os2.h>

/* compiler header */

#include <limits.h>
#include <ctype.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys\stat.h>
#include <share.h>
#include <io.h>


/* no header file - defined in the runtime startup code */
int _CRT_init(void);

/* BEGIN HACK */
/* dummy define and declarations for IPX should be replaced by */
/* original ipx headers when these are available for this platform */

typedef struct sockaddr_ipx {
    short sa_family;
    char  sa_netnum[4];
    char  sa_nodenum[6];
    unsigned short sa_socket;
} SOCKADDR_IPX;

#define NSPROTO_IPX      1000
#define NSPROTO_SPX      1256
#define NSPROTO_SPXII    1257

#define AF_IPX          AF_NS           /* IPX protocols: IPX, SPX, etc. */

#define WSAEDISCON              (WSABASEERR+101)

/* END HACK */

#define HANDLE ULONG

#define SOCKET int

#define TCP_NODELAY     0x01    /* don't delay send to coalesce packets */

#define _MAX_ENV        2048
#define _MAX_CMD        2048

#   define  PTR_SIZE_T(s)               ((int *)&(s))
#   define  PTR_FD_SET(s)               ((int *)&(s))


