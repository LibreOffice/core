#include "macros.h"
#include <tlhelp32.h>

IMPLEMENT_THUNK( psapi, WINDOWS, BOOL, WINAPI, EnumProcesses, ( LPDWORD lpProcesses, DWORD cbSize, LPDWORD lpcbCopied ) )
{
    BOOL    fSuccess = FALSE;
    HANDLE  hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

    if ( IsValidHandle( hSnapshot ) )
    {
        PROCESSENTRY32  pe;

        if ( lpcbCopied )
            *lpcbCopied = 0;

        pe.dwSize = sizeof(pe);
        if ( Process32First( hSnapshot, &pe ) )
        {
            fSuccess = TRUE;

            while ( cbSize >= sizeof(*lpProcesses) )
            {
                *(lpProcesses++) = pe.th32ProcessID;
                if ( lpcbCopied )
                    *lpcbCopied += sizeof(*lpProcesses);
                cbSize -= sizeof(*lpProcesses);

                if ( !Process32Next( hSnapshot, &pe ) )
                    break;
            }
        }

        CloseHandle( hSnapshot );
    }
    else
        SetLastError( ERROR_INVALID_HANDLE );

    return fSuccess;
}


