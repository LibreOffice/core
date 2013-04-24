/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

sal_Bool safeWrite( int fd, void* data, sal_uInt32 dataSize );

// This function *will* read |count| bytes from |fd|, busy looping
// if needed. Don't use it when you don't know if you can request enough
// data. It will return sal_False for any partial transfer or error.
sal_Bool safeRead( int fd, void* buffer, sal_uInt32 count );

#ifdef __cplusplus
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
