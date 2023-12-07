/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#pragma once
#include <sfx2/sidebar/PanelLayout.hxx>
#include <wrtsh.hxx>

namespace sw::sidebar
{
class QuickFindPanel : public PanelLayout
{
public:
    static std::unique_ptr<PanelLayout> Create(weld::Widget* pParent);

    QuickFindPanel(weld::Widget* pParent);
    virtual ~QuickFindPanel() override;

private:
    std::unique_ptr<weld::Entry> m_xSearchFindEntry;
    std::unique_ptr<weld::TreeView> m_xSearchFindsList;
    std::vector<std::unique_ptr<SwPaM>> m_vPaMs;
    int m_nRowHeight;

    SwWrtShell* m_pWrtShell;

    DECL_LINK(SearchFindEntryActivateHandler, weld::Entry&, bool);
    DECL_LINK(SearchFindsListCustomGetSizeHandler, weld::TreeView::get_size_args, Size);
    DECL_LINK(SearchFindsListRender, weld::TreeView::render_args, void);
    DECL_LINK(SearchFindsListSelectionChangedHandler, weld::TreeView&, void);
    DECL_LINK(SearchFindEntryChangedHandler, weld::Entry&, void);
    DECL_LINK(SearchFindsListRowActivatedHandler, weld::TreeView&, bool);
    void FillSearchFindsList();
};
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
