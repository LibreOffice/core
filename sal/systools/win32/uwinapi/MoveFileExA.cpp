#define _UWINAPI_
#include "uwinapi.h"

#define WININIT_FILENAME    "wininit.ini"
#define RENAME_SECTION      "rename"

IMPLEMENT_THUNK( kernel32, WINDOWS, BOOL, WINAPI, MoveFileExA, ( LPCSTR lpExistingFileNameA, LPCSTR lpNewFileNameA, DWORD dwFlags ) )
{
    BOOL    fSuccess = FALSE;   // assume failure

    // Windows 9x has a special mechanism to move files after reboot

    if ( dwFlags & MOVEFILE_DELAY_UNTIL_REBOOT )
    {
        CHAR    szExistingFileNameA[MAX_PATH];
        CHAR    szNewFileNameA[MAX_PATH] = "NUL";

        // Path names in WININIT.INI must be in short path name form

        if (
            GetShortPathNameA( lpExistingFileNameA, szExistingFileNameA, MAX_PATH ) &&
            (!lpNewFileNameA || GetShortPathNameA( lpNewFileNameA, szNewFileNameA, MAX_PATH ))
            )
        {
            CHAR    szBuffer[32767];    // The buffer size must not exceed 32K
            DWORD   dwBufLen = GetPrivateProfileSectionA( RENAME_SECTION, szBuffer, bufsizeof(szBuffer), WININIT_FILENAME );

            CHAR    szRename[MAX_PATH]; // This is enough for at most to times 67 chracters
            lstrcpyA( szRename, szNewFileNameA );
            lstrcatA( szRename, "=" );
            lstrcatA( szRename, szExistingFileNameA );
            size_t  lnRename = lstrlenA(szRename);

            if ( dwBufLen + lnRename + 2 <= bufsizeof(szBuffer) )
            {
                CopyMemory( &szBuffer[dwBufLen], szRename, lnRename );
                szBuffer[dwBufLen + lnRename ] = 0;
                szBuffer[dwBufLen + lnRename + 1 ] = 0;

                fSuccess = WritePrivateProfileSectionA( RENAME_SECTION, szBuffer, WININIT_FILENAME );
            }
            else
                SetLastError( ERROR_BUFFER_OVERFLOW );
        }
    }
    else
    {

        fSuccess = MoveFileA( lpExistingFileNameA, lpNewFileNameA );

        if ( !fSuccess && 0 != (dwFlags & (MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING)) )
        {
            BOOL    bFailIfExist = 0 == (dwFlags & MOVEFILE_REPLACE_EXISTING);

            fSuccess = CopyFileA( lpExistingFileNameA, lpNewFileNameA, bFailIfExist );

            // In case of successfull copy do not return FALSE if delete fails.
            // Error detection is done by GetLastError()

            if ( fSuccess )
            {
                SetLastError( NO_ERROR );
                DeleteFileA( lpExistingFileNameA );
            }
        }

    }

    return fSuccess;
}

