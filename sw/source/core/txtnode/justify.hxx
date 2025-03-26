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
#include <TextFrameIndex.hxx>

namespace sw::Justify
{
/// Get model position base on given kern array.
/// @param rKernArray text positions from OutDev::GetTextArray().
/// @param nLen number of elements to process in rKernArray.
/// @param nX the visual position
SW_DLLPUBLIC sal_Int32 GetModelPosition(const KernArray& rKernArray, sal_Int32 nLen,
                                        tools::Long nX);
/// Distribute space between words and letters.
/// @param[in,out] rKernArray text positions from OutDev::GetTextArray().
/// @param aText string used to determine where space and kern are inserted.
/// @param nStt starting index of rText.
/// @param nLen number of elements to process in rKernArray and rText.
/// @param nSpaceAdd amount of space to insert for each CH_BLANK.
/// @param nKern amount of space to insert between letters.
/// @param bNoHalfSpace whether to split the space into two halves.
///        Split spaces are inserted before and after CH_BLANK.
///        Set to true in word line mode and for Arabic text to avoid splitting.
SW_DLLPUBLIC void SpaceDistribution(KernArray& rKernArray, std::u16string_view aText,
                                    sal_Int32 nStt, sal_Int32 nLen, tools::Long nSpaceAdd,
                                    tools::Long nKern, bool bNoHalfSpace);

/// Snap ideographs to text grids:
/// a) Ideographic open brackets are aligned to the rightmost edge of spanned grids so that
//  they can be closer to the next ideograph.
/// b) Ideographic close brackets, ideographic comma, and ideographic fullstop are aligned
/// to the leftmost edge of spanned grids so that they can be closer to the previous
/// ideograph.
/// c) Other ideographs are aligned to the center of the spanned grids.
/// @param[in,out] rKernArray text positions from OutDev::GetTextArray().
/// @param aText string used to determine where space and kern are inserted.
/// @param nStt starting index of rText.
/// @param nLen number of elements to process in rKernArray and rText.
/// @param nGridWidth width of a text grid
/// @param bForceLeft for align to the left edge of the grid disregard of the punctuation type.
/// This is useful for calculate text width, line break, and conversion model position.
/// @return the delta offset of first glyph so text origin can be updated accordingly.
SW_DLLPUBLIC tools::Long SnapToGrid(KernArray& rKernArray, std::u16string_view aText,
                                    sal_Int32 nStt, sal_Int32 nLen, tools::Long nGridWidth,
                                    bool bForceLeft);

/// Snap ideographs to text grids edge ( used when snap to char is off ):
/// space will be distributed ( in case that alignment is set to justify. ).
/// @param[in,out] rKernArray text positions from OutDev::GetTextArray().
/// @param nLen number of elements to process in rKernArray and rText.
/// @param nGridWidth width of a text grid
/// @param nSpace amount of space distributed under justify text alignment mode.
/// @param nKern letter spacing.
/// @param nBaseFontSize document font size, used for MSO-compatible grid
/// @param bUseMsoCompatibleGrid changes grid algorithm to match MSO
SW_DLLPUBLIC void SnapToGridEdge(KernArray& rKernArray, sal_Int32 nLen, tools::Long nGridWidth,
                                 tools::Long nSpace, tools::Long nKern, tools::Long nBaseFontSize,
                                 bool bUseMsoCompatibleGrid);

/// Performs a kashida justification on the kerning array
/// @param aKashPositions Array of kashida insertion positions relative to paragraph
/// @param[in,out] rKernArray text positions from OutDev::GetTextArray()
/// @param[out] pKashidaArray Array marking locations for inserted tatweel glyphs. Optional.
/// @param nStt String start index relative to the paragraph
/// @param nLen Length of substring
/// @param nSpaceAdd Amount of space to add to each kashida insertion opportunity
SW_DLLPUBLIC bool KashidaJustify(std::span<TextFrameIndex const> aKashPositions,
                                 KernArray& rKernArray, sal_Bool* pKashidaArray, sal_Int32 nStt,
                                 sal_Int32 nLen, tools::Long nSpaceAdd);

/// tdf#88908: Perform a CJK space balancing on the kerning array
/// @param[in,out] rKernArray text positions from OutDev::GetTextArray().
/// @param aText string used to determine where space and kern are inserted.
/// @param nStt starting index of rText.
/// @param nLen number of elements to process in rKernArray and rText.
/// @param nSpaceWidth new size of qualifying spaces.
/// @param bInsideCjkScript the containing script is CJK
SW_DLLPUBLIC void BalanceCjkSpaces(KernArray& rKernArray, std::u16string_view aText, sal_Int32 nStt,
                                   sal_Int32 nLen, double dSpaceWidth, bool bInsideCjkScript);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
