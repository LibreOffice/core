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

#include <vendorbase.hxx>
#include "util.hxx"

using namespace std;
using namespace osl;


namespace jfw_plugin
{

MalformedVersionException::~MalformedVersionException() = default;

VendorBase::VendorBase()
{
}

bool VendorBase::initialize(vector<pair<OUString, OUString> > props)
{
    //get java.vendor, java.version, java.home
    //from system properties

    bool bVersion = false;
    bool bVendor = false;
    bool bHome = false;
    bool bArch = false;

    for (auto const& prop : props)
    {
        if(! bVendor && prop.first == "java.vendor")
        {
            m_sVendor = prop.second;
            bVendor = true;
        }
        else if (!bVersion && prop.first == "java.version")
        {
            m_sVersion = prop.second;
            bVersion = true;
        }
        else if (!bHome && prop.first == "java.home")
        {
#ifndef JVM_ONE_PATH_CHECK
           OUString fileURL;
           if (osl_getFileURLFromSystemPath(prop.second.pData,& fileURL.pData) ==
               osl_File_E_None)
           {
               //make sure that the drive letter have all the same case
               //otherwise file:///c:/jre and file:///C:/jre produce two
               //different objects!!!
               if (makeDriveLetterSame( & fileURL))
               {
                   m_sHome = fileURL;
                   bHome = true;
               }
           }
#else
           m_sHome = prop.second;
           bHome = true;
#endif
        }
        else if (!bArch && prop.first == "os.arch")
        {
            m_sArch = prop.second;
            bArch = true;
        }
        if (bVendor && bVersion && bHome && bArch) {
            break;
        }
    }
    if (!bVersion || !bVendor || !bHome || !bArch)
        return false;

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
                m_sLD_LIBRARY_PATH+= OUStringChar(SAL_PATHSEPARATOR);
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

const OUString & VendorBase::getVendor() const
{
    return m_sVendor;
}
const OUString & VendorBase::getVersion() const
{
    return m_sVersion;
}

const OUString & VendorBase::getHome() const
{
    return m_sHome;
}

const OUString & VendorBase::getLibraryPath() const
{
    return m_sLD_LIBRARY_PATH;
}

const OUString & VendorBase::getRuntimeLibrary() const
{
    return m_sRuntimeLibrary;
}

bool VendorBase::isValidArch() const
{
    // Warning: These values come from the "os.arch" property.
    // It is not defined what the exact values are.
    // Oracle JRE 8 has "x86" and "amd64", the others were found at http://lopica.sourceforge.net/os.html .
    // There might still be missing some options; we need to extend the check once we find out.
#if defined _WIN64
    return m_sArch == "amd64" || m_sArch == "x86_64";
#elif defined _WIN32
    return m_sArch == "x86" || m_sArch == "i386" || m_sArch == "i686";
#else
    (void)this;
    return true;
#endif
}

bool VendorBase::needsRestart() const
{
    return !getLibraryPath().isEmpty();
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
