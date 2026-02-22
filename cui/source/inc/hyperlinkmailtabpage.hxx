/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/weld/weld.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/hlnkitem.hxx>
#include "hyperlinktabpagebase.hxx"

class HyperlinkMailTabPage final : public HyperlinkTabPageBase
{
private:
    std::unique_ptr<SvxHyperURLBox> m_xCbbReceiver;
    std::unique_ptr<weld::Button> m_xBtAdrBook;
    std::unique_ptr<weld::Entry> m_xEdSubject;

    DECL_STATIC_LINK(HyperlinkMailTabPage, ClickAdrBookHdl_Impl, weld::Button&, void);
    DECL_LINK(ModifiedReceiverHdl_Impl, weld::ComboBox&, void);

    virtual void FillDlgFields(const OUString& rStrURL) override;
    virtual void GetCurrentItemData(OUString& rStrURL, OUString& aStrName, OUString& aStrIntName,
                                    OUString& aStrFrame, SvxLinkInsertMode& eMode) override;
    virtual void ClearPageSpecificControls() override;

    OUString CreateAbsoluteURL() const;
    virtual void SetInitFocus() override;
    void SetScheme(std::u16string_view rScheme);
    void RemoveImproperProtocol(std::u16string_view aProperScheme);

public:
    HyperlinkMailTabPage(weld::Container* pParent, weld::DialogController* pController,
                         const SfxItemSet* pSet);

    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pParent, weld::DialogController* pController, const SfxItemSet* pSet);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
