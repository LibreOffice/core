/*************************************************************************
 *
 *  $RCSfile: config.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: mh $ $Date: 2000-11-13 18:05:48 $
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

#define _CONFIG_CXX

#include <string.h>

#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif
#ifndef _NEW_HXX
#include <new.hxx>
#endif
#ifndef _STREAM_HXX
#include <stream.hxx>
#endif
#ifndef _DEBUG_HXX
#include <debug.hxx>
#endif

#ifndef _CONFIG_HXX
#include <config.hxx>
#endif

#ifndef _VOS_MACROS_HXX
#include <vos/macros.hxx>
#endif
#ifndef _VOS_PROFILE_HXX
#include <vos/profile.hxx>
#endif

#pragma hdrstop

#define MAXBUFLEN   1024        // Fuer Buffer bei VOS-Funktionen

// -----------------
// - ImplConfigData -
// -----------------

struct ImplKeyData
{
    ImplKeyData*    mpNext;
    ByteString      maKey;
    ByteString      maValue;
    BOOL            mbIsComment;
};

struct ImplGroupData
{
    ImplGroupData*  mpNext;
    ImplKeyData*    mpFirstKey;
    ByteString      maGroupName;
    USHORT          mnEmptyLines;
};

struct ImplConfigData
{
    ImplGroupData*  mpFirstGroup;
    XubString       maFileName;
    ULONG           mnDataUpdateId;
    ULONG           mnTimeStamp;
    LineEnd         meLineEnd;
    USHORT          mnRefCount;
    BOOL            mbModified;
    BOOL            mbRead;
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
    ::rtl::OUString aUncPath;
    // check ir rFileName is already UNC; if not make it so
    if( rPath.CompareToAscii( "//", 2 ) == COMPARE_EQUAL )
        aUncPath = rPath;
    else if( ::osl::FileBase::normalizePath( rPath, aUncPath ) != ::osl::FileBase::E_None )
        aUncPath = rPath;
    return aUncPath;
}

static ULONG ImplSysGetConfigTimeStamp( const XubString& rFileName )
{
    ULONG nTimeStamp = 0;
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

static BYTE* ImplSysReadConfig( const XubString& rFileName,
                                ULONG& rRead, BOOL& rbRead, ULONG& rTimeStamp )
{
    BYTE*           pBuf = NULL;
    ::osl::File aFile( rFileName );

    if( aFile.open( osl_File_OpenFlag_Read ) == ::osl::FileBase::E_None )
    {
        aFile.setPos( Pos_End, 0 );
        sal_uInt64 nPos = 0, nRead = 0;
        if( aFile.getPos( nPos ) == ::osl::FileBase::E_None )
        {
            pBuf = (BYTE*)SvMemAlloc( nPos );
            aFile.setPos( Pos_Absolut, 0 );
            if( aFile.read( pBuf, nPos, nRead ) == ::osl::FileBase::E_None && nRead == nPos )
            {
                rTimeStamp = ImplSysGetConfigTimeStamp( rFileName );
                rbRead = TRUE;
                rRead = nRead;
            }
            else
            {
                SvMemFree( pBuf );
                pBuf = NULL;
            }
        }
    }

    return pBuf;
}

// -----------------------------------------------------------------------

static BOOL ImplSysWriteConfig( const XubString& rFileName,
                                const BYTE* pBuf, ULONG nBufLen, ULONG& rTimeStamp )
{
    BOOL bSuccess = FALSE;

    ::osl::File aFile( rFileName );
    ::osl::FileBase::RC eError = aFile.open( osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
    if( eError != ::osl::FileBase::E_None )
        eError = aFile.open( osl_File_OpenFlag_Write );
    if( eError == ::osl::FileBase::E_None )
    {
        // truncate
        aFile.setSize( 0 );
        sal_uInt64 nWritten;
        if( aFile.write( pBuf, nBufLen, nWritten ) == ::osl::FileBase::E_None && nWritten == nBufLen )
        {
            bSuccess = TRUE;
            rTimeStamp = ImplSysGetConfigTimeStamp( rFileName );
        }
    }

    return bSuccess;
}

// -----------------------------------------------------------------------

static String ImplMakeConfigName( const XubString* pFileName,
                                  const XubString* pPathName )
{
    ::rtl::OUString aName;
    ::rtl::OUString aFileName;
    ::rtl::OUString aPathName;
    if ( pFileName )
        aFileName = *pFileName;
    if ( pPathName )
        aPathName = toUncPath( *pPathName );
    ::vos::OProfile::getProfileName( aName, aFileName, aPathName );
    return aName;
}

// -----------------------------------------------------------------------

static void ImplMakeConfigList( ImplConfigData* pData,
                                const BYTE* pBuf, ULONG nLen )
{
    // kein Buffer, keine Daten
    if ( !nLen )
        return;

    // Buffer parsen und Liste zusammenbauen
    unsigned int    nStart;
    unsigned int    nLineLen;
    unsigned int    nNameLen;
    unsigned int    nKeyLen;
    unsigned int    i;
    const BYTE*     pLine;
    ImplKeyData*    pPrevKey;
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
                        pKey->mbIsComment   = TRUE;
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
                    pKey->maValue = ByteString( (const sal_Char*)pLine, nLineLen );
                    pKey->mbIsComment = TRUE;
                }
                else
                {
                    pKey->mbIsComment = FALSE;
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
                            pKey->maValue = ByteString( (const sal_Char*)pLine, nLineLen );
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

static BYTE* ImplGetConfigBuffer( const ImplConfigData* pData, ULONG& rLen )
{
    BYTE*           pWriteBuf;
    BYTE*           pBuf;
    BYTE            aLineEndBuf[2];
    ImplKeyData*    pKey;
    ImplGroupData*  pGroup;
    unsigned int    nBufLen;
    USHORT          nValueLen;
    USHORT          nKeyLen;
    USHORT          nLineEndLen;

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
        pWriteBuf = (BYTE*)SvMemAlloc( nLineEndLen );
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
    pWriteBuf = (BYTE*)SvMemAlloc( nBufLen );
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
            USHORT nEmptyLines = pGroup->mnEmptyLines;
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
    ULONG           nTimeStamp = 0;
    ULONG           nRead = 0;
    BOOL            bRead = FALSE;
    BYTE*           pBuf = ImplSysReadConfig( pData->maFileName, nRead, bRead, nTimeStamp );

    // Aus dem Buffer die Config-Verwaltungsliste aufbauen
    if ( pBuf )
    {
        ImplMakeConfigList( pData, pBuf, nRead );
        SvMemFree( pBuf );
    }
    pData->mnTimeStamp = nTimeStamp;
    pData->mbModified  = FALSE;
    if ( bRead )
        pData->mbRead = TRUE;
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
    ULONG   nBufLen;
    BYTE*   pBuf = ImplGetConfigBuffer( pData, nBufLen );
    if ( pBuf )
    {
        if ( ImplSysWriteConfig( pData->maFileName, pBuf, nBufLen, pData->mnTimeStamp ) )
            pData->mbModified = FALSE;
        SvMemFree( pBuf );
    }
    else
        pData->mbModified = FALSE;
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
    pData->mbRead           = FALSE;
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

BOOL Config::ImplUpdateConfig() const
{
    // Wenn sich TimeStamp unterscheidet, dann Datei neu einlesen
    if ( mpData->mnTimeStamp != ImplSysGetConfigTimeStamp( maFileName ) )
    {
        ImplDeleteConfigData( mpData );
        ImplReadConfig( mpData );
        mpData->mnDataUpdateId++;
        return TRUE;
    }
    else
        return FALSE;
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
    mbPersistence   = TRUE;

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
    mbPersistence   = TRUE;

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

void Config::Modified()
{
}

// -----------------------------------------------------------------------

String Config::GetDefDirectory()
{
    String aDefConfig = ImplMakeConfigName( NULL, NULL );
    // kill the last path element to obtain the path
    // path separator is always / since we only use UNC notation nowadays
    // (which also means that aDefConfig is an absolute path already)
    int nPos = aDefConfig.SearchBackward( '/' );
    if( nPos != STRING_NOTFOUND )
        aDefConfig.Erase( nPos );

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
        mpData->mbRead = TRUE;
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
            mpData->mbModified = TRUE;
            Modified();
        }

        // Gruppen auf ungluetig setzen
        mnDataUpdateId = mpData->mnDataUpdateId;
        mpData->mnDataUpdateId++;
    }
}

// -----------------------------------------------------------------------

ByteString Config::GetGroupName( USHORT nGroup ) const
{
    // Config-Daten evt. updaten
    if ( !mnLockCount )
        ImplUpdateConfig();

    ImplGroupData*  pGroup = mpData->mpFirstGroup;
    USHORT          nGroupCount = 0;
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

USHORT Config::GetGroupCount() const
{
    // Config-Daten evt. updaten
    if ( !mnLockCount )
        ImplUpdateConfig();

    ImplGroupData*  pGroup = mpData->mpFirstGroup;
    USHORT          nGroupCount = 0;
    while ( pGroup )
    {
        nGroupCount++;
        pGroup = pGroup->mpNext;
    }

    return nGroupCount;
}

// -----------------------------------------------------------------------

BOOL Config::HasGroup( const ByteString& rGroup ) const
{
    // Config-Daten evt. updaten
    if ( !mnLockCount )
        ImplUpdateConfig();

    ImplGroupData*  pGroup = mpData->mpFirstGroup;
    BOOL            bRet = FALSE;

    while( pGroup )
    {
        if( pGroup->maGroupName.EqualsIgnoreCaseAscii( rGroup ) )
        {
            bRet = TRUE;
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
        mpData->mbRead = TRUE;
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

        BOOL bNewValue;
        if ( !pKey )
        {
            pKey              = new ImplKeyData;
            pKey->mpNext      = NULL;
            pKey->maKey       = rKey;
            pKey->mbIsComment = FALSE;
            if ( pPrevKey )
                pPrevKey->mpNext = pKey;
            else
                pGroup->mpFirstKey = pKey;
            bNewValue = TRUE;
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
                mpData->mbModified = TRUE;
                Modified();
            }
        }
    }
}

// -----------------------------------------------------------------------

void Config::WriteKey( const ByteString& rKey, const UniString& rValue, rtl_TextEncoding eEncoding )
{
    WriteKey( rKey, ByteString( rValue, eEncoding ) );
}

// -----------------------------------------------------------------------

void Config::DeleteKey( const ByteString& rKey )
{
    // Config-Daten evt. updaten
    if ( !mnLockCount || !mpData->mbRead )
    {
        ImplUpdateConfig();
        mpData->mbRead = TRUE;
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
                mpData->mbModified = TRUE;
                Modified();
            }
        }
    }
}

// -----------------------------------------------------------------------

USHORT Config::GetKeyCount() const
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
    USHORT nCount = 0;
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

ByteString Config::GetKeyName( USHORT nKey ) const
{
#ifdef DBG_UTIL
    ByteString aTraceStr( "Config::GetKeyName( " );
    aTraceStr += nKey;
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

ByteString Config::ReadKey( USHORT nKey ) const
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

BOOL Config::Update()
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

