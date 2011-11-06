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



#define _WIN32_WINDOWS 0x0410

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>

#ifdef UNICODE
#define _UNICODE
#define _tstring    wstring
#else
#define _tstring    string
#endif
#include <tchar.h>
#include <string>


std::_tstring GetMsiProperty( MSIHANDLE handle, const std::_tstring& sProperty )
{
    std::_tstring   result;
    TCHAR   szDummy[1] = TEXT("");
    DWORD   nChars = 0;

    if ( MsiGetProperty( handle, sProperty.c_str(), szDummy, &nChars ) == ERROR_MORE_DATA )
    {
        DWORD nBytes = ++nChars * sizeof(TCHAR);
        LPTSTR buffer = reinterpret_cast<LPTSTR>(_alloca(nBytes));
        ZeroMemory( buffer, nBytes );
        MsiGetProperty(handle, sProperty.c_str(), buffer, &nChars);
        result = buffer;
    }

    return  result;
}

/*
    Called during installation to customize the start menu folder icon.
    See: http://msdn.microsoft.com/library/en-us/shellcc/platform/shell/programmersguide/shell_basics/shell_basics_extending/custom.asp
*/
extern "C" UINT __stdcall InstallStartmenuFolderIcon( MSIHANDLE handle )
{
    std::_tstring   sOfficeMenuFolder = GetMsiProperty( handle, TEXT("OfficeMenuFolder") );
    std::_tstring sDesktopFile = sOfficeMenuFolder + TEXT("Desktop.ini");

    //    MessageBox(NULL, sDesktopFile.c_str(), TEXT("OfficeMenuFolder"), MB_OK | MB_ICONINFORMATION);

    std::_tstring   sIconFile = GetMsiProperty( handle, TEXT("INSTALLLOCATION") ) + TEXT("program\\soffice.exe");

    OSVERSIONINFO   osverinfo;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx( &osverinfo );

    if (osverinfo.dwMajorVersion < 6 /* && osverinfo.dwMinorVersion  */ )
    {
        // This icon (18) is a Windows folder until XP Version (number is 0 based)
        WritePrivateProfileString(
            TEXT(".ShellClassInfo"),
            TEXT("IconFile"),
            sIconFile.c_str(),
            sDesktopFile.c_str() );

        // FYI: in tool 'ResHack' this icon can be found on position '19' (number is 1 based)
        WritePrivateProfileString(
            TEXT(".ShellClassInfo"),
            TEXT("IconIndex"),
            TEXT("18"),
            sDesktopFile.c_str() );
    }
    // else
    // {
    //     // at the moment there exists no Vista Icon, so we use the default folder icon.
    //     // add the icon into desktop/util/verinfo.rc
    // }

    // The value '0' is to avoid a message like "You Are Deleting a System Folder" warning when deleting or moving the folder.
    WritePrivateProfileString(
        TEXT(".ShellClassInfo"),
        TEXT("ConfirmFileOp"),
        TEXT("0"),
        sDesktopFile.c_str() );

    /*
      WritePrivateProfileString(
      TEXT(".ShellClassInfo"),
      TEXT("InfoTip"),
      TEXT("StarOffice Productivity Suite"),
      sDesktopFile.c_str() );
    */

    SetFileAttributes( sDesktopFile.c_str(), FILE_ATTRIBUTE_HIDDEN );
    SetFileAttributes( sOfficeMenuFolder.c_str(), FILE_ATTRIBUTE_SYSTEM );


    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall DeinstallStartmenuFolderIcon(MSIHANDLE handle)
{
    std::_tstring   sOfficeMenuFolder = GetMsiProperty( handle, TEXT("OfficeMenuFolder") );
    std::_tstring sDesktopFile = sOfficeMenuFolder + TEXT("Desktop.ini");

    SetFileAttributes( sDesktopFile.c_str(), FILE_ATTRIBUTE_NORMAL );
    DeleteFile( sDesktopFile.c_str() );

    SetFileAttributes( sOfficeMenuFolder.c_str(), FILE_ATTRIBUTE_NORMAL );

    return ERROR_SUCCESS;
}
