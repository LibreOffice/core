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
#define _CONFIG_CXX

#include <cstddef>
#include <cstdlib>
#include <limits>
#include <new>
#include <string.h>

#ifdef WNT
#include "stdlib.h"
#endif

#include <osl/file.hxx>
#include <tools/stream.hxx>
#include <tools/debug.hxx>
#include <tools/config.hxx>
#include <osl/security.h>
#include <rtl/strbuf.hxx>

struct ImplKeyData
{
    ImplKeyData*    mpNext;
    rtl::OString maKey;
    rtl::OString maValue;
    sal_Bool            mbIsComment;
};

struct ImplGroupData
{
    ImplGroupData*  mpNext;
    ImplKeyData*    mpFirstKey;
    rtl::OString maGroupName;
    sal_uInt16          mnEmptyLines;
};

struct ImplConfigData
{
    ImplGroupData*  mpFirstGroup;
    rtl::OUString   maFileName;
    sal_uIntPtr         mnDataUpdateId;
    sal_uIntPtr         mnTimeStamp;
    LineEnd         meLineEnd;
    sal_uInt16          mnRefCount;
    sal_Bool            mbModified;
    sal_Bool            mbRead;
    sal_Bool            mbIsUTF8BOM;
};

static String toUncPath( const String& rPath )
{
    ::rtl::OUString aFileURL;

    // check if rFileName is already a URL; if not make it so
    if( rPath.CompareToAscii( "file://", 7 ) == COMPARE_EQUAL )
        aFileURL = rPath;
    else if( ::osl::FileBase::getFileURLFromSystemPath( rPath, aFileURL ) != ::osl::FileBase::E_None )
        aFileURL = rPath;

    return aFileURL;
}

static sal_uIntPtr ImplSysGetConfigTimeStamp( const rtl::OUString& rFileName )
{
    sal_uIntPtr nTimeStamp = 0;
    ::osl::DirectoryItem aItem;
    ::osl::FileStatus aStatus( osl_FileStatus_Mask_ModifyTime );

    if( ::osl::DirectoryItem::get( rFileName, aItem ) == ::osl::FileBase::E_None &&
        aItem.getFileStatus( aStatus ) == ::osl::FileBase::E_None )
    {
        nTimeStamp = aStatus.getModifyTime().Seconds;
    }

    return nTimeStamp;
}

static sal_uInt8* ImplSysReadConfig( const rtl::OUString& rFileName,
                                sal_uInt64& rRead, sal_Bool& rbRead, sal_Bool& rbIsUTF8BOM, sal_uIntPtr& rTimeStamp )
{
    sal_uInt8*          pBuf = NULL;
    ::osl::File aFile( rFileName );

    if( aFile.open( osl_File_OpenFlag_Read ) == ::osl::FileBase::E_None )
    {
        sal_uInt64 nPos = 0;
        if( aFile.getSize( nPos ) == ::osl::FileBase::E_None )
        {
            if (nPos > std::numeric_limits< std::size_t >::max()) {
                aFile.close();
                return 0;
            }
            pBuf = new sal_uInt8[static_cast< std::size_t >(nPos)];
            sal_uInt64 nRead = 0;
            if( aFile.read( pBuf, nPos, nRead ) == ::osl::FileBase::E_None && nRead == nPos )
            {
                //skip the byte-order-mark 0xEF 0xBB 0xBF, if it was UTF8 files
                unsigned char BOM[3] = {0xEF, 0xBB, 0xBF};
                if (nRead > 2 && memcmp(pBuf, BOM, 3) == 0)
                {
                    nRead -= 3;
                    memmove(pBuf, pBuf + 3, sal::static_int_cast<sal_Size>(nRead * sizeof(sal_uInt8)) );
                    rbIsUTF8BOM = sal_True;
                }

                rTimeStamp = ImplSysGetConfigTimeStamp( rFileName );
                rbRead = sal_True;
                rRead = nRead;
            }
            else
            {
                delete[] pBuf;
                pBuf = NULL;
            }
        }
        aFile.close();
    }

    return pBuf;
}

static sal_Bool ImplSysWriteConfig( const rtl::OUString& rFileName,
                                const sal_uInt8* pBuf, sal_uIntPtr nBufLen, sal_Bool rbIsUTF8BOM, sal_uIntPtr& rTimeStamp )
{
    sal_Bool bSuccess = sal_False;
    sal_Bool bUTF8BOMSuccess = sal_False;

    ::osl::File aFile( rFileName );
    ::osl::FileBase::RC eError = aFile.open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
    if( eError != ::osl::FileBase::E_None )
        eError = aFile.open( osl_File_OpenFlag_Write );
    if( eError == ::osl::FileBase::E_None )
    {
        // truncate
        aFile.setSize( 0 );
        sal_uInt64 nWritten;

        //write the the byte-order-mark 0xEF 0xBB 0xBF first , if it was UTF8 files
        if ( rbIsUTF8BOM )
        {
            unsigned char BOM[3] = {0xEF, 0xBB, 0xBF};
            sal_uInt64 nUTF8BOMWritten;
            if( aFile.write( BOM, 3, nUTF8BOMWritten ) == ::osl::FileBase::E_None && 3 == nUTF8BOMWritten )
            {
                bUTF8BOMSuccess = sal_True;
            }
        }

        if( aFile.write( pBuf, nBufLen, nWritten ) == ::osl::FileBase::E_None && nWritten == nBufLen )
        {
            bSuccess = sal_True;
        }
        if ( rbIsUTF8BOM ? bSuccess && bUTF8BOMSuccess : bSuccess )
        {
            rTimeStamp = ImplSysGetConfigTimeStamp( rFileName );
        }
    }

    return rbIsUTF8BOM ? bSuccess && bUTF8BOMSuccess : bSuccess;
}

namespace {
rtl::OString makeOString(const sal_uInt8* p, sal_uInt64 n)
{
    if (n > SAL_MAX_INT32)
    {
        #ifdef WNT
        abort();
        #else
        ::std::abort(); //TODO: handle this gracefully
        #endif
    }
    return rtl::OString(
        reinterpret_cast< char const * >(p),
        sal::static_int_cast< sal_Int32 >(n));
}
}

static void ImplMakeConfigList( ImplConfigData* pData,
                                const sal_uInt8* pBuf, sal_uInt64 nLen )
{
    if ( !nLen )
        return;

    // Parse buffer and build config list
    sal_uInt64 nStart;
    sal_uInt64 nLineLen;
    sal_uInt64 nNameLen;
    sal_uInt64 nKeyLen;
    sal_uInt64 i;
    const sal_uInt8*    pLine;
    ImplKeyData*    pPrevKey = NULL;
    ImplKeyData*    pKey;
    ImplGroupData*  pPrevGroup = NULL;
    ImplGroupData*  pGroup = NULL;
    i = 0;
    while ( i < nLen )
    {
        // Ctrl+Z
        if ( pBuf[i] == 0x1A )
            break;

        // Remove spaces and tabs
        while ( (pBuf[i] == ' ') || (pBuf[i] == '\t') )
            i++;

        // remember line-starts
        nStart = i;
        pLine = pBuf+i;

        // search line-endings
        while (  (i < nLen) && pBuf[i] && (pBuf[i] != '\r') && (pBuf[i] != '\n') &&
                (pBuf[i] != 0x1A) )
            i++;

        nLineLen = i-nStart;

        // if Line-ending is found, continue once
        if ( (i+1 < nLen) &&
             (pBuf[i] != pBuf[i+1]) &&
             ((pBuf[i+1] == '\r') || (pBuf[i+1] == '\n')) )
            i++;
        i++;

        // evaluate line
        if ( *pLine == '[' )
        {
            pGroup               = new ImplGroupData;
            pGroup->mpNext       = NULL;
            pGroup->mpFirstKey   = NULL;
            pGroup->mnEmptyLines = 0;
            if ( pPrevGroup )
                pPrevGroup->mpNext = pGroup;
            else
                pData->mpFirstGroup = pGroup;
            pPrevGroup  = pGroup;
            pPrevKey    = NULL;
            pKey        = NULL;

            // filter group names
            pLine++;
            nLineLen--;
            // remove spaces and tabs
            while ( (*pLine == ' ') || (*pLine == '\t') )
            {
                nLineLen--;
                pLine++;
            }
            nNameLen = 0;
            while ( (nNameLen < nLineLen) && (pLine[nNameLen] != ']') )
                nNameLen++;
            if ( nNameLen )
            {
                while ( (pLine[nNameLen-1] == ' ') || (pLine[nNameLen-1] == '\t') )
                    nNameLen--;
            }
            pGroup->maGroupName = makeOString(pLine, nNameLen);
        }
        else
        {
            if ( nLineLen )
            {
                // If no group exists yet, add to default
                if ( !pGroup )
                {
                    pGroup              = new ImplGroupData;
                    pGroup->mpNext      = NULL;
                    pGroup->mpFirstKey  = NULL;
                    pGroup->mnEmptyLines = 0;
                    if ( pPrevGroup )
                        pPrevGroup->mpNext = pGroup;
                    else
                        pData->mpFirstGroup = pGroup;
                    pPrevGroup  = pGroup;
                    pPrevKey    = NULL;
                }

                // if empty line, append it
                if ( pPrevKey )
                {
                    while ( pGroup->mnEmptyLines )
                    {
                        pKey                = new ImplKeyData;
                        pKey->mbIsComment   = sal_True;
                        pPrevKey->mpNext    = pKey;
                        pPrevKey            = pKey;
                        pGroup->mnEmptyLines--;
                    }
                }

                // Generate new key
                pKey        = new ImplKeyData;
                pKey->mpNext = NULL;
                if ( pPrevKey )
                    pPrevKey->mpNext = pKey;
                else
                    pGroup->mpFirstKey = pKey;
                pPrevKey = pKey;
                if ( pLine[0] == ';' )
                {
                    pKey->maValue = makeOString(pLine, nLineLen);
                    pKey->mbIsComment = sal_True;
                }
                else
                {
                    pKey->mbIsComment = sal_False;
                    nNameLen = 0;
                    while ( (nNameLen < nLineLen) && (pLine[nNameLen] != '=') )
                        nNameLen++;
                    nKeyLen = nNameLen;
                    // Remove spaces and tabs
                    if ( nNameLen )
                    {
                        while ( (pLine[nNameLen-1] == ' ') || (pLine[nNameLen-1] == '\t') )
                            nNameLen--;
                    }
                    pKey->maKey = makeOString(pLine, nNameLen);
                    nKeyLen++;
                    if ( nKeyLen < nLineLen )
                    {
                        pLine += nKeyLen;
                        nLineLen -= nKeyLen;
                        // Remove spaces and tabs
                        while ( (*pLine == ' ') || (*pLine == '\t') )
                        {
                            nLineLen--;
                            pLine++;
                        }
                        if ( nLineLen )
                        {
                            while ( (pLine[nLineLen-1] == ' ') || (pLine[nLineLen-1] == '\t') )
                                nLineLen--;
                            pKey->maValue = makeOString(pLine, nLineLen);
                        }
                    }
                }
            }
            else
            {
                // Spaces are counted and appended only after key generation,
                // as we want to store spaces even after adding new keys
                if ( pGroup )
                    pGroup->mnEmptyLines++;
            }
        }
    }
}

static sal_uInt8* ImplGetConfigBuffer( const ImplConfigData* pData, sal_uIntPtr& rLen )
{
    sal_uInt8*      pWriteBuf;
    sal_uInt8*      pBuf;
    sal_uInt8       aLineEndBuf[2] = {0, 0};
    ImplKeyData*    pKey;
    ImplGroupData*  pGroup;
    unsigned int    nBufLen;
    sal_uInt32      nValueLen;
    sal_uInt32      nKeyLen;
    sal_uInt32      nLineEndLen;

    if ( pData->meLineEnd == LINEEND_CR )
    {
        aLineEndBuf[0] = _CR;
        nLineEndLen = 1;
    }
    else if ( pData->meLineEnd == LINEEND_LF )
    {
        aLineEndBuf[0] = _LF;
        nLineEndLen = 1;
    }
    else
    {
        aLineEndBuf[0] = _CR;
        aLineEndBuf[1] = _LF;
        nLineEndLen = 2;
    }

    nBufLen = 0;
    pGroup = pData->mpFirstGroup;
    while ( pGroup )
    {
        // Don't write empty groups
        if ( pGroup->mpFirstKey )
        {
            nBufLen += pGroup->maGroupName.getLength() + nLineEndLen + 2;
            pKey = pGroup->mpFirstKey;
            while ( pKey )
            {
                nValueLen = pKey->maValue.getLength();
                if ( pKey->mbIsComment )
                    nBufLen += nValueLen + nLineEndLen;
                else
                    nBufLen += pKey->maKey.getLength() + nValueLen + nLineEndLen + 1;

                pKey = pKey->mpNext;
            }

            // Write empty lines after each group
            if ( !pGroup->mnEmptyLines )
                pGroup->mnEmptyLines = 1;
            nBufLen += nLineEndLen * pGroup->mnEmptyLines;
        }

        pGroup = pGroup->mpNext;
    }

    // Output buffer length
    rLen = nBufLen;
    if ( !nBufLen )
    {
        pWriteBuf = new sal_uInt8[nLineEndLen];
        if ( pWriteBuf )
        {
            pWriteBuf[0] = aLineEndBuf[0];
            if ( nLineEndLen == 2 )
                pWriteBuf[1] = aLineEndBuf[1];
            return pWriteBuf;
        }
        else
            return 0;
    }

    // Allocate new write buffer (caller frees it)
    pWriteBuf = new sal_uInt8[nBufLen];
    if ( !pWriteBuf )
        return 0;

    // fill buffer
    pBuf = pWriteBuf;
    pGroup = pData->mpFirstGroup;
    while ( pGroup )
    {
        // Don't write empty groups
        if ( pGroup->mpFirstKey )
        {
            *pBuf = '[';    pBuf++;
            memcpy( pBuf, pGroup->maGroupName.getStr(), pGroup->maGroupName.getLength() );
            pBuf += pGroup->maGroupName.getLength();
            *pBuf = ']';    pBuf++;
            *pBuf = aLineEndBuf[0]; pBuf++;
            if ( nLineEndLen == 2 )
            {
                *pBuf = aLineEndBuf[1]; pBuf++;
            }
            pKey = pGroup->mpFirstKey;
            while ( pKey )
            {
                nValueLen = pKey->maValue.getLength();
                if ( pKey->mbIsComment )
                {
                    if ( nValueLen )
                    {
                        memcpy( pBuf, pKey->maValue.getStr(), nValueLen );
                        pBuf += nValueLen;
                    }
                    *pBuf = aLineEndBuf[0]; pBuf++;
                    if ( nLineEndLen == 2 )
                    {
                        *pBuf = aLineEndBuf[1]; pBuf++;
                    }
                }
                else
                {
                    nKeyLen = pKey->maKey.getLength();
                    memcpy( pBuf, pKey->maKey.getStr(), nKeyLen );
                    pBuf += nKeyLen;
                    *pBuf = '=';    pBuf++;
                    memcpy( pBuf, pKey->maValue.getStr(), nValueLen );
                    pBuf += nValueLen;
                    *pBuf = aLineEndBuf[0]; pBuf++;
                    if ( nLineEndLen == 2 )
                    {
                        *pBuf = aLineEndBuf[1]; pBuf++;
                    }
                }

                pKey = pKey->mpNext;
            }

            // Store empty line after each group
            sal_uInt16 nEmptyLines = pGroup->mnEmptyLines;
            while ( nEmptyLines )
            {
                *pBuf = aLineEndBuf[0]; pBuf++;
                if ( nLineEndLen == 2 )
                {
                    *pBuf = aLineEndBuf[1]; pBuf++;
                }
                nEmptyLines--;
            }
        }

        pGroup = pGroup->mpNext;
    }

    return pWriteBuf;
}

static void ImplReadConfig( ImplConfigData* pData )
{
    sal_uIntPtr nTimeStamp = 0;
    sal_uInt64  nRead = 0;
    sal_Bool    bRead = sal_False;
    sal_Bool    bIsUTF8BOM =sal_False;
    sal_uInt8*  pBuf = ImplSysReadConfig( pData->maFileName, nRead, bRead, bIsUTF8BOM, nTimeStamp );

    // Read config list from buffer
    if ( pBuf )
    {
        ImplMakeConfigList( pData, pBuf, nRead );
        delete[] pBuf;
    }
    pData->mnTimeStamp = nTimeStamp;
    pData->mbModified  = sal_False;
    if ( bRead )
        pData->mbRead = sal_True;
    if ( bIsUTF8BOM )
        pData->mbIsUTF8BOM = sal_True;
}

static void ImplWriteConfig( ImplConfigData* pData )
{
#ifdef DBG_UTIL
    if ( DbgIsAssert() )
    {
        if ( pData->mnTimeStamp != ImplSysGetConfigTimeStamp( pData->maFileName ) )
        {
            OSL_TRACE( "Config overwrites modified configfile:\n %s", rtl::OUStringToOString(pData->maFileName, RTL_TEXTENCODING_UTF8).getStr() );
        }
    }
#endif

    // Read config list from buffer
    sal_uIntPtr nBufLen;
    sal_uInt8*  pBuf = ImplGetConfigBuffer( pData, nBufLen );
    if ( pBuf )
    {
        if ( ImplSysWriteConfig( pData->maFileName, pBuf, nBufLen, pData->mbIsUTF8BOM, pData->mnTimeStamp ) )
            pData->mbModified = sal_False;
        delete[] pBuf;
    }
    else
        pData->mbModified = sal_False;
}

static void ImplDeleteConfigData( ImplConfigData* pData )
{
    ImplKeyData*    pTempKey;
    ImplKeyData*    pKey;
    ImplGroupData*  pTempGroup;
    ImplGroupData*  pGroup = pData->mpFirstGroup;
    while ( pGroup )
    {
        pTempGroup = pGroup->mpNext;

        // remove all keys
        pKey = pGroup->mpFirstKey;
        while ( pKey )
        {
            pTempKey = pKey->mpNext;
            delete pKey;
            pKey = pTempKey;
        }

        // remove group and continue
        delete pGroup;
        pGroup = pTempGroup;
    }

    pData->mpFirstGroup = NULL;
}

static ImplConfigData* ImplGetConfigData( const rtl::OUString& rFileName )
{
    ImplConfigData* pData;

    pData                   = new ImplConfigData;
    pData->maFileName       = rFileName;
    pData->mpFirstGroup     = NULL;
    pData->mnDataUpdateId   = 0;
    pData->meLineEnd        = LINEEND_CRLF;
    pData->mnRefCount       = 0;
    pData->mbRead           = sal_False;
    pData->mbIsUTF8BOM      = sal_False;
    ImplReadConfig( pData );

    return pData;
}

static void ImplFreeConfigData( ImplConfigData* pDelData )
{
    ImplDeleteConfigData( pDelData );
    delete pDelData;
}

sal_Bool Config::ImplUpdateConfig() const
{
    // Re-read file if timestamp differs
    if ( mpData->mnTimeStamp != ImplSysGetConfigTimeStamp( maFileName ) )
    {
        ImplDeleteConfigData( mpData );
        ImplReadConfig( mpData );
        mpData->mnDataUpdateId++;
        return sal_True;
    }
    else
        return sal_False;
}

ImplGroupData* Config::ImplGetGroup() const
{
    if ( !mpActGroup || (mnDataUpdateId != mpData->mnDataUpdateId) )
    {
        ImplGroupData* pPrevGroup = NULL;
        ImplGroupData* pGroup = mpData->mpFirstGroup;
        while ( pGroup )
        {
            if ( pGroup->maGroupName.equalsIgnoreAsciiCase(maGroupName) )
                break;

            pPrevGroup = pGroup;
            pGroup = pGroup->mpNext;
        }

        // Add group if not exists
        if ( !pGroup )
        {
            pGroup               = new ImplGroupData;
            pGroup->mpNext       = NULL;
            pGroup->mpFirstKey   = NULL;
            pGroup->mnEmptyLines = 1;
            if ( pPrevGroup )
                pPrevGroup->mpNext = pGroup;
            else
                mpData->mpFirstGroup = pGroup;
        }

        // Always inherit group names and upate cache members
        pGroup->maGroupName             = maGroupName;
        ((Config*)this)->mnDataUpdateId = mpData->mnDataUpdateId;
        ((Config*)this)->mpActGroup     = pGroup;
    }

    return mpActGroup;
}

Config::Config( const rtl::OUString& rFileName )
{
    // Initialize config data
    maFileName      = toUncPath( rFileName );
    mpData          = ImplGetConfigData( maFileName );
    mpActGroup      = NULL;
    mnDataUpdateId  = 0;
    mnLockCount     = 1;
    mbPersistence   = sal_True;

#ifdef DBG_UTIL
    rtl::OStringBuffer aTraceStr(
        RTL_CONSTASCII_STRINGPARAM("Config::Config( "));
    aTraceStr.append(rtl::OUStringToOString(maFileName, RTL_TEXTENCODING_UTF8));
    aTraceStr.append(RTL_CONSTASCII_STRINGPARAM(" )"));
    OSL_TRACE("%s", aTraceStr.getStr());
#endif
}

Config::~Config()
{
#ifdef DBG_UTIL
    OSL_TRACE( "Config::~Config()" );
#endif

    Flush();
    ImplFreeConfigData( mpData );
}

void Config::SetGroup(const rtl::OString& rGroup)
{
    // If group is to be reset, it needs to be updated on next call
    if ( maGroupName != rGroup )
    {
        maGroupName     = rGroup;
        mnDataUpdateId  = mpData->mnDataUpdateId-1;
    }
}

void Config::DeleteGroup(const rtl::OString& rGroup)
{
    // Update config data if necessary
    if ( !mnLockCount || !mpData->mbRead )
    {
        ImplUpdateConfig();
        mpData->mbRead = sal_True;
    }

    ImplGroupData* pPrevGroup = NULL;
    ImplGroupData* pGroup = mpData->mpFirstGroup;
    while ( pGroup )
    {
        if ( pGroup->maGroupName.equalsIgnoreAsciiCase(rGroup) )
            break;

        pPrevGroup = pGroup;
        pGroup = pGroup->mpNext;
    }

    if ( pGroup )
    {
        // Remove all keys
        ImplKeyData* pTempKey;
        ImplKeyData* pKey = pGroup->mpFirstKey;
        while ( pKey )
        {
            pTempKey = pKey->mpNext;
            delete pKey;
            pKey = pTempKey;
        }

        // Rewire pointers and remove group
        if ( pPrevGroup )
            pPrevGroup->mpNext = pGroup->mpNext;
        else
            mpData->mpFirstGroup = pGroup->mpNext;
        delete pGroup;

        // Rewrite config data
        if ( !mnLockCount && mbPersistence )
            ImplWriteConfig( mpData );
        else
        {
            mpData->mbModified = sal_True;
        }

        mnDataUpdateId = mpData->mnDataUpdateId;
        mpData->mnDataUpdateId++;
    }
}

rtl::OString Config::GetGroupName(sal_uInt16 nGroup) const
{
    // Update config data if necessary
    if ( !mnLockCount )
        ImplUpdateConfig();

    ImplGroupData*  pGroup = mpData->mpFirstGroup;
    sal_uInt16          nGroupCount = 0;
    rtl::OString aGroupName;
    while ( pGroup )
    {
        if ( nGroup == nGroupCount )
        {
            aGroupName = pGroup->maGroupName;
            break;
        }

        nGroupCount++;
        pGroup = pGroup->mpNext;
    }

    return aGroupName;
}

sal_uInt16 Config::GetGroupCount() const
{
    // Update config data if necessary
    if ( !mnLockCount )
        ImplUpdateConfig();

    ImplGroupData*  pGroup = mpData->mpFirstGroup;
    sal_uInt16          nGroupCount = 0;
    while ( pGroup )
    {
        nGroupCount++;
        pGroup = pGroup->mpNext;
    }

    return nGroupCount;
}

sal_Bool Config::HasGroup(const rtl::OString& rGroup) const
{
    // Update config data if necessary
    if ( !mnLockCount )
        ImplUpdateConfig();

    ImplGroupData*  pGroup = mpData->mpFirstGroup;
    sal_Bool            bRet = sal_False;

    while( pGroup )
    {
        if( pGroup->maGroupName.equalsIgnoreAsciiCase(rGroup) )
        {
            bRet = sal_True;
            break;
        }

        pGroup = pGroup->mpNext;
    }

    return bRet;
}

rtl::OString Config::ReadKey(const rtl::OString& rKey) const
{
    return ReadKey(rKey, rtl::OString());
}

rtl::OUString Config::ReadKey(const rtl::OString& rKey, rtl_TextEncoding eEncoding) const
{
    if ( mpData->mbIsUTF8BOM )
        eEncoding = RTL_TEXTENCODING_UTF8;
    return rtl::OStringToOUString(ReadKey(rKey), eEncoding);
}

rtl::OString Config::ReadKey(const rtl::OString& rKey, const rtl::OString& rDefault) const
{
#ifdef DBG_UTIL
    rtl::OStringBuffer aTraceStr(
        RTL_CONSTASCII_STRINGPARAM("Config::ReadKey( "));
    aTraceStr.append(rKey);
    aTraceStr.append(RTL_CONSTASCII_STRINGPARAM(" ) from "));
    aTraceStr.append(GetGroup());
    aTraceStr.append(RTL_CONSTASCII_STRINGPARAM(" in "));
    aTraceStr.append(rtl::OUStringToOString(maFileName, RTL_TEXTENCODING_UTF8));
    OSL_TRACE("%s", aTraceStr.getStr());
#endif

    // Update config data if necessary
    if ( !mnLockCount )
        ImplUpdateConfig();

    // Search key, return value if found
    ImplGroupData* pGroup = ImplGetGroup();
    if ( pGroup )
    {
        ImplKeyData* pKey = pGroup->mpFirstKey;
        while ( pKey )
        {
            if ( !pKey->mbIsComment && pKey->maKey.equalsIgnoreAsciiCase(rKey) )
                return pKey->maValue;

            pKey = pKey->mpNext;
        }
    }

    return rDefault;
}

void Config::WriteKey(const rtl::OString& rKey, const rtl::OString& rStr)
{
#ifdef DBG_UTIL
    rtl::OStringBuffer aTraceStr(RTL_CONSTASCII_STRINGPARAM("Config::WriteKey( "));
    aTraceStr.append(rKey);
    aTraceStr.append(RTL_CONSTASCII_STRINGPARAM(", "));
    aTraceStr.append(rStr);
    aTraceStr.append(RTL_CONSTASCII_STRINGPARAM(" ) to "));
    aTraceStr.append(GetGroup());
    aTraceStr.append(RTL_CONSTASCII_STRINGPARAM(" in "));
    aTraceStr.append(rtl::OUStringToOString(maFileName, RTL_TEXTENCODING_UTF8));
    OSL_TRACE("%s", aTraceStr.getStr());
#endif

    // Update config data if necessary
    if ( !mnLockCount || !mpData->mbRead )
    {
        ImplUpdateConfig();
        mpData->mbRead = sal_True;
    }

    // Search key and update value if found
    ImplGroupData* pGroup = ImplGetGroup();
    if ( pGroup )
    {
        ImplKeyData* pPrevKey = NULL;
        ImplKeyData* pKey = pGroup->mpFirstKey;
        while ( pKey )
        {
            if ( !pKey->mbIsComment && pKey->maKey.equalsIgnoreAsciiCase(rKey) )
                break;

            pPrevKey = pKey;
            pKey = pKey->mpNext;
        }

        sal_Bool bNewValue;
        if ( !pKey )
        {
            pKey              = new ImplKeyData;
            pKey->mpNext      = NULL;
            pKey->maKey       = rKey;
            pKey->mbIsComment = sal_False;
            if ( pPrevKey )
                pPrevKey->mpNext = pKey;
            else
                pGroup->mpFirstKey = pKey;
            bNewValue = sal_True;
        }
        else
            bNewValue = pKey->maValue != rStr;

        if ( bNewValue )
        {
            pKey->maValue = rStr;

            if ( !mnLockCount && mbPersistence )
                ImplWriteConfig( mpData );
            else
            {
                mpData->mbModified = sal_True;
            }
        }
    }
}

void Config::DeleteKey(const rtl::OString& rKey)
{
    // Update config data if necessary
    if ( !mnLockCount || !mpData->mbRead )
    {
        ImplUpdateConfig();
        mpData->mbRead = sal_True;
    }

    // Search key and update value
    ImplGroupData* pGroup = ImplGetGroup();
    if ( pGroup )
    {
        ImplKeyData* pPrevKey = NULL;
        ImplKeyData* pKey = pGroup->mpFirstKey;
        while ( pKey )
        {
            if ( !pKey->mbIsComment && pKey->maKey.equalsIgnoreAsciiCase(rKey) )
                break;

            pPrevKey = pKey;
            pKey = pKey->mpNext;
        }

        if ( pKey )
        {
            // Rewire group pointers and delete
            if ( pPrevKey )
                pPrevKey->mpNext = pKey->mpNext;
            else
                pGroup->mpFirstKey = pKey->mpNext;
            delete pKey;

            // Rewrite config file
            if ( !mnLockCount && mbPersistence )
                ImplWriteConfig( mpData );
            else
            {
                mpData->mbModified = sal_True;
            }
        }
    }
}

sal_uInt16 Config::GetKeyCount() const
{
#ifdef DBG_UTIL
    rtl::OStringBuffer aTraceStr(
        RTL_CONSTASCII_STRINGPARAM("Config::GetKeyCount()"));
    aTraceStr.append(RTL_CONSTASCII_STRINGPARAM(" from "));
    aTraceStr.append(GetGroup());
    aTraceStr.append(RTL_CONSTASCII_STRINGPARAM(" in "));
    aTraceStr.append(rtl::OUStringToOString(maFileName, RTL_TEXTENCODING_UTF8));
    OSL_TRACE("%s", aTraceStr.getStr());
#endif

    // Update config data if necessary
    if ( !mnLockCount )
        ImplUpdateConfig();

    // Search key and update value
    sal_uInt16 nCount = 0;
    ImplGroupData* pGroup = ImplGetGroup();
    if ( pGroup )
    {
        ImplKeyData* pKey = pGroup->mpFirstKey;
        while ( pKey )
        {
            if ( !pKey->mbIsComment )
                nCount++;

            pKey = pKey->mpNext;
        }
    }

    return nCount;
}

rtl::OString Config::GetKeyName(sal_uInt16 nKey) const
{
#ifdef DBG_UTIL
    rtl::OStringBuffer aTraceStr(
        RTL_CONSTASCII_STRINGPARAM("Config::GetKeyName( "));
    aTraceStr.append(static_cast<sal_Int32>(nKey));
    aTraceStr.append(RTL_CONSTASCII_STRINGPARAM(" ) from "));
    aTraceStr.append(GetGroup());
    aTraceStr.append(RTL_CONSTASCII_STRINGPARAM(" in "));
    aTraceStr.append(rtl::OUStringToOString(
        maFileName, RTL_TEXTENCODING_UTF8));
    OSL_TRACE("%s", aTraceStr.getStr());
#endif

    // search key and return name if found
    ImplGroupData* pGroup = ImplGetGroup();
    if ( pGroup )
    {
        ImplKeyData* pKey = pGroup->mpFirstKey;
        while ( pKey )
        {
            if ( !pKey->mbIsComment )
            {
                if ( !nKey )
                    return pKey->maKey;
                nKey--;
            }

            pKey = pKey->mpNext;
        }
    }

    return rtl::OString();
}

rtl::OString Config::ReadKey(sal_uInt16 nKey) const
{
#ifdef DBG_UTIL
    rtl::OStringBuffer aTraceStr(
        RTL_CONSTASCII_STRINGPARAM("Config::ReadKey( "));
    aTraceStr.append(static_cast<sal_Int32>(nKey));
    aTraceStr.append(RTL_CONSTASCII_STRINGPARAM(" ) from "));
    aTraceStr.append(GetGroup());
    aTraceStr.append(RTL_CONSTASCII_STRINGPARAM(" in "));
    aTraceStr.append(rtl::OUStringToOString(maFileName,
        RTL_TEXTENCODING_UTF8));
    OSL_TRACE("%s", aTraceStr.getStr());
#endif

    // Search key and return value if found
    ImplGroupData* pGroup = ImplGetGroup();
    if ( pGroup )
    {
        ImplKeyData* pKey = pGroup->mpFirstKey;
        while ( pKey )
        {
            if ( !pKey->mbIsComment )
            {
                if ( !nKey )
                    return pKey->maValue;
                nKey--;
            }

            pKey = pKey->mpNext;
        }
    }

    return rtl::OString();
}

void Config::Flush()
{
    if ( mpData->mbModified && mbPersistence )
        ImplWriteConfig( mpData );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
