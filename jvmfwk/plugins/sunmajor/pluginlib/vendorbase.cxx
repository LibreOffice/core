/*************************************************************************
 *
 *  $RCSfile: vendorbase.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:53:01 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "osl/file.hxx"

#include "vendorbase.hxx"
#include "util.hxx"
#include "sunjre.hxx"
#include "sunversion.hxx"

using namespace std;
using namespace rtl;
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

char const* const * VendorBase::getJavaExePaths(int* size)
{
    static char const * ar[] = {
#ifdef WNT
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
    for (it_prop i = props.begin(); i != props.end(); i++)
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
    for(i_path ip = libpaths.begin(); ip != libpaths.end(); ip++)
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
    for(i_path il = ld_paths.begin(); il != ld_paths.end(); il ++, c++)
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

char const* const* VendorBase::getRuntimePaths(int* size)
{
    return NULL;
}

char const* const* VendorBase::getLibraryPaths(int* size)
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

int VendorBase::compareVersions(const rtl::OUString& sSecond) const
{
    OUString sFirst = getVersion();

    SunVersion version1(sFirst);
    SunVersion version2(sSecond);
    if ( ! (version1 &&  version2))
        throw MalformedVersionException();

    if(version1 == version2)
        return 0;
    if(version1 > version2)
        return 1;
    else
        return -1;
}


}
