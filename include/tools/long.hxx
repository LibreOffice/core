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

/**
 * We have an unfortunate problem in that, on Windows (both 32 and 64-bit), long is always 32-bit.
 * But on Linux (64-bit) long is 64-bit. Which leads to nasty situations where things that work
 * on Linux, fail on Windows.
 * So.....,
 * (1) introduce a typedef that will replace all usage of long in our codebase.
 * (2) to start with, that typedef acts that same as the long it replaces
 * (3) once the replacement is complete, this typedef will, on windows, become 64-bit.
 * (4) and once the fallout from (3) is fixed, we can start replacing tools::Long with things
 *     like sal_Int64.
 */
namespace tools
{
#if SAL_TYPES_SIZEOFPOINTER == 8
typedef sal_Int64 Long;
typedef sal_uInt64 ULong;
#else
typedef sal_Int32 Long;
typedef sal_uInt32 ULong;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
