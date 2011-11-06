/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
#if defined(WNT)
        "/bin/client/jvm.dll",
        "/bin/hotspot/jvm.dll",
        "/bin/classic/jvm.dll",
    "/bin/jrockit/jvm.dll"
#elif defined(OS2)
        "/bin/classic/jvm.dll",
        "/bin/client/jvm.dll",
        "/bin/hotspot/jvm.dll"
#elif UNX
#ifdef MACOSX
        "/../../../../../Frameworks/JavaVM.framework/JavaVM"  //as of  1.6.0_22
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
    *size = sizeof(ar) / sizeof (char*);
    return ar;
}

char const* const* OtherInfo::getLibraryPaths(int* size)
{

#ifdef UNX
    static char const * ar[] = {
#ifdef MACOSX
        //mac version does not have a ld library path anymore
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
