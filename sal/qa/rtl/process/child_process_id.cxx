/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: child_process_id.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:27:12 $
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

#include <stdio.h>

#ifndef _SAL_MAIN_H_
#include "sal/main.h"
#endif

#ifndef _RTL_PROCESS_H_
#include <rtl/process.h>
#endif

#ifndef _RTL_PROCESS_CONST_H_
#include <rtl_Process_Const.h>
#endif

void printUuid( sal_uInt8 *pNode )
{
    for( sal_Int32 i1 = 0 ; i1 < 4 ; i1++ )
    {
        for( sal_Int32 i2 = 0 ; i2 < 4 ; i2++ )
        {
            sal_uInt8 nValue = pNode[i1*4 +i2];
            if (nValue < 16)
            {
                printf( "0");
            }
            printf( "%02x" ,nValue );
        }
        if( i1 == 3 )
            break;
        //printf( "-" );
    }
}

// ----------------------------------- Main -----------------------------------

SAL_IMPLEMENT_MAIN_WITH_ARGS(,)
{
    sal_uInt8 pTargetUUID[16];
    rtl_getGlobalProcessId( pTargetUUID );
    printUuid( pTargetUUID );
    return 1;
}
