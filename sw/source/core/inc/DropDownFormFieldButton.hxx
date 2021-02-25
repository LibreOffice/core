/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "FormFieldButton.hxx"

class SwEditWin;
namespace sw::mark
{
class DropDownFieldmark;
}

/**
 * This button is shown when the cursor is on a drop-down form field.
 * The user can select an item of the field using this button while filling in a form.
 */
class DropDownFormFieldButton : public FormFieldButton
{
private:
    std::unique_ptr<weld::TreeView> m_xTreeView;

    DECL_LINK(MyListBoxHandler, weld::TreeView&, bool);

    void InitDropdown();

public:
    DropDownFormFieldButton(SwEditWin* pEditWin, sw::mark::DropDownFieldmark& rFieldMark);
    virtual ~DropDownFormFieldButton() override;

    virtual void LaunchPopup() override;
    virtual void DestroyPopup() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
