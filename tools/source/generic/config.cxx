/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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
    OString         maKey;
    OString         maValue;
    bool            mbIsComment;
};

struct ImplGroupData
{
    ImplGroupData*  mpNext;
    ImplKeyData*    mpFirstKey;
    OString         maGroupName;
    sal_uInt16      mnEmptyLines;
};

struct ImplConfigData
{
    ImplGroupData*  mpFirstGroup;
    OUString        maFileName;
    sal_uIntPtr     mnDataUpdateId;
    sal_uIntPtr     mnTimeStamp;
    LineEnd         meLineEnd;
    sal_uInt16      mnRefCount;
    bool            mbModified;
    bool            mbRead;
    bool            mbIsUTF8BOM;
};

static OUString toUncPath( const OUString& rPath )
{
    OUString aFileURL;

    
    if( rPath.startsWith( "file:
    {
        aFileURL = rPath;
    }
    else if( ::osl::FileBase::getFileURLFromSystemPath( rPath, aFileURL ) != ::osl::FileBase::E_None )
    {
        aFileURL = rPath;
    }
    return aFileURL;
}

static sal_uIntPtr ImplSysGetConfigTimeStamp( const OUString& rFileName )
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

static sal_uInt8* ImplSysReadConfig( const OUString& rFileName,
                                sal_uInt64& rRead, bool& rbRead, bool& rbIsUTF8BOM, sal_uIntPtr& rTimeStamp )
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
                
                unsigned char BOM[3] = {0xEF, 0xBB, 0xBF};
                if (nRead > 2 && memcmp(pBuf, BOM, 3) == 0)
                {
                    nRead -= 3;
                    memmove(pBuf, pBuf + 3, sal::static_int_cast<sal_Size>(nRead * sizeof(sal_uInt8)) );
                    rbIsUTF8BOM = true;
                }

                rTimeStamp = ImplSysGetConfigTimeStamp( rFileName );
                rbRead = true;
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

static bool ImplSysWriteConfig( const OUString& rFileName,
                                const sal_uInt8* pBuf, sal_uIntPtr nBufLen, bool rbIsUTF8BOM, sal_uIntPtr& rTimeStamp )
{
    bool bSuccess = false;
    bool bUTF8BOMSuccess = false;

    ::osl::File aFile( rFileName );
    ::osl::FileBase::RC eError = aFile.open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
    if( eError != ::osl::FileBase::E_None )
        eError = aFile.open( osl_File_OpenFlag_Write );
    if( eError == ::osl::FileBase::E_None )
    {
        
        aFile.setSize( 0 );
        sal_uInt64 nWritten;

        
        if ( rbIsUTF8BOM )
        {
            unsigned char BOM[3] = {0xEF, 0xBB, 0xBF};
            sal_uInt64 nUTF8BOMWritten;
            if( aFile.write( BOM, 3, nUTF8BOMWritten ) == ::osl::FileBase::E_None && 3 == nUTF8BOMWritten )
            {
                bUTF8BOMSuccess = true;
            }
        }

        if( aFile.write( pBuf, nBufLen, nWritten ) == ::osl::FileBase::E_None && nWritten == nBufLen )
        {
            bSuccess = true;
        }
        if ( rbIsUTF8BOM ? bSuccess && bUTF8BOMSuccess : bSuccess )
        {
            rTimeStamp = ImplSysGetConfigTimeStamp( rFileName );
        }
    }

    return rbIsUTF8BOM ? bSuccess && bUTF8BOMSuccess : bSuccess;
}

namespace {
OString makeOString(const sal_uInt8* p, sal_uInt64 n)
{
    if (n > SAL_MAX_INT32)
    {
        #ifdef WNT
        abort();
        #else
        ::std::abort(); 
        #endif
    }
    return OString(
        reinterpret_cast< char const * >(p),
        sal::static_int_cast< sal_Int32 >(n));
}
}

static void ImplMakeConfigList( ImplConfigData* pData,
                                const sal_uInt8* pBuf, sal_uInt64 nLen )
{
    if ( !nLen )
        return;

    
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
        
        if ( pBuf[i] == 0x1A )
            break;

        
        while ( (pBuf[i] == ' ') || (pBuf[i] == '\t') )
            i++;

        
        nStart = i;
        pLine = pBuf+i;

        
        while (  (i < nLen) && pBuf[i] && (pBuf[i] != '\r') && (pBuf[i] != '\n') &&
                (pBuf[i] != 0x1A) )
            i++;

        nLineLen = i-nStart;

        
        if ( (i+1 < nLen) &&
             (pBuf[i] != pBuf[i+1]) &&
             ((pBuf[i+1] == '\r') || (pBuf[i+1] == '\n')) )
            i++;
        i++;

        
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

            
            pLine++;
            nLineLen--;
            
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

                
                if ( pPrevKey )
                {
                    while ( pGroup->mnEmptyLines )
                    {
                        pKey                = new ImplKeyData;
                        pKey->mbIsComment   = true;
                        pPrevKey->mpNext    = pKey;
                        pPrevKey            = pKey;
                        pGroup->mnEmptyLines--;
                    }
                }

                
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
                    pKey->mbIsComment = true;
                }
                else
                {
                    pKey->mbIsComment = false;
                    nNameLen = 0;
                    while ( (nNameLen < nLineLen) && (pLine[nNameLen] != '=') )
                        nNameLen++;
                    nKeyLen = nNameLen;
                    
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
        aLineEndBuf[0] = '\r';
        nLineEndLen = 1;
    }
    else if ( pData->meLineEnd == LINEEND_LF )
    {
        aLineEndBuf[0] = '\n';
        nLineEndLen = 1;
    }
    else
    {
        aLineEndBuf[0] = '\r';
        aLineEndBuf[1] = '\n';
        nLineEndLen = 2;
    }

    nBufLen = 0;
    pGroup = pData->mpFirstGroup;
    while ( pGroup )
    {
        
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

            
            if ( !pGroup->mnEmptyLines )
                pGroup->mnEmptyLines = 1;
            nBufLen += nLineEndLen * pGroup->mnEmptyLines;
        }

        pGroup = pGroup->mpNext;
    }

    
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

    
    pWriteBuf = new sal_uInt8[nBufLen];
    if ( !pWriteBuf )
        return 0;

    
    pBuf = pWriteBuf;
    pGroup = pData->mpFirstGroup;
    while ( pGroup )
    {
        
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
    bool    bRead = false;
    bool    bIsUTF8BOM = false;
    sal_uInt8*  pBuf = ImplSysReadConfig( pData->maFileName, nRead, bRead, bIsUTF8BOM, nTimeStamp );

    
    if ( pBuf )
    {
        ImplMakeConfigList( pData, pBuf, nRead );
        delete[] pBuf;
    }
    pData->mnTimeStamp = nTimeStamp;
    pData->mbModified  = false;
    if ( bRead )
        pData->mbRead = true;
    if ( bIsUTF8BOM )
        pData->mbIsUTF8BOM = true;
}

static void ImplWriteConfig( ImplConfigData* pData )
{
    SAL_WARN_IF( pData->mnTimeStamp != ImplSysGetConfigTimeStamp( pData->maFileName ),
        "tools.generic", "Config overwrites modified configfile: " << pData->maFileName );

    
    sal_uIntPtr nBufLen;
    sal_uInt8*  pBuf = ImplGetConfigBuffer( pData, nBufLen );
    if ( pBuf )
    {
        if ( ImplSysWriteConfig( pData->maFileName, pBuf, nBufLen, pData->mbIsUTF8BOM, pData->mnTimeStamp ) )
            pData->mbModified = false;
        delete[] pBuf;
    }
    else
        pData->mbModified = false;
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

        
        pKey = pGroup->mpFirstKey;
        while ( pKey )
        {
            pTempKey = pKey->mpNext;
            delete pKey;
            pKey = pTempKey;
        }

        
        delete pGroup;
        pGroup = pTempGroup;
    }

    pData->mpFirstGroup = NULL;
}

static ImplConfigData* ImplGetConfigData( const OUString& rFileName )
{
    ImplConfigData* pData;

    pData                   = new ImplConfigData;
    pData->maFileName       = rFileName;
    pData->mpFirstGroup     = NULL;
    pData->mnDataUpdateId   = 0;
    pData->meLineEnd        = LINEEND_CRLF;
    pData->mnRefCount       = 0;
    pData->mbRead           = false;
    pData->mbIsUTF8BOM      = false;
    ImplReadConfig( pData );

    return pData;
}

static void ImplFreeConfigData( ImplConfigData* pDelData )
{
    ImplDeleteConfigData( pDelData );
    delete pDelData;
}

bool Config::ImplUpdateConfig() const
{
    
    if ( mpData->mnTimeStamp != ImplSysGetConfigTimeStamp( maFileName ) )
    {
        ImplDeleteConfigData( mpData );
        ImplReadConfig( mpData );
        mpData->mnDataUpdateId++;
        return true;
    }
    else
        return false;
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

        
        pGroup->maGroupName             = maGroupName;
        ((Config*)this)->mnDataUpdateId = mpData->mnDataUpdateId;
        ((Config*)this)->mpActGroup     = pGroup;
    }

    return mpActGroup;
}

Config::Config( const OUString& rFileName )
{
    
    maFileName      = toUncPath( rFileName );
    mpData          = ImplGetConfigData( maFileName );
    mpActGroup      = NULL;
    mnDataUpdateId  = 0;
    mnLockCount     = 1;
    mbPersistence   = true;

#ifdef DBG_UTIL
    OStringBuffer aTraceStr("Config::Config( ");
    aTraceStr.append(OUStringToOString(maFileName, RTL_TEXTENCODING_UTF8));
    aTraceStr.append(" )");
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

void Config::SetGroup(const OString& rGroup)
{
    
    if ( maGroupName != rGroup )
    {
        maGroupName     = rGroup;
        mnDataUpdateId  = mpData->mnDataUpdateId-1;
    }
}

void Config::DeleteGroup(const OString& rGroup)
{
    
    if ( !mnLockCount || !mpData->mbRead )
    {
        ImplUpdateConfig();
        mpData->mbRead = true;
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
        
        ImplKeyData* pTempKey;
        ImplKeyData* pKey = pGroup->mpFirstKey;
        while ( pKey )
        {
            pTempKey = pKey->mpNext;
            delete pKey;
            pKey = pTempKey;
        }

        
        if ( pPrevGroup )
            pPrevGroup->mpNext = pGroup->mpNext;
        else
            mpData->mpFirstGroup = pGroup->mpNext;
        delete pGroup;

        
        if ( !mnLockCount && mbPersistence )
            ImplWriteConfig( mpData );
        else
        {
            mpData->mbModified = true;
        }

        mnDataUpdateId = mpData->mnDataUpdateId;
        mpData->mnDataUpdateId++;
    }
}

OString Config::GetGroupName(sal_uInt16 nGroup) const
{
    
    if ( !mnLockCount )
        ImplUpdateConfig();

    ImplGroupData*  pGroup = mpData->mpFirstGroup;
    sal_uInt16          nGroupCount = 0;
    OString aGroupName;
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

bool Config::HasGroup(const OString& rGroup) const
{
    
    if ( !mnLockCount )
        ImplUpdateConfig();

    ImplGroupData*  pGroup = mpData->mpFirstGroup;
    bool            bRet = false;

    while( pGroup )
    {
        if( pGroup->maGroupName.equalsIgnoreAsciiCase(rGroup) )
        {
            bRet = true;
            break;
        }

        pGroup = pGroup->mpNext;
    }

    return bRet;
}

OString Config::ReadKey(const OString& rKey) const
{
    return ReadKey(rKey, OString());
}

OUString Config::ReadKey(const OString& rKey, rtl_TextEncoding eEncoding) const
{
    if ( mpData->mbIsUTF8BOM )
        eEncoding = RTL_TEXTENCODING_UTF8;
    return OStringToOUString(ReadKey(rKey), eEncoding);
}

OString Config::ReadKey(const OString& rKey, const OString& rDefault) const
{
#ifdef DBG_UTIL
    OStringBuffer aTraceStr("Config::ReadKey( ");
    aTraceStr.append(rKey);
    aTraceStr.append(" ) from ");
    aTraceStr.append(GetGroup());
    aTraceStr.append(" in ");
    aTraceStr.append(OUStringToOString(maFileName, RTL_TEXTENCODING_UTF8));
    OSL_TRACE("%s", aTraceStr.getStr());
#endif

    
    if ( !mnLockCount )
        ImplUpdateConfig();

    
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

void Config::WriteKey(const OString& rKey, const OString& rStr)
{
#ifdef DBG_UTIL
    OStringBuffer aTraceStr("Config::WriteKey( ");
    aTraceStr.append(rKey);
    aTraceStr.append(", ");
    aTraceStr.append(rStr);
    aTraceStr.append(" ) to ");
    aTraceStr.append(GetGroup());
    aTraceStr.append(" in ");
    aTraceStr.append(OUStringToOString(maFileName, RTL_TEXTENCODING_UTF8));
    OSL_TRACE("%s", aTraceStr.getStr());
#endif

    
    if ( !mnLockCount || !mpData->mbRead )
    {
        ImplUpdateConfig();
        mpData->mbRead = true;
    }

    
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

        bool bNewValue;
        if ( !pKey )
        {
            pKey              = new ImplKeyData;
            pKey->mpNext      = NULL;
            pKey->maKey       = rKey;
            pKey->mbIsComment = false;
            if ( pPrevKey )
                pPrevKey->mpNext = pKey;
            else
                pGroup->mpFirstKey = pKey;
            bNewValue = true;
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
                mpData->mbModified = true;
            }
        }
    }
}

void Config::DeleteKey(const OString& rKey)
{
    
    if ( !mnLockCount || !mpData->mbRead )
    {
        ImplUpdateConfig();
        mpData->mbRead = true;
    }

    
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
            
            if ( pPrevKey )
                pPrevKey->mpNext = pKey->mpNext;
            else
                pGroup->mpFirstKey = pKey->mpNext;
            delete pKey;

            
            if ( !mnLockCount && mbPersistence )
                ImplWriteConfig( mpData );
            else
            {
                mpData->mbModified = true;
            }
        }
    }
}

sal_uInt16 Config::GetKeyCount() const
{
#ifdef DBG_UTIL
    OStringBuffer aTraceStr("Config::GetKeyCount()");
    aTraceStr.append(" from ");
    aTraceStr.append(GetGroup());
    aTraceStr.append(" in ");
    aTraceStr.append(OUStringToOString(maFileName, RTL_TEXTENCODING_UTF8));
    OSL_TRACE("%s", aTraceStr.getStr());
#endif

    
    if ( !mnLockCount )
        ImplUpdateConfig();

    
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

OString Config::GetKeyName(sal_uInt16 nKey) const
{
#ifdef DBG_UTIL
    OStringBuffer aTraceStr("Config::GetKeyName( ");
    aTraceStr.append(static_cast<sal_Int32>(nKey));
    aTraceStr.append(" ) from ");
    aTraceStr.append(GetGroup());
    aTraceStr.append(" in ");
    aTraceStr.append(OUStringToOString(
        maFileName, RTL_TEXTENCODING_UTF8));
    OSL_TRACE("%s", aTraceStr.getStr());
#endif

    
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

    return OString();
}

OString Config::ReadKey(sal_uInt16 nKey) const
{
#ifdef DBG_UTIL
    OStringBuffer aTraceStr("Config::ReadKey( ");
    aTraceStr.append(static_cast<sal_Int32>(nKey));
    aTraceStr.append(" ) from ");
    aTraceStr.append(GetGroup());
    aTraceStr.append(" in ");
    aTraceStr.append(OUStringToOString(maFileName,
        RTL_TEXTENCODING_UTF8));
    OSL_TRACE("%s", aTraceStr.getStr());
#endif

    
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

    return OString();
}

void Config::Flush()
{
    if ( mpData->mbModified && mbPersistence )
        ImplWriteConfig( mpData );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
