/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: remove_quickstart_link.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 03:37:28 $
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

#pragma warning(push, 1) /* disable warnings within system headers */
#pragma warning(disable: 4917)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <msiquery.h>
#pragma warning(pop)

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
