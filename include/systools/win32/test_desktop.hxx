/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <stdlib.h>
#include <windows.h>

namespace sal::systools
{
// Create a desktop, to avoid popups interfering with active user session, because we don't use svp
// vcl plugin on Windows for unit testing. Set LIBO_TEST_DEFAULT_DESKTOP environment variable to any
// value when running the unit test process, to avoid use of a separate desktop.
void maybeCreateTestDesktop()
{
    if (getenv("LIBO_TEST_DEFAULT_DESKTOP") == nullptr)
    {
        if (HDESK hDesktop
            = CreateDesktopW(L"LIBO_TEST_DESKTOP", nullptr, nullptr, 0, GENERIC_ALL, nullptr))
            SetThreadDesktop(hDesktop);
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
