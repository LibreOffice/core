/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PathFindFileNameW.cpp,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 04:34:18 $
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

#define _SHLWAPI_
#include <shlwapi.h>

#include <mbstring.h>

IMPLEMENT_THUNK( shlwapi, WINDOWS, LPWSTR, WINAPI, PathFindFileNameW,
(
    LPCWSTR lpPathW
))
{
    AUTO_WSTR2STR(lpPath);
    char* pFname = PathFindFileNameA(lpPathA);

    if (pFname > lpPathA)
    {
        *pFname = '\0';
        LPWSTR pOutW = const_cast<LPWSTR>(lpPathW);
        return (pOutW + _mbslen(reinterpret_cast<unsigned char*>(lpPathA)));
    }
    else
        return const_cast<LPWSTR>(lpPathW);
}
