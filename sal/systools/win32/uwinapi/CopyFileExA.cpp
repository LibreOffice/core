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
#if defined(_MSC_VER) && (_MSC_VER >= 1400)
#pragma warning(disable:4740)
#endif

#define _WIN32_WINNT 0x0400
#include "macros.h"

#define BUFSIZE 16384

static DWORD CALLBACK DefCopyProgressRoutine(
    LARGE_INTEGER   TotalFileSize,  // total file size, in bytes
    LARGE_INTEGER   TotalBytesTransferred,
                                    // total number of bytes transferred
    LARGE_INTEGER   StreamSize,     // total number of bytes for this stream
    LARGE_INTEGER   StreamBytesTransferred,
                                    // total number of bytes transferred for
                                    // this stream
    DWORD       dwStreamNumber,     // the current stream
    DWORD       dwCallbackReason,   // reason for callback
    HANDLE  hSourceFile,            // handle to the source file
    HANDLE  hDestinationFile,       // handle to the destination file
    LPVOID  lpData                  // passed by CopyFileEx
)
{
    return PROGRESS_CONTINUE;
}


IMPLEMENT_THUNK( kernel32, WINDOWS, BOOL, WINAPI, CopyFileExA, ( LPCSTR lpExistingFileNameA, LPCSTR lpNewFileNameA, LPPROGRESS_ROUTINE  lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags ) )
{
    BOOL    fSuccess = FALSE; // Assume failure

    HANDLE  hSourceFile = CreateFileA(
        lpExistingFileNameA,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
        );

    if ( IsValidHandle(hSourceFile) )
    {
        LARGE_INTEGER   FileSize, BytesTransferred;
        HANDLE  hTargetFile = NULL;

        SetLastError( ERROR_SUCCESS );
        FileSize.LowPart = GetFileSize( hSourceFile, (LPDWORD)&FileSize.HighPart );
        BytesTransferred.QuadPart = 0;

        if ( (DWORD)-1 != FileSize.LowPart || ERROR_SUCCESS == GetLastError() )
            hTargetFile = CreateFileA(
                lpNewFileNameA,
                GENERIC_WRITE,
                0,
                NULL,
                (DWORD) ((dwCopyFlags & COPY_FILE_FAIL_IF_EXISTS) ? CREATE_NEW : CREATE_ALWAYS),
                0,
                NULL
                );

        if ( IsValidHandle(hTargetFile) )
        {
            DWORD dwProgressResult = PROGRESS_CONTINUE;

            fSuccess = SetEndOfFile( hTargetFile );

            if ( fSuccess )
            {
                if ( !lpProgressRoutine )
                    lpProgressRoutine = DefCopyProgressRoutine;

                dwProgressResult = lpProgressRoutine(
                    FileSize,
                    BytesTransferred,
                    FileSize,
                    BytesTransferred,
                    1,
                    CALLBACK_STREAM_SWITCH,
                    hSourceFile,
                    hTargetFile,
                    lpData
                    );

                // Suppress further notifications

                if ( PROGRESS_QUIET == dwProgressResult )
                {
                    lpProgressRoutine = DefCopyProgressRoutine;
                    dwProgressResult = PROGRESS_CONTINUE;
                }
            }

            while ( fSuccess && PROGRESS_CONTINUE == dwProgressResult )
            {
                BYTE    buffer[BUFSIZE];
                DWORD   dwBytesRead, dwBytesWritten = 0;

                fSuccess = ReadFile( hSourceFile, buffer, BUFSIZE, &dwBytesRead, NULL );

                if ( !dwBytesRead ) break;

                if ( fSuccess )
                    fSuccess = WriteFile( hTargetFile, buffer, dwBytesRead, &dwBytesWritten, NULL );

                if ( fSuccess )
                {
                    BytesTransferred.QuadPart += (LONGLONG)dwBytesWritten;

                    if ( pbCancel && *pbCancel )
                        dwProgressResult = PROGRESS_CANCEL;
                    else
                        dwProgressResult = lpProgressRoutine(
                            FileSize,
                            BytesTransferred,
                            FileSize,
                            BytesTransferred,
                            1,
                            CALLBACK_CHUNK_FINISHED,
                            hSourceFile,
                            hTargetFile,
                            lpData
                            );

                }

            }

            CloseHandle( hTargetFile );

            if ( PROGRESS_CANCEL == dwProgressResult )
                DeleteFileA( lpNewFileNameA );
        }


        CloseHandle( hSourceFile );
    }

    return fSuccess;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
