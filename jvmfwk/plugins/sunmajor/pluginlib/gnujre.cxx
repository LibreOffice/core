/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: gnujre.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 15:23:47 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_jvmfwk.hxx"

#include "osl/file.hxx"
#include "osl/thread.h"
#include "gnujre.hxx"
#include "util.hxx"

using namespace rtl;
using namespace std;
using namespace osl;

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
    };
    *size = sizeof (ar) / sizeof (char*);
    return ar;
}

#if defined(MIPS) && defined(OSL_LITENDIAN)
#define GCJ_JFW_PLUGIN_ARCH "mipsel"
#else
#define GCJ_JFW_PLUGIN_ARCH JFW_PLUGIN_ARCH
#endif

char const* const* GnuInfo::getRuntimePaths(int * size)
{
    static char const* ar[]= {
          "/lib/" GCJ_JFW_PLUGIN_ARCH "/client/libjvm.so",
          "/gcj-4.1.1/libjvm.so",
          "/libgcj.so.7",
          "/libgcj.so.6"
    };
    *size = sizeof(ar) / sizeof (char*);
    return ar;
}

bool GnuInfo::initialize(vector<pair<OUString, OUString> > props)
{
    //get java.vendor, java.version, java.home,
    //javax.accessibility.assistive_technologies from system properties

    OUString sVendor;
    typedef vector<pair<OUString, OUString> >::const_iterator it_prop;
    OUString sVendorProperty(
        RTL_CONSTASCII_USTRINGPARAM("java.vendor"));
    OUString sVersionProperty(
        RTL_CONSTASCII_USTRINGPARAM("java.version"));
    OUString sJavaHomeProperty(
        RTL_CONSTASCII_USTRINGPARAM("java.home"));
    OUString sGNUHomeProperty(
        RTL_CONSTASCII_USTRINGPARAM("gnu.classpath.home.url"));
    OUString sAccessProperty(
        RTL_CONSTASCII_USTRINGPARAM("javax.accessibility.assistive_technologies"));

    bool bVersion = false;
    bool bVendor = false;
    bool bHome = false;
    bool bJavaHome = false;
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
        else if (!bHome && sGNUHomeProperty.equals(i->first))
        {
            m_sHome = i->second;
            bHome = true;
        }
        else if (!bJavaHome && sJavaHomeProperty.equals(i->first))
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
                   m_sJavaHome = fileURL;
                   bJavaHome = true;
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

    if (!m_sJavaHome.getLength())
        m_sJavaHome = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///usr/lib"));

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
    {
        m_sHome = m_sJavaHome;
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
    }

#ifdef X86_64
    //Make one last final legacy attempt on x86_64 in case the distro placed it in lib64 instead
    if (!bRt && m_sJavaHome != rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///usr/lib")))
    {
        m_sHome = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///usr/lib64"));
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
    }
#endif

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

int GnuInfo::compareVersions(const rtl::OUString&) const
{
    return 0;
}

}
