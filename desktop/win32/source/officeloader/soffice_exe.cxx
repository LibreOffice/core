/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "../loader.hxx"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    // Check if its parent has a console (i.e. this process is launched from command line), and if
    // so, attach to it. It will enable child process to retrieve this console if it needs to output
    // to console
    const bool bHasConsole = AttachConsole(ATTACH_PARENT_PROCESS);
    return desktop_win32::officeloader_impl(!bHasConsole);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
