/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <tools/long.hxx>

#ifdef DBG_UTIL

#include <o3tl/strong_int.hxx>

typedef o3tl::strong_int<tools::Long, struct SwTwipsTag> SwTwips;
constexpr SwTwips INVALID_TWIPS(LONG_MAX);
constexpr SwTwips TWIPS_MAX(LONG_MAX - 1);

namespace std
{
constexpr SwTwips max(SwTwips a, SwTwips b) { return a > b ? a : b; }
constexpr SwTwips abs(SwTwips a) { return a > SwTwips(0) ? a : -a; }
};

#else

typedef tools::Long SwTwips;
#define INVALID_TWIPS LONG_MAX
#define TWIPS_MAX (LONG_MAX - 1)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
