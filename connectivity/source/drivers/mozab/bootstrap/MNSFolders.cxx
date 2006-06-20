/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: MNSFolders.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 01:46:46 $
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
#ifndef _MNSFOLDERS_HXX_
#include <MNSFolders.hxx>
#endif

#ifdef UNIX
#include <sys/types.h>
#include <strings.h>
#endif // End UNIX

#ifdef WNT
#include "pre_include_windows.h"
#include <windows.h>
#include <stdlib.h>
#include <shlobj.h>
#include <objidl.h>
#include "post_include_windows.h"
#endif // End WNT

static const char * DefaultProductDir[] =
{
#if defined(XP_WIN)
    "Mozilla\\",
    "Mozilla\\Firefox\\",
    "Thunderbird\\"
#else
    ".mozilla/",
    ".mozilla/firefox/",
    ".thunderbird/"
#endif
};
#if defined(XP_MAC) || defined(XP_MACOSX)
#define APP_REGISTRY_NAME "Application Registry"
#elif defined(XP_WIN) || defined(XP_OS2)
#define APP_REGISTRY_NAME "registry.dat"
#else
#define APP_REGISTRY_NAME "appreg"
#endif

::rtl::OString getAppDir()
{
#if defined(WNT)
    char szPath[MAX_PATH];
    if (!SHGetSpecialFolderPath(NULL, szPath, CSIDL_APPDATA, 0))
            return ::rtl::OString();
    return ::rtl::OString(szPath) + ::rtl::OString("\\");
#elif defined(UNIX)
    const char* homeDir = getenv("HOME");
    return ::rtl::OString(homeDir) + ::rtl::OString("/");
#endif
}

::rtl::OString getRegistryDir(::com::sun::star::mozilla::MozillaProductType product)
{
    if (product == ::com::sun::star::mozilla::MozillaProductType_Default)
    {
        return ::rtl::OString();
    }
    sal_Int32 type = product - 1;
    return getAppDir() + ::rtl::OString(DefaultProductDir[type]);
}

::rtl::OString getRegistryFileName(::com::sun::star::mozilla::MozillaProductType product)
{
    if (product == ::com::sun::star::mozilla::MozillaProductType_Default)
    {
        return ::rtl::OString();
    }
    return getRegistryDir(product) + ::rtl::OString(APP_REGISTRY_NAME);
}


