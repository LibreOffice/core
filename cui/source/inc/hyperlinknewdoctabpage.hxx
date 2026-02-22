/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <tools/urlobj.hxx>
#include <vcl/weld/weld.hxx>
#include <vcl/weld/TreeView.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/tabdlg.hxx>
#include "hyperlinktabpagebase.hxx"

class HyperlinkDocNewTabPage final : public HyperlinkTabPageBase
{
private:
    std::unique_ptr<weld::RadioButton> m_xRbtEditNow;
    std::unique_ptr<weld::RadioButton> m_xRbtEditLater;
    std::unique_ptr<SvxHyperURLBox> m_xCbbPath;
    std::unique_ptr<weld::Button> m_xBtCreate;
    std::unique_ptr<weld::TreeView> m_xLbDocTypes;

    bool ImplGetURLObject(const OUString& rPath, std::u16string_view rBase,
                          INetURLObject& aURLObject) const;
    void FillDocumentList();

    DECL_LINK(ClickNewHdl_Impl, weld::Button&, void);
    DECL_STATIC_LINK(HyperlinkDocNewTabPage, DispatchDocument, void*, void);

    virtual void FillDlgFields(const OUString& rStrURL) override;
    virtual void GetCurrentItemData(OUString& rStrURL, OUString& aStrName, OUString& aStrIntName,
                                    OUString& aStrFrame, SvxLinkInsertMode& eMode) override;
    virtual void ClearPageSpecificControls() override;

public:
    HyperlinkDocNewTabPage(weld::Container* pParent, weld::DialogController* pController,
                           const SfxItemSet* pSet);
    virtual ~HyperlinkDocNewTabPage() override;

    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pParent, weld::DialogController* pController, const SfxItemSet* pSet);

    virtual void SetInitFocus() override;
    void DoApply();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
