/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include "../loader.hxx"

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    // no console for unopkg.bin when started by unopkg.exe
    return desktop_win32::unopkgloader_impl(false);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
