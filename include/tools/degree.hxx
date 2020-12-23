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
#include <o3tl/strong_int.hxx>

/** tenths of a Degree, normally rotation */
typedef o3tl::strong_int<sal_Int16, struct Degree10Tag> Degree10;

/** custom literal */
constexpr Degree10 operator""_deg10(unsigned long long n) { return Degree10{ n }; }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
