/*************************************************************************
 *
 *  $RCSfile: wntmsc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:07 $
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

#include <stdio.h>
#include <ctype.h>
#include <limits.h>

#include "wntmsc.hxx"
#include "errinf.hxx"

#ifndef _DEBUG_HXX
#include <debug.hxx>
#endif
#ifndef _LIST_HXX
#include <list.hxx>
#endif
#ifndef _WLDCRD_HXX
#include <wldcrd.hxx>
#endif
#ifndef _FSYS_HXX
#include <fsys.hxx>
#endif
#ifndef _BIGINT_HXX
#include "bigint.hxx"
#endif

DECLARE_LIST( DirEntryList, DirEntry* );
DECLARE_LIST( FSysSortList, FSysSort* );
DECLARE_LIST( FileStatList, FileStat* );

int Sys2SolarError_Impl( int nSysErr );

static ByteString sLastCaseSensitiveDir = "";
static BOOL   bLastCaseSensitive    = FALSE;

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
    BOOL bOk = FALSE;
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
            delete pBuf;
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
    BOOL bOk = FALSE;
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
|*    Beschreibung
|*    Ersterstellung    MI 11.05.95
|*    Letzte Aenderung  MI 11.05.95
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
    BOOL bW95 = ( nVer & 0xFF ) >= 4;

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
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI  10.06.93
|*    Letzte Aenderung  TPF 26.02.1999
|*
*************************************************************************/

BOOL DirEntry::IsCaseSensitive( FSysPathStyle eFormatter ) const
{

    if (eFormatter==FSYS_STYLE_HOST)
    {
/*
        DirEntry            aRoot(*this);
        aRoot.ToAbs();
        aRoot =             aRoot[Level()-1];
        String aRootDir =   aRoot.GetFull(FSYS_STYLE_HOST, TRUE);

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
            return (nFlags & FS_CASE_SENSITIVE) ? TRUE : FALSE;
        }
        else
        {
            return FALSE;
        }
*/
        //
        // guter versuch, aber FS_CASE_SENSITIVE ist D?nnsinn in T?ten:
        //
        // sFileSysName     FS_CASE_SENSITIVE
        // FAT              FALSE
        // NTFS             TRUE !!!
        // NWCompat         FALSE
        // Samba            FALSE
        //
        // NT spricht auch NTFS lediglich case preserving an, also ist unter NT alles case insensitiv
        //

        return FALSE;
    }
    else
    {
        BOOL isCaseSensitive = FALSE;   // ich bin unter win32, also ist der default case insensitiv
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
                    isCaseSensitive = FALSE;
                    break;
                }
            case FSYS_STYLE_SYSV:
            case FSYS_STYLE_BSD:
                {
                    isCaseSensitive = TRUE;
                    break;
                }
            default:
                {
                    isCaseSensitive = FALSE;    // ich bin unter win32, also ist der default case insensitiv
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
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 26.04.91
|*    Letzte Aenderung  MA 02.12.91
|*
*************************************************************************/

BOOL DirEntry::ToAbs()
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    if ( FSYS_FLAG_VOLUME == eFlag )
    {
        eFlag = FSYS_FLAG_ABSROOT;
        return TRUE;
    }

    if ( IsAbs() )
    {
        return TRUE;
    }


    char sBuf[256];
    char *pOld;
    ByteString aFullName( GetFull(), osl_getThreadTextEncoding() );
    FSysFailOnErrorImpl();
    if ( GetFullPathName((char*)aFullName.GetBuffer(),256,sBuf,&pOld) > 511 )
        return FALSE;

    *this = DirEntry( String(sBuf, osl_getThreadTextEncoding() ));
    return TRUE;
}


/*************************************************************************
|*
|*    DirEntry::GetVolume()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 27.08.92
|*    Letzte Aenderung  MI 28.08.92
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
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 26.04.91
|*    Letzte Aenderung  MI 21.05.92
|*
*************************************************************************/

BOOL DirEntry::SetCWD( BOOL bSloppy )
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    FSysFailOnErrorImpl();

    if ( eFlag == FSYS_FLAG_CURRENT && !aName.Len() )
        return TRUE;

    if ( SetCurrentDirectory(ByteString(GetFull(), osl_getThreadTextEncoding()).GetBuffer()) )
    {
        nError = FSYS_ERR_OK;
        return TRUE;
    }

    if ( bSloppy && pParent &&
         SetCurrentDirectory(ByteString(pParent->GetFull(), osl_getThreadTextEncoding()).GetBuffer()) )
    {
        nError = FSYS_ERR_OK;
        return TRUE;
    }

    nError = FSYS_ERR_NOTADIRECTORY;
    return FALSE;
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
        BOOL bIsDirAndWantsDir =
                ( ( pDir->eAttrMask & FSYS_KIND_DIR ) &&
#ifdef ICC
                    ( pDosEntry->d_type & ( strcmp(pDosEntry->d_name,".") ||
                      strcmp(pDosEntry->d_name,"..")) ) );
#else
                    ( pDosEntry->d_type & DOS_DIRECT ) );
#endif
        BOOL bIsFileAndWantsFile =
                ( ( pDir->eAttrMask & FSYS_KIND_FILE ) &&
#ifdef ICC
                    !( pDosEntry->d_type & ( strcmp(pDosEntry->d_name,".") ||
                      strcmp(pDosEntry->d_name,"..")) ) &&
#else
                    !( pDosEntry->d_type & DOS_DIRECT ) &&
#endif
                    !( pDosEntry->d_type & DOS_VOLUMEID ) );
        BOOL bIsHidden = pDosEntry->d_type & _A_HIDDEN;
        BOOL bWantsHidden = 0 == ( pDir->eAttrMask & FSYS_KIND_VISIBLE );
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
                pTemp->ImpChangeParent( new DirEntry( *pParent ), FALSE );
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
        bReady = TRUE;
    return 0;
}

/*************************************************************************
|*
|*    InitFileStat()
|*
|*    Beschreibung      gemeinsamer Teil der Ctoren fuer FileStat
|*    Ersterstellung    MI 28.08.92
|*    Letzte Aenderung  MI 28.08.92
|*
*************************************************************************/

void FileStat::ImpInit( void* p )
{
    _WIN32_FIND_DATAA *pDirEnt = (_WIN32_FIND_DATAA*) p;

    nError = FSYS_ERR_OK;
    nSize = pDirEnt->nFileSizeLow;

    SYSTEMTIME aSysTime;
    FILETIME aLocTime;

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
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 27.08.92
|*    Letzte Aenderung  MI 28.08.92
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
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 27.08.92
|*    Letzte Aenderung  MI 28.08.92
|*
*************************************************************************/

#ifdef ENABLEUNICODE
// #define UNICODE
#endif

#include <prewin.h>
#include <shlobj.h>
#include <postwin.h>

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

    ULONG   chEaten = lstrlen( pszPath );
    DWORD   dwAttributes = FILE_ATTRIBUTE_DIRECTORY;

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
    TCHAR   cBackup;

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

BOOL Exists_Impl( const ByteString & crPath )
{
    // We do not know if OLE was initialized for this thread

    CoInitialize( NULL );

    BOOL    bSuccess = SUCCEEDED( SHResolvePath(NULL, crPath.GetBuffer(), NULL) );

    CoUninitialize();

    return bSuccess;
}

//---------------------------------------------------------------------------

BOOL FileStat::Update( const DirEntry& rDirEntry, BOOL bForceAccess )
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
            return FALSE;
        }

        // Sonderbehandlung falls es sich um eine Root ohne Laufwerk handelt

        if ( !rDirEntry.aName.Len() && rDirEntry.eFlag == FSYS_FLAG_ABSROOT )
        {
            nKindFlags = FSYS_KIND_DIR;
            nError = FSYS_ERR_OK;
            return TRUE;
        }

        // keine Error-Boxen anzeigen
        FSysFailOnErrorImpl();

        // Redirect
        String aPath( rDirEntry.GetFull() );
#ifndef BOOTSTRAP
        BOOL bRedirected = FSysRedirector::DoRedirect( aPath );
#endif
        DirEntry aDirEntry( aPath );

        // ist ein Medium im Laufwerk?
        HACK("wie?")
        BOOL bAccess = TRUE;
        const DirEntry *pTop = aDirEntry.ImpGetTopPtr();
        ByteString aName = ByteString(pTop->aName).ToLowerAscii();
        if ( !bForceAccess &&
                ( pTop->eFlag == FSYS_FLAG_ABSROOT ||
                pTop->eFlag == FSYS_FLAG_RELROOT ||
                pTop->eFlag == FSYS_FLAG_VOLUME ) )
            if ( aName == "a:" || aName == "b:" )
                bAccess = FALSE;
            else
                DBG_TRACE( "FSys: will access removable device!" );
        if ( bAccess && ( aName == "a:" || aName == "b:" ) )
            DBG_WARNING( "floppy will clatter" );

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
                return FALSE;
            }

            ByteString aRootDir = aDirEntry.aName;
            aRootDir += ByteString( "\\" );
            USHORT nType = GetDriveType( (char *) aRootDir.GetBuffer() );       //TPF: 2i
            if ( nType == 1 || nType == 0 )
            {
                nError = FSYS_ERR_NOTEXISTS;
                nKindFlags = 0;
                return FALSE;
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

            return TRUE;
        }

        // Statusinformation vom Betriebssystem holen
        HANDLE h; //()
        _WIN32_FIND_DATAA aEntry;
        DirEntry aAbsEntry( aDirEntry );
        if ( bAccess && aAbsEntry.ToAbs() )
        {
            // im Namen k"onnen auch ';*?' als normale Zeichen vorkommen
            ByteString aFilePath( aAbsEntry.GetFull(), osl_getThreadTextEncoding() );

            // MI: dann gehen Umlaute auf Novell-Servern nicht / wozu ueberhaupt
            // CharUpperBuff( (char*) aFilePath.GetStr(), aFilePath.Len() );
            DBG_TRACE1( "FileStat: %s", aFilePath.GetBuffer() );
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
                    return FALSE;
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
                        return TRUE;
                    }
                    else
                    {
                        nKindFlags = FSYS_KIND_UNKNOWN;
                        nError = FSYS_ERR_NOTEXISTS;
                        return FALSE;
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
                return TRUE;
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

BOOL IsRedirectable_Impl( const ByteString &rPath )
{
    if ( rPath.Len() >= 3 && ':' == rPath.GetBuffer()[1] )
    {
        ByteString aVolume = rPath.Copy( 0, 3 );
        USHORT nType = GetDriveType( (char *) aVolume.GetBuffer() );
        SetLastError( ERROR_SUCCESS );
        return DRIVE_FIXED != nType;
    }
    return FALSE;
}

/*************************************************************************
|*
|*    TempDirImpl()
|*
|*    Beschreibung      liefert den Namens des Directories fuer temporaere
|*                      Dateien
|*    Ersterstellung    MI 16.03.94
|*    Letzte Aenderung  MI 16.03.94
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
    BOOL bOK = GetDiskFreeSpace( aVol.GetBuffer(),
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

void FSysEnableSysErrorBox( BOOL bEnable )
{   // Preserve other Bits!!
    sal_uInt32 nErrorMode = SetErrorMode( bEnable ? 0 : SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX );
    if ( bEnable )
        nErrorMode &= ~(SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
    else
        nErrorMode |= (SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX);
    SetErrorMode( nErrorMode );
}



