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

#include <config_features.h>
#include <config_folders.h>

#include "util.hxx"

#include "osl/process.h"
#include "osl/security.hxx"
#include "osl/file.hxx"
#include "osl/module.hxx"
#include "rtl/byteseq.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/instance.hxx"
#include "salhelper/linkhelper.hxx"
#include "salhelper/thread.hxx"
#include "boost/scoped_array.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include <utility>
#include <algorithm>
#include <map>

#if defined WNT
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#endif
#include <string.h>

#include "sunjre.hxx"
#include "vendorlist.hxx"
#include "diagnostics.h"

using namespace osl;
using namespace std;

using ::rtl::Reference;

#ifdef WNT
#define HKEY_SUN_JRE L"Software\\JavaSoft\\Java Runtime Environment"
#define HKEY_SUN_SDK L"Software\\JavaSoft\\Java Development Kit"
#endif

#ifdef UNX
#if !(defined MACOSX && defined X86_64)
namespace {
char const *g_arJavaNames[] = {
    "",
    "j2re",
    "j2se",
    "j2sdk",
    "jdk",
    "jre",
    "java",
    "Home",
    "IBMJava2-ppc-142"
};
/* These are directory names which could contain multiple java installations.
 */
char const *g_arCollectDirs[] = {
    "",
#ifndef JVM_ONE_PATH_CHECK
    "j2re/",
    "j2se/",
    "j2sdk/",
    "jdk/",
    "jre/",
    "java/",
#endif
    "jvm/"
};

/* These are directories in which a java installation is
   looked for.
*/
char const *g_arSearchPaths[] = {
#ifdef MACOSX
    "",
    "Library/Internet Plug-Ins/JavaAppletPlugin.plugin/Contents/Home/bin",
    "System/Library/Frameworks/JavaVM.framework/Versions/1.4.2/"
#else
#ifndef JVM_ONE_PATH_CHECK
    "",
    "usr/",
    "usr/local/",
    "usr/local/IBMJava2-ppc-142",
    "usr/local/j2sdk1.3.1",
#ifdef X86_64
    "usr/lib64/",
#endif
    "usr/lib/",
    "usr/bin/"
#else
    JVM_ONE_PATH_CHECK
#endif
#endif
};
}
#endif
#endif //  UNX

namespace jfw_plugin
{
extern VendorSupportMapEntry gVendorMap[];

bool getSDKInfoFromRegistry(vector<OUString> & vecHome);
bool getJREInfoFromRegistry(vector<OUString>& vecJavaHome);
bool decodeOutput(const OString& s, OUString* out);



namespace
{
    OUString getLibraryLocation()
    {
        OUString libraryFileUrl;
        OSL_VERIFY(osl::Module::getUrlFromAddress((void *)(sal_IntPtr)getLibraryLocation, libraryFileUrl));
        return getDirFromFile(libraryFileUrl);
    }

    struct InitBootstrap
    {
        rtl::Bootstrap * operator()(const OUString& sIni)
        {
            static rtl::Bootstrap aInstance(sIni);
            return & aInstance;

        }
   };

   struct InitBootstrapData
   {
       OUString const & operator()()
       {
           static OUString sIni;
            OUStringBuffer buf( 255);
            buf.append( getLibraryLocation());
#if HAVE_FEATURE_MACOSX_MACLIKE_APP_STRUCTURE
            buf.appendAscii( "/../" LIBO_ETC_FOLDER );
#endif
            buf.appendAscii( SAL_CONFIGFILE("/sunjavaplugin") );
            sIni = buf.makeStringAndClear();
            JFW_TRACE2("[Java framework] sunjavaplugin: "
                       "Using configuration file \n" +  sIni);
            return sIni;
        }
   };
}

rtl::Bootstrap * getBootstrap()
{
    return rtl_Instance< rtl::Bootstrap, InitBootstrap,
        ::osl::MutexGuard, ::osl::GetGlobalMutex,
        OUString, InitBootstrapData >::create(
            InitBootstrap(), ::osl::GetGlobalMutex(), InitBootstrapData());
}




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
        if (osl_closeFile(m_rHandle) != osl_File_E_None)
        {
            OSL_FAIL("unexpected situation");
        }
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

    Result readLine(OString * pLine) SAL_THROW(());

private:
    enum { BUFFER_SIZE = 1024 };

    sal_Char m_aBuffer[BUFFER_SIZE];
    FileHandleGuard m_aGuard;
    int m_nSize;
    int m_nIndex;
    bool m_bLf;
};

FileHandleReader::Result
FileHandleReader::readLine(OString * pLine)
    SAL_THROW(())
{
    OSL_ENSURE(pLine, "specification violation");

    for (bool bEof = true;; bEof = false)
    {
        if (m_nIndex == m_nSize)
        {
            sal_uInt64 nRead = 0;
            switch (osl_readFile(
                        m_aGuard.getHandle(), m_aBuffer, sizeof(m_aBuffer), &nRead))
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
            case osl_File_E_INTR:
                continue;

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
                *pLine += OString(m_aBuffer + nStart,
                                       m_nIndex - 1 - nStart);
                    //TODO! check for overflow, and not very efficient
                return RESULT_OK;
            }

        *pLine += OString(m_aBuffer + nStart, m_nIndex - nStart);
            //TODO! check for overflow, and not very efficient
    }
}

class AsynchReader: public salhelper::Thread
{
    size_t  m_nDataSize;
    boost::scoped_array<sal_Char> m_arData;

    bool m_bError;
    bool m_bDone;
    FileHandleGuard m_aGuard;

    virtual ~AsynchReader() {}

    void execute();
public:

    AsynchReader(oslFileHandle & rHandle);

    /** only call this function after this thread has finished.

        That is, call join on this instance and then call getData.

     */
    OString getData();
};

AsynchReader::AsynchReader(oslFileHandle & rHandle):
    Thread("jvmfwkAsyncReader"), m_nDataSize(0), m_bError(false),
    m_bDone(false), m_aGuard(rHandle)
{
}

OString AsynchReader::getData()
{
    return OString(m_arData.get(), m_nDataSize);
}

void AsynchReader::execute()
{
    const sal_uInt64 BUFFER_SIZE = 4096;
    sal_Char aBuffer[BUFFER_SIZE];
    while (true)
    {
        sal_uInt64 nRead;
        //the function blocks until something could be read or the pipe closed.
        switch (osl_readFile(
                    m_aGuard.getHandle(), aBuffer, BUFFER_SIZE, &nRead))
        {
        case osl_File_E_PIPE: //HACK! for windows
            nRead = 0;
        case osl_File_E_None:
            break;
        default:
            m_bError = true;
            return;
        }

        if (nRead == 0)
        {
            m_bDone = true;
            break;
        }
        else if (nRead <= BUFFER_SIZE)
        {
            //Save the data we have in m_arData into a temporary array
            boost::scoped_array<sal_Char> arTmp( new sal_Char[m_nDataSize]);
            memcpy(arTmp.get(), m_arData.get(), m_nDataSize);
            //Enlarge m_arData to hold the newly read data
            m_arData.reset(new sal_Char[(size_t)(m_nDataSize + nRead)]);
            //Copy back the data that was already in m_arData
            memcpy(m_arData.get(), arTmp.get(), m_nDataSize);
            //Add the newly read data to m_arData
            memcpy(m_arData.get() + m_nDataSize, aBuffer, (size_t) nRead);
            m_nDataSize += (size_t) nRead;
        }
    }
}


bool getJavaProps(const OUString & exePath,
#ifdef JVM_ONE_PATH_CHECK
                  const OUString & homePath,
#endif
                  std::vector<std::pair<OUString, OUString> >& props,
                  bool * bProcessRun)
{
    bool ret = false;

    OSL_ASSERT(!exePath.isEmpty());
    OUString usStartDir;
    //We need to set the CLASSPATH in case the office is started from
    //a different directory. The JREProperties.class is expected to reside
    //next to the plugin.
    OUString sThisLib;
    if (osl_getModuleURLFromAddress((void *) (sal_IntPtr)& getJavaProps,
                                    & sThisLib.pData) == sal_False)
        return false;
    sThisLib = getDirFromFile(sThisLib);
    OUString sClassPath;
    if (osl_getSystemPathFromFileURL(sThisLib.pData, & sClassPath.pData)
        != osl_File_E_None)
        return false;

    //check if we shall examine a Java for accessibility support
    //If the bootstrap variable is "1" then we pass the argument
    //"noaccessibility" to JREProperties.class. This will prevent
    //that it calls   java.awt.Toolkit.getDefaultToolkit();
    OUString sValue;
    getBootstrap()->getFrom("JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY", sValue);

    //prepare the arguments
    sal_Int32 cArgs = 3;
    OUString arg1 = OUString("-classpath");// + sClassPath;
    OUString arg2 = sClassPath;
    OUString arg3("JREProperties");
    OUString arg4 = "noaccessibility";
    rtl_uString *args[4] = {arg1.pData, arg2.pData, arg3.pData};

    // Only add the fourth param if the bootstrap parameter is set.
    if (sValue.equals(OUString::number( 1)))
    {
        args[3] = arg4.pData;
        cArgs = 4;
    }

    oslProcess javaProcess= 0;
    oslFileHandle fileOut= 0;
    oslFileHandle fileErr= 0;

    FileHandleReader stdoutReader(fileOut);
    rtl::Reference< AsynchReader > stderrReader(new AsynchReader(fileErr));

    JFW_TRACE2("\n[Java framework] Executing: " + exePath + ".\n");
    oslProcessError procErr =
        osl_executeProcess_WithRedirectedIO( exePath.pData,//usExe.pData,
                                             args,
                                             cArgs,                 //sal_uInt32   nArguments,
                                             osl_Process_HIDDEN, //oslProcessOption Options,
                                             NULL, //oslSecurity Security,
                                             usStartDir.pData,//usStartDir.pData,//usWorkDir.pData, //rtl_uString *strWorkDir,
                                             NULL, //rtl_uString *strEnvironment[],
                                             0, //  sal_uInt32   nEnvironmentVars,
                                             &javaProcess, //oslProcess *pProcess,
                                             NULL,//oslFileHandle *pChildInputWrite,
                                             &fileOut,//oslFileHandle *pChildOutputRead,
                                             &fileErr);//oslFileHandle *pChildErrorRead);

    if( procErr != osl_Process_E_None)
    {
        JFW_TRACE2("[Java framework] Execution failed. \n");
        *bProcessRun = false;
        return ret;
    }
    else
    {
        JFW_TRACE2("[Java framework] Java executed successfully.\n");
        *bProcessRun = true;
    }

    //Start asynchronous reading (different thread) of error stream
    stderrReader->launch();

    //Use this thread to read output stream
    FileHandleReader::Result rs = FileHandleReader::RESULT_OK;
    while (1)
    {
        OString aLine;
        rs = stdoutReader.readLine( & aLine);
        if (rs != FileHandleReader::RESULT_OK)
            break;
        OUString sLine;
        if (!decodeOutput(aLine, &sLine))
            continue;
        JFW_TRACE2("[Java framework]:\" " << sLine << " \".\n");
        sLine = sLine.trim();
        if (sLine.isEmpty())
            continue;
        //The JREProperties class writes key value pairs, separated by '='
        sal_Int32 index = sLine.indexOf('=', 0);
        OSL_ASSERT(index != -1);
        OUString sKey = sLine.copy(0, index);
        OUString sVal = sLine.copy(index + 1);

#ifdef JVM_ONE_PATH_CHECK
        //replace absolute path by linux distro link
        OUString sHomeProperty("java.home");
        if(sHomeProperty.equals(sKey))
        {
            sVal = homePath + "/jre";
        }
#endif

        props.push_back(std::make_pair(sKey, sVal));
    }

    if (rs != FileHandleReader::RESULT_ERROR && !props.empty())
        ret = true;

    //process error stream data
    stderrReader->join();
    JFW_TRACE2("[Java framework]  Java wrote to stderr:\" "
               << stderrReader->getData().getStr() << " \".\n");

    TimeValue waitMax= {5 ,0};
    procErr = osl_joinProcessWithTimeout(javaProcess, &waitMax);
    OSL_ASSERT(procErr == osl_Process_E_None);
    osl_freeProcessHandle(javaProcess);
    return ret;
}

/* converts the properties printed by JREProperties.class into
    readable strings. The strings are encoded as integer values separated
    by spaces.
 */
bool decodeOutput(const OString& s, OUString* out)
{
    OSL_ASSERT(out != 0);
    OUStringBuffer buff(512);
    sal_Int32 nIndex = 0;
    do
    {
        OString aToken = s.getToken( 0, ' ', nIndex );
        if (!aToken.isEmpty())
        {
            for (sal_Int32 i = 0; i < aToken.getLength(); ++i)
            {
                if (aToken[i] < '0' || aToken[i] > '9')
                    return false;
            }
            sal_Unicode value = (sal_Unicode)(aToken.toInt32());
            buff.append(value);
        }
    } while (nIndex >= 0);

    *out = buff.makeStringAndClear();
    return true;
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


bool getJavaInfoFromRegistry(const wchar_t* szRegKey,
                             vector<OUString>& vecJavaHome)
{
    HKEY    hRoot;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, szRegKey, 0, KEY_ENUMERATE_SUB_KEYS, &hRoot)
        == ERROR_SUCCESS)
    {
        DWORD dwIndex = 0;
        const DWORD BUFFSIZE = 1024;
        wchar_t bufVersion[BUFFSIZE];
        DWORD nNameLen = BUFFSIZE;
        FILETIME fileTime;
        nNameLen = sizeof(bufVersion);

        // Iterate over all subkeys of HKEY_LOCAL_MACHINE\Software\JavaSoft\Java Runtime Environment
        while (RegEnumKeyExW(hRoot, dwIndex, bufVersion, &nNameLen, NULL, NULL, NULL, &fileTime) != ERROR_NO_MORE_ITEMS)
        {
            HKEY    hKey;
            // Open a Java Runtime Environment sub key, e.g. "1.4.0"
            if (RegOpenKeyExW(hRoot, bufVersion, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS)
            {
                DWORD   dwType;
                DWORD   dwTmpPathLen= 0;
                // Get the path to the JavaHome every JRE entry
                // Find out how long the string for JavaHome is and allocate memory to hold the path
                if( RegQueryValueExW(hKey, L"JavaHome", 0, &dwType, NULL, &dwTmpPathLen)== ERROR_SUCCESS)
                {
                    char* szTmpPath= (char *) malloc( dwTmpPathLen);
                    // Get the path for the runtime lib
                    if(RegQueryValueExW(hKey, L"JavaHome", 0, &dwType, (unsigned char*) szTmpPath, &dwTmpPathLen) == ERROR_SUCCESS)
                    {
                        // There can be several version entries refering with the same JavaHome,e.g 1.4 and 1.4.1
                        OUString usHome((sal_Unicode*) szTmpPath);
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
            nNameLen = BUFFSIZE;
        }
        RegCloseKey(hRoot);
    }
    return true;
}



bool getSDKInfoFromRegistry(vector<OUString> & vecHome)
{
    return getJavaInfoFromRegistry(HKEY_SUN_SDK, vecHome);
}

bool getJREInfoFromRegistry(vector<OUString>& vecJavaHome)
{
    return getJavaInfoFromRegistry(HKEY_SUN_JRE, vecJavaHome);
}

#endif // WNT

void bubbleSortVersion(vector<rtl::Reference<VendorBase> >& vec)
{
    if(vec.empty())
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

            int nCmp = 0;
            // comparing invalid SunVersion s is possible, they will be less than a
            // valid version

            //check if version of current is recognized, by comparing it with itself
            try
            {
                cur->compareVersions(cur->getVersion());
            }
            catch (MalformedVersionException &)
            {
                nCmp = -1; // current < next
            }
            //The version of cur is valid, now compare with the second version
            if (nCmp == 0)
            {
                try
                {
                    nCmp = cur->compareVersions(next->getVersion());
                }
                catch (MalformedVersionException & )
                {
                    //The second version is invalid, therefor it is regardes less.
                    nCmp = 1;
                }
            }
            if(nCmp == 1) // cur > next
            {
                rtl::Reference<VendorBase> less = next;
                vec.at(j-1)= cur;
                vec.at(j)= less;
            }
        }
        ++cIter;
    }
}


bool getJREInfoFromBinPath(
    const OUString& path, vector<rtl::Reference<VendorBase> > & vecInfos)
{
    // file:///c:/jre/bin
    //map:       jre/bin/java.exe
    bool ret = false;

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
        for (c_it i = vecPaths.begin(); i != vecPaths.end(); ++i)
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
            if (!sHome.isEmpty())
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

#ifndef JVM_ONE_PATH_CHECK
    createJavaInfoFromJavaHome(vecInfos);
    //this function should be called after createJavaInfoDirScan.
    //Otherwise in SDKs Java may be started twice
     createJavaInfoFromPath(vecInfos);
#endif

#ifdef UNX
    createJavaInfoDirScan(vecInfos);
#endif

    bubbleSortVersion(vecInfos);
    return vecInfos;
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
bool getJREInfoByPath(const OUString& path,
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

/** Checks if the path is a directory. Links are resolved.
    In case of an error the returned string has the length 0.
    Otherwise the returned string is the "resolved" file URL.
 */
OUString resolveDirPath(const OUString & path)
{
    OUString ret;
    salhelper::LinkResolver aResolver(osl_FileStatus_Mask_Type |
                                       osl_FileStatus_Mask_FileURL);
    if (aResolver.fetchFileStatus(path) == osl::FileBase::E_None)
    {
        //check if this is a directory
        if (aResolver.m_aStatus.getFileType() == FileStatus::Directory)
        {
#ifndef JVM_ONE_PATH_CHECK
            ret = aResolver.m_aStatus.getFileURL();
#else
            ret = path;
#endif
        }
    }
    return ret;
}
/** Checks if the path is a file. If it is a link to a file than
    it is resolved.
 */
OUString resolveFilePath(const OUString & path)
{
    OUString ret;
    salhelper::LinkResolver aResolver(osl_FileStatus_Mask_Type |
                                       osl_FileStatus_Mask_FileURL);
    if (aResolver.fetchFileStatus(path) == osl::FileBase::E_None)
    {
        //check if this is a file
        if (aResolver.m_aStatus.getFileType() == FileStatus::Regular)
        {
#ifndef JVM_ONE_PATH_CHECK
            ret = aResolver.m_aStatus.getFileURL();
#else
            ret = path;
#endif
        }
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
    if (sResolvedDir.isEmpty())
        return 0;

    //check if the directory path is good, that is a JRE was already recognized.
    //Then we need not detect it again
    //For example, a sun JKD contains <jdk>/bin/java and <jdk>/jre/bin/java.
    //When <jdk>/bin/java has been found then we need not find <jdk>/jre/bin/java.
    //Otherwise we would execute java two times for evers JDK found.
    MapIt entry2 = find_if(mapJREs.begin(), mapJREs.end(),
                           SameOrSubDirJREMap(sResolvedDir));
    if (entry2 != mapJREs.end())
    {
        JFW_TRACE2(OUString("[Java framework] sunjavaplugin")
                   + SAL_DLLEXTENSION + ": "
                   + "JRE found again (detected before): " + sResolvedDir
                   + ".\n");
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

        bool bBreak = false;
        typedef vector<OUString>::const_iterator c_it;
        for (c_it i = vecPaths.begin(); i != vecPaths.end(); ++i)
        {
            //if the path is a link, then resolve it
            //check if the executable exists at all

            //path can be only "file:///". Then do not append a '/'
            //sizeof counts the terminating 0
            OUString sFullPath;
            if (path.getLength() == sizeof("file:///") - 1)
                sFullPath = sResolvedDir + (*i);
            else
                sFullPath = sResolvedDir +
                OUString("/") + (*i);


            sFilePath = resolveFilePath(sFullPath);

            if (sFilePath.isEmpty())
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
                JFW_TRACE2(OUString("[Java framework] sunjavaplugin")
                   + SAL_DLLEXTENSION + ": "
                   + "JRE found again (detected before): " + sFilePath
                   + ".\n");

                return entry->second;
            }

            bool bProcessRun= false;
            if (getJavaProps(sFilePath,
#ifdef JVM_ONE_PATH_CHECK
                             sResolvedDir,
#endif
                             props, & bProcessRun) == false)
            {
                //The java executable could not be run or the system properties
                //could not be retrieved. We can assume that this java is corrupt.
                vecBadPaths.push_back(sFilePath);
                //If there was a java executable, that could be run but we did not get
                //the system properties, then we also assume that the whole Java installation
                //does not work. In a jdk there are two executables. One in jdk/bin and the other
                //in jdk/jre/bin. We do not search any further, because we assume that if one java
                //does not work then the other does not work as well. This saves us to run java
                //again which is quite costly.
                if (bProcessRun == true)
                {
                    // 1.3.1 special treatment: jdk/bin/java and /jdk/jre/bin/java are links to
                    //a script, named .java_wrapper. The script starts jdk/bin/sparc/native_threads/java
                    //or jdk/jre/bin/sparc/native_threads/java. The script uses the name with which it was
                    //invoked to build the path to the executable. It we start the script directy as .java_wrapper
                    //then it tries to start a jdk/.../native_threads/.java_wrapper. Therefore the link, which
                    //is named java, must be used to start the script.
                    getJavaProps(sFullPath,
#ifdef JVM_ONE_PATH_CHECK
                                 sResolvedDir,
#endif
                                 props, & bProcessRun);
                    // Either we found a working 1.3.1
                    //Or the java is broken. In both cases we stop searchin under this "root" directory
                    bBreak = true;
                    break;
                }
                //sFilePath is no working java executable. We continue with another possible
                //path.
                else
                {
                    continue;
                }
            }
            //sFilePath is a java and we could get the system properties. We proceed with this
            //java.
            else
            {
                bBreak = true;
                break;
            }
        }
        if (bBreak)
            break;
    }

    if (props.empty())
        return rtl::Reference<VendorBase>();

    //find java.vendor property
    typedef vector<pair<OUString, OUString> >::const_iterator c_ip;
    OUString sVendor("java.vendor");
    OUString sVendorName;

    for (c_ip i = props.begin(); i != props.end(); ++i)
    {
        if (sVendor.equals(i->first))
        {
            sVendorName = i->second;
            break;
        }
    }

    if (!sVendorName.isEmpty())
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
        vecBadPaths.push_back(sFilePath);
    else
    {
        JFW_TRACE2(OUString("[Java framework] sunjavaplugin")
                   + SAL_DLLEXTENSION + ": "
                   + "Found JRE: " + sResolvedDir
                   + " \n at: " + path + ".\n");

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
    static OUString sCurDir(".");
    static OUString sParentDir("..");
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
                if(!usTokenUrl.isEmpty())
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
                    if(!usBin.isEmpty())
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
        FileStatus status(osl_FileStatus_Mask_FileURL);
        if (item.getFileStatus(status) == File::E_None)
        {
            *fileURL = status.getFileURL();
            ret = true;
        }
    }
    return ret;
}

#ifdef UNX
#ifdef SOLARIS

void createJavaInfoDirScan(vector<rtl::Reference<VendorBase> >& vecInfos)
{
    JFW_TRACE2("\n[Java framework] Checking \"/usr/jdk/latest\"\n");
    getJREInfoByPath("file:////usr/jdk/latest", vecInfos);
}

#elif defined MACOSX && defined X86_64

void createJavaInfoDirScan(vector<rtl::Reference<VendorBase> >& vecInfos)
{
    // Oracle Java 7
    getJREInfoByPath("file:///Library/Internet Plug-Ins/JavaAppletPlugin.plugin/Contents/Home", vecInfos);
}

#else
void createJavaInfoDirScan(vector<rtl::Reference<VendorBase> >& vecInfos)
{
    OUString excMessage = "[Java framework] sunjavaplugin: "
                          "Error in function createJavaInfoDirScan in util.cxx.";
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



    OUString usFile("file:///");
    for( int ii = 0; ii < cSearchPaths; ii ++)
    {
        OUString usDir1(usFile + arPaths[ii]);
        DirectoryItem item;
        if(DirectoryItem::get(usDir1, item) == File::E_None)
        {
            for(int j= 0; j < cCollectDirs; j++)
            {
                OUString usDir2(usDir1 + arCollectDirs[j]);
                // prevent that we scan the whole /usr, /usr/lib, etc directories
                if (!arCollectDirs[j].isEmpty())
                {
                    //usr/java/xxx
                    //Examin every subdirectory
                    Directory aCollectionDir(usDir2);

                    Directory::RC openErr = aCollectionDir.open();
                    switch (openErr)
                    {
                    case File::E_None:
                        break;
                    case File::E_NOENT:
                    case File::E_NOTDIR:
                        continue;
                    case File::E_ACCES:
                        JFW_TRACE2(OUString("[Java framework] sunjavaplugin: ")
                                   + "Could not read directory " + usDir2
                                   + " because of missing access rights.");
                        continue;
                    default:
                        JFW_TRACE2(OUString("[Java framework] sunjavaplugin: ")
                                   + "Could not read directory "
                                   + usDir2 + ". Osl file error: "
                                   + OUString::number(openErr));
                        continue;
                    }

                    DirectoryItem curIt;
                    File::RC errNext = File::E_None;
                    while( (errNext = aCollectionDir.getNextItem(curIt)) == File::E_None)
                    {
                        FileStatus aStatus(osl_FileStatus_Mask_FileURL);
                        File::RC errStatus = File::E_None;
                        if ((errStatus = curIt.getFileStatus(aStatus)) != File::E_None)
                        {
                            JFW_TRACE2(excMessage + "getFileStatus failed with error "
                                + OUString::number(errStatus));
                            continue;
                        }
                        JFW_TRACE2(OUString("[Java framework] sunjavaplugin: ") +
                                   "Checking if directory: " + aStatus.getFileURL() +
                                   " is a Java. \n");

                        getJREInfoByPath(aStatus.getFileURL(),vecInfos);
                    }

                    JFW_ENSURE(errNext == File::E_None || errNext == File::E_NOENT,
                                OUString("[Java framework] sunjavaplugin: ")
                                + "Error while iterating over contens of "
                                + usDir2 + ". Osl file error: "
                                + OUString::number(openErr));
                }
                else
                {
                    //usr/java
                    //When we look directly into a dir like /usr, /usr/lib, etc. then we only
                    //look for certain java directories, such as jre, jdk, etc. Whe do not want
                    //to examine the whole directory because of performance reasons.
                    DirectoryItem item2;
                    if(DirectoryItem::get(usDir2, item2) == File::E_None)
                    {
                        for( int k= 0; k < cJavaNames; k++)
                        {
                            // /usr/java/j2re1.4.0
                            OUString usDir3(usDir2 + arNames[k]);

                            DirectoryItem item3;
                            if(DirectoryItem::get(usDir3, item) == File::E_None)
                            {
                                //remove trailing '/'
                                sal_Int32 islash = usDir3.lastIndexOf('/');
                                if (islash == usDir3.getLength() - 1
                                    && (islash
                                        > RTL_CONSTASCII_LENGTH("file://")))
                                    usDir3 = usDir3.copy(0, islash);
                                getJREInfoByPath(usDir3,vecInfos);
                            }
                        }
                    }
                }
            }
        }
    }
}
#endif // ifdef SOLARIS
#endif // ifdef UNX
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
