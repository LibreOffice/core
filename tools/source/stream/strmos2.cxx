/*************************************************************************
 *
 *  $RCSfile: strmos2.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:09 $
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

#include <string.h>
#include <limits.h>

#define INCL_PM
#define INCL_DOS
#include <svpm.h>
#include <bseerr.h>

#include <debug.hxx>
#include <fsys.hxx>
#include <stream.hxx>
#include <fastfsys.hxx>

// -----------------------------------------------------------------------

// --------------
// - StreamData -
// --------------

class StreamData
{
public:
    HFILE   hFile;
    BOOL    bIsEof;

            StreamData()
            {
                hFile = 0;
                bIsEof = TRUE;
            }
};

// -----------------------------------------------------------------------

ULONG GetSvError( APIRET nPMError )
{
    static struct { APIRET pm; ULONG sv; } errArr[] =
    {
        { ERROR_FILE_NOT_FOUND,         SVSTREAM_FILE_NOT_FOUND },
        { ERROR_PATH_NOT_FOUND,         SVSTREAM_PATH_NOT_FOUND },
        { ERROR_TOO_MANY_OPEN_FILES,    SVSTREAM_TOO_MANY_OPEN_FILES },
        { ERROR_ACCESS_DENIED,          SVSTREAM_ACCESS_DENIED },
        { ERROR_INVALID_ACCESS,         SVSTREAM_INVALID_ACCESS },
        { ERROR_SHARING_VIOLATION,      SVSTREAM_SHARING_VIOLATION },
        { ERROR_SHARING_BUFFER_EXCEEDED,SVSTREAM_SHARE_BUFF_EXCEEDED },
        { ERROR_CANNOT_MAKE,            SVSTREAM_CANNOT_MAKE },
        { ERROR_INVALID_PARAMETER,      SVSTREAM_INVALID_PARAMETER },
        { ERROR_DRIVE_LOCKED,           SVSTREAM_LOCKING_VIOLATION },
        { ERROR_LOCK_VIOLATION,      SVSTREAM_LOCKING_VIOLATION },
        { ERROR_FILENAME_EXCED_RANGE,   SVSTREAM_INVALID_PARAMETER },
        { ERROR_ATOMIC_LOCK_NOT_SUPPORTED, SVSTREAM_INVALID_PARAMETER },
        { ERROR_READ_LOCKS_NOT_SUPPORTED, SVSTREAM_INVALID_PARAMETER },


        { 0xFFFF, SVSTREAM_GENERALERROR }
    };

    ULONG nRetVal = SVSTREAM_GENERALERROR;    // Standardfehler
    int i=0;
    do
    {
        if( errArr[i].pm == nPMError )
        {
            nRetVal = errArr[i].sv;
            break;
        }
        i++;
    }
    while( errArr[i].pm != 0xFFFF );
    return nRetVal;
}

/*************************************************************************
|*
|*    SvFileStream::SvFileStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

SvFileStream::SvFileStream( const String& rFileName, StreamMode nOpenMode )
{
    bIsOpen             = FALSE;
    nLockCounter        = 0;
    bIsWritable         = FALSE;
    pInstanceData       = new StreamData;

    SetBufferSize( 8192 );
    Open( rFileName, nOpenMode );
}

/*************************************************************************
|*
|*    SvFileStream::SvFileStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 22.11.94
|*    Letzte Aenderung  OV 22.11.94
|*
*************************************************************************/

SvFileStream::SvFileStream()
{
    bIsOpen             = FALSE;
    nLockCounter        = 0;
    bIsWritable         = FALSE;
    pInstanceData       = new StreamData;
    SetBufferSize( 8192 );
}

/*************************************************************************
|*
|*    SvFileStream::~SvFileStream()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 14.06.94
|*    Letzte Aenderung  OV 14.06.94
|*
*************************************************************************/

SvFileStream::~SvFileStream()
{
    Close();
    if( pInstanceData )
        delete pInstanceData;
}

/*************************************************************************
|*
|*    SvFileStream::GetFileHandle()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 14.06.94
|*    Letzte Aenderung  OV 14.06.94
|*
*************************************************************************/

ULONG SvFileStream::GetFileHandle() const
{
    return (ULONG)pInstanceData->hFile;
}

/*************************************************************************
|*
|*    SvFileStream::IsA()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 14.06.94
|*    Letzte Aenderung  OV 14.06.94
|*
*************************************************************************/

USHORT SvFileStream::IsA() const
{
    return ID_FILESTREAM;
}

/*************************************************************************
|*
|*    SvFileStream::GetData()
|*
|*    Beschreibung      STREAM.SDW, Prueft nicht Eof; IsEof danach rufbar
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

ULONG SvFileStream::GetData( char* pData, ULONG nSize )
{
#ifdef DBG_UTIL
    String aTraceStr( "SvFileStream::GetData(): " );
    aTraceStr += nSize;
    aTraceStr += " Bytes from ";
    aTraceStr += aFilename;
    DBG_TRACE( aTraceStr );
#endif

    ULONG nCount = 0L;
    if( IsOpen() )
    {
        APIRET nResult;
        nResult = DosRead( pInstanceData->hFile,(PVOID)pData,nSize,&nCount );
        if( nResult )
            SetError(::GetSvError(nResult) );
    }
    return nCount;
}

/*************************************************************************
|*
|*    SvFileStream::PutData()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

ULONG SvFileStream::PutData( const char* pData, ULONG nSize )
{
#ifdef DBG_UTIL
    String aTraceStr( "SvFileStrean::PutData: " );
    aTraceStr += nSize;
    aTraceStr += " Bytes to ";
    aTraceStr += aFilename;
    DBG_TRACE( aTraceStr );
#endif

    ULONG nCount = 0L;
    if( IsOpen() )
    {
        APIRET nResult;
        nResult = DosWrite( pInstanceData->hFile,(PVOID)pData,nSize,&nCount );
        if( nResult )
            SetError(::GetSvError(nResult) );
        else if( !nCount )
            SetError( SVSTREAM_DISK_FULL );
    }
    return nCount;
}

/*************************************************************************
|*
|*    SvFileStream::SeekPos()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

ULONG SvFileStream::SeekPos( ULONG nPos )
{
    ULONG nNewPos = 0L;
    if( IsOpen() )
    {
        APIRET nResult;

        if( nPos != STREAM_SEEK_TO_END )
            nResult = DosSetFilePtr( pInstanceData->hFile,(long)nPos,
                 FILE_BEGIN, &nNewPos );
        else
            nResult = DosSetFilePtr( pInstanceData->hFile,0L,
                 FILE_END, &nNewPos );

        if( nResult )
            SetError(::GetSvError(nResult) );
    }
    else
        SetError( SVSTREAM_GENERALERROR );
    return nNewPos;
}

/*************************************************************************
|*
|*    SvFileStream::Tell()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/
/*
ULONG SvFileStream::Tell()
{
    ULONG nPos = 0L;

    if( IsOpen() )
    {
        APIRET nResult;
        nResult = DosSetFilePtr(pInstanceData->hFile,0L,FILE_CURRENT,&nPos);
        if( nResult )
            SetError(::GetSvError(nResult) );
    }
    return nPos;
}
*/

/*************************************************************************
|*
|*    SvFileStream::FlushData()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

void SvFileStream::FlushData()
{
    if( IsOpen() )
    {
        APIRET nResult;
        nResult = DosResetBuffer(pInstanceData->hFile );
        if( nResult )
            SetError(::GetSvError(nResult) );
    }
}

/*************************************************************************
|*
|*    SvFileStream::LockRange()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

BOOL SvFileStream::LockRange( ULONG nByteOffset, ULONG nBytes )
{
    BOOL bRetVal = FALSE;
    if( IsOpen() )
    {
        APIRET   nResult;
        FILELOCK aLockArea, aUnlockArea;
        aUnlockArea.lOffset = 0L;
        aUnlockArea.lRange      = 0L;
        aLockArea.lOffset       = (long)nByteOffset;
        aLockArea.lRange        = (long)nBytes;

        nResult = DosSetFileLocks(pInstanceData->hFile,
            &aUnlockArea, &aLockArea,
            1000UL,  // Zeit in ms bis Abbruch
            0L       // kein Atomic-Lock
        );

        if( nResult )
            SetError(::GetSvError(nResult) );
        else
            bRetVal = TRUE;
    }
    return bRetVal;
}

/*************************************************************************
|*
|*    SvFileStream::UnlockRange()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

BOOL SvFileStream::UnlockRange( ULONG nByteOffset, ULONG nBytes )
{
    BOOL bRetVal = FALSE;
    if( IsOpen() )
    {
        APIRET   nResult;
        FILELOCK aLockArea, aUnlockArea;
        aLockArea.lOffset       = 0L;
        aLockArea.lRange        = 0L;
        aUnlockArea.lOffset = (long)nByteOffset;
        aUnlockArea.lRange      = (long)nBytes;

        nResult = DosSetFileLocks(pInstanceData->hFile,
            &aUnlockArea, &aLockArea,
            1000UL,  // Zeit in ms bis Abbruch
            0L       // kein Atomic-Lock
        );

        if( nResult )
            SetError(::GetSvError(nResult) );
        else
            bRetVal = TRUE;
    }
    return bRetVal;
}

/*************************************************************************
|*
|*    SvFileStream::LockFile()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

BOOL SvFileStream::LockFile()
{
    BOOL bRetVal = FALSE;
    if( !nLockCounter )
    {
        if( LockRange( 0L, LONG_MAX ) )
        {
            nLockCounter = 1;
            bRetVal = TRUE;
        }
    }
    else
    {
        nLockCounter++;
        bRetVal = TRUE;
    }
    return bRetVal;
}

/*************************************************************************
|*
|*    SvFileStream::UnlockFile()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

BOOL SvFileStream::UnlockFile()
{
    BOOL bRetVal = FALSE;
    if( nLockCounter > 0)
    {
        if( nLockCounter == 1)
        {
            if( UnlockRange( 0L, LONG_MAX ) )
            {
                nLockCounter = 0;
                bRetVal = TRUE;
            }
        }
        else
        {
            nLockCounter--;
            bRetVal = TRUE;
        }
    }
    return bRetVal;
}

/*************************************************************************
|*
|*    SvFileStream::Open()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

BOOL createLongNameEA   ( const PCSZ pszPath, ULONG ulAttributes, const String& aLongName );

void SvFileStream::Open( const String& rFilename, StreamMode nOpenMode )
{
        String aParsedFilename;

        if      ( Folder::IsAvailable() && (rFilename.Search("{") < 9) )
        {
                String          aVirtualPart;
                String          aRealPart;
                String          aVirtualPath;
                ItemIDPath      aVirtualURL;
                ULONG           nDivider = 0;

                String          aVirtualString(rFilename);

                for (int x=aVirtualString.Len(); x>0; x--)
                {
                        if (aVirtualString.Copy(x,1).Compare("}")==COMPARE_EQUAL)
                        {
                                nDivider = x;
                                break;
                        }
                }

                aVirtualPart = aVirtualString.Copy(0,nDivider+1);
                aRealPart = aVirtualString.Copy(nDivider+2);

                aVirtualURL  = aVirtualPart;
                aVirtualPath = aVirtualURL.GetHostNotationPath();

                DirEntry aTempDirEntry(aVirtualPath);

                aTempDirEntry += aRealPart;

                aParsedFilename = aTempDirEntry.GetFull();
        }
        else
        {
                aParsedFilename = rFilename;
        }

    Close();
    SvStream::ClearBuffer();

    ULONG   nActionTaken;
    ULONG   nOpenAction     = 0L;
    ULONG   nShareBits      = 0L;
    ULONG   nReadWriteBits  = 0L;

    eStreamMode = nOpenMode;
    eStreamMode &= ~STREAM_TRUNC; // beim ReOpen nicht cutten

    nOpenMode |= STREAM_SHARE_DENYNONE;  // definierten Zustand garantieren

    // ********* Zugriffsflags ***********
    if( nOpenMode & STREAM_SHARE_DENYNONE)
        nShareBits = OPEN_SHARE_DENYNONE;

    if( nOpenMode & STREAM_SHARE_DENYREAD)
        nShareBits = OPEN_SHARE_DENYREAD;

    if( nOpenMode & STREAM_SHARE_DENYWRITE)
        nShareBits = OPEN_SHARE_DENYWRITE;

    if( nOpenMode & STREAM_SHARE_DENYALL)
        nShareBits = OPEN_SHARE_DENYREADWRITE;

        if( (nOpenMode & STREAM_READ) )
    {
        if( nOpenMode & STREAM_WRITE )
            nReadWriteBits |= OPEN_ACCESS_READWRITE;
        else
        {
            nReadWriteBits |= OPEN_ACCESS_READONLY;
            nOpenMode |= STREAM_NOCREATE;
        }
    }
    else
        nReadWriteBits |= OPEN_ACCESS_WRITEONLY;


    if( nOpenMode & STREAM_NOCREATE )
    {
        // Datei nicht erzeugen
        nOpenAction = OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS;
    }
    else
    {
        // Datei erzeugen, wenn nicht vorhanden
        nOpenAction = OPEN_ACTION_CREATE_IF_NEW;
        if( nOpenMode & STREAM_TRUNC )
            // Auf Nullaenge kuerzen, wenn existiert
            nOpenAction |= OPEN_ACTION_REPLACE_IF_EXISTS;
        else
            // Inhalt der Datei nicht wegwerfen
            nOpenAction |= OPEN_ACTION_OPEN_IF_EXISTS;
    }

    //
    // resolves long FAT names used by OS2
    //
    BOOL bIsLongOS2=FALSE;
    if (Folder::IsAvailable())
    {
        DirEntry aDirEntry(rFilename);
        if  (aDirEntry.IsLongNameOnFAT())
        {
            // in kurzen Pfad wandeln
            ItemIDPath      aItemIDPath(rFilename);
            aParsedFilename = aItemIDPath.GetHostNotationPath();
            bIsLongOS2 = TRUE;
        }
    }

    aFilename = aParsedFilename;
    FSysRedirector::DoRedirect( aFilename );

#ifdef DBG_UTIL
    String aTraceStr( "SvFileStream::Open(): " );
    aTraceStr += aFilename;
    DBG_TRACE( aTraceStr );
#endif

    APIRET nRet = DosOpen( (const char*)aFilename, &pInstanceData->hFile,
    &nActionTaken, 0L, FILE_NORMAL, nOpenAction,
    nReadWriteBits | nShareBits | OPEN_FLAGS_NOINHERIT, 0L);

    if( nRet == ERROR_TOO_MANY_OPEN_FILES )
    {
        long nToAdd = 10;
        ULONG nCurMaxFH;
        nRet = DosSetRelMaxFH( &nToAdd, &nCurMaxFH );
        nRet = DosOpen( (const char*)aFilename, &pInstanceData->hFile,
        &nActionTaken, 0L, FILE_NORMAL, nOpenAction,
        nReadWriteBits | nShareBits | OPEN_FLAGS_NOINHERIT, 0L);
    }

    // Bei Fehler pruefen, ob wir lesen duerfen
    if( nRet==ERROR_ACCESS_DENIED || nRet==ERROR_SHARING_VIOLATION )
    {
        nReadWriteBits = OPEN_ACCESS_READONLY;
        nRet = DosOpen( (const char*)aFilename, &pInstanceData->hFile,
            &nActionTaken, 0L, FILE_NORMAL, nOpenAction,
            nReadWriteBits | nShareBits | OPEN_FLAGS_NOINHERIT, 0L);
    }

        if( nRet )
    {
        bIsOpen = FALSE;
        SetError(::GetSvError(nRet) );
    }
    else
    {
        bIsOpen     = TRUE;
        pInstanceData->bIsEof = FALSE;
        if( nReadWriteBits != OPEN_ACCESS_READONLY )
            bIsWritable = TRUE;
    }

    if (bIsOpen && bIsLongOS2)
    {
        //file schließen, da sonst createLongName u.U. nicht möglich
        Close();

        // erzeugtem File langen Namen geben
        DirEntry aDirEntry(rFilename);
        createLongNameEA((const char*)aFilename,  FILE_NORMAL, aDirEntry.GetName());

        // und wieder oeffnen
        ReOpen();
    }

}

/*************************************************************************
|*
|*    SvFileStream::ReOpen()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

void SvFileStream::ReOpen()
{
    if( !bIsOpen && aFilename.Len() )
        Open( aFilename, eStreamMode );
}

/*************************************************************************
|*
|*    SvFileStream::Close()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

void SvFileStream::Close()
{
    if( IsOpen() )
    {
#ifdef DBG_UTIL
        String aTraceStr( "SvFileStream::Close(): " );
        aTraceStr += aFilename;
        DBG_TRACE( aTraceStr );
#endif

        if( nLockCounter )
        {
            nLockCounter = 1;
            UnlockFile();
        }
        Flush();
        DosClose( pInstanceData->hFile );
    }

    bIsOpen     = FALSE;
    nLockCounter= 0;
    bIsWritable = FALSE;
    pInstanceData->bIsEof = TRUE;
    SvStream::ClearBuffer();
    SvStream::ClearError();
}

/*************************************************************************
|*
|*    SvFileStream::ResetError()
|*
|*    Beschreibung      STREAM.SDW; Setzt Filepointer auf Dateianfang
|*    Ersterstellung    OV 15.06.94
|*    Letzte Aenderung  OV 15.06.94
|*
*************************************************************************/

void SvFileStream::ResetError()
{
    SvStream::ClearError();
}

/*************************************************************************
|*
|*    SvFileStream::SetSize()
|*
|*    Beschreibung
|*    Ersterstellung    OV 19.10.95
|*    Letzte Aenderung  OV 19.10.95
|*
*************************************************************************/

void SvFileStream::SetSize( ULONG nSize )
{
    if( IsOpen() )
    {
        APIRET nRet = DosSetFileSize( pInstanceData->hFile, nSize );
        if( nRet )
            SetError( ::GetSvError( nRet ) );
    }
}

/*************************************************************************
|*
|*    SvSharedMemoryStream::AllocateMemory()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    CL 05.05.95
|*    Letzte Aenderung  CL 05.05.95
|*
*************************************************************************/

BOOL SvSharedMemoryStream::AllocateMemory( ULONG nNewSize )
{
        DBG_ASSERT(aHandle==0,"Keine Handles unter OS/2");
        DBG_ASSERT(nNewSize,"Cannot allocate zero Bytes");
        APIRET nRet = DosAllocSharedMem( (void**)&pBuf, (PSZ)NULL, nNewSize,
                                     PAG_READ | PAG_WRITE | PAG_COMMIT |
                                     OBJ_GIVEABLE | OBJ_GETTABLE );
    return( nRet == 0 );
}

/*************************************************************************
|*
|*    SvSharedMemoryStream::ReAllocateMemory()   (Bozo-Algorithmus)
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    CL 05.05.95
|*    Letzte Aenderung  CL 05.05.95
|*
*************************************************************************/

BOOL SvSharedMemoryStream::ReAllocateMemory( long nDiff )
{
        DBG_ASSERT(aHandle==0,"Keine Handles unter OS/2");
        BOOL bRetVal    = FALSE;
    ULONG nNewSize  = nSize + nDiff;
        if( nNewSize )
        {
                // neuen Speicher nicht ueber AllocateMemory holen, da wir den
                // alten Speicher behalten wollen, falls nicht genuegend Platz
                // fuer den neuen Block da ist
                char* pNewBuf;
            APIRET nRet = DosAllocSharedMem( (void**)&pNewBuf,(PSZ)NULL,nNewSize,
                                PAG_READ | PAG_WRITE | PAG_COMMIT |
                                OBJ_GIVEABLE | OBJ_GETTABLE );
            DBG_ASSERT(!nRet,"DosAllocSharedMem failed");

            if( !nRet )
            {
                bRetVal = TRUE; // Success!
                if( nNewSize < nSize )      // Verkleinern ?
                {
                    memcpy( pNewBuf, pBuf, (size_t)nNewSize );
                    if( nPos > nNewSize )
                        nPos = 0L;
                    if( nEndOfData >= nNewSize )
                        nEndOfData = nNewSize-1L;
                }
                else
                    memcpy( pNewBuf, pBuf, (size_t)nSize );

                FreeMemory(); // den alten Block loeschen ...

                        // und den neuen Block in Dienst stellen
                        pBuf  = pNewBuf;
                nSize = nNewSize;
            }
        }
        else
        {
                bRetVal = TRUE;
                FreeMemory();
                pBuf = 0;
                nSize = 0;
                nEndOfData = 0;
        }
    return bRetVal;
}

void SvSharedMemoryStream::FreeMemory()
{
        DBG_ASSERT(aHandle==0,"Keine Handles unter OS/2");
    DosFreeMem( pBuf );
}

/*************************************************************************
|*
|*    SvSharedMemoryStream::SetHandle()
|*
|*    Beschreibung      STREAM.SDW
|*    Ersterstellung    OV 05.10.95
|*    Letzte Aenderung  OV 05.10.95
|*
*************************************************************************/

void* SvSharedMemoryStream::SetHandle( void*, ULONG,BOOL, ULONG )
{
        DBG_ERROR("OS/2 does not support memory handles");
    // return SetBuffer(aNewHandle, nSize, bOwnsData, nEOF );
        return 0;
}


