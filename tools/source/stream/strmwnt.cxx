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

#ifdef _WIN32
#if !defined WIN32_LEAN_AND_MEAN
# define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#include <osl/thread.h>
#include <tools/stream.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

// class FileBase
#include <osl/file.hxx>
using namespace osl;

class StreamData
{
public:
    HANDLE      hFile;

                StreamData() : hFile(nullptr)
                {
                }
};

static ErrCode GetSvError( DWORD nWntError )
{
    static struct { DWORD wnt; ErrCode sv; } errArr[] =
    {
        { ERROR_SUCCESS,                ERRCODE_NONE },
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

        { DWORD(0xFFFFFFFF), SVSTREAM_GENERALERROR }
    };

    ErrCode nRetVal = SVSTREAM_GENERALERROR;    // default error
    int i=0;
    do
    {
        if( errArr[i].wnt == nWntError )
        {
            nRetVal = errArr[i].sv;
            break;
        }
        i++;
    } while( errArr[i].wnt != DWORD(0xFFFFFFFF) );
    return nRetVal;
}

SvFileStream::SvFileStream( const OUString& rFileName, StreamMode nMode )
{
    bIsOpen             = false;
    nLockCounter        = 0;
    m_isWritable        = false;
    pInstanceData.reset( new StreamData );

    SetBufferSize( 8192 );
    // convert URL to SystemPath, if necessary
    OUString aFileName;

    if ( FileBase::getSystemPathFromFileURL( rFileName, aFileName ) != FileBase::E_None )
        aFileName = rFileName;
    Open( aFileName, nMode );
}

SvFileStream::SvFileStream()
{
    bIsOpen             = false;
    nLockCounter        = 0;
    m_isWritable        = false;
    pInstanceData.reset( new StreamData );

    SetBufferSize( 8192 );
}

SvFileStream::~SvFileStream()
{
    Close();
}

/// Does not check for EOF, makes isEof callable
std::size_t SvFileStream::GetData( void* pData, std::size_t nSize )
{
    DWORD nCount = 0;
    if( IsOpen() )
    {
        bool bResult = ReadFile(pInstanceData->hFile,pData,nSize,&nCount,nullptr);
        if( !bResult )
        {
            std::size_t nTestError = GetLastError();
            SetError(::GetSvError( nTestError ) );
        }
    }
    return nCount;
}

std::size_t SvFileStream::PutData( const void* pData, std::size_t nSize )
{
    DWORD nCount = 0;
    if( IsOpen() )
    {
        if(!WriteFile(pInstanceData->hFile,pData,nSize,&nCount,nullptr))
            SetError(::GetSvError( GetLastError() ) );
    }
    return nCount;
}

sal_uInt64 SvFileStream::SeekPos(sal_uInt64 const nPos)
{
    // check if a truncated STREAM_SEEK_TO_END was passed
    assert(nPos != SAL_MAX_UINT32);
    DWORD nNewPos = 0;
    if( IsOpen() )
    {
        if( nPos != STREAM_SEEK_TO_END )
            // 64-Bit files are not supported
            nNewPos=SetFilePointer(pInstanceData->hFile,nPos,nullptr,FILE_BEGIN);
        else
            nNewPos=SetFilePointer(pInstanceData->hFile,0L,nullptr,FILE_END);

        if( nNewPos == 0xFFFFFFFF )
        {
            SetError(::GetSvError( GetLastError() ) );
            nNewPos = 0;
        }
    }
    else
        SetError( SVSTREAM_GENERALERROR );
    return static_cast<sal_uInt64>(nNewPos);
}

void SvFileStream::FlushData()
{
    if( IsOpen() )
    {
        if( !FlushFileBuffers(pInstanceData->hFile) )
            SetError(::GetSvError(GetLastError()));
    }
}

bool SvFileStream::LockRange(sal_uInt64 const nByteOffset, std::size_t nBytes)
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

bool SvFileStream::UnlockRange(sal_uInt64 const nByteOffset, std::size_t nBytes)
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

bool SvFileStream::LockFile()
{
    bool bRetVal = false;
    if( !nLockCounter )
    {
        if( LockRange( 0L, LONG_MAX ) )
        {
            nLockCounter = 1;
            bRetVal = true;
        }
    }
    else
    {
        nLockCounter++;
        bRetVal = true;
    }
    return bRetVal;
}

void SvFileStream::UnlockFile()
{
    if( nLockCounter > 0)
    {
        if( nLockCounter == 1)
        {
            if( UnlockRange( 0L, LONG_MAX ) )
            {
                nLockCounter = 0;
            }
        }
        else
        {
            nLockCounter--;
        }
    }
}

/*
      NOCREATE       TRUNC   NT-Action
      ----------------------------------------------
         0 (Create)    0     OPEN_ALWAYS
         0 (Create)    1     CREATE_ALWAYS
         1             0     OPEN_EXISTING
         1             1     TRUNCATE_EXISTING
*/
void SvFileStream::Open( const OUString& rFilename, StreamMode nMode )
{
    OUString aParsedFilename(rFilename);

    SetLastError( ERROR_SUCCESS );
    Close();
    SvStream::ClearBuffer();

    m_eStreamMode = nMode;
    m_eStreamMode &= ~StreamMode::TRUNC; // don't truncate on reopen

    aFilename = aParsedFilename;
    SetLastError( ERROR_SUCCESS );  // might be changed by Redirector

    DWORD   nOpenAction;
    DWORD   nShareMode      = FILE_SHARE_READ | FILE_SHARE_WRITE;
    DWORD   nAccessMode     = 0;
    UINT    nOldErrorMode = SetErrorMode( SEM_FAILCRITICALERRORS|SEM_NOOPENFILEERRORBOX );

    if( nMode & StreamMode::SHARE_DENYREAD)
        nShareMode &= ~FILE_SHARE_READ;

    if( nMode & StreamMode::SHARE_DENYWRITE)
        nShareMode &= ~FILE_SHARE_WRITE;

    if( nMode & StreamMode::SHARE_DENYALL)
        nShareMode = 0;

    if( nMode & StreamMode::READ )
        nAccessMode |= GENERIC_READ;
    if( nMode & StreamMode::WRITE )
        nAccessMode |= GENERIC_WRITE;

    if( nAccessMode == GENERIC_READ )       // ReadOnly ?
        nMode |= StreamMode::NOCREATE;   // Don't create if readonly

    // Assignment based on true/false table above
    if( !(nMode & StreamMode::NOCREATE) )
    {
        if( nMode & StreamMode::TRUNC )
            nOpenAction = CREATE_ALWAYS;
        else
            nOpenAction = OPEN_ALWAYS;
    }
    else
    {
        if( nMode & StreamMode::TRUNC )
            nOpenAction = TRUNCATE_EXISTING;
        else
            nOpenAction = OPEN_EXISTING;
    }

    pInstanceData->hFile = CreateFileW(
        o3tl::toW(aFilename.getStr()),
        nAccessMode,
        nShareMode,
        nullptr,
        nOpenAction,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
        nullptr
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
        ErrCode nErr = ::GetSvError( GetLastError() );
        if(nErr==SVSTREAM_ACCESS_DENIED || nErr==SVSTREAM_SHARING_VIOLATION)
        {
            nMode &= ~StreamMode::WRITE;
            nAccessMode = GENERIC_READ;
            // OV, 28.1.97: Win32 sets file to length 0
            // if Openaction is CREATE_ALWAYS
            nOpenAction = OPEN_EXISTING;
            SetLastError( ERROR_SUCCESS );
            pInstanceData->hFile = CreateFileW(
                o3tl::toW(aFilename.getStr()),
                GENERIC_READ,
                nShareMode,
                nullptr,
                nOpenAction,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
                nullptr
            );
            if( GetLastError() == ERROR_ALREADY_EXISTS )
                SetLastError( ERROR_SUCCESS );
        }
    }

    if( GetLastError() != ERROR_SUCCESS )
    {
        bIsOpen = false;
        SetError(::GetSvError( GetLastError() ) );
    }
    else
    {
        bIsOpen     = true;
        // pInstanceData->bIsEof = false;
        if( nAccessMode & GENERIC_WRITE )
            m_isWritable = true;
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
    bIsOpen     = false;
    nLockCounter= 0;
    m_isWritable = false;
    SvStream::ClearBuffer();
    SvStream::ClearError();
}

/// Reset filepointer to beginning of file
void SvFileStream::ResetError()
{
    SvStream::ClearError();
}

void SvFileStream::SetSize(sal_uInt64 const nSize)
{

    if( IsOpen() )
    {
        bool bError = false;
        HANDLE hFile = pInstanceData->hFile;
        DWORD const nOld = SetFilePointer( hFile, 0L, nullptr, FILE_CURRENT );
        if( nOld != 0xffffffff )
        {
            if( SetFilePointer(hFile,nSize,nullptr,FILE_BEGIN ) != 0xffffffff)
            {
                bool bSucc = SetEndOfFile( hFile );
                if( !bSucc )
                    bError = true;
            }
            if( SetFilePointer( hFile,nOld,nullptr,FILE_BEGIN ) == 0xffffffff)
                bError = true;
        }
        if( bError )
            SetError(::GetSvError( GetLastError() ) );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
