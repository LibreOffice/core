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

#include <address.hxx>
#include <editeng/svxenum.hxx>
#include <rtl/ustring.hxx>
#include <svx/rotmodit.hxx>
#include <tools/degree.hxx>
#include <tools/long.hxx>

#include <cmath>
#include <string_view>
#include <utility>

class ScDocument;
class ScEditEngineDefaulter;
class ScPatternAttr;
class SfxItemSet;

namespace sc
{
/** Get left, right or centered alignment from the right-to-left context. Standard, block and repeat
    resolve to the contextual left or right.
 */
SvxCellHorJustify getAlignmentFromContext(SvxCellHorJustify eInHorJust, bool bCellIsValue,
                                          bool bCellIsCallable, const OUString& rText,
                                          const ScPatternAttr& rPattern, const SfxItemSet* pCondSet,
                                          const ScDocument& rDocument, SCTAB nTab,
                                          bool bNumberFormatIsText);

/// Whether the number format of the cell is a text format.
bool isNumberFormatText(const ScDocument& rDocument, SCCOL nColumn, SCROW nRow, SCTAB nTab);

/** Whether the text holds characters that only the EditEngine lays out: soft hyphen, zero width
    space, direction marks, no-break hyphen and word joiner. The no-break space counts only without
    a repeat character, because it is the thousands separator of some number formats.
 */
bool hasEditCharacters(std::u16string_view rText, bool bHasRepeatCharacter);

/** Set the paragraph alignment of text whose lines run down the cell: text turned to read from top
    to bottom (bTopToBottom) or from bottom to top, and Asian vertical text, which reads from top
    to bottom. The vertical alignment of the cell places the lines.
 */
void setVerticalTextAlignment(ScEditEngineDefaulter& rEngine, bool bTopToBottom,
                              SvxCellVerJustify eVerJust, SvxCellJustifyMethod eVerJustMethod,
                              SvxCellHorJustify eHorJust);

/// Asian vertical text breaks its columns this many pixels above the bottom of the cell, or the
/// text would reach out of the cell. The value is a heuristic.
constexpr tools::Long constAsianVerticalBottomGap = 5;

/** Where stacked text starts across its cell, in the units of the arguments. Stacked text that
    wraps starts at the left margin. Otherwise the text, nNeeded wide with both margins, moves by
    the alignment of the cell. nOnePixel is the size of a device pixel.
 */
template <typename T>
T stackedTextStartX(T nAlignLeft, T nCellWidth, T nNeeded, T nLeftMargin, T nRightMargin,
                    SvxCellHorJustify eHorJust, bool bBreak, T nOnePixel)
{
    if (bBreak)
        return nAlignLeft + nLeftMargin;
    if (eHorJust == SvxCellHorJustify::Right)
        return nAlignLeft - (nNeeded - nCellWidth + nRightMargin + nOnePixel);
    if (eHorJust == SvxCellHorJustify::Center)
        return nAlignLeft - (nNeeded - nCellWidth + nRightMargin + nOnePixel - nLeftMargin) / 2;
    return nAlignLeft + nLeftMargin;
}

/** Bring the rotation of cell text into [0..36000[ and away from the angles whose sine is zero,
    which the placement of rotated text divides by. Text turned that little overflows nothing, so
    its rotate mode becomes the standard one.
 */
void normalizeTextRotation(Degree100& rRotate, SvxRotateMode& rRotateMode);

/** The width and the height of the box that text nWidth wide and nHeight high takes in its cell
    when it is turned by an angle with the given sine and cosine. Text that leans with its cell
    takes the width of one slanted line.
 */
template <typename T>
std::pair<T, T> rotatedTextExtent(T nWidth, T nHeight, double fSin, double fCos,
                                  SvxRotateMode eRotateMode)
{
    const double fAbsSin = std::fabs(fSin);
    const double fAbsCos = std::fabs(fCos);
    const T nExtentWidth = eRotateMode == SVX_ROTATE_MODE_STANDARD
                               ? static_cast<T>(nWidth * fAbsCos + nHeight * fAbsSin)
                               : static_cast<T>(nHeight / fAbsSin);
    return { nExtentWidth, static_cast<T>(nHeight * fAbsCos + nWidth * fAbsSin) };
}

/// How far the box of rotated text starts from the left of the cell, by the alignment of the cell.
/// Rotated text is centered by default.
template <typename T>
T rotatedTextAlignX(SvxCellHorJustify eHorJust, T nAvailWidth, T nExtentWidth)
{
    if (eHorJust == SvxCellHorJustify::Right)
        return nAvailWidth - nExtentWidth;
    if (eHorJust == SvxCellHorJustify::Center || eHorJust == SvxCellHorJustify::Standard)
        return (nAvailWidth - nExtentWidth) / 2;
    return 0;
}

/// How far the box of rotated text starts from the top of the cell, by the alignment of the cell.
template <typename T>
T rotatedTextAlignY(SvxCellVerJustify eVerJust, T nCellHeight, T nExtentHeight)
{
    if (eVerJust == SvxCellVerJustify::Bottom || eVerJust == SvxCellVerJustify::Standard)
        return nCellHeight - nExtentHeight;
    if (eVerJust == SvxCellVerJustify::Center)
        return (nCellHeight - nExtentHeight) / 2;
    return 0;
}

/** Where rotated text starts within its box, as an offset across and down. The text turns around
    its start, so the start moves by the turned text to keep it in the box. Text that leans with its
    cell also moves along the slant by the vertical alignment. nCellHeight is the height of the
    cell within its margins and nTotalHeight with them.
 */
template <typename T>
std::pair<double, double> rotatedTextOffset(T nWidth, T nHeight, double fSin, double fCos,
                                            SvxRotateMode eRotateMode, SvxCellVerJustify eVerJust,
                                            T nCellHeight, T nExtentHeight, T nTotalHeight)
{
    double fAddX = 0.0;
    double fAddY = 0.0;
    if (fCos > 0.0 && eRotateMode != SVX_ROTATE_MODE_STANDARD)
        fAddX += nHeight * fCos * (fCos / std::fabs(fSin));
    if (fCos < 0.0 && eRotateMode == SVX_ROTATE_MODE_STANDARD)
        fAddX -= nWidth * fCos;
    if (fSin < 0.0)
        fAddX -= nHeight * fSin;
    if (fSin > 0.0)
        fAddY += nWidth * fSin;
    if (fCos < 0.0)
        fAddY -= nHeight * fCos;

    if (eRotateMode != SVX_ROTATE_MODE_STANDARD)
    {
        const double fSkew = nTotalHeight * fCos / std::fabs(fSin);
        if (eRotateMode == SVX_ROTATE_MODE_CENTER)
            fAddX -= fSkew * 0.5;
        if ((eRotateMode == SVX_ROTATE_MODE_TOP && fSin > 0.0)
            || (eRotateMode == SVX_ROTATE_MODE_BOTTOM && fSin < 0.0))
            fAddX -= fSkew;

        T nUp = 0;
        if (eVerJust == SvxCellVerJustify::Center)
            nUp = (nCellHeight - nExtentHeight) / 2;
        else if (eVerJust == SvxCellVerJustify::Top)
        {
            if (fSin > 0.0)
                nUp = nCellHeight - nExtentHeight;
        }
        else if (fSin < 0.0)
            nUp = nCellHeight - nExtentHeight;
        if (nUp)
            fAddX += nUp * fCos / std::fabs(fSin);
    }
    return { fAddX, fAddY };
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
