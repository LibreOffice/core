/*************************************************************************
 *
 *  $RCSfile: util.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-07-23 11:52:41 $
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

#include "util.hxx"

#include "osl/process.h"
#include "osl/security.hxx"
#include "osl/thread.hxx"
#include "osl/file.hxx"
#include "osl/module.hxx"
#include "rtl/byteseq.hxx"
#include "boost/scoped_array.hpp"
#include <utility>
#include <algorithm>
#include <map>
#ifdef WNT
#include <windows.h>
#endif

#include "sunjre.hxx"
#include "vendorlist.hxx"
using namespace rtl;
using namespace osl;
using namespace std;

#ifdef WNT
#define HKEY_SUN_JRE "Software\\JavaSoft\\Java Runtime Environment"
#define HKEY_SUN_SDK "Software\\JavaSoft\\Java Development Kit"
#endif

#ifdef UNX
namespace {
char *g_arJavaNames[] = {
    "",
    "j2re",
    "j2se",
    "j2sdk",
    "jdk",
    "jre",
    "java"
};
/* These are directory names which could contain multiple java installations.
 */
char *g_arCollectDirs[] = {
    "",
    "j2re/",
    "j2se/",
    "j2sdk/",
    "jdk/",
    "jre/",
    "java/"
};

/* These are directories in which a java installation is
   looked for.
*/
char *g_arSearchPaths[] = {
    "",
    "usr/",
    "usr/local/",
    "usr/lib/",
    "usr/bin/"
};
}
#endif //  UNX

namespace jfw_plugin
{
extern VendorSupportMapEntry gVendorMap[];

bool getSDKInfoFromRegistry(vector<OUString> & vecHome);
bool getJREInfoFromRegistry(vector<OUString>& vecJavaHome);

class FileHandleGuard
{
public:
    inline FileHandleGuard(oslFileHandle & rHandle) SAL_THROW(()):
        m_rHandle(rHandle) {}

    inline ~FileHandleGuard() SAL_THROW(());

    inline oslFileHandle & getHandle() SAL_THROW(()) { return m_rHandle; }

private:
    oslFileHandle & m_rHandle;

    FileHandleGuard(FileHandleGuard &); // not implemented
    void operator =(FileHandleGuard); // not implemented
};

inline FileHandleGuard::~FileHandleGuard() SAL_THROW(())
{
    if (m_rHandle != 0)
    {
        oslFileError eError = osl_closeFile(m_rHandle);
        OSL_ENSURE(eError == osl_File_E_None, "unexpected situation");
    }
}

class FileHandleReader
{
public:
    enum Result
    {
        RESULT_OK,
        RESULT_EOF,
        RESULT_ERROR
    };

    inline FileHandleReader(oslFileHandle & rHandle) SAL_THROW(()):
        m_aGuard(rHandle), m_nSize(0), m_nIndex(0), m_bLf(false) {}

    Result readLine(rtl::OString * pLine) SAL_THROW(());

private:
    enum { BUFFER_SIZE = 1024 };

    sal_Char m_aBuffer[BUFFER_SIZE];
    FileHandleGuard m_aGuard;
    int m_nSize;
    int m_nIndex;
    bool m_bLf;
};

FileHandleReader::Result
FileHandleReader::readLine(rtl::OString * pLine)
    SAL_THROW(())
{
    OSL_ENSURE(pLine, "specification violation");

    for (bool bEof = true;; bEof = false)
    {
        if (m_nIndex == m_nSize)
        {
            sal_uInt64 nRead;
            switch (osl_readFile(
                        m_aGuard.getHandle(), m_aBuffer, BUFFER_SIZE, &nRead))
            {
            case osl_File_E_PIPE: //HACK! for windows
                nRead = 0;
            case osl_File_E_None:
                if (nRead == 0)
                {
                    m_bLf = false;
                    return bEof ? RESULT_EOF : RESULT_OK;
                }
                m_nIndex = 0;
                m_nSize = static_cast< int >(nRead);
                break;

            default:
                return RESULT_ERROR;
            }
        }

        if (m_bLf && m_aBuffer[m_nIndex] == 0x0A)
            ++m_nIndex;
        m_bLf = false;

        int nStart = m_nIndex;
        while (m_nIndex != m_nSize)
            switch (m_aBuffer[m_nIndex++])
            {
            case 0x0D:
                m_bLf = true;
            case 0x0A:
                *pLine += rtl::OString(m_aBuffer + nStart,
                                       m_nIndex - 1 - nStart);
                    //TODO! check for overflow, and not very efficient
                return RESULT_OK;
            }

        *pLine += rtl::OString(m_aBuffer + nStart, m_nIndex - nStart);
            //TODO! check for overflow, and not very efficient
    }
}


bool getJavaProps(const OUString & exePath,
                  std::vector<std::pair<rtl::OUString, rtl::OUString> >& props,
                  bool * bProcessRun)
{
    bool ret = false;

    OSL_ASSERT( exePath.getLength() > 0);
    OUString usStartDir;
    //We need to set the CLASSPATH in case the office is started from
    //a different directory. The JREProperties.class is expected to reside
    //next to the plugin.
    rtl::OUString sThisLib;
    if (osl_getModuleURLFromAddress((void *) & getJavaProps,
                                    & sThisLib.pData) == sal_False)
        return false;
    sThisLib = getDirFromFile(sThisLib);
    OUString sClassPath;
    if (osl_getSystemPathFromFileURL(sThisLib.pData, & sClassPath.pData)
        != osl_File_E_None)
        return false;
    //prepare the arguments
    OUString arg1(RTL_CONSTASCII_USTRINGPARAM("-Dfile.encoding=UTF8"));
    OUString arg2 = OUString(RTL_CONSTASCII_USTRINGPARAM("-classpath"));// + sClassPath;
    OUString arg3 = sClassPath;
    OUString arg4(RTL_CONSTASCII_USTRINGPARAM("JREProperties"));
    rtl_uString *args[] = {arg1.pData, arg2.pData, arg3.pData, arg4.pData};

    oslProcess javaProcess= 0;
    oslFileHandle fileOut= 0;
    oslFileHandle fileErr= 0;

    FileHandleReader stdoutReader(fileOut);
    FileHandleReader stderrReader(fileErr);
    oslProcessError procErr =
        osl_executeProcess_WithRedirectedIO( exePath.pData,//usExe.pData,
                                             args,
                                             4,                 //sal_uInt32   nArguments,
                                             osl_Process_HIDDEN, //oslProcessOption Options,
                                             Security().getHandle(), //oslSecurity Security,
                                             usStartDir.pData,//usStartDir.pData,//usWorkDir.pData, //rtl_uString *strWorkDir,
                                             NULL, //rtl_uString *strEnvironment[],
                                             0, //  sal_uInt32   nEnvironmentVars,
                                             &javaProcess, //oslProcess *pProcess,
                                             NULL,//oslFileHandle *pChildInputWrite,
                                             &fileOut,//oslFileHandle *pChildOutputRead,
                                             &fileErr);//oslFileHandle *pChildErrorRead);

    if( procErr != osl_Process_E_None)
    {
        *bProcessRun = false;
        return ret;
    }
    else
    {
        *bProcessRun = true;
    }
#if OSL_DEBUG_LEVEL >=2
     OString aLine;
     FileHandleReader::Result rserr = FileHandleReader::RESULT_OK;
     bool bIntro = false;
     while (1)
     {
         rserr = stderrReader.readLine(&aLine);
         if (rserr != FileHandleReader::RESULT_OK)
             break;
         else
         {
             if (bIntro == false)
             {
                 fprintf(stdout, "#error while executing process: \n");
                 bIntro = true;
             }
         }
         fprintf(stdout,"%s\n", aLine.getStr());
     }
#endif

    FileHandleReader::Result rs = FileHandleReader::RESULT_OK;
    while (1)
    {
        OString aLine;
        rs = stdoutReader.readLine( & aLine);
        if (rs != FileHandleReader::RESULT_OK)
            break;
        OUString sLine = OStringToOUString(aLine, RTL_TEXTENCODING_UTF8);
        sLine = sLine.trim();
        if (sLine.getLength() == 0)
            continue;
        //The JREProperties class writes key value pairs, separated by '='
        sal_Int32 index = sLine.indexOf('=', 0);
        OSL_ASSERT(index != -1);
        OUString sKey = sLine.copy(0, index);
        OUString sValue = sLine.copy(index + 1);

        props.push_back(std::make_pair(sKey, sValue));
    }

    if (rs != FileHandleReader::RESULT_ERROR && props.size()>0)
        ret = true;

   TimeValue waitMax= {5 ,0};
   procErr = osl_joinProcessWithTimeout(javaProcess, &waitMax);
   OSL_ASSERT(procErr == osl_Process_E_None);

   return ret;
}


#if defined WNT
void createJavaInfoFromWinReg(std::vector<rtl::Reference<VendorBase> > & vecInfos)
{
        // Get Java s from registry
    std::vector<OUString> vecJavaHome;
    if(getSDKInfoFromRegistry(vecJavaHome))
    {
        // create impl objects
        typedef std::vector<OUString>::iterator ItHome;
        for(ItHome it_home= vecJavaHome.begin(); it_home != vecJavaHome.end();
            it_home++)
        {
            getJREInfoByPath(*it_home, vecInfos);
        }
    }

    vecJavaHome.clear();
    if(getJREInfoFromRegistry(vecJavaHome))
    {
        typedef std::vector<OUString>::iterator ItHome;
        for(ItHome it_home= vecJavaHome.begin(); it_home != vecJavaHome.end();
            it_home++)
        {
            getJREInfoByPath(*it_home, vecInfos);
        }
   }
}


bool getJavaInfoFromRegistry(const bool bSdk, const char* szRegKey,
                             vector<OUString>& vecJavaHome)
{
    HKEY    hRoot;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, szRegKey, 0, KEY_ENUMERATE_SUB_KEYS, &hRoot)
        == ERROR_SUCCESS)
    {
        DWORD dwIndex = 0;
        DWORD nNameLen;
        char bufVersion[1024];
        FILETIME fileTime;
        nNameLen = sizeof(bufVersion);

        // Iterate over all subkeys of HKEY_LOCAL_MACHINE\Software\JavaSoft\Java Runtime Environment
        while (RegEnumKeyEx(hRoot, dwIndex, bufVersion, &nNameLen, NULL, NULL, NULL, &fileTime) != ERROR_NO_MORE_ITEMS)
        {
            HKEY    hKey;
            // Open a Java Runtime Environment sub key, e.g. "1.4.0"
            if (RegOpenKeyEx(hRoot, bufVersion, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
            {
                DWORD   dwType;
                DWORD   dwTmpPathLen= 0;
                // Get the path to the JavaHome every JRE entry
                // Find out how long the string for JavaHome is and allocate memory to hold the path
                if( RegQueryValueExA(hKey, "JavaHome", 0, &dwType, NULL, &dwTmpPathLen)== ERROR_SUCCESS)
                {
                    char* szTmpPath= (char *) malloc( dwTmpPathLen);
                    // Get the path for the runtime lib
                    if(RegQueryValueExA(hKey, "JavaHome", 0, &dwType, (unsigned char*) szTmpPath, &dwTmpPathLen) == ERROR_SUCCESS)
                    {
                        // There can be several version entries refering with the same JavaHome,e.g 1.4 and 1.4.1
                        OUString usHome= OUString::createFromAscii(szTmpPath);
                        // check if there is already an entry with the same JavaHomeruntime lib
                        // if so, we use the one with the more accurate version
                        bool bAppend= true;
                        OUString usHomeUrl;
                        if (osl_getFileURLFromSystemPath(usHome.pData, & usHomeUrl.pData) ==
                            osl_File_E_None)
                        {
                            //iterate over the vector with java home strings
                            typedef vector<OUString>::iterator ItHome;
                            for(ItHome itHome= vecJavaHome.begin();
                                itHome != vecJavaHome.end(); itHome++)
                            {
                                if(usHomeUrl.equals(*itHome))
                                {
                                    bAppend= false;
                                    break;
                                }
                            }
                            // Save the home dir
                            if(bAppend)
                            {
                                vecJavaHome.push_back(usHomeUrl);
                            }
                        }
                    }
                    free( szTmpPath);
                    RegCloseKey(hKey);
                }
            }
            dwIndex ++;
            nNameLen = sizeof(bufVersion);
        }
        RegCloseKey(hRoot);
    }
    return true;
}



bool getSDKInfoFromRegistry(vector<OUString> & vecHome)
{
    return getJavaInfoFromRegistry(true, HKEY_SUN_SDK, vecHome);
}

bool getJREInfoFromRegistry(vector<OUString>& vecJavaHome)
{
    return getJavaInfoFromRegistry(false, HKEY_SUN_JRE, vecJavaHome);
}

#endif // WNT

void bubbleSortVersion(vector<rtl::Reference<VendorBase> >& vec)
{
    if(vec.size() == 0)
        return;
    int size= vec.size() - 1;
    int cIter= 0;
    // sort for version
    for(int i= 0; i < size; i++)
    {
        for(int j= size; j > 0 + cIter; j--)
        {
            rtl::Reference<VendorBase>& cur= vec.at(j);
            rtl::Reference<VendorBase>& next= vec.at(j-1);

                // comparing invalid SunVersion s is possible, they will be less than a
                // valid version
            int nCmp = cur->compareVersions(next->getVersion());
            if(nCmp == 1) // cur > next
            {
                rtl::Reference<VendorBase> less = next;
                vec.at(j-1)= cur;
                vec.at(j)= less;
            }
        }
        cIter++;
    }
}


bool getJREInfoFromBinPath(
    const rtl::OUString& path, vector<rtl::Reference<VendorBase> > & vecInfos)
{
    // file:///c:/jre/bin
    //map:       jre/bin/java.exe
    bool ret = false;
    vector<pair<OUString, OUString> > props;

    for ( sal_Int32 pos = 0;
          gVendorMap[pos].sVendorName != NULL; ++pos )
    {
        vector<OUString> vecPaths;
        getJavaExePaths_func pFunc = gVendorMap[pos].getJavaFunc;

        int size = 0;
        char const* const* arExePaths = (*pFunc)(&size);
        vecPaths = getVectorFromCharArray(arExePaths, size);

        //make sure argument path does not end with '/'
        OUString sBinPath = path;
        if (path.lastIndexOf('/') == (path.getLength() - 1))
            sBinPath = path.copy(0, path.getLength() - 1);

        typedef vector<OUString>::const_iterator c_it;
        for (c_it i = vecPaths.begin(); i != vecPaths.end(); i++)
        {
            //the map contains e.g. jre/bin/java.exe
            //get the directory where the executable is contained
            OUString sHome;
            sal_Int32 index = i->lastIndexOf('/');
            if (index == -1)
            {
                //map contained only : "java.exe, then the argument
                //path is already the home directory
                sHome = sBinPath;
            }
            else
            {
                // jre/bin/jre -> jre/bin
                OUString sMapPath(i->getStr(), index);
                index = sBinPath.lastIndexOf(sMapPath);
                if (index != -1
                    && (index + sMapPath.getLength() == sBinPath.getLength())
                    && sBinPath[index - 1] == '/')
                {
                    sHome = OUString(sBinPath.getStr(), index - 1);
                }
            }
            if (sHome.getLength() > 0)
            {
                ret = getJREInfoByPath(sHome, vecInfos);
                if (ret)
                    break;
            }
        }
        if (ret)
            break;
    }
    return ret;
}

vector<Reference<VendorBase> > getAllJREInfos()
{
    vector<Reference<VendorBase> > vecInfos;

#if defined WNT
    // Get Javas from the registry
    createJavaInfoFromWinReg(vecInfos);
#endif // WNT

    createJavaInfoFromJavaHome(vecInfos);
    //this function should be called after createJavaInfoDirScan.
    //Otherwise in SDKs Java may be started twice
     createJavaInfoFromPath(vecInfos);

#ifdef UNX
    createJavaInfoDirScan(vecInfos);
#endif
    bubbleSortVersion(vecInfos);
    return vecInfos;
}


std::vector<rtl::Reference<VendorBase> > getAllJREInfos(
    const rtl::OUString& sVendor,
    const rtl::OUString& sMinVersion,
    const rtl::OUString& sMaxVersion,
    const std::vector<rtl::OUString> & vecExcludeVersions)
{
    vector<rtl::Reference<VendorBase> > ret;
    vector<rtl::Reference<VendorBase> > vecInfos =
        getAllJREInfos();

    bool bVendor = sVendor.getLength() > 0 ? true : false;
    bool bMinVersion = sMinVersion.getLength() > 0 ? true : false;
    bool bMaxVersion = sMaxVersion.getLength() > 0 ? true : false;
    bool bExcludeList = vecExcludeVersions.size() > 0 ? true : false;
    typedef vector<rtl::Reference<VendorBase> >::iterator it;
    for (it i= vecInfos.begin(); i != vecInfos.end(); i++)
    {

        rtl::Reference<VendorBase>& cur = *i;

        if (bVendor)
        {
            if (sVendor.equals(cur->getVendor()) == sal_False)
                continue;
        }
        if (bMinVersion)
        {
            if (cur->compareVersions(sMinVersion) == -1)
                continue;
        }

        if (bMaxVersion)
        {
            if (cur->compareVersions(sMaxVersion) == 1)
                continue;
        }

        if (bExcludeList)
        {
            bool bExclude = false;
            typedef vector<OUString>::const_iterator it_s;
            for (it_s ii = vecExcludeVersions.begin();
                 ii != vecExcludeVersions.end(); ii++)
            {
                if (cur->compareVersions(*ii) == 0)
                {
                    bExclude = true;
                    break;
                }
            }
            if (bExclude == true)
                continue;
        }
        ret.push_back(*i);
    }

     return ret;
}

vector<OUString> getVectorFromCharArray(char const * const * ar, int size)
{
    vector<OUString> vec;
    for( int i = 0; i < size; i++)
    {
        OUString s(ar[i], strlen(ar[i]), RTL_TEXTENCODING_UTF8);
        vec.push_back(s);
    }
    return vec;
}
bool getJREInfoByPath(const rtl::OUString& path,
                      std::vector<rtl::Reference<VendorBase> > & vecInfos)
{
    bool ret = false;

    rtl::Reference<VendorBase> aInfo = getJREInfoByPath(path);
    if (aInfo.is())
    {
        ret = true;
        vector<rtl::Reference<VendorBase> >::const_iterator it_impl= std::find_if(
            vecInfos.begin(),vecInfos.end(), InfoFindSame(aInfo->getHome()));
        if(it_impl == vecInfos.end())
        {
            vecInfos.push_back(aInfo);
        }
    }
    return ret;
}

/** Checks if the path is a directory. If it is a link to a directory than
    it is resolved.
    In case of an error the returned string has the length 0
 */
OUString resolveDirPath(const OUString & path)
{
    OUString sResolved = path;
    OUString ret;
    while (1)
    {
        DirectoryItem item;
        if (DirectoryItem::get(sResolved, item) == File::E_None)
        {
            FileStatus status(FileStatusMask_Type |
                              FileStatusMask_LinkTargetURL |
                              FileStatusMask_FileURL);

            if (item.getFileStatus(status) == File::E_None)
            {
                FileStatus::Type t = status.getFileType();
                if (t == FileStatus::Directory)
                {
                    ret = sResolved;
                    break;
                }
                else if ( t == FileStatus::Link )
                {
                    sResolved = status.getLinkTargetURL();
                }
                else
                {
                    break;
                }
            }
            else
                break;
            }
        else
            break;
    }
    return ret;
}
/** Checks if the path is a file. If it is a link to a file than
    it is resolved.
    In case of an error the returned string has the length 0
 */
OUString resolveFilePath(const OUString & path)
{
    OUString sResolved = path;
    OUString ret;
    while (1)
    {
        DirectoryItem item;
        if (DirectoryItem::get(sResolved, item) == File::E_None)
        {
            FileStatus status(FileStatusMask_Type |
                              FileStatusMask_LinkTargetURL |
                              FileStatusMask_FileURL);

            if (item.getFileStatus(status) == File::E_None)
            {
                FileStatus::Type t = status.getFileType();
                if (t == FileStatus::Regular)
                {
                    ret = sResolved;
                    break;
                }
                else if ( t == FileStatus::Link )
                {
                    sResolved = status.getLinkTargetURL();
                }
                else
                {
                    break;
                }
            }
            else
                break;
            }
        else
            break;
    }
    return ret;
}

rtl::Reference<VendorBase> getJREInfoByPath(
    const OUString& path)
{
    rtl::Reference<VendorBase> ret;
    static vector<OUString> vecBadPaths;

    static map<OUString, rtl::Reference<VendorBase> > mapJREs;
    typedef map<OUString, rtl::Reference<VendorBase> >::const_iterator MapIt;
    typedef map<OUString, rtl::Reference<VendorBase> > MAPJRE;
    OUString sFilePath;
    typedef vector<OUString>::const_iterator cit_path;
    vector<pair<OUString, OUString> > props;

    OUString sResolvedDir = resolveDirPath(path);
    // If this path is invalid then there is no chance to find a JRE here
    if (sResolvedDir.getLength() == 0)
        return 0;

    //check if the directory path is good, that is a JRE was already recognized.
    //Then we need not detect it again
    MapIt entry2 = find_if(mapJREs.begin(), mapJREs.end(),
                           SameOrSubDirJREMap(sResolvedDir));
    if (entry2 != mapJREs.end())
    {
#if OSL_DEBUG_LEVEL >= 2
        OString _s = OUStringToOString(sResolvedDir, osl_getThreadTextEncoding());
        fprintf(stdout,"###JRE found again (detected before): %s\n", _s.getStr());
#endif
        return entry2->second;
    }

    for ( sal_Int32 pos = 0;
          gVendorMap[pos].sVendorName != NULL; ++pos )
    {
        vector<OUString> vecPaths;
        getJavaExePaths_func pFunc = gVendorMap[pos].getJavaFunc;

        int size = 0;
        char const* const* arExePaths = (*pFunc)(&size);
        vecPaths = getVectorFromCharArray(arExePaths, size);

        bool bOk = false;
        typedef vector<OUString>::const_iterator c_it;
        for (c_it i = vecPaths.begin(); i != vecPaths.end(); i++)
        {
            //if the path is a link, then resolve it
            //check if the executable exists at all
            bool bExe = false;
            bool bError = false;



            //path can be only "file:///". Then do not append a '/'
            //sizeof counts the terminating 0
            OUString sFullPath;
            if (path.getLength() == sizeof("file:///") - 1)
                sFullPath = sResolvedDir + (*i);
            else
                sFullPath = sResolvedDir +
                OUString(RTL_CONSTASCII_USTRINGPARAM("/")) + (*i);

            sFilePath = resolveFilePath(sFullPath);

            if (sFilePath.getLength() == 0)
            {
                //The file path (to java exe) is not valid
                cit_path ifull = find(vecBadPaths.begin(), vecBadPaths.end(), sFullPath);
                if (ifull == vecBadPaths.end())
                    vecBadPaths.push_back(sFullPath);
                continue;
            }

            cit_path ifile = find(vecBadPaths.begin(), vecBadPaths.end(), sFilePath);
            if (ifile != vecBadPaths.end())
                continue;

            MapIt entry =  mapJREs.find(sFilePath);
            if (entry != mapJREs.end())
            {
#if OSL_DEBUG_LEVEL >= 2
                OString _s = OUStringToOString(sFilePath, osl_getThreadTextEncoding());
                fprintf(stdout,"###JRE found again (detected before): %s\n", _s.getStr());
#endif

                return entry->second;
            }

            bool bProcessRun= false;
            if (getJavaProps(sFilePath, props, & bProcessRun) == false)
            {
                vecBadPaths.push_back(sFilePath);
                //if there was a java executable, that is the process was started
                //then we can assume that it is not necessary to search for another
                //executable under the same root folder.
                if (bProcessRun == true)
                {
                    bOk = true;
                    break;
                }
                else
                    continue;
            }
            else
            {
                bOk = true;
                break;
            }
        }
        if (bOk)
            break;
    }

    if (props.size() == 0)
        return rtl::Reference<VendorBase>();

    //find java.vendor property
    typedef vector<pair<OUString, OUString> >::const_iterator c_ip;
    OUString sVendor(RTL_CONSTASCII_USTRINGPARAM("java.vendor"));
    OUString sVendorName;

    for (c_ip i = props.begin(); i != props.end(); i++)
    {
        if (sVendor.equals(i->first))
        {
            sVendorName = i->second;
            break;
        }
    }

    if (sVendorName.getLength() > 0)
    {
        //find the creator func for the respective vendor name
        for ( sal_Int32 c = 0;
              gVendorMap[c].sVendorName != NULL; ++c )
        {
            OUString sNameMap(gVendorMap[c].sVendorName, strlen(gVendorMap[c].sVendorName),
                              RTL_TEXTENCODING_ASCII_US);
            if (sNameMap.equals(sVendorName))
            {
                ret = createInstance(gVendorMap[c].createFunc, props);
                break;
            }
        }
    }
    if (ret.is() == false)
        vecBadPaths.push_back(sResolvedDir);
    else
    {
#if OSL_DEBUG_LEVEL >= 2
        OString _s = OUStringToOString(sResolvedDir, osl_getThreadTextEncoding());
        OString _s2 = OUStringToOString(path, osl_getThreadTextEncoding());
        fprintf(stdout,"###Detected another JRE: %s\n at: %s\n" ,
                _s.getStr(), _s2.getStr());
#endif
        mapJREs.insert(MAPJRE::value_type(sResolvedDir, ret));
        mapJREs.insert(MAPJRE::value_type(sFilePath, ret));
    }

    return ret;
}

Reference<VendorBase> createInstance(createInstance_func pFunc,
                                     vector<pair<OUString, OUString> > properties)
{

    Reference<VendorBase> aBase = (*pFunc)();
    if (aBase.is())
    {
        if (aBase->initialize(properties) == false)
            aBase = 0;
    }
    return aBase;
}

inline OUString getDirFromFile(const OUString& usFilePath)
{
    sal_Int32 index= usFilePath.lastIndexOf('/');
    return OUString(usFilePath.getStr(), index);
}

void createJavaInfoFromPath(vector<rtl::Reference<VendorBase> >& vecInfos)
{
// Get Java from PATH environment variable
    static OUString sCurDir(RTL_CONSTASCII_USTRINGPARAM("."));
    static OUString sParentDir(RTL_CONSTASCII_USTRINGPARAM(".."));
    char *szPath= getenv("PATH");
    if(szPath)
    {
        OUString usAllPath(szPath, strlen(szPath), osl_getThreadTextEncoding());
        sal_Int32 nIndex = 0;
        do
        {
            OUString usToken = usAllPath.getToken( 0, SAL_PATHSEPARATOR, nIndex );
            OUString usTokenUrl;
            if(File::getFileURLFromSystemPath(usToken, usTokenUrl) == File::E_None)
            {
                if(usTokenUrl.getLength())
                {
                    OUString usBin;
                    // "."
                    if(usTokenUrl.equals(sCurDir))
                    {
                        OUString usWorkDirUrl;
                        if(osl_Process_E_None == osl_getProcessWorkingDir(&usWorkDirUrl.pData))
                            usBin= usWorkDirUrl;
                    }
                    // ".."
                    else if(usTokenUrl.equals(sParentDir))
                    {
                        OUString usWorkDir;
                        if(osl_Process_E_None == osl_getProcessWorkingDir(&usWorkDir.pData))
                            usBin= getDirFromFile(usWorkDir);
                    }
                    else
                    {
                        usBin = usTokenUrl;
                    }
                    if(usBin.getLength())
                    {
                        getJREInfoFromBinPath(usBin, vecInfos);
                    }
                }
            }
        }
        while ( nIndex >= 0 );
    }
}

void createJavaInfoFromJavaHome(vector<rtl::Reference<VendorBase> >& vecInfos)
{
    // Get Java from JAVA_HOME environment
    char *szJavaHome= getenv("JAVA_HOME");
    if(szJavaHome)
    {
        OUString sHome(szJavaHome,strlen(szJavaHome),osl_getThreadTextEncoding());
        OUString sHomeUrl;
        if(File::getFileURLFromSystemPath(sHome, sHomeUrl) == File::E_None)
        {
            getJREInfoByPath(sHomeUrl, vecInfos);
        }
    }
}

bool makeDriveLetterSame(OUString * fileURL)
{
    bool ret = false;
    DirectoryItem item;
    if (DirectoryItem::get(*fileURL, item) == File::E_None)
    {
        FileStatus status(FileStatusMask_FileURL);
        if (item.getFileStatus(status) == File::E_None)
        {
            *fileURL = status.getFileURL();
            ret = true;
        }
    }
    return ret;
}

#ifdef UNX
void createJavaInfoDirScan(vector<rtl::Reference<VendorBase> >& vecInfos)
{
    int cJavaNames= sizeof(g_arJavaNames) / sizeof(char*);
    boost::scoped_array<OUString> sarJavaNames(new OUString[cJavaNames]);
    OUString *arNames = sarJavaNames.get();
    for(int i= 0; i < cJavaNames; i++)
        arNames[i] = OUString(g_arJavaNames[i], strlen(g_arJavaNames[i]),
                              RTL_TEXTENCODING_UTF8);

    int cSearchPaths= sizeof(g_arSearchPaths) / sizeof(char*);
    boost::scoped_array<OUString> sarPathNames(new OUString[cSearchPaths]);
    OUString *arPaths = sarPathNames.get();
    for(int c = 0; c < cSearchPaths; c++)
        arPaths[c] = OUString(g_arSearchPaths[c], strlen(g_arSearchPaths[c]),
                               RTL_TEXTENCODING_UTF8);

    int cCollectDirs = sizeof(g_arCollectDirs) / sizeof(char*);
    boost::scoped_array<OUString> sarCollectDirs(new OUString[cCollectDirs]);
    OUString *arCollectDirs = sarCollectDirs.get();
    for(int d = 0; d < cCollectDirs; d++)
        arCollectDirs[d] = OUString(g_arCollectDirs[d], strlen(g_arCollectDirs[d]),
                               RTL_TEXTENCODING_UTF8);



    OUString usFile(RTL_CONSTASCII_USTRINGPARAM("file:///"));
    for( int ii = 0; ii < cSearchPaths; ii ++)
    {
        OUString usDir1(usFile + arPaths[ii]);
        DirectoryItem item;
        if(DirectoryItem::get(usDir1, item) == File::E_None)
        {
            for(int j= 0; j < cJavaNames; j++)
            {
                // /usr/java/
                OUString usDir2(usDir1 + arCollectDirs[j]);
                DirectoryItem item2;
                if(DirectoryItem::get(usDir2, item2) == File::E_None)
                {
                    for( int k= 0; k < cJavaNames; k++)
                    {
                        // /usr/java/j2re1.4.0
                        OUString usDir3(usDir2 + arNames[k]);

//                         OString _s = OUStringToOString(usDir3, osl_getThreadTextEncoding());
//                         fprintf(stdout,"###directory: %s\n", _s.getStr());

                        DirectoryItem item3;
                        if(DirectoryItem::get(usDir3, item) == File::E_None)
                        {
                            //remove trailing '/'
                            sal_Int32 islash = usDir3.lastIndexOf('/');
                            if (islash == usDir3.getLength() - 1
                                && islash > sizeof("file:///") - 2)
                                usDir3 = usDir3.copy(0, islash);
                            getJREInfoByPath(usDir3,vecInfos);
                        }
                    }
                }
            }
        }
    }
}
#endif
}
