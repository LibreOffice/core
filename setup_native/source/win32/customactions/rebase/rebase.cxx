#undef UNICODE
#undef _UNICODE

#pragma once

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#include <imagehlp.h>
#include <tchar.h>
#include <strsafe.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>
#include <time.h>
#include <string>

const DWORD PE_Signature = 0x00004550;

#ifdef DEBUG
inline void OutputDebugStringFormat( LPCSTR pFormat, ... )
{
    CHAR    buffer[1024];
    va_list args;

    va_start( args, pFormat );
    StringCchVPrintfA( buffer, sizeof(buffer), pFormat, args );
    OutputDebugStringA( buffer );
}
#else
static inline void OutputDebugStringFormat( LPCSTR, ... )
{
}
#endif

static bool IsValidHandle( HANDLE handle )
{
    return NULL != handle && INVALID_HANDLE_VALUE != handle;
}

static std::string GetMsiProperty(MSIHANDLE handle, const std::string& sProperty)
{
    std::string result;
    TCHAR       szDummy[1] = TEXT("");
    DWORD       nChars = 0;

    if (MsiGetProperty(handle, sProperty.c_str(), szDummy, &nChars) == ERROR_MORE_DATA)
    {
        DWORD nBytes = ++nChars * sizeof(TCHAR);
        LPTSTR buffer = reinterpret_cast<LPTSTR>(_alloca(nBytes));
        ZeroMemory( buffer, nBytes );
        MsiGetProperty(handle, sProperty.c_str(), buffer, &nChars);
        result = buffer;
    }
    return result;
}

static BOOL rebaseImage( const std::string& filePath, LPVOID address )
{
    ULONG ulOldImageSize;
    ULONG_PTR lpOldImageBase;
    ULONG ulNewImageSize;
    ULONG_PTR lpNewImageBase = reinterpret_cast<ULONG_PTR>(address);

    BOOL bResult = ReBaseImage(
        filePath.c_str(),
        "",
        TRUE,
        FALSE,
        FALSE,
        0,
        &ulOldImageSize,
        &lpOldImageBase,
        &ulNewImageSize,
        &lpNewImageBase,
        (ULONG)time(NULL) );

    return bResult;
}

static BOOL rebaseImage( MSIHANDLE /*handle*/, const std::string& sFilePath, LPVOID address )
{
    std::string mystr;
    mystr = "Full file: " + sFilePath;

    BOOL bResult = rebaseImage( sFilePath, address );

    if ( !bResult )
    {
        OutputDebugStringFormat( "Rebasing library %s failed", mystr.c_str() );
    }

    return bResult;
}

static BOOL rebaseImagesInFolder( MSIHANDLE handle, const std::string& sPath, LPVOID address )
{
    std::string   sDir     = sPath;
    std::string   sPattern = sPath + TEXT("*.dll");

    WIN32_FIND_DATA aFindFileData;
    HANDLE  hFind = FindFirstFile( sPattern.c_str(), &aFindFileData );

    if ( IsValidHandle(hFind) )
    {
        BOOL fSuccess = false;

        do
        {
            std::string sLibFile = sDir + aFindFileData.cFileName;
            rebaseImage( handle, sLibFile, address );
            fSuccess = FindNextFile( hFind, &aFindFileData );
        }
        while ( fSuccess );

        FindClose( hFind );
    }

    return ERROR_SUCCESS;
}

static BOOL rebaseImages( MSIHANDLE handle, LPVOID pAddress )
{
    std::string sOfficeInstallPath = GetMsiProperty(handle, TEXT("OFFICEINSTALLLOCATION"));
    std::string sBasisInstallPath = GetMsiProperty(handle, TEXT("BASISINSTALLLOCATION"));
    std::string sUreInstallPath = GetMsiProperty(handle, TEXT("UREINSTALLLOCATION"));

    std::string sBasisDir  = sBasisInstallPath + TEXT("program\\");
    std::string sOfficeDir = sOfficeInstallPath + TEXT("program\\");
    std::string sUreDir    = sUreInstallPath + TEXT("bin\\");

    BOOL bResult = rebaseImagesInFolder( handle, sBasisDir, pAddress );
    bResult &= rebaseImagesInFolder( handle, sOfficeDir, pAddress );
    bResult &= rebaseImagesInFolder( handle, sUreDir, pAddress );

    return bResult;
}

static BOOL IsServerSystem( MSIHANDLE /*handle*/ )
{
    OSVERSIONINFOEX osVersionInfoEx;
    osVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx(reinterpret_cast<LPOSVERSIONINFO>(&osVersionInfoEx));

    if ( osVersionInfoEx.wProductType != VER_NT_WORKSTATION )
        return TRUE;
    else
        return FALSE;
}

extern "C" BOOL __stdcall RebaseLibrariesOnProperties( MSIHANDLE handle )
{
    static LPVOID pDefault = reinterpret_cast<LPVOID>(0x10000000);

    std::string sDontOptimizeLibs = GetMsiProperty(handle, TEXT("DONTOPTIMIZELIBS"));
    if ( sDontOptimizeLibs.length() > 0 && sDontOptimizeLibs == "1" )
        return TRUE;

    if ( !IsServerSystem( handle ))
        return rebaseImages( handle, pDefault );

    return TRUE;
}
