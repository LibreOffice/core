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
#include <tools/wldcrd.hxx>
#include <tools/fsys.hxx>
#include <tools/bigint.hxx>

int Sys2SolarError_Impl( int nSysErr );

static sal_Bool   bLastCaseSensitive    = sal_False;

//--------------------------------------------------------------------

ByteString Upper_Impl( const ByteString &rStr )
{
    ByteString aRet( rStr.GetBuffer() ); // es muss ein neuer String entstehen!
    CharUpperBuff( (char*) aRet.GetBuffer(), aRet.Len() );
    return aRet;
}

//--------------------------------------------------------------------

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
            // *.* dahinter, ggf mit "\\" abtrennen (falls nicht schon da)
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

/*************************************************************************
|*
|*    DirEntry::GetPathStyle() const
|*
*************************************************************************/

ErrCode GetPathStyle_Impl( const String &rDevice, FSysPathStyle &rStyle )
{
    ByteString aRootDir(rDevice, osl_getThreadTextEncoding());
    if ( aRootDir.Len() && aRootDir.GetBuffer()[aRootDir.Len()-1] != '\\' )
        aRootDir += '\\';

    char sVolumeName[256];
    char sFileSysName[16];
    DWORD nSerial[2];
    DWORD nMaxCompLen[2];
    DWORD nFlags[2];

    // Windows95 hat VFAT, WindowsNT nicht
    DWORD nVer = GetVersion();
    sal_Bool bW95 = ( nVer & 0xFF ) >= 4;

    FSysFailOnErrorImpl();
    rStyle = FSYS_STYLE_UNKNOWN;
    if ( GetVolumeInformation(
            (char*) aRootDir.GetBuffer(),
            sVolumeName, 256, (LPDWORD) &nSerial, (LPDWORD) &nMaxCompLen,
            (LPDWORD) &nFlags, sFileSysName, 16 ) )
    {
        // FAT/VFAT?
        if ( 0 == strcmp( "FAT", sFileSysName ) )
            rStyle = bW95 ? FSYS_STYLE_VFAT : FSYS_STYLE_FAT;

        // NTFS?
        else if ( 0 == strcmp( "NTFS", sFileSysName ) )
            rStyle = FSYS_STYLE_NTFS;

        // HPFS?
        else if ( 0 == strcmp( "HPFS", sFileSysName ) )
            rStyle = FSYS_STYLE_HPFS;

        // NWCOMPA/NWFS?
        else if ( 0 == strncmp( "NW", sFileSysName, 2 ) )
            rStyle = FSYS_STYLE_NWFS;

        return ERRCODE_NONE;
    }

    return ERRCODE_IO_INVALIDDEVICE;
}

FSysPathStyle DirEntry::GetPathStyle( const String &rDevice )
{

    FSysPathStyle eStyle;
    GetPathStyle_Impl( rDevice, eStyle );
    return eStyle;
}

/*************************************************************************
|*
|*    DirEntry::IsCaseSensitive()
|*
*************************************************************************/

sal_Bool DirEntry::IsCaseSensitive( FSysPathStyle eFormatter ) const
{

    if (eFormatter==FSYS_STYLE_HOST)
    {
/*
        DirEntry            aRoot(*this);
        aRoot.ToAbs();
        aRoot =             aRoot[Level()-1];
        String aRootDir =   aRoot.GetFull(FSYS_STYLE_HOST, sal_True);

        char sVolumeName[256];
        DWORD nVolumeSerial;
        DWORD nMaxCompLen;
        DWORD nFlags;
        char sFileSysName[16];

        if ( GetVolumeInformation(  (char*) aRootDir.GetStr(),
                                    sVolumeName,
                                    256,
                                    (LPDWORD) &nVolumeSerial,
                                    (LPDWORD) &nMaxCompLen,
                                    (LPDWORD) &nFlags,
                                    sFileSysName,
                                    16 ))
        {
            return (nFlags & FS_CASE_SENSITIVE) ? sal_True : sal_False;
        }
        else
        {
            return sal_False;
        }
*/
        //
        // guter versuch, aber FS_CASE_SENSITIVE ist D?nnsinn in T?ten:
        //
        // sFileSysName     FS_CASE_SENSITIVE
        // FAT              sal_False
        // NTFS             sal_True !!!
        // NWCompat         sal_False
        // Samba            sal_False
        //
        // NT spricht auch NTFS lediglich case preserving an, also ist unter NT alles case insensitiv
        //

        return sal_False;
    }
    else
    {
        sal_Bool isCaseSensitive = sal_False;   // ich bin unter win32, also ist der default case insensitiv
        switch ( eFormatter )
        {
            case FSYS_STYLE_MAC:
            case FSYS_STYLE_FAT:
            case FSYS_STYLE_VFAT:
            case FSYS_STYLE_NTFS:
            case FSYS_STYLE_NWFS:
            case FSYS_STYLE_HPFS:
            case FSYS_STYLE_DETECT:
                {
                    isCaseSensitive = sal_False;
                    break;
                }
            case FSYS_STYLE_SYSV:
            case FSYS_STYLE_BSD:
                {
                    isCaseSensitive = sal_True;
                    break;
                }
            default:
                {
                    isCaseSensitive = sal_False;    // ich bin unter win32, also ist der default case insensitiv
                    break;
                }
        }
        return isCaseSensitive;
    }
}

/*************************************************************************
|*
|*    DirEntry::ToAbs()
|*
*************************************************************************/

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
    ByteString aFullName( GetFull(), osl_getThreadTextEncoding() );
    FSysFailOnErrorImpl();
    if ( GetFullPathName((char*)aFullName.GetBuffer(),256,sBuf,&pOld) > 511 )
        return sal_False;

    *this = DirEntry( String(sBuf, osl_getThreadTextEncoding() ));
    return sal_True;
}


/*************************************************************************
|*
|*    DirEntry::GetVolume()
|*
*************************************************************************/

String DirEntry::GetVolume() const
{
  DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    String aRet;
    const DirEntry *pTop = ImpGetTopPtr();
    ByteString aName = ByteString( pTop->aName ).ToLowerAscii();

    if ( ( pTop->eFlag == FSYS_FLAG_ABSROOT ||
           pTop->eFlag == FSYS_FLAG_RELROOT ||
           pTop->eFlag == FSYS_FLAG_VOLUME )
         && aName != "a:" && aName != "b:" && Exists() )
    {
        char sFileSysName[256];
        char sVolumeName[256];
        DWORD nVolumeNameLen = 256;
        DWORD nSerial[2];
        DWORD nMaxCompLen[2];
        DWORD nFlags[2];
        ByteString aRootDir = pTop->aName;
        FSysFailOnErrorImpl();

        // Network-Device zuerst probieren wegen langsamer Samba-Drives
        if ( !WNetGetConnection( (char*) aRootDir.GetBuffer(),
                                 sVolumeName, &nVolumeNameLen ) )
            aRet = String( sVolumeName, osl_getThreadTextEncoding());

        // dann den VolumeNamen fuer lokale Drives
        if ( aRet.Len() == 0 )
        {
            aRootDir += "\\";
            if ( GetVolumeInformation( (char*) aRootDir.GetBuffer(),
                                       sVolumeName, 256,
                                       (LPDWORD) &nSerial, (LPDWORD) &nMaxCompLen,
                                       (LPDWORD) &nFlags, sFileSysName, 256 ) )
                aRet = String( sVolumeName, osl_getThreadTextEncoding());
        }
    }

    return aRet;
}

/*************************************************************************
|*
|*    DirEntry::SetCWD()
|*
*************************************************************************/

sal_Bool DirEntry::SetCWD( sal_Bool bSloppy ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    FSysFailOnErrorImpl();

    if ( eFlag == FSYS_FLAG_CURRENT && !aName.Len() )
        return sal_True;

    if ( SetCurrentDirectory(ByteString(GetFull(), osl_getThreadTextEncoding()).GetBuffer()) )
    {
        return sal_True;
    }

    if ( bSloppy && pParent &&
         SetCurrentDirectory(ByteString(pParent->GetFull(), osl_getThreadTextEncoding()).GetBuffer()) )
    {
        return sal_True;
    }

    return sal_False;
}

//-------------------------------------------------------------------------

USHORT DirReader_Impl::Init()
{
    // Block-Devices auflisten?
    if ( pDir->eAttrMask & FSYS_KIND_BLOCK )
    {
        // CWD merken
        DirEntry aCurrentDir;
        aCurrentDir.ToAbs();

        // einzeln auf Existenz und Masken-konformit"at pr"ufen
        USHORT nRead = 0;
        char sDrive[3] = { '?', ':', 0 };
        char sRoot[4] = { '?', ':', '\\', 0 };
        for ( char c = 'a'; c <= 'z'; c++ )
        {
            sDrive[0] = c;
            sRoot[0] = c;
            DirEntry* pDrive = new DirEntry( sDrive, FSYS_FLAG_VOLUME, FSYS_STYLE_HOST );
            if ( pDir->aNameMask.Matches( String( ByteString(sDrive), osl_getThreadTextEncoding())) && GetDriveType( sRoot ) != 1 )
            {
                if ( pDir->pStatLst ) //Status fuer Sort gewuenscht?
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

        // CWD restaurieren
        aCurrentDir.SetCWD();
        return nRead;
    }

    return 0;
}

//-------------------------------------------------------------------------

USHORT DirReader_Impl::Read()
{
    // Directories und Files auflisten?
    if ( ( pDir->eAttrMask & FSYS_KIND_DIR ||
           pDir->eAttrMask & FSYS_KIND_FILE ) &&
           ( ( pDosEntry = readdir( pDosDir ) ) != NULL ) )
    {
        // Gross/Kleinschreibung nicht beruecksichtigen
        ByteString aLowerName = pDosEntry->d_name;
        CharLowerBuff( (char*) aLowerName.GetBuffer(), aLowerName.Len() );

        // Flags pruefen
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
             pDir->aNameMask.Matches( String(aLowerName, osl_getThreadTextEncoding()) ) )
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
            DirEntry *pTemp = new DirEntry( ByteString(pDosEntry->d_name),
                                            eFlag, FSYS_STYLE_NTFS );
#ifdef FEAT_FSYS_DOUBLESPEED
            pTemp->ImpSetStat( new FileStat( (void*) pDosDir, (void*) 0 ) );
#endif
            if ( pParent )
                pTemp->ImpChangeParent( new DirEntry( *pParent ), sal_False );
            if ( pDir->pStatLst ) //Status fuer Sort gewuenscht?
            {
                FileStat *pNewStat = new FileStat( (void*) pDosDir, (void*) 0 );
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

/*************************************************************************
|*
|*    InitFileStat()
|*
|*    Beschreibung      gemeinsamer Teil der Ctoren fuer FileStat
|*
*************************************************************************/

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

/*************************************************************************
|*
|*    FileStat::FileStat()
|*
*************************************************************************/

FileStat::FileStat( const void *pInfo,      // struct dirent
                    const void * ):         // dummy
    aDateCreated(0),
    aTimeCreated(0),
    aDateModified(0),
    aTimeModified(0),
    aDateAccessed(0),
    aTimeAccessed(0)
{
    ImpInit( ( (dirent*) pInfo ) );
}

/*************************************************************************
|*
|*    FileStat::Update()
|*
*************************************************************************/

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

//---------------------------------------------------------------------------

void SHFreeMem( void *p )
{
    LPMALLOC    pMalloc = NULL;

    if ( SUCCEEDED(SHGetMalloc(&pMalloc)) )
    {
        pMalloc->Free( p );
        pMalloc->Release();
    }
}

//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------

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

//---------------------------------------------------------------------------

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
    if ( pszPathCopy )
        lstrcpy( pszPathCopy, pszPath );
    else
        return E_OUTOFMEMORY;

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

//---------------------------------------------------------------------------
// The Wrapper
//---------------------------------------------------------------------------

sal_Bool Exists_Impl( const ByteString & crPath )
{
    // We do not know if OLE was initialized for this thread

    CoInitialize( NULL );

    sal_Bool    bSuccess = SUCCEEDED( SHResolvePath(NULL, crPath.GetBuffer(), NULL) );

    CoUninitialize();

    return bSuccess;
}

//---------------------------------------------------------------------------

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

        // Sonderbehandlung falls es sich um eine Root ohne Laufwerk handelt

        if ( !rDirEntry.aName.Len() && rDirEntry.eFlag == FSYS_FLAG_ABSROOT )
        {
            nKindFlags = FSYS_KIND_DIR;
            nError = FSYS_ERR_OK;
            return sal_True;
        }

        // keine Error-Boxen anzeigen
        FSysFailOnErrorImpl();

        // Redirect
        String aPath( rDirEntry.GetFull() );
#ifndef BOOTSTRAP
        FSysRedirector::DoRedirect( aPath );
#endif
        DirEntry aDirEntry( aPath );

        // ist ein Medium im Laufwerk?
        HACK("wie?")
        sal_Bool bAccess = sal_True;
        const DirEntry *pTop = aDirEntry.ImpGetTopPtr();
        ByteString aName = ByteString(pTop->aName).ToLowerAscii();
        if ( !bForceAccess &&
                ( pTop->eFlag == FSYS_FLAG_ABSROOT ||
                pTop->eFlag == FSYS_FLAG_RELROOT ||
                pTop->eFlag == FSYS_FLAG_VOLUME ) )
            if ( aName == "a:" || aName == "b:" )
                bAccess = sal_False;
            else
                OSL_TRACE( "FSys: will access removable device!" );
        if ( bAccess && ( aName == "a:" || aName == "b:" ) ) {
            DBG_WARNING( "floppy will clatter" );
        }

        // Sonderbehandlung, falls es sich um ein Volume handelt
        if ( aDirEntry.eFlag == FSYS_FLAG_VOLUME ||
             aDirEntry.eFlag == FSYS_FLAG_ABSROOT )
        {
            if ( aDirEntry.eFlag == FSYS_FLAG_VOLUME )
                nKindFlags = FSYS_KIND_DEV | ( aDirEntry.aName.Len() == 2
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

            ByteString aRootDir = aDirEntry.aName;
            aRootDir += ByteString( "\\" );
            UINT nType = GetDriveType( (char *) aRootDir.GetBuffer() );       //TPF: 2i
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

        // Statusinformation vom Betriebssystem holen
        HANDLE h; //()
        _WIN32_FIND_DATAA aEntry = {};
        DirEntry aAbsEntry( aDirEntry );
        if ( bAccess && aAbsEntry.ToAbs() )
        {
            // im Namen k"onnen auch ';*?' als normale Zeichen vorkommen
            ByteString aFilePath( aAbsEntry.GetFull(), osl_getThreadTextEncoding() );

            // MI: dann gehen Umlaute auf Novell-Servern nicht / wozu ueberhaupt
            // CharUpperBuff( (char*) aFilePath.GetStr(), aFilePath.Len() );
            OSL_TRACE( "FileStat: %s", aFilePath.GetBuffer() );
            h = aFilePath.Len() < 230
                    // die Win32-API ist hier sehr schwammig
                    ? FindFirstFile( (char *) aFilePath.GetBuffer(), &aEntry )//TPF: 2i
                    : INVALID_HANDLE_VALUE;

            if ( INVALID_HANDLE_VALUE != h )
            {
                if ( !( aEntry.dwFileAttributes & 0x40 ) ) // com1: etc. e.g. not encrypted (means normal)
                {
                    ByteString  aUpperName = Upper_Impl(ByteString(aAbsEntry.GetName(), osl_getThreadTextEncoding()));

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
                DirEntry *pTop = aAbsEntry.ImpGetTopPtr();
                if ( pTop->GetFlag() == FSYS_FLAG_ABSROOT &&
                     ( pTop->aName.Len() > 1 && (pTop->aName.GetBuffer()[1] != ':' )) )
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
            // Sonderbehandlung falls es sich um eine Wildcard handelt
            ByteString aTempName( aDirEntry.GetName(), osl_getThreadTextEncoding() );
            if ( strchr( aTempName.GetBuffer(), '?' ) ||
                 strchr( aTempName.GetBuffer(), '*' ) ||
                 strchr( aTempName.GetBuffer(), ';' ) )
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

sal_Bool IsRedirectable_Impl( const ByteString &rPath )
{
    if ( rPath.Len() >= 3 && ':' == rPath.GetBuffer()[1] )
    {
        ByteString aVolume = rPath.Copy( 0, 3 );
        UINT nType = GetDriveType( (char *) aVolume.GetBuffer() );
        SetLastError( ERROR_SUCCESS );
        return DRIVE_FIXED != nType;
    }
    return sal_False;
}

/*************************************************************************
|*
|*    TempDirImpl()
|*
|*    Beschreibung      liefert den Namens des Directories fuer temporaere
|*                      Dateien
|*
*************************************************************************/

const char* TempDirImpl( char *pBuf )
{
    if ( !GetTempPath( MAX_PATH, pBuf ) &&
        !GetWindowsDirectory( pBuf, MAX_PATH ) &&
        !GetEnvironmentVariable( "HOMEPATH", pBuf, MAX_PATH ) )
        return 0;

    return pBuf;
}

//=======================================================================

ErrCode FileStat::QueryDiskSpace( const String &rPath,
                                  BigInt &rFreeBytes, BigInt &rTotalBytes )
{
    DWORD nSectorsPerCluster;   /* address of sectors per cluster   */
    DWORD nBytesPerSector;      /* address of bytes per sector  */
    DWORD nFreeClusters;        /* address of number of free clusters   */
    DWORD nClusters;            /* address of total number of clusters  */

    ByteString aVol( DirEntry(rPath).ImpGetTopPtr()->GetName(), osl_getThreadTextEncoding());
    bool bOK = GetDiskFreeSpace( aVol.GetBuffer(),
                        &nSectorsPerCluster, &nBytesPerSector,
                        &nFreeClusters, &nClusters );
    if ( !bOK )
        return Sys2SolarError_Impl( GetLastError() );

    BigInt aBytesPerCluster( BigInt(nSectorsPerCluster) *
                             BigInt(nBytesPerSector) );
    rFreeBytes = aBytesPerCluster * BigInt(nFreeClusters);
    rTotalBytes = aBytesPerCluster * BigInt(nClusters);
    return 0;
}

//=========================================================================

void FSysEnableSysErrorBox( sal_Bool bEnable )
{   // Preserve other Bits!!
    sal_uInt32 nErrorMode = SetErrorMode( bEnable ? 0 : SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX );
    if ( bEnable )
        nErrorMode &= ~(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
    else
        nErrorMode |= (SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
    SetErrorMode( nErrorMode );
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
