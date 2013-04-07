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
#ifndef INCLUDED_JFW_PLUGIN_UTIL_HXX
#define INCLUDED_JFW_PLUGIN_UTIL_HXX

#include "rtl/ustring.hxx"
#include "rtl/bootstrap.hxx"
#include <vector>
#include "vendorbase.hxx"

namespace jfw_plugin
{

class VendorBase;
std::vector<OUString> getVectorFromCharArray(char const * const * ar, int size);

/*   The function uses the relative paths, such as "bin/java.exe" and the provided
     path to derive the home directory. The home directory is then used as
     argument to getJREInfoByPath. For example usBinDir is
     file:///c:/j2sdk/jre/bin then file:///c:/j2sdk/jre would be derived.
 */
bool getJREInfoFromBinPath(
    const OUString& path, std::vector<rtl::Reference<VendorBase> > & vecInfos);
inline OUString getDirFromFile(const OUString& usFilePath);
void createJavaInfoFromPath(std::vector<rtl::Reference<VendorBase> >& vecInfos);
void createJavaInfoFromJavaHome(std::vector<rtl::Reference<VendorBase> > &vecInfos);
void createJavaInfoDirScan(std::vector<rtl::Reference<VendorBase> >& vecInfos);
#ifdef WNT
void createJavaInfoFromWinReg(std::vector<rtl::Reference<VendorBase> >& vecInfos);
#endif

bool makeDriveLetterSame(OUString * fileURL);


/* for std::find_if
   Used to find a JavaInfo::Impl object in a std::vector<Impl*> which has a member usJavaHome
   as the specified string in the constructor.
*/
struct InfoFindSame
{
    OUString sJava;
    InfoFindSame(const OUString& sJavaHome):sJava(sJavaHome){}

    bool operator () (const rtl::Reference<VendorBase> & aVendorInfo)
    {
        return aVendorInfo->getHome().equals(sJava) == sal_True ? true : false;
    }
};

struct SameOrSubDirJREMap
{
    OUString s1;
    SameOrSubDirJREMap(const OUString& s):s1(s){
    }

    bool operator () (const std::pair<const OUString, rtl::Reference<VendorBase> > & s2)
    {
        if (s1 == s2.first)
            return true;
        OUString sSub;
        sSub = s2.first + OUString("/");
        if (s1.match(sSub) == sal_True)
            return true;
        return false;
    }
};


/* Creates a VendorBase object if a JRE could be found at the specified path.

   This depends if there is a JRE at all and if it is from a vendor that
   is supported by this plugin.
 */
rtl::Reference<VendorBase> getJREInfoByPath(const OUString& path);

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
bool getJREInfoByPath(const OUString& path,
                      std::vector<rtl::Reference<VendorBase> > & vecInfos);

std::vector<rtl::Reference<VendorBase> > getAllJREInfos();

bool getJavaProps(
    const OUString & exePath,
#ifdef JVM_ONE_PATH_CHECK
    const OUString & homePath,
#endif
    std::vector<std::pair<OUString, OUString> >& props,
    bool * bProcessRun);

void  createJavaInfoFromWinReg(std::vector<rtl::Reference<VendorBase> > & vecInfos);

void bubbleSortVersion(std::vector<rtl::Reference<VendorBase> >& vec);

rtl::Bootstrap* getBootstrap();
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
