/*************************************************************************
 *
 *  $RCSfile: CopyFileExA.cpp,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hro $ $Date: 2002-08-26 13:43:19 $
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
                (dwCopyFlags & COPY_FILE_FAIL_IF_EXISTS) ? CREATE_NEW : CREATE_ALWAYS,
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
                DWORD   dwBytesRead, dwBytesWritten;

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
