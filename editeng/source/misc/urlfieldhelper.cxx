/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <editeng/urlfieldhelper.hxx>

#include <editeng/flditem.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>

void URLFieldHelper::RemoveURLField(EditView& pEditView)
{
    pEditView.SelectFieldAtCursor();
    const SvxFieldItem* pFieldItem = pEditView.GetFieldAtSelection();
    const SvxFieldData* pField = pFieldItem ? pFieldItem->GetField() : nullptr;
    if (auto pUrlField = dynamic_cast<const SvxURLField*>(pField))
    {
        ESelection aSel = pEditView.GetSelection();
        pEditView.getEditEngine().QuickInsertText(pUrlField->GetRepresentation(), aSel);
        pEditView.Invalidate();
    }
}

bool URLFieldHelper::IsCursorAtURLField(const EditView& pEditView, bool bAlsoCheckBeforeCursor)
{
    // tdf#128666 Make sure only URL field (or nothing) is selected
    ESelection aSel = pEditView.GetSelection();
    if (aSel.start.nPara != aSel.end.nPara || std::abs(aSel.end.nIndex - aSel.start.nIndex) > 1)
        return false;

    const SvxFieldData* pField
        = pEditView.GetFieldUnderMouseOrInSelectionOrAtCursor(bAlsoCheckBeforeCursor);
    if (dynamic_cast<const SvxURLField*>(pField))
        return true;

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
