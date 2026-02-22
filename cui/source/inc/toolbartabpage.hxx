/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#pragma once

#include <sal/config.h>

#include <framework/layoutmanager.hxx>

#include <sfx2/tabdlg.hxx>
#include <vcl/weld/weld.hxx>
#include <vcl/weld/TreeView.hxx>

class ToolbarTabPage : public SfxTabPage
{
private:
    rtl::Reference<framework::LayoutManager> m_xLayoutManager;
    std::unique_ptr<weld::TreeView> m_pToolbarList;
    std::unique_ptr<weld::Label> m_pLockAllLabel;
    std::unique_ptr<weld::CheckButton> m_pLockAll;
    virtual void Reset(const SfxItemSet* /* rSet*/) override;

    void ShowToolbar(const OUString& sName, const bool bShow);
    void LockToolbar(const OUString& sName, const bool bLock);
    void SensitiveToolbar(const OUString& sName, const bool bSensitive);
    void UpdateAllLocked();

    DECL_LINK(ToggleHdl, const weld::TreeView::iter_col&, void);
    DECL_LINK(CheckBoxHdl, weld::Toggleable&, void);

public:
    ToolbarTabPage(weld::Container* pPage, weld::DialogController* pController,
                   const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
