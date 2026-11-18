/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config.h>

#import <Foundation/Foundation.h>

#include <common/FileUtil.hpp>

namespace FileUtil
{
    bool platformDependentCheckDiskSpace(const std::string& path, int64_t enoughSpace)
    {
        // FIXME: We don't actually use the path parameter here
        NSDictionary *atDict = [[NSFileManager defaultManager] attributesOfFileSystemForPath:@"/" error:NULL];
        long long freeSpace = [[atDict objectForKey:NSFileSystemFreeSize] longLongValue];
        long long totalSpace = [[atDict objectForKey:NSFileSystemSize] longLongValue];

        if (freeSpace > enoughSpace)
            return true;

        if (static_cast<double>(freeSpace) / totalSpace <= 0.05)
            return false;
        return true;
    }
} // namespace FileUtil

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
