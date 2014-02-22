/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */


#include "osl/thread.h"
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
#if defined(WNT)
        "bin/java.exe",
        "jre/bin/java.exe"
#elif defined UNX
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
#if defined(WNT)
        "/bin/client/jvm.dll",
        "/bin/hotspot/jvm.dll",
        "/bin/classic/jvm.dll",
    "/bin/jrockit/jvm.dll"
#elif defined UNX
#ifdef MACOSX
        "/../../../../../Frameworks/JavaVM.framework/JavaVM"  
#else
        "/lib/" JFW_PLUGIN_ARCH "/client/libjvm.so", 
        "/lib/" JFW_PLUGIN_ARCH "/server/libjvm.so", 
        "/lib/" JFW_PLUGIN_ARCH "/classic/libjvm.so", 
        "/lib/" JFW_PLUGIN_ARCH "/jrockit/libjvm.so", 
        "/bin/classic/libjvm.so", 
        "/jre/bin/classic/libjvm.so" 
#endif
#endif

    };
    *size = sizeof(ar) / sizeof (char*);
    return ar;
}

char const* const* OtherInfo::getLibraryPaths(int* size)
{

#if defined(UNX) && !defined(MACOSX)
    
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

    *size = sizeof(ar) / sizeof (char*);
    return ar;
#else
    *size = 0;
    return NULL;
#endif
}

int OtherInfo::compareVersions(const OUString& /*sSecond*/) const
{
    
    
    
    
    return 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
