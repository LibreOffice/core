#include "quickstarter.hxx"
#include <psapi.h>
#include <tlhelp32.h>
#include <malloc.h>

std::string GetOfficeInstallationPath(MSIHANDLE handle)
{
    std::string progpath;
    DWORD sz = 0;
    LPTSTR dummy = TEXT("");

    if (MsiGetProperty(handle, TEXT("OfficeFolder"), dummy, &sz) == ERROR_MORE_DATA)
    {
        sz++; // space for the final '\0'
        DWORD nbytes = sz * sizeof(TCHAR);
        LPTSTR buff = reinterpret_cast<LPTSTR>(_alloca(nbytes));
        ZeroMemory(buff, nbytes);
        MsiGetProperty(handle, TEXT("OfficeFolder"), buff, &sz);
        progpath = buff;
    }
    return progpath;
}

std::string GetOfficeProductName(MSIHANDLE handle)
{
    std::string productname;
    DWORD sz = 0;
    LPTSTR dummy = TEXT("");

    if (MsiGetProperty(handle, TEXT("ProductName"), dummy, &sz) == ERROR_MORE_DATA)
    {
        sz++; // space for the final '\0'
        DWORD nbytes = sz * sizeof(TCHAR);
        LPTSTR buff = reinterpret_cast<LPTSTR>(_alloca(nbytes));
        ZeroMemory(buff, nbytes);
        MsiGetProperty(handle, TEXT("ProductName"), buff, &sz);
        productname = buff;
    }
    return productname;
}

inline bool IsValidHandle( HANDLE handle )
{
    return NULL != handle && INVALID_HANDLE_VALUE != handle;
}


static HANDLE WINAPI _CreateToolhelp32Snapshot( DWORD dwFlags, DWORD th32ProcessID )
{
    typedef HANDLE (WINAPI *FN_PROC)( DWORD dwFlags, DWORD th32ProcessID );
    static FN_PROC  lpProc = NULL;

    HANDLE  hSnapshot = NULL;

    if ( !lpProc )
    {
        HMODULE hLibrary = GetModuleHandle("KERNEL32.DLL");

        if ( hLibrary )
            lpProc = reinterpret_cast< FN_PROC >(GetProcAddress( hLibrary, "CreateToolhelp32Snapshot" ));
    }

    if ( lpProc )
        hSnapshot = lpProc( dwFlags, th32ProcessID );

    return hSnapshot;
}

static BOOL WINAPI _Process32First( HANDLE hSnapshot, PROCESSENTRY32 *lppe32 )
{
    typedef BOOL (WINAPI *FN_PROC)( HANDLE hSnapshot, PROCESSENTRY32 *lppe32 );
    static FN_PROC  lpProc = NULL;

    BOOL    fSuccess = FALSE;

    if ( !lpProc )
    {
        HMODULE hLibrary = GetModuleHandle("KERNEL32.DLL");

        if ( hLibrary )
            lpProc = reinterpret_cast< FN_PROC >(GetProcAddress( hLibrary, "Process32First" ));
    }

    if ( lpProc )
        fSuccess = lpProc( hSnapshot, lppe32 );

    return fSuccess;
}

static BOOL WINAPI _Process32Next( HANDLE hSnapshot, PROCESSENTRY32 *lppe32 )
{
    typedef BOOL (WINAPI *FN_PROC)( HANDLE hSnapshot, PROCESSENTRY32 *lppe32 );
    static FN_PROC  lpProc = NULL;

    BOOL    fSuccess = FALSE;

    if ( !lpProc )
    {
        HMODULE hLibrary = GetModuleHandle("KERNEL32.DLL");

        if ( hLibrary )
            lpProc = reinterpret_cast< FN_PROC >(GetProcAddress( hLibrary, "Process32Next" ));
    }

    if ( lpProc )
        fSuccess = lpProc( hSnapshot, lppe32 );

    return fSuccess;
}

static std::string GetProcessImagePath_9x( DWORD dwProcessId )
{
    std::string sImagePath;

    HANDLE  hSnapshot = _CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );

    if ( IsValidHandle( hSnapshot ) )
    {
        PROCESSENTRY32  pe32 = { 0 };

        pe32.dwSize = sizeof(PROCESSENTRY32);

        BOOL    fSuccess = _Process32First( hSnapshot, &pe32 );
        bool    found = false;

        while ( !found && fSuccess )
        {
            if ( pe32.th32ProcessID == dwProcessId )
            {
                found = true;
                sImagePath = pe32.szExeFile;
            }

            if ( !found )
                fSuccess = _Process32Next( hSnapshot, &pe32 );
        }

        CloseHandle( hSnapshot );
    }

    return sImagePath;
}

static DWORD WINAPI _GetModuleFileNameExA( HANDLE hProcess, HMODULE hModule, LPSTR lpFileName, DWORD nSize )
{
    typedef DWORD (WINAPI *FN_PROC)( HANDLE hProcess, HMODULE hModule, LPSTR lpFileName, DWORD nSize );

    static FN_PROC  lpProc = NULL;

    if ( !lpProc )
    {
        HMODULE hLibrary = LoadLibrary("PSAPI.DLL");

        if ( hLibrary )
            lpProc = reinterpret_cast< FN_PROC >(GetProcAddress( hLibrary, "GetModuleFileNameExA" ));
    }

    if ( lpProc )
        return lpProc( hProcess, hModule, lpFileName, nSize );

    return 0;

}

static std::string GetProcessImagePath_NT( DWORD dwProcessId )
{
    std::string sImagePath;

    HANDLE  hProcess = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId );

    if ( IsValidHandle( hProcess ) )
    {
        CHAR    szPathBuffer[MAX_PATH] = "";

        if ( _GetModuleFileNameExA( hProcess, NULL, szPathBuffer, sizeof(szPathBuffer) ) )
            sImagePath = szPathBuffer;

        CloseHandle( hProcess );
    }

    return sImagePath;
}

std::string GetProcessImagePath( DWORD dwProcessId )
{
    return (LONG)GetVersion() < 0 ? GetProcessImagePath_9x( dwProcessId ) : GetProcessImagePath_NT( dwProcessId );
}

