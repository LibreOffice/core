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

#ifdef _MSC_VER
#pragma warning (push,1)
#endif

#include <stdio.h>
#include <ctype.h>
#include <limits.h>

#ifdef _MSC_VER
#pragma warning (pop)
#endif

#include "wntmsc.hxx"
#include <tools/errinf.hxx>
#include <tools/debug.hxx>
#include <tools/fsys.hxx>
#include <vector>

#define INIT_WIN32_FIND_DATAA { 0, { 0, 0 }, { 0, 0 }, { 0, 0 }, 0, 0, 0, 0, { 0 }, { 0 } }

int Sys2SolarError_Impl( int nSysErr );

rtl::OString Upper_Impl(const rtl::OString &rStr)
{
    std::vector<sal_Char> aBuffer(rStr.getLength());
    memcpy(&aBuffer[0], rStr.getStr(), rStr.getLength());
    CharUpperBuff(&aBuffer[0], rStr.getLength());
    return rtl::OString(&aBuffer[0], rStr.getLength());
}

DIR *opendir( const char* pPfad )
{
    DIR *pDir = new DIR;
    if ( pDir )
        pDir->p = (char*) pPfad;
    return pDir;
}

struct dirent *readdir( DIR *pDir )
{
    bool bOk = false;
    if ( pDir->p )
    {
        char *pBuf = new char[ strlen( pDir->p ) + 5 ];
        if ( pBuf )
        {
            // if string ends with *.*, seperate with "\\" (unless it exists)
            strcpy( pBuf, pDir->p );
            strcat( pBuf, "\\*.*" + ( *(pBuf + strlen( pBuf ) - 1 ) == '\\' ) );
            CharUpperBuff( pBuf, strlen(pBuf) );
            pDir->h = FindFirstFile( pBuf, &pDir->aDirEnt );
            bOk = pDir->h != INVALID_HANDLE_VALUE;
            pDir->p = NULL;
            delete [] pBuf;
        }
        else
            pDir->h = INVALID_HANDLE_VALUE;
    }
    else
    {
        bOk = FindNextFile( pDir->h, &pDir->aDirEnt );
    }

    return bOk ? &pDir->aDirEnt : NULL;
}

int closedir( DIR *pDir )
{
    sal_Bool bOk = sal_False;
    if ( pDir )
    {
        bOk = 0 != pDir->p || FindClose( pDir->h );
        delete pDir;
    }
    return bOk;
}

sal_Bool DirEntry::ToAbs()
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    if ( FSYS_FLAG_VOLUME == eFlag )
    {
        eFlag = FSYS_FLAG_ABSROOT;
        return sal_True;
    }

    if ( IsAbs() )
    {
        return sal_True;
    }


    char sBuf[256];
    char *pOld;
    rtl::OString aFullName(rtl::OUStringToOString(GetFull(),
        osl_getThreadTextEncoding()));
    FSysFailOnErrorImpl();
    if ( GetFullPathName(aFullName.getStr(), 256, sBuf, &pOld) > 511 )
        return sal_False;

    *this = DirEntry( String(sBuf, osl_getThreadTextEncoding() ));
    return sal_True;
}

String DirEntry::GetVolume() const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    String aRet;
    const DirEntry *pTop = ImpGetTopPtr();
    rtl::OString aTopName = rtl::OString(pTop->aName).toAsciiLowerCase();

    if ( ( pTop->eFlag == FSYS_FLAG_ABSROOT ||
           pTop->eFlag == FSYS_FLAG_RELROOT ||
           pTop->eFlag == FSYS_FLAG_VOLUME )
         && !aTopName.equalsL(RTL_CONSTASCII_STRINGPARAM("a:"))
         && !aTopName.equalsL(RTL_CONSTASCII_STRINGPARAM("b:")) && Exists() )
    {
        char sFileSysName[256];
        char sVolumeName[256];
        DWORD nVolumeNameLen = 256;
        DWORD nSerial[2];
        DWORD nMaxCompLen[2];
        DWORD nFlags[2];
        rtl::OString aRootDir = pTop->aName;
        FSysFailOnErrorImpl();

        // Try network device first due to slow samba drives
        if ( !WNetGetConnection( aRootDir.getStr(),
                                 sVolumeName, &nVolumeNameLen ) )
            aRet = String( sVolumeName, osl_getThreadTextEncoding());

        // Append volume name for local drives
        if ( aRet.Len() == 0 )
        {
            aRootDir += rtl::OString(RTL_CONSTASCII_STRINGPARAM("\\"));
            if ( GetVolumeInformation( aRootDir.getStr(),
                                       sVolumeName, 256,
                                       (LPDWORD) &nSerial, (LPDWORD) &nMaxCompLen,
                                       (LPDWORD) &nFlags, sFileSysName, 256 ) )
                aRet = String( sVolumeName, osl_getThreadTextEncoding());
        }
    }

    return aRet;
}

sal_Bool DirEntry::SetCWD( sal_Bool bSloppy ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    FSysFailOnErrorImpl();

    if ( eFlag == FSYS_FLAG_CURRENT && !aName.getLength() )
        return sal_True;

    if ( SetCurrentDirectory(rtl::OUStringToOString(GetFull(), osl_getThreadTextEncoding()).getStr()) )
    {
        return sal_True;
    }

    if ( bSloppy && pParent &&
         SetCurrentDirectory(rtl::OUStringToOString(pParent->GetFull(), osl_getThreadTextEncoding()).getStr()) )
    {
        return sal_True;
    }

    return sal_False;
}

USHORT DirReader_Impl::Init()
{
    // List Block-devices?
    if ( pDir->eAttrMask & FSYS_KIND_BLOCK )
    {
        // remember CWD
        DirEntry aCurrentDir;
        aCurrentDir.ToAbs();

        // Check for existence and conformity to flags
        USHORT nRead = 0;
        char sDrive[3] = { '?', ':', 0 };
        char sRoot[4] = { '?', ':', '\\', 0 };
        for ( char c = 'a'; c <= 'z'; c++ )
        {
            sDrive[0] = c;
            sRoot[0] = c;
            DirEntry* pDrive = new DirEntry( sDrive, FSYS_FLAG_VOLUME );
            if ( pDir->aNameMask.Matches( String(rtl::OStringToOUString(sDrive, osl_getThreadTextEncoding())) ) && GetDriveType( sRoot ) != 1 )
            {
                if ( pDir->pStatLst ) // Status required by sorting criteria?
                {
                    FileStat *pNewStat = new FileStat( *pDrive );
                    pDir->ImpSortedInsert( pDrive, pNewStat );
                }
                else
                    pDir->ImpSortedInsert( pDrive, NULL );
                ++nRead;
            }
            else
                delete pDrive;
        }

        // restore CWD
        aCurrentDir.SetCWD();
        return nRead;
    }

    return 0;
}

USHORT DirReader_Impl::Read()
{
    // List directories and Files?
    if ( ( pDir->eAttrMask & FSYS_KIND_DIR ||
           pDir->eAttrMask & FSYS_KIND_FILE ) &&
           ( ( pDosEntry = readdir( pDosDir ) ) != NULL ) )
    {
        // Do not distinguish between lower-/upper-case letters
        size_t nLen = strlen(pDosEntry->d_name);
        std::vector<char> aBuffer(nLen);
        memcpy(&aBuffer[0], pDosEntry->d_name, nLen);
        CharLowerBuff(&aBuffer[0], nLen);
        rtl::OString aLowerName(&aBuffer[0], nLen);

        // check Flags
        sal_Bool bIsDirAndWantsDir =
                ( ( pDir->eAttrMask & FSYS_KIND_DIR ) &&
#ifdef ICC
                    ( pDosEntry->d_type & ( strcmp(pDosEntry->d_name,".") ||
                      strcmp(pDosEntry->d_name,"..")) ) );
#else
                    ( pDosEntry->d_type & DOS_DIRECT ) );
#endif
        sal_Bool bIsFileAndWantsFile =
                ( ( pDir->eAttrMask & FSYS_KIND_FILE ) &&
#ifdef ICC
                    !( pDosEntry->d_type & ( strcmp(pDosEntry->d_name,".") ||
                      strcmp(pDosEntry->d_name,"..")) ) &&
#else
                    !( pDosEntry->d_type & DOS_DIRECT ) &&
#endif
                    !( pDosEntry->d_type & DOS_VOLUMEID ) );
        sal_Bool bIsHidden = (pDosEntry->d_type & _A_HIDDEN) != 0;
        sal_Bool bWantsHidden = 0 == ( pDir->eAttrMask & FSYS_KIND_VISIBLE );
        if ( ( bIsDirAndWantsDir || bIsFileAndWantsFile ) &&
             ( bWantsHidden || !bIsHidden ) &&
             pDir->aNameMask.Matches( rtl::OStringToOUString(aLowerName, osl_getThreadTextEncoding()) ) )
        {
#ifdef DBG_UTIL
            DbgOutf( "%s %s flags:%x found",
                pDosEntry->d_name,
                bIsFileAndWantsFile ? "file" : "dir",
                pDosEntry->d_type );
#endif
            DirEntryFlag eFlag =
                    0 == strcmp( pDosEntry->d_name, "." ) ? FSYS_FLAG_CURRENT
                :   0 == strcmp( pDosEntry->d_name, ".." ) ? FSYS_FLAG_PARENT
                :   FSYS_FLAG_NORMAL;
            DirEntry *pTemp = new DirEntry( rtl::OString(pDosEntry->d_name),
                                            eFlag );
#ifdef FEAT_FSYS_DOUBLESPEED
            pTemp->ImpSetStat( new FileStat( (void*) pDosDir ) );
#endif
            if ( pParent )
                pTemp->ImpChangeParent( new DirEntry( *pParent ), sal_False );
            if ( pDir->pStatLst ) // Status required by sorting criteria?
            {
                FileStat *pNewStat = new FileStat( (void*) pDosDir );
                pDir->ImpSortedInsert( pTemp, pNewStat );
            }
            else
                pDir->ImpSortedInsert( pTemp, NULL );
            return 1;
        }
#ifdef DBG_UTIL
        else
            DbgOutf( "%s flags:%x skipped",
                pDosEntry->d_name,
                pDosEntry->d_type );
#endif

    }
    else
        bReady = sal_True;
    return 0;
}

/// shared part of CTors for FileStat
void FileStat::ImpInit( void* p )
{
    _WIN32_FIND_DATAA *pDirEnt = (_WIN32_FIND_DATAA*) p;

    nError = FSYS_ERR_OK;
    nSize = pDirEnt->nFileSizeLow;

    SYSTEMTIME aSysTime;
    FILETIME aLocTime;

    // use the last write date / time when the creation date / time isn't set
    if ( ( pDirEnt->ftCreationTime.dwLowDateTime == 0 ) &&
         ( pDirEnt->ftCreationTime.dwHighDateTime == 0 ) )
    {
        pDirEnt->ftCreationTime.dwLowDateTime = pDirEnt->ftLastWriteTime.dwLowDateTime;
        pDirEnt->ftCreationTime.dwHighDateTime = pDirEnt->ftLastWriteTime.dwHighDateTime;
    }

    // use the last write date / time when the last accessed date / time isn't set
    if ( ( pDirEnt->ftLastAccessTime.dwLowDateTime == 0 ) &&
         ( pDirEnt->ftLastAccessTime.dwHighDateTime == 0 ) )
    {
        pDirEnt->ftLastAccessTime.dwLowDateTime = pDirEnt->ftLastWriteTime.dwLowDateTime;
        pDirEnt->ftLastAccessTime.dwHighDateTime = pDirEnt->ftLastWriteTime.dwHighDateTime;
    }

    FileTimeToLocalFileTime( &pDirEnt->ftCreationTime, &aLocTime );
    FileTimeToSystemTime( &aLocTime, &aSysTime );
    aDateCreated  = Date( aSysTime.wDay, aSysTime.wMonth, aSysTime.wYear );
    aTimeCreated  = Time( aSysTime.wHour, aSysTime.wMinute,
                            aSysTime.wSecond, 0 );

    FileTimeToLocalFileTime( &pDirEnt->ftLastWriteTime, &aLocTime );
    FileTimeToSystemTime( &aLocTime, &aSysTime );
    aDateModified = Date( aSysTime.wDay, aSysTime.wMonth, aSysTime.wYear );
    aTimeModified = Time( aSysTime.wHour, aSysTime.wMinute,
                            aSysTime.wSecond, 0 );

    FileTimeToLocalFileTime( &pDirEnt->ftLastAccessTime, &aLocTime );
    FileTimeToSystemTime( &aLocTime, &aSysTime );
    aDateAccessed = Date( aSysTime.wDay, aSysTime.wMonth, aSysTime.wYear );
    aTimeAccessed = Time( aSysTime.wHour, aSysTime.wMinute,
                            aSysTime.wSecond, 0 );

    nKindFlags = FSYS_KIND_FILE;
    if ( pDirEnt->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
        nKindFlags = FSYS_KIND_DIR;
}

FileStat::FileStat( const void *pInfo ): // struct dirent
    aDateCreated(0),
    aTimeCreated(0),
    aDateModified(0),
    aTimeModified(0),
    aDateAccessed(0),
    aTimeAccessed(0)
{
    ImpInit( ( (dirent*) pInfo ) );
}

#ifdef _MSC_VER
#pragma warning(push, 1)
#pragma warning(disable: 4917)
#endif

#include <shlobj.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#ifdef UNICODE
#define lstrchr     wcschr
#define lstrncmp    wcsncmp
#else
#define lstrchr     strchr
#define lstrncmp    strncmp
#endif

void SHFreeMem( void *p )
{
    LPMALLOC    pMalloc = NULL;

    if ( SUCCEEDED(SHGetMalloc(&pMalloc)) )
    {
        pMalloc->Free( p );
        pMalloc->Release();
    }
}

HRESULT SHGetIDListFromPath( HWND hwndOwner, LPCTSTR pszPath, LPITEMIDLIST *ppidl )
{
    if ( IsBadWritePtr(ppidl, sizeof(LPITEMIDLIST)) )
        return E_INVALIDARG;

    LPSHELLFOLDER   pDesktopFolder = NULL;

    HRESULT hResult = SHGetDesktopFolder( &pDesktopFolder );
    if ( FAILED(hResult) )
        return hResult;

    ULONG chEaten = lstrlen( pszPath );
    DWORD dwAttributes = FILE_ATTRIBUTE_DIRECTORY;

#ifdef UNICODE
    LPOLESTR    wszPath = pszPath;
#else
    WCHAR   wszPath[MAX_PATH];
    MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, pszPath, -1, wszPath, MAX_PATH );
#endif

    hResult = pDesktopFolder->ParseDisplayName( hwndOwner, (LPBC)NULL, wszPath, &chEaten, ppidl, &dwAttributes );
    pDesktopFolder->Release();

    return hResult;
}

HRESULT SHGetFolderFromIDList( LPCITEMIDLIST pidl, LPSHELLFOLDER *ppFolder )
{
    if ( IsBadWritePtr(ppFolder, sizeof(LPSHELLFOLDER)) )
        return E_INVALIDARG;

    *ppFolder = NULL;

    LPSHELLFOLDER pDesktopFolder = NULL;

    HRESULT hResult = SHGetDesktopFolder( &pDesktopFolder );
    if ( FAILED(hResult) )
        return hResult;

    hResult = pDesktopFolder->BindToObject( pidl, (LPBC)NULL, IID_IShellFolder, (LPVOID *)ppFolder );
    pDesktopFolder->Release();

    return hResult;
}

HRESULT SHResolvePath( HWND hwndOwner, LPCTSTR pszPath, LPITEMIDLIST *ppidl )
{
    // If hwndOwner is NULL, use the desktop window, because dialogs need a parent

#ifdef BOOTSTRAP
    return  NO_ERROR;
#else
    if ( !hwndOwner )
        hwndOwner = GetDesktopWindow();

    HRESULT hResult = NOERROR;
    LPTSTR  pszPathCopy;
    LPTSTR  pszTrailingPath;
    TCHAR   cBackup = 0;

    // First make a copy of the path

    pszPathCopy = new TCHAR[lstrlen(pszPath) + 1];
    lstrcpy( pszPathCopy, pszPath );

    // Determine the first token

    if ( !lstrncmp( pszPathCopy, "\\\\", 2 ) )
        pszTrailingPath = lstrchr( pszPathCopy + 2, '\\' );
    else
        pszTrailingPath = lstrchr( pszPathCopy, '\\' );

    // Now scan the path tokens

    while ( SUCCEEDED(hResult) )
    {
        if ( pszTrailingPath )
        {
            cBackup = *(++pszTrailingPath);
            *pszTrailingPath = 0;
        }

        LPITEMIDLIST    pidl = NULL;

        // Make item ID list from leading path

        hResult = SHGetIDListFromPath( hwndOwner, pszPathCopy, &pidl );

        // if path exists try to open it as folder

        if ( SUCCEEDED(hResult) )
        {
            // Only open the folder if it was not the last token

            if ( pszTrailingPath )
            {
                LPSHELLFOLDER   pFolder;

                // Create a folder instance
                hResult = SHGetFolderFromIDList( pidl, &pFolder);

                // Is it a folder ?
                if ( SUCCEEDED(hResult) )
                {
                    // No try to instantiate an enumerator.
                    // This should popup a login dialog if any

                    LPENUMIDLIST    pEnum = NULL;

                    hResult = pFolder->EnumObjects( hwndOwner,
                        SHCONTF_NONFOLDERS | SHCONTF_FOLDERS | SHCONTF_INCLUDEHIDDEN,
                        &pEnum );

                    // Release the enumerator interface
                    if ( SUCCEEDED(hResult) )
                        pEnum->Release();

                    // Release the folder interface
                    pFolder->Release();
                }

                SHFreeMem( pidl );
            }
            else // It was the last token
            {
                if ( ppidl )
                    *ppidl = pidl;
                else
                    SHFreeMem( pidl );
            }
        }


        // Forward to next token

        if ( pszTrailingPath )
        {
            *pszTrailingPath = cBackup;
            pszTrailingPath = lstrchr( pszTrailingPath, '\\' );
        }
        else
            break;
    }

    // Free the working copy of the path
    delete pszPathCopy;

    // NOERROR or OLE error code
    return hResult;
#endif
}

// The Wrapper

sal_Bool Exists_Impl(const rtl::OString& crPath)
{
    // We do not know if OLE was initialized for this thread

    CoInitialize( NULL );

    sal_Bool    bSuccess = SUCCEEDED( SHResolvePath(NULL, crPath.getStr(), NULL) );

    CoUninitialize();

    return bSuccess;
}

sal_Bool FileStat::Update( const DirEntry& rDirEntry, sal_Bool bForceAccess )
{
        nSize = 0;
        nKindFlags = 0;
        aCreator.Erase();
        aType.Erase();
        aDateCreated = Date(0);
        aTimeCreated = Time(0);
        aDateModified = Date(0);
        aTimeModified = Time(0);
        aDateAccessed = Date(0);
        aTimeAccessed = Time(0);

        if ( !rDirEntry.IsValid() )
        {
            nError = FSYS_ERR_UNKNOWN;
            nKindFlags = 0;
            return sal_False;
        }

        // Special treatment if it's a root without device

        if ( !rDirEntry.aName.getLength() && rDirEntry.eFlag == FSYS_FLAG_ABSROOT )
        {
            nKindFlags = FSYS_KIND_DIR;
            nError = FSYS_ERR_OK;
            return sal_True;
        }

        // Don't show error boxes
        FSysFailOnErrorImpl();

        // Redirect
        String aPath( rDirEntry.GetFull() );
        DirEntry aDirEntry( aPath );

        // Is a medium in this device?
        HACK("How?")
        sal_Bool bAccess = sal_True;
        const DirEntry *pTop = aDirEntry.ImpGetTopPtr();
        rtl::OString aName = rtl::OString(pTop->aName).toAsciiLowerCase();
        if ( !bForceAccess &&
                ( pTop->eFlag == FSYS_FLAG_ABSROOT ||
                pTop->eFlag == FSYS_FLAG_RELROOT ||
                pTop->eFlag == FSYS_FLAG_VOLUME ) )
        {
            if ( aName.equalsL(RTL_CONSTASCII_STRINGPARAM("a:")) ||
                 aName.equalsL(RTL_CONSTASCII_STRINGPARAM("b:")) )
                bAccess = sal_False;
            else
                OSL_TRACE( "FSys: will access removable device!" );
        }
        if ( bAccess && ( aName.equalsL(RTL_CONSTASCII_STRINGPARAM("a:")) ||
                          aName.equalsL(RTL_CONSTASCII_STRINGPARAM("b:")) ) )
        {
            DBG_WARNING( "floppy will clatter" );
        }

        // Special treatment if it's a volume
        if ( aDirEntry.eFlag == FSYS_FLAG_VOLUME ||
             aDirEntry.eFlag == FSYS_FLAG_ABSROOT )
        {
            if ( aDirEntry.eFlag == FSYS_FLAG_VOLUME )
                nKindFlags = FSYS_KIND_DEV | ( aDirEntry.aName.getLength() == 2
                                        ? FSYS_KIND_BLOCK
                                        : FSYS_KIND_CHAR );
            else
                nKindFlags = FSYS_KIND_DIR;

            if ( !bAccess )
            {
                if ( aDirEntry.eFlag == FSYS_FLAG_VOLUME )
                    nKindFlags |= FSYS_KIND_REMOVEABLE;
                nError = FSYS_ERR_NOTEXISTS;
                nKindFlags = 0;
                return sal_False;
            }

            rtl::OString aRootDir = aDirEntry.aName;
            aRootDir += rtl::OString(RTL_CONSTASCII_STRINGPARAM("\\"));
            UINT nType = GetDriveType( aRootDir.getStr() );       //TPF: 2i
            if ( nType == 1 || nType == 0 )
            {
                nError = FSYS_ERR_NOTEXISTS;
                nKindFlags = 0;
                return sal_False;
            }

            if ( aDirEntry.eFlag == FSYS_FLAG_VOLUME )
                nKindFlags = nKindFlags |
                     ( ( nType == DRIVE_REMOVABLE ) ? FSYS_KIND_REMOVEABLE : 0 ) |
                     ( ( nType == DRIVE_FIXED     ) ? FSYS_KIND_FIXED      : 0 ) |
                     ( ( nType == DRIVE_REMOTE    ) ? FSYS_KIND_REMOTE     : 0 ) |
                     ( ( nType == DRIVE_RAMDISK   ) ? FSYS_KIND_RAM        : 0 ) |
                     ( ( nType == DRIVE_CDROM     ) ? FSYS_KIND_CDROM      : 0 ) |
                     ( ( nType == 0               ) ? FSYS_KIND_UNKNOWN    : 0 );

            nError = ERRCODE_NONE;

            return sal_True;
        }

        // Get status data from OS
        HANDLE h; //()
        _WIN32_FIND_DATAA aEntry = INIT_WIN32_FIND_DATAA;
        DirEntry aAbsEntry( aDirEntry );
        if ( bAccess && aAbsEntry.ToAbs() )
        {
            // names can contain ';*?' as normal characters
            rtl::OString aFilePath(rtl::OUStringToOString(aAbsEntry.GetFull(), osl_getThreadTextEncoding()));

            OSL_TRACE( "FileStat: %s", aFilePath.getStr() );
            h = aFilePath.getLength() < 230
                    ? FindFirstFile( aFilePath.getStr(), &aEntry )//TPF: 2i
                    : INVALID_HANDLE_VALUE;

            if ( INVALID_HANDLE_VALUE != h )
            {
                if ( !( aEntry.dwFileAttributes & 0x40 ) ) // com1: etc. e.g. not encrypted (means normal)
                {
                    rtl::OString aUpperName = Upper_Impl(rtl::OUStringToOString(aAbsEntry.GetName(), osl_getThreadTextEncoding()));

                    // HRO: #74051# Compare also with short alternate filename
                    if ( aUpperName != Upper_Impl( aEntry.cFileName ) && aUpperName != Upper_Impl( aEntry.cAlternateFileName ) )
                        h = INVALID_HANDLE_VALUE;
                }
            }

            if ( INVALID_HANDLE_VALUE == h )
            {
                DWORD   dwError = GetLastError();

                if ( ERROR_BAD_NET_NAME == dwError )
                {
                    nKindFlags = FSYS_KIND_UNKNOWN;
                    nError = FSYS_ERR_NOTEXISTS;
                    return sal_False;
                }

                // UNC-Volume?
                DirEntry *pTop2 = aAbsEntry.ImpGetTopPtr();
                if ( pTop2->GetFlag() == FSYS_FLAG_ABSROOT &&
                     ( pTop2->aName.getLength() > 1 && (pTop2->aName[1] != ':' )) )
                {
                    if ( bForceAccess )
                    {
                        if ( Exists_Impl( aFilePath ) )
                    {
                        nKindFlags = FSYS_KIND_DIR|FSYS_KIND_REMOTE;
                        nError = FSYS_ERR_OK;
                        return sal_True;
                    }
                    else
                    {
                        nKindFlags = FSYS_KIND_UNKNOWN;
                        nError = FSYS_ERR_NOTEXISTS;
                        return sal_False;
                    }
                    }
                }
            }
        }
        else
            h = INVALID_HANDLE_VALUE;

        if ( h == INVALID_HANDLE_VALUE )
        {
            // Special treatment if name contains wildcard
            rtl::OString aTempName(rtl::OUStringToOString(aDirEntry.GetName(), osl_getThreadTextEncoding()));
            if ( strchr( aTempName.getStr(), '?' ) ||
                 strchr( aTempName.getStr(), '*' ) ||
                 strchr( aTempName.getStr(), ';' ) )
            {
                nKindFlags = FSYS_KIND_WILD;
                nError = FSYS_ERR_OK;
                return sal_True;
            }

            if ( bAccess )
            {
                nError = FSYS_ERR_NOTEXISTS;
                nKindFlags = FSYS_KIND_UNKNOWN;
            }
            else
                nKindFlags = FSYS_KIND_REMOVEABLE;
        }
        else
        {
            ImpInit( &aEntry );
            FindClose( h );
        }

        if ( 0 != nError )
            nKindFlags = 0;

        return 0 == nError;

}

sal_Bool IsRedirectable_Impl( const rtl::OString& rPath )
{
    if ( rPath.getLength() >= 3 && ':' == rPath[1] )
    {
        rtl::OString aVolume = rPath.copy( 0, 3 );
        UINT nType = GetDriveType( aVolume.getStr() );
        SetLastError( ERROR_SUCCESS );
        return DRIVE_FIXED != nType;
    }
    return sal_False;
}

/// get name of the directory for temporary files
const char* TempDirImpl( char *pBuf )
{
    if ( !GetTempPath( MAX_PATH, pBuf ) &&
        !GetWindowsDirectory( pBuf, MAX_PATH ) &&
        !GetEnvironmentVariable( "HOMEPATH", pBuf, MAX_PATH ) )
        return 0;

    return pBuf;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
