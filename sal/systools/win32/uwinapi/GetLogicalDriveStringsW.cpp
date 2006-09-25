/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: GetLogicalDriveStringsW.cpp,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-09-25 13:16:31 $
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

#include "macros.h"

IMPLEMENT_THUNK( kernel32, WINDOWS, DWORD, WINAPI, GetLogicalDriveStringsW, ( DWORD cchBuffer, LPWSTR lpBufferW ) )
{
    AUTO_STR( lpBuffer, cchBuffer );

    DWORD   dwResult = GetLogicalDriveStringsA( cchBuffer, lpBufferA );


    if ( dwResult && dwResult < cchBuffer )
        STRBUF2WSTR( lpBuffer, (int) (dwResult + 1), (int) cchBuffer );

    return dwResult;
}