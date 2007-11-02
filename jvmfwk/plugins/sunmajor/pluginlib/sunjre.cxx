/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sunjre.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 12:27:21 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_jvmfwk.hxx"

#include "osl/thread.h"
#include "sunjre.hxx"
#include "sunversion.hxx"
#include "diagnostics.h"

using namespace rtl;
using namespace std;

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
namespace jfw_plugin
{

Reference<VendorBase> SunInfo::createInstance()
{
    return new SunInfo;
}

char const* const* SunInfo::getJavaExePaths(int * size)
{
    static char const * ar[] = {
#if defined(WNT) || defined(OS2)
        "java.exe",
        "bin/java.exe",
        "jre/bin/java.exe"
#elif UNX
        "java",
        "bin/java",
        "jre/bin/java"
#endif
    };
        *size = sizeof (ar) / sizeof (char*);
    return ar;
}

char const* const* SunInfo::getRuntimePaths(int * size)
{
    static char const* ar[]= {
#if defined(WNT) || defined(OS2)
        "/bin/client/jvm.dll",
        "/bin/hotspot/jvm.dll",
        "/bin/classic/jvm.dll"
#elif UNX
        "/lib/" JFW_PLUGIN_ARCH "/client/libjvm.so",
        "/lib/" JFW_PLUGIN_ARCH "/server/libjvm.so",
        "/lib/" JFW_PLUGIN_ARCH "/classic/libjvm.so"
#endif

    };
    *size = sizeof(ar) / sizeof (char*);
    return ar;
}

char const* const* SunInfo::getLibraryPaths(int* size)
{
#ifdef UNX
    static char const * ar[] = {

        "/lib/" JFW_PLUGIN_ARCH "/client",
        "/lib/" JFW_PLUGIN_ARCH "/native_threads",
        "/lib/" JFW_PLUGIN_ARCH

    };
    *size = sizeof(ar) / sizeof (char*);
    return ar;
#else
    size = 0;
    return NULL;
#endif
}

int SunInfo::compareVersions(const rtl::OUString& sSecond) const
{
    OUString sFirst = getVersion();

    SunVersion version1(sFirst);
    JFW_ENSURE(version1, OUSTR("[Java framework] sunjavaplugin"SAL_DLLEXTENSION
                               " does not know the version: ")
               + sFirst + OUSTR(" as valid for a SUN JRE."));
    SunVersion version2(sSecond);
    if ( ! version2)
        throw MalformedVersionException();

    if(version1 == version2)
        return 0;
    if(version1 > version2)
        return 1;
    else
        return -1;
}


}
