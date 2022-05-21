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

namespace
{
enum class IdeographicPunctuationClass
{
    NONE,
    OPEN_BRACKET,
    CLOSE_BRACKET,
    COMMA_OR_FULLSTOP
};

IdeographicPunctuationClass lcl_WhichPunctuationClass(sal_Unicode cChar)
{
    if ((cChar < 0x3001 || cChar > 0x3002) && (cChar < 0x3008 || cChar > 0x3011)
        && (cChar < 0x3014 || cChar > 0x301F) && 0xFF62 != cChar && 0xFF63 != cChar)
        return IdeographicPunctuationClass::NONE;
    else if (0x3001 == cChar || 0x3002 == cChar)
        return IdeographicPunctuationClass::COMMA_OR_FULLSTOP;
    else if (0x3009 == cChar || 0x300B == cChar || 0x300D == cChar || 0x300F == cChar
             || 0x3011 == cChar || 0x3015 == cChar || 0x3017 == cChar || 0x3019 == cChar
             || 0x301B == cChar || 0x301E == cChar || 0x301F == cChar || 0xFF63 == cChar)
        // right punctuation
        return IdeographicPunctuationClass::CLOSE_BRACKET;

    return IdeographicPunctuationClass::OPEN_BRACKET;
}
}

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

tools::Long SnapToGrid(std::vector<sal_Int32>& rKernArray, const OUString& rText, sal_Int32 nStt,
                       sal_Int32 nLen, tools::Long nGridWidth, tools::Long nWidth)
{
    assert(nStt + nLen <= rText.getLength());
    assert(nLen <= sal_Int32(rKernArray.size()));

    tools::Long nDelta = 0; // delta offset to text origin

    // Change the average width per character to an appropriate grid width
    // basically get the ratio of the avg width to the grid unit width, then
    // multiple this ratio to give the new avg width - which in this case
    // gives a new grid width unit size

    tools::Long nAvgWidthPerChar = rKernArray[nLen - 1] / nLen;

    const sal_uLong nRatioAvgWidthCharToGridWidth
        = nAvgWidthPerChar ? (nAvgWidthPerChar - 1) / nGridWidth + 1 : 1;

    nAvgWidthPerChar = nRatioAvgWidthCharToGridWidth * nGridWidth;

    // the absolute end position of the first character is also its width
    tools::Long nCharWidth = rKernArray[0];
    sal_uLong nHalfWidth = nAvgWidthPerChar / 2;

    tools::Long nNextFix = 0;

    // we work out the start position (origin) of the first character,
    // and we set the next "fix" offset to half the width of the char.
    // The exceptions are for punctuation characters that are not centered
    // so in these cases we just add half a regular "average" character width
    // to the first characters actual width to allow the next character to
    // be centered automatically
    // If the character is "special right", then the offset is correct already
    // so the fix offset is as normal - half the average character width

    sal_Unicode cChar = rText[nStt];
    IdeographicPunctuationClass eClass = lcl_WhichPunctuationClass(cChar);
    switch (eClass)
    {
        case IdeographicPunctuationClass::NONE:
            // Centered
            nDelta = (nAvgWidthPerChar - nCharWidth) / 2;
            nNextFix = nCharWidth / 2;
            break;
        case IdeographicPunctuationClass::CLOSE_BRACKET:
        case IdeographicPunctuationClass::COMMA_OR_FULLSTOP:
            // Closer to previous ideograph
            nNextFix = nHalfWidth;
            break;
        default:
            // case IdeographicPunctuationClass::OPEN_BRACKET: closer to next ideograph.
            nDelta = nAvgWidthPerChar - nCharWidth;
            nNextFix = nCharWidth - nHalfWidth;
    }

    // calculate offsets
    for (sal_Int32 j = 1; j < nLen; ++j)
    {
        tools::Long nCurrentCharWidth = rKernArray[j] - rKernArray[j - 1];
        nNextFix += nAvgWidthPerChar;

        // almost the same as getting the offset for the first character:
        // punctuation characters are not centered, so just add half an
        // average character width minus the characters actual char width
        // to get the offset into the centre of the next character

        cChar = rText[nStt + j];
        eClass = lcl_WhichPunctuationClass(cChar);
        switch (eClass)
        {
            case IdeographicPunctuationClass::NONE:
                // Centered
                rKernArray[j - 1] = nNextFix - (nCurrentCharWidth / 2);
                break;
            case IdeographicPunctuationClass::CLOSE_BRACKET:
            case IdeographicPunctuationClass::COMMA_OR_FULLSTOP:
                // Closer to previous ideograph
                rKernArray[j - 1] = nNextFix - nHalfWidth;
                break;
            default:
                // case IdeographicPunctuationClass::OPEN_BRACKET: closer to next ideograph.
                rKernArray[j - 1] = nNextFix + nHalfWidth - nCurrentCharWidth;
        }
    }

    // the layout engine requires the total width of the output
    rKernArray[nLen - 1] = nWidth - nDelta;

    return nDelta;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
