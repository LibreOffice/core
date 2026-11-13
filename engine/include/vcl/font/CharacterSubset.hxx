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

#pragma once

#include <sal/types.h>
#include <vcl/BinaryDataContainer.hxx>
#include <vcl/dllapi.h>
#include <vcl/vclenum.hxx>

#include <set>
#include <utility>
#include <vector>

namespace font
{
/// The font file rFontData cut down to the glyphs of rCharacters, shaping and drawing them as the
/// whole font does. Empty when rFontData cannot be cut down.
VCL_DLLPUBLIC BinaryDataContainer subsetToCharacters(const BinaryDataContainer& rFontData,
                                                     const std::set<sal_UCS4>& rCharacters);

/// The characters of rCharacters as ranges of code points, each one the first and the last code
/// point of a run without gaps, in ascending order.
VCL_DLLPUBLIC std::vector<std::pair<sal_UCS4, sal_UCS4>>
characterRanges(const std::set<sal_UCS4>& rCharacters);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
