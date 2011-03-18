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

#include <string.h>
#include <limits.h>

#define INCL_PM
#define INCL_DOS
#define INCL_DOSERRORS
#include <svpm.h>

#include <tools/debug.hxx>
#include <tools/fsys.hxx>
#include <tools/stream.hxx>

// class FileBase
#include <osl/file.hxx>

using namespace osl;

// class FileBase
#include <osl/file.hxx>

using namespace osl;

// -----------------------------------------------------------------------

// --------------
// - StreamData -
// --------------

class StreamData
{
public:
    HFILE   hFile;
    sal_Bool    bIsEof;

            StreamData()
            {
                hFile = 0;
                bIsEof = sal_True;
            }
};

// -----------------------------------------------------------------------

sal_uIntPtr GetSvError( APIRET nPMError )
{
    static struct { APIRET pm; sal_uIntPtr sv; } errArr[] =
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

    sal_uIntPtr nRetVal = SVSTREAM_GENERALERROR;    // Standardfehler
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
*************************************************************************/

SvFileStream::SvFileStream( const String& rFileName, StreamMode nOpenMode )
{
    bIsOpen             = sal_False;
    nLockCounter        = 0;
    bIsWritable         = sal_False;
    pInstanceData       = new StreamData;

    SetBufferSize( 8192 );
    // convert URL to SystemPath, if necessary
    ::rtl::OUString aFileName, aNormPath;

    if ( FileBase::getSystemPathFromFileURL( rFileName, aFileName ) != FileBase::E_None )
        aFileName = rFileName;
    Open( aFileName, nOpenMode );
}

/*************************************************************************
|*
|*    SvFileStream::SvFileStream()
|*
*************************************************************************/

SvFileStream::SvFileStream()
{
    bIsOpen             = sal_False;
    nLockCounter        = 0;
    bIsWritable         = sal_False;
    pInstanceData       = new StreamData;
    SetBufferSize( 8192 );
}

/*************************************************************************
|*
|*    SvFileStream::~SvFileStream()
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
*************************************************************************/

sal_uIntPtr SvFileStream::GetFileHandle() const
{
    return (sal_uIntPtr)pInstanceData->hFile;
}

/*************************************************************************
|*
|*    SvFileStream::IsA()
|*
*************************************************************************/

sal_uInt16 SvFileStream::IsA() const
{
    return ID_FILESTREAM;
}

/*************************************************************************
|*
|*    SvFileStream::GetData()
|*
|*    Beschreibung      STREAM.SDW, Prueft nicht Eof; IsEof danach rufbar
|*
*************************************************************************/

sal_uIntPtr SvFileStream::GetData( void* pData, sal_uIntPtr nSize )
{
#ifdef DBG_UTIL
    ByteString aTraceStr( "SvFileStream::GetData(): " );
    aTraceStr += ByteString::CreateFromInt64(nSize);
    aTraceStr += " Bytes from ";
    aTraceStr += ByteString(aFilename, osl_getThreadTextEncoding());
    OSL_TRACE( "%s", aTraceStr.GetBuffer() );
#endif

    sal_uIntPtr nCount = 0L;
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
*************************************************************************/

sal_uIntPtr SvFileStream::PutData( const void* pData, sal_uIntPtr nSize )
{
#ifdef DBG_UTIL
    ByteString aTraceStr( "SvFileStrean::PutData: " );
    aTraceStr += ByteString::CreateFromInt64(nSize);
    aTraceStr += " Bytes to ";
    aTraceStr += ByteString(aFilename, osl_getThreadTextEncoding());
    OSL_TRACE( "%s", aTraceStr.GetBuffer() );
#endif

    sal_uIntPtr nCount = 0L;
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
*************************************************************************/

sal_uIntPtr SvFileStream::SeekPos( sal_uIntPtr nPos )
{
    sal_uIntPtr nNewPos = 0L;
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
*************************************************************************/
/*
sal_uIntPtr SvFileStream::Tell()
{
    sal_uIntPtr nPos = 0L;

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
*************************************************************************/

sal_Bool SvFileStream::LockRange( sal_uIntPtr nByteOffset, sal_uIntPtr nBytes )
{
    sal_Bool bRetVal = sal_False;
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
            bRetVal = sal_True;
    }
    return bRetVal;
}

/*************************************************************************
|*
|*    SvFileStream::UnlockRange()
|*
*************************************************************************/

sal_Bool SvFileStream::UnlockRange( sal_uIntPtr nByteOffset, sal_uIntPtr nBytes )
{
    sal_Bool bRetVal = sal_False;
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
            bRetVal = sal_True;
    }
    return bRetVal;
}

/*************************************************************************
|*
|*    SvFileStream::LockFile()
|*
*************************************************************************/

sal_Bool SvFileStream::LockFile()
{
    sal_Bool bRetVal = sal_False;
    if( !nLockCounter )
    {
        if( LockRange( 0L, LONG_MAX ) )
        {
            nLockCounter = 1;
            bRetVal = sal_True;
        }
    }
    else
    {
        nLockCounter++;
        bRetVal = sal_True;
    }
    return bRetVal;
}

/*************************************************************************
|*
|*    SvFileStream::UnlockFile()
|*
*************************************************************************/

sal_Bool SvFileStream::UnlockFile()
{
    sal_Bool bRetVal = sal_False;
    if( nLockCounter > 0)
    {
        if( nLockCounter == 1)
        {
            if( UnlockRange( 0L, LONG_MAX ) )
            {
                nLockCounter = 0;
                bRetVal = sal_True;
            }
        }
        else
        {
            nLockCounter--;
            bRetVal = sal_True;
        }
    }
    return bRetVal;
}

/*************************************************************************
|*
|*    SvFileStream::Open()
|*
*************************************************************************/

void SvFileStream::Open( const String& rFilename, StreamMode nOpenMode )
{
        String aParsedFilename;

        {
                aParsedFilename = rFilename;
        }

    Close();
    SvStream::ClearBuffer();

    sal_uIntPtr   nActionTaken;
    sal_uIntPtr   nOpenAction     = 0L;
    sal_uIntPtr   nShareBits      = 0L;
    sal_uIntPtr   nReadWriteBits  = 0L;

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

    aFilename = aParsedFilename;
    ByteString aFileNameA( aFilename, gsl_getSystemTextEncoding());
    FSysRedirector::DoRedirect( aFilename );

#ifdef DBG_UTIL
    ByteString aTraceStr( "SvFileStream::Open(): " );
    aTraceStr +=  aFileNameA;
    OSL_TRACE( "%s", aTraceStr.GetBuffer() );
#endif

    APIRET nRet = DosOpen( aFileNameA.GetBuffer(), &pInstanceData->hFile,
    &nActionTaken, 0L, FILE_NORMAL, nOpenAction,
    nReadWriteBits | nShareBits | OPEN_FLAGS_NOINHERIT, 0L);

    if( nRet == ERROR_TOO_MANY_OPEN_FILES )
    {
        long nToAdd = 10;
        sal_uIntPtr nCurMaxFH;
        nRet = DosSetRelMaxFH( &nToAdd, &nCurMaxFH );
        nRet = DosOpen( aFileNameA.GetBuffer(), &pInstanceData->hFile,
        &nActionTaken, 0L, FILE_NORMAL, nOpenAction,
        nReadWriteBits | nShareBits | OPEN_FLAGS_NOINHERIT, 0L);
    }

    // Bei Fehler pruefen, ob wir lesen duerfen
    if( nRet==ERROR_ACCESS_DENIED || nRet==ERROR_SHARING_VIOLATION )
    {
        nReadWriteBits = OPEN_ACCESS_READONLY;
        nRet = DosOpen( aFileNameA.GetBuffer(), &pInstanceData->hFile,
            &nActionTaken, 0L, FILE_NORMAL, nOpenAction,
            nReadWriteBits | nShareBits | OPEN_FLAGS_NOINHERIT, 0L);
    }

        if( nRet )
    {
        bIsOpen = sal_False;
        SetError(::GetSvError(nRet) );
    }
    else
    {
        bIsOpen     = sal_True;
        pInstanceData->bIsEof = sal_False;
        if( nReadWriteBits != OPEN_ACCESS_READONLY )
            bIsWritable = sal_True;
    }
}

/*************************************************************************
|*
|*    SvFileStream::ReOpen()
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
*************************************************************************/

void SvFileStream::Close()
{
    if( IsOpen() )
    {
#ifdef DBG_UTIL
        ByteString aTraceStr( "SvFileStream::Close(): " );
        aTraceStr += ByteString(aFilename, osl_getThreadTextEncoding());
        OSL_TRACE( "%s", aTraceStr.GetBuffer() );
#endif

        if( nLockCounter )
        {
            nLockCounter = 1;
            UnlockFile();
        }
        Flush();
        DosClose( pInstanceData->hFile );
    }

    bIsOpen     = sal_False;
    nLockCounter= 0;
    bIsWritable = sal_False;
    pInstanceData->bIsEof = sal_True;
    SvStream::ClearBuffer();
    SvStream::ClearError();
}

/*************************************************************************
|*
|*    SvFileStream::ResetError()
|*
|*    Beschreibung      STREAM.SDW; Setzt Filepointer auf Dateianfang
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
*************************************************************************/

void SvFileStream::SetSize( sal_uIntPtr nSize )
{
    if( IsOpen() )
    {
        APIRET nRet = DosSetFileSize( pInstanceData->hFile, nSize );
        if( nRet )
            SetError( ::GetSvError( nRet ) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
