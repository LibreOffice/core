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

#if defined WNT
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#endif

#include <string>
#include <string.h>
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
#include "com/sun/star/lang/IllegalArgumentException.hpp"
#include "cppuhelper/bootstrap.hxx"

#include "framework.hxx"
#include "fwkutil.hxx"

using namespace osl;

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;

namespace jfw
{

bool isAccessibilitySupportDesired()
{
    OUString sValue;
    if ((sal_True == ::rtl::Bootstrap::get(
        OUString(RTL_CONSTASCII_USTRINGPARAM("JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY")), sValue))
        && sValue.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("1")))
        )
        return false;

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
#if OSL_DEBUG_LEVEL > 1
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
#if OSL_DEBUG_LEVEL > 1
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
    unsigned char* pBuf = new unsigned char[lenBuf];
    const sal_Int8* pData = data.getConstArray();
    for (sal_Int32 i = 0; i < lenBuf; i++)
    {
        sal_Int8 curChar = *pData++;
        //find the index of the first 4bits
        //  TODO  What happens if text is not valid Hex characters?
        unsigned char nibble = 0;
        for (unsigned char j = 0; j < 16; j++)
        {
            if (curChar == decodingTable[j])
            {
                nibble = j;
                break;
            }
        }
        nibble <<= 4;
        curChar = *pData++;
        //find the index for the next 4bits
        for (unsigned char j = 0; j < 16; j++)
        {
            if (curChar == decodingTable[j])
            {
                nibble |= j;
                break;
            }
        }
        pBuf[i] = nibble;
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

rtl::OUString getExecutableDirectory()
{
    rtl_uString* sExe = NULL;
    if (osl_getExecutableFile( & sExe) != osl_Process_E_None)
        throw FrameworkException(
            JFW_E_ERROR,
            "[Java framework] Error in function getExecutableDirectory (fwkutil.cxx)");

    rtl::OUString ouExe(sExe, SAL_NO_ACQUIRE);
    return getDirFromFile(ouExe);
}

rtl::OUString findPlugin(
    const rtl::OUString & baseUrl, const rtl::OUString & plugin)
{
    rtl::OUString expandedPlugin;
    try
    {
        expandedPlugin = cppu::bootstrap_expandUri(plugin);
    }
    catch (com::sun::star::lang::IllegalArgumentException & e)
    {
        throw FrameworkException(
            JFW_E_ERROR,
            (rtl::OString(
                RTL_CONSTASCII_STRINGPARAM(
                    "[Java framework] IllegalArgumentException in"
                    " findPlugin: "))
             + rtl::OUStringToOString(e.Message, osl_getThreadTextEncoding())));
    }
    rtl::OUString sUrl;
    try
    {
        sUrl = rtl::Uri::convertRelToAbs(baseUrl, expandedPlugin);
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
#if defined(MACOSX)
            rtl::OUString path = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DYLD_LIBRARY_PATH"));
#elif defined(AIX)
            rtl::OUString path = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LIBPATH"));
#else
            rtl::OUString path = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("LD_LIBRARY_PATH"));
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

    if (!osl::Module::getUrlFromAddress(
            reinterpret_cast< oslGenericFunction >(getLibraryLocation),
            libraryFileUrl))
        throw FrameworkException(JFW_E_ERROR, sExcMsg);

    return getDirFromFile(libraryFileUrl);
}

jfw::FileStatus checkFileURL(const rtl::OUString & sURL)
{
    jfw::FileStatus ret = jfw::FILE_OK;
    DirectoryItem item;
    File::RC rc_item = DirectoryItem::get(sURL, item);
    if (File::E_None == rc_item)
    {
        osl::FileStatus status(FileStatusMask_Validate);

        File::RC rc_stat = item.getFileStatus(status);
        if (File::E_None == rc_stat)
        {
            ret = FILE_OK;
        }
        else if (File::E_NOENT == rc_stat)
        {
            ret = FILE_DOES_NOT_EXIST;
        }
        else
        {
            ret = FILE_INVALID;
        }
    }
    else if (File::E_NOENT == rc_item)
    {
        ret = FILE_DOES_NOT_EXIST;
    }
    else
    {
        ret = FILE_INVALID;
    }
    return ret;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
