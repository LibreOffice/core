/*************************************************************************
 *
 *  $RCSfile: os2.cxx,v $
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

#define INCL_DOSEXCEPTIONS

#ifdef __BORLANDC__
#include <alloc.h>
#else
#include <malloc.h>
#endif

#ifndef _DEBUG_HXX
#include <debug.hxx>
#endif
#ifndef _LIST_HXX
#include <list.hxx>
#endif
#ifndef _BIGINT_HXX
#include <bigint.hxx>
#endif
#ifndef _FSYS_HXX
#include <fsys.hxx>
#endif
#include "comdep.hxx"

#ifdef OS2
#ifndef _VOS_MUTEX_HXX //autogen
#include <vos/mutex.hxx>
#endif
#endif

int Sys2SolarError_Impl( int nSysErr );

DECLARE_LIST( DirEntryList, DirEntry* );
DECLARE_LIST( FSysSortList, FSysSort* );
DECLARE_LIST( FileStatList, FileStat* );

static char sCaseMap[256];
static BOOL bCaseMap = FALSE;
static BOOL bDriveMap = FALSE;

struct DriveMapItem
{
    DirEntryKind    nKind;
    char            cName;
    FSysPathStyle   nStyle;
};

void CreateCaseMapImpl();
void CreateDriveMapImpl();

static DriveMapItem aDriveMap[26];

static String sLastCaseSensitiveDir = "";
static BOOL   bLastCaseSensitive    = FALSE;

//====================================================================

int ApiRet2ToSolarError_Impl( int nApiRet )
{
    switch ( nApiRet )
    {
        case NO_ERROR:                      return ERRCODE_NONE;
        case ERROR_FILE_NOT_FOUND:          return ERRCODE_IO_NOTEXISTS;
        case ERROR_PATH_NOT_FOUND:          return ERRCODE_IO_NOTEXISTSPATH;
        case ERROR_TOO_MANY_OPEN_FILES:     return ERRCODE_IO_TOOMANYOPENFILES;
        case ERROR_ACCESS_DENIED:           return ERRCODE_IO_ACCESSDENIED;
        case ERROR_NOT_ENOUGH_MEMORY:       return ERRCODE_IO_OUTOFMEMORY;
        case ERROR_BAD_FORMAT:              return ERRCODE_IO_WRONGFORMAT;
        case ERROR_NOT_SAME_DEVICE:         return ERRCODE_IO_INVALIDDEVICE;
        case ERROR_WRITE_PROTECT:           return ERRCODE_IO_INVALIDDEVICE;
        case ERROR_BAD_UNIT:                return ERRCODE_IO_INVALIDDEVICE;
        case ERROR_CRC:                     return ERRCODE_IO_INVALIDDEVICE;
        case ERROR_NOT_DOS_DISK:            return ERRCODE_IO_INVALIDDEVICE;
        case ERROR_WRITE_FAULT:             return ERRCODE_IO_CANTWRITE;
        case ERROR_READ_FAULT:              return ERRCODE_IO_CANTREAD;
        case ERROR_SHARING_VIOLATION:       return ERRCODE_IO_LOCKVIOLATION;
        case ERROR_LOCK_VIOLATION:          return ERRCODE_IO_LOCKVIOLATION;
        case ERROR_WRONG_DISK:              return ERRCODE_IO_LOCKVIOLATION;
        case ERROR_HANDLE_DISK_FULL:        return ERRCODE_IO_OUTOFSPACE;
        case ERROR_NOT_SUPPORTED:           return ERRCODE_IO_NOTSUPPORTED;
        case ERROR_DUP_NAME:                return ERRCODE_IO_ALREADYEXISTS;
        case ERROR_BAD_NETPATH:             return ERRCODE_IO_NOTEXISTSPATH;
        case ERROR_DEV_NOT_EXIST:           return ERRCODE_IO_NOTEXISTS;
        case ERROR_NETWORK_ACCESS_DENIED:   return ERRCODE_IO_ACCESSDENIED;
        case ERROR_INVALID_PARAMETER:       return ERRCODE_IO_INVALIDPARAMETER;
        case ERROR_NET_WRITE_FAULT:         return ERRCODE_IO_CANTWRITE;
        case ERROR_DEVICE_IN_USE:           return ERRCODE_IO_INVALIDPARAMETER;
        case ERROR_DISK_FULL:               return ERRCODE_IO_OUTOFSPACE;
        case ERROR_BAD_ARGUMENTS:           return ERRCODE_IO_INVALIDPARAMETER;
        case ERROR_BAD_PATHNAME:            return ERRCODE_IO_NOTEXISTSPATH;
        case ERROR_LOCK_FAILED:             return ERRCODE_IO_LOCKVIOLATION;
        case ERROR_LOCKED:                  return ERRCODE_IO_LOCKVIOLATION;
        case ERROR_DUPLICATE_NAME:          return ERRCODE_IO_ALREADYEXISTS;
        case ERROR_DIRECTORY_IN_CDS:        return ERRCODE_IO_LOCKVIOLATION;
        case ERROR_CURRENT_DIRECTORY:       return ERRCODE_IO_LOCKVIOLATION;
        case ERROR_FILENAME_EXCED_RANGE:    return ERRCODE_IO_NAMETOOLONG;
    }

    DBG_TRACE1( "FSys: unknown apiret error %d occured", nApiRet );
    return FSYS_ERR_UNKNOWN;
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
    APIRET nRet;
    ULONG nCount = 1;
    if ( pDir->p )
    {
        char *pBuf = new char[ strlen( pDir->p ) + 5 ];
        if ( pBuf )
        {
            // *.* dahinter, ggf mit "\\" abtrennen (falls nicht schon da)
            strcpy( pBuf, pDir->p );
            strcat( pBuf, "\\*.*" + ( *(pBuf + strlen( pBuf ) - 1 ) == '\\' ) );
            pDir->aDirHdl = HDIR_SYSTEM;
            nRet = DosFindFirst( (PSZ) pBuf, &pDir->aDirHdl, 23,
                         (PVOID) &pDir->aDirEnt, sizeof( FILEFINDBUF3 ),
                         &nCount, FIL_STANDARD );
            pDir->p = NULL;
            delete pBuf;
        }
        else
            nRet = 1;
    }
    else
    {
        nRet = DosFindNext( pDir->aDirHdl,
                            (PVOID) &pDir->aDirEnt, sizeof( pDir->aDirEnt ),
                            &nCount );
    }

    return !nRet ? &pDir->aDirEnt : NULL;
}

int closedir( DIR *pDir )
{
    if ( pDir )
    {
        if ( !pDir->p )
            DosFindClose( pDir->aDirHdl );
        delete pDir;
    }
    return (pDir != NULL);
}

typedef struct _FSInfo
{
    ULONG   nSerial;
    BYTE    nNameLen;
    UCHAR   sName[40];
} FSInfo;

char* volumeid( const char* pPfad )
{
    static FSInfo   aFSInfoBuf;

    ULONG           ulFSInfoLevel = FSIL_VOLSER;
    ULONG           nDriveNumber;

    nDriveNumber = toupper(*pPfad) - 'A' + 1;

    if ( nDriveNumber >= 3 )
    {
        APIRET rc = DosQueryFSInfo(
            nDriveNumber, ulFSInfoLevel, &aFSInfoBuf, sizeof(FSInfo) );
        if ( rc )
            return 0;
        return (char*) aFSInfoBuf.sName;
    }
    return 0;
}

/*************************************************************************
|*
|*    DirEntry::ToAbs()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 26.04.91
|*    Letzte Aenderung  MA 02.12.91 13:30
|*
*************************************************************************/

BOOL DirEntry::ToAbs()
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

        if (paVirtualURL)
        {
            return TRUE;
        }

    if ( FSYS_FLAG_VOLUME == eFlag )
    {
        eFlag = FSYS_FLAG_ABSROOT;
        return TRUE;
    }

    if ( IsAbs() )
        return TRUE;

    char sBuf[512];

    // Device bestimmen
    DirEntry* pTop = ImpGetTopPtr();
    ULONG nDev, nMap;
    if ( ( pTop->eFlag == FSYS_FLAG_VOLUME || pTop->eFlag == FSYS_FLAG_RELROOT ) &&
         pTop->aName.Len() )
        nDev = tolower(aName(0)) - 'a' + 1;
    else
        DosQueryCurrentDisk( &nDev, &nMap );

    // das dortige CWD ermitteln;
    ULONG nLen = sizeof( sBuf ) - 1;
    DirEntry *pCWD = 0;
    DosQueryCurrentDir( nDev, (PSZ) sBuf, &nLen );
    if ( sBuf[0] == 0 )
        strcpy( sBuf, "\\" );
    else
        pCWD = new DirEntry( String( sBuf ) );

    // das Device voranstellen
    String aTempStr( (char)(nDev + 'a' - 1) );
    aTempStr += ':';
    if ( pCWD )
        pCWD->ImpGetTopPtr()->pParent =
            new DirEntry( aTempStr, FSYS_FLAG_ABSROOT, DEFSTYLE );
    else
        pCWD = new DirEntry( aTempStr, FSYS_FLAG_ABSROOT, DEFSTYLE );

    // die relative Root durch ihr aktuelles Verzeichnis ersetzen
    if ( pTop == this &&
         ( pTop->eFlag == FSYS_FLAG_RELROOT || pTop->eFlag == FSYS_FLAG_VOLUME ) )
    {
        *pTop = *pCWD;
        delete pCWD;
    }
    else if ( pTop->eFlag == FSYS_FLAG_ABSROOT )
    {
        pTop->aName = aTempStr;
        delete pCWD;
    }
    else
    {
        // dir\\..-Sequenzen aufloesen
        while ( (pTop = ImpGetTopPtr())->eFlag == FSYS_FLAG_PARENT &&
                pTop != this )
        {
            pCWD->CutName();
            ImpGetPreTopPtr()->pParent = 0;
            delete pTop;
        }

        // Pfad davorhaengen
        if ( pTop == this && eFlag == FSYS_FLAG_PARENT )
        {
            *this = pCWD->GetPath();
            delete pCWD;
        }
        else
            ImpGetTopPtr()->pParent = pCWD;
    }

    return IsAbs();
}

/*************************************************************************
|*
|*    DirEntry::GetVolume()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 04.03.92
|*    Letzte Aenderung  MI 04.03.92
|*
*************************************************************************/

String DirEntry::GetVolume() const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    const DirEntry *pTop = ImpGetTopPtr();

    if ( pTop->eFlag == FSYS_FLAG_ABSROOT ||
         pTop->eFlag == FSYS_FLAG_VOLUME )
    {
        const char *pVol;
        pVol = volumeid( (char*) pTop->aName.GetStr() );
        return FSYS2GUI( String( pVol ? pVol : "" ) );
    }

    return String();
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
    if (paVirtualURL)
    {
        String aTempString(GetRealPathFromVirtualURL());
        if (aTempString.Len()==0)
        {
            return FALSE;
        }
        DirEntry aTempDirEntry(aTempString);
        return aTempDirEntry.SetCWD(bSloppy);
    }

    // is the device to be changed?
    DirEntry *pTop = ImpGetTopPtr();
    if ( ( pTop->eFlag == FSYS_FLAG_ABSROOT ||
           pTop->eFlag == FSYS_FLAG_RELROOT || pTop->eFlag == FSYS_FLAG_VOLUME )
         && pTop->aName.Len() )
        if ( DosSetDefaultDisk( pTop->aName.Lower()(0)-'a'+1 ) )
            return FALSE;

    APIRET nRet = DosSetCurrentDir( (PSZ) (const char *) GetFull() );
    if ( nRet && bSloppy )
        nRet = DosSetCurrentDir( (PSZ) (const char *) GetPath().GetFull() );
    return !nRet;
}

/*************************************************************************
|*
|*    DirEntry::MoveTo()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 26.04.91
|*    Letzte Aenderung  MA 02.12.91 14:07
|*
*************************************************************************/

BOOL createLongNameEA( const PCSZ pszPath, ULONG ulAttributes, const String& aLongName );

FSysError DirEntry::MoveTo( const DirEntry& rDest ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    DirEntry aTmpDest(rDest);
    FileStat aTmpStat(aTmpDest);
    if ( aTmpStat.IsKind(FSYS_KIND_DIR) )
        aTmpDest += DirEntry( GetName() );

    String aSource( GetFull() );
    String aDest( aTmpDest.GetFull() );
    String aShortSource("");
    String aShortDest("");

    if (Folder::IsAvailable())
    {
        if  (IsLongNameOnFAT())
        {
            // in kurzen Pfad wandeln
            ItemIDPath      aItemIDPath(aSource);
            aShortSource = aItemIDPath.GetHostNotationPath();
        }
        if  (rDest.IsLongNameOnFAT())
        {
            // in kurzen Pfad wandeln
            ItemIDPath      aItemIDPath(aDest);
            aShortDest = aItemIDPath.GetHostNotationPath();
        }
    }

    APIRET nRet = DosMove( aShortSource.Len()>0?(PSZ)aShortSource.GetStr():(PSZ)aSource.GetStr(),
                           aShortDest.Len()>0?(PSZ)aShortDest.GetStr():(PSZ)aDest.GetStr());

    if ( nRet == ERROR_DIRECTORY_IN_CDS ||
         nRet == ERROR_CURRENT_DIRECTORY )
    {
        // 2nd chance with modified CWD
        DosSetCurrentDir( (PSZ) "\\" );
        nRet = DosMove( aShortSource.Len()>0?(PSZ)aShortSource.GetStr():(PSZ)aSource.GetStr(),
                        aShortDest.Len()>0?(PSZ)aShortDest.GetStr():(PSZ)aDest.GetStr());
    }
    else if ( nRet == ERROR_NOT_SAME_DEVICE )
    {
        // other volume => copy+delete
        FileCopier aMover( *this, rDest );
        nRet = aMover.Execute( FSYS_ACTION_MOVE|FSYS_ACTION_RECURSIVE );
        return nRet;
    }

    if ( (nRet==NO_ERROR) && aShortDest.Len()>0)
    {
    createLongNameEA((const char*)aShortDest,  FILE_NORMAL, rDest.GetName());
    }

    return ApiRet2ToSolarError_Impl( nRet );
}

//-------------------------------------------------------------------------

USHORT DirReader_Impl::Init()
{
    USHORT nRead = 0;
    if ( pDir->eAttrMask & FSYS_KIND_BLOCK )
    {
        CreateDriveMapImpl();
        DirEntry aCurrentDir;
        aCurrentDir.ToAbs();
        char sDrive[3] = "?:";

        for ( char c = START_DRV; c <= 'z'; c++ )
        {
            sDrive[0] = c;
            DirEntry* pDrive = new DirEntry( sDrive,
                FSYS_FLAG_VOLUME, FSYS_STYLE_HOST );
            if ( aDriveMap[c-'a'].nKind != FSYS_KIND_UNKNOWN &&
                 pDir->aNameMask.Matches( sDrive ) )
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

        aCurrentDir.SetCWD();

    }

// OS2 ist beim Directory auslesen nicht threadfest
#ifdef OS2
    static NAMESPACE_VOS( OMutex )* pMutex = 0;
    if( !pMutex ) pMutex = new NAMESPACE_VOS( OMutex );
    NAMESPACE_VOS( OGuard ) aGuard( pMutex );
#endif
    // Workaround: kein async-read, da OS/2 dann nix mehr liefert
    for ( USHORT nLastRead; nLastRead = Read(); )
        nRead += nLastRead;

    return nRead;
}

//-------------------------------------------------------------------------

USHORT DirReader_Impl::Read()
{
    // Directories und Files auflisten?
    if ( ( pDir->eAttrMask & FSYS_KIND_DIR || pDir->eAttrMask & FSYS_KIND_FILE ) &&
         ( ( pDosEntry = readdir( pDosDir ) ) != NULL ) )
    {
        if ( ( ( ( pDir->eAttrMask & FSYS_KIND_DIR ) &&
                    ( pDosEntry->d_type & DOS_DIRECT ) ) ||
                ( ( pDir->eAttrMask & FSYS_KIND_FILE ) &&
                !( pDosEntry->d_type & DOS_DIRECT ) ) ) &&
                pDir->aNameMask.Matches(
                    ToLowerImpl( String( pDosEntry->d_name ) ) ) )
        {
            String aName( FSYS2GUI( pDosEntry->d_name ) );
            DirEntryFlag eFlag =
                    aName == "." ? FSYS_FLAG_CURRENT
                :   aName == ".." ? FSYS_FLAG_PARENT
                :   FSYS_FLAG_NORMAL;
            DirEntry *pTemp = new DirEntry( String( aName ),
                                            eFlag, FSYS_STYLE_NTFS );
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
    }
    else
        bReady = TRUE;
    return 0;
}

/*************************************************************************
|*
|*    FileStat::FileStat()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MA 05.11.91
|*    Letzte Aenderung  MA 07.11.91
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
        paVirtualURL    = NULL;

    struct dirent *pDirent = (struct dirent*) pInfo;

    nSize = pDirent->d_size;

    aDateCreated  = MsDos2Date( pDirent->d_date );
    aTimeCreated  = MsDos2Time( pDirent->d_time );
    aDateModified = aDateModified;
    aTimeModified = aTimeModified;
    aDateAccessed = aDateModified;
    aTimeAccessed = aTimeModified;

    nKindFlags = FSYS_KIND_FILE;
    if ( pDirent->d_type & DOS_DIRECT )
        nKindFlags = FSYS_KIND_DIR;
}

/*************************************************************************
|*
|*    FileStat::Update()
|*
|*    Beschreibung      FSYS.SDW
|*    Ersterstellung    MI 11.06.91
|*    Letzte Aenderung  MA 07.11.91
|*
*************************************************************************/

struct _FSYS_FSQBUFFER
{
    FSQBUFFER2  aBuf;
    UCHAR       sBuf[256];
};

BOOL FileStat::Update( const DirEntry& rDirEntry, BOOL bAccessRemovableDevice )
{
        paVirtualURL    = rDirEntry.paVirtualURL;

    nSize = 0;
    FSysPathStyle eStyle = FSYS_STYLE_UNKNOWN;
    aCreator.Erase();
    aType.Erase();
    aDateCreated = Date(0);
    aTimeCreated = Time(0);
    aDateModified = Date(0);
    aTimeModified = Time(0);
    aDateAccessed = Date(0);
    aTimeAccessed = Time(0);

        if (paVirtualURL)
        {
                if (rDirEntry.GetRealPathFromVirtualURL().Len()==0)
                {
                        // keine reale repräsentation für VirtualURL, also alles setzen,
                        // lediglich dir/file unterscheidbar
                        nKindFlags = FSYS_KIND_ALL;

                        if (Folder(ItemIDPath(rDirEntry.GetFull())).IsValid())  //ist dir?
                        {
                                nKindFlags &= ~FSYS_KIND_FILE;  //ja, also kein file
                        }
                        else
                        {
                                nKindFlags &= ~FSYS_KIND_DIR;   //nein, also kein dir
                        }

                        nError = FSYS_ERR_OK;
                    return TRUE;
                }
        }

        nError = rDirEntry.IsValid() ? FSYS_ERR_OK : FSYS_ERR_UNKNOWN;
    if ( nError > FSYS_ERR_OK )
        return FALSE;

    // Sonderbehandlung falls es sich um eine Wildcard handelt
    String aTempName( rDirEntry.GetName() );
    if ( strchr( (char*) (const char*) aTempName, '?' ) ||
         strchr( (char*) (const char*) aTempName, '*' ) ||
         strchr( (char*) (const char*) aTempName, ';' ) )
    {
        nKindFlags = FSYS_KIND_WILD;
        nError = FSYS_ERR_OK;
        return TRUE;
    }

    // Sonderbehandlung falls es sich um eine Root handelt
    if ( rDirEntry.eFlag == FSYS_FLAG_VOLUME ||
         rDirEntry.eFlag == FSYS_FLAG_ABSROOT )
    {
        if ( rDirEntry.eFlag == FSYS_FLAG_VOLUME )
            nKindFlags = FSYS_KIND_DEV;
        else
            nKindFlags = FSYS_KIND_DIR;

        if ( rDirEntry.aName.Len() == 2 )
        {
            if ( !bDriveMap )
                CreateDriveMapImpl();

            DriveMapItem &rItem = aDriveMap[toupper(rDirEntry.aName[0]) - 'A'];
            if ( !rItem.nKind )
            {
                nError = ERRCODE_IO_INVALIDDEVICE;
                                nKindFlags = FSYS_KIND_UNKNOWN;
                return FALSE;
            }
            else
            {
                if ( rDirEntry.eFlag == FSYS_FLAG_VOLUME )
                    nKindFlags |= FSYS_KIND_BLOCK | rItem.nKind;
                eStyle = rItem.nStyle;
            }
        }

        nError = FSYS_ERR_OK;
        return TRUE;
    }

    // disable error-boxes for hard-errors
    DosError(FERR_DISABLEHARDERR);

    // Statusinformation vom Betriebssystem holen
    DirEntry    aTempDirEntry( rDirEntry );
    char*       p;
    String      aFullName;

    aTempDirEntry.ToAbs();
    aFullName = aTempDirEntry.GetFull();

        if (aTempDirEntry.paVirtualURL)
        {
                aFullName = rDirEntry.GetRealPathFromVirtualURL();
        }

        if (Folder::IsAvailable() && aTempDirEntry.IsLongNameOnFAT())
        {
                // in String mit kurzem Pfad wandeln
                ItemIDPath      aItemIDPath(aTempDirEntry.GetFull());
                aFullName = aItemIDPath.GetHostNotationPath();
        }

    p = (char *) aFullName.GetStr();

    FILESTATUS3 filestat;
    memset( &filestat, 0, sizeof( filestat ) );
    if( DosQueryPathInfo( (PSZ)p, 1, &filestat, sizeof( filestat ) ) )
    {
        nError = FSYS_ERR_NOTEXISTS;
        nKindFlags = FSYS_KIND_UNKNOWN;
        return FALSE;
    }

    nError = FSYS_ERR_OK;
    nSize = filestat.cbFile;

    nKindFlags = FSYS_KIND_UNKNOWN;
    if( filestat.attrFile & FILE_DIRECTORY )
        nKindFlags |= FSYS_KIND_DIR;
    if ( nKindFlags == FSYS_KIND_UNKNOWN )
        nKindFlags = nKindFlags | FSYS_KIND_FILE;

    aDateModified = Date( filestat.fdateLastWrite.day,
                          filestat.fdateLastWrite.month,
                          filestat.fdateLastWrite.year + 1980 );

    aTimeModified = Time( filestat.ftimeLastWrite.hours,
                          filestat.ftimeLastWrite.minutes,
                          filestat.ftimeLastWrite.twosecs*2 );

    if ( filestat.fdateCreation.day )
    {
        aDateCreated  = Date( filestat.fdateCreation.day,
                              filestat.fdateCreation.month,
                              filestat.fdateCreation.year + 1980 );

        aTimeCreated  = Time( filestat.ftimeCreation.hours,
                              filestat.ftimeCreation.minutes,
                              filestat.ftimeCreation.twosecs*2 );
    }
    else
    {
        aDateCreated = aDateModified;
        aTimeCreated = aTimeModified;
    }

    if ( filestat.fdateLastAccess.day > 0 )
    {
        aDateAccessed = Date( filestat.fdateLastAccess.day,
                              filestat.fdateLastAccess.month,
                              filestat.fdateLastAccess.year + 1980 );

        aTimeAccessed = Time( filestat.ftimeLastAccess.hours,
                                filestat.ftimeLastAccess.minutes,
                                filestat.ftimeLastAccess.twosecs*2 );
    }
    else
    {
        aDateAccessed = aDateModified;
        aTimeAccessed = aTimeModified;
    }

    return TRUE;
}

#if SUPD>364
BOOL IsRedirectable_Impl( const String &rPath )
{
    if ( rPath.Len() >= 3 && ':' == rPath.GetStr()[1] )
    {
        DriveMapItem &rItem = aDriveMap[toupper(rPath[0]) - 'A'];
        return FSYS_KIND_FIXED != rItem.nKind;
    }
    return FALSE;
}
#endif

const char* TempDirImpl( char *pBuf )
{
#if defined( __BORLANDC__ ) && (__BORLANDC__ < 0x0460)
    PSZ         pVar;
#else
    PCSZ        pVar;
#endif
    USHORT      nRet;
    BOOL        bAppendTemp = FALSE; // mu\s noch \\temp angeh"angt werden

    // Erstmal sehen, ob TEMP oder TMP gesetzt sind
    nRet = DosScanEnv( (PSZ)"TEMP", &pVar );
    if( nRet )
        nRet = DosScanEnv( (PSZ)"temp", &pVar );
    if( nRet )
        nRet = DosScanEnv( (PSZ)"TMP", &pVar );
    if( nRet )
        nRet = DosScanEnv( (PSZ)"tmp", &pVar );

    // falls das geklappt hat, und ein Backslash dranhaengt,
    // oder falls es bisher nicht geklappt hat,
    // muessen wir nachher einen Backslash entfernen
    BOOL bRemoveBS = nRet || *(pVar+strlen(pVar)-1) == '\\';

    // Keine temp-Variable gefunden, dann gehen wir mal auf die Suche
    // nach dem System-Laufwerk
    if( nRet )
    {
        nRet = DosScanEnv( (PSZ)"USER_INI",&pVar );
        bAppendTemp = (0 == nRet);
    }
    if( nRet )
    {
        nRet = DosScanEnv( (PSZ)"SYSTEM_INI", &pVar );
        bAppendTemp = (0 == nRet);
    }
    if( nRet )
        // Wenn das immer noch nicht reicht nehmen wir eben die Root von C:
#ifdef __BORLANDC__
        pVar = (PSZ)"c:\\temp\\";
#else
        pVar = (PCSZ)"c:\\temp\\";
#endif
    strcpy( pBuf, (const char*)pVar );

    // jetzt haengt ggf. ein Backlash dran, den wir abschneiden,
    // ggf. inklusive dahinter haengendem Dateinamen
    if ( bRemoveBS )
    {
        char *pTail = pBuf + strlen(pBuf) - 1;
        for ( char cLast = *pTail; cLast != '\\'; cLast = *(--pTail) )
            *pTail = 0;
    }

    if ( bAppendTemp )
        strcat( pBuf, "\\temp" );
    DirEntry( pBuf ).MakeDir();

    return pBuf;
}

#define CURRENT_COUNTRY 0
#define NLS_CODEPAGE 850

/*====================================================================
 * CreateCaseMapImpl()
 * creates a map of each character to convert to lower
 *--------------------------------------------------------------------*/

void CreateCaseMapImpl()
{
    // build a string starting with code 0 as first character upto 255
    char sTemp[256];
    for ( USHORT n = 0; n < 256; ++n )
        sTemp[n] = (char) n;

    // convert string to upper case
    COUNTRYCODE aCountry;
    aCountry.country = CURRENT_COUNTRY;   /* Country code */
    aCountry.codepage = NLS_CODEPAGE;     /* Code page */
    DosMapCase( 255, &aCountry, sTemp+1 );

    // fill a global buffer starting with code 0 as first character upto 255
    for ( n = 0; n < 256; ++n )
        sCaseMap[n] = (char) n;

    // reorder by upper-code and store in a global buffer
    for ( n = 255; n > 0; --n )
        // was this character converted?
        if ( sTemp[n] != (char) n )
            // we found a convertion from upper to lower
            sCaseMap[ (unsigned char) sTemp[n] ] = (char) n;

    bCaseMap = TRUE;
}

String ToLowerImpl( const String& rSource )
{
    if ( !bCaseMap )
        CreateCaseMapImpl();

    // TH sagt: International ist zu langsam, also mit einer eigenen Map
    String aLower( rSource );
    for ( USHORT n = 0; n < aLower.Len(); ++n )
        aLower[n] = sCaseMap[ (unsigned char) aLower[n] ];
    return aLower;
}

/*====================================================================
 * CreateDriveMapImpl()
 * creates a map of drive-infos like FileSystem (style) and Kind (remote)
 *--------------------------------------------------------------------*/

typedef struct _FSQBUFFER_
{
    FSQBUFFER2  aBuf;
    UCHAR       sBuf[64];
} FSQBUFFER_;

void CreateDriveMapImpl()
{
#ifdef POWERPC
    // !!!!! Hack, da der untere Teil mit Beta 2 noch abstuertzt !!!!!
    BYTE nFloppies = 1;
    for ( USHORT nDrive = 0; nDrive < 26; ++nDrive )
    {
        if ( nDrive < nFloppies )
        {
            aDriveMap[nDrive].nKind = FSYS_KIND_REMOVEABLE;
            aDriveMap[nDrive].nStyle = FSYS_STYLE_FAT;
        }
        else
        {
            aDriveMap[nDrive].nKind = FSYS_KIND_UNKNOWN;
            aDriveMap[nDrive].nStyle = FSYS_STYLE_UNKNOWN;
        }
    }

    aDriveMap[2].nKind = FSYS_KIND_FIXED;
    aDriveMap[2].nStyle = FSYS_STYLE_FAT;
#else
    FSQBUFFER_  aBuf;
    ULONG       nBufLen;
    APIRET      nRet;

    // disable error-boxes for hard-errors
    DosError(FERR_DISABLEHARDERR);

    // determine number of floppy-drives
    BYTE nFloppies;
    nRet = DosDevConfig( (void*) &nFloppies, DEVINFO_FLOPPY );

    // reset the map
    for ( USHORT nDrive = 0; nDrive < 26; ++nDrive )
    {
        if ( nDrive < nFloppies )
        {
            aDriveMap[nDrive].nKind = FSYS_KIND_REMOVEABLE;
            aDriveMap[nDrive].nStyle = FSYS_STYLE_FAT;
        }
        else
        {
            aDriveMap[nDrive].nKind = FSYS_KIND_UNKNOWN;
            aDriveMap[nDrive].nStyle = FSYS_STYLE_UNKNOWN;
        }
    }

    // determine file-system via DosOpen/DocDevIOCtrl
    for ( nDrive = 2; nDrive < 26; ++nDrive )
    {
        // open drive
        BOOL bFixed;
        HFILE nDevHandle;
        char pDriveName[3] = "#:";
        pDriveName[0] = nDrive+'a';
        ULONG nAction;
        nRet = DosOpen( (PSZ) pDriveName, &nDevHandle,
            &nAction, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS,
            OPEN_FLAGS_DASD|OPEN_SHARE_DENYNONE|OPEN_ACCESS_READONLY,
            0 );

        // exists?
        if ( !nRet )
        {
            // removeable?
            BYTE nDriveId = nDrive;
            ULONG nParaOutLen, nDataOutLen;
            nRet = DosDevIOCtl(nDevHandle, 8, 0x20,
                &nDriveId, sizeof(nDriveId), &nParaOutLen,
                &bFixed, sizeof(bFixed), &nDataOutLen );

            // prepare the drive-map
            if ( !nRet && !bFixed )
                aDriveMap[nDrive].nKind = FSYS_KIND_REMOVEABLE;

            // close drive
            DosClose(nDevHandle);
        }
        else if ( nRet == ERROR_NOT_READY )
            aDriveMap[nDrive].nKind = FSYS_KIND_REMOVEABLE | FSYS_KIND_CDROM;
    }

    // determine file-system via FSAttach
    nRet = 0;
    for ( USHORT n = 3; nRet != ERROR_NO_MORE_ITEMS; ++n )
    {
        nBufLen = sizeof( aBuf );
        nRet = DosQueryFSAttach( 0, n, FSAIL_DRVNUMBER,
            (_FSQBUFFER2*) &aBuf, &nBufLen );
        if ( !nRet )
        {
            nDrive = toupper(aBuf.aBuf.szName[0]) - 'A';

            if ( aDriveMap[nDrive].nKind == FSYS_KIND_UNKNOWN )
                aDriveMap[nDrive].nKind =
                    aBuf.aBuf.iType == 3 ? FSYS_KIND_FIXED :
                    aBuf.aBuf.iType == 4 ? FSYS_KIND_REMOTE :
                    FSYS_KIND_UNKNOWN;

            char *pType = (char*)(aBuf.aBuf.szName + aBuf.aBuf.cbName + 1);
            aDriveMap[nDrive].nStyle =
                strcmp( pType, "FAT" ) == 0 ? FSYS_STYLE_FAT :
                strcmp( pType, "HPFS" ) == 0 ? FSYS_STYLE_HPFS :
                strcmp( pType, "NWFS" ) == 0 ? FSYS_STYLE_NWFS :
                strcmp( pType, "EXT2" ) == 0 ? FSYS_STYLE_UNX :
                strcmp( pType, "NFS" ) == 0 ? FSYS_STYLE_UNX :
                FSYS_STYLE_UNKNOWN;
            if ( strcmp( pType, "CDFS" ) == 0 )
                aDriveMap[nDrive].nKind = FSYS_KIND_CDROM|FSYS_KIND_REMOVEABLE;
        }
    }
#endif

    bDriveMap = TRUE;
}

Time MsDos2Time( const time_t *pTimeT )
{
    tm *pTm = localtime( pTimeT );
    if ( pTm )
        return Time( pTm->tm_hour, pTm->tm_min, pTm->tm_sec );
    else
        return Time(0);
}

Date MsDos2Date( const time_t *pTimeT )
{
    tm *pTm = localtime( pTimeT );
    if ( pTm )
        return Date( pTm->tm_mday, pTm->tm_mon + 1, pTm->tm_year );
    else
        return Date(0);
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

FSysPathStyle DirEntry::GetPathStyle( const String &rDevice )
{
    // UNC-Pathname?
    if ( !rDevice || ( rDevice.Len() > 1 && rDevice[ (USHORT) 1] != ':' ) )
        return FSYS_STYLE_UNKNOWN;

    if ( !bDriveMap )
        CreateDriveMapImpl();
    return aDriveMap[toupper(rDevice[0]) - 'A'].nStyle;
}

/*************************************************************************
|*
|*    DirEntry::IsCaseSensitive() const
|*
|*    Beschreibung
|*    Ersterstellung    TPF 26.02.1999
|*    Letzte Aenderung
|*
*************************************************************************/

BOOL DirEntry::IsCaseSensitive( FSysPathStyle eFormatter ) const
{
    if (eFormatter==FSYS_STYLE_HOST)
    {
        if  (GetPathStyle(GetDevice().GetName()) == FSYS_STYLE_UNX)
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        BOOL isCaseSensitive = FALSE;           // ich bin unter OS2, also ist der default im Zweifelsfall case insensitiv
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
                    isCaseSensitive = FALSE;    // ich bin unter OS2, also ist der default im Zweifelsfall case insensitiv
                    break;
                }
        }
        return isCaseSensitive;
    }
}




//=========================================================================

ErrCode FileStat::QueryDiskSpace( const String &rPath,
                                  BigInt &rFreeBytes, BigInt &rTotalBytes )
{
    FSALLOCATE aFSInfoBuf;
    String aVolume( DirEntry(rPath).ImpGetTopPtr()->GetName() );
    ULONG nDriveNumber = toupper( aVolume(0) ) - 'A' + 1;

    APIRET rc = DosQueryFSInfo( nDriveNumber, FSIL_ALLOC,
                                &aFSInfoBuf, sizeof(aFSInfoBuf) );
    if ( rc )
        return Sys2SolarError_Impl( rc );

    BigInt aBytesPerCluster( BigInt(aFSInfoBuf.cbSector) *
                             BigInt(aFSInfoBuf.cSectorUnit) );
    rFreeBytes = aBytesPerCluster * BigInt(aFSInfoBuf.cUnitAvail);
    rTotalBytes = aBytesPerCluster * BigInt(aFSInfoBuf.cUnit);
    return 0;
}

//=========================================================================

void FSysEnableSysErrorBox( BOOL bEnable )
{
    DosError( bEnable ? 0 : FERR_DISABLEHARDERR );
}

