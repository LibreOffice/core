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

#include "shlxtmsi.hxx"

#include <malloc.h>

/*
    Called during installation to customize the start menu folder icon.
    See: http://msdn.microsoft.com/library/en-us/shellcc/platform/shell/programmersguide/shell_basics/shell_basics_extending/custom.asp
*/
extern "C" UINT __stdcall InstallStartmenuFolderIcon( MSIHANDLE handle )
{
    std::wstring sOfficeMenuFolder = GetMsiPropertyW( handle, L"OfficeMenuFolder" );
    std::wstring sDesktopFile = sOfficeMenuFolder + L"Desktop.ini";

    // at the moment there exists no Vista Icon, so we use the default folder icon.
    // add the icon into desktop/util/verinfo.rc

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
