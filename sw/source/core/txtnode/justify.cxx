/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vector>
#include <sal/types.h>
#include <swfont.hxx>
#include "justify.hxx"

namespace Justify
{
void SpaceDistribution(std::vector<sal_Int32>& rKernArray, const OUString& rText, sal_Int32 nStt,
                       sal_Int32 nLen, tools::Long nSpaceAdd, tools::Long nKern, bool bNoHalfSpace)
{
    assert(nStt + nLen <= rText.getLength());
    assert(nLen <= sal_Int32(rKernArray.size()));
    // nSpaceSum contains the sum of the intermediate space distributed
    // among Spaces by the Justification.
    // The Spaces themselves will be positioned in the middle of the
    // intermediate space, hence the nSpace/2.
    // In case of word-by-word underlining they have to be positioned
    // at the beginning of the intermediate space, so that the space
    // is not underlined.
    // A Space at the beginning or end of the text must be positioned
    // before (resp. after) the whole intermediate space, otherwise
    // the underline/strike-through would have gaps.
    tools::Long nSpaceSum = 0;
    // in word line mode and for Arabic, we disable the half space trick:
    const tools::Long nHalfSpace = bNoHalfSpace ? 0 : nSpaceAdd / 2;
    const tools::Long nOtherHalf = nSpaceAdd - nHalfSpace;
    tools::Long nKernSum = nKern;
    sal_Unicode cChPrev = rText[nStt];

    if (nSpaceAdd && (cChPrev == CH_BLANK))
        nSpaceSum = nHalfSpace;

    sal_Int32 nPrevIdx = 0;

    for (sal_Int32 i = 1; i < nLen; ++i, nKernSum += nKern)
    {
        // Find the beginning of the next cluster that has a different kern value.
        while (i < nLen && rKernArray[i] == rKernArray[nPrevIdx])
            ++i;

        if (i == nLen)
            break;

        sal_Unicode nCh = rText[nStt + i];

        // Apply SpaceSum
        if (cChPrev == CH_BLANK)
        {
            // no Pixel is lost:
            nSpaceSum += nOtherHalf;
        }

        if (nCh == CH_BLANK)
        {
            if (i + 1 == nLen)
                nSpaceSum += nSpaceAdd;
            else
                nSpaceSum += nHalfSpace;
        }

        cChPrev = nCh;
        rKernArray[nPrevIdx] += nKernSum + nSpaceSum;
        // In word line mode and for Arabic, we disabled the half space trick. If a portion
        // ends with a blank, the full nSpaceAdd value has been added to the character in
        // front of the blank. This leads to painting artifacts, therefore we remove the
        // nSpaceAdd value again:
        if (bNoHalfSpace && i + 1 == nLen && nCh == CH_BLANK)
            rKernArray[nPrevIdx] = rKernArray[nPrevIdx] - nSpaceAdd;

        // Advance nPrevIdx and assign kern values to previous cluster.
        for (tools::Long nValue = rKernArray[nPrevIdx++]; nPrevIdx < i; ++nPrevIdx)
            rKernArray[nPrevIdx] = nValue;
    }

    // the layout engine requires the total width of the output
    while (nPrevIdx < nLen)
        rKernArray[nPrevIdx++] += nKernSum + nSpaceSum;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
