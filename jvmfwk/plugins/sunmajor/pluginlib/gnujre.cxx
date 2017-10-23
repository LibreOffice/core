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


#include <osl/file.hxx>
#include <osl/thread.h>
#include "gnujre.hxx"
#include "util.hxx"

using namespace std;
using namespace osl;
using ::rtl::Reference;

namespace jfw_plugin
{

Reference<VendorBase> GnuInfo::createInstance()
{
    return new GnuInfo;
}

char const* const* GnuInfo::getJavaExePaths(int * size)
{
    static char const * ar[] = {
        "gij",
        "bin/gij",
        "gij-4.3",
        "bin/gij-4.3",
        "gij-4.2",
        "bin/gij-4.2",
        "gij-4.1",
        "bin/gij-4.1"
    };
    *size = SAL_N_ELEMENTS (ar);
    return ar;
}

#if defined(MIPS) && defined(OSL_LITENDIAN)
#define GCJ_JFW_PLUGIN_ARCH "mipsel"
#elif defined(MIPS64) && defined(OSL_LITENDIAN)
#define GCJ_JFW_PLUGIN_ARCH "mips64el"
#else
#define GCJ_JFW_PLUGIN_ARCH JFW_PLUGIN_ARCH
#endif

char const* const* GnuInfo::getRuntimePaths(int * size)
{
    static char const* ar[]= {
        "/libjvm.so",
        "/lib/" GCJ_JFW_PLUGIN_ARCH "/client/libjvm.so",
        "/gcj-4.1.1/libjvm.so",
        "/gcj-4.3-90/libjvm.so",
        "/gcj-4.2-81/libjvm.so",
        "/gcj-4.2/libjvm.so",
        "/gcj-4.2.1/libjvm.so",
        "/gcj-4.2.2/libjvm.so",
        "/gcj-4.2.3/libjvm.so",
        "/gcj-4.1-71/libjvm.so",
        "/gcj-4_1/libjvm.so",
        "/gcj-4.1/libjvm.so",
        "/libgcj.so.81",
        "/libgcj.so.80",
        "/libgcj.so.8",
        "/libgcj.so.71",
        "/libgcj.so.70",
        "/libgcj.so.7",
        "/libgcj.so.6"
    };
    *size = SAL_N_ELEMENTS(ar);
    return ar;
}

char const* const* GnuInfo::getLibraryPaths(int* /*size*/)
{
    return nullptr;
}

bool GnuInfo::initialize(vector<pair<OUString, OUString> > props)
{
    //get java.vendor, java.version, java.home,
    //javax.accessibility.assistive_technologies from system properties

    OUString sJavaLibraryPath;
    OUString const sVendorProperty("java.vendor");
    OUString const sVersionProperty("java.version");
    OUString const sJavaHomeProperty("java.home");
    OUString const sJavaLibraryPathProperty("java.library.path");
    OUString const sGNUHomeProperty("gnu.classpath.home.url");
    OUString const sAccessProperty("javax.accessibility.assistive_technologies");

    bool bVersion = false;
    bool bVendor = false;
    bool bHome = false;
    bool bJavaHome = false;
    bool bJavaLibraryPath = false;
    bool bAccess = false;

    for (auto const& prop : props)
    {
        if(! bVendor && sVendorProperty == prop.first)
        {
            m_sVendor = prop.second;
            bVendor = true;
        }
        else if (!bVersion && sVersionProperty == prop.first)
        {
            m_sVersion = prop.second;
            bVersion = true;
        }
        else if (!bHome && sGNUHomeProperty == prop.first)
        {
            m_sHome = prop.second;
            bHome = true;
        }
        else if (!bJavaHome && sJavaHomeProperty == prop.first)
        {
           OUString fileURL;
           if (osl_getFileURLFromSystemPath(prop.second.pData,& fileURL.pData) ==
               osl_File_E_None)
           {
               //make sure that the drive letter have all the same case
               //otherwise file:///c:/jre and file:///C:/jre produce two
               //different objects!!!
               if (makeDriveLetterSame( & fileURL))
               {
                   m_sJavaHome = fileURL;
                   bJavaHome = true;
               }
           }
        }
        else if (!bJavaLibraryPath && sJavaLibraryPathProperty == prop.first)
        {
            sal_Int32 nIndex = 0;
            osl_getFileURLFromSystemPath(prop.second.getToken(0, ':', nIndex).pData, &sJavaLibraryPath.pData);
            bJavaLibraryPath = true;
        }
        else if (!bAccess && sAccessProperty == prop.first)
        {
            if (!prop.second.isEmpty())
            {
                m_bAccessibility = true;
                bAccess = true;
            }
        }
        // the javax.accessibility.xxx property may not be set. Therefore we
        //must search through all properties.

    }
    if (!bVersion || !bVendor || !bHome)
        return false;

    if (m_sJavaHome.isEmpty())
        m_sJavaHome = "file:///usr/lib";

    // init m_sRuntimeLibrary
    OSL_ASSERT(!m_sHome.isEmpty());
    //call virtual function to get the possible paths to the runtime library.

    int size = 0;
    char const* const* arRtPaths = getRuntimePaths( & size);
    vector<OUString> libpaths = getVectorFromCharArray(arRtPaths, size);

    bool bRt = false;
    for (auto const& libpath : libpaths)
    {
        //Construct an absolute path to the possible runtime
        OUString usRt= m_sHome + libpath;
        DirectoryItem item;
        if(DirectoryItem::get(usRt, item) == File::E_None)
        {
            //found runtime lib
            m_sRuntimeLibrary = usRt;
            bRt = true;
            break;
        }
    }

    if (!bRt)
    {
        m_sHome = m_sJavaHome;
        for (auto const& libpath : libpaths)
        {
            //Construct an absolute path to the possible runtime
            OUString usRt= m_sHome + libpath;
            DirectoryItem item;
            if(DirectoryItem::get(usRt, item) == File::E_None)
            {
                //found runtime lib
                m_sRuntimeLibrary = usRt;
                bRt = true;
                break;
            }
        }
    }

    // try to find it by the java.library.path property
    if (!bRt && m_sJavaHome != sJavaLibraryPath)
    {
        m_sHome = sJavaLibraryPath;
        for (auto const& libpath : libpaths)
        {
            //Construct an absolute path to the possible runtime
            OUString usRt= m_sHome + libpath;
            DirectoryItem item;
            if(DirectoryItem::get(usRt, item) == File::E_None)
            {
                //found runtime lib
                m_sRuntimeLibrary = usRt;
                bRt = true;
                break;
            }
        }
    }

#ifdef X86_64
    //Make one last final legacy attempt on x86_64 in case the distro placed it in lib64 instead
    if (!bRt && m_sJavaHome != "file:///usr/lib")
    {
        m_sHome = "file:///usr/lib64";
        for (auto const& libpath : libpaths)
        {
            //Construct an absolute path to the possible runtime
            OUString usRt= m_sHome + libpath;
            DirectoryItem item;
            if(DirectoryItem::get(usRt, item) == File::E_None)
            {
                //found runtime lib
                m_sRuntimeLibrary = usRt;
                bRt = true;
                break;
            }
        }
    }
#endif

    if (!bRt)
        return false;

    // init m_sLD_LIBRARY_PATH
    OSL_ASSERT(!m_sHome.isEmpty());
    size = 0;
    char const * const * arLDPaths = getLibraryPaths( & size);
    vector<OUString> ld_paths = getVectorFromCharArray(arLDPaths, size);

    bool bLdPath = true;
    int c = 0;
    for (auto const& ld_path : ld_paths)
    {
        OUString usAbsUrl= m_sHome + ld_path;
        // convert to system path
        OUString usSysPath;
        if(File::getSystemPathFromFileURL(usAbsUrl, usSysPath) == File::E_None)
        {

            if(c > 0)
                m_sLD_LIBRARY_PATH+= OUStringLiteral1(SAL_PATHSEPARATOR);
            m_sLD_LIBRARY_PATH+= usSysPath;
        }
        else
        {
            bLdPath = false;
            break;
        }
        ++c;
    }
    return bLdPath;
}

int GnuInfo::compareVersions(const OUString&) const
{
    return 0;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
