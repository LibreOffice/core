/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include "osl/thread.h"
#include "sunjre.hxx"
#include "sunversion.hxx"
#include "diagnostics.h"

using namespace std;

#define OUSTR(x) ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(x) )
namespace jfw_plugin
{

rtl::Reference<VendorBase> SunInfo::createInstance()
{
    return new SunInfo;
}

char const* const* SunInfo::getJavaExePaths(int * size)
{
    static char const * ar[] = {
#if defined(WNT)
        "java.exe",
        "bin/java.exe",
        "jre/bin/java.exe"
#elif UNX
        "java",
        "bin/java",
        "jre/bin/java"
#endif
    };
    *size = SAL_N_ELEMENTS(ar);
    return ar;
}

char const* const* SunInfo::getRuntimePaths(int * size)
{
    static char const* ar[]= {
#if defined(WNT)
        "/bin/client/jvm.dll",
        "/bin/hotspot/jvm.dll",
        "/bin/classic/jvm.dll",
        // The 64-bit JRE has the jvm in bin/server
        "/bin/server/jvm.dll"
#elif UNX
        "/lib/" JFW_PLUGIN_ARCH "/client/libjvm.so",
        "/lib/" JFW_PLUGIN_ARCH "/server/libjvm.so",
        "/lib/" JFW_PLUGIN_ARCH "/classic/libjvm.so"
#endif
    };
    *size = SAL_N_ELEMENTS(ar);
    return ar;
}

char const* const* SunInfo::getLibraryPaths(int* size)
{
#ifdef UNX
    static char const * ar[] = {
        "/lib/" JFW_PLUGIN_ARCH "/client",
        "/lib/" JFW_PLUGIN_ARCH "/server",
        "/lib/" JFW_PLUGIN_ARCH "/native_threads",
        "/lib/" JFW_PLUGIN_ARCH
    };
    *size = SAL_N_ELEMENTS(ar);
    return ar;
#else
    *size = 0;
    return NULL;
#endif
}

int SunInfo::compareVersions(const rtl::OUString& sSecond) const
{
    rtl::OUString sFirst = getVersion();

    SunVersion version1(sFirst);
    JFW_ENSURE(version1, OUSTR("[Java framework] sunjavaplugin" SAL_DLLEXTENSION
                               " does not know the version: ")
               + sFirst + OUSTR(" as valid for a SUN/Oracle JRE."));
    SunVersion version2(sSecond);
    if ( ! version2)
        throw MalformedVersionException();

    if (version1 == version2)
        return 0;
    if (version1 > version2)
        return 1;
    else
        return -1;
}


}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
