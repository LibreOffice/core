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

#include <uimode.hrc>

class UITabPage : public SfxTabPage
{
private:
    std::unique_ptr<weld::Image> m_pImage;
    std::unique_ptr<weld::RadioButton> m_pRadioButtons[std::size(UIMODES_ARRAY)];
    std::unique_ptr<weld::Label> m_pInfoLabel;

    void UpdateImage(std::u16string_view sFileName);
    static OUString GetCurrentMode();
    int GetActiveRadioButton();
    DECL_LINK(SelectUIMode, weld::Toggleable&, void);

public:
    UITabPage(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet);

    OUString GetSelectedMode();
    static OUString GetCurrentApp();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
