/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <editeng/urlfieldhelper.hxx>

#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/editview.hxx>
#include <editeng/editeng.hxx>

void URLFieldHelper::RemoveURLField(EditView& pEditView)
{
    pEditView.SelectFieldAtCursor();
    const std::optional<SvxFieldItem> oFieldItem = pEditView.GetFieldAtSelection();
    const SvxFieldData* pField = oFieldItem ? oFieldItem->GetField() : nullptr;
    auto pUrlField = dynamic_cast<const SvxURLField*>(pField);
    if (!pUrlField)
        return;

    ESelection aSel = pEditView.GetSelection();
    if (oFieldItem->Which() == EE_CHAR_URL)
    {
        // The link text is real text, so only the link attribute goes.
        pEditView.getEditEngine().RemoveAttribs(aSel, false, EE_CHAR_URL);
    }
    else
    {
        // Replace the placeholder field with its text.
        pEditView.getEditEngine().QuickInsertText(pUrlField->GetRepresentation(), aSel);
    }
    pEditView.Invalidate();
}

bool URLFieldHelper::IsCursorAtURLField(const EditView& pEditView, bool bAlsoCheckBeforeCursor)
{
    // tdf#128666 Make sure only URL field (or nothing) is selected
    ESelection aSel = pEditView.GetSelection();
    if (aSel.start.nPara != aSel.end.nPara || std::abs(aSel.end.nIndex - aSel.start.nIndex) > 1)
        return false;

    const std::optional<SvxFieldItem> oFieldItem
        = pEditView.GetFieldAtSelection(bAlsoCheckBeforeCursor);
    if (dynamic_cast<const SvxURLField*>(oFieldItem ? oFieldItem->GetField() : nullptr))
        return true;

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
