/*************************************************************************
 *
 *  $RCSfile: fwkutil.cxx,v $
 *
 *  $Revision: 1.22 $
 *
 *  last change: $Author: obo $ $Date: 2005-06-17 10:12:42 $
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

#ifdef WNT
#include <windows.h>
#endif

#include <string>
#include "osl/mutex.hxx"
#include "osl/module.hxx"
#include "osl/thread.hxx"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/bootstrap.hxx"
#include "osl/file.hxx"
#include "osl/process.h"
#include "rtl/instance.hxx"
#include "rtl/uri.hxx"
#include "osl/getglobalmutex.hxx"

#include "framework.hxx"
#include "fwkutil.hxx"

using namespace rtl;
using namespace osl;

namespace jfw
{

struct Init
{
    osl::Mutex * operator()()
        {
            static osl::Mutex aInstance;
            return &aInstance;
        }
};
osl::Mutex * getFwkMutex()
{
    return rtl_Instance< osl::Mutex, Init, ::osl::MutexGuard,
        ::osl::GetGlobalMutex >::create(
            Init(), ::osl::GetGlobalMutex());
}


bool isAccessibilitySupportDesired()
{
    bool retVal = false;
#ifdef WNT
    HKEY    hKey = 0;
    if (RegOpenKeyEx(HKEY_CURRENT_USER,
                     "Software\\OpenOffice.org\\Accessibility\\AtToolSupport",
                     0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD   dwType = 0;
        DWORD   dwLen = 16;
        unsigned char arData[16];
        if( RegQueryValueEx(hKey, "SupportAssistiveTechnology", NULL, &dwType, arData,
                            & dwLen)== ERROR_SUCCESS)
        {
            if (dwType == REG_SZ)
            {
                if (strcmp((char*) arData, "true") == 0
                    || strcmp((char*) arData, "1") == 0)
                    retVal = true;
                else if (strcmp((char*) arData, "false") == 0
                         || strcmp((char*) arData, "0") == 0)
                    retVal = false;
#if OSL_DEBUG_LEVER > 1
                else
                    OSL_ASSERT(0);
#endif
            }
            else if (dwType == REG_DWORD)
            {
                if (arData[0] == 1)
                    retVal = true;
                else if (arData[0] == 0)
                    retVal = false;
#if OSL_DEBUG_LEVER > 1
                else
                    OSL_ASSERT(0);
#endif
            }
        }
    }
    RegCloseKey(hKey);

#elif UNX
    char buf[16];
    // use 2 shells to suppress the eventual "gcontool-2 not found" message
    // of the shell trying to execute the command
    FILE* fp = popen( "/bin/sh 2>/dev/null -c \"gconftool-2 -g /desktop/gnome/interface/accessibility\"", "r" );
    if( fp )
    {
        if( fgets( buf, sizeof(buf), fp ) )
        {
            int nCompare = strncasecmp( buf, "true", 4 );
            retVal = (nCompare == 0 ? true : false);
        }
        pclose( fp );
    }
#endif
    return retVal;
}


rtl::ByteSequence encodeBase16(const rtl::ByteSequence& rawData)
{
    static char EncodingTable[] =
        {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    sal_Int32 lenRaw = rawData.getLength();
    char* pBuf = new char[lenRaw * 2];
    const sal_Int8* arRaw = rawData.getConstArray();

    char* pCurBuf = pBuf;
    for (int i = 0; i < lenRaw; i++)
    {
        unsigned char curChar = arRaw[i];
        curChar >>= 4;

        *pCurBuf = EncodingTable[curChar];
        pCurBuf++;

        curChar = arRaw[i];
        curChar &= 0x0F;

        *pCurBuf = EncodingTable[curChar];
        pCurBuf++;
    }

    rtl::ByteSequence ret((sal_Int8*) pBuf, lenRaw * 2);
    delete [] pBuf;
    return ret;
}

rtl::ByteSequence decodeBase16(const rtl::ByteSequence& data)
{
    static char decodingTable[] =
        {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    sal_Int32 lenData = data.getLength();
    sal_Int32 lenBuf = lenData / 2; //always divisable by two
    char* pBuf = new char[lenBuf];
    const sal_Int8* arData = data.getConstArray();

    char* pCurBuf = pBuf;
    const sal_Int8* pData = arData;
    for (int i = 0; i < lenBuf; i++)
    {
        sal_Int8 curChar = *pData;
        //find the index of the first 4bits
        //  TODO  What happens if text is not valid Hex characters?
        char nibble = 0;
        for (int ii = 0; ii < 16; ii++)
        {
            if (curChar == decodingTable[ii])
            {
                nibble = ii;
                break;
            }
        }
        nibble <<= 4;
        pData++;
        curChar = *pData;
        //find the index for the next 4bits
        for (int j = 0; j < 16; j++)
        {
            if (curChar == decodingTable[j])
            {
                nibble |= j;
                break;
            }
        }
        *pCurBuf = nibble;
        pData++;
        pCurBuf++;
    }
    rtl::ByteSequence ret((sal_Int8*) pBuf, lenBuf );
    delete [] pBuf;
    return ret;
}

rtl::OUString getDirFromFile(const rtl::OUString& usFilePath)
{
    sal_Int32 index= usFilePath.lastIndexOf('/');
    return rtl::OUString(usFilePath.getStr(), index);
}

rtl::OUString getFileFromURL(const rtl::OUString& sFileURL)
{
    sal_Int32 index= sFileURL.lastIndexOf('/');
    if (index == -1)
        return sFileURL;
    return sFileURL.copy(index + 1);
}

rtl::OUString getExecutableDirectory()
{
    rtl_uString* sExe = NULL;
    if (osl_getExecutableFile( & sExe) != osl_Process_E_None)
        throw FrameworkException(
            JFW_E_ERROR,
            "[Java framework] Error in function getApplicationBase (fwkutil.cxx)");

    rtl::OUString ouExe(sExe, SAL_NO_ACQUIRE);
    return getDirFromFile(ouExe);
}

rtl::OUString findPlugin(
    const rtl::OUString & baseUrl, const rtl::OUString & plugin)
{
    rtl::OUString sUrl;
    try
    {
        sUrl = rtl::Uri::convertRelToAbs(baseUrl, plugin);
    }
    catch (rtl::MalformedUriException & e)
    {
        throw FrameworkException(
            JFW_E_ERROR,
            (rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "[Java framework] rtl::MalformedUriException in"
                    " findPlugin: "))
             + rtl::OUStringToOString(
                 e.getMessage(), osl_getThreadTextEncoding())));
    }
    if (checkFileURL(sUrl) == jfw::FILE_OK)
    {
        return sUrl;
    }
    rtl::OUString retVal;
    rtl::OUString sProgDir = getExecutableDirectory();
    sUrl = sProgDir + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"))
        + plugin;
    jfw::FileStatus s = checkFileURL(sUrl);
    if (s == jfw::FILE_INVALID || s == jfw::FILE_DOES_NOT_EXIST)
    {
        //If only the name of the library is given, then
        //use PATH, LD_LIBRARY_PATH etc. to locate the plugin
        if (plugin.indexOf('/') == -1)
        {
            rtl::OUString url;
#ifdef UNX
#ifdef MACOSX
            rtl::OUString path = rtl::OUString::createFromAscii("DYLD_LIBRARY_PATH");
#else
            rtl::OUString path = rtl::OUString::createFromAscii("LD_LIBRARY_PATH");
#endif
            rtl::OUString env_path;
            oslProcessError err = osl_getEnvironment(path.pData, &env_path.pData);
            if (err != osl_Process_E_None && err != osl_Process_E_NotFound)
                throw FrameworkException(
                    JFW_E_ERROR,
                    "[Java framework] Error in function findPlugin (fwkutil.cxx).");
            if (err == osl_Process_E_NotFound)
                return retVal;
            if (osl_searchFileURL(plugin.pData, env_path.pData, &url.pData)
                                == osl_File_E_None)
#else
            if (osl_searchFileURL(plugin.pData, NULL, &url.pData)
                == osl_File_E_None)
#endif
                retVal = url;
            else
                throw FrameworkException(
                    JFW_E_ERROR,
                    "[Java framework] Error in function findPlugin (fwkutil.cxx).");
        }
    }
    else
    {
        retVal = sUrl;
    }
    return retVal;
}

rtl::OUString getLibraryLocation()
{
    rtl::OString sExcMsg("[Java framework] Error in function getLibraryLocation "
                         "(fwkutil.cxx).");
    rtl::OUString libraryFileUrl;

    if (osl::Module::getUrlFromAddress((void *) getLibraryLocation, libraryFileUrl)
        == sal_False)
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    return getDirFromFile(libraryFileUrl);
}

//Todo is this still needed?
rtl::OUString searchFileNextToThisLib(const rtl::OUString & sFile)
{
    rtl::OUString ret;
    rtl::OUString sLib;
    if (osl_getModuleURLFromAddress((void *) & searchFileNextToThisLib,
                                    & sLib.pData) == sal_True)
    {
        sLib = getDirFromFile(sLib);
        rtl::OUStringBuffer sBufVendor(256);
        sBufVendor.append(sLib);
        sBufVendor.appendAscii("/");
        sBufVendor.append(sFile);
        sLib =  sBufVendor.makeStringAndClear();
        //check if the file exists
        osl::DirectoryItem item;
        osl::File::RC fileError = osl::DirectoryItem::get(sLib, item);
        if (fileError == osl::FileBase::E_None)
            ret = sLib;
    }
    return ret;
}

jfw::FileStatus checkFileURL(const rtl::OUString & path)
{
    rtl::OString sExcMsg("[Java framework] Error in function "
                         "resolveFileURL (fwkutil.cxx).");
    OUString sResolved = path;
    jfw::FileStatus ret = jfw::FILE_OK;
    while (1)
    {
        DirectoryItem item;
        File::RC fileErr =
            DirectoryItem::get(sResolved, item);
        if (fileErr == File::E_None)
        {
            osl::FileStatus status(FileStatusMask_Type |
                              FileStatusMask_LinkTargetURL |
                              FileStatusMask_FileURL);

            if (item.getFileStatus(status) == File::E_None)
            {
                osl::FileStatus::Type t = status.getFileType();
                if (t == osl::FileStatus::Regular)
                {
                    ret = jfw::FILE_OK;
                    break;
                }
                else if ( t == osl::FileStatus::Link )
                {
                    sResolved = status.getLinkTargetURL();
                }
                else
                {
                    ret = FILE_INVALID;
                    break;
                }
            }
            else
            {
                throw FrameworkException(JFW_E_ERROR, sExcMsg);
            }
        }
        else if(fileErr == File::E_NOENT)
        {
            ret = FILE_DOES_NOT_EXIST;
            break;
        }
        else
        {
            ret = FILE_INVALID;
            break;
        }
    }
    return ret;
}
const rtl::Bootstrap& getBootstrap()
{
    static rtl::Bootstrap *pBootstrap = 0;
    rtl::OUString sIni;
    if( !pBootstrap )
    {
        rtl::OUStringBuffer buf( 255);
        buf.append( getLibraryLocation());
        buf.appendAscii( SAL_CONFIGFILE("/jvmfwk3") );
        sIni = buf.makeStringAndClear();
        static rtl::Bootstrap  bootstrap(sIni);
        pBootstrap = &bootstrap;
#if OSL_DEBUG_LEVEL >=2
        rtl::OString o = rtl::OUStringToOString( sIni , osl_getThreadTextEncoding() );
        fprintf(stderr, "[Java framework] Using configuration file %s\n" , o.getStr() );
#endif
    }

    return *pBootstrap;

}

}
