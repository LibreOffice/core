/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "contentcontrolbutton.hxx"

class SwEditWin;
class SwContentControl;

/**
 * This button is shown when the cursor is inside a drop-down content control.
 * The user can select a list item using this button while filling in a form.
 */
class SwDropDownContentControlButton final : public SwContentControlButton
{
private:
    std::unique_ptr<weld::TreeView> m_xTreeView;

    DECL_LINK(ListBoxHandler, weld::TreeView&, bool);

    void InitDropdown();

public:
    SwDropDownContentControlButton(SwEditWin* pEditWin,
                                   const std::shared_ptr<SwContentControl>& pContentControl);
    virtual ~SwDropDownContentControlButton() override;

    virtual void LaunchPopup() override;
    virtual void DestroyPopup() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
