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
#include <vcl/kernarray.hxx>
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

tools::Long lcl_MinGridWidth(tools::Long nGridWidth, tools::Long nCharWidth)
{
    tools::Long nCount = nCharWidth > nGridWidth ? (nCharWidth - 1) / nGridWidth + 1 : 1;
    return nCount * nGridWidth;
}

tools::Long lcl_OffsetFromGridEdge(tools::Long nMinWidth, tools::Long nCharWidth, sal_Unicode cChar,
                                   bool bForceLeft)
{
    if (bForceLeft)
        return 0;

    tools::Long nOffset = 0;

    switch (lcl_WhichPunctuationClass(cChar))
    {
        case IdeographicPunctuationClass::NONE:
            // Centered
            nOffset = (nMinWidth - nCharWidth) / 2;
            break;
        case IdeographicPunctuationClass::OPEN_BRACKET:
            // Align to next edge, closer to next ideograph
            nOffset = nMinWidth - nCharWidth;
            break;
        default:
            // CLOSE_BRACKET or COMMA_OR_FULLSTOP:
            // Align to previous edge, closer to previous ideograph.
            break;
    }
    return nOffset;
}
}

namespace sw::Justify
{
sal_Int32 GetModelPosition(const KernArray& rKernArray, sal_Int32 nLen, tools::Long nX)
{
    tools::Long nLeft = 0, nRight = 0;
    sal_Int32 nLast = 0, nIdx = 0;

    do
    {
        nRight = rKernArray[nLast];
        ++nIdx;
        while (nIdx < nLen && rKernArray[nIdx] == rKernArray[nLast])
            ++nIdx;

        if (nIdx < nLen)
        {
            if (nX < nRight)
                return (nX - nLeft < nRight - nX) ? nLast : nIdx;

            nLeft = nRight;
            nLast = nIdx;
        }
    } while (nIdx < nLen);
    return nIdx;
}

void SpaceDistribution(KernArray& rKernArray, std::u16string_view aText, sal_Int32 nStt,
                       sal_Int32 nLen, tools::Long nSpaceAdd, tools::Long nKern, bool bNoHalfSpace)
{
    assert(nStt + nLen <= sal_Int32(aText.size()));
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
    double nSpaceSum = 0;
    // in word line mode and for Arabic, we disable the half space trick:
    const double nHalfSpace = bNoHalfSpace ? 0 : nSpaceAdd / 2.0;
    const double nOtherHalf = nSpaceAdd - nHalfSpace;
    tools::Long nKernSum = nKern;
    sal_Unicode cChPrev = aText[nStt];

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

        sal_Unicode nCh = aText[nStt + i];

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
            rKernArray[nPrevIdx] += -nSpaceAdd;

        // Advance nPrevIdx and assign kern values to previous cluster.
        for (double nValue = rKernArray[nPrevIdx++]; nPrevIdx < i; ++nPrevIdx)
            rKernArray[nPrevIdx] = nValue;
    }

    // the layout engine requires the total width of the output
    while (nPrevIdx < nLen)
    {
        rKernArray[nPrevIdx] += nKernSum + nSpaceSum;
        ++nPrevIdx;
    }
}

tools::Long SnapToGrid(KernArray& rKernArray, std::u16string_view aText, sal_Int32 nStt,
                       sal_Int32 nLen, tools::Long nGridWidth, bool bForceLeft)
{
    assert(nStt + nLen <= sal_Int32(aText.size()));
    assert(nLen <= sal_Int32(rKernArray.size()));

    tools::Long nCharWidth = rKernArray[0];
    tools::Long nMinWidth = lcl_MinGridWidth(nGridWidth, nCharWidth);
    tools::Long nDelta = lcl_OffsetFromGridEdge(nMinWidth, nCharWidth, aText[nStt], bForceLeft);
    tools::Long nEdge = nMinWidth - nDelta;

    sal_Int32 nLast = 0;

    for (sal_Int32 i = 1; i < nLen; ++i)
    {
        if (rKernArray[i] == rKernArray[nLast])
            continue;

        nCharWidth = rKernArray[i] - rKernArray[nLast];
        nMinWidth = lcl_MinGridWidth(nGridWidth, nCharWidth);
        tools::Long nX
            = nEdge + lcl_OffsetFromGridEdge(nMinWidth, nCharWidth, aText[nStt + i], bForceLeft);
        nEdge += nMinWidth;

        while (nLast < i)
        {
            rKernArray[nLast] = nX;
            ++nLast;
        }
    }

    while (nLast < nLen)
    {
        rKernArray[nLast] = nEdge;
        ++nLast;
    }

    return nDelta;
}

namespace
{
tools::Long lcl_MsoGridWidth(tools::Long nGridWidth, tools::Long nBaseFontSize,
                             tools::Long nCellSize)
{
    return nCellSize + (nGridWidth - nBaseFontSize);
}

void lcl_MsoCompatSnapToGridEdge(KernArray& rKernArray, sal_Int32 nLen, tools::Long nGridWidth,
                                 tools::Long nSpace, tools::Long nKern, tools::Long nBaseFontSize)
{
    tools::Long nCharWidth = rKernArray[0];
    tools::Long nEdge = lcl_MsoGridWidth(nGridWidth, nBaseFontSize, nCharWidth + nKern) + nSpace;

    sal_Int32 nLast = 0;

    for (sal_Int32 i = 1; i < nLen; ++i)
    {
        if (rKernArray[i] == rKernArray[nLast])
            continue;

        nCharWidth = rKernArray[i] - rKernArray[nLast];
        tools::Long nMinWidth = lcl_MsoGridWidth(nGridWidth, nBaseFontSize, nCharWidth + nKern);
        while (nLast < i)
        {
            rKernArray[nLast] = nEdge;
            ++nLast;
        }

        nEdge += nMinWidth + nSpace;
    }

    while (nLast < nLen)
    {
        rKernArray[nLast] = nEdge;
        ++nLast;
    }
}
}

void SnapToGridEdge(KernArray& rKernArray, sal_Int32 nLen, tools::Long nGridWidth,
                    tools::Long nSpace, tools::Long nKern, tools::Long nBaseFontSize,
                    bool bUseMsoCompatibleGrid)
{
    assert(nLen <= sal_Int32(rKernArray.size()));

    if (bUseMsoCompatibleGrid)
    {
        lcl_MsoCompatSnapToGridEdge(rKernArray, nLen, nGridWidth, nSpace, nKern, nBaseFontSize);
        return;
    }

    tools::Long nCharWidth = rKernArray[0];
    tools::Long nEdge = lcl_MinGridWidth(nGridWidth, nCharWidth + nKern) + nSpace;

    sal_Int32 nLast = 0;

    for (sal_Int32 i = 1; i < nLen; ++i)
    {
        if (rKernArray[i] == rKernArray[nLast])
            continue;

        nCharWidth = rKernArray[i] - rKernArray[nLast];
        tools::Long nMinWidth = lcl_MinGridWidth(nGridWidth, nCharWidth + nKern);
        while (nLast < i)
        {
            rKernArray[nLast] = nEdge;
            ++nLast;
        }

        nEdge += nMinWidth + nSpace;
    }

    while (nLast < nLen)
    {
        rKernArray[nLast] = nEdge;
        ++nLast;
    }
}

bool KashidaJustify(std::span<TextFrameIndex const> aKashPositions, KernArray& rKernArray,
                    sal_Bool* pKashidaArray, sal_Int32 nStt, sal_Int32 nLen, tools::Long nSpaceAdd)
{
    SAL_WARN_IF(!nLen, "sw.core", "Kashida justification without text?!");

    auto stKashPosIt = aKashPositions.begin();

    tools::Long nKashAdd = 0;
    bool bHasAnyKashida = false;
    for (sal_Int32 nIdx = 0; nIdx < nLen; ++nIdx)
    {
        bool bInsert = false;
        while (stKashPosIt != aKashPositions.end())
        {
            auto nRelKashIdx = static_cast<sal_Int32>(*stKashPosIt) - nStt;
            bInsert = (nRelKashIdx == nIdx);

            if (nRelKashIdx >= nIdx)
                break;

            ++stKashPosIt;
        }

        if (bInsert)
        {
            if (pKashidaArray)
            {
                pKashidaArray[nIdx] = true;
            }

            nKashAdd += nSpaceAdd;
            bHasAnyKashida = true;
        }

        rKernArray[nIdx] += nKashAdd;
    }

    return bHasAnyKashida;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
