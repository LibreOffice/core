/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once
#include <sal/types.h>

constexpr sal_Int64 SEC_MASK = SAL_CONST_INT64(1000000000);
constexpr sal_Int64 MIN_MASK = SAL_CONST_INT64(100000000000);
constexpr sal_Int64 HOUR_MASK = SAL_CONST_INT64(10000000000000);

/** Get current local timestamp.
    Both pDate and pTime can be null.
    Returns true if succeeded, false otherwise.
 */
bool GetSystemDateTime(sal_Int32* pDate, sal_Int64* pTime);

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
