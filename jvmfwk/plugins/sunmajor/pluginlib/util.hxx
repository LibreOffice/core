/*************************************************************************
 *
 *  $RCSfile: util.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:52:51 $
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
#if !defined INCLUDED_JFW_PLUGIN_UTIL_HXX
#define INCLUDED_JFW_PLUGIN_UTIL_HXX

#include "rtl/ustring.hxx"
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
        if (s1.match(sSub) == true)
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

std::vector<rtl::Reference<VendorBase> > getAllJREInfos(
    const rtl::OUString& sVendor,
    const rtl::OUString& sMinVersion,
    const rtl::OUString& sMaxVersion,
    const std::vector<rtl::OUString> & vecExludeVersions);

bool getJavaProps(
    const rtl::OUString & exePath,
    std::vector<std::pair<rtl::OUString, rtl::OUString> >& props,
    bool * bProcessRun);

void  createJavaInfoFromWinReg(std::vector<rtl::Reference<VendorBase> > & vecInfos);

void bubbleSortVersion(std::vector<rtl::Reference<VendorBase> >& vec);

}

#endif
