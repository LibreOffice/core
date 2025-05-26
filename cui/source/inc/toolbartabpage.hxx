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

#include <com/sun/star/frame/XLayoutManager.hpp>

#include <sfx2/tabdlg.hxx>
class ToolbarTabPage : public SfxTabPage
{
private:
    css::uno::Reference<css::frame::XLayoutManager> m_xLayoutManager;
    std::unique_ptr<weld::TreeView> m_pToolbarList;
    virtual void Reset(const SfxItemSet* /* rSet*/) override;
    DECL_LINK(ToggleHdl, const weld::TreeView::iter_col&, void);

public:
    ToolbarTabPage(weld::Container* pPage, weld::DialogController* pController,
                   const SfxItemSet& rSet);
    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pPage, weld::DialogController* pController, const SfxItemSet* rSet);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
