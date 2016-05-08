/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#define UNICODE
#define _UNICODE

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <msiquery.h>
#ifdef _WIN32_WINNT_WINBLUE
#include <VersionHelpers.h>
#endif
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>
#include <string>

std::wstring GetMsiPropertyW( MSIHANDLE handle, const std::wstring& sProperty )
{
    std::wstring   result;
    WCHAR   szDummy[1] = L"";
    DWORD   nChars = 0;

    if ( MsiGetPropertyW( handle, sProperty.c_str(), szDummy, &nChars ) == ERROR_MORE_DATA )
    {
        DWORD nBytes = ++nChars * sizeof(WCHAR);
        PWSTR buffer = reinterpret_cast<PWSTR>(_alloca(nBytes));
        ZeroMemory( buffer, nBytes );
        MsiGetPropertyW(handle, sProperty.c_str(), buffer, &nChars);
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
    std::wstring sOfficeMenuFolder = GetMsiPropertyW( handle, L"OfficeMenuFolder" );
    std::wstring sDesktopFile = sOfficeMenuFolder + L"Desktop.ini";
    std::wstring sIconFile = GetMsiPropertyW( handle, L"INSTALLLOCATION" ) + L"program\\soffice.exe";

// the Win32 SDK 8.1 deprecates GetVersionEx()
#ifdef _WIN32_WINNT_WINBLUE
    bool const bIsVistaOrLater = IsWindowsVistaOrGreater() ? true : false;
#else
    OSVERSIONINFO   osverinfo;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx( &osverinfo );
    bool const bIsVistaOrLater = (osverinfo.dwMajorVersion >= 6);
#endif

    if (!bIsVistaOrLater)
    {
        WritePrivateProfileStringW(
            L".ShellClassInfo",
            L"IconFile",
            sIconFile.c_str(),
            sDesktopFile.c_str() );

        WritePrivateProfileStringW(
            L".ShellClassInfo",
            L"IconIndex",
            L"0",
            sDesktopFile.c_str() );
    }
    // else
    // {
    //     // at the moment there exists no Vista Icon, so we use the default folder icon.
    //     // add the icon into desktop/util/verinfo.rc
    // }

    // The value '0' is to avoid a message like "You Are Deleting a System Folder" warning when deleting or moving the folder.
    WritePrivateProfileStringW(
        L".ShellClassInfo",
        L"ConfirmFileOp",
        L"0",
        sDesktopFile.c_str() );

    SetFileAttributesW( sDesktopFile.c_str(), FILE_ATTRIBUTE_HIDDEN );
    SetFileAttributesW( sOfficeMenuFolder.c_str(), FILE_ATTRIBUTE_SYSTEM );


    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall DeinstallStartmenuFolderIcon(MSIHANDLE handle)
{
    std::wstring sOfficeMenuFolder = GetMsiPropertyW( handle, L"OfficeMenuFolder" );
    std::wstring sDesktopFile = sOfficeMenuFolder + L"Desktop.ini";

    SetFileAttributesW( sDesktopFile.c_str(), FILE_ATTRIBUTE_NORMAL );
    DeleteFileW( sDesktopFile.c_str() );

    SetFileAttributesW( sOfficeMenuFolder.c_str(), FILE_ATTRIBUTE_NORMAL );

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
