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
#include <editutil.hxx>
#include <global.hxx>
#include <patattr.hxx>
#include <scitems.hxx>

#include <com/sun/star/i18n/DirectionProperty.hpp>
#include <editeng/adjustitem.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/frmdiritem.hxx>
#include <editeng/justifyitem.hxx>
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

bool hasEditCharacters(std::u16string_view rText, bool bHasRepeatCharacter)
{
    for (const sal_Unicode cCharacter : rText)
    {
        switch (cCharacter)
        {
            case CHAR_NBSP:
                // tdf#122676: CHAR_NBSP is a thousands separator of some number formats.
                if (!bHasRepeatCharacter)
                    return true;
                break;
            case CHAR_SHY:
            case CHAR_ZWSP:
            case CHAR_LRM:
            case CHAR_RLM:
            case CHAR_NBHY:
            case CHAR_WJ:
                return true;
            default:
                break;
        }
    }
    return false;
}

void setVerticalTextAlignment(ScEditEngineDefaulter& rEngine, bool bTopToBottom,
                              SvxCellVerJustify eVerJust, SvxCellJustifyMethod eVerJustMethod,
                              SvxCellHorJustify eHorJust)
{
    SvxAdjust eSvxAdjust = SvxAdjust::Left;
    switch (eVerJust)
    {
        case SvxCellVerJustify::Top:
            eSvxAdjust = bTopToBottom ? SvxAdjust::Left : SvxAdjust::Right;
            break;
        case SvxCellVerJustify::Center:
            eSvxAdjust = SvxAdjust::Center;
            break;
        case SvxCellVerJustify::Bottom:
        case SvxCellVerJustify::Standard:
            eSvxAdjust = bTopToBottom ? SvxAdjust::Right : SvxAdjust::Left;
            break;
        case SvxCellVerJustify::Block:
            eSvxAdjust = SvxAdjust::Block;
            break;
    }

    rEngine.SetDefaultItem(SvxAdjustItem(eSvxAdjust, EE_PARA_JUST));
    rEngine.SetDefaultItem(SvxJustifyMethodItem(eVerJustMethod, EE_PARA_JUST_METHOD));

    if (eHorJust == SvxCellHorJustify::Block)
        rEngine.SetDefaultItem(SvxVerJustifyItem(SvxCellVerJustify::Block, EE_PARA_VER_JUST));
}

void normalizeTextRotation(Degree100& rRotate, SvxRotateMode& rRotateMode)
{
    // tdf#143377 To use the same limits to avoid too big Skew
    // with TextOrientation in Calc, use 1/2 degree here, too.
    // This equals '50' in the notation here (100th degree)
    static const sal_Int32 nMinRad(50);

    // bring the rotation to the range [0..36000[
    rRotate = Degree100(((rRotate.get() % 36000) + 36000) % 36000);

    // check for to be avoided extreme values and correct
    if (rRotate < Degree100(nMinRad))
    {
        // range [0..50]
        rRotate = Degree100(nMinRad);
        rRotateMode = SVX_ROTATE_MODE_STANDARD; // no overflow
    }
    else if (rRotate > Degree100(36000 - nMinRad))
    {
        // range [35950..36000[
        rRotate = Degree100(36000 - nMinRad);
        rRotateMode = SVX_ROTATE_MODE_STANDARD; // no overflow
    }
    else if (rRotate > Degree100(18000 - nMinRad) && (rRotate < Degree100(18000 + nMinRad)))
    {
        // range 50 around 18000, [17950..18050]
        rRotate = (rRotate > Degree100(18000)) ? Degree100(18000 + nMinRad)
                                               : Degree100(18000 - nMinRad);
        rRotateMode = SVX_ROTATE_MODE_STANDARD; // no overflow
    }
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
