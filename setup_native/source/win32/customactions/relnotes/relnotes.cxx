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



#ifdef _MSC_VER
#pragma warning(push,1) // disable warnings within system headers
#pragma warning(disable: 4917)
#endif
#include <windows.h>
#include <msiquery.h>
#include <shlobj.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <strsafe.h>
#include <string>

//----------------------------------------------------------
#ifdef DEBUG
inline void OutputDebugStringFormat( LPCTSTR pFormat, ... )
{
    TCHAR    buffer[1024];
    va_list  args;

    va_start( args, pFormat );
    StringCchVPrintf( buffer, sizeof(buffer), pFormat, args );
    OutputDebugString( buffer );
}
#else
static inline void OutputDebugStringFormat( LPCTSTR, ... )
{
}
#endif

//----------------------------------------------------------
inline bool IsValidHandle( HANDLE handle )
{
    return (NULL != handle) && (INVALID_HANDLE_VALUE != handle);
}

//----------------------------------------------------------
static bool GetMsiProp(MSIHANDLE handle, LPCTSTR name, /*out*/std::wstring& value)
{
    DWORD sz = 0;
    LPTSTR dummy = TEXT("");
    if (MsiGetProperty(handle, name, dummy, &sz) == ERROR_MORE_DATA)
    {
        sz++;
        DWORD nbytes = sz * sizeof(TCHAR);
        LPTSTR buff = reinterpret_cast<LPTSTR>(_alloca(nbytes));
        ZeroMemory(buff, nbytes);
        MsiGetProperty(handle, name, buff, &sz);
        value = buff;
        return true;
    }
    return false;
}

//----------------------------------------------------------
//----------------------------------------------------------
//----------------------------------------------------------
UINT ShowReleaseNotes( TCHAR* pFileName, TCHAR* pFilePath )
{
    TCHAR sFullPath[ MAX_PATH ];

    if ( FAILED( StringCchCopy( sFullPath, MAX_PATH, pFilePath ) ) )
    {
        OutputDebugStringFormat( TEXT("DEBUG: ShowReleaseNotes: Could not copy path [%s]"), pFilePath );
        return ERROR_SUCCESS;
    }

    if ( FAILED( StringCchCat( sFullPath, MAX_PATH, pFileName ) ) )
    {
        OutputDebugStringFormat( TEXT("DEBUG: ShowReleaseNotes: Could not append filename [%s]"), pFileName );
        return ERROR_SUCCESS;
    }

    HANDLE hFile = CreateFile( sFullPath, 0, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if ( IsValidHandle(hFile) )
    {
        CloseHandle( hFile );
        OutputDebugStringFormat( TEXT("DEBUG: ShowReleaseNotes: Found file [%s]"), sFullPath );

        SHELLEXECUTEINFOW aExecInf;
        ZeroMemory( &aExecInf, sizeof( aExecInf ) );

        aExecInf.cbSize = sizeof( aExecInf );
        aExecInf.fMask  = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI;
        aExecInf.lpVerb = TEXT("open");
        aExecInf.lpFile = sFullPath;
        aExecInf.lpDirectory = NULL;
        aExecInf.nShow = SW_SHOWNORMAL;

        SetLastError( 0 );
        ShellExecuteEx( &aExecInf );
    }
    else
    {
        OutputDebugStringFormat( TEXT("DEBUG: ShowReleaseNotes: File not found [%s]"), sFullPath );
    }

    return ERROR_SUCCESS;
}

//----------------------------------------------------------
extern "C" UINT __stdcall ShowReleaseNotesBefore( MSIHANDLE )
{
    TCHAR szPath[MAX_PATH];

    if( FAILED( SHGetSpecialFolderPath( NULL, szPath, CSIDL_COMMON_DOCUMENTS, true ) ) )
        return ERROR_SUCCESS;

    OutputDebugString( TEXT("DEBUG: ShowReleaseNotesBefore called") );

    return ShowReleaseNotes( TEXT("\\sun\\releasenote1.url"), szPath );
}

//----------------------------------------------------------
extern "C" UINT __stdcall ShowReleaseNotesAfter( MSIHANDLE )
{
    TCHAR szPath[MAX_PATH];

    if( FAILED( SHGetSpecialFolderPath( NULL, szPath, CSIDL_COMMON_DOCUMENTS, true ) ) )
        return ERROR_SUCCESS;

    OutputDebugString( TEXT("DEBUG: ShowReleaseNotesAfter called") );

    return ShowReleaseNotes( TEXT("\\sun\\releasenote2.url"), szPath );
}

//----------------------------------------------------------
extern "C" UINT __stdcall ShowSurveyAfter( MSIHANDLE handle )
{
    std::wstring prodname;

    GetMsiProp( handle, TEXT("ProductName"), prodname );
    std::wstring::size_type nIndex = prodname.find( TEXT( "Apache OpenOffice" ) );
    if( std::wstring::npos == nIndex )
        return ERROR_SUCCESS;

    OutputDebugString( TEXT("DEBUG: ShowSurveyAfter called") );

    SHELLEXECUTEINFOW aExecInf;
    ZeroMemory( &aExecInf, sizeof( aExecInf ) );

    aExecInf.cbSize = sizeof( aExecInf );
    aExecInf.fMask  = SEE_MASK_FLAG_DDEWAIT | SEE_MASK_FLAG_NO_UI;
    aExecInf.lpVerb = TEXT("open");
    //aExecInf.lpFile = TEXT("http://surveys.services.openoffice.org/deinstall");
    aExecInf.lpFile = TEXT("http://www.openoffice.org/support/");
    aExecInf.lpDirectory = NULL;
    aExecInf.nShow = SW_SHOWNORMAL;

    SetLastError( 0 );
    ShellExecuteEx( &aExecInf );

    return ERROR_SUCCESS;
}

