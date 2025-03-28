/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "FrameControl.hxx"

class SwEditWin;
class SwContentControl;

/// In case the content control has an alias/title, this widget shows it above the top left corner
/// of the content control till the cursor is inside the content control.
class SwContentControlAliasButton final : public SwFrameMenuButtonBase
{
    std::unique_ptr<weld::Button> m_xPushButton;
    OUString m_sLabel;
    bool m_bReadOnly = false;

public:
    SwContentControlAliasButton(SwEditWin* pEditWin, const SwContentControl* pContentControl);
    ~SwContentControlAliasButton() override;

    bool Contains(const Point& rDocPt) const override;
    void SetReadonly(bool bReadonly) override;
    void ShowAll(bool bShow) override;
    void dispose() override;

    void SetContentControl(const SwContentControl* pContentControl);
    void SetOffset(Point aTopLeftPixel);

private:
    DECL_LINK(ClickHdl, weld::Button&, void);
    void PaintButton();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
