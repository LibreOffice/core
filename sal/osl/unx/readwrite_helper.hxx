/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SAL_OSL_UNX_READWRITE_HELPER_HXX
#define INCLUDED_SAL_OSL_UNX_READWRITE_HELPER_HXX

#include <sal/config.h>

#include <cstddef>

bool safeWrite(int fd, void* data, std::size_t dataSize);

// This function *will* read |count| bytes from |fd|, busy looping
// if needed. Don't use it when you don't know if you can request enough
// data. It will return sal_False for any partial transfer or error.
bool safeRead(int fd, void* buffer, std::size_t count);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
