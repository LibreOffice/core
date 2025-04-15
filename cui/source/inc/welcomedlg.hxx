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
    std::unique_ptr<weld::Button> m_xOKBtn; // release notes / apply
    std::unique_ptr<weld::Button> m_xResetBtn; // hidden
    std::unique_ptr<weld::Button> m_xCancelBtn; // next / close
    DECL_LINK(OnActivatePage, const OUString&, void);
    DECL_LINK(OnApplyClick, weld::Button&, void);
    DECL_LINK(OnNextClick, weld::Button&, void);

public:
    WelcomeDialog(weld::Window* pParent);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
