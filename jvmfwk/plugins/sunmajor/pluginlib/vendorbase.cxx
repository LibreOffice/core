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


#include "osl/file.hxx"

#include "vendorbase.hxx"
#include "util.hxx"
#include "sunjre.hxx"

using namespace std;
using namespace osl;


namespace jfw_plugin
{
rtl::Reference<VendorBase> createInstance(createInstance_func pFunc,
                                          vector<pair<OUString, OUString> > properties);







//##############################################################################

MalformedVersionException::MalformedVersionException()
{}
MalformedVersionException::MalformedVersionException(
    const MalformedVersionException & )
{}
MalformedVersionException::~MalformedVersionException()
{}
MalformedVersionException &
MalformedVersionException::operator =(
    const MalformedVersionException &)
{
    return *this;
}
//##############################################################################


VendorBase::VendorBase(): m_bAccessibility(false)
{
}

rtl::Reference<VendorBase> VendorBase::createInstance()
{
    VendorBase *pBase = new VendorBase();
    return rtl::Reference<VendorBase>(pBase);
}

bool VendorBase::initialize(vector<pair<OUString, OUString> > props)
{
    //get java.vendor, java.version, java.home,
    //javax.accessibility.assistive_technologies from system properties

    typedef vector<pair<OUString, OUString> >::const_iterator it_prop;
    OUString sVendorProperty("java.vendor");
    OUString sVersionProperty("java.version");
    OUString sHomeProperty("java.home");
    OUString sAccessProperty("javax.accessibility.assistive_technologies");

    bool bVersion = false;
    bool bVendor = false;
    bool bHome = false;
    bool bAccess = false;

    typedef vector<pair<OUString, OUString> >::const_iterator it_prop;
    for (it_prop i = props.begin(); i != props.end(); ++i)
    {
        if(! bVendor && sVendorProperty.equals(i->first))
        {
            m_sVendor = i->second;
            bVendor = true;
        }
        else if (!bVersion && sVersionProperty.equals(i->first))
        {
            m_sVersion = i->second;
            bVersion = true;
        }
        else if (!bHome && sHomeProperty.equals(i->first))
        {
#ifndef JVM_ONE_PATH_CHECK
           OUString fileURL;
           if (osl_getFileURLFromSystemPath(i->second.pData,& fileURL.pData) ==
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
           m_sHome = i->second;
           bHome = true;
#endif
        }
        else if (!bAccess && sAccessProperty.equals(i->first))
        {
            if (!i->second.isEmpty())
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

    // init m_sRuntimeLibrary
    OSL_ASSERT(!m_sHome.isEmpty());
    //call virtual function to get the possible paths to the runtime library.

    int size = 0;
    char const* const* arRtPaths = getRuntimePaths( & size);
    vector<OUString> libpaths = getVectorFromCharArray(arRtPaths, size);

    bool bRt = false;
    typedef vector<OUString>::const_iterator i_path;
    for(i_path ip = libpaths.begin(); ip != libpaths.end(); ++ip)
    {
        //Construct an absolute path to the possible runtime
        OUString usRt= m_sHome + *ip;
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

    char arSep[]= {SAL_PATHSEPARATOR, 0};
    OUString sPathSep= OUString::createFromAscii(arSep);
    bool bLdPath = true;
    int c = 0;
    for(i_path il = ld_paths.begin(); il != ld_paths.end(); ++il, ++c)
    {
        OUString usAbsUrl= m_sHome + *il;
        // convert to system path
        OUString usSysPath;
        if(File::getSystemPathFromFileURL(usAbsUrl, usSysPath) == File::E_None)
        {

            if(c > 0)
                m_sLD_LIBRARY_PATH+= sPathSep;
            m_sLD_LIBRARY_PATH+= usSysPath;
        }
        else
        {
            bLdPath = false;
            break;
        }
    }
    if (bLdPath == false)
        return false;

    return true;
}

char const* const* VendorBase::getRuntimePaths(int* /*size*/)
{
    return NULL;
}

char const* const* VendorBase::getLibraryPaths(int* /*size*/)
{
    return NULL;
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

const OUString & VendorBase::getLibraryPaths() const
{
    return m_sLD_LIBRARY_PATH;
}

const OUString & VendorBase::getRuntimeLibrary() const
{
    return m_sRuntimeLibrary;
}
bool VendorBase::supportsAccessibility() const
{
    return m_bAccessibility;
}

bool VendorBase::needsRestart() const
{
    if (!getLibraryPaths().isEmpty())
        return true;
    return false;
}

int VendorBase::compareVersions(const OUString& /*sSecond*/) const
{
    OSL_FAIL("[Java framework] VendorBase::compareVersions must be "
               "overridden in derived class.");
    return 0;
}




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
