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
///        Splitted spaces are inserted before and after CH_BLANK.
///        Set to true in word line mode and for Arabic text to avoid splitting.
SW_DLLPUBLIC void SpaceDistribution(std::vector<sal_Int32>& rKernArray, const OUString& rText,
                                    sal_Int32 nStt, sal_Int32 nLen, tools::Long nSpaceAdd,
                                    tools::Long nKern, bool bNoHalfSpace);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
