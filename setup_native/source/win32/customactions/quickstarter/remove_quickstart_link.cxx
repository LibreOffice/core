/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: remove_quickstart_link.cxx,v $
 * $Revision: 1.7 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#pragma warning(disable: 4917)
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <string>
#include "quickstarter.hxx"


extern "C" UINT __stdcall RemoveQuickstarterLink( MSIHANDLE hMSI )
{
    CHAR    szStartupPath[MAX_PATH];

    if ( SHGetSpecialFolderPathA( NULL, szStartupPath, CSIDL_STARTUP, FALSE ) )
    {
        std::string sQuickstartLinkPath = szStartupPath;

        sQuickstartLinkPath += "\\";
        sQuickstartLinkPath += GetQuickstarterLinkName( hMSI );
        sQuickstartLinkPath += ".lnk";

        DeleteFileA( sQuickstartLinkPath.c_str() );
    }

    return ERROR_SUCCESS;
}
