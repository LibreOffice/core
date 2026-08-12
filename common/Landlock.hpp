/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <string>
#include <vector>

namespace Landlock
{
    enum class Access : char
    {
        ReadOnly, ReadOnlyDir, ReadWrite, ReadWriteDir
    };
    struct Permission {
        std::string _path;
        Access _access;
        Permission(const std::string &path, Access access) :
            _path(path), _access(access) {}
    };

    /// True when the kernel can enforce landlock rules for this process
    bool isSupported();

    /// Lock-down process paths denying read/write access to many of them
    bool lock(const std::vector<Permission> &perms);

    /// belt + braces internal restriction / protection mechanism,
    /// FIXME: deprecate this in favour of pure landlock in 27.04
    void setAllowedPaths(const std::vector<Permission> &perms);

}; // namespace Landlock

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
