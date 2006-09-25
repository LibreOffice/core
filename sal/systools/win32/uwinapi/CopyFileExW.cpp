/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CopyFileExW.cpp,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-09-25 13:15:11 $
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
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning(disable:4740)
#endif

#define _WIN32_WINNT 0x0400
#include "macros.h"

IMPLEMENT_THUNK( kernel32, WINDOWS, BOOL, WINAPI, CopyFileExW, ( LPCWSTR lpExistingFileNameW, LPCWSTR lpNewFileNameW, LPPROGRESS_ROUTINE  lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags ) )
{
    AUTO_WSTR2STR( lpExistingFileName );
    AUTO_WSTR2STR( lpNewFileName );

    return CopyFileExA( lpExistingFileNameA, lpNewFileNameA, lpProgressRoutine, lpData, pbCancel, dwCopyFlags );
}