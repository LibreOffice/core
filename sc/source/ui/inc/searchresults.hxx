/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/weld.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/childwin.hxx>

class ScDocument;
class ScRangeList;

namespace sc {

class SearchResultsDlg : public SfxDialogController
{
    OUString aSkipped;
    SfxBindings* mpBindings;
    ScDocument* mpDoc;
    bool mbSorted;
    std::unique_ptr<weld::TreeView> mxList;
    std::unique_ptr<weld::Label> mxSearchResults;
    std::unique_ptr<weld::CheckButton> mxShowDialog;

    DECL_LINK(ListSelectHdl, weld::TreeView&, void);
    DECL_LINK(HeaderBarClick, int, void);
    DECL_STATIC_LINK(SearchResultsDlg, OnShowToggled, weld::ToggleButton&, void);
public:
    SearchResultsDlg(SfxBindings* _pBindings, weld::Window* pParent);
    virtual ~SearchResultsDlg() override;

    virtual void Close() override;

    void FillResults( ScDocument& rDoc, const ScRangeList& rMatchedRanges, bool bCellNotes );
};

class SearchResultsDlgWrapper : public SfxChildWindow
{
    std::shared_ptr<SearchResultsDlg> m_xDialog;
public:
    SearchResultsDlgWrapper(
        vcl::Window* _pParent, sal_uInt16 nId, SfxBindings* pBindings, SfxChildWinInfo* pInfo );

    virtual ~SearchResultsDlgWrapper() override;

    SFX_DECL_CHILDWINDOW_WITHID(SearchResultsDlgWrapper);
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
