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
 * on Linux, failed on Windows.
 * So.....,
 * (1) introduce a typedef that replaces (almost) all previous usage
 * (2) on 64-bit Windows, this typedef is forced to 64-bit (32-bit platforms, including 32-bit Windows, are left alone)
 * (3) fix fallout from (2)
 *
 * As a consequence of the above, it would be best, over the long term, to regard usage
 * of tools::Long in the codebase as meaning "we're not sure what the ideal size of the datatype is",
 * and where possible, replace it with a better datatype like sal_Int32/sal_Int64/etc.
 *
 */
namespace tools
{
#if defined _WIN64
typedef sal_Int64 Long;
typedef sal_uInt64 ULong;
#else
typedef long Long;
typedef unsigned long ULong;
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
