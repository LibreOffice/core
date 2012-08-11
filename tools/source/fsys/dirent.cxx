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

#if !defined UNX
#ifdef WNT
#include <windows.h>
#undef GetObject
#endif
#include <io.h>
#include <process.h>
#endif

#if defined(UNX)
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
#include <vector>

#ifdef UNX
#define _MAX_PATH 260
#endif

#include <tools/stream.hxx>
#include <osl/mutex.hxx>
#include <osl/file.hxx>
#include <rtl/instance.hxx>
#include <comphelper/string.hxx>

using namespace osl;
using ::rtl::OUString;

int ApiRet2ToSolarError_Impl( int nApiRet );

int Sys2SolarError_Impl( int nSysErr )
{
    switch ( nSysErr )
    {
#ifdef WNT
        case NO_ERROR:                  return ERRCODE_NONE;
        case ERROR_INVALID_FUNCTION:    return ERRCODE_IO_GENERAL;
        case ERROR_FILE_NOT_FOUND:      return ERRCODE_IO_NOTEXISTS;
        case ERROR_PATH_NOT_FOUND:      return ERRCODE_IO_NOTEXISTSPATH;
        case ERROR_TOO_MANY_OPEN_FILES: return ERRCODE_IO_TOOMANYOPENFILES;
        case ERROR_ACCESS_DENIED:       return ERRCODE_IO_ACCESSDENIED;
        case ERROR_INVALID_HANDLE:      return ERRCODE_IO_GENERAL;
        case ERROR_NOT_ENOUGH_MEMORY:   return ERRCODE_IO_OUTOFMEMORY;
        case ERROR_INVALID_BLOCK:       return ERRCODE_IO_GENERAL;
        case ERROR_BAD_FORMAT:          return ERRCODE_IO_WRONGFORMAT;
        case ERROR_INVALID_ACCESS:      return ERRCODE_IO_ACCESSDENIED;
        case ERROR_INVALID_DRIVE:       return ERRCODE_IO_INVALIDDEVICE;
        case ERROR_CURRENT_DIRECTORY:   return ERRCODE_IO_CURRENTDIR;
        case ERROR_NOT_SAME_DEVICE:     return ERRCODE_IO_NOTSAMEDEVICE;
        case ERROR_WRITE_PROTECT:       return ERRCODE_IO_CANTWRITE;
        case ERROR_BAD_UNIT:            return ERRCODE_IO_INVALIDDEVICE;
        case ERROR_NOT_READY:           return ERRCODE_IO_DEVICENOTREADY;
        case ERROR_BAD_COMMAND:         return ERRCODE_IO_GENERAL;
        case ERROR_CRC:                 return ERRCODE_IO_BADCRC;
        case ERROR_BAD_LENGTH:          return ERRCODE_IO_INVALIDLENGTH;
        case ERROR_SEEK:                return ERRCODE_IO_CANTSEEK;
        case ERROR_NOT_DOS_DISK:        return ERRCODE_IO_WRONGFORMAT;
        case ERROR_SECTOR_NOT_FOUND:    return ERRCODE_IO_GENERAL;
        case ERROR_WRITE_FAULT:         return ERRCODE_IO_CANTWRITE;
        case ERROR_READ_FAULT:          return ERRCODE_IO_CANTREAD;
        case ERROR_GEN_FAILURE:         return ERRCODE_IO_GENERAL;
        case ERROR_SHARING_VIOLATION:   return ERRCODE_IO_LOCKVIOLATION;
        case ERROR_LOCK_VIOLATION:      return ERRCODE_IO_LOCKVIOLATION;
        case ERROR_WRONG_DISK:          return ERRCODE_IO_INVALIDDEVICE;
        case ERROR_NOT_SUPPORTED:       return ERRCODE_IO_NOTSUPPORTED;
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

class DirEntryStack
{
private:
    ::std::vector< DirEntry* >  maStack;

public:
                        DirEntryStack() {};
                        ~DirEntryStack();

    inline  void        Push( DirEntry *pEntry );
    inline  DirEntry*   Pop();
    inline  DirEntry*   Top();
    inline  DirEntry*   Bottom();
    inline  bool        Empty();
    inline  void        Clear();
};

inline void DirEntryStack::Push( DirEntry *pEntry )
{
    maStack.push_back( pEntry );
}

inline DirEntry* DirEntryStack::Pop()
{
    DirEntry*   pEntry = NULL;
    if ( !maStack.empty() ) {
        pEntry = maStack.back();
        maStack.pop_back();
    }
    return pEntry;
}

inline DirEntry* DirEntryStack::Top()
{
    return maStack.empty() ? NULL : maStack.back();
}

inline DirEntry* DirEntryStack::Bottom()
{
    return maStack.empty() ? NULL : maStack.front();
}

inline bool DirEntryStack::Empty()
{
    return maStack.empty();
}

inline void DirEntryStack::Clear()
{
    maStack.clear();
}

DBG_NAME( DirEntry );

DirEntryStack::~DirEntryStack()
{
    maStack.clear();
}

#ifdef DBG_UTIL
/** Check DirEntry for DBG_UTIL

    @param p Pointer to DirEntry
    @return char* Error-TExtension or NULL
*/
const char* ImpCheckDirEntry( const void* p )
{
    DirEntry* p0 = (DirEntry*)p;

    if ( p0->pParent )
        DBG_CHKOBJ( p0->pParent, DirEntry, ImpCheckDirEntry );

    return NULL;
}
#endif

/** Insert "..." for max length of nMaxChars */
rtl::OString ImplCutPath( const rtl::OString& rStr, sal_Int32 nMax, char cAccDel )
{
    sal_Int32 nMaxPathLen = nMax;
    sal_Bool bInsertPrefix = sal_False;
    sal_Int32 nBegin = rStr.indexOf(cAccDel);
    rtl::OStringBuffer aCutPath(rStr);

    if( nBegin == -1 )
        nBegin = 0;
    else
        nMaxPathLen += 2;   // Prefix <Disk>:

    while( aCutPath.getLength() > nMaxPathLen )
    {
        sal_Int32 nEnd = aCutPath.toString().indexOf(cAccDel, nBegin + 1);
        sal_Int32 nCount;

        if ( nEnd != -1 )
        {
            nCount = nEnd - nBegin;
            aCutPath.remove(nBegin, nCount);
            bInsertPrefix = sal_True;
        }
        else
            break;
    }

    if ( aCutPath.getLength() > nMaxPathLen )
    {
        for ( sal_Int32 n = nMaxPathLen; n > nMaxPathLen/2; --n )
        {
            if (!comphelper::string::isalnumAscii(aCutPath[n]))
            {
                comphelper::string::truncateToLength(aCutPath, n);
                aCutPath.append(RTL_CONSTASCII_STRINGPARAM("..."));
                break;
            }
        }
    }

    if ( bInsertPrefix )
    {
        rtl::OStringBuffer aIns;
        aIns.append(cAccDel).append(RTL_CONSTASCII_STRINGPARAM("..."));
        aCutPath.insert(nBegin, aIns.makeStringAndClear());
    }

    return aCutPath.makeStringAndClear();
}

FSysError DirEntry::ImpParseName( const rtl::OString& rPfad )
{
#if defined(WNT)
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    // put single names onto stack
    String aPfad(rtl::OStringToOUString(rPfad, osl_getThreadTextEncoding()));
    DirEntryStack   aStack;

    do
    {
        // split name before first "\\",
        // if '\\' is at beginning of string, name is set to '\\'.
        // ":" also splits the string and belongs to the name.

        // search first occurance of '\\', '/' or ':'
        sal_uInt16 nPos;
        for ( nPos = 0;
              nPos < aPfad.Len() &&                             //?O
                  aPfad.GetChar(nPos) != '\\' && aPfad.GetChar(nPos) != '/' &&      //?O
                  aPfad.GetChar(nPos) != ':';                               //?O
              nPos++ )
            /* do nothing */;

        // is the name a UNC pathname?
        if ( nPos == 0 && aPfad.Len() > 1 &&
             ( ( aPfad.GetChar(0) == '\\' && aPfad.GetChar(1) == '\\' ) ||
               ( aPfad.GetChar(0) == '/' && aPfad.GetChar(1) == '/' ) ) )
        {
            for ( nPos = 2; aPfad.Len() > nPos; ++nPos )
                if ( aPfad.GetChar(nPos) == '\\' || aPfad.GetChar(nPos) == '/' )
                    break;
            aName = rtl::OUStringToOString(aPfad.Copy( 2, nPos-2 ), osl_getThreadTextEncoding());
            aStack.Push( new DirEntry( aName, FSYS_FLAG_ABSROOT ) );
        }
        // Is the name the root of the current drive?
        else if ( nPos == 0 && aPfad.Len() > 0 &&
                  ( aPfad.GetChar(0) == '\\' || aPfad.GetChar(0) == '/' ) )
        {
            // Push root directory of current drive
            aStack.Push( new DirEntry( FSYS_FLAG_ABSROOT ) );
        }
        else
        {
            // Is the name itself a drive?
            if ( nPos < aPfad.Len() && aPfad.GetChar(nPos) == ':' )
            {
                aName = rtl::OUStringToOString(aPfad.Copy( 0, nPos + 1 ), osl_getThreadTextEncoding());

                // Is the name the root of a drive?
                if ( (nPos + 1) < aPfad.Len() &&
                     ( aPfad.GetChar(nPos+1) == '\\' || aPfad.GetChar(nPos+1) == '/' ) )
                {
                    // unsupported if stack not empty or is a Novell format (URL)
                    if ( !aStack.Empty() || aName.getLength() > 2 )
                    {
                        aName = rPfad;
                        return FSYS_ERR_MISPLACEDCHAR;
                    }
                    // Push as root directory of drive
                    aStack.Push( new DirEntry( aName, FSYS_FLAG_ABSROOT ) );
                }
                else
                {
                    // clear stack if another drive is currently on it
                    if ( !aStack.Empty() )
                    {
                        rtl::OString aThis(aStack.Bottom()->aName);
                        aThis = aThis.toAsciiLowerCase();
                        rtl::OString aOther(aName);
                        aOther = aOther.toAsciiLowerCase();
                        if (aThis.compareTo(aOther) != 0)
                            aStack.Clear();
                    }

                    if ( aStack.Empty() )
                        aStack.Push( new DirEntry( aName, FSYS_FLAG_RELROOT ) );
                }
            }
            // Name is not a drive
            else
            {
                // split the name without seperator
                aName = rtl::OUStringToOString(aPfad.Copy( 0, nPos ), osl_getThreadTextEncoding());

                // Is the name the current directory?
                if ( aName == "." )
                    /* do nothing */;

                // Is the name the parent directory?
                else if ( aName == ".." )
                {
                    // Is the stack empty, or a parent (or relative root) on it?
                    if ( ( aStack.Empty() ) ||
                         ( aStack.Top()->eFlag == FSYS_FLAG_PARENT ) ||
                         ( aStack.Top()->eFlag == FSYS_FLAG_RELROOT ) )
                        // add leading parent to stack
                        aStack.Push( new DirEntry( FSYS_FLAG_PARENT ) );

                    // It's an absolute root path
                    else if ( aStack.Top()->eFlag == FSYS_FLAG_ABSROOT )
                    {
                        // Then there is no parent directory
                        aName = rPfad;
                        return FSYS_ERR_NOTEXISTS;
                    }
                    else
                        // Otherwise removee parent TOS
                        delete aStack.Pop();
                }

                else
                {
                    // add ordinary entries to the stack
                    DirEntry *pNew = new DirEntry( aName, FSYS_FLAG_NORMAL );
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

        // determine remainder of path
        aPfad.Erase( 0, nPos + 1 );
        while ( aPfad.Len() && ( aPfad.GetChar(0) == '\\' || aPfad.GetChar(0) == '/' ) )
            aPfad.Erase( 0, 1 );
    }
    while ( aPfad.Len() );

    sal_uIntPtr nErr = ERRCODE_NONE;
    // Set the main entry itself
    if ( aStack.Empty() )
    {
        eFlag = FSYS_FLAG_CURRENT;
        aName = rtl::OString();
    }
    else
    {
        eFlag = aStack.Top()->eFlag;
        aName = aStack.Top()->aName;
        nErr = aStack.Top()->nError;
        delete aStack.Pop();
    }

    // pop parent entry from stack
    DirEntry** pTemp = &pParent;
    while ( !aStack.Empty() )
    {
        *pTemp = aStack.Pop();

        // set member pointer to the pParent of the member's own parent
        pTemp = &( (*pTemp)->pParent );
    }

    // Does this describe a volume?
    if ( !pParent && eFlag == FSYS_FLAG_RELROOT && !aName.isEmpty() )
        eFlag = FSYS_FLAG_VOLUME;

    // use full aName if error code was set
    if ( nErr )
        aName = rPfad;
    return nErr;
#else
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    // Add single names to the stack
    DirEntryStack   aStack;
    rtl::OString aPfad(rPfad);
    do
    {
        // split names on the first occurance of "/",
        // if '/' starts the string, it itself becomes the name

        // search first occurance of "/"
        sal_uInt16 nPos;
        for ( nPos = 0;
              nPos < aPfad.getLength() && aPfad[nPos] != '/';
              nPos++ )
            /* do nothing */;

        // is the name the root of the current drive?
        if ( nPos == 0 && !aPfad.isEmpty() && ( aPfad[0] == '/' ) )
        {
            // push root directory of current drive to stack
            aStack.Push( new DirEntry( FSYS_FLAG_ABSROOT ) );
        }
        else
        {
            // split name without seperator
            aName = aPfad.copy(0, nPos);

            // Is the name the current directory?
            if ( aName == "." )
                /* do nothing */;

#ifdef UNX
            // Is the name the user's home directory?
            else if ( aName == "~" )
            {
                DirEntry aHome( String( (const char *) getenv( "HOME" ), osl_getThreadTextEncoding()) );
                for ( sal_uInt16 n = aHome.Level(); n; --n )
                    aStack.Push( new DirEntry( aHome[ (sal_uInt16) n-1 ] ) );
            }
#endif
            // Is the name the current parent directory?
            else if ( aName == ".." )
            {
                // Is the stack empty, or a parent (or relative root) is on top?
                if ( ( aStack.Empty() ) || ( aStack.Top()->eFlag == FSYS_FLAG_PARENT ) )
                {
                    // push leading parents to stack
                    aStack.Push( new DirEntry(rtl::OString(), FSYS_FLAG_PARENT) );
                }
                // Is the name an absolute root?
                else if ( aStack.Top()->eFlag == FSYS_FLAG_ABSROOT )
                {
                    // they do not have parent directories
                    return FSYS_ERR_NOTEXISTS;
                }
                else
                    // otherwise remove parent TOS from stack
                    delete aStack.Pop();
            }
            else
            {
                DirEntry *pNew = NULL;
                // push ordinary entries on the stack
                pNew = new DirEntry( aName, FSYS_FLAG_NORMAL );
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

        // get remainder of path
        aPfad = nPos < aPfad.getLength()
            ? aPfad.copy(nPos + 1) : rtl::OString();
        while ( !aPfad.isEmpty() && ( aPfad[0] == '/' ) )
            aPfad = aPfad.copy(1);
    }
    while (!aPfad.isEmpty());

    // insert main entry itself
    if ( aStack.Empty() )
    {
        eFlag = FSYS_FLAG_CURRENT;
        aName = rtl::OString();
    }
    else
    {
        eFlag = aStack.Top()->eFlag;
        aName = aStack.Top()->aName;
        delete aStack.Pop();
    }

    // Get parent entries from stack
    DirEntry** pTemp = &pParent;
    while ( !aStack.Empty() )
    {
        *pTemp = aStack.Pop();
        pTemp = &( (*pTemp)->pParent );
    }

    return FSYS_ERR_OK;
#endif
}

static FSysPathStyle GetStyle( FSysPathStyle eStyle )
{
    if ( eStyle == FSYS_STYLE_HOST || eStyle == FSYS_STYLE_DETECT )
        return DEFSTYLE;
    else
        return eStyle;
}

/** Convert name to match OS norm. */
void DirEntry::ImpTrim()
{
    // Do not trim wildcard characters
    if ( ( aName.indexOf( '*' ) != -1 ) ||
         ( aName.indexOf( '?' ) != -1 ) ||
         ( aName.indexOf( ';' ) != -1 ) )
        return;

#if defined(WNT)
    if ( aName.getLength() > 254 )
    {
        nError = ERRCODE_IO_MISPLACEDCHAR|ERRCODE_WARNING_MASK;
        aName = aName.copy(254);
    }
#else
    if ( aName.getLength() > 250 )
    {
        nError = ERRCODE_IO_MISPLACEDCHAR|ERRCODE_WARNING_MASK;
        aName = aName.copy(250);
    }
#endif
}

DirEntry::DirEntry( const rtl::OString& rName, DirEntryFlag eDirFlag ) :
#ifdef FEAT_FSYS_DOUBLESPEED
            pStat( 0 ),
#endif
            aName( rName )
{
    DBG_CTOR( DirEntry, ImpCheckDirEntry );

    pParent         = NULL;
    eFlag           = eDirFlag;
    nError          = FSYS_ERR_OK;

    ImpTrim();
}

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

DirEntry::DirEntry( const String& rInitName, FSysPathStyle eStyle )
#ifdef FEAT_FSYS_DOUBLESPEED
            : pStat( 0 )
#endif
{
    DBG_CTOR( DirEntry, ImpCheckDirEntry );

    (void) eStyle; // only used for DBG_UTIL

    pParent         = NULL;

    // faster check for empty string
    if ( !rInitName.Len())
    {
        eFlag                   = FSYS_FLAG_CURRENT;
        nError                  = FSYS_ERR_OK;
        return;
    }

    rtl::OString aTmpName(rtl::OUStringToOString(rInitName, osl_getThreadTextEncoding()));
    if (aTmpName.matchIgnoreAsciiCase(rtl::OString(RTL_CONSTASCII_STRINGPARAM("file:"))))
    {
        DBG_WARNING( "File URLs are not permitted but accepted" );
        aTmpName = rtl::OUStringToOString(INetURLObject( rInitName ).PathToFileName(), osl_getThreadTextEncoding());
#ifdef DBG_UTIL
                eStyle = FSYS_STYLE_HOST;
#endif
    }
    else
    {
        ::rtl::OUString aTmp;
        ::rtl::OUString aOInitName;
        if ( FileBase::getFileURLFromSystemPath( OUString( rInitName ), aTmp ) == FileBase::E_None )
        {
            aOInitName = OUString( rInitName );
            aTmpName = rtl::OUStringToOString(aOInitName, osl_getThreadTextEncoding());
        }

#ifdef DBG_UTIL
        if (eStyle == FSYS_STYLE_HOST && aTmpName.indexOf( "://" ) != -1)
        {
            rtl::OStringBuffer aErr(RTL_CONSTASCII_STRINGPARAM("DirEntries akzeptieren nur File URLS: "));
            aErr.append(aTmpName);
            DBG_WARNING(aErr.getStr());
        }
#endif
    }

    nError  = ImpParseName( aTmpName );

    if ( nError != FSYS_ERR_OK )
        eFlag = FSYS_FLAG_INVALID;
}

DirEntry::DirEntry( const rtl::OString& rInitName, FSysPathStyle eStyle )
#ifdef FEAT_FSYS_DOUBLESPEED
            : pStat( 0 )
#endif
{
    DBG_CTOR( DirEntry, ImpCheckDirEntry );

    (void) eStyle; // only used for DBG_UTIL

    pParent         = NULL;

    // faster check for empty string
    if ( rInitName.isEmpty() )
    {
        eFlag                   = FSYS_FLAG_CURRENT;
        nError                  = FSYS_ERR_OK;
        return;
    }

    rtl::OString aTmpName( rInitName );
    if (aTmpName.matchIgnoreAsciiCase(rtl::OString(RTL_CONSTASCII_STRINGPARAM("file:"))))
    {
        DBG_WARNING( "File URLs are not permitted but accepted" );
        aTmpName = rtl::OUStringToOString(INetURLObject( rInitName ).PathToFileName(), osl_getThreadTextEncoding());
#ifdef DBG_UTIL
        eStyle = FSYS_STYLE_HOST;
#endif
    }
#ifdef DBG_UTIL
    else
    {
        if( eStyle == FSYS_STYLE_HOST && rInitName.indexOf("://") != -1 )
        {
            rtl::OStringBuffer aErr(RTL_CONSTASCII_STRINGPARAM("DirEntries akzeptieren nur File URLS: "));
            aErr.append(rInitName);
            DBG_WARNING(aErr.getStr());
        }
    }
#endif

    nError  = ImpParseName( aTmpName );

    if ( nError != FSYS_ERR_OK )
        eFlag = FSYS_FLAG_INVALID;
}

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

DirEntry::~DirEntry()
{
    DBG_DTOR( DirEntry, ImpCheckDirEntry );

    delete pParent;
#ifdef FEAT_FSYS_DOUBLESPEED
    delete pStat;
#endif

}

const DirEntry* DirEntry::ImpGetTopPtr() const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    const DirEntry *pTemp = this;
    while ( pTemp->pParent )
        pTemp = pTemp->pParent;

    return pTemp;
}

DirEntry* DirEntry::ImpGetTopPtr()
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    DirEntry *pTemp = this;
    while ( pTemp->pParent )
        pTemp = pTemp->pParent;

    return pTemp;
}

DirEntry* DirEntry::ImpChangeParent( DirEntry* pNewParent, sal_Bool bNormalize )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    DirEntry *pTemp = pParent;
    if ( bNormalize && pNewParent &&
         pNewParent->eFlag == FSYS_FLAG_RELROOT && pNewParent->aName.isEmpty() )
    {
        pParent = 0;
        delete pNewParent;
    }
    else
        pParent = pNewParent;

    return pTemp;
}

sal_Bool DirEntry::Exists( FSysAccess nAccess ) const
{
    static osl::Mutex aLocalMutex;
    osl::MutexGuard aGuard( aLocalMutex );
        if ( !IsValid() )
                return sal_False;

#if defined WNT
    // get special file names from system
    if ( aName.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("CLOCK$")) ||
           aName.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("CON")) ||
           aName.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("AUX")) ||
           aName.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("COM1")) ||
           aName.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("COM2")) ||
           aName.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("COM3")) ||
           aName.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("COM4")) ||
           aName.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("LPT1")) ||
           aName.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("LPT2")) ||
           aName.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("LPT3")) ||
           aName.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("NUL")) ||
           aName.equalsIgnoreAsciiCaseL(RTL_CONSTASCII_STRINGPARAM("PRN")) )
        return sal_True;
#endif

        FSysFailOnErrorImpl();
        DirEntryKind eKind = FileStat( *this, nAccess ).GetKind();
        if ( eKind & ( FSYS_KIND_FILE | FSYS_KIND_DIR ) )
        {
            return sal_True;
        }

#if defined WNT
        if ( 0 != ( eKind & FSYS_KIND_DEV ) )
        {
            return DRIVE_EXISTS( ImpGetTopPtr()->aName[0] );
        }
#endif

        return 0 != ( eKind & ( FSYS_KIND_FILE | FSYS_KIND_DIR ) );
}

sal_Bool DirEntry::First()
{
    FSysFailOnErrorImpl();

        String    aUniPathName( GetPath().GetFull() );
        rtl::OString aPathName(rtl::OUStringToOString(aUniPathName, osl_getThreadTextEncoding()));

        DIR *pDir = opendir(aPathName.getStr());
        if ( pDir )
        {
                WildCard aWildeKarte(rtl::OStringToOUString(CMP_LOWER(aName), osl_getThreadTextEncoding()));
                for ( dirent* pEntry = readdir( pDir );
                          pEntry;
                          pEntry = readdir( pDir ) )
                {
                        rtl::OString aFound(pEntry->d_name);
                        if (aWildeKarte.Matches(rtl::OStringToOUString(CMP_LOWER(aFound), osl_getThreadTextEncoding())))
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

String DirEntry::GetFull( FSysPathStyle eStyle, sal_Bool bWithDelimiter,
                          sal_uInt16 nMaxChars ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    rtl::OStringBuffer aBuf;
    eStyle = GetStyle( eStyle );
    if ( pParent )
    {
        if ( ( pParent->eFlag == FSYS_FLAG_ABSROOT ||
               pParent->eFlag == FSYS_FLAG_RELROOT ||
               pParent->eFlag == FSYS_FLAG_VOLUME ) )
        {
            aBuf.append(rtl::OUStringToOString(pParent->GetName( eStyle ), osl_getThreadTextEncoding()));
            aBuf.append(rtl::OUStringToOString(GetName( eStyle ), osl_getThreadTextEncoding()));
        }
        else
        {
            aBuf.append(rtl::OUStringToOString(pParent->GetFull( eStyle ), osl_getThreadTextEncoding()));
            aBuf.append(ACCESSDELIM_C(eStyle));
            aBuf.append(rtl::OUStringToOString(GetName( eStyle ), osl_getThreadTextEncoding()));
        }
    }
    else
    {
        aBuf.append(rtl::OUStringToOString(GetName(eStyle), osl_getThreadTextEncoding()));
    }

    //! Hack
    if ( bWithDelimiter )
        if ( aBuf[aBuf.getLength()-1] != ACCESSDELIM_C(eStyle) )
            aBuf.append(ACCESSDELIM_C(eStyle));

    rtl::OString aRet = aBuf.makeStringAndClear();

    // HACK
    if ( nMaxChars < STRING_MAXLEN )
        aRet = ImplCutPath( aRet, nMaxChars, ACCESSDELIM_C(eStyle) );

    return rtl::OStringToOUString(aRet, osl_getThreadTextEncoding());
}

DirEntry DirEntry::GetPath() const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    if ( pParent )
        return DirEntry( *pParent );

    return DirEntry();
}

String DirEntry::GetExtension( char cSep ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    const char *p0 = aName.getStr();
    const char *p1 = p0 + aName.getLength() - 1;
    while ( p1 >= p0 && *p1 != cSep )
    p1--;

    if ( p1 >= p0 )
    {
        // found a cSep at position p1
        return rtl::OStringToOUString(aName.copy(p1 - p0 + 1),
            osl_getThreadTextEncoding());
    }

    return String();
}

String DirEntry::GetBase( char cSep ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    const char *p0 = aName.getStr();
    const char *p1 = p0 + aName.getLength() - 1;
    while ( p1 >= p0 && *p1 != cSep )
        p1--;

    if ( p1 >= p0 )
    {
        // found a cSep at position p1
        return rtl::OStringToOUString(aName.copy(0, p1 - p0),
            osl_getThreadTextEncoding());

    }
    // did not find a cSep
    return rtl::OStringToOUString(aName, osl_getThreadTextEncoding());
}

String DirEntry::GetName( FSysPathStyle eStyle ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    rtl::OStringBuffer aRet;
    eStyle = GetStyle( eStyle );

    switch( eFlag )
    {
        case FSYS_FLAG_PARENT:
            aRet.append(ACTPARENT(eStyle));
            break;

        case FSYS_FLAG_ABSROOT:
        {
            aRet.append(aName);
            aRet.append(ACCESSDELIM_C(eStyle));
            break;
        }

        case FSYS_FLAG_INVALID:
        case FSYS_FLAG_VOLUME:
        {
            aRet.append(aName);
            break;
        }

        case FSYS_FLAG_RELROOT:
            if ( aName.isEmpty() )
            {
                aRet.append(ACTCURRENT(eStyle));
                break;
            }

        default:
            aRet.append(aName);
            break;
    }

    return rtl::OStringToOUString(aRet.makeStringAndClear(),
        osl_getThreadTextEncoding());
}

bool DirEntry::IsAbs() const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

#ifdef UNX
    return ( pParent ? pParent->IsAbs() : eFlag == FSYS_FLAG_ABSROOT );
#else
    return ( pParent ? pParent->IsAbs() : eFlag == FSYS_FLAG_ABSROOT && !aName.isEmpty() );
#endif
}

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
            aName = rtl::OString();
        }
    }
    else
    {
        eFlag = FSYS_FLAG_CURRENT;
        aName = rtl::OString();
        delete pParent;
        pParent = NULL;
    }

    return aOldName;
}

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

    const DirEntry *pThis = (DirEntry *)this;
    const DirEntry *pWith = (DirEntry *)&rEntry;
    while( pThis && pWith && (pThis->eFlag == pWith->eFlag) )
    {
        if ( CMP_LOWER(pThis->aName) != CMP_LOWER(pWith->aName) )
            break;
        pThis = pThis->pParent;
        pWith = pWith->pParent;
    }

    return ( !pThis && !pWith );
}

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

    // set name and type, but keep refs
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

    if (
        (eFlag == FSYS_FLAG_RELROOT && aName.isEmpty()) ||
        (
         (!pEntryTop->aName.isEmpty()  ||
          ((rEntry.Level()>1)?(rEntry[rEntry.Level()-2].aName.equalsIgnoreAsciiCase(RFS_IDENTIFIER)):sal_False))
          &&
         (pEntryTop->eFlag == FSYS_FLAG_ABSROOT ||
          pEntryTop->eFlag == FSYS_FLAG_RELROOT ||
          pEntryTop->eFlag == FSYS_FLAG_VOLUME)
        )
       )
    {
                return rEntry;
    }

    // something + "." (=> pEntryTop == &rEntry)
    if (pEntryTop->eFlag == FSYS_FLAG_RELROOT && pEntryTop->aName.isEmpty())
    {
        DBG_ASSERT( pEntryTop == &rEntry, "DirEntry::op+ buggy" );
        return *this;
    }

    // root += ".." (=> impossible)
        if ( pEntryTop->eFlag == FSYS_FLAG_PARENT && pThisTop == this &&
                ( eFlag == FSYS_FLAG_ABSROOT ) )
                return DirEntry( FSYS_FLAG_INVALID );

        // something += abs (=> only append device if existant)
        if ( pEntryTop->eFlag == FSYS_FLAG_ABSROOT )
        {
                rtl::OString aDevice;
                if ( pThisTop->eFlag == FSYS_FLAG_ABSROOT )
                    aDevice = pThisTop->aName;
                DirEntry aRet = rEntry;
                if ( !aDevice.isEmpty() )
                    aRet.ImpGetTopPtr()->aName = aDevice;
                return aRet;
        }

        // something += ".." (=> break apart)
        if ( eFlag == FSYS_FLAG_NORMAL && pEntryTop->eFlag == FSYS_FLAG_PARENT )
        {
                String aConcated( GetFull() );
                aConcated += ACCESSDELIM_C(FSYS_STYLE_HOST);
                aConcated += rEntry.GetFull();
                return DirEntry( aConcated );
        }

        // otherwise append consecutively
        DirEntry aRet( rEntry );
        DirEntry *pTop = aRet.ImpGetTopPtr();
        pTop->pParent = new DirEntry( *this );

        return aRet;
}

DirEntry &DirEntry::operator+=( const DirEntry& rEntry )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    return *this = *this + rEntry;
}

String DirEntry::GetAccessDelimiter( FSysPathStyle eFormatter )
{
        return rtl::OUString( ACCESSDELIM_C( GetStyle( eFormatter ) ) );
}

void DirEntry::SetExtension( const String& rExtension, char cSep )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    // do not set extensions for drives
    if(eFlag == FSYS_FLAG_ABSROOT)
    {
        nError = FSYS_ERR_NOTSUPPORTED;
        return;
    }

    rtl::OStringBuffer aBuf(aName);

    // search cSep within aName
    const sal_Char *p0 = aBuf.getStr();
    const sal_Char *p1 = p0 + aBuf.getLength() - 1;
    while ( p1 >= p0 && *p1 != cSep )
        p1--;
    if ( p1 >= p0 )
    {
        // found a cSep on position p1

        sal_Int32 n = static_cast<sal_Int32>(
                p1 - p0 + 1 - ( rExtension.Len() ? 0 : 1 ));

        aBuf.remove(n, aBuf.getLength()-n);
    }
    else if ( rExtension.Len() )
    {
        // no cSep was found
        aBuf.append(cSep);
    }

    aBuf.append(rtl::OUStringToOString(rExtension,
        osl_getThreadTextEncoding()));

    aName = aBuf.makeStringAndClear();
}

void DirEntry::SetName( const String& rName, FSysPathStyle eFormatter )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    if ( eFormatter == FSYS_STYLE_HOST || eFormatter == FSYS_STYLE_DETECT )
        eFormatter = DEFSTYLE;
    sal_Char cAccDelim(ACCESSDELIM_C(eFormatter));

    if ( (eFlag != FSYS_FLAG_NORMAL) ||
         (aName.indexOf(':') != -1) ||
         (aName.indexOf(cAccDelim) != -1) )
    {
        eFlag = FSYS_FLAG_INVALID;
    }
    else
    {
        aName = rtl::OUStringToOString(rName, osl_getThreadTextEncoding());
    }
}

sal_Bool DirEntry::Find( const String& rPfad, char cDelim )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    if ( ImpGetTopPtr()->eFlag == FSYS_FLAG_ABSROOT )
            return sal_True;

    sal_Bool bWild = aName.indexOf( '*' ) != -1 ||
                     aName.indexOf( '?' ) != -1;

    if ( !cDelim )
            cDelim = SEARCHDELIM(DEFSTYLE)[0];

    rtl::OString aThis = rtl::OStringBuffer()
        .append(ACCESSDELIM_C(DEFSTYLE))
        .append(rtl::OUStringToOString(GetFull(),
            osl_getThreadTextEncoding()))
        .makeStringAndClear();
    sal_Int32 nIndex = 0;
    do
    {
        rtl::OStringBuffer aPath(rtl::OUStringToOString(rPfad,
            osl_getThreadTextEncoding()).getToken( 0, cDelim, nIndex ));

        if ( aPath.getLength() )
        {
            if (aPath[aPath.getLength()-1] == ACCESSDELIM_C(DEFSTYLE))
                aPath.remove(aPath.getLength()-1, 1);
            aPath.append(aThis);
            DirEntry aEntry(rtl::OStringToOUString(
                aPath.makeStringAndClear(), osl_getThreadTextEncoding()));
            if ( aEntry.ToAbs() &&
                     ( ( !bWild && aEntry.Exists() ) || ( bWild && aEntry.First() ) ) )
            {
                    (*this) = aEntry;
                    return sal_True;
            }
        }
    }
    while ( nIndex >= 0 );
    return sal_False;
}

#ifndef UNX
DirEntry DirEntry::GetDevice() const
{
        DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

        const DirEntry *pTop = ImpGetTopPtr();

        if ( ( pTop->eFlag == FSYS_FLAG_ABSROOT || pTop->eFlag == FSYS_FLAG_RELROOT ) &&
                 !pTop->aName.isEmpty() )
                return DirEntry( pTop->aName, FSYS_FLAG_VOLUME );
        else
                return DirEntry( rtl::OString(), FSYS_FLAG_INVALID );
}
#endif

String DirEntry::GetSearchDelimiter( FSysPathStyle eFormatter )
{
    return rtl::OStringToOUString(rtl::OString(SEARCHDELIM(GetStyle(eFormatter))), osl_getThreadTextEncoding());
}

namespace
{
    struct TempNameBase_Impl : public rtl::Static< DirEntry, TempNameBase_Impl > {};
}

DirEntry DirEntry::TempName( DirEntryKind eKind ) const
{
        // use base-temp-dir if necessary
        const DirEntry &rEntry = TempNameBase_Impl::get();
        if ( !pParent && FSYS_FLAG_CURRENT != rEntry.eFlag && FSYS_FLAG_ABSROOT != eFlag )
        {
                DirEntry aFactory( rEntry );
                aFactory += GetName();
                return aFactory.TempName();
        }

        rtl::OString aDirName;
        char *ret_val;
        size_t i;

        // determine Directory, Prefix and Extension
        char pfx[6];
        char ext[5];
        const char *dir;
        const char *pWild = strchr( aName.getStr(), '*' );
        if ( !pWild )
            pWild = strchr( aName.getStr(), '?' );

        if ( pWild )
        {
            if ( pParent )
                aDirName = rtl::OUStringToOString(pParent->GetFull(), osl_getThreadTextEncoding());
            strncpy( pfx, aName.getStr(), Min( (int)5, (int)(pWild-aName.getStr()) ) );
            pfx[ pWild-aName.getStr() ] = 0;
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
            aDirName = rtl::OUStringToOString(GetFull(), osl_getThreadTextEncoding());
            strcpy( pfx, "lo" );
            strcpy( ext, ".tmp" );
        }
        dir = aDirName.getStr();

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
#if defined(WNT)
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
                    if ( FSYS_KIND_DIR == eKind )
                    {
                        if (0 == _mkdir(rtl::OUStringToOString(aRedirected, osl_getThreadTextEncoding()).getStr()))
                        {
                            aRet = DirEntry( aRetVal );
                            break;
                        }
                    }
                    else
                    {
#if defined(UNX)
                        if (access(rtl::OUStringToOString(aRedirected, osl_getThreadTextEncoding()).getStr(), F_OK))
                        {
                                aRet = DirEntry( aRetVal );
                                break;
                        }
#else
                        struct stat aStat;
                        if (stat(rtl::OUStringToOString(aRedirected, osl_getThreadTextEncoding()).getStr(), &aStat))
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

const DirEntry &DirEntry::operator[]( sal_uInt16 nParentLevel ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    //TPF: maybe to be implemented (FastFSys)

    const DirEntry *pRes = this;
    while ( pRes && nParentLevel-- )
        pRes = pRes->pParent;

    return *pRes;
}

sal_Bool DirEntry::MakeDir( sal_Bool bSloppy ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

        // fast check if exists
        if ( FileStat( *this ).IsKind( FSYS_KIND_DIR ) )
                return sal_True;
        if ( bSloppy && pParent )
                 if ( FileStat( *pParent ).IsKind( FSYS_KIND_DIR ) )
                          return sal_True;

        const DirEntry *pNewDir = bSloppy ? pParent : this;
        if ( pNewDir )
        {
                // Create path to dir
                if ( pNewDir->pParent && !pNewDir->pParent->MakeDir(sal_False) )
                        return sal_False;

                // create dir ourselves
                if ( pNewDir->eFlag == FSYS_FLAG_ABSROOT ||
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
                                rtl::OString bDirName(rtl::OUStringToOString(aDirName, osl_getThreadTextEncoding()));

#ifdef WIN32
                                SetLastError(0);
#endif
                                sal_Bool bResult = (0 == _mkdir(bDirName.getStr()));
                                if ( !bResult )
                                {
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

FSysError DirEntry::CopyTo( const DirEntry& rDest, FSysAction nActions ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

        if ( FSYS_ACTION_COPYFILE != (nActions & FSYS_ACTION_COPYFILE) )
#ifdef UNX
    {
        // create hardlink
        HACK(redirection missing)
        rtl::OString aThis(rtl::OUStringToOString(GetFull(), osl_getThreadTextEncoding()));
        rtl::OString aDest(rtl::OUStringToOString(rDest.GetFull(), osl_getThreadTextEncoding()));
        if (link(aThis.getStr(), aDest.getStr()) == -1)
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

#if defined WNT || defined UNX
FSysError DirEntry::MoveTo( const DirEntry& rNewName ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    DirEntry aDest(rNewName);
    FileStat aDestStat(rNewName);
    if ( aDestStat.IsKind(FSYS_KIND_DIR ) )
    {
        aDest += DirEntry(rtl::OStringToOUString(aName, osl_getThreadTextEncoding()));
    }
    if ( aDest.Exists() )
    {
        return FSYS_ERR_ALREADYEXISTS;
    }

        FSysFailOnErrorImpl();
        String aFrom( GetFull() );

        String aTo( aDest.GetFull() );

        rtl::OString bFrom(rtl::OUStringToOString(aFrom, osl_getThreadTextEncoding()));
        rtl::OString bTo(rtl::OUStringToOString(aTo, osl_getThreadTextEncoding()));

#ifdef WNT
        // MoveTo nun atomar
        SetLastError(0);

        DirEntry aFromDevice(rtl::OStringToOUString(bFrom, osl_getThreadTextEncoding()));
        DirEntry aToDevice(rtl::OStringToOUString(bTo,osl_getThreadTextEncoding()));
        aFromDevice.ToAbs();
        aToDevice.ToAbs();
        aFromDevice=aFromDevice.GetDevice();
        aToDevice=aToDevice.GetDevice();

        if (aFromDevice==aToDevice)
        {
            // same device, use intra-device-move with MoveFile
            MoveFile( bFrom.getStr(), bTo.getStr() );
            // Note: MoveFile is buggy for cross-device operations.
            // Return value is TRUE, even if the operation was only partially successful.
            // MoveFile has varying behavior between differing NT-versions.
            return Sys2SolarError_Impl( GetLastError() );
        }
        else
        {
            // Not the same device, use inter-device-move with copy/delete
            FSysError nCopyError = CopyTo(rNewName, FSYS_ACTION_COPYFILE);

            DirEntry aKill(rtl::OStringToOUString(bTo, osl_getThreadTextEncoding()));
            FileStat aKillStat(String(rtl::OStringToOUString(bTo, osl_getThreadTextEncoding())));
            if ( aKillStat.IsKind(FSYS_KIND_DIR ) )
            {
                aKill += String(rtl::OStringToOUString(aName, osl_getThreadTextEncoding()));
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
        if ( ( aFrom != aTo ) && ( 0 != rename( bFrom.getStr(), bTo.getStr() ) ) )
#if !defined(UNX)
            return Sys2SolarError_Impl( GetLastError() );
#else
        {
                if( errno == EXDEV )
                // simple rename does not work cross device
                {
                        FILE *fpIN  = fopen( bFrom.getStr(), "r" );
                        FILE *fpOUT = fopen( bTo.getStr(), "w" );
                        if( fpIN && fpOUT )
                        {
                                char pBuf[ 16384 ];
                                int nBytes, nWritten, nErr = 0;
                                errno = 0;
                                while( ( nBytes = fread( pBuf, 1, sizeof(pBuf), fpIN ) ) && ! nErr )
                                {
                                    nWritten = fwrite( pBuf, 1, nBytes, fpOUT );
                                    // Error in fwrite ?
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
                                    unlink( bTo.getStr() );
                                    return Sys2SolarError_Impl( nErr );
                                }
                                else
                                {
                                    unlink( bFrom.getStr() );
                                }
                        }
                        else
                        {
                            if ( fpIN )
                                fclose( fpIN );
                            if ( fpOUT )
                                fclose( fpOUT );
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

// For the WNT case we always return already above, so avoid warning
// C4702: unreachable code. Possibly also in non-WNT cases we always
// return already above, but gcc apparently doesn't mind.
#ifndef WNT
        return ERRCODE_NONE;
#endif
}

#endif

FSysError DirEntry::Kill(  FSysAction nActions ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

        FSysError eError = FSYS_ERR_OK;
        FSysFailOnErrorImpl();

        // Terminate name string with two '0'
        String aTmpName( GetFull() );
        rtl::OString bTmpName(rtl::OUStringToOString(aTmpName, osl_getThreadTextEncoding()));

        char *pName = new char[bTmpName.getLength()+2];
        strcpy( pName, bTmpName.getStr() );
        pName[bTmpName.getLength()+1] = (char) 0;

        // delete read-only files as well
        sal_Bool isReadOnly = FileStat::GetReadOnlyFlag(*this);
        if (isReadOnly)
        {
            FileStat::SetReadOnlyFlag(*this, sal_False);
        }

        // directory?
        if ( FileStat( *this ).IsKind(FSYS_KIND_DIR) )
        {
                // Delete recursively?
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

                // remove Dir myself
#ifdef WIN32
                SetLastError(0);
#endif
                if ( eError == FSYS_ERR_OK && 0 != _rmdir( (char*) pName ) )
                {
                        // Change CWD if deletion failed
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
#if defined(WNT)
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

        // restore original read-only flag upon error
        if ( isReadOnly && (eError!=ERRCODE_NONE) )
        {
            FileStat::SetReadOnlyFlag(*this, isReadOnly);
        }

        delete[] pName;
        return eError;
}

/** Check if rSubEntry is (in)directly beneath *this */
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

sal_Bool DirEntry::IsValid() const
{
        return (nError == FSYS_ERR_OK);
}

#if defined(DBG_UTIL)
void FSysTest()
{
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
