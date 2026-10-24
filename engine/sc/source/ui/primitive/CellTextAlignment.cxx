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

#include <CellTextAlignment.hxx>

#include <attrib.hxx>
#include <document.hxx>
#include <global.hxx>
#include <patattr.hxx>
#include <scitems.hxx>

#include <com/sun/star/i18n/DirectionProperty.hpp>
#include <editeng/frmdiritem.hxx>
#include <svl/numformat.hxx>
#include <unotools/charclass.hxx>

using namespace css;

namespace
{
bool beginsWithRTLCharacter(const OUString& rStr)
{
    if (rStr.isEmpty())
        return false;

    switch (ScGlobal::getCharClass().getCharacterDirection(rStr, 0))
    {
        case i18n::DirectionProperty_RIGHT_TO_LEFT:
        case i18n::DirectionProperty_RIGHT_TO_LEFT_ARABIC:
        case i18n::DirectionProperty_RIGHT_TO_LEFT_EMBEDDING:
        case i18n::DirectionProperty_RIGHT_TO_LEFT_OVERRIDE:
            return true;
        default:;
    }

    return false;
}
}

namespace sc
{
SvxCellHorJustify getAlignmentFromContext(SvxCellHorJustify eInHorJust, bool bCellIsValue,
                                          bool bCellIsCallable, const OUString& rText,
                                          const ScPatternAttr& rPattern, const SfxItemSet* pCondSet,
                                          const ScDocument& rDocument, SCTAB nTab,
                                          bool bNumberFormatIsText)
{
    SvxCellHorJustify eHorJustContext = eInHorJust;
    bool bUseWritingDirection = false;
    if (eInHorJust == SvxCellHorJustify::Standard)
    {
        // fdo#32530: Default alignment depends on value vs
        // string, and the direction of the 1st letter.
        if (beginsWithRTLCharacter(rText)) //If language is RTL
        {
            if (bCellIsValue)
                eHorJustContext
                    = bNumberFormatIsText ? SvxCellHorJustify::Right : SvxCellHorJustify::Left;
            else
                eHorJustContext = SvxCellHorJustify::Right;
        }
        else if (bCellIsValue) //If language is not RTL
            eHorJustContext
                = bNumberFormatIsText ? SvxCellHorJustify::Left : SvxCellHorJustify::Right;
        else if (bCellIsCallable)
            eHorJustContext = SvxCellHorJustify::Center;
        else
            bUseWritingDirection = true;
    }

    if (bUseWritingDirection || eInHorJust == SvxCellHorJustify::Block
        || eInHorJust == SvxCellHorJustify::Repeat)
    {
        SvxFrameDirection nDirection = rPattern.GetItem(ATTR_WRITINGDIR, pCondSet).GetValue();
        if (nDirection == SvxFrameDirection::Horizontal_LR_TB
            || nDirection == SvxFrameDirection::Vertical_LR_TB)
            eHorJustContext = SvxCellHorJustify::Left;
        else if (nDirection == SvxFrameDirection::Environment)
        {
            // fdo#73588: The content of the cell must also
            // begin with a RTL character to be right
            // aligned; otherwise, it should be left aligned.
            eHorJustContext = (rDocument.IsLayoutRTL(nTab) && beginsWithRTLCharacter(rText))
                                  ? SvxCellHorJustify::Right
                                  : SvxCellHorJustify::Left;
        }
        else
            eHorJustContext = SvxCellHorJustify::Right;
    }
    return eHorJustContext;
}

bool isNumberFormatText(const ScDocument& rDocument, SCCOL nColumn, SCROW nRow, SCTAB nTab)
{
    sal_uInt32 nCurrentNumberFormat = rDocument.GetNumberFormat(nColumn, nRow, nTab);
    SvNumberFormatter* pNumberFormatter = rDocument.GetFormatTable();
    return pNumberFormatter->GetType(nCurrentNumberFormat) == SvNumFormatType::TEXT;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
