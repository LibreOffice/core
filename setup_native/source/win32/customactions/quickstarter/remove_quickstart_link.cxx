/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: remove_quickstart_link.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 16:34:11 $
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
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shlobj.h>
#include <msiquery.h>
#include <string>
#include "quickstarter.hxx"


extern "C" UINT __stdcall RemoveQuickstarterLink( MSIHANDLE hMSI )
{
    CHAR    szStartupPath[MAX_PATH];

    if ( SHGetSpecialFolderPathA( NULL, szStartupPath, CSIDL_STARTUP, FALSE ) )
    {
        std::string sQuickstartLinkPath = szStartupPath;

        sQuickstartLinkPath += "\\";
        sQuickstartLinkPath += GetOfficeProductName( hMSI );
        sQuickstartLinkPath += ".lnk";

        DeleteFileA( sQuickstartLinkPath.c_str() );
    }

    return ERROR_SUCCESS;
}
