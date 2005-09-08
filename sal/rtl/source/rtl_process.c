/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: rtl_process.c,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:04:35 $
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
#include <string.h>
#include <osl/mutex.h>
#include <rtl/uuid.h>

/* rtl_getCommandArg, rtl_getCommandArgCount see cmdargs.cxx  */

void SAL_CALL rtl_getGlobalProcessId( sal_uInt8 *pTargetUUID )
{
    static sal_uInt8 *pUuid = 0;
    if( ! pUuid )
    {
        osl_acquireMutex( * osl_getGlobalMutex() );
        if( ! pUuid )
        {
            static sal_uInt8 aUuid[16];
            rtl_createUuid( aUuid , 0 , sal_False );
            pUuid = aUuid;
        }
        osl_releaseMutex( * osl_getGlobalMutex() );
    }
    memcpy( pTargetUUID , pUuid , 16 );
}

