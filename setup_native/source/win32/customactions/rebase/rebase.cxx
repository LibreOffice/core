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
#include <hash_map>

const DWORD PE_Signature = 0x00004550;
typedef std::pair< std::string, bool > StringPair;
typedef std::hash_map< std::string, bool > ExcludeLibsMap;

#ifdef DEBUG
static void OutputDebugStringFormat( LPCSTR pFormat, ... )
{
    CHAR    buffer[1024];
    va_list args;

    va_start( args, pFormat );
    StringCchVPrintfA( buffer, sizeof(buffer), pFormat, args );
    OutputDebugStringA( buffer );
}
#else
static void OutputDebugStringFormat( LPCSTR, ... )
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

static BOOL rebaseImagesInFolder( MSIHANDLE handle, const std::string& sPath, LPVOID address, ExcludeLibsMap& rExcludeMap )
{
    std::string     sDir     = sPath;
    std::string     sPattern = sPath + TEXT("*.dll");
    WIN32_FIND_DATA aFindFileData;

    HANDLE hFind = FindFirstFile( sPattern.c_str(), &aFindFileData );
    if ( IsValidHandle(hFind) )
    {
        BOOL fSuccess = false;

        do
        {
            std::string sFileName = aFindFileData.cFileName;
            if ( rExcludeMap.find( sFileName ) == rExcludeMap.end() )
            {
                OutputDebugStringFormat( "Rebase library: %s", sFileName.c_str() );
                std::string sLibFile = sDir +  sFileName;
                rebaseImage( handle, sLibFile, address );
            }
            else
            {
                OutputDebugStringFormat( "Exclude library %s from rebase", sFileName.c_str() );
            }

            fSuccess = FindNextFile( hFind, &aFindFileData );
        }
        while ( fSuccess );

        FindClose( hFind );
    }

    return ERROR_SUCCESS;
}

static BOOL rebaseImages( MSIHANDLE handle, LPVOID pAddress, ExcludeLibsMap& rMap )
{
    std::string sInstallPath = GetMsiProperty(handle, TEXT("INSTALLLOCATION"));

    std::string sBasisDir  = sInstallPath + TEXT("Basis\\program\\");
    std::string sOfficeDir = sInstallPath + TEXT("program\\");
    std::string sUreDir    = sInstallPath + TEXT("URE\\bin\\");

    BOOL bResult = rebaseImagesInFolder( handle, sBasisDir, pAddress, rMap );
    bResult &= rebaseImagesInFolder( handle, sOfficeDir, pAddress, rMap );
    bResult &= rebaseImagesInFolder( handle, sUreDir, pAddress, rMap );

    return bResult;
}

static BOOL IsServerSystem( MSIHANDLE /*handle*/ )
{
    OSVERSIONINFOEX osVersionInfoEx;
    osVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx(reinterpret_cast<LPOSVERSIONINFO>(&osVersionInfoEx));

    if ( osVersionInfoEx.wProductType != VER_NT_WORKSTATION )
    {
        OutputDebugStringFormat( "Server system detected. No rebase necessary!" );
        return TRUE;
    }
    else
    {
        OutputDebugStringFormat( "Client system detected. Rebase necessary!" );
        return FALSE;
    }
}

static void InitExcludeFromRebaseList( MSIHANDLE handle, ExcludeLibsMap& rMap )
{
    size_t      nPos( 0 );
    const TCHAR cDelim = ',';
    std::string sLibsExcluded = GetMsiProperty(handle, TEXT("EXCLUDE_FROM_REBASE"));

    while ( nPos < sLibsExcluded.size() )
    {
        size_t nDelPos = sLibsExcluded.find_first_of( cDelim, nPos );

        std::string sExcludedLibName;
        if ( nDelPos != std::string::npos )
        {
            sExcludedLibName = sLibsExcluded.substr( nPos, nDelPos - nPos );
            nPos = nDelPos+1;
        }
        else
        {
            sExcludedLibName = sLibsExcluded.substr( nPos );
            nPos = sLibsExcluded.size();
        }

        if ( sExcludedLibName.size() > 0 )
        {
            OutputDebugStringFormat( "Insert library %s into exclude from rebase list", sExcludedLibName.c_str() );
            rMap.insert( StringPair( sExcludedLibName, true ));
        }
    }
}

extern "C" BOOL __stdcall RebaseLibrariesOnProperties( MSIHANDLE handle )
{
    static LPVOID pDefault = reinterpret_cast<LPVOID>(0x10000000);

    OutputDebugStringFormat( "RebaseLibrariesOnProperties has been called" );
    std::string sDontOptimizeLibs = GetMsiProperty(handle, TEXT("DONTOPTIMIZELIBS"));
    if ( sDontOptimizeLibs.length() > 0 && sDontOptimizeLibs == "1" )
    {
        OutputDebugStringFormat( "Don't optimize libraries set. No rebase necessary!" );
        return TRUE;
    }

    if ( !IsServerSystem( handle ))
    {
        ExcludeLibsMap aExcludeLibsMap;
        InitExcludeFromRebaseList( handle, aExcludeLibsMap );

        return rebaseImages( handle, pDefault, aExcludeLibsMap );
    }

    return TRUE;
}
