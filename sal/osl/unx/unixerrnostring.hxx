/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>

// Return the symbolic name of an errno value, like "ENOENT".

// Rationale why to use this and not strerror(): This is intended to be used in SAL_INFO() and
// SAL_WARN(). Such messages are intended to be read by developers, not end-users. Developers are
// (or should be) familiar with symbolic errno names in code anyway. strerror() is localized and the
// localised error strings might be less familiar to a developer that happens to run a localised
// environment.

std::string UnixErrnoString(int nErrno);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
