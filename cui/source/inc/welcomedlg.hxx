/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <sfx2/tabdlg.hxx>

class WelcomeDialog : public SfxTabDialogController
{
private:
    bool m_bFirstStart;

    std::unique_ptr<weld::Button> m_xActionBtn; // release notes / apply
    std::unique_ptr<weld::Button> m_xNextBtn; // next / close
    std::unique_ptr<weld::Button> m_xOKBtn; // hidden
    std::unique_ptr<weld::Button> m_xResetBtn; // hidden
    std::unique_ptr<weld::Button> m_xCancelBtn; // hidden
    std::unique_ptr<weld::CheckButton> m_xShowAgain;

    virtual void PageCreated(const OUString& rId, SfxTabPage& rPage) override;

    DECL_LINK(OnActivatePage, const OUString&, void);
    DECL_LINK(OnActionClick, weld::Button&, void);
    DECL_LINK(OnNextClick, weld::Button&, void);

public:
    WelcomeDialog(weld::Window* pParent, const bool bIsFirstStart);
    virtual ~WelcomeDialog() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
