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


#if defined(_WIN32)
#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#endif

#include <string>
#include <string.h>
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
#include <memory>

using namespace osl;


namespace jfw
{

bool isAccessibilitySupportDesired()
{
    OUString sValue;
    if (::rtl::Bootstrap::get( "JFW_PLUGIN_DO_NOT_CHECK_ACCESSIBILITY", sValue) &&
        sValue == "1" )
        return false;

#ifdef _WIN32
    bool retVal = false;
    HKEY    hKey = nullptr;
    if (RegOpenKeyExA(HKEY_CURRENT_USER,
                      "Software\\LibreOffice\\Accessibility\\AtToolSupport",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        DWORD   dwType = 0;
        DWORD   dwLen = 16;
        unsigned char arData[16];
        if( RegQueryValueExA(hKey, "SupportAssistiveTechnology", nullptr, &dwType, arData,
                             &dwLen)== ERROR_SUCCESS)
        {
            if (dwType == REG_SZ)
            {
                arData[std::min(dwLen, DWORD(15))] = 0;
                if (strcmp(reinterpret_cast<char*>(arData), "true") == 0
                    || strcmp(reinterpret_cast<char*>(arData), "1") == 0)
                    retVal = true;
                else if (strcmp(reinterpret_cast<char*>(arData), "false") == 0
                         || strcmp(reinterpret_cast<char*>(arData), "0") == 0)
                    retVal = false;
                else
                    SAL_WARN("jfw", "bad registry value " << arData);
            }
            else if (dwType == REG_DWORD)
            {
                if (arData[0] == 1)
                    retVal = true;
                else if (arData[0] == 0)
                    retVal = false;
                else
                    SAL_WARN(
                        "jfw", "bad registry value " << unsigned(arData[0]));
            }
        }
        RegCloseKey(hKey);
    }
#elif defined UNX
    // Java is no longer required for a11y - we use atk directly.
    bool retVal = ::rtl::Bootstrap::get( "JFW_PLUGIN_FORCE_ACCESSIBILITY", sValue) && sValue == "1";
#endif

    return retVal;
}

rtl::ByteSequence encodeBase16(const rtl::ByteSequence& rawData)
{
    static const char EncodingTable[] =
        {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    sal_Int32 lenRaw = rawData.getLength();
    std::unique_ptr<char[]> pBuf(new char[lenRaw * 2]);
    const sal_Int8* arRaw = rawData.getConstArray();

    char* pCurBuf = pBuf.get();
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

    rtl::ByteSequence ret(reinterpret_cast<sal_Int8*>(pBuf.get()), lenRaw * 2);
    return ret;
}

rtl::ByteSequence decodeBase16(const rtl::ByteSequence& data)
{
    static const char decodingTable[] =
        {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    sal_Int32 lenData = data.getLength();
    sal_Int32 lenBuf = lenData / 2; //always divisable by two
    std::unique_ptr<unsigned char[]> pBuf(new unsigned char[lenBuf]);
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
    rtl::ByteSequence ret(reinterpret_cast<sal_Int8*>(pBuf.get()), lenBuf );
    return ret;
}

OUString getDirFromFile(const OUString& usFilePath)
{
    sal_Int32 index = usFilePath.lastIndexOf('/');
    return usFilePath.copy(0, index);
}

OUString getLibraryLocation()
{
    OUString libraryFileUrl;

    if (!osl::Module::getUrlFromAddress(
            reinterpret_cast< oslGenericFunction >(getLibraryLocation),
            libraryFileUrl))
        throw FrameworkException(JFW_E_ERROR,
                    "[Java framework] Error in function getLibraryLocation (fwkutil.cxx).");

    return getDirFromFile(libraryFileUrl);
}

jfw::FileStatus checkFileURL(const OUString & sURL)
{
    jfw::FileStatus ret = jfw::FILE_OK;
    DirectoryItem item;
    File::RC rc_item = DirectoryItem::get(sURL, item);
    if (File::E_None == rc_item)
    {
        osl::FileStatus status(osl_FileStatus_Mask_Validate);

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
