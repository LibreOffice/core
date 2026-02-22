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

#include <memory>

#include <svx/hlnkitem.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/childwin.hxx>
#include <vcl/weld/weld.hxx>

class HyperlinkDialog : public SfxTabDialogController
{
private:
    std::unique_ptr<SfxItemSet> m_pInputSet;
    bool m_bInitialized = false;

    void Apply();
    DECL_LINK(ClickApplyHdl, weld::Button&, void);
    void CheckAndInitializeHyperlinkData();
    static OUString DetermineTabPageId(const SvxHyperlinkItem* pHyperItem);
    void Activate() override;
    void PageCreated(const OUString&, SfxTabPage& rTabPage) override;
    virtual short Ok() override;

protected:
    virtual SfxItemSet* CreateInputItemSet(const OUString& rName) override;

public:
    HyperlinkDialog(weld::Window* pParent, SfxChildWindow* pChildWindow);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
