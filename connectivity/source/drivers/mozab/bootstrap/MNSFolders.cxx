/*************************************************************************
 *
 *  $RCSfile: MNSFolders.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2005-02-21 12:25:41 $
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
#ifndef _MNSFOLDERS_HXX_
#include <MNSFolders.hxx>
#endif

#ifdef UNIX
#include <sys/types.h>
#include <strings.h>
#endif // End UNIX

#ifdef WNT
#include <windows.h>
#include <stdlib.h>
#include <shlobj.h>
#include <objidl.h>
#endif // End WNT

static char * DefaultProductDir[] =
{
#if defined(XP_WIN) || defined(XP_MAC) || defined(XP_MACOSX)
    "Mozilla\\",
    "Mozilla\\Firefox\\",
    "Thunderbird\\"
#else
    ".mozilla\/",
    ".mozilla\/firefox\/",
    ".thunderbird\/"
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
    return ::rtl::OString(homeDir) + ::rtl::OString("\/");
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


