/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include "osl/file.hxx"

#include "vendorbase.hxx"
#include "util.hxx"
#include "sunjre.hxx"

using namespace std;
using namespace osl;

using ::rtl::OUString;

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

char const* const * VendorBase::getJavaExePaths(int* size)
{
    static char const * ar[] = {
#if defined(WNT) || defined(OS2)
        "java.exe",
        "bin/java.exe"
#elif UNX
        "java",
        "bin/java"
#endif
    };
    *size = sizeof(ar) / sizeof(char*);
    return ar;
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

    OUString sVendor;
    typedef vector<pair<OUString, OUString> >::const_iterator it_prop;
    OUString sVendorProperty(
        RTL_CONSTASCII_USTRINGPARAM("java.vendor"));
    OUString sVersionProperty(
        RTL_CONSTASCII_USTRINGPARAM("java.version"));
    OUString sHomeProperty(
        RTL_CONSTASCII_USTRINGPARAM("java.home"));
    OUString sAccessProperty(
        RTL_CONSTASCII_USTRINGPARAM("javax.accessibility.assistive_technologies"));

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
        }
        else if (!bAccess && sAccessProperty.equals(i->first))
        {
            if (i->second.getLength() > 0)
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
    OSL_ASSERT(m_sHome.getLength());
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
    OSL_ASSERT(m_sHome.getLength());
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
    if (getLibraryPaths().getLength() > 0)
        return true;
    return false;
}

int VendorBase::compareVersions(const rtl::OUString& /*sSecond*/) const
{
    OSL_ENSURE(0, "[Java framework] VendorBase::compareVersions must be "
               "overridden in derived class.");
    return 0;
}




}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
