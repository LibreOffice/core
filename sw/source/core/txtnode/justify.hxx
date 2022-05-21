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

namespace Justify
{
/// Distribute space between words and letters.
/// @param[in,out] rKernArray text positions from OutDev::GetTextArray().
/// @param rText string used to determine where space and kern are inserted.
/// @param nStt starting index of rText.
/// @param nLen number of elements to process in rKernArray and rText.
/// @param nSpaceAdd amount of space to insert for each CH_BLANK.
/// @param nKern amount of space to insert between letters.
/// @param bNoHalfSpace whether to split the space into two halves.
///        Split spaces are inserted before and after CH_BLANK.
///        Set to true in word line mode and for Arabic text to avoid splitting.
SW_DLLPUBLIC void SpaceDistribution(std::vector<sal_Int32>& rKernArray, const OUString& rText,
                                    sal_Int32 nStt, sal_Int32 nLen, tools::Long nSpaceAdd,
                                    tools::Long nKern, bool bNoHalfSpace);

/// Snap ideographs to text grids:
/// a) Ideographic open brackets are aligned to the rightmost edge of spanned grids so that
//  they can be closer to the next ideograph.
/// b) Ideographic close brackets, ideogrpahic comma, and idographic fullstop are aligned
/// to the leftmost edge of spanned grids so that they can be closer to the previous
/// ideograph.
/// c) Other ideographs are aligned to the center of the spnaned grids.
/// @param[in,out] rKernArray text positions from OutDev::GetTextArray().
/// @param rText string used to determine where space and kern are inserted.
/// @param nStt starting index of rText.
/// @param nLen number of elements to process in rKernArray and rText.
/// @param nGirdWidth width of a text gird
/// @param nWidth width of the whole portion.
/// @return the delta offset of first glyph so text origin can be updated accordingly.
SW_DLLPUBLIC tools::Long SnapToGrid(std::vector<sal_Int32>& rKernArray, const OUString& rText,
                                    sal_Int32 nStt, sal_Int32 nLen, tools::Long nGridWidth,
                                    tools::Long nWidth);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
