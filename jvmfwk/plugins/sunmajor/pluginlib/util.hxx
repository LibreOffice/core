/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: util.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:32:31 $
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
