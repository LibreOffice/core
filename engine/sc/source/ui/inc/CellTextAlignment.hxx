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
#include <tools/long.hxx>

#include <string_view>

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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
