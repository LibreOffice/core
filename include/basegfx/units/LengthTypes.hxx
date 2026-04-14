/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <basegfx/units/Length.hxx>
#include <basegfx/range/Range2D.hxx>
#include <basegfx/tuple/Size2D.hxx>

namespace gfx
{
using Tuple2DL = basegfx::Tuple2D<gfx::Length>;
using Size2DL = basegfx::Size2D<gfx::Length>;
using Range2DL = basegfx::Range2D<gfx::Length, gfx::LengthTraits>;
} // end namespace gfx

// To be consistent with other types in basegfx
namespace basegfx
{
using gfx::Tuple2DL;
using gfx::Size2DL;
using gfx::Range2DL;
}
