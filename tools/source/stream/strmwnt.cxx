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

// TODO: StreamMode <-> AllocateMemory

#include <string.h>
#include <limits.h>

#ifdef WNT
#include <windows.h>
#endif

#include <tools/fsys.hxx>
#include <tools/stream.hxx>

// class FileBase
#include <osl/file.hxx>
using namespace osl;

class StreamData
{
public:
    HANDLE      hFile;

                StreamData()
                {
                    hFile = 0;
                }
};

static sal_uIntPtr GetSvError( DWORD nWntError )
{
    static struct { DWORD wnt; sal_uIntPtr sv; } errArr[] =
    {
        { ERROR_SUCCESS,                SVSTREAM_OK },
        { ERROR_ACCESS_DENIED,          SVSTREAM_ACCESS_DENIED },
        { ERROR_ACCOUNT_DISABLED,       SVSTREAM_ACCESS_DENIED },
        { ERROR_ACCOUNT_EXPIRED,        SVSTREAM_ACCESS_DENIED },
        { ERROR_ACCOUNT_RESTRICTION,    SVSTREAM_ACCESS_DENIED },
        { ERROR_ATOMIC_LOCKS_NOT_SUPPORTED, SVSTREAM_INVALID_PARAMETER },
        { ERROR_BAD_PATHNAME,           SVSTREAM_PATH_NOT_FOUND },
        // Filename too long
        { ERROR_BUFFER_OVERFLOW,        SVSTREAM_INVALID_PARAMETER },
        { ERROR_DIRECTORY,              SVSTREAM_INVALID_PARAMETER },
        { ERROR_DRIVE_LOCKED,           SVSTREAM_LOCKING_VIOLATION },
        { ERROR_FILE_NOT_FOUND,         SVSTREAM_FILE_NOT_FOUND },
        { ERROR_FILENAME_EXCED_RANGE,   SVSTREAM_INVALID_PARAMETER },
        { ERROR_INVALID_ACCESS,         SVSTREAM_INVALID_ACCESS },
        { ERROR_INVALID_DRIVE,          SVSTREAM_PATH_NOT_FOUND },
        { ERROR_INVALID_HANDLE,         SVSTREAM_INVALID_HANDLE },
        { ERROR_INVALID_NAME,           SVSTREAM_PATH_NOT_FOUND },
        { ERROR_INVALID_PARAMETER,      SVSTREAM_INVALID_PARAMETER },
        { ERROR_IS_SUBST_PATH,          SVSTREAM_INVALID_PARAMETER },
        { ERROR_IS_SUBST_TARGET,        SVSTREAM_INVALID_PARAMETER },
        { ERROR_LOCK_FAILED,            SVSTREAM_LOCKING_VIOLATION },
        { ERROR_LOCK_VIOLATION,         SVSTREAM_LOCKING_VIOLATION },
        { ERROR_NEGATIVE_SEEK,          SVSTREAM_SEEK_ERROR },
        { ERROR_PATH_NOT_FOUND,         SVSTREAM_PATH_NOT_FOUND },
        { ERROR_READ_FAULT,             SVSTREAM_READ_ERROR },
        { ERROR_SEEK,                   SVSTREAM_SEEK_ERROR },
        { ERROR_SEEK_ON_DEVICE,         SVSTREAM_SEEK_ERROR },
        { ERROR_SHARING_BUFFER_EXCEEDED,SVSTREAM_SHARE_BUFF_EXCEEDED },
        { ERROR_SHARING_PAUSED,         SVSTREAM_SHARING_VIOLATION },
        { ERROR_SHARING_VIOLATION,      SVSTREAM_SHARING_VIOLATION },
        { ERROR_TOO_MANY_OPEN_FILES,    SVSTREAM_TOO_MANY_OPEN_FILES },
        { ERROR_WRITE_FAULT,            SVSTREAM_WRITE_ERROR },
        { ERROR_WRITE_PROTECT,          SVSTREAM_ACCESS_DENIED },
        { ERROR_DISK_FULL,              SVSTREAM_DISK_FULL },

        { (DWORD)0xFFFFFFFF, SVSTREAM_GENERALERROR }
    };

    sal_uIntPtr nRetVal = SVSTREAM_GENERALERROR;    // default error
    int i=0;
    do
    {
        if( errArr[i].wnt == nWntError )
        {
            nRetVal = errArr[i].sv;
            break;
        }
        i++;
    } while( errArr[i].wnt != (DWORD)0xFFFFFFFF );
    return nRetVal;
}

SvFileStream::SvFileStream( const String& rFileName, StreamMode nMode )
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
    Open( aFileName, nMode );
}

SvFileStream::SvFileStream()
{
    bIsOpen             = sal_False;
    nLockCounter        = 0;
    bIsWritable         = sal_False;
    pInstanceData       = new StreamData;

    SetBufferSize( 8192 );
}

SvFileStream::~SvFileStream()
{
    Close();
    if (pInstanceData)
        delete pInstanceData;
}

sal_uIntPtr SvFileStream::GetFileHandle() const
{
    return (sal_uIntPtr)pInstanceData->hFile;
}

sal_uInt16 SvFileStream::IsA() const
{
    return ID_FILESTREAM;
}

/// Does not check for EOF, makes isEof callable
sal_uIntPtr SvFileStream::GetData( void* pData, sal_uIntPtr nSize )
{
    DWORD nCount = 0;
    if( IsOpen() )
    {
        bool bResult = ReadFile(pInstanceData->hFile,(LPVOID)pData,nSize,&nCount,NULL);
        if( !bResult )
        {
            sal_uIntPtr nTestError = GetLastError();
            SetError(::GetSvError( nTestError ) );
        }
    }
    return (DWORD)nCount;
}

sal_uIntPtr SvFileStream::PutData( const void* pData, sal_uIntPtr nSize )
{
    DWORD nCount = 0;
    if( IsOpen() )
    {
        if(!WriteFile(pInstanceData->hFile,(LPVOID)pData,nSize,&nCount,NULL))
            SetError(::GetSvError( GetLastError() ) );
    }
    return nCount;
}

sal_uIntPtr SvFileStream::SeekPos( sal_uIntPtr nPos )
{
    DWORD nNewPos = 0;
    if( IsOpen() )
    {
        if( nPos != STREAM_SEEK_TO_END )
            // 64-Bit files are not supported
            nNewPos=SetFilePointer(pInstanceData->hFile,nPos,NULL,FILE_BEGIN);
        else
            nNewPos=SetFilePointer(pInstanceData->hFile,0L,NULL,FILE_END);

        if( nNewPos == 0xFFFFFFFF )
        {
            SetError(::GetSvError( GetLastError() ) );
            nNewPos = 0L;
        }
    }
    else
        SetError( SVSTREAM_GENERALERROR );
    return (sal_uIntPtr)nNewPos;
}

void SvFileStream::FlushData()
{
    if( IsOpen() )
    {
        if( !FlushFileBuffers(pInstanceData->hFile) )
            SetError(::GetSvError(GetLastError()));
    }
}

sal_Bool SvFileStream::LockRange( sal_uIntPtr nByteOffset, sal_uIntPtr nBytes )
{
    bool bRetVal = false;
    if( IsOpen() )
    {
        bRetVal = ::LockFile(pInstanceData->hFile,nByteOffset,0L,nBytes,0L );
        if( !bRetVal )
            SetError(::GetSvError(GetLastError()));
    }
    return bRetVal;
}

sal_Bool SvFileStream::UnlockRange( sal_uIntPtr nByteOffset, sal_uIntPtr nBytes )
{
    bool bRetVal = false;
    if( IsOpen() )
    {
        bRetVal = ::UnlockFile(pInstanceData->hFile,nByteOffset,0L,nBytes,0L );
        if( !bRetVal )
            SetError(::GetSvError(GetLastError()));
    }
    return bRetVal;
}

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

/*
      NOCREATE       TRUNC   NT-Action
      ----------------------------------------------
         0 (Create)    0     OPEN_ALWAYS
         0 (Create)    1     CREATE_ALWAYS
         1             0     OPEN_EXISTING
         1             1     TRUNCATE_EXISTING
*/
void SvFileStream::Open( const String& rFilename, StreamMode nMode )
{
    String aParsedFilename(rFilename);

    SetLastError( ERROR_SUCCESS );
    Close();
    SvStream::ClearBuffer();

    eStreamMode = nMode;
    eStreamMode &= ~STREAM_TRUNC; // don't truncate on reopen

    //    !!! NoOp: Use ToAbs() otherwise
    //    !!! DirEntry aDirEntry( rFilename );
    //    !!! aFilename = aDirEntry.GetFull();
    aFilename = aParsedFilename;
    rtl::OString aFileNameA(rtl::OUStringToOString(aFilename, osl_getThreadTextEncoding()));
    SetLastError( ERROR_SUCCESS );  // might be changed by Redirector

    DWORD   nOpenAction;
    DWORD   nShareMode      = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD   nAccessMode     = 0L;
    UINT    nOldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX );

    if( nMode & STREAM_SHARE_DENYREAD)
        nShareMode &= ~FILE_SHARE_READ;

    if( nMode & STREAM_SHARE_DENYWRITE)
        nShareMode &= ~FILE_SHARE_WRITE;

    if( nMode & STREAM_SHARE_DENYALL)
        nShareMode = 0;

    if( (nMode & STREAM_READ) )
        nAccessMode |= GENERIC_READ;
    if( (nMode & STREAM_WRITE) )
        nAccessMode |= GENERIC_WRITE;

    if( nAccessMode == GENERIC_READ )       // ReadOnly ?
        nMode |= STREAM_NOCREATE;   // Don't create if readonly

    // Assignment based on true/false table above
    if( !(nMode & STREAM_NOCREATE) )
    {
        if( nMode & STREAM_TRUNC )
            nOpenAction = CREATE_ALWAYS;
        else
            nOpenAction = OPEN_ALWAYS;
    }
    else
    {
        if( nMode & STREAM_TRUNC )
            nOpenAction = TRUNCATE_EXISTING;
        else
            nOpenAction = OPEN_EXISTING;
    }

    pInstanceData->hFile = CreateFile(
        aFileNameA.getStr(),
        nAccessMode,
        nShareMode,
        (LPSECURITY_ATTRIBUTES)NULL,
        nOpenAction,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
        (HANDLE) NULL
    );

    if(  pInstanceData->hFile!=INVALID_HANDLE_VALUE && (
        // Did Create Always overwrite a file?
        GetLastError() == ERROR_ALREADY_EXISTS ||
        // Did Create Always open a new file?
        GetLastError() == ERROR_FILE_NOT_FOUND  ))
    {
        // If so, no error
        if( nOpenAction == OPEN_ALWAYS || nOpenAction == CREATE_ALWAYS )
            SetLastError( ERROR_SUCCESS );
    }

    // Otherwise, determine if we're allowed to read
    if( (pInstanceData->hFile==INVALID_HANDLE_VALUE) &&
         (nAccessMode & GENERIC_WRITE))
    {
        sal_uIntPtr nErr = ::GetSvError( GetLastError() );
        if(nErr==SVSTREAM_ACCESS_DENIED || nErr==SVSTREAM_SHARING_VIOLATION)
        {
            nMode &= (~STREAM_WRITE);
            nAccessMode = GENERIC_READ;
            // OV, 28.1.97: Win32 sets file to length 0
            // if Openaction is CREATE_ALWAYS
            nOpenAction = OPEN_EXISTING;
            SetLastError( ERROR_SUCCESS );
            pInstanceData->hFile = CreateFile(
                aFileNameA.getStr(),
                GENERIC_READ,
                nShareMode,
                (LPSECURITY_ATTRIBUTES)NULL,
                nOpenAction,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
                (HANDLE) NULL
            );
            if( GetLastError() == ERROR_ALREADY_EXISTS )
                SetLastError( ERROR_SUCCESS );
        }
    }

    if( GetLastError() != ERROR_SUCCESS )
    {
        bIsOpen = sal_False;
        SetError(::GetSvError( GetLastError() ) );
    }
    else
    {
        bIsOpen     = sal_True;
        // pInstanceData->bIsEof = sal_False;
        if( nAccessMode & GENERIC_WRITE )
            bIsWritable = sal_True;
    }
    SetErrorMode( nOldErrorMode );
}

void SvFileStream::Close()
{
    if( IsOpen() )
    {
        if( nLockCounter )
        {
            nLockCounter = 1;
            UnlockFile();
        }
        Flush();
        CloseHandle( pInstanceData->hFile );
    }
    bIsOpen     = sal_False;
    nLockCounter= 0;
    bIsWritable = sal_False;
    SvStream::ClearBuffer();
    SvStream::ClearError();
}

/// Reset filepointer to beginning of file
void SvFileStream::ResetError()
{
    SvStream::ClearError();
}

void SvFileStream::SetSize( sal_uIntPtr nSize )
{

    if( IsOpen() )
    {
        int bError = sal_False;
        HANDLE hFile = pInstanceData->hFile;
        sal_uIntPtr nOld = SetFilePointer( hFile, 0L, NULL, FILE_CURRENT );
        if( nOld != 0xffffffff )
        {
            if( SetFilePointer(hFile,nSize,NULL,FILE_BEGIN ) != 0xffffffff)
            {
                bool bSucc = SetEndOfFile( hFile );
                if( !bSucc )
                    bError = sal_True;
            }
            if( SetFilePointer( hFile,nOld,NULL,FILE_BEGIN ) == 0xffffffff)
                bError = sal_True;
        }
        if( bError )
            SetError(::GetSvError( GetLastError() ) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
