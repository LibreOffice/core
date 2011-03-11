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
#include "precompiled_tools.hxx"

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

#define MAXBUFLEN   1024        // Fuer Buffer bei VOS-Funktionen

// -----------------
// - ImplConfigData -
// -----------------

struct ImplKeyData
{
    ImplKeyData*    mpNext;
    ByteString      maKey;
    ByteString      maValue;
    sal_Bool            mbIsComment;
};

struct ImplGroupData
{
    ImplGroupData*  mpNext;
    ImplKeyData*    mpFirstKey;
    ByteString      maGroupName;
    sal_uInt16          mnEmptyLines;
};

struct ImplConfigData
{
    ImplGroupData*  mpFirstGroup;
    XubString       maFileName;
    sal_uIntPtr         mnDataUpdateId;
    sal_uIntPtr         mnTimeStamp;
    LineEnd         meLineEnd;
    sal_uInt16          mnRefCount;
    sal_Bool            mbModified;
    sal_Bool            mbRead;
    sal_Bool            mbIsUTF8BOM;
};

// =======================================================================

static ByteString& getEmptyByteString()
{
    static ByteString aEmpty;
    return aEmpty;
}

// =======================================================================

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

static sal_uIntPtr ImplSysGetConfigTimeStamp( const XubString& rFileName )
{
    sal_uIntPtr nTimeStamp = 0;
    ::osl::DirectoryItem aItem;
    ::osl::FileStatus aStatus( osl_FileStatus_Mask_ModifyTime );

    int nError = 0;
    if( ( nError = ::osl::DirectoryItem::get( rFileName, aItem ) ) == ::osl::FileBase::E_None &&
        aItem.getFileStatus( aStatus ) == ::osl::FileBase::E_None )
    {
        nTimeStamp = aStatus.getModifyTime().Seconds;
    }

    return nTimeStamp;
}

// -----------------------------------------------------------------------

static sal_uInt8* ImplSysReadConfig( const XubString& rFileName,
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
                    rtl_moveMemory(pBuf, pBuf + 3, sal::static_int_cast<sal_Size>(nRead * sizeof(sal_uInt8)) );
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

// -----------------------------------------------------------------------

static sal_Bool ImplSysWriteConfig( const XubString& rFileName,
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

// -----------------------------------------------------------------------

static String ImplMakeConfigName( const XubString* pFileName,
                                  const XubString* pPathName )
{
    ::rtl::OUString aFileName;
    ::rtl::OUString aPathName;
    if ( pFileName )
    {
#ifdef UNX
        aFileName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "." ));
        aFileName += *pFileName;
        aFileName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "rc" ));
#else
        aFileName = *pFileName;
        aFileName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".ini" ));
#endif
    }
    else
    {
#ifdef UNX
        aFileName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".sversionrc" ));
#else
        aFileName = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "sversion.ini" ));
#endif
    }

    // #88208# in case pPathName is set but empty and pFileName is set
    // and not empty just return the filename; on the default case
    // prepend default path as usual
    if ( pPathName && pPathName->Len() )
        aPathName = toUncPath( *pPathName );
    else if( pPathName && pFileName && pFileName->Len() )
        return aFileName;
    else
    {
        oslSecurity aSec = osl_getCurrentSecurity();
        osl_getConfigDir( aSec, &aPathName.pData );
        osl_freeSecurityHandle( aSec );
    }

    ::rtl::OUString aName( aPathName );
    aName += ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "/" ));
    aName += aFileName;

    return aName;
}

// -----------------------------------------------------------------------

namespace {

ByteString makeByteString(sal_uInt8 const * p, sal_uInt64 n) {
    if (n > STRING_MAXLEN) {
        #ifdef WNT
        abort();
        #else
        ::std::abort(); //TODO: handle this gracefully
        #endif
    }
    return ByteString(
        reinterpret_cast< char const * >(p),
        sal::static_int_cast< xub_StrLen >(n));
}

}

static void ImplMakeConfigList( ImplConfigData* pData,
                                const sal_uInt8* pBuf, sal_uInt64 nLen )
{
    // kein Buffer, keine Daten
    if ( !nLen )
        return;

    // Buffer parsen und Liste zusammenbauen
    sal_uInt64 nStart;
    sal_uInt64 nLineLen;
    xub_StrLen      nNameLen;
    xub_StrLen      nKeyLen;
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

        // Spaces und Tabs entfernen
        while ( (pBuf[i] == ' ') || (pBuf[i] == '\t') )
            i++;

        // Zeilenanfang merken
        nStart = i;
        pLine = pBuf+i;

        // Zeilenende suchen
        while (  (i < nLen) && pBuf[i] && (pBuf[i] != '\r') && (pBuf[i] != '\n') &&
                (pBuf[i] != 0x1A) )
            i++;

        nLineLen = i-nStart;

        // Wenn Zeilenende (CR/LF), dann noch einen weiterschalten
        if ( (i+1 < nLen) &&
             (pBuf[i] != pBuf[i+1]) &&
             ((pBuf[i+1] == '\r') || (pBuf[i+1] == '\n')) )
            i++;
        i++;

        // Zeile auswerten
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

            // Gruppennamen rausfiltern
            pLine++;
            nLineLen--;
            // Spaces und Tabs entfernen
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
            pGroup->maGroupName = ByteString( (const sal_Char*)pLine, nNameLen );
        }
        else
        {
            if ( nLineLen )
            {
                // Wenn noch keine Gruppe existiert, dann alle Keys in die
                // Default-Gruppe
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

                // Falls Leerzeile vorhanden, dann anhaengen
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

                // Neuen Key erzeugen
                pKey        = new ImplKeyData;
                pKey->mpNext = NULL;
                if ( pPrevKey )
                    pPrevKey->mpNext = pKey;
                else
                    pGroup->mpFirstKey = pKey;
                pPrevKey = pKey;
                if ( pLine[0] == ';' )
                {
                    pKey->maValue = makeByteString(pLine, nLineLen);
                    pKey->mbIsComment = sal_True;
                }
                else
                {
                    pKey->mbIsComment = sal_False;
                    nNameLen = 0;
                    while ( (nNameLen < nLineLen) && (pLine[nNameLen] != '=') )
                        nNameLen++;
                    nKeyLen = nNameLen;
                    // Spaces und Tabs entfernen
                    if ( nNameLen )
                    {
                        while ( (pLine[nNameLen-1] == ' ') || (pLine[nNameLen-1] == '\t') )
                            nNameLen--;
                    }
                    pKey->maKey = ByteString( (const sal_Char*)pLine, nNameLen );
                    nKeyLen++;
                    if ( nKeyLen < nLineLen )
                    {
                        pLine += nKeyLen;
                        nLineLen -= nKeyLen;
                        // Spaces und Tabs entfernen
                        while ( (*pLine == ' ') || (*pLine == '\t') )
                        {
                            nLineLen--;
                            pLine++;
                        }
                        if ( nLineLen )
                        {
                            while ( (pLine[nLineLen-1] == ' ') || (pLine[nLineLen-1] == '\t') )
                                nLineLen--;
                            pKey->maValue = makeByteString(pLine, nLineLen);
                        }
                    }
                }
            }
            else
            {
                // Leerzeilen werden nur gezaehlt und beim Erzeugen des
                // naechsten Keys angehaengt, da wir Leerzeilen am Ende
                // einer Gruppe auch nach hinzufuegen von neuen Keys nur
                // am Ende der Gruppe wieder speichern wollen
                if ( pGroup )
                    pGroup->mnEmptyLines++;
            }
        }
    }
}

// -----------------------------------------------------------------------

static sal_uInt8* ImplGetConfigBuffer( const ImplConfigData* pData, sal_uIntPtr& rLen )
{
    sal_uInt8*          pWriteBuf;
    sal_uInt8*          pBuf;
    sal_uInt8           aLineEndBuf[2] = {0, 0};
    ImplKeyData*    pKey;
    ImplGroupData*  pGroup;
    unsigned int    nBufLen;
    sal_uInt16          nValueLen;
    sal_uInt16          nKeyLen;
    sal_uInt16          nLineEndLen;

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

    // Buffergroesse ermitteln
    nBufLen = 0;
    pGroup = pData->mpFirstGroup;
    while ( pGroup )
    {
        // Leere Gruppen werden nicht geschrieben
        if ( pGroup->mpFirstKey )
        {
            nBufLen += pGroup->maGroupName.Len() + nLineEndLen + 2;
            pKey = pGroup->mpFirstKey;
            while ( pKey )
            {
                nValueLen = pKey->maValue.Len();
                if ( pKey->mbIsComment )
                    nBufLen += nValueLen + nLineEndLen;
                else
                    nBufLen += pKey->maKey.Len() + nValueLen + nLineEndLen + 1;

                pKey = pKey->mpNext;
            }

            // Leerzeile nach jeder Gruppe auch wieder speichern
            if ( !pGroup->mnEmptyLines )
                pGroup->mnEmptyLines = 1;
            nBufLen += nLineEndLen * pGroup->mnEmptyLines;
        }

        pGroup = pGroup->mpNext;
    }

    // Laenge dem Aufrufer mitteilen
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

    // Schreibbuffer anlegen (wird vom Aufrufer zerstoert)
    pWriteBuf = new sal_uInt8[nBufLen];
    if ( !pWriteBuf )
        return 0;

    // Buffer fuellen
    pBuf = pWriteBuf;
    pGroup = pData->mpFirstGroup;
    while ( pGroup )
    {
        // Leere Gruppen werden nicht geschrieben
        if ( pGroup->mpFirstKey )
        {
            *pBuf = '[';    pBuf++;
            memcpy( pBuf, pGroup->maGroupName.GetBuffer(), pGroup->maGroupName.Len() );
            pBuf += pGroup->maGroupName.Len();
            *pBuf = ']';    pBuf++;
            *pBuf = aLineEndBuf[0]; pBuf++;
            if ( nLineEndLen == 2 )
            {
                *pBuf = aLineEndBuf[1]; pBuf++;
            }
            pKey = pGroup->mpFirstKey;
            while ( pKey )
            {
                nValueLen = pKey->maValue.Len();
                if ( pKey->mbIsComment )
                {
                    if ( nValueLen )
                    {
                        memcpy( pBuf, pKey->maValue.GetBuffer(), nValueLen );
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
                    nKeyLen = pKey->maKey.Len();
                    memcpy( pBuf, pKey->maKey.GetBuffer(), nKeyLen );
                    pBuf += nKeyLen;
                    *pBuf = '=';    pBuf++;
                    memcpy( pBuf, pKey->maValue.GetBuffer(), nValueLen );
                    pBuf += nValueLen;
                    *pBuf = aLineEndBuf[0]; pBuf++;
                    if ( nLineEndLen == 2 )
                    {
                        *pBuf = aLineEndBuf[1]; pBuf++;
                    }
                }

                pKey = pKey->mpNext;
            }

            // Leerzeile nach jeder Gruppe auch wieder speichern
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

// -----------------------------------------------------------------------

static void ImplReadConfig( ImplConfigData* pData )
{
    sal_uIntPtr         nTimeStamp = 0;
    sal_uInt64 nRead = 0;
    sal_Bool            bRead = sal_False;
    sal_Bool                bIsUTF8BOM =sal_False;
    sal_uInt8*          pBuf = ImplSysReadConfig( pData->maFileName, nRead, bRead, bIsUTF8BOM, nTimeStamp );

    // Aus dem Buffer die Config-Verwaltungsliste aufbauen
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

// -----------------------------------------------------------------------

static void ImplWriteConfig( ImplConfigData* pData )
{
#ifdef DBG_UTIL
    if ( DbgIsAssert() )
    {
        if ( pData->mnTimeStamp != ImplSysGetConfigTimeStamp( pData->maFileName ) )
        {
            DBG_ERROR1( "Config overwrites modified configfile:\n %s", ByteString( pData->maFileName, RTL_TEXTENCODING_UTF8 ).GetBuffer() );
        }
    }
#endif

    // Aus der Config-Liste einen Buffer zusammenbauen
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

// -----------------------------------------------------------------------

static void ImplDeleteConfigData( ImplConfigData* pData )
{
    ImplKeyData*    pTempKey;
    ImplKeyData*    pKey;
    ImplGroupData*  pTempGroup;
    ImplGroupData*  pGroup = pData->mpFirstGroup;
    while ( pGroup )
    {
        pTempGroup = pGroup->mpNext;

        // Alle Keys loeschen
        pKey = pGroup->mpFirstKey;
        while ( pKey )
        {
            pTempKey = pKey->mpNext;
            delete pKey;
            pKey = pTempKey;
        }

        // Gruppe loeschen und weiterschalten
        delete pGroup;
        pGroup = pTempGroup;
    }

    pData->mpFirstGroup = NULL;
}

// =======================================================================

static ImplConfigData* ImplGetConfigData( const XubString& rFileName )
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

// -----------------------------------------------------------------------

static void ImplFreeConfigData( ImplConfigData* pDelData )
{
    ImplDeleteConfigData( pDelData );
    delete pDelData;
}

// =======================================================================

sal_Bool Config::ImplUpdateConfig() const
{
    // Wenn sich TimeStamp unterscheidet, dann Datei neu einlesen
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

// -----------------------------------------------------------------------

ImplGroupData* Config::ImplGetGroup() const
{
    if ( !mpActGroup || (mnDataUpdateId != mpData->mnDataUpdateId) )
    {
        ImplGroupData* pPrevGroup = NULL;
        ImplGroupData* pGroup = mpData->mpFirstGroup;
        while ( pGroup )
        {
            if ( pGroup->maGroupName.EqualsIgnoreCaseAscii( maGroupName ) )
                break;

            pPrevGroup = pGroup;
            pGroup = pGroup->mpNext;
        }

        // Falls Gruppe noch nicht existiert, dann dazufuegen
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

        // Gruppenname immer uebernehmen, da er auch in dieser Form
        // geschrieben werden soll. Ausserdem die Cache-Members der
        // Config-Klasse updaten
        pGroup->maGroupName             = maGroupName;
        ((Config*)this)->mnDataUpdateId = mpData->mnDataUpdateId;
        ((Config*)this)->mpActGroup     = pGroup;
    }

    return mpActGroup;
}

// =======================================================================

Config::Config()
{
    // Daten initialisieren und einlesen
    maFileName      = ImplMakeConfigName( NULL, NULL );
    mpData          = ImplGetConfigData( maFileName );
    mpActGroup      = NULL;
    mnDataUpdateId  = 0;
    mnLockCount     = 1;
    mbPersistence   = sal_True;

#ifdef DBG_UTIL
    DBG_TRACE( "Config::Config()" );
#endif
}

// -----------------------------------------------------------------------

Config::Config( const XubString& rFileName )
{
    // Daten initialisieren und einlesen
    maFileName      = toUncPath( rFileName );
    mpData          = ImplGetConfigData( maFileName );
    mpActGroup      = NULL;
    mnDataUpdateId  = 0;
    mnLockCount     = 1;
    mbPersistence   = sal_True;

#ifdef DBG_UTIL
    ByteString aTraceStr( "Config::Config( " );
    aTraceStr += ByteString( maFileName, RTL_TEXTENCODING_UTF8 );
    aTraceStr += " )";
    DBG_TRACE( aTraceStr.GetBuffer() );
#endif
}

// -----------------------------------------------------------------------

Config::~Config()
{
#ifdef DBG_UTIL
    DBG_TRACE( "Config::~Config()" );
#endif

    Flush();
    ImplFreeConfigData( mpData );
}

// -----------------------------------------------------------------------

String Config::GetDefDirectory()
{
    ::rtl::OUString aDefConfig;
    oslSecurity aSec = osl_getCurrentSecurity();
    osl_getConfigDir( aSec, &aDefConfig.pData );
    osl_freeSecurityHandle( aSec );

    return aDefConfig;
}

// -----------------------------------------------------------------------

XubString Config::GetConfigName( const XubString& rPath,
                                 const XubString& rBaseName )
{
    return ImplMakeConfigName( &rBaseName, &rPath );
}

// -----------------------------------------------------------------------

void Config::SetGroup( const ByteString& rGroup )
{
    // Wenn neue Gruppe gesetzt wird, muss beim naechsten mal die
    // Gruppe neu ermittelt werden
    if ( maGroupName != rGroup )
    {
        maGroupName     = rGroup;
        mnDataUpdateId  = mpData->mnDataUpdateId-1;
    }
}

// -----------------------------------------------------------------------

void Config::DeleteGroup( const ByteString& rGroup )
{
    // Config-Daten evt. updaten
    if ( !mnLockCount || !mpData->mbRead )
    {
        ImplUpdateConfig();
        mpData->mbRead = sal_True;
    }

    ImplGroupData* pPrevGroup = NULL;
    ImplGroupData* pGroup = mpData->mpFirstGroup;
    while ( pGroup )
    {
        if ( pGroup->maGroupName.EqualsIgnoreCaseAscii( rGroup ) )
            break;

        pPrevGroup = pGroup;
        pGroup = pGroup->mpNext;
    }

    if ( pGroup )
    {
        // Alle Keys loeschen
        ImplKeyData* pTempKey;
        ImplKeyData* pKey = pGroup->mpFirstKey;
        while ( pKey )
        {
            pTempKey = pKey->mpNext;
            delete pKey;
            pKey = pTempKey;
        }

        // Gruppe weiterschalten und loeschen
        if ( pPrevGroup )
            pPrevGroup->mpNext = pGroup->mpNext;
        else
            mpData->mpFirstGroup = pGroup->mpNext;
        delete pGroup;

        // Config-Datei neu schreiben
        if ( !mnLockCount && mbPersistence )
            ImplWriteConfig( mpData );
        else
        {
            mpData->mbModified = sal_True;
        }

        // Gruppen auf ungluetig setzen
        mnDataUpdateId = mpData->mnDataUpdateId;
        mpData->mnDataUpdateId++;
    }
}

// -----------------------------------------------------------------------

ByteString Config::GetGroupName( sal_uInt16 nGroup ) const
{
    // Config-Daten evt. updaten
    if ( !mnLockCount )
        ImplUpdateConfig();

    ImplGroupData*  pGroup = mpData->mpFirstGroup;
    sal_uInt16          nGroupCount = 0;
    ByteString      aGroupName;
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

// -----------------------------------------------------------------------

sal_uInt16 Config::GetGroupCount() const
{
    // Config-Daten evt. updaten
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

// -----------------------------------------------------------------------

sal_Bool Config::HasGroup( const ByteString& rGroup ) const
{
    // Config-Daten evt. updaten
    if ( !mnLockCount )
        ImplUpdateConfig();

    ImplGroupData*  pGroup = mpData->mpFirstGroup;
    sal_Bool            bRet = sal_False;

    while( pGroup )
    {
        if( pGroup->maGroupName.EqualsIgnoreCaseAscii( rGroup ) )
        {
            bRet = sal_True;
            break;
        }

        pGroup = pGroup->mpNext;
    }

    return bRet;
}

// -----------------------------------------------------------------------

ByteString Config::ReadKey( const ByteString& rKey ) const
{
    return ReadKey( rKey, getEmptyByteString() );
}

// -----------------------------------------------------------------------

UniString Config::ReadKey( const ByteString& rKey, rtl_TextEncoding eEncoding ) const
{
    if ( mpData->mbIsUTF8BOM )
        eEncoding = RTL_TEXTENCODING_UTF8;
    return UniString( ReadKey( rKey ), eEncoding );
}

// -----------------------------------------------------------------------

ByteString Config::ReadKey( const ByteString& rKey, const ByteString& rDefault ) const
{
#ifdef DBG_UTIL
    ByteString aTraceStr( "Config::ReadKey( " );
    aTraceStr += rKey;
    aTraceStr += " ) from ";
    aTraceStr += GetGroup();
    aTraceStr += " in ";
    aTraceStr += ByteString( maFileName, RTL_TEXTENCODING_UTF8 );
    DBG_TRACE( aTraceStr.GetBuffer() );
#endif

    // Config-Daten evt. updaten
    if ( !mnLockCount )
        ImplUpdateConfig();

    // Key suchen und Value zurueckgeben
    ImplGroupData* pGroup = ImplGetGroup();
    if ( pGroup )
    {
        ImplKeyData* pKey = pGroup->mpFirstKey;
        while ( pKey )
        {
            if ( !pKey->mbIsComment && pKey->maKey.EqualsIgnoreCaseAscii( rKey ) )
                return pKey->maValue;

            pKey = pKey->mpNext;
        }
    }

    return rDefault;
}

// -----------------------------------------------------------------------

void Config::WriteKey( const ByteString& rKey, const ByteString& rStr )
{
#ifdef DBG_UTIL
    ByteString aTraceStr( "Config::WriteKey( " );
    aTraceStr += rKey;
    aTraceStr += ", ";
    aTraceStr += rStr;
    aTraceStr += " ) to ";
    aTraceStr += GetGroup();
    aTraceStr += " in ";
    aTraceStr += ByteString( maFileName, RTL_TEXTENCODING_UTF8 );
    DBG_TRACE( aTraceStr.GetBuffer() );
    DBG_ASSERTWARNING( rStr != ReadKey( rKey ), "Config::WriteKey() with the same Value" );
#endif

    // Config-Daten evt. updaten
    if ( !mnLockCount || !mpData->mbRead )
    {
        ImplUpdateConfig();
        mpData->mbRead = sal_True;
    }

    // Key suchen und Value setzen
    ImplGroupData* pGroup = ImplGetGroup();
    if ( pGroup )
    {
        ImplKeyData* pPrevKey = NULL;
        ImplKeyData* pKey = pGroup->mpFirstKey;
        while ( pKey )
        {
            if ( !pKey->mbIsComment && pKey->maKey.EqualsIgnoreCaseAscii( rKey ) )
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

// -----------------------------------------------------------------------

void Config::WriteKey( const ByteString& rKey, const UniString& rValue, rtl_TextEncoding eEncoding )
{
    if ( mpData->mbIsUTF8BOM  )
        eEncoding = RTL_TEXTENCODING_UTF8;
    WriteKey( rKey, ByteString( rValue, eEncoding ) );
}

// -----------------------------------------------------------------------

void Config::DeleteKey( const ByteString& rKey )
{
    // Config-Daten evt. updaten
    if ( !mnLockCount || !mpData->mbRead )
    {
        ImplUpdateConfig();
        mpData->mbRead = sal_True;
    }

    // Key suchen und Value setzen
    ImplGroupData* pGroup = ImplGetGroup();
    if ( pGroup )
    {
        ImplKeyData* pPrevKey = NULL;
        ImplKeyData* pKey = pGroup->mpFirstKey;
        while ( pKey )
        {
            if ( !pKey->mbIsComment && pKey->maKey.EqualsIgnoreCaseAscii( rKey ) )
                break;

            pPrevKey = pKey;
            pKey = pKey->mpNext;
        }

        if ( pKey )
        {
            // Gruppe weiterschalten und loeschen
            if ( pPrevKey )
                pPrevKey->mpNext = pKey->mpNext;
            else
                pGroup->mpFirstKey = pKey->mpNext;
            delete pKey;

            // Config-Datei neu schreiben
            if ( !mnLockCount && mbPersistence )
                ImplWriteConfig( mpData );
            else
            {
                mpData->mbModified = sal_True;
            }
        }
    }
}

// -----------------------------------------------------------------------

sal_uInt16 Config::GetKeyCount() const
{
#ifdef DBG_UTIL
    ByteString aTraceStr( "Config::GetKeyCount()" );
    aTraceStr += " from ";
    aTraceStr += GetGroup();
    aTraceStr += " in ";
    aTraceStr += ByteString( maFileName, RTL_TEXTENCODING_UTF8 );
    DBG_TRACE( aTraceStr.GetBuffer() );
#endif

    // Config-Daten evt. updaten
    if ( !mnLockCount )
        ImplUpdateConfig();

    // Key suchen und Value zurueckgeben
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

// -----------------------------------------------------------------------

ByteString Config::GetKeyName( sal_uInt16 nKey ) const
{
#ifdef DBG_UTIL
    ByteString aTraceStr( "Config::GetKeyName( " );
    aTraceStr += ByteString::CreateFromInt32(nKey);
    aTraceStr += " ) from ";
    aTraceStr += GetGroup();
    aTraceStr += " in ";
    aTraceStr += ByteString( maFileName, RTL_TEXTENCODING_UTF8 );
    DBG_TRACE( aTraceStr.GetBuffer() );
#endif

    // Key suchen und Name zurueckgeben
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

    return getEmptyByteString();
}

// -----------------------------------------------------------------------

ByteString Config::ReadKey( sal_uInt16 nKey ) const
{
#ifdef DBG_UTIL
    ByteString aTraceStr( "Config::ReadKey( " );
    aTraceStr += ByteString::CreateFromInt32( nKey );
    aTraceStr += " ) from ";
    aTraceStr += GetGroup();
    aTraceStr += " in ";
    aTraceStr += ByteString( maFileName, RTL_TEXTENCODING_UTF8 );
    DBG_TRACE( aTraceStr.GetBuffer() );
#endif

    // Key suchen und Value zurueckgeben
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

    return getEmptyByteString();
}

// -----------------------------------------------------------------------

void Config::EnterLock()
{
    // Config-Daten evt. updaten
    if ( !mnLockCount )
        ImplUpdateConfig();

    mnLockCount++;
}

// -----------------------------------------------------------------------

void Config::LeaveLock()
{
    DBG_ASSERT( mnLockCount, "Config::LeaveLook() without Config::EnterLook()" );
    mnLockCount--;

    if ( (mnLockCount == 0) && mpData->mbModified && mbPersistence )
        ImplWriteConfig( mpData );
}

// -----------------------------------------------------------------------

sal_Bool Config::Update()
{
    return ImplUpdateConfig();
}

// -----------------------------------------------------------------------

void Config::Flush()
{
    if ( mpData->mbModified && mbPersistence )
        ImplWriteConfig( mpData );
}

// -----------------------------------------------------------------------

void Config::SetLineEnd( LineEnd eLineEnd )
{
    mpData->meLineEnd = eLineEnd;
}

// -----------------------------------------------------------------------

LineEnd Config::GetLineEnd() const
{
    return mpData->meLineEnd;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
