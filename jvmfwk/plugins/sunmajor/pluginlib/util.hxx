/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: util.hxx,v $
 * $Revision: 1.7 $
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
#if !defined INCLUDED_JFW_PLUGIN_UTIL_HXX
#define INCLUDED_JFW_PLUGIN_UTIL_HXX

#include "rtl/ustring.hxx"
#include "rtl/bootstrap.hxx"
#include <vector>
#include "vendorbase.hxx"

namespace jfw_plugin
{

class VendorBase;
std::vector<rtl::OUString> getVectorFromCharArray(char const * const * ar, int size);

/*   The function uses the relative paths, such as "bin/java.exe" as provided by
     VendorBase::getJavaExePaths and the provided path to derive the the home directory.
     The home directory is then used as argument to getJREInfoByPath. For example
     usBinDir is file:///c:/j2sdk/jre/bin then file:///c:/j2sdk/jre would be derived.
 */
bool getJREInfoFromBinPath(
    const rtl::OUString& path, std::vector<rtl::Reference<VendorBase> > & vecInfos);
inline rtl::OUString getDirFromFile(const rtl::OUString& usFilePath);
void createJavaInfoFromPath(std::vector<rtl::Reference<VendorBase> >& vecInfos);
void createJavaInfoFromJavaHome(std::vector<rtl::Reference<VendorBase> > &vecInfos);
void createJavaInfoDirScan(std::vector<rtl::Reference<VendorBase> >& vecInfos);
#ifdef WNT
void createJavaInfoFromWinReg(std::vector<rtl::Reference<VendorBase> >& vecInfos);
#endif

bool makeDriveLetterSame(rtl::OUString * fileURL);


/* for std::find_if
   Used to find a JavaInfo::Impl object in a std::vector<Impl*> which has a member usJavaHome
   as the specified string in the constructor.
*/
struct InfoFindSame
{
    rtl::OUString sJava;
    InfoFindSame(const rtl::OUString& sJavaHome):sJava(sJavaHome){}

    bool operator () (const rtl::Reference<VendorBase> & aVendorInfo)
    {
        return aVendorInfo->getHome().equals(sJava) == sal_True ? true : false;
    }
};

struct SameOrSubDirJREMap
{
    rtl::OUString s1;
    SameOrSubDirJREMap(const rtl::OUString& s):s1(s){
    }

    bool operator () (const std::pair<const rtl::OUString, rtl::Reference<VendorBase> > & s2)
    {
        if (s1 == s2.first)
            return true;
        rtl::OUString sSub;
        sSub = s2.first + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
        if (s1.match(sSub) == sal_True)
            return true;
        return false;
    }
};


/* Creates a VendorBase object if a JRE could be found at the specified path.

   This depends if there is a JRE at all and if it is from a vendor that
   is supported by this plugin.
 */
rtl::Reference<VendorBase> getJREInfoByPath(const rtl::OUString& path);

/* Creates a VendorBase object if a JRE could be found at the specified path.

   The difference to the other getJREInfoByPath is that this function checks
   first if the path corresponds to one of the VendorBase::getHome path already
   contained in vecInfo. Only if there is no such entry, then the other
   getJREInfoByPath is called. Again the created VendorBase is compared to
   those contained in vecInfos. If it it not in there then it's added.

   @return
   true a VendorBase was created and added to the end of vecInfos.
   false - no VendorBase has been created. Either the path did not represent a
   supported JRE installation or there was already a VendorBase in vecInfos.
 */
bool getJREInfoByPath(const rtl::OUString& path,
                      std::vector<rtl::Reference<VendorBase> > & vecInfos);

std::vector<rtl::Reference<VendorBase> > getAllJREInfos();

bool getJavaProps(
    const rtl::OUString & exePath,
    std::vector<std::pair<rtl::OUString, rtl::OUString> >& props,
    bool * bProcessRun);

void  createJavaInfoFromWinReg(std::vector<rtl::Reference<VendorBase> > & vecInfos);

void bubbleSortVersion(std::vector<rtl::Reference<VendorBase> >& vec);

rtl::Bootstrap* getBootstrap();
}

#endif
