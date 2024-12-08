/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sal/config.h>

#include <prewin.h>
#include <appmodel.h>
#include <postwin.h>

namespace sal::systools
{
// Returns true if the current process is run as a Windows store app, which has some specifics
inline bool IsStorePackagedApp()
{
    UINT32 size = 0;
    return GetCurrentPackageFullName(&size, nullptr) == ERROR_INSUFFICIENT_BUFFER;
}
} // sal::systools

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
