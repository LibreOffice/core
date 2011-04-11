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

#define INCL_DOSEXCEPTIONS

#include <stdlib.h>

#ifdef __BORLANDC__
#include <alloc.h>
#else
#include <malloc.h>
#endif
#include <tools/debug.hxx>
#include <tools/bigint.hxx>
#include <tools/fsys.hxx>
#include "comdep.hxx"

#ifdef OS2
#include <osl/mutex.hxx>
#endif

int Sys2SolarError_Impl( int nSysErr );

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

    OSL_TRACE( "FSys: unknown apiret error %d occurred", nApiRet );
    return FSYS_ERR_UNKNOWN;
}

//--------------------------------------------------------------------

char* volumeid( const char* pPfad )
{
    static FSINFO   aFSInfoBuf;
    ULONG           ulFSInfoLevel = FSIL_VOLSER;
    ULONG           nDriveNumber;

    nDriveNumber = toupper(*pPfad) - 'A' + 1;

    if ( nDriveNumber >= 3 )
    {
        APIRET rc = DosQueryFSInfo(
            nDriveNumber, ulFSInfoLevel, &aFSInfoBuf, sizeof(FSINFO) );
        if ( rc )
            return 0;
        return (char*) aFSInfoBuf.vol.szVolLabel;
    }
    return 0;
}

//--------------------------------------------------------------------

/*************************************************************************
|*
|*    DirEntry::ToAbs()
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
        return TRUE;

    char sBuf[_MAX_PATH + 1];
    *this = DirEntry( String( getcwd( sBuf, _MAX_PATH ), osl_getThreadTextEncoding() ) ) + *this;

    return IsAbs();
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
        const char *pVol;
        pVol = volumeid( (char*) pTop->aName.GetBuffer() );
        if (pVol)
            aRet = String( pVol, osl_getThreadTextEncoding());
    }

    return aRet;
}

/*************************************************************************
|*
|*    DirEntry::SetCWD()
|*
*************************************************************************/

BOOL DirEntry::SetCWD( BOOL bSloppy ) const
{
    DBG_CHKTHIS( DirEntry, ImpCheckDirEntry );

    if ( eFlag == FSYS_FLAG_CURRENT && !aName.Len() )
        return TRUE;

    if ( !chdir(ByteString(GetFull(), osl_getThreadTextEncoding()).GetBuffer()) )
    {
        //nError = FSYS_ERR_OK;
        return TRUE;
    }

    if ( bSloppy && pParent &&
         !chdir(ByteString(pParent->GetFull(), osl_getThreadTextEncoding()).GetBuffer()) )
    {
        //nError = FSYS_ERR_OK;
        return TRUE;
    }

    //nError = FSYS_ERR_NOTADIRECTORY;
    return FALSE;
}

/*************************************************************************
|*
|*    DirEntry::MoveTo()
|*
*************************************************************************/


//-------------------------------------------------------------------------

USHORT DirReader_Impl::Init()
{
    // Block-Devices auflisten?
    if ( pDir->eAttrMask & FSYS_KIND_BLOCK )
    {
        CreateDriveMapImpl();
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
            if ( pDir->aNameMask.Matches( String( ByteString(sDrive), osl_getThreadTextEncoding()))
                && aDriveMap[c-'a'].nKind != FSYS_KIND_UNKNOWN )
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
    if (!pDosDir)
    {
        pDosDir = opendir( (char*) ByteString(aPath, osl_getThreadTextEncoding()).GetBuffer() );
    }

    if (!pDosDir)
    {
        bReady = TRUE;
        return 0;
    }

    // Directories und Files auflisten?
    if ( ( pDir->eAttrMask & FSYS_KIND_DIR || pDir->eAttrMask & FSYS_KIND_FILE ) &&
         ( ( pDosEntry = readdir( pDosDir ) ) != NULL ) )
    {
        String aD_Name(pDosEntry->d_name, osl_getThreadTextEncoding());
        if ( pDir->aNameMask.Matches( aD_Name  ) )
        {
            DirEntryFlag eFlag =
                    0 == strcmp( pDosEntry->d_name, "." ) ? FSYS_FLAG_CURRENT
                :   0 == strcmp( pDosEntry->d_name, ".." ) ? FSYS_FLAG_PARENT
                :   FSYS_FLAG_NORMAL;
            DirEntry *pTemp = new DirEntry( ByteString(pDosEntry->d_name), eFlag, FSYS_STYLE_UNX );
            if ( pParent )
                pTemp->ImpChangeParent( new DirEntry( *pParent ), FALSE);
            FileStat aStat( *pTemp );
            if ( ( ( ( pDir->eAttrMask & FSYS_KIND_DIR ) &&
                     ( aStat.IsKind( FSYS_KIND_DIR ) ) ) ||
                   ( ( pDir->eAttrMask & FSYS_KIND_FILE ) &&
                     !( aStat.IsKind( FSYS_KIND_DIR ) ) ) ) &&
                 !( pDir->eAttrMask & FSYS_KIND_VISIBLE &&
                    pDosEntry->d_name[0] == '.' ) )
            {
                if ( pDir->pStatLst ) //Status fuer Sort gewuenscht?
                    pDir->ImpSortedInsert( pTemp, new FileStat( aStat ) );
                else
                    pDir->ImpSortedInsert( pTemp, NULL );;
                return 1;
            }
            else
                delete pTemp;
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
    struct dirent *pDirent = (struct dirent*) pInfo;

    nSize = pDirent->d_size;

    aDateCreated  = MsDos2Date( (FDATE*) &pDirent->d_date );
    aTimeCreated  = MsDos2Time( (FTIME*) &pDirent->d_time );
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
*************************************************************************/

struct _FSYS_FSQBUFFER
{
    FSQBUFFER2  aBuf;
    UCHAR       sBuf[256];
};

BOOL FileStat::Update( const DirEntry& rDirEntry, BOOL bAccessRemovableDevice )
{
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

    if ( !rDirEntry.IsValid() )
    {
        nError = FSYS_ERR_NOTEXISTS;
        return FALSE;
    }

    // Sonderbehandlung falls es sich um eine Root ohne Laufwerk handelt
    if ( !rDirEntry.aName.Len() && rDirEntry.eFlag == FSYS_FLAG_ABSROOT )
    {
        nKindFlags = FSYS_KIND_DIR;
        nError = FSYS_ERR_OK;
        return TRUE;
    }

    // Sonderbehandlung falls es sich um eine Wildcard handelt
    ByteString aTempName( rDirEntry.GetName(), osl_getThreadTextEncoding() );
    if ( strchr( aTempName.GetBuffer(), '?' ) ||
         strchr( aTempName.GetBuffer(), '*' ) ||
         strchr( aTempName.GetBuffer(), ';' ) )
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

            ByteString rDirEntryUpperCase = ByteString( rDirEntry.aName ).ToUpperAscii();
            DriveMapItem &rItem = aDriveMap[rDirEntryUpperCase.GetChar(0) - 'A'];
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

    aTempDirEntry.ToAbs();
    ByteString aFullName( aTempDirEntry.GetFull(), osl_getThreadTextEncoding() );

    p = (char *) aFullName.GetBuffer();

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

BOOL IsRedirectable_Impl( const ByteString &rPath )
{
    if ( rPath.Len() >= 3 && ':' == rPath.GetBuffer()[1] )
    {
        ByteString aVolume = rPath.Copy( 0, 3 );
        DriveMapItem &rItem = aDriveMap[toupper(aVolume.GetChar(0)) - 'A'];
        return FSYS_KIND_FIXED != rItem.nKind;
    }
    return FALSE;
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
    PSZ         pVar;
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
    if( nRet )
        nRet = DosScanEnv( (PSZ)"TMPDIR", &pVar );

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
    USHORT      nDrive;

    // disable error-boxes for hard-errors
    DosError(FERR_DISABLEHARDERR);

    // determine number of floppy-drives
    BYTE nFloppies;
    nRet = DosDevConfig( (void*) &nFloppies, DEVINFO_FLOPPY );

    // reset the map
    for ( nDrive = 0; nDrive < 26; ++nDrive )
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
                strcmp( pType, "FAT32" ) == 0 ? FSYS_STYLE_VFAT :
                strcmp( pType, "NTFS" ) == 0 ? FSYS_STYLE_NTFS :
                strcmp( pType, "HPFS" ) == 0 ? FSYS_STYLE_HPFS :
                strcmp( pType, "JFS" ) == 0 ? FSYS_STYLE_HPFS :
                strcmp( pType, "RAMFS" ) == 0 ? FSYS_STYLE_HPFS :
                strcmp( pType, "NDFS32" ) == 0 ? FSYS_STYLE_HPFS :
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
*************************************************************************/

FSysPathStyle DirEntry::GetPathStyle( const String &rDevice )
{
    ByteString aRootDir(rDevice, osl_getThreadTextEncoding());
    // UNC-Pathname?
    if ( aRootDir.Len()==0 || ( aRootDir.Len() > 1 && aRootDir.GetChar(1) != ':' ) )
        return FSYS_STYLE_UNKNOWN;

    if ( !bDriveMap )
        CreateDriveMapImpl();
    return aDriveMap[toupper(aRootDir.GetChar(0)) - 'A'].nStyle;
}

/*************************************************************************
|*
|*    DirEntry::IsCaseSensitive() const
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
    ByteString aVol( DirEntry(rPath).ImpGetTopPtr()->GetName(), osl_getThreadTextEncoding());
    ULONG nDriveNumber = toupper( aVol.GetChar(0) ) - 'A' + 1;

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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
