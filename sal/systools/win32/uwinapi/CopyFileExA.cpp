/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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