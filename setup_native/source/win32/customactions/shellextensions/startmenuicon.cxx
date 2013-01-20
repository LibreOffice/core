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

#define _WIN32_WINDOWS 0x0410

#ifdef _MSC_VER
#pragma warning(push, 1) /* disable warnings within system headers */
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <../tools/msiprop.hxx>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <malloc.h>
#include <tchar.h>
#include <string>


/*
    Called during installation to customize the start menu folder icon.
    See: http://msdn.microsoft.com/library/en-us/shellcc/platform/shell/programmersguide/shell_basics/shell_basics_extending/custom.asp
*/
extern "C" UINT __stdcall InstallStartmenuFolderIcon( MSIHANDLE handle )
{
    std::string   sOfficeMenuFolder = GetMsiPropValue( handle, TEXT("OfficeMenuFolder") );
    std::string   sDesktopFile = sOfficeMenuFolder + TEXT("Desktop.ini");
    std::string   sIconFile = GetMsiPropValue( handle, TEXT("INSTALLLOCATION") ) + TEXT("program\\soffice.exe");

    OSVERSIONINFO   osverinfo;
    osverinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx( &osverinfo );

    if (osverinfo.dwMajorVersion < 6 /* && osverinfo.dwMinorVersion  */ )
    {
        WritePrivateProfileString(
            TEXT(".ShellClassInfo"),
            TEXT("IconFile"),
            sIconFile.c_str(),
            sDesktopFile.c_str() );

        WritePrivateProfileString(
            TEXT(".ShellClassInfo"),
            TEXT("IconIndex"),
            TEXT("0"),
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

    SetFileAttributes( sDesktopFile.c_str(), FILE_ATTRIBUTE_HIDDEN );
    SetFileAttributes( sOfficeMenuFolder.c_str(), FILE_ATTRIBUTE_SYSTEM );


    return ERROR_SUCCESS;
}

extern "C" UINT __stdcall DeinstallStartmenuFolderIcon(MSIHANDLE handle)
{
    std::string   sOfficeMenuFolder = GetMsiPropValue( handle, TEXT("OfficeMenuFolder") );
    std::string sDesktopFile = sOfficeMenuFolder + TEXT("Desktop.ini");

    SetFileAttributes( sDesktopFile.c_str(), FILE_ATTRIBUTE_NORMAL );
    DeleteFile( sDesktopFile.c_str() );

    SetFileAttributes( sOfficeMenuFolder.c_str(), FILE_ATTRIBUTE_NORMAL );

    return ERROR_SUCCESS;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
