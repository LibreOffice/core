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

#if !defined UNX
#include <io.h>
#include <process.h>
#endif

#if defined(UNX) || defined(OS2)
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tools/debug.hxx>
#include "comdep.hxx"
#include <tools/fsys.hxx>
#define _TOOLS_HXX
#include <tools/urlobj.hxx>

#ifdef UNX
#define _MAX_PATH 260
#endif
#include <tools/stream.hxx>

#include <osl/mutex.hxx>

#include <osl/file.hxx>
#include <rtl/instance.hxx>


using namespace osl;

using ::rtl::OUString;

int ApiRet2ToSolarError_Impl( int nApiRet );

//--------------------------------------------------------------------
int Sys2SolarError_Impl( int nSysErr )
{
    switch ( nSysErr )
    {
#ifdef WNT
                case NO_ERROR:                                  return ERRCODE_NONE;
                case ERROR_INVALID_FUNCTION:    return ERRCODE_IO_GENERAL;
                case ERROR_FILE_NOT_FOUND:              return ERRCODE_IO_NOTEXISTS;
                case ERROR_PATH_NOT_FOUND:              return ERRCODE_IO_NOTEXISTSPATH;
                case ERROR_TOO_MANY_OPEN_FILES: return ERRCODE_IO_TOOMANYOPENFILES;
                case ERROR_ACCESS_DENIED:               return ERRCODE_IO_ACCESSDENIED;
                case ERROR_INVALID_HANDLE:              return ERRCODE_IO_GENERAL;
                case ERROR_NOT_ENOUGH_MEMORY:   return ERRCODE_IO_OUTOFMEMORY;
                case ERROR_INVALID_BLOCK:               return ERRCODE_IO_GENERAL;
//              case ERROR_BAD_ENVIRONMENT:     return ERRCODE_IO_;
                case ERROR_BAD_FORMAT:                  return ERRCODE_IO_WRONGFORMAT;
                case ERROR_INVALID_ACCESS:              return ERRCODE_IO_ACCESSDENIED;
//              case ERROR_INVALID_DATA:                return ERRCODE_IO_;
                case ERROR_INVALID_DRIVE:               return ERRCODE_IO_INVALIDDEVICE;
                case ERROR_CURRENT_DIRECTORY:   return ERRCODE_IO_CURRENTDIR;
                case ERROR_NOT_SAME_DEVICE:     return ERRCODE_IO_NOTSAMEDEVICE;
//              case ERROR_NO_MORE_FILES:               return ERRCODE_IO_;
                case ERROR_WRITE_PROTECT:               return ERRCODE_IO_CANTWRITE;
                case ERROR_BAD_UNIT:                    return ERRCODE_IO_INVALIDDEVICE;
                case ERROR_NOT_READY:                   return ERRCODE_IO_DEVICENOTREADY;
                case ERROR_BAD_COMMAND:                 return ERRCODE_IO_GENERAL;
                case ERROR_CRC:                                 return ERRCODE_IO_BADCRC;
                case ERROR_BAD_LENGTH:                  return ERRCODE_IO_INVALIDLENGTH;
                case ERROR_SEEK:                                return ERRCODE_IO_CANTSEEK;
                case ERROR_NOT_DOS_DISK:                return ERRCODE_IO_WRONGFORMAT;
                case ERROR_SECTOR_NOT_FOUND:    return ERRCODE_IO_GENERAL;
                case ERROR_WRITE_FAULT:                 return ERRCODE_IO_CANTWRITE;
                case ERROR_READ_FAULT:                  return ERRCODE_IO_CANTREAD;
                case ERROR_GEN_FAILURE:                 return ERRCODE_IO_GENERAL;
                case ERROR_SHARING_VIOLATION:   return ERRCODE_IO_LOCKVIOLATION;
                case ERROR_LOCK_VIOLATION:              return ERRCODE_IO_LOCKVIOLATION;
                case ERROR_WRONG_DISK:                  return ERRCODE_IO_INVALIDDEVICE;
                case ERROR_NOT_SUPPORTED:               return ERRCODE_IO_NOTSUPPORTED;
#else
        case 0:         return ERRCODE_NONE;
        case ENOENT:    return ERRCODE_IO_NOTEXISTS;
        case EACCES:    return ERRCODE_IO_ACCESSDENIED;
        case EEXIST:    return ERRCODE_IO_ALREADYEXISTS;
        case EINVAL:    return ERRCODE_IO_INVALIDPARAMETER;
        case EMFILE:    return ERRCODE_IO_TOOMANYOPENFILES;
        case ENOMEM:    return ERRCODE_IO_OUTOFMEMORY;
        case ENOSPC:    return ERRCODE_IO_OUTOFSPACE;
#endif
    }

    OSL_TRACE( "FSys: unknown system error %d occurred", nSysErr );
    return FSYS_ERR_UNKNOWN;
}

//--------------------------------------------------------------------

#ifndef BOOTSTRAP

FSysRedirector* FSysRedirector::_pRedirector = 0;
sal_Bool FSysRedirector::_bEnabled = sal_True;
#ifdef UNX
sal_Bool bInRedirection = sal_True;
#else
sal_Bool bInRedirection = sal_False;
#endif
static osl::Mutex* pRedirectMutex = 0;

//------------------------------------------------------------------------
void FSysRedirector::Register( FSysRedirector *pRedirector )
{
        if ( pRedirector )
            pRedirectMutex = new osl::Mutex;
        else
                DELETEZ( pRedirectMutex );
        _pRedirector = pRedirector;
}

//------------------------------------------------------------------------

void FSysRedirector::DoRedirect( String &rPath )
{
        String aURL(rPath);

        // if redirection is disabled or not even registered do nothing
        if ( !_bEnabled || !pRedirectMutex )
                return;

        // redirect only removable or remote volumes
        if ( !IsRedirectable_Impl( ByteString( aURL, osl_getThreadTextEncoding() ) ) )
                return;

        // Redirection is acessible only by one thread per time
        // dont move the guard behind the bInRedirection check!!!
        // think of nested calls (when called from callback)
        osl::MutexGuard aGuard( pRedirectMutex );

        // if already in redirection, dont redirect
        if ( bInRedirection )
                return;

        // dont redirect on nested calls
        bInRedirection = sal_True;

        // convert to URL
#ifndef UNX
        for ( sal_Unicode *p = (sal_Unicode*)aURL.GetBuffer(); *p; ++p )
                if ( '\\' == *p ) *p = '/';
                else if ( ':' == *p ) *p = '|';
#endif

        aURL.Insert( String("file:///", osl_getThreadTextEncoding()), 0 );

        // do redirection
        Redirector();

        bInRedirection = sal_False;
        return;
}

//------------------------------------------------------------------------

FSysRedirector* FSysRedirector::Redirector()
{
        if ( !_pRedirector )
                Register( new FSysRedirector );
        return _pRedirector;
}

#endif // BOOTSTRAP

//--------------------------------------------------------------------

class DirEntryStack: public List
{
public:
                        DirEntryStack() {};
                        ~DirEntryStack();

    inline  void        Push( DirEntry *pEntry );
    inline  DirEntry*   Pop();
    inline  DirEntry*   Top();
    inline  DirEntry*   Bottom();
};

inline void DirEntryStack::Push( DirEntry *pEntry )
{
    List::Insert( pEntry, LIST_APPEND );
}

inline DirEntry* DirEntryStack::Pop()
{
    return (DirEntry*) List::Remove( Count() - 1 );
}

inline DirEntry* DirEntryStack::Top()
{
    return (DirEntry*) List::GetObject( Count() - 1 );
}

inline DirEntry* DirEntryStack::Bottom()
{
    return (DirEntry*) List::GetObject( 0 );
}

//--------------------------------------------------------------------

DBG_NAME( DirEntry );

/*************************************************************************
|*
|*    DirEntry::~DirEntryStack()
|*
*************************************************************************/

DirEntryStack::~DirEntryStack()
{
    while ( Count() )
        delete Pop();
}

/*************************************************************************
|*
|*    ImpCheckDirEntry()
|*
|*    Beschreibung      Pruefung eines DirEntry fuer DBG_UTIL
|*    Parameter         void* p     Zeiger auf den DirEntry
|*    Return-Wert       char*       Fehlermeldungs-TExtension oder NULL
|*
*************************************************************************/

#ifdef DBG_UTIL
const char* ImpCheckDirEntry( const void* p )
{
    DirEntry* p0 = (DirEntry*)p;

    if ( p0->pParent )
        DBG_CHKOBJ( p0->pParent, DirEntry, ImpCheckDirEntry );

    return NULL;
}
#endif

/*************************************************************************
|*
|*    ImplCutPath()
|*
|*    Beschreibung      Fuegt ... ein, damit maximal nMaxChars lang
|*
*************************************************************************/

ByteString ImplCutPath( const ByteString& rStr, sal_uInt16 nMax, char cAccDel )
{
    sal_uInt16  nMaxPathLen = nMax;
    ByteString  aCutPath( rStr );
    sal_Bool    bInsertPrefix = sal_False;
    sal_uInt16  nBegin = aCutPath.Search( cAccDel );

    if( nBegin == STRING_NOTFOUND )
        nBegin = 0;
    else
        nMaxPathLen += 2;   // fuer Prefix <Laufwerk>:

    while( aCutPath.Len() > nMaxPathLen )
    {
        sal_uInt16 nEnd = aCutPath.Search( cAccDel, nBegin + 1 );
        sal_uInt16 nCount;

        if ( nEnd != STRING_NOTFOUND )
        {
            nCount = nEnd - nBegin;
            aCutPath.Erase( nBegin, nCount );
            bInsertPrefix = sal_True;
        }
        else
            break;
    }

    if ( aCutPath.Len() > nMaxPathLen )
    {
        for ( sal_uInt16 n = nMaxPathLen; n > nMaxPathLen/2; --n )
            if ( !ByteString(aCutPath.GetChar(n)).IsAlphaNumericAscii() )
            {
                aCutPath.Erase( n );
                aCutPath += "...";
                break;
            }
    }

    if ( bInsertPrefix )
    {
        ByteString aIns( cAccDel );
        aIns += "...";
        aCutPath.Insert( aIns, nBegin );
    }

    return aCutPath;
}

/*************************************************************************
|*
|*    DirEntry::ImpParseOs2Name()
|*
*************************************************************************/

FSysError DirEntry::ImpParseOs2Name( const ByteString& rPfad, FSysPathStyle eStyle  )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    // die einzelnen Namen auf einen Stack packen
    String       aPfad( rPfad, osl_getThreadTextEncoding() );
    DirEntryStack   aStack;

    do
    {
        // den Namen vor dem ersten "\\" abspalten,
        // falls '\\' am Anfang, ist der Name '\\',
        // der Rest immer ohne die fuehrenden '\\'.
        // ein ":" trennt ebenfalls, gehoert aber zum Namen
        // den ersten '\\', '/' oder ':' suchen
        sal_uInt16 nPos;
        for ( nPos = 0;
              nPos < aPfad.Len() &&                             //?O
                  aPfad.GetChar(nPos) != '\\' && aPfad.GetChar(nPos) != '/' &&      //?O
                  aPfad.GetChar(nPos) != ':';                               //?O
              nPos++ )
            /* do nothing */;

        // ist der Name ein UNC Pathname?
        if ( nPos == 0 && aPfad.Len() > 1 &&
             ( ( aPfad.GetChar(0) == '\\' && aPfad.GetChar(1) == '\\' ) ||
               ( aPfad.GetChar(0) == '/' && aPfad.GetChar(1) == '/' ) ) )
        {
            for ( nPos = 2; aPfad.Len() > nPos; ++nPos )
                if ( aPfad.GetChar(nPos) == '\\' || aPfad.GetChar(nPos) == '/' )
                    break;
            aName = ByteString( aPfad.Copy( 2, nPos-2 ), osl_getThreadTextEncoding() );
            aStack.Push( new DirEntry( aName, FSYS_FLAG_ABSROOT, eStyle ) );
        }
        // ist der Name die Root des aktuellen Drives?
        else if ( nPos == 0 && aPfad.Len() > 0 &&
                  ( aPfad.GetChar(0) == '\\' || aPfad.GetChar(0) == '/' ) )
        {
            // Root-Directory des aktuellen Drives
            aStack.Push( new DirEntry( FSYS_FLAG_ABSROOT ) );
        }
        else
        {
            // ist der Name ein Drive?
            if ( nPos < aPfad.Len() && aPfad.GetChar(nPos) == ':' )
            {
                aName = ByteString( aPfad.Copy( 0, nPos + 1 ), osl_getThreadTextEncoding() );

                // ist der Name die Root des Drives
                if ( (nPos + 1) < aPfad.Len() &&
                     ( aPfad.GetChar(nPos+1) == '\\' || aPfad.GetChar(nPos+1) == '/' ) )
                {
                    // schon was auf dem Stack?
                    // oder Novell-Format? (not supported wegen URLs)
                        if ( aStack.Count() || aName.Len() > 2 )
                        {
                            aName = rPfad;
                            return FSYS_ERR_MISPLACEDCHAR;
                        }
                    // Root-Directory des Drive
                    aStack.Push( new DirEntry( aName, FSYS_FLAG_ABSROOT, eStyle ) );
                }
                else
                {
                    // liegt ein anderes Drive auf dem Stack?
                    if ( aStack.Count() &&
                         COMPARE_EQUAL != aStack.Bottom()->aName.CompareIgnoreCaseToAscii(aName) )
                        aStack.Clear();

                    // liegt jetzt nichts mehr auf dem Stack?
                    if ( !aStack.Count() )
                        aStack.Push( new DirEntry( aName, FSYS_FLAG_RELROOT, eStyle ) );
                }
            }

            // es ist kein Drive
            else
            {
                // den Namen ohne Trenner abspalten
                aName = ByteString( aPfad.Copy( 0, nPos ), osl_getThreadTextEncoding() );

                // stellt der Name die aktuelle Directory dar?
                if ( aName == "." )
                    /* do nothing */;

                // stellt der Name die Parent-Directory dar?
                else if ( aName == ".." )
                {
                    // ist nichts, ein Parent oder eine relative Root
                    // auf dem Stack?
                    if ( ( aStack.Count() == 0 ) ||
                         ( aStack.Top()->eFlag == FSYS_FLAG_PARENT ) ||
                         ( aStack.Top()->eFlag == FSYS_FLAG_RELROOT ) )
                        // fuehrende Parents kommen auf den Stack
                        aStack.Push( new DirEntry( FSYS_FLAG_PARENT ) );

                    // ist es eine absolute Root
                    else if ( aStack.Top()->eFlag == FSYS_FLAG_ABSROOT )
                    {
                        // die hat keine Parent-Directory
                        aName = rPfad;
                        return FSYS_ERR_NOTEXISTS;
                    }
                    else
                        // sonst hebt der Parent den TOS auf
                        delete aStack.Pop();
                }

                else
                {
                    if ( eStyle == FSYS_STYLE_FAT )
                    {
                        // ist der Name grundsaetzlich ungueltig?
                        int         nPunkte = 0;
                        const char *pChar;
                        for ( pChar = aName.GetBuffer();
                              nPunkte < 2 && *pChar != 0;
                              pChar++ )
                        {
                            if ( *pChar == ';' )
                                nPunkte = 0;
                            else
                                nPunkte += ( *pChar == '.' ) ? 1 : 0;
                        }
                        if ( nPunkte > 1 )
                        {
                            aName = rPfad;
                            return FSYS_ERR_MISPLACEDCHAR;
                        }
                    }

                    // normalen Entries kommen auf den Stack
                                        DirEntry *pNew = new DirEntry( aName, FSYS_FLAG_NORMAL, eStyle );
                                        if ( !pNew->IsValid() )
                                        {
                                                aName = rPfad;
                                                ErrCode eErr = pNew->GetError();
                                                delete pNew;
                                                return eErr;
                                        }
                    aStack.Push( pNew );
                }
            }
        }

        // den Restpfad bestimmen
        aPfad.Erase( 0, nPos + 1 );
        while ( aPfad.Len() && ( aPfad.GetChar(0) == '\\' || aPfad.GetChar(0) == '/' ) )
            aPfad.Erase( 0, 1 );
    }
    while ( aPfad.Len() );

    sal_uIntPtr nErr = ERRCODE_NONE;
    // Haupt-Entry (selbst) zuweisen
    if ( aStack.Count() == 0 )
    {
        eFlag = FSYS_FLAG_CURRENT;
        aName.Erase();
    }
    else
    {
        eFlag = aStack.Top()->eFlag;
        aName = aStack.Top()->aName;
        nErr = aStack.Top()->nError;
        delete aStack.Pop();
    }

    // die Parent-Entries vom Stack holen
    DirEntry** pTemp = &pParent; // Zeiger auf den Member pParent setzen
    while ( aStack.Count() )
    {
        *pTemp = aStack.Pop();

        // Zeiger auf den Member pParent des eigenen Parent setzen
        pTemp = &( (*pTemp)->pParent );
    }

    // wird damit ein Volume beschrieben?
    if ( !pParent && eFlag == FSYS_FLAG_RELROOT && aName.Len() )
        eFlag = FSYS_FLAG_VOLUME;

    // bei gesetztem ErrorCode den Namen komplett "ubernehmen
    if ( nErr )
        aName = rPfad;
    return nErr;
}

/*************************************************************************
|*
|*    DirEntry::ImpParseName()
|*
*************************************************************************/

FSysError DirEntry::ImpParseName( const ByteString& rbInitName,
                                  FSysPathStyle eStyle )
{
    String  rInitName( rbInitName, osl_getThreadTextEncoding() );
    if ( eStyle == FSYS_STYLE_HOST )
        eStyle = DEFSTYLE;

    // KI-Division of FSys
    if ( eStyle == FSYS_STYLE_DETECT )
    {
        sal_Unicode cFirst = rInitName.GetChar(0);
        if ( rInitName.Len() == 2 && rInitName.GetChar(1) == ':' &&
             ((cFirst >= 'A' && cFirst <= 'Z') ||
              (cFirst >= 'a' && cFirst <= 'z')))
           eStyle = FSYS_STYLE_HPFS;
        else if ( rInitName.Len() > 2 && rInitName.GetChar(1) == ':' )
        {
            if ( rInitName.Search( ':', 2 ) == STRING_NOTFOUND )
                eStyle = FSYS_STYLE_HPFS;
            else
                eStyle = FSYS_STYLE_MAC;
        }
        else if ( rInitName.Search( '/' ) != STRING_NOTFOUND )
            eStyle = FSYS_STYLE_BSD;
        else if ( rInitName.Search( '\\' ) != STRING_NOTFOUND )
            eStyle = FSYS_STYLE_HPFS;
        else if ( rInitName.Search( ':' ) != STRING_NOTFOUND )
            eStyle = FSYS_STYLE_MAC;
        else
            eStyle = FSYS_STYLE_HPFS;
    }

    switch ( eStyle )
    {
        case FSYS_STYLE_FAT:
        case FSYS_STYLE_VFAT:
        case FSYS_STYLE_HPFS:
        case FSYS_STYLE_NTFS:
        case FSYS_STYLE_NWFS:
            return ImpParseOs2Name( rbInitName, eStyle );

        case FSYS_STYLE_BSD:
        case FSYS_STYLE_SYSV:
            return ImpParseUnixName( rbInitName, eStyle );

        case FSYS_STYLE_MAC:
            return FSYS_ERR_OK;

        default:
            return FSYS_ERR_UNKNOWN;
    }
}

/*************************************************************************
|*
|*    GetStyle()
|*
*************************************************************************/

static FSysPathStyle GetStyle( FSysPathStyle eStyle )
{
    if ( eStyle == FSYS_STYLE_HOST || eStyle == FSYS_STYLE_DETECT )
        return DEFSTYLE;
    else
        return eStyle;
}

/*************************************************************************
|*
|*    DirEntry::ImpTrim()
|*
|*    Beschreibung      bringt den Namen auf Betriebssystem-Norm
|*                      z.B. 8.3 lower beim MS-DOS Formatter
|*                      wirkt nicht rekursiv
|*
*************************************************************************/

void DirEntry::ImpTrim( FSysPathStyle eStyle )
{
    // Wildcards werden nicht geclipt
    if ( ( aName.Search( '*' ) != STRING_NOTFOUND ) ||
         ( aName.Search( '?' ) != STRING_NOTFOUND ) ||
         ( aName.Search( ';' ) != STRING_NOTFOUND ) )
        return;

    switch ( eStyle )
    {
        case FSYS_STYLE_FAT:
        {
            sal_uInt16 nPunktPos = aName.Search( '.' );
            if ( nPunktPos == STRING_NOTFOUND )
            {
                if ( aName.Len() > 8 )
                {
                    nError = ERRCODE_IO_MISPLACEDCHAR|ERRCODE_WARNING_MASK;
                    aName.Erase( 8 );
                }
            }
            else
            {
                if ( nPunktPos > 8 )
                {
                    nError = ERRCODE_IO_MISPLACEDCHAR|ERRCODE_WARNING_MASK;
                    aName.Erase( 8, nPunktPos - 8 );
                    nPunktPos = 8;
                }
                if ( aName.Len() > nPunktPos + 3 )
                {
                    if ( aName.Len() - nPunktPos > 4 )
                    {
                        nError = ERRCODE_IO_MISPLACEDCHAR|ERRCODE_WARNING_MASK;
                        aName.Erase( nPunktPos + 4 );
                    }
                }
            }
            aName.ToLowerAscii();
            break;
        }

        case FSYS_STYLE_VFAT:
        case FSYS_STYLE_HPFS:
        case FSYS_STYLE_NTFS:
        case FSYS_STYLE_NWFS:
            if ( aName.Len() > 254 )
            {
                nError = ERRCODE_IO_MISPLACEDCHAR|ERRCODE_WARNING_MASK;
                aName.Erase( 254 );
            }

            if ( eStyle == FSYS_STYLE_HPFS &&
                 ( eFlag == FSYS_FLAG_ABSROOT || eFlag == FSYS_FLAG_RELROOT ) )
                aName.ToUpperAscii();
            break;

        case FSYS_STYLE_SYSV:
            if ( aName.Len() > 14 )
            {
                nError = ERRCODE_IO_MISPLACEDCHAR|ERRCODE_WARNING_MASK;
                aName.Erase( 14 );
            }
            break;

        case FSYS_STYLE_BSD:
            if ( aName.Len() > 250 )
            {
                nError = ERRCODE_IO_MISPLACEDCHAR|ERRCODE_WARNING_MASK;
                aName.Erase( 250 );
            }
            break;

        case FSYS_STYLE_MAC:
            if ( eFlag & ( FSYS_FLAG_ABSROOT | FSYS_FLAG_VOLUME ) )
            {
                if ( aName.Len() > 27 )
                {
                    nError = ERRCODE_IO_MISPLACEDCHAR|ERRCODE_WARNING_MASK;
                    aName.Erase( 27 );
                }
            }
            else
            {
                if ( aName.Len() > 31 )
                {
                    nError = ERRCODE_IO_MISPLACEDCHAR|ERRCODE_WARNING_MASK;
                    aName.Erase( 31 );
                }
            }
            break;

        default:
            /* kann nicht sein */;
    }
}

/*************************************************************************
|*
|*    DirEntry::DirEntry()
|*
*************************************************************************/

DirEntry::DirEntry( const ByteString& rName, DirEntryFlag eDirFlag,
                    FSysPathStyle eStyle ) :
#ifdef FEAT_FSYS_DOUBLESPEED
            pStat( 0 ),
#endif
            aName( rName )
{
    DBG_CTOR( DirEntry, ImpCheckDirEntry );

    pParent         = NULL;
    eFlag           = eDirFlag;
    nError          = FSYS_ERR_OK;

    ImpTrim( eStyle );
}

/*************************************************************************
|*
|*    DirEntry::DirEntry()
|*
*************************************************************************/

DirEntry::DirEntry( const DirEntry& rOrig ) :
#ifdef FEAT_FSYS_DOUBLESPEED
            pStat( rOrig.pStat ? new FileStat(*rOrig.pStat) : 0 ),
#endif
            aName( rOrig.aName )
{
    DBG_CTOR( DirEntry, ImpCheckDirEntry );

    eFlag           = rOrig.eFlag;
    nError          = rOrig.nError;

    if ( rOrig.pParent )
    {
        pParent = new DirEntry( *rOrig.pParent );
    }
    else
    {
        pParent = NULL;
    }
}

/*************************************************************************
|*
|*    DirEntry::DirEntry()
|*
*************************************************************************/

DirEntry::DirEntry( const String& rInitName, FSysPathStyle eStyle )
#ifdef FEAT_FSYS_DOUBLESPEED
            : pStat( 0 )
#endif
{
    DBG_CTOR( DirEntry, ImpCheckDirEntry );

    pParent         = NULL;

    // schnelle Loesung fuer Leerstring
    if ( !rInitName.Len())
    {
        eFlag                   = FSYS_FLAG_CURRENT;
        nError                  = FSYS_ERR_OK;
        return;
    }

    ByteString aTmpName(rInitName, osl_getThreadTextEncoding());
    if( eStyle == FSYS_STYLE_URL || aTmpName.CompareIgnoreCaseToAscii("file:",5 ) == COMPARE_EQUAL )
    {
#ifndef BOOTSTRAP
        DBG_WARNING( "File URLs are not permitted but accepted" );
        aTmpName = ByteString(String(INetURLObject( rInitName ).PathToFileName()), osl_getThreadTextEncoding());
                eStyle = FSYS_STYLE_HOST;
#endif // BOOTSTRAP
    }
    else
    {
        ::rtl::OUString aTmp;
        ::rtl::OUString aOInitName;
        if ( FileBase::getFileURLFromSystemPath( OUString( rInitName ), aTmp ) == FileBase::E_None )
        {
            aOInitName = OUString( rInitName );
            aTmpName = ByteString( String(aOInitName), osl_getThreadTextEncoding() );
        }

#ifdef DBG_UTIL
        // ASF nur bei Default eStyle, nicht z.B. aus MakeShortName()
        if( eStyle == FSYS_STYLE_HOST &&
            aTmpName.Search( "://" ) != STRING_NOTFOUND )
        {
            ByteString aErr = "DirEntries akzeptieren nur File URLS: ";
            aErr += aTmpName;
            DBG_WARNING( aErr.GetBuffer() );
        }
#endif
    }

    nError  = ImpParseName( aTmpName, eStyle );

    if ( nError != FSYS_ERR_OK )
        eFlag = FSYS_FLAG_INVALID;
}

/*************************************************************************/

DirEntry::DirEntry( const ByteString& rInitName, FSysPathStyle eStyle )
#ifdef FEAT_FSYS_DOUBLESPEED
            : pStat( 0 )
#endif
{
    DBG_CTOR( DirEntry, ImpCheckDirEntry );

    pParent         = NULL;

    // schnelle Loesung fuer Leerstring
    if ( !rInitName.Len() )
    {
        eFlag                   = FSYS_FLAG_CURRENT;
        nError                  = FSYS_ERR_OK;
        return;
    }

    ByteString aTmpName( rInitName );
    if( eStyle == FSYS_STYLE_URL || rInitName.CompareIgnoreCaseToAscii("file:",5 ) == COMPARE_EQUAL )
    {
#ifndef BOOTSTRAP
        DBG_WARNING( "File URLs are not permitted but accepted" );
        aTmpName = ByteString(String(INetURLObject( rInitName ).PathToFileName()), osl_getThreadTextEncoding());
        eStyle = FSYS_STYLE_HOST;
#endif
    }
#ifdef DBG_UTIL
    else
        // ASF nur bei Default eStyle, nicht z.B. aus MakeShortName()
        if( eStyle == FSYS_STYLE_HOST &&
            rInitName.Search( "://" ) != STRING_NOTFOUND )
        {
            ByteString aErr = "DirEntries akzeptieren nur File URLS: ";
            aErr += rInitName;
            DBG_WARNING( aErr.GetBuffer() );
        }
#endif

    nError  = ImpParseName( aTmpName, eStyle );

    if ( nError != FSYS_ERR_OK )
        eFlag = FSYS_FLAG_INVALID;
}

/*************************************************************************
|*
|*    DirEntry::DirEntry()
|*
*************************************************************************/

DirEntry::DirEntry( DirEntryFlag eDirFlag )
#ifdef FEAT_FSYS_DOUBLESPEED
            : pStat( 0 )
#endif
{
    DBG_CTOR( DirEntry, ImpCheckDirEntry );

    eFlag           = eDirFlag;
    nError          = ( eFlag == FSYS_FLAG_INVALID ) ? FSYS_ERR_UNKNOWN : FSYS_ERR_OK;
    pParent         = NULL;
}

/*************************************************************************
|*
|*    DirEntry::~DirEntry()
|*
*************************************************************************/

DirEntry::~DirEntry()
{
    DBG_DTOR( DirEntry, ImpCheckDirEntry );

    delete pParent;
#ifdef FEAT_FSYS_DOUBLESPEED
    delete pStat;
#endif

}

/*************************************************************************
|*
|*    DirEntry::ImpGetTopPtr() const
|*
*************************************************************************/

const DirEntry* DirEntry::ImpGetTopPtr() const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    const DirEntry *pTemp = this;
    while ( pTemp->pParent )
        pTemp = pTemp->pParent;

    return pTemp;
}

/*************************************************************************
|*
|*    DirEntry::ImpGetTopPtr()
|*
*************************************************************************/

DirEntry* DirEntry::ImpGetTopPtr()
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    DirEntry *pTemp = this;
    while ( pTemp->pParent )
        pTemp = pTemp->pParent;

    return pTemp;
}

/*************************************************************************
|*
|*    DirEntry::ImpChangeParent()
|*
*************************************************************************/

DirEntry* DirEntry::ImpChangeParent( DirEntry* pNewParent, sal_Bool bNormalize )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    DirEntry *pTemp = pParent;
    if ( bNormalize && pNewParent &&
         pNewParent->eFlag == FSYS_FLAG_RELROOT && !pNewParent->aName.Len() )
    {
        pParent = 0;
        delete pNewParent;
    }
    else
        pParent = pNewParent;

    return pTemp;
}

/*************************************************************************
|*
|*    DirEntry::Exists()
|*
*************************************************************************/

sal_Bool DirEntry::Exists( FSysAccess nAccess ) const
{
#ifndef BOOTSTRAP
    static osl::Mutex aLocalMutex;
    osl::MutexGuard aGuard( aLocalMutex );
#endif
        if ( !IsValid() )
                return sal_False;

#if defined WNT || defined OS2
    // spezielle Filenamen sind vom System da
    if ( ( aName.CompareIgnoreCaseToAscii("CLOCK$") == COMPARE_EQUAL ||
           aName.CompareIgnoreCaseToAscii("CON") == COMPARE_EQUAL ||
           aName.CompareIgnoreCaseToAscii("AUX") == COMPARE_EQUAL ||
           aName.CompareIgnoreCaseToAscii("COM1") == COMPARE_EQUAL ||
           aName.CompareIgnoreCaseToAscii("COM2") == COMPARE_EQUAL ||
           aName.CompareIgnoreCaseToAscii("COM3") == COMPARE_EQUAL ||
           aName.CompareIgnoreCaseToAscii("COM4") == COMPARE_EQUAL ||
           aName.CompareIgnoreCaseToAscii("LPT1") == COMPARE_EQUAL ||
           aName.CompareIgnoreCaseToAscii("LPT2") == COMPARE_EQUAL ||
           aName.CompareIgnoreCaseToAscii("LPT3") == COMPARE_EQUAL ||
           aName.CompareIgnoreCaseToAscii("NUL") == COMPARE_EQUAL ||
           aName.CompareIgnoreCaseToAscii("PRN") == COMPARE_EQUAL ) )
        return sal_True;
#endif

        FSysFailOnErrorImpl();
        DirEntryKind eKind = FileStat( *this, nAccess ).GetKind();
        if ( eKind & ( FSYS_KIND_FILE | FSYS_KIND_DIR ) )
        {
                return sal_True;
        }

#if defined WNT || defined OS2
        if ( 0 != ( eKind & FSYS_KIND_DEV ) )
        {
                return DRIVE_EXISTS( ImpGetTopPtr()->aName.GetChar(0) );
        }
#endif

        return 0 != ( eKind & ( FSYS_KIND_FILE | FSYS_KIND_DIR ) );
}

/*************************************************************************
|*
|*    DirEntry::First()
|*
*************************************************************************/

sal_Bool DirEntry::First()
{
    FSysFailOnErrorImpl();

        String    aUniPathName( GetPath().GetFull() );
#ifndef BOOTSTRAP
        FSysRedirector::DoRedirect( aUniPathName );
        ByteString aPathName(aUniPathName, osl_getThreadTextEncoding());
#else
        ByteString aPathName(aUniPathName, gsl_getSystemTextEncoding());
#endif
        aPathName = GUI2FSYS( aPathName );

        DIR      *pDir = opendir( (char*) aPathName.GetBuffer() );
        if ( pDir )
        {
#ifndef BOOTSTRAP
                WildCard aWildeKarte( String(CMP_LOWER( aName ), osl_getThreadTextEncoding()) );
#else
                WildCard aWildeKarte( String(CMP_LOWER( aName ), gsl_getSystemTextEncoding()) );
#endif
                for ( dirent* pEntry = readdir( pDir );
                          pEntry;
                          pEntry = readdir( pDir ) )
                {
                        ByteString aFound( FSYS2GUI( ByteString( pEntry->d_name ) ) );
                        if ( aWildeKarte.Matches( String(CMP_LOWER( aFound ), osl_getThreadTextEncoding())))
                        {
                                aName = aFound;
                                closedir( pDir );
                                return sal_True;
                        }
                }
                closedir( pDir );
        }
        return sal_False;
}

/*************************************************************************
|*
|*    DirEntry::GetFull()
|*
*************************************************************************/

String DirEntry::GetFull( FSysPathStyle eStyle, sal_Bool bWithDelimiter,
                          sal_uInt16 nMaxChars ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    ByteString aRet;
    eStyle = GetStyle( eStyle );
    if ( pParent )
    {
        if ( ( pParent->eFlag == FSYS_FLAG_ABSROOT ||
               pParent->eFlag == FSYS_FLAG_RELROOT ||
               pParent->eFlag == FSYS_FLAG_VOLUME ) )
        {
            aRet  = ByteString(pParent->GetName( eStyle ), osl_getThreadTextEncoding());
            aRet += ByteString(GetName( eStyle ), osl_getThreadTextEncoding());
        }
        else
        {
            aRet  = ByteString(pParent->GetFull( eStyle ), osl_getThreadTextEncoding());
            aRet += ACCESSDELIM_C(eStyle);
            aRet += ByteString(GetName( eStyle ), osl_getThreadTextEncoding());
        }
    }
    else
    {
        aRet = ByteString(GetName( eStyle ), osl_getThreadTextEncoding());
    }

    if ( ( eStyle == FSYS_STYLE_MAC ) &&
         ( ImpGetTopPtr()->eFlag != FSYS_FLAG_VOLUME )  &&
         ( ImpGetTopPtr()->eFlag != FSYS_FLAG_ABSROOT ) &&
         ( aRet.GetChar(0) != ':' ) )
        aRet.Insert( ACCESSDELIM_C(eStyle), 0 );

    //! Hack
    if ( bWithDelimiter )
        if ( aRet.GetChar( aRet.Len()-1 ) != ACCESSDELIM_C(eStyle) )
            aRet += ACCESSDELIM_C(eStyle);

    //! noch ein Hack
    if ( nMaxChars < STRING_MAXLEN )
        aRet = ImplCutPath( aRet, nMaxChars, ACCESSDELIM_C(eStyle) );

    return String(aRet, osl_getThreadTextEncoding());
}

/*************************************************************************
|*
|*    DirEntry::GetPath()
|*
*************************************************************************/

DirEntry DirEntry::GetPath() const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    if ( pParent )
        return DirEntry( *pParent );

    return DirEntry();
}

/*************************************************************************
|*
|*    DirEntry::GetExtension()
|*
*************************************************************************/

String DirEntry::GetExtension( char cSep ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    const char *p0 = ( aName.GetBuffer() );
    const char *p1 = p0 + aName.Len() - 1;
    while ( p1 >= p0 && *p1 != cSep )
    p1--;

    if ( p1 >= p0 )
        // es wurde ein cSep an der Position p1 gefunden
        return String(
            aName.Copy( static_cast< xub_StrLen >(p1 - p0 + 1) ),
            osl_getThreadTextEncoding());
    return String();
}

/*************************************************************************
|*
|*    DirEntry::GetBase()
|*
*************************************************************************/

String DirEntry::GetBase( char cSep ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    const char *p0 = ( aName.GetBuffer() );
    const char *p1 = p0 + aName.Len() - 1;
    while ( p1 >= p0 && *p1 != cSep )
        p1--;

    if ( p1 >= p0 )
        // es wurde ein cSep an der Position p1 gefunden
        return String(
            aName.Copy( 0, static_cast< xub_StrLen >(p1 - p0) ),
            osl_getThreadTextEncoding());

    else
        // es wurde kein cSep gefunden
        return String(aName, osl_getThreadTextEncoding());
}

/*************************************************************************
|*
|*    DirEntry::GetName()
|*
*************************************************************************/

String DirEntry::GetName( FSysPathStyle eStyle ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    ByteString aRet;
    eStyle = GetStyle( eStyle );

    switch( eFlag )
    {
        case FSYS_FLAG_PARENT:
            aRet = ACTPARENT(eStyle);
                        break;

        case FSYS_FLAG_ABSROOT:
        {
            if ( eStyle == FSYS_STYLE_URL )
            {
                aRet = "file:///";
                aRet += aName;

#ifndef UNX
                if ( aName.Len())
                {
                    if ( aName.GetChar(aName.Len()-1) == ':' )
                    {
                        aRet.SetChar(aRet.Len()-1, '|');
                    }
                    else
                    {
                        aRet.Insert( '/', 5 );
                    }
                    aRet += "/";
                }
#endif
            }
            else if ( eStyle != FSYS_STYLE_MAC &&
                                 aName.Len() > 1 && aName.GetChar( 1 ) != ':'  )
            {
                // UNC-Pathname
                aRet = ACCESSDELIM_C(eStyle);
                aRet += ACCESSDELIM_C(eStyle);
                aRet += aName ;
                aRet += ACCESSDELIM_C(eStyle);
            }
            else
            {
                aRet = aName;
                aRet += ACCESSDELIM_C(eStyle);
            }
            break;
        }

        case FSYS_FLAG_INVALID:
        case FSYS_FLAG_VOLUME:
        {
            if ( eStyle == FSYS_STYLE_URL )
            {
                aRet = "file:///";
                aRet += aName;
#ifndef UNX
                if ( aName.Len() && aName.GetChar(aName.Len()-1) == ':' )
                {
                    aRet.SetChar(aRet.Len()-1, '|');
                }
#endif
            }
            else
            {
                aRet = aName;
            }

            break;
        }

        case FSYS_FLAG_RELROOT:
            if ( !aName.Len() )
            {
                aRet = ACTCURRENT(eStyle);
                break;
            }

        default:
            aRet = aName;
    }

    return String(aRet, osl_getThreadTextEncoding());
}

/*************************************************************************
|*
|*    DirEntry::IsAbs()
|*
*************************************************************************/

bool DirEntry::IsAbs() const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

#ifdef UNX
    return ( pParent ? pParent->IsAbs() : eFlag == FSYS_FLAG_ABSROOT );
#else
    return ( pParent ? pParent->IsAbs() : eFlag == FSYS_FLAG_ABSROOT && aName.Len() > 0 );
#endif
}

/*************************************************************************
|*
|*    DirEntry::CutName()
|*
*************************************************************************/

String DirEntry::CutName( FSysPathStyle eStyle )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    eStyle = GetStyle( eStyle );

    String aOldName( GetName( eStyle ) );

    if ( pParent )
    {
        DirEntry *pOldParent = pParent;
        if ( pOldParent )
        {
            pParent = pOldParent->pParent;
            eFlag = pOldParent->eFlag;
            aName = pOldParent->aName;
            pOldParent->pParent = NULL;
            delete pOldParent;
        }
        else
        {
            eFlag = FSYS_FLAG_CURRENT;
            aName.Erase();
        }
    }
    else
    {
        eFlag = FSYS_FLAG_CURRENT;
        aName.Erase();
        delete pParent;
        pParent = NULL;
    }

    return aOldName;
}

/*************************************************************************
|*
|*    DirEntry::NameCompare
|*
|*    Beschreibung      Vergleich nur die Namen (ohne Pfad, aber mit Gross/Klein)
|*
*************************************************************************/

StringCompare DirEntry::NameCompare( const DirEntry &rWith ) const
{
        ByteString aThisName;
        ByteString aParameterName;

#ifdef UNX
                aThisName = aName;
                aParameterName = rWith.aName;
#else
                aThisName = ByteString(aName).ToLowerAscii();
                aParameterName = ByteString(rWith.aName).ToLowerAscii();
#endif

    return aThisName.CompareTo( aParameterName );
}


/*************************************************************************
|*
|*    DirEntry::operator==()
|*
*************************************************************************/

sal_Bool DirEntry::operator==( const DirEntry& rEntry ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    // test wheather the contents are textual the same

    if ( nError && ( nError == rEntry.nError ) )
        return sal_True;
    if ( nError || rEntry.nError ||
         ( eFlag == FSYS_FLAG_INVALID ) ||
         ( rEntry.eFlag == FSYS_FLAG_INVALID ) )
        return sal_False;

#ifndef OS2
    const
#endif
    DirEntry *pThis = (DirEntry *)this;
#ifndef OS2
    const
#endif
    DirEntry *pWith = (DirEntry *)&rEntry;
    while( pThis && pWith && (pThis->eFlag == pWith->eFlag) )
    {
        if ( CMP_LOWER(pThis->aName) != CMP_LOWER(pWith->aName) )
            break;
        pThis = pThis->pParent;
        pWith = pWith->pParent;
    }

    return ( !pThis && !pWith );
}

/*************************************************************************
|*
|*    DirEntry::operator=()
|*
*************************************************************************/

DirEntry& DirEntry::operator=( const DirEntry& rEntry )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    if ( this == &rEntry )
        return *this;
    if ( rEntry.nError != FSYS_ERR_OK ) {
        OSL_FAIL("Zuweisung mit invalidem DirEntry");
        nError = rEntry.nError;
        return *this;
    }

    // Name und Typ uebernehmen, Refs beibehalten
    aName                       = rEntry.aName;
    eFlag                       = rEntry.eFlag;
    nError                      = FSYS_ERR_OK;

    DirEntry *pOldParent = pParent;
    if ( rEntry.pParent )
        pParent = new DirEntry( *rEntry.pParent );
    else
        pParent = NULL;

    if ( pOldParent )
        delete pOldParent;
    return *this;
}

/*************************************************************************
|*
|*    DirEntry::operator+()
|*
*************************************************************************/

DirEntry DirEntry::operator+( const DirEntry& rEntry ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );
#ifdef DBG_UTIL
        static sal_Bool bTested = sal_False;
        if ( !bTested )
        {
                bTested = sal_True;
                FSysTest();
        }
#endif

        const DirEntry *pEntryTop = rEntry.ImpGetTopPtr();
        const DirEntry *pThisTop = ImpGetTopPtr();

        // "." + irgendwas oder irgendwas + "d:irgendwas"
/* TPF:org
    if ( ( eFlag == FSYS_FLAG_RELROOT && !aName ) ||
                 ( pEntryTop->aName.Len() &&
                        ( pEntryTop->eFlag == FSYS_FLAG_ABSROOT ||
                      pEntryTop->eFlag == FSYS_FLAG_RELROOT ||
                          pEntryTop->eFlag == FSYS_FLAG_VOLUME ) ) )
                return rEntry;
*/

    if (
        (eFlag == FSYS_FLAG_RELROOT && !aName.Len()) ||
        (
         (pEntryTop->aName.Len()  ||
          ((rEntry.Level()>1)?(rEntry[rEntry.Level()-2].aName.CompareIgnoreCaseToAscii(RFS_IDENTIFIER)==COMPARE_EQUAL):sal_False))
          &&
         (pEntryTop->eFlag == FSYS_FLAG_ABSROOT ||
          pEntryTop->eFlag == FSYS_FLAG_RELROOT ||
          pEntryTop->eFlag == FSYS_FLAG_VOLUME)
        )
       )
    {
                return rEntry;
    }

    // irgendwas + "." (=> pEntryTop == &rEntry)
    if ( pEntryTop->eFlag == FSYS_FLAG_RELROOT && !pEntryTop->aName.Len() )
    {
                DBG_ASSERT( pEntryTop == &rEntry, "DirEntry::op+ buggy" );
                return *this;
    }

    // root += ".." (=> unmoeglich)
        if ( pEntryTop->eFlag == FSYS_FLAG_PARENT && pThisTop == this &&
                ( eFlag == FSYS_FLAG_ABSROOT ) )
                return DirEntry( FSYS_FLAG_INVALID );

        // irgendwas += abs (=> nur Device uebernehmen falls vorhanden)
        if ( pEntryTop->eFlag == FSYS_FLAG_ABSROOT )
        {
                ByteString aDevice;
                if ( pThisTop->eFlag == FSYS_FLAG_ABSROOT )
                        aDevice = pThisTop->aName;
                DirEntry aRet = rEntry;
                if ( aDevice.Len() )
                        aRet.ImpGetTopPtr()->aName = aDevice;
                return aRet;
        }

        // irgendwas += ".." (=> aufloesen)
        if ( eFlag == FSYS_FLAG_NORMAL && pEntryTop->eFlag == FSYS_FLAG_PARENT )
        {
                String aConcated( GetFull() );
                aConcated += ACCESSDELIM_C(FSYS_STYLE_HOST);
                aConcated += rEntry.GetFull();
                return DirEntry( aConcated );
        }

        // sonst einfach hintereinander haengen
        DirEntry aRet( rEntry );
        DirEntry *pTop = aRet.ImpGetTopPtr();
        pTop->pParent = new DirEntry( *this );

        return aRet;
}

/*************************************************************************
|*
|*    DirEntry::operator+=()
|*
*************************************************************************/

DirEntry &DirEntry::operator+=( const DirEntry& rEntry )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    return *this = *this + rEntry;
}

/*************************************************************************
|*
|*    DirEntry::GetAccessDelimiter()
|*
*************************************************************************/

String DirEntry::GetAccessDelimiter( FSysPathStyle eFormatter )
{
        return String( ACCESSDELIM_C( GetStyle( eFormatter ) ) );
}

/*************************************************************************
|*
|*    DirEntry::SetExtension()
|*
*************************************************************************/

void DirEntry::SetExtension( const String& rExtension, char cSep )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    // do not set extensions for drives
    if(eFlag == FSYS_FLAG_ABSROOT)
    {
        nError = FSYS_ERR_NOTSUPPORTED;
        return;
    }

    // cSep im Namen suchen
    const char *p0 = ( aName.GetBuffer() );
    const char *p1 = p0 + aName.Len() - 1;
    while ( p1 >= p0 && *p1 != cSep )
        p1--;
    if ( p1 >= p0 )
    {
        // es wurde ein cSep an der Position p1 gefunden
        aName.Erase(
            static_cast< xub_StrLen >(
                p1 - p0 + 1 - ( rExtension.Len() ? 0 : 1 )) );
        aName += ByteString(rExtension, osl_getThreadTextEncoding());
    }
    else if ( rExtension.Len() )
    {
        // es wurde kein cSep gefunden
        aName += cSep;
        aName += ByteString(rExtension, osl_getThreadTextEncoding());
    }
}

/*************************************************************************
|*
|*    DirEntry::CutExtension()
|*
*************************************************************************/

String DirEntry::CutExtension( char cSep )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    const char *p0 = ( aName.GetBuffer() );
    const char *p1 = p0 + aName.Len() - 1;
    while ( p1 >= p0 && *p1 != cSep )
        p1--;

    if ( p1 >= p0 )
    {
        // es wurde ein cSep an der Position p1 gefunden
        aName.Erase( static_cast< xub_StrLen >(p1-p0) );
        return String(p1 + 1, osl_getThreadTextEncoding());
    }

    return String();
}

/*************************************************************************
|*
|*    DirEntry::SetName()
|*
*************************************************************************/

void DirEntry::SetName( const String& rName, FSysPathStyle eFormatter )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

        if ( eFormatter == FSYS_STYLE_HOST || eFormatter == FSYS_STYLE_DETECT )
        eFormatter = DEFSTYLE;
    ByteString aAccDelim( ACCESSDELIM_C( eFormatter ) );

    if ( (eFlag != FSYS_FLAG_NORMAL) ||
         (aName.Search( ':' ) != STRING_NOTFOUND) ||
         (aName.Search( aAccDelim ) != STRING_NOTFOUND) ||
         (eFormatter == FSYS_STYLE_FAT && (aName.GetTokenCount( '.' ) > 2) ) )
    {
        eFlag = FSYS_FLAG_INVALID;
    }
    else
        {
        aName = ByteString(rName, osl_getThreadTextEncoding());
        }
}

/*************************************************************************
|*
|*    DirEntry::Find()
|*
*************************************************************************/
sal_Bool DirEntry::Find( const String& rPfad, char cDelim )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

        if ( ImpGetTopPtr()->eFlag == FSYS_FLAG_ABSROOT )
                return sal_True;

        sal_Bool bWild = aName.Search( '*' ) != STRING_NOTFOUND ||
                                 aName.Search( '?' ) != STRING_NOTFOUND;
        if ( !cDelim )
                cDelim = SEARCHDELIM(DEFSTYLE)[0];

        sal_uInt16 nTokenCount = rPfad.GetTokenCount( cDelim );
        sal_uInt16 nIndex = 0;
        ByteString aThis = ACCESSDELIM(DEFSTYLE);
        aThis += ByteString(GetFull(), osl_getThreadTextEncoding());
        for ( sal_uInt16 nToken = 0; nToken < nTokenCount; ++nToken )
        {
            ByteString aPath = ByteString(rPfad, osl_getThreadTextEncoding()).GetToken( 0, cDelim, nIndex );

            if ( aPath.Len() )
            {
                if (aPath.GetChar(aPath.Len()-1)== ACCESSDELIM(DEFSTYLE)[0])
                        aPath.Erase(aPath.Len()-1);
                aPath += aThis;
                DirEntry aEntry( String(aPath, osl_getThreadTextEncoding()));
                if ( aEntry.ToAbs() &&
                         ( ( !bWild && aEntry.Exists() ) || ( bWild && aEntry.First() ) ) )
                {
                        (*this) = aEntry;
                        return sal_True;
                }
            }
        }
        return sal_False;
}

/*************************************************************************
|*
|*    DirEntry::ImpToRel()
|*
*************************************************************************/

sal_Bool DirEntry::ImpToRel( String aCurStr )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

        DirEntry aThis(*this);
    aThis.ToAbs();
    String aThisStr( aThis.GetFull( FSYS_STYLE_HPFS ) );

    // #109512 preserve case of path even if caseinsensitive
    String aThisCompareStr( aThisStr ), aCurCompareStr( aCurStr );
    if ( ! IsCaseSensitive() )
    {
        aThisCompareStr.ToLowerAscii();
        aCurCompareStr.ToLowerAscii();
    }

    // "Ubereinstimmung pr"ufen
    sal_uInt16 nPos = aThisCompareStr.Match( aCurCompareStr );
    if ( nPos == STRING_MATCH && aThisStr.Len() != aCurStr.Len() )
        nPos = Min( aThisStr.Len(), aCurStr.Len() );

    // Sonderfall, die DirEntries sind identisch
    if ( nPos == STRING_MATCH )
    {
        // dann ist der relative Pfad das aktuelle Verzeichnis
        *this = DirEntry();
        return sal_True;
    }

    // Sonderfall, die DirEntries sind total verschieden
    if ( nPos == 0 )
    {
        // dann ist der relativste Pfad absolut
        *this = aThis;
        return sal_False;
    }

    // sonst nehmen wir die identischen Einzelteile vorne weg
    while ( nPos > 0 && aThisStr.GetChar(nPos) != '\\' )
        --nPos;
        aThisStr.Erase( 0, nPos + ( ( aThisStr.GetChar(nPos) == '\\' ) ? 1 : 0 ) );
    aCurStr.Erase( 0, nPos + ( ( aCurStr.GetChar(nPos) == '\\' ) ? 1 : 0 ) );

    // und fuellen mit dem Level der Directories auf
    for ( nPos = 0; nPos < aCurStr.Len(); ++nPos )
        if ( aCurStr.GetChar(nPos) == '\\' )
            aThisStr.Insert( String( "..\\", osl_getThreadTextEncoding() ), 0 );

    // das ist dann unser relativer Pfad
    *this = DirEntry( aThisStr, FSYS_STYLE_HPFS );
    return sal_True;
}

/*************************************************************************
|*
|*    DirEntry::CutRelParents()
|*
*************************************************************************/

sal_uInt16 DirEntry::CutRelParents()
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

        // erstes '..' finden
    DirEntry *pDir = 0;
    DirEntry *pPar;

    for (  pPar = this;
          pPar && pPar->eFlag != FSYS_FLAG_PARENT;
          pPar = pPar->pParent )
        pDir = pPar;

    // '..' zaehlen
    sal_uInt16 nParCount = 0;
    while ( pPar && pPar->eFlag == FSYS_FLAG_PARENT )
    {
        ++nParCount;
        pPar = pPar->pParent;
    }

    // cutten
    if ( pDir )
        DELETEZ(pDir->pParent);
    else
        eFlag = FSYS_FLAG_CURRENT;

    return nParCount;
}

/*************************************************************************
|*
|*    DirEntry::ToRel()
|*
*************************************************************************/

sal_Bool DirEntry::ToRel()
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

        DirEntry aCur;
    aCur.ToAbs();
        return ImpToRel( aCur.GetFull( FSYS_STYLE_HPFS ) );
}

/*************************************************************************
|*
|*    DirEntry::ToRel()
|*
*************************************************************************/

sal_Bool DirEntry::ToRel( const DirEntry& rStart )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

        DirEntry aStart( rStart );
        aStart.ToAbs();
        return ImpToRel( aStart.GetFull( FSYS_STYLE_HPFS ) );
}

/*************************************************************************
|*
|*    DirEntry::GetDevice()
|*
*************************************************************************/

#ifndef UNX

DirEntry DirEntry::GetDevice() const
{
        DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

        const DirEntry *pTop = ImpGetTopPtr();

        if ( ( pTop->eFlag == FSYS_FLAG_ABSROOT || pTop->eFlag == FSYS_FLAG_RELROOT ) &&
                 pTop->aName.Len() )
                return DirEntry( pTop->aName, FSYS_FLAG_VOLUME, FSYS_STYLE_HOST );
        else
                return DirEntry( ByteString(), FSYS_FLAG_INVALID, FSYS_STYLE_HOST );
}

#endif

/*************************************************************************
|*
|*    DirEntry::SetBase()
|*
*************************************************************************/

void DirEntry::SetBase( const String& rBase, char cSep )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    const char *p0 = ( aName.GetBuffer() );
    const char *p1 = p0 + aName.Len() - 1;
    while ( p1 >= p0 && *p1 != cSep )
        p1--;

    if ( p1 >= p0 )
    {
        // es wurde ein cSep an der Position p1 gefunden
        aName.Erase( 0, static_cast< xub_StrLen >(p1 - p0) );
        aName.Insert( ByteString(rBase, osl_getThreadTextEncoding()), 0 );
    }
    else
        aName = ByteString(rBase, osl_getThreadTextEncoding());
}

/*************************************************************************
|*
|*    DirEntry::GetSearchDelimiter()
|*
*************************************************************************/

String DirEntry::GetSearchDelimiter( FSysPathStyle eFormatter )
{
    return String( ByteString(SEARCHDELIM( GetStyle( eFormatter ) ) ), osl_getThreadTextEncoding());
}

/*************************************************************************
|*
|*    DirEntry::GetMaxNameLen()
|*
|*    Beschreibung      Liefert die maximale Anzahl von Zeichen in
|*                      einzelnen Namensteile. Bei FileSystmen mit
|*                      fester Extension (FAT) zaehlt diese nicht mit.
|*                      Bei unbekannten FileSytemen und FSYS_STYLE_URL
|*                      wird USHRT_MAX zurueckgegeben.
|*
*************************************************************************/

sal_uInt16 DirEntry::GetMaxNameLen( FSysPathStyle eFormatter )
{
    eFormatter = GetStyle( eFormatter );
    switch ( eFormatter )
    {
        case FSYS_STYLE_MAC:    return  31;

        case FSYS_STYLE_FAT:    return   8;

        case FSYS_STYLE_VFAT:
        case FSYS_STYLE_NTFS:
        case FSYS_STYLE_NWFS:
        case FSYS_STYLE_HPFS:   return 255;


        case FSYS_STYLE_SYSV:   return  14;

        case FSYS_STYLE_BSD:    return 250;

        default:
            return USHRT_MAX;
    }
}

/*************************************************************************
|*
|*    DirEntry::TempName()
|*
|*    Beschreibung      FSYS.SDW - Aha, wo?
|*
*************************************************************************/
namespace { struct TempNameBase_Impl : public rtl::Static< DirEntry, TempNameBase_Impl > {}; }

const DirEntry& DirEntry::SetTempNameBase( const String &rBase )
{
        DirEntry aTempDir = DirEntry().TempName().GetPath();
        aTempDir += DirEntry( rBase );
#ifdef UNX
        ByteString aName( aTempDir.GetFull(), osl_getThreadTextEncoding());
        if ( access( aName.GetBuffer(), W_OK | X_OK | R_OK ) )
        {
            // Create the directory and only on success give all rights to
            // everyone. Use mkdir instead of DirEntry::MakeDir because
            // this returns sal_True even if directory already exists.

            if ( !mkdir( aName.GetBuffer(), S_IRWXU | S_IRWXG | S_IRWXO ) )
                chmod( aName.GetBuffer(), S_IRWXU | S_IRWXG | S_IRWXO );

            // This will not create a directory but perhaps FileStat called
            // there modifies the DirEntry

            aTempDir.MakeDir();
        }
#else
        aTempDir.MakeDir();
#endif
        DirEntry &rEntry = TempNameBase_Impl::get();
        rEntry = aTempDir.TempName( FSYS_KIND_DIR );
        return rEntry;
}

DirEntry DirEntry::TempName( DirEntryKind eKind ) const
{
        // ggf. Base-Temp-Dir verwenden (macht Remote keinen Sinn => vorher)
        const DirEntry &rEntry = TempNameBase_Impl::get();
        if ( !pParent && FSYS_FLAG_CURRENT != rEntry.eFlag && FSYS_FLAG_ABSROOT != eFlag )
        {
                DirEntry aFactory( rEntry );
                aFactory += GetName();
                return aFactory.TempName();
        }

        ByteString aDirName;
        char *ret_val;
        size_t i;

        // determine Directory, Prefix and Extension
        char pfx[6];
        char ext[5];
        const char *dir;
        const char *pWild = strchr( aName.GetBuffer(), '*' );
        if ( !pWild )
            pWild = strchr( aName.GetBuffer(), '?' );

        if ( pWild )
        {
            if ( pParent )
                aDirName = ByteString(pParent->GetFull(), osl_getThreadTextEncoding());
            strncpy( pfx, aName.GetBuffer(), Min( (int)5, (int)(pWild-aName.GetBuffer()) ) );
            pfx[ pWild-aName.GetBuffer() ] = 0;
            const char *pExt = strchr( pWild, '.' );
            if ( pExt )
            {
                strncpy( ext, pExt, 4 );
                ext[4] = 0;
            }
            else
                strcpy( ext, ".tmp" );
        }
        else
        {
            aDirName = ByteString(GetFull(), osl_getThreadTextEncoding());
            strcpy( pfx, "lo" );
            strcpy( ext, ".tmp" );
        }
        dir = aDirName.GetBuffer();

        char sBuf[_MAX_PATH];
        if ( eFlag == FSYS_FLAG_CURRENT || ( !pParent && pWild ) )
            dir = TempDirImpl(sBuf);

        DirEntry aRet(FSYS_FLAG_INVALID);
        i = strlen(dir);
        // need to add ?\\? + prefix + number + pid + .ext + '\0'
#       define TMPNAME_SIZE  ( 1 + 5 + 5 + 10 + 4 + 1 )
        ret_val = new char[i + TMPNAME_SIZE ];
        if (ret_val)
        {
            strcpy(ret_val,dir);

            /* Make sure directory ends with a separator    */
#if defined(WNT) || defined(OS2)
            if ( i>0 && ret_val[i-1] != '\\' && ret_val[i-1] != '/' &&
                 ret_val[i-1] != ':')
                ret_val[i++] = '\\';
#elif defined UNX
            if (i>0 && ret_val[i-1] != '/')
                ret_val[i++] = '/';
#else
#error unknown operating system
#endif

            strncpy(ret_val + i, pfx, 5);
            ret_val[i + 5] = '\0';      /* strncpy doesn't put a 0 if more  */
            i = strlen(ret_val);        /* than 'n' chars.          */

            /* Prefix can have 5 chars, leaving 3 for numbers. 26 ** 3 == 17576
             * Welcome to the 21st century, we can have longer filenames now ;)
             * New format: pfx + "5 char milli/micro second res" + "current pid" + ".tmp"
             */
#if (defined MSC || defined __MINGW32__) && defined WNT
            /* Milliseconds !! */
            static unsigned long u = GetTickCount();
            unsigned long mypid = static_cast<unsigned long>(_getpid());
#else
            /* Microseconds !! */
            static unsigned long u = clock();
            unsigned long mypid = static_cast<unsigned long>(getpid());
#endif
            for ( unsigned long nOld = u; ++u != nOld; ) /* Hae??? */
            {
                u %= 100000;  /* on *NIX repeats every 100ms, maybe less if CLOCKS_PER_SEC > 10^6 */
                snprintf(ret_val+i, TMPNAME_SIZE, "%05lu%lu", u, mypid);

                strcat(ret_val,ext);

                        if ( FSYS_KIND_FILE == eKind )
                        {
                                SvFileStream aStream( String( ret_val, osl_getThreadTextEncoding()),
                                                        STREAM_WRITE|STREAM_SHARE_DENYALL );
                                if ( aStream.IsOpen() )
                                {
                                        aStream.Seek( STREAM_SEEK_TO_END );
                                        if ( 0 == aStream.Tell() )
                                        {
                                                aRet = DirEntry( String( ret_val, osl_getThreadTextEncoding()));
                                                break;
                                        }
                                        aStream.Close();
                                }
                        }
                        else
                        {
                                // Redirect
                String aRetVal(ret_val, osl_getThreadTextEncoding());
                                String aRedirected (aRetVal);
#ifndef BOOTSTRAP
                                FSysRedirector::DoRedirect( aRedirected );
#endif
                                if ( FSYS_KIND_DIR == eKind )
                                {
                                                if ( 0 == _mkdir( ByteString(aRedirected.GetBuffer(), osl_getThreadTextEncoding()).GetBuffer() ) )
                                        {
                                                aRet = DirEntry( aRetVal );
                                                break;
                                        }
                                }
                                else
                                {
#if defined(UNX) || defined(OS2)
                                        if( access( ByteString(aRedirected, osl_getThreadTextEncoding()).GetBuffer(), F_OK ) )
                                        {
                                                aRet = DirEntry( aRetVal );
                                                break;
                                        }
#else
                                        struct stat aStat;
                                        if ( stat( ByteString(aRedirected, osl_getThreadTextEncoding()).GetBuffer(), &aStat ) )
                                        {
                                            aRet = DirEntry( aRetVal );
                                            break;
                                        }
#endif
                                }
                        }
            }

            delete[] ret_val;
            ret_val = 0;
        }

        return aRet;
}

/*************************************************************************
|*
|*    DirEntry::operator[]()
|*
*************************************************************************/

const DirEntry &DirEntry::operator[]( sal_uInt16 nParentLevel ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

        //TPF: maybe to be implemented (FastFSys)

        const DirEntry *pRes = this;
    while ( pRes && nParentLevel-- )
        pRes = pRes->pParent;

    return *pRes;
}

/*************************************************************************
|*
|*    DirEntry::ImpParseUnixName()
|*
*************************************************************************/

FSysError DirEntry::ImpParseUnixName( const ByteString& rPfad, FSysPathStyle eStyle )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    // die einzelnen Namen auf einen Stack packen
    DirEntryStack   aStack;
    ByteString      aPfad( rPfad );
    do
    {
        // den Namen vor dem ersten "/" abspalten,
        // falls '/' am Anfang, ist der Name '/',
        // der Rest immer ohne die fuehrenden '/'.
        // den ersten '/' suchen
        sal_uInt16 nPos;
        for ( nPos = 0;
              nPos < aPfad.Len() && aPfad.GetChar(nPos) != '/';
              nPos++ )
            /* do nothing */;

            // ist der Name die Root des aktuellen Drives?
        if ( nPos == 0 && aPfad.Len() > 0 && ( aPfad.GetChar(0) == '/' ) )
        {
            // Root-Directory des aktuellen Drives
            aStack.Push( new DirEntry( FSYS_FLAG_ABSROOT ) );
        }
        else
        {
            // den Namen ohne Trenner abspalten
            aName = aPfad.Copy( 0, nPos );

                        // stellt der Name die aktuelle Directory dar?
            if ( aName == "." )
                /* do nothing */;

#ifdef UNX
                        // stellt der Name das User-Dir dar?
                        else if ( aName == "~" )
                        {
                                DirEntry aHome( String( (const char *) getenv( "HOME" ), osl_getThreadTextEncoding()) );
                                for ( sal_uInt16 n = aHome.Level(); n; --n )
                                        aStack.Push( new DirEntry( aHome[ (sal_uInt16) n-1 ] ) );
                        }
#endif

                // stellt der Name die Parent-Directory dar?
            else if ( aName == ".." )
            {
                // ist nichts, ein Parent oder eine relative Root
                // auf dem Stack?
                if ( ( aStack.Count() == 0 ) ||
                     ( aStack.Top()->eFlag == FSYS_FLAG_PARENT ) )
                    // fuehrende Parents kommen auf den Stack
                    aStack.Push( new DirEntry( ByteString(), FSYS_FLAG_PARENT, eStyle ) );

                // ist es eine absolute Root
                else if ( aStack.Top()->eFlag == FSYS_FLAG_ABSROOT ) {
                    // die hat keine Parent-Directory
                    return FSYS_ERR_NOTEXISTS;
                }
                else
                    // sonst hebt der Parent den TOS auf
                    delete aStack.Pop();
            }
            else
            {
                DirEntry *pNew = NULL;
                // normalen Entries kommen auf den Stack
                                pNew = new DirEntry( aName, FSYS_FLAG_NORMAL, eStyle );
                                if ( !pNew->IsValid() )
                                {
                                        aName = rPfad;
                                        ErrCode eErr = pNew->GetError();
                                        delete pNew;
                                        return eErr;
                                }
                aStack.Push( pNew );
                        }
        }

        // den Restpfad bestimmen
        aPfad.Erase( 0, nPos + 1 );
        while ( aPfad.Len() && ( aPfad.GetChar(0) == '/' ) )
            aPfad.Erase( 0, 1 );
    }
    while ( aPfad.Len() );

    // Haupt-Entry (selbst) zuweisen
    if ( aStack.Count() == 0 )
    {
        eFlag = FSYS_FLAG_CURRENT;
        aName.Erase();
    }
    else
    {
        eFlag = aStack.Top()->eFlag;
        aName = aStack.Top()->aName;
        delete aStack.Pop();
    }

    // die Parent-Entries vom Stack holen
    DirEntry** pTemp = &pParent;
    while ( aStack.Count() )
    {
        *pTemp = aStack.Pop();
        pTemp = &( (*pTemp)->pParent );
    }

    return FSYS_ERR_OK;
}

/*************************************************************************
|*
|*    DirEntry::MakeShortName()
|*
*************************************************************************/

ErrCode CreateEntry_Impl( const DirEntry &rPath, DirEntryKind eKind )
{
    // versuchen, anzulegen (ausser bei FSYS_KIND_ALL)
    ErrCode eErr = ERRCODE_NONE;
    if ( FSYS_KIND_FILE == eKind )
    {
        SvFileStream aStream( rPath.GetFull(), STREAM_STD_WRITE );
        aStream.WriteLine( "" );
        eErr = aStream.GetError();
    }
    else if ( FSYS_KIND_ALL != eKind )
        eErr = rPath.MakeDir() ? ERRCODE_NONE : ERRCODE_IO_UNKNOWN;

    // erfolgreich?
    if ( !rPath.Exists() )
        eErr = ERRCODE_IO_UNKNOWN;  // Doch was schiefgegangen ?

    // ggf. wieder l"oschen
    if ( FSYS_KIND_NONE == eKind )
        rPath.Kill();

    // Fehlercode zur?ckliefern
    return eErr;
}

sal_Bool IsValidEntry_Impl( const DirEntry &rPath,
                        const String &rLongName,
                        DirEntryKind eKind,
                        sal_Bool bIsShortened,
                        sal_Bool bUseDelim )
{
    // Parameter-Pr"uefung
    DBG_ASSERT( eKind == FSYS_KIND_NONE || eKind == FSYS_KIND_ALL ||
                eKind == FSYS_KIND_FILE || eKind == FSYS_KIND_DIR,
                "invalid entry-kind" );

    // Alle von MSDOS erreichbaren FSYS_STYLES muessen den
    // MSDOS Filenamenanforderungen genuegen. Sonst wird probiert,
    // ob sich eine Datei des gewuenschten Names anlegen laesst.
    FSysPathStyle eStyle = DirEntry::GetPathStyle( rPath.GetDevice().GetName() );
    DirEntry aPath(rPath);
    DirEntry aName(rLongName, eStyle);
    if ( !aName.IsValid() || aName.Level() != 1 )
        return sal_False;
    aPath += aName;
    if ( 1 == aPath.Level() )
        return sal_False;
    if ( eStyle == FSYS_STYLE_FAT || eStyle == FSYS_STYLE_NWFS ||
         eStyle == FSYS_STYLE_UNKNOWN )
    {
        DirEntry aDosEntry( rLongName, FSYS_STYLE_FAT );
        if ( !aDosEntry.IsValid() )
            return sal_False;
    }

        // Pfad-Trenner sind nicht erlaubt (bei ungek"urzten auch nicht FSYS_SHORTNAME_DELIMITER)
        char cDelim = bUseDelim == 2 ? FSYS_SHORTNAME_DELIMITER : char(0);
    if (
         rLongName.Search(DirEntry::GetAccessDelimiter()) != STRING_NOTFOUND ||
         (!bIsShortened && rLongName.Search(cDelim) != STRING_NOTFOUND)
       )
    {
        return sal_False;
    }

    // MI: Abfrage nach 'CON:' etc. wird jetzt in Exists() mitgemacht
    if ( aPath.Exists() )
        return sal_False;

    return (ERRCODE_NONE == CreateEntry_Impl( aPath, eKind ));
}

//-------------------------------------------------------------------------

#define MAX_EXT_FAT         3
#define MAX_LEN_FAT         8
#define INVALID_CHARS_FAT   "\\/\"':|^<>[]?* "

#define MAX_EXT_MAC        16   // nur wegen sinnvoller Namensk"rzung
#define MAX_LEN_MAC        31
#define INVALID_CHARS_MAC   "\":"

#define MAX_EXT_MAX       250
#define MAX_LEN_MAX       255
#define INVALID_CHARS_DEF   "\\/\"':|^<>?*"

sal_Bool DirEntry::MakeShortName( const String& rLongName, DirEntryKind eKind,
                              sal_Bool bUseDelim, FSysPathStyle eStyle )
{
        String aLongName(rLongName);

        // Alle '#' aus den Dateinamen entfernen, weil das INetURLObject
        // damit Probleme hat. Siehe auch #51246#
        aLongName.EraseAllChars( '#' );
        ByteString bLongName(aLongName, osl_getThreadTextEncoding());

        // Auf Novell-Servern (wegen der rottigen Clients) nur 7bit ASCII

        // bei FSYS_KIND_ALL den alten Namen merken und abh"angen (rename)
        ByteString aOldName;
        if ( FSYS_KIND_ALL == eKind )
        {
            aOldName = ByteString(CutName(), osl_getThreadTextEncoding());
            aOldName = CMP_LOWER(aOldName);
        }

        // ist der Langname direkt verwendbar?
        if ( IsValidEntry_Impl( *this, aLongName, eKind, sal_False, bUseDelim ) )
        {
            operator+=( DirEntry(aLongName) );
            return sal_True;
        }

        // max L"angen feststellen
        sal_uInt16 nMaxExt, nMaxLen;
        if ( FSYS_STYLE_DETECT == eStyle )
            eStyle = DirEntry::GetPathStyle( GetDevice().GetName() );
        ByteString aInvalidChars;
        switch ( eStyle )
        {
            case FSYS_STYLE_FAT:
                nMaxExt = MAX_EXT_FAT;
                nMaxLen = MAX_LEN_FAT;
                aInvalidChars = INVALID_CHARS_FAT;
                break;

            case FSYS_STYLE_MAC:
                nMaxExt = MAX_EXT_MAC;
                nMaxLen = MAX_LEN_MAC;
                aInvalidChars = INVALID_CHARS_MAC;
                break;

            default:
                nMaxExt = MAX_EXT_MAX;
                nMaxLen = MAX_LEN_MAX;
                aInvalidChars = INVALID_CHARS_DEF;
        }

        // Extension abschneiden und kuerzen
        ByteString aExt;
        ByteString aFName = bLongName;
        if ( FSYS_STYLE_MAC != eStyle )
        {
            DirEntry aUnparsed;
            aUnparsed.aName = bLongName;
            aExt = ByteString(aUnparsed.CutExtension(), osl_getThreadTextEncoding());
            aFName = aUnparsed.aName;
            if ( aExt.Len() > nMaxExt )
            {
                char c = aExt.GetChar( aExt.Len() - 1 );
                aExt.Erase(nMaxExt-1);
                aExt += c;
            }
        }

        if ( FSYS_STYLE_FAT != eStyle )
        {
                // ausser auf einem FAT-System geh"ort die Extension zur
                // Maxl"ange. Muss also vorher mit dem Punkt abgezogen werden.
                nMaxLen -= ( aExt.Len() + 1 );
        }

        // Name k"urzen
        ByteString aSName;
        for ( const char *pc = aFName.GetBuffer(); aSName.Len() < nMaxLen && *pc; ++pc )
        {
            if ( STRING_NOTFOUND == aInvalidChars.Search( *pc ) &&
                 (unsigned char) *pc >= (unsigned char) 32 &&
                 ( !aSName.Len() || *pc != ' ' || aSName.GetChar(aSName.Len()-1) != ' ' ) )
                aSName += *pc;
        }
        aSName.EraseTrailingChars();

        // HRO: #74246# Also cut leading spaces
        aSName.EraseLeadingChars();

        if ( !aSName.Len() )
            aSName = "noname";

        // kommt dabei der alte Name raus?
        ByteString aNewName = aSName;
        if ( aExt.Len() )
            ( aNewName += '.' ) += aExt;
        operator+=( DirEntry(String(aNewName, osl_getThreadTextEncoding())) );
        if ( FSYS_KIND_ALL == eKind && CMP_LOWER(aName) == aOldName )
        if ( FSYS_KIND_ALL == eKind && CMP_LOWER(ByteString(GetName(), osl_getThreadTextEncoding())) == aOldName )
            return sal_True;

        // kann der gek"urzte Name direkt verwendet werden?
        if ( !Exists() && (ERRCODE_NONE == CreateEntry_Impl( *this, eKind )) )
            return sal_True;

        // darf '?##' verwendet werden, um eindeutigen Name zu erzeugen?
        if ( bUseDelim )
        {
                // eindeutigen Namen per '?##' erzeugen
            aSName.Erase( nMaxLen-3 );
            if ( bUseDelim != 2 )
                        aSName += FSYS_SHORTNAME_DELIMITER;
            for ( int n = 1; n < 99; ++n )
            {
                // Name zusammensetzen
                ByteString aTmpStr( aSName );
                aTmpStr += ByteString::CreateFromInt32(n);
                if ( aExt.Len() )
                    ( aTmpStr += '.' ) += aExt;

                // noch nicht vorhanden?
                SetName( String(aTmpStr, osl_getThreadTextEncoding()) );

                if ( !Exists() )
                {
                    // Fehler setzen !!!
                    nError = CreateEntry_Impl( *this, eKind );
                    return (ERRCODE_NONE == nError);
                }
            }
        }

        // keine ## mehr frei / ?## soll nicht verwendet werden
        nError = ERRCODE_IO_ALREADYEXISTS;
        return sal_False;
}

/*************************************************************************
|*
|*    DirEntry::CreatePath()
|*
*************************************************************************/

sal_Bool DirEntry::MakeDir( sal_Bool bSloppy ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

        // Schnellpruefung, ob vorhanden
        if ( FileStat( *this ).IsKind( FSYS_KIND_DIR ) )
                return sal_True;
        if ( bSloppy && pParent )
                 if ( FileStat( *pParent ).IsKind( FSYS_KIND_DIR ) )
                          return sal_True;

        const DirEntry *pNewDir = bSloppy ? pParent : this;
        if ( pNewDir )
        {
                // den Path zum Dir erzeugen
                if ( pNewDir->pParent && !pNewDir->pParent->MakeDir(sal_False) )
                        return sal_False;

                // das Dir selbst erzeugen
                if ( pNewDir->eFlag == FSYS_FLAG_ABSROOT ||
                         pNewDir->eFlag == FSYS_FLAG_ABSROOT ||
                         pNewDir->eFlag == FSYS_FLAG_VOLUME )
                        return sal_True;
                else
                {
                        //? nError = ???
                        if ( FileStat( *pNewDir ).IsKind( FSYS_KIND_DIR ) )
                                return sal_True;
                        else
                        {
                                FSysFailOnErrorImpl();
                                String aDirName(pNewDir->GetFull());
#ifndef BOOTSTRAP
                                FSysRedirector::DoRedirect( aDirName );
#endif
                                ByteString bDirName( aDirName, osl_getThreadTextEncoding() );
                                bDirName = GUI2FSYS( bDirName );

#ifdef WIN32
                                SetLastError(0);
#endif
                                sal_Bool bResult = (0 == _mkdir( (char*) bDirName.GetBuffer() ));
                                if ( !bResult )
                                {
                                    // Wer hat diese Methode const gemacht ?
#ifdef WIN32
                                    ((DirEntry *)this)->SetError( Sys2SolarError_Impl(  GetLastError() ) );
#else
                                    ((DirEntry *)this)->SetError( Sys2SolarError_Impl(  errno ) );
#endif
                                }

                                return bResult;
                        }
                }
        }
        return sal_True;
}

/*************************************************************************
|*
|*    DirEntry::CopyTo()
|*
*************************************************************************/

FSysError DirEntry::CopyTo( const DirEntry& rDest, FSysAction nActions ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

        if ( FSYS_ACTION_COPYFILE != (nActions & FSYS_ACTION_COPYFILE) )
#ifdef UNX
    {
        // Hardlink anlegen
                HACK(redirection missing)
    ByteString aThis(GUI2FSYS(GetFull()), osl_getThreadTextEncoding());
    ByteString aDest(GUI2FSYS(rDest.GetFull()), osl_getThreadTextEncoding());
        if (link( aThis.GetBuffer(), aDest.GetBuffer() ) == -1)
            return Sys2SolarError_Impl(  errno );
        else
            return FSYS_ERR_OK;
    }
#else
        return FSYS_ERR_NOTSUPPORTED;
#endif

        FileCopier fc(*this, rDest);
        return fc.Execute(nActions);
}

/*************************************************************************
|*
|*    DirEntry::MoveTo()
|*
*************************************************************************/

#if defined WNT || defined UNX || defined OS2

FSysError DirEntry::MoveTo( const DirEntry& rNewName ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

/*
    FileStat aSourceStat(*this);
    if ( !aSourceStat.IsKind(FSYS_KIND_FILE) )
        return FSYS_ERR_NOTAFILE;
*/

    DirEntry aDest(rNewName);
    FileStat aDestStat(rNewName);
    if ( aDestStat.IsKind(FSYS_KIND_DIR ) )
    {
        aDest += String(aName, osl_getThreadTextEncoding());
    }
    if ( aDest.Exists() )
    {
        return FSYS_ERR_ALREADYEXISTS;
    }

#if defined(OS2)
    if ( FileStat(*this).IsKind(FSYS_KIND_DIR) && aDest.GetPath() != GetPath() )
    {
        return FSYS_ERR_NOTSUPPORTED;
    }
#endif

        FSysFailOnErrorImpl();
        String aFrom( GetFull() );

#ifndef BOOTSTRAP
        FSysRedirector::DoRedirect(aFrom);
#endif

        String aTo( aDest.GetFull() );

#ifndef BOOTSTRAP
        FSysRedirector::DoRedirect(aTo);
#endif

        ByteString bFrom(aFrom, osl_getThreadTextEncoding());
        ByteString bTo(aTo, osl_getThreadTextEncoding());
        bFrom = GUI2FSYS(bFrom);
        bTo = GUI2FSYS(bTo);

#ifdef WNT
        // MoveTo nun atomar
        SetLastError(0);

        DirEntry aFromDevice(String(bFrom, osl_getThreadTextEncoding()));
        DirEntry aToDevice(String(bTo,osl_getThreadTextEncoding()));
        aFromDevice.ToAbs();
        aToDevice.ToAbs();
        aFromDevice=aFromDevice.GetDevice();
        aToDevice=aToDevice.GetDevice();

        //Quelle und Ziel auf gleichem device?
        if (aFromDevice==aToDevice)
        {
            // ja, also intra-device-move mit MoveFile
            MoveFile( bFrom.GetBuffer(), bTo.GetBuffer() );
            // MoveFile ist buggy bei cross-device operationen.
            // Der R?ckgabewert ist auch dann sal_True, wenn nur ein Teil der Operation geklappt hat.
            // Zudem zeigt MoveFile unterschiedliches Verhalten bei unterschiedlichen NT-Versionen.
            return Sys2SolarError_Impl( GetLastError() );
        }
        else
        {
            //nein, also inter-device-move mit copy/delete
            FSysError nCopyError = CopyTo(rNewName, FSYS_ACTION_COPYFILE);

            DirEntry aKill(String(bTo, osl_getThreadTextEncoding()));
            FileStat aKillStat(String(bTo, osl_getThreadTextEncoding()));
            if ( aKillStat.IsKind(FSYS_KIND_DIR ) )
            {
                aKill += String(aName, osl_getThreadTextEncoding());
            }

            if (nCopyError==FSYS_ERR_OK)
            {
                if (Kill()==FSYS_ERR_OK)
                {
                    return FSYS_ERR_OK;
                }
                else
                {
                    aKill.Kill();
                    return FSYS_ERR_ACCESSDENIED;
                }
            }
            else
            {
                aKill.Kill();
                return nCopyError;
            }
        }
#else
        // #68639#
        // on some nfs connections rename with from == to
        // leads to destruction of file
        if ( ( aFrom != aTo ) && ( 0 != rename( bFrom.GetBuffer(), bTo.GetBuffer() ) ) )
#if !defined(UNX) && !defined(OS2)
            return Sys2SolarError_Impl( GetLastError() );
#else
        {
                if( errno == EXDEV )
// cross device geht latuernich nicht mit rename
                {
                        FILE *fpIN  = fopen( bFrom.GetBuffer(), "r" );
                        FILE *fpOUT = fopen( bTo.GetBuffer(), "w" );
                        if( fpIN && fpOUT )
                        {
                                char pBuf[ 16384 ];
                                int nBytes, nWritten, nErr = 0;
                                errno = 0;
                                while( ( nBytes = fread( pBuf, 1, sizeof(pBuf), fpIN ) ) && ! nErr )
                                {
                                    nWritten = fwrite( pBuf, 1, nBytes, fpOUT );
                                    // Fehler im fwrite     ?
                                    if( nWritten < nBytes )
                                    {
                                        nErr = errno;
                                        break;
                                    }
                                }
                                fclose( fpIN );
                                fclose( fpOUT );
                                if ( nErr )
                                {
                                    unlink( bTo.GetBuffer() );
                                    return Sys2SolarError_Impl( nErr );
                                }
                                else
                                {
                                    unlink( bFrom.GetBuffer() );
                                }
                        }
                        else
                        {
                            return Sys2SolarError_Impl( EXDEV );
                        }
                }
                else
                {
                    return Sys2SolarError_Impl( errno );
                }
        }
#endif
#endif
        return ERRCODE_NONE;
}

#endif

/*************************************************************************
|*
|*    DirEntry::Kill()
|*
*************************************************************************/

FSysError DirEntry::Kill(  FSysAction nActions ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

        FSysError eError = FSYS_ERR_OK;
        FSysFailOnErrorImpl();

        // Name als doppelt 0-terminierter String
        String aTmpName( GetFull() );
#ifndef BOOTSTRAP
        FSysRedirector::DoRedirect( aTmpName );
#endif
        ByteString bTmpName( aTmpName, osl_getThreadTextEncoding());
        bTmpName = GUI2FSYS(bTmpName);

        char *pName = new char[bTmpName.Len()+2];
        strcpy( pName, bTmpName.GetBuffer() );
        pName[bTmpName.Len()+1] = (char) 0;

        //read-only files sollen auch geloescht werden koennen
        sal_Bool isReadOnly = FileStat::GetReadOnlyFlag(*this);
        if (isReadOnly)
        {
            FileStat::SetReadOnlyFlag(*this, sal_False);
        }

        // directory?
        if ( FileStat( *this ).IsKind(FSYS_KIND_DIR) )
        {
                // Inhalte recursiv loeschen?
                if ( FSYS_ACTION_RECURSIVE == (nActions & FSYS_ACTION_RECURSIVE) )
                {
                        Dir aDir( *this, FSYS_KIND_DIR|FSYS_KIND_FILE );
                        for ( sal_uInt16 n = 0; eError == FSYS_ERR_OK && n < aDir.Count(); ++n )
                        {
                                const DirEntry &rSubDir = aDir[n];
                                DirEntryFlag flag = rSubDir.GetFlag();
                                if ( flag != FSYS_FLAG_CURRENT && flag != FSYS_FLAG_PARENT )
                                        eError = rSubDir.Kill(nActions);
                        }
                }

                // das Dir selbst loeschen
#ifdef WIN32
                SetLastError(0);
#endif
                if ( eError == FSYS_ERR_OK && 0 != _rmdir( (char*) pName ) )
                //
                {
                        // falls L"oschen nicht ging, CWD umsetzen
#ifdef WIN32
                    eError = Sys2SolarError_Impl( GetLastError() );
#else
                    eError = Sys2SolarError_Impl( errno );
#endif
                        if ( eError )
                        {
                                GetPath().SetCWD();
#ifdef WIN32
                                SetLastError(0);
#endif
                                if (_rmdir( (char*) pName) != 0)
                                {
#ifdef WIN32
                                    eError = Sys2SolarError_Impl( GetLastError() );
#else
                                    eError = Sys2SolarError_Impl( errno );
#endif
                                }
                                else
                                {
                                    eError = FSYS_ERR_OK;
                                }
                        }
                }
        }
        else
        {
                if ( FSYS_ACTION_USERECYCLEBIN == (nActions & FSYS_ACTION_USERECYCLEBIN) )
                {
#ifdef OS2
                        eError = ApiRet2ToSolarError_Impl( DosDelete( (PSZ) pName ) );
#elif defined(WNT)
                        SHFILEOPSTRUCT aOp;
                        aOp.hwnd = 0;
                        aOp.wFunc = FO_DELETE;
                        aOp.pFrom = pName;
                        aOp.pTo = 0;
                        aOp.fFlags = FOF_ALLOWUNDO|FOF_SILENT|FOF_NOCONFIRMATION;
                        aOp.hNameMappings = 0;
                        aOp.lpszProgressTitle = 0;
                        eError = Sys2SolarError_Impl( SHFileOperation( &aOp ) );
#else
                        eError = ERRCODE_IO_NOTSUPPORTED;
#endif
                }
                else
                {
#ifdef WIN32
                    SetLastError(0);
#endif
                    if ( 0 != _unlink( (char*) pName ) )
                    {
#ifdef WIN32
                        eError = Sys2SolarError_Impl( GetLastError() );
#else
                        eError = Sys2SolarError_Impl( errno );
#endif
                    }
                    else
                    {
                        eError = ERRCODE_NONE;
                    }
                }
        }

        //falls Fehler, originales read-only flag wieder herstellen
        if ( isReadOnly && (eError!=ERRCODE_NONE) )
        {
            FileStat::SetReadOnlyFlag(*this, isReadOnly);
        }

        delete[] pName;
        return eError;
}

/*************************************************************************
|*
|*    DirEntry::Contains()
|*
|*    Beschreibung      ob rSubEntry direkt oder indirect in *this liegt
|*
*************************************************************************/

sal_Bool DirEntry::Contains( const DirEntry &rSubEntry ) const
{
    DBG_ASSERT( IsAbs() && rSubEntry.IsAbs(), "must be absolute entries" );

        sal_uInt16 nThisLevel = Level();
    sal_uInt16 nSubLevel = rSubEntry.Level();
    if ( nThisLevel < nSubLevel )
    {
        for ( ; nThisLevel; --nThisLevel, --nSubLevel )
            if ( (*this)[nThisLevel-1] != rSubEntry[nSubLevel-1] )
                return sal_False;
        return sal_True;
    }
    return sal_False;
}

/*************************************************************************
|*
|*    DirEntry::Level()
|*
*************************************************************************/

sal_uInt16 DirEntry::Level() const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    sal_uInt16 nLevel = 0;
    const DirEntry *pRes = this;
    while ( pRes )
    {
        pRes = pRes->pParent;
        nLevel++;
    }

    return nLevel;
}

/*************************************************************************
|*
|*    DirEntry::ConvertNameToSystem()
|*
*************************************************************************/

String DirEntry::ConvertNameToSystem( const String &rName )
{
    return rName;
}

/*************************************************************************
|*
|*    DirEntry::ConvertSystemToName()
|*
*************************************************************************/

String DirEntry::ConvertSystemToName( const String &rName )
{
    return rName;
}

/*************************************************************************
|*
|*    DirEntry::IsValid()
|*
*************************************************************************/

sal_Bool DirEntry::IsValid() const
{
        return (nError == FSYS_ERR_OK);
}

/*************************************************************************
|*
|*    DirEntry::IsRFSAvailable()
|*
*************************************************************************/

sal_Bool DirEntry::IsRFSAvailable()
{
    return sal_False;
}

/*************************************************************************
|*
|*    IsLongNameOnFAT()
|*
|*    Beschreibung      ?berpr?ft , ob das DirEntry einen langen
|*                      Filenamen auf einer FAT-Partition enth?lt (EAs).
|*                      (eigentlich nur f?r OS2 interessant)
|*
*************************************************************************/

sal_Bool DirEntry::IsLongNameOnFAT() const
{
        // FAT-System?
        DirEntry aTempDirEntry(*this);
        aTempDirEntry.ToAbs();
        if (DirEntry::GetPathStyle(aTempDirEntry.GetDevice().GetName().GetChar(0)) != FSYS_STYLE_FAT)
        {
            return sal_False;       // nein, also false
        }

        // DirEntry-Kette auf lange Dateinamen pr?fen
        for( sal_uInt16 iLevel = this->Level(); iLevel > 0; iLevel-- )
        {
            const DirEntry& rEntry = (const DirEntry&) (*this)[iLevel-1];
            String  aBase( rEntry.GetBase() );
            String  aExtension( rEntry.GetExtension() );

            if (aBase.Len()>8)  // Name > 8?
            {
                return sal_True;
            }

            if (aExtension.Len()>3) // Extension > 3?
            {
                return sal_True;
            }
        }
        return sal_False;
}

//========================================================================

#if defined(DBG_UTIL)

void FSysTest()
{
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
