/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: otherjre.cxx,v $
 * $Revision: 1.15 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_jvmfwk.hxx"

#include "osl/thread.h"
#include "otherjre.hxx"

using namespace rtl;
using namespace std;


namespace jfw_plugin
{

Reference<VendorBase> OtherInfo::createInstance()
{
    return new OtherInfo;
}


char const* const* OtherInfo::getJavaExePaths(int * size)
{
    static char const * ar[] = {
#if defined(WNT) || defined(OS2)
        "bin/java.exe",
        "jre/bin/java.exe"
#elif UNX
        "bin/java",
        "jre/bin/java"
#endif
    };
        *size = sizeof (ar) / sizeof (char*);
    return ar;
}

char const* const* OtherInfo::getRuntimePaths(int * size)
{
    static char const* ar[]= {
#if defined(WNT) || defined(OS2)
        "/bin/client/jvm.dll",
        "/bin/hotspot/jvm.dll",
        "/bin/classic/jvm.dll",
    "/bin/jrockit/jvm.dll"
#elif UNX
#ifdef MACOSX
        "/../../../JavaVM"
#else
    "/bin/classic/libjvm.so", // for IBM Java
        "/jre/bin/classic/libjvm.so", // for IBM Java
        "/lib/" JFW_PLUGIN_ARCH "/client/libjvm.so", // for Blackdown PPC
        "/lib/" JFW_PLUGIN_ARCH "/server/libjvm.so", // for Blackdown AMD64
        "/lib/" JFW_PLUGIN_ARCH "/classic/libjvm.so", // for Blackdown PPC
    "/lib/" JFW_PLUGIN_ARCH "/jrockit/libjvm.so" // for Java of BEA Systems
#endif
#endif

    };
    *size = sizeof(ar) / sizeof (char*);
    return ar;
}

char const* const* OtherInfo::getLibraryPaths(int* size)
{

#ifdef UNX
    static char const * ar[] = {
#ifdef MACOSX
        "/../Libraries",
        "/lib"
#else
        "/bin",
        "/jre/bin",
        "/bin/classic",
        "/jre/bin/classic",
        "/lib/" JFW_PLUGIN_ARCH "/client",
        "/lib/" JFW_PLUGIN_ARCH "/server",
        "/lib/" JFW_PLUGIN_ARCH "/classic",
        "/lib/" JFW_PLUGIN_ARCH "/jrockit",
        "/lib/" JFW_PLUGIN_ARCH "/native_threads",
        "/lib/" JFW_PLUGIN_ARCH
#endif
    };

    *size = sizeof(ar) / sizeof (char*);
    return ar;
#else
    size = 0;
    return NULL;
#endif
}

int OtherInfo::compareVersions(const rtl::OUString& /*sSecond*/) const
{
    //Need to provide an own algorithm for comparing version.
    //Because this function returns always 0, which means the version of
    //this JRE and the provided version "sSecond" are equal, one cannot put
    //any excludeVersion entries in the javavendors.xml file.
    return 0;
}

}
