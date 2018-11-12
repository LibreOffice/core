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


#include <osl/thread.h>
#include "otherjre.hxx"

using namespace std;

using ::rtl::Reference;
namespace jfw_plugin
{

Reference<VendorBase> OtherInfo::createInstance()
{
    return new OtherInfo;
}


char const* const* OtherInfo::getJavaExePaths(int * size)
{
    static char const * ar[] = {
#if defined(_WIN32)
        "bin/java.exe",
        "jre/bin/java.exe"
#elif defined UNX
        "bin/java",
        "jre/bin/java"
#endif
    };
    *size = SAL_N_ELEMENTS (ar);
    return ar;
}

char const* const* OtherInfo::getRuntimePaths(int * size)
{
    static char const* ar[]= {
#if defined(_WIN32)
        "/bin/client/jvm.dll",
        "/bin/hotspot/jvm.dll",
        "/bin/classic/jvm.dll",
        "/bin/jrockit/jvm.dll",
        "/bin/server/jvm.dll" // needed by Azul
#elif defined UNX
#ifdef MACOSX
        "/../../../../../Frameworks/JavaVM.framework/JavaVM", //as of  1.6.0_22
        "/lib/server/libjvm.dylib" // needed by Azul
#else
        "/lib/" JFW_PLUGIN_ARCH "/client/libjvm.so", // for Blackdown PPC
        "/lib/" JFW_PLUGIN_ARCH "/server/libjvm.so", // for Blackdown AMD64
        "/lib/" JFW_PLUGIN_ARCH "/classic/libjvm.so", // for Blackdown PPC
        "/lib/" JFW_PLUGIN_ARCH "/jrockit/libjvm.so", // for Java of BEA Systems
        "/bin/classic/libjvm.so", // fallback for older for IBM Java
        "/jre/bin/classic/libjvm.so" // fallback for older for IBM Java
#endif
#endif

    };
    *size = SAL_N_ELEMENTS(ar);
    return ar;
}

char const* const* OtherInfo::getLibraryPaths(int* size)
{

#if defined(UNX) && !defined(MACOSX)
    //mac version does not have a ld library path anymore
    static char const * ar[] = {
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
    };

    *size = SAL_N_ELEMENTS(ar);
    return ar;
#else
    *size = 0;
    return nullptr;
#endif
}

int OtherInfo::compareVersions(const OUString& /*sSecond*/) const
{
    //Need to provide an own algorithm for comparing version.
    //Because this function returns always 0, which means the version of
    //this JRE and the provided version "sSecond" are equal, one cannot put
    //any excludeVersion entries in the javavendors.xml file.
    return 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
