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
#include "precompiled_sal.hxx"
#   include "pipeimpl.h"

#ifndef _INC_MALLOC
#   include <malloc.h>
#endif

#ifndef _INC_TCHAR
#   ifdef UNICODE
#       define _UNICODE
#   endif
#   include <tchar.h>
#endif

const TCHAR PIPE_NAME_PREFIX_MAPPING[] = TEXT("PIPE_FILE_MAPPING_");
const TCHAR PIPE_NAME_PREFIX_SYNCHRONIZE[] = TEXT("PIPE_SYNCHRONIZE_MUTEX_");
const TCHAR PIPE_NAME_PREFIX_CONNECTION[] = TEXT("PIPE_CONNECTION_SEMAPHORE_");

const DWORD PIPE_BUFFER_SIZE = 4096;


//============================================================================
//  PipeData
//============================================================================

struct PipeData
{
    DWORD   dwProcessId;
    HANDLE  hReadPipe;
    HANDLE  hWritePipe;
};

//============================================================================
//  Pipe
//============================================================================

#ifdef UNICODE
#define Pipe        PipeW
#define ClientPipe  ClientPipeW
#define ServerPipe  ServerPipeW
#else
#define Pipe        PipeA
#define ClientPipe  ClientPipeA
#define ServerPipe  ServerPipeA
#endif

class Pipe
{
protected:
    HANDLE  m_hReadPipe;    // Handle to use for reading
    HANDLE  m_hWritePipe;   // Handle to use for writing

    Pipe( HANDLE hReadPipe, HANDLE hWritePipe );

    static HANDLE CreatePipeDataMutex( LPCTSTR lpName, BOOL bInitialOwner );
    static HANDLE CreatePipeDataMapping( LPCTSTR lpName );
    static HANDLE OpenPipeDataMapping( LPCTSTR lpName );
    static HANDLE CreatePipeConnectionSemaphore( LPCTSTR lpName, LONG lInitialCount, LONG lMaximumcount );

public:
    Pipe( const Pipe& );
    const Pipe& operator = ( const Pipe& );
    virtual ~Pipe();

    virtual bool Close();
    virtual bool Write( LPCVOID lpBuffer, DWORD dwBytesToWrite, LPDWORD lpBytesWritten, bool bWait = true );
    virtual bool Read( LPVOID lpBuffer, DWORD dwBytesToRead, LPDWORD lpBytesRead, bool bWait = true );

    virtual Pipe *AcceptConnection()
    {
        SetLastError( ERROR_INVALID_HANDLE );
        return NULL;
    }

    void * operator new( size_t nBytes )
    {
        return HeapAlloc( GetProcessHeap(), 0, nBytes );
    }

    void operator delete( void *ptr )
    {
        HeapFree( GetProcessHeap(), 0, ptr );
    }

    bool is() const
    {
        return (FALSE != HeapValidate( GetProcessHeap(), 0, this ));
    }

};

//============================================================================
//  ClientPipe
//============================================================================

class ClientPipe : public Pipe
{
protected:
    ClientPipe( HANDLE hReadPipe, HANDLE hWritePipe );
public:
    static ClientPipe* Create( LPCTSTR lpName );
};

//============================================================================
//  ServerPipe
//============================================================================

class ServerPipe : public Pipe
{
protected:
    HANDLE  m_hMapping;
    HANDLE  m_hSynchronize;
    LPTSTR  m_lpName;

    ServerPipe( LPCTSTR lpName, HANDLE hMapping, HANDLE hSynchronize, HANDLE hReadPipe, HANDLE hWritePipe );
public:
    virtual ~ServerPipe();

    static ServerPipe *Create( LPCTSTR lpName );

    virtual Pipe *AcceptConnection();
};

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HANDLE  Pipe::CreatePipeDataMapping( LPCTSTR lpName )
{
    HANDLE  hMapping = NULL;
    LPTSTR  lpMappingName = (LPTSTR)alloca( _tcslen(lpName) * sizeof(TCHAR) + sizeof(PIPE_NAME_PREFIX_MAPPING) );

    if ( lpMappingName )
    {
        _tcscpy( lpMappingName, PIPE_NAME_PREFIX_MAPPING );
        _tcscat( lpMappingName, lpName );

        LPTSTR  lpMappingFileName = (LPTSTR)alloca( MAX_PATH * sizeof(TCHAR) );

        if ( lpMappingFileName )
        {
            DWORD   nChars = GetTempPath( MAX_PATH, lpMappingFileName );

            if ( MAX_PATH + _tcslen(lpName) < nChars + 1 )
            {
                lpMappingFileName = (LPTSTR)alloca( (nChars + 1 + _tcslen(lpName)) * sizeof(TCHAR) );
                if ( lpMappingFileName )
                    nChars = GetTempPath( nChars, lpMappingFileName );
                else
                {
                    nChars = 0;
                    SetLastError( ERROR_NOT_ENOUGH_MEMORY );
                }
            }

            if ( nChars )
            {
                _tcscat( lpMappingFileName, lpMappingName );

                HANDLE hFile = CreateFile(
                    lpMappingFileName,
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    OPEN_ALWAYS,
                    FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
                    NULL );

                if ( IsValidHandle(hFile) )
                {
                    hMapping = CreateFileMapping(
                        (HANDLE)hFile,
                        (LPSECURITY_ATTRIBUTES)NULL,
                        PAGE_READWRITE,
                        0,
                        sizeof(PipeData),
                        lpMappingName );

                    CloseHandle( hFile );
                }
            }
        }
        else
            SetLastError( ERROR_NOT_ENOUGH_MEMORY );
    }

    return hMapping;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HANDLE  Pipe::OpenPipeDataMapping( LPCTSTR lpName )
{
    HANDLE  hMapping = NULL;
    LPTSTR  lpMappingName = (LPTSTR)alloca( _tcslen(lpName) * sizeof(TCHAR) + sizeof(PIPE_NAME_PREFIX_MAPPING) );

    if ( lpMappingName )
    {
        _tcscpy( lpMappingName, PIPE_NAME_PREFIX_MAPPING );
        _tcscat( lpMappingName, lpName );

        hMapping = OpenFileMapping( FILE_MAP_ALL_ACCESS, FALSE, lpMappingName );
    }

    return hMapping;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HANDLE  Pipe::CreatePipeDataMutex( LPCTSTR lpName, BOOL bInitialOwner )
{
    HANDLE  hMutex = NULL;
    LPTSTR  lpMutexName = (LPTSTR)alloca( _tcslen(lpName) * sizeof(TCHAR) + sizeof(PIPE_NAME_PREFIX_SYNCHRONIZE) );

    if ( lpMutexName )
    {
        _tcscpy( lpMutexName, PIPE_NAME_PREFIX_SYNCHRONIZE );
        _tcscat( lpMutexName, lpName );

        hMutex = CreateMutex( NULL, bInitialOwner, lpMutexName );
    }

    return hMutex;
}

//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------

HANDLE Pipe::CreatePipeConnectionSemaphore( LPCTSTR lpName, LONG lInitialCount, LONG lMaximumCount )
{
    HANDLE  hSemaphore = NULL;
    LPTSTR  lpSemaphoreName = (LPTSTR)alloca( _tcslen(lpName) * sizeof(TCHAR) + sizeof(PIPE_NAME_PREFIX_CONNECTION) );

    if ( lpSemaphoreName )
    {
        _tcscpy( lpSemaphoreName, PIPE_NAME_PREFIX_CONNECTION );
        _tcscat( lpSemaphoreName, lpName );

        hSemaphore = CreateSemaphore( NULL, lInitialCount, lMaximumCount, lpSemaphoreName );
    }

    return hSemaphore;
}


//----------------------------------------------------------------------------
//  Pipe copy ctor
//----------------------------------------------------------------------------

Pipe::Pipe( const Pipe& rPipe ) :
m_hReadPipe( INVALID_HANDLE_VALUE ),
m_hWritePipe( INVALID_HANDLE_VALUE )
{
    DuplicateHandle(
        GetCurrentProcess(),
        rPipe.m_hReadPipe,
        GetCurrentProcess(),
        &m_hReadPipe,
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS );

    DuplicateHandle(
        GetCurrentProcess(),
        rPipe.m_hWritePipe,
        GetCurrentProcess(),
        &m_hWritePipe,
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS );
}

//----------------------------------------------------------------------------
//  Pipe assignment operator
//----------------------------------------------------------------------------

const Pipe& Pipe::operator = ( const Pipe& rPipe )
{
    Close();

    DuplicateHandle(
        GetCurrentProcess(),
        rPipe.m_hReadPipe,
        GetCurrentProcess(),
        &m_hReadPipe,
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS );

    DuplicateHandle(
        GetCurrentProcess(),
        rPipe.m_hWritePipe,
        GetCurrentProcess(),
        &m_hWritePipe,
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS );

    return *this;
}

//----------------------------------------------------------------------------
//  Pipe ctor
//----------------------------------------------------------------------------

Pipe::Pipe( HANDLE hReadPipe, HANDLE hWritePipe ) :
m_hReadPipe( INVALID_HANDLE_VALUE ),
m_hWritePipe( INVALID_HANDLE_VALUE )
{
    DuplicateHandle(
        GetCurrentProcess(),
        hReadPipe,
        GetCurrentProcess(),
        &m_hReadPipe,
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS );

    DuplicateHandle(
        GetCurrentProcess(),
        hWritePipe,
        GetCurrentProcess(),
        &m_hWritePipe,
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS );
}

//----------------------------------------------------------------------------
//  Pipe dtor
//----------------------------------------------------------------------------

Pipe::~Pipe()
{
    Close();
}

//----------------------------------------------------------------------------
//  Pipe Close
//----------------------------------------------------------------------------

bool Pipe::Close()
{
    bool    fSuccess = false;   // Assume failure

    if ( IsValidHandle(m_hReadPipe) )
    {
        CloseHandle( m_hReadPipe );
        m_hReadPipe = INVALID_HANDLE_VALUE;
    }

    if ( IsValidHandle(m_hWritePipe) )
    {
        CloseHandle( m_hWritePipe );
        m_hWritePipe = INVALID_HANDLE_VALUE;
    }

    return fSuccess;
}

//----------------------------------------------------------------------------
//  Pipe Write
//----------------------------------------------------------------------------

bool Pipe::Write( LPCVOID lpBuffer, DWORD dwBytesToWrite, LPDWORD lpBytesWritten, bool bWait )
{
    DWORD   dwBytesAvailable = 0;
    BOOL    fSuccess = TRUE;

    if ( !bWait )
        fSuccess = PeekNamedPipe( m_hReadPipe, NULL, 0, NULL, &dwBytesAvailable, NULL );

    if ( fSuccess )
    {
        if ( !bWait && dwBytesToWrite > PIPE_BUFFER_SIZE - dwBytesAvailable )
            dwBytesToWrite = PIPE_BUFFER_SIZE - dwBytesAvailable ;

        return !!WriteFile( m_hWritePipe, lpBuffer, dwBytesToWrite, lpBytesWritten, NULL );
    }

    return false;
}

//----------------------------------------------------------------------------
//  Pipe Read
//----------------------------------------------------------------------------

bool Pipe::Read( LPVOID lpBuffer, DWORD dwBytesToRead, LPDWORD lpBytesRead, bool bWait )
{
    DWORD   dwBytesAvailable = 0;
    BOOL    fSuccess = TRUE;

    if ( !bWait )
        fSuccess = PeekNamedPipe( m_hReadPipe, NULL, 0, NULL, &dwBytesAvailable, NULL );

    if ( fSuccess )
    {
        if ( bWait || dwBytesAvailable )
            return !!ReadFile( m_hReadPipe, lpBuffer, dwBytesToRead, lpBytesRead, NULL );
        else
        {
            *lpBytesRead = 0;
            return true;
        }
    }

    return false;
}



//----------------------------------------------------------------------------
//  Client pipe dtor
//----------------------------------------------------------------------------

ClientPipe::ClientPipe( HANDLE hReadPipe, HANDLE hWritePipe ) : Pipe( hReadPipe, hWritePipe )
{
}

//----------------------------------------------------------------------------
//  Client pipe creation
//----------------------------------------------------------------------------

ClientPipe *ClientPipe::Create( LPCTSTR lpName )
{
    ClientPipe  *pPipe = NULL;  // Assume failure

    HANDLE  hMapping = OpenPipeDataMapping( lpName );

    if ( IsValidHandle(hMapping) )
    {
        PipeData    *pData = (PipeData*)MapViewOfFile( hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0 );

        if ( pData )
        {
            HANDLE  hSourceProcess = OpenProcess( PROCESS_DUP_HANDLE, FALSE, pData->dwProcessId );

            if ( IsValidHandle(hSourceProcess) )
            {
                BOOL fSuccess;
                HANDLE  hReadPipe = INVALID_HANDLE_VALUE, hWritePipe = INVALID_HANDLE_VALUE;

                fSuccess = DuplicateHandle(
                    hSourceProcess,
                    pData->hReadPipe,
                    GetCurrentProcess(),
                    &hReadPipe,
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS );

                fSuccess = fSuccess && DuplicateHandle(
                    hSourceProcess,
                    pData->hWritePipe,
                    GetCurrentProcess(),
                    &hWritePipe,
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS );

                if ( fSuccess )
                    pPipe = new ClientPipe( hReadPipe, hWritePipe );

                if ( IsValidHandle(hWritePipe) )
                    CloseHandle( hWritePipe );

                if ( IsValidHandle(hReadPipe) )
                    CloseHandle( hReadPipe );

                HANDLE  hConnectionRequest = CreatePipeConnectionSemaphore( lpName, 0, 1 );

                ReleaseSemaphore( hConnectionRequest, 1, NULL );

                CloseHandle( hConnectionRequest );

                CloseHandle( hSourceProcess );
            }

            UnmapViewOfFile( pData );
        }

        CloseHandle( hMapping );
    }

    return pPipe;
}



//----------------------------------------------------------------------------
//  ServerPipe ctor
//----------------------------------------------------------------------------

ServerPipe::ServerPipe( LPCTSTR lpName, HANDLE hMapping, HANDLE hSynchronize, HANDLE hReadPipe, HANDLE hWritePipe ) : Pipe( hReadPipe, hWritePipe ),
m_hMapping( NULL ),
m_hSynchronize( NULL ),
m_lpName( NULL )
{
    DuplicateHandle(
        GetCurrentProcess(),
        hMapping,
        GetCurrentProcess(),
        &m_hMapping,
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS );

    DuplicateHandle(
        GetCurrentProcess(),
        hSynchronize,
        GetCurrentProcess(),
        &m_hSynchronize,
        0,
        FALSE,
        DUPLICATE_SAME_ACCESS
        );
    m_lpName = new TCHAR[_tcslen(lpName) + 1];
    if ( m_lpName )
        _tcscpy( m_lpName, lpName );
}

//----------------------------------------------------------------------------
//  ServerPipe dtor
//----------------------------------------------------------------------------

ServerPipe::~ServerPipe()
{
    if ( IsValidHandle(m_hMapping) )
        CloseHandle( m_hMapping );
    if ( m_lpName )
        delete[]m_lpName;
}

//----------------------------------------------------------------------------
//  ServerPipe AcceptConnection
//----------------------------------------------------------------------------

Pipe *ServerPipe::AcceptConnection()
{
    Pipe    *pPipe = NULL;  // Assume failure;

    HANDLE  hConnectionRequest = CreatePipeConnectionSemaphore( m_lpName, 0, 1 );

    if ( WAIT_OBJECT_0 == WaitForSingleObject( hConnectionRequest, INFINITE ) )
    {
        pPipe = new Pipe( *this );
        Close();

        // Create new inbound Pipe

        HANDLE  hClientWritePipe = NULL, hServerReadPipe = NULL;

        BOOL    fSuccess = CreatePipe( &hServerReadPipe, &hClientWritePipe, NULL, PIPE_BUFFER_SIZE );


        if ( fSuccess )
        {
            // Create outbound pipe

            HANDLE  hClientReadPipe = NULL, hServerWritePipe = NULL;

            if ( CreatePipe( &hClientReadPipe, &hServerWritePipe, NULL, PIPE_BUFFER_SIZE ) )
            {
                m_hReadPipe = hServerReadPipe;
                m_hWritePipe = hServerWritePipe;

                PipeData    *pData = (PipeData *)MapViewOfFile( m_hMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(PipeData) );

                HANDLE  hSynchronize = CreatePipeDataMutex( m_lpName, TRUE );

                CloseHandle( pData->hReadPipe );
                CloseHandle( pData->hWritePipe );

                pData->hReadPipe = hClientReadPipe;
                pData->hWritePipe = hClientWritePipe;

                ReleaseMutex( hSynchronize );

                CloseHandle( hSynchronize );

            }
            else
            {
                CloseHandle( hClientWritePipe );
                CloseHandle( hServerWritePipe );
            }
        }

        ReleaseMutex( hConnectionRequest );
    }

    CloseHandle( hConnectionRequest );

    return pPipe;
}

//----------------------------------------------------------------------------
//  Pipe creation
//----------------------------------------------------------------------------

ServerPipe *ServerPipe::Create( LPCTSTR lpName )
{
    ServerPipe  *pPipe = NULL;

    HANDLE  hMapping = CreatePipeDataMapping( lpName );

    if ( IsValidHandle(hMapping) )
    {
        if ( ERROR_FILE_EXISTS != GetLastError() )
        {
            HANDLE  hSynchronize = CreatePipeDataMutex( lpName, FALSE);

            WaitForSingleObject( hSynchronize, INFINITE );

            PipeData    *pData = (PipeData*)MapViewOfFile( hMapping, FILE_MAP_ALL_ACCESS, 0, 0, 0 );

            if ( pData )
            {

                // Initialize pipe data

                pData->dwProcessId = 0;
                pData->hReadPipe = NULL;
                pData->hWritePipe = NULL;

                // Create inbound pipe

                HANDLE  hServerReadPipe = NULL, hClientWritePipe = NULL;

                BOOL    fSuccess = CreatePipe( &hServerReadPipe, &hClientWritePipe, NULL, PIPE_BUFFER_SIZE );

                if ( fSuccess )
                {
                    // Create outbound pipe

                    HANDLE  hServerWritePipe = NULL, hClientReadPipe = NULL;

                    fSuccess = CreatePipe( &hClientReadPipe, &hServerWritePipe, NULL, PIPE_BUFFER_SIZE );

                    if ( fSuccess )
                    {
                        pData->dwProcessId = GetCurrentProcessId();
                        pData->hReadPipe = hClientReadPipe;
                        pData->hWritePipe = hClientWritePipe;
                        pPipe = new ServerPipe( lpName, hMapping, hSynchronize, hServerReadPipe, hServerWritePipe );

                        CloseHandle( hServerWritePipe );
                        CloseHandle( hServerReadPipe );
                    }
                    else
                    {
                        CloseHandle( hServerReadPipe );
                        CloseHandle( hClientWritePipe );
                    }
                }

                UnmapViewOfFile( pData );
            }

            ReleaseMutex( hSynchronize );
            CloseHandle( hSynchronize );
        }

        CloseHandle( hMapping );
    }

    return pPipe;
}


//----------------------------------------------------------------------------
//  C style API
//----------------------------------------------------------------------------

const TCHAR LOCAL_PIPE_PREFIX[] = TEXT("\\\\.\\PIPE\\" );

extern "C" HANDLE WINAPI CreateSimplePipe( LPCTSTR lpName )
{
    int nPrefixLen = _tcslen( LOCAL_PIPE_PREFIX );
    if ( 0 == _tcsnicmp( lpName, LOCAL_PIPE_PREFIX, nPrefixLen ) )
        lpName += nPrefixLen;
    return (HANDLE)ServerPipe::Create( lpName );
}

extern "C" HANDLE WINAPI OpenSimplePipe( LPCTSTR lpName )
{
    int nPrefixLen = _tcslen( LOCAL_PIPE_PREFIX );
    if ( 0 == _tcsnicmp( lpName, LOCAL_PIPE_PREFIX, nPrefixLen ) )
        lpName += nPrefixLen;
    return (HANDLE)ClientPipe::Create( lpName );
}

extern "C" HANDLE WINAPI AcceptSimplePipeConnection( HANDLE hPipe )
{
    Pipe    *pPipe = (Pipe *)hPipe;

    if ( pPipe->is() )
        return (HANDLE)pPipe->AcceptConnection();
    else
    {
        SetLastError( ERROR_INVALID_HANDLE );
        return NULL;
    }
}

extern "C" BOOL WINAPI WaitForSimplePipe( LPCTSTR /*lpName*/, DWORD /*dwTimeOut*/ )
{
    return FALSE;
}

extern "C" BOOL WINAPI WriteSimplePipe( HANDLE hPipe, LPCVOID lpBuffer, DWORD dwBytesToWrite, LPDWORD lpBytesWritten, BOOL bWait )
{
    Pipe    *pPipe = (Pipe *)hPipe;

    if ( pPipe->is() )
        return pPipe->Write( lpBuffer, dwBytesToWrite, lpBytesWritten, bWait );
    else
    {
        SetLastError( ERROR_INVALID_HANDLE );
        return FALSE;
    }
}

extern "C" BOOL WINAPI ReadSimplePipe( HANDLE hPipe, LPVOID lpBuffer, DWORD dwBytesToRead, LPDWORD lpBytesRead, BOOL bWait )
{
    Pipe    *pPipe = (Pipe *)hPipe;

    if ( pPipe->is() )
        return pPipe->Read( lpBuffer, dwBytesToRead, lpBytesRead, bWait );
    else
    {
        SetLastError( ERROR_INVALID_HANDLE );
        return FALSE;
    }
}

extern "C" BOOL WINAPI CloseSimplePipe( HANDLE hPipe )
{
    Pipe    *pPipe = (Pipe *)hPipe;

    if ( pPipe->is() )
    {
        delete pPipe;
        return TRUE;
    }
    else
    {
        SetLastError( ERROR_INVALID_HANDLE );
        return FALSE;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
