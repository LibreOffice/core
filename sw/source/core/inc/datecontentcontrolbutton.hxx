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
class SvNumberFormatter;
class SwContentControl;

/**
 * This button is shown when the cursor is on a date content control.  The user can select a date
 * from a date picker.
 */
class SwDateContentControlButton final : public SwContentControlButton
{
private:
    SvNumberFormatter* m_pNumberFormatter;

    std::unique_ptr<weld::Calendar> m_xCalendar;

    DECL_LINK(SelectHandler, weld::Calendar&, void);

public:
    SwDateContentControlButton(SwEditWin* pEditWin,
                               const std::shared_ptr<SwContentControl>& pContentControl,
                               SvNumberFormatter* pNumberFormatter);
    ~SwDateContentControlButton() override;

    void LaunchPopup() override;
    void DestroyPopup() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
