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
#include <vcl/timer.hxx>
#include "hyperlinktabpagebase.hxx"

class HyperlinkInternetTP final : public HyperlinkTabPageBase
{
private:
    std::unique_ptr<SvxHyperURLBox> m_xCbbTarget;

    DECL_LINK(ModifiedTargetHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(LostFocusTargetHdl_Impl, weld::Widget&, void);
    DECL_LINK(TimeoutHdl_Impl, Timer*, void);

    OUString CreateAbsoluteURL() const;

    void SetScheme(std::u16string_view rScheme);
    void RemoveImproperProtocol(std::u16string_view aProperScheme);
    static INetProtocol GetSmartProtocolFromButtons();

    void RefreshMarkWindow();

protected:
    virtual void FillDlgFields(const OUString& rStrURL) override;
    virtual void GetCurrentItemData(OUString& rStrURL, OUString& aStrName, OUString& aStrIntName,
                                    OUString& aStrFrame, SvxLinkInsertMode& eMode) override;
    virtual void ClearPageSpecificControls() override;

    virtual bool ShouldOpenMarkWnd() override { return false; }
    virtual void SetMarkWndShouldOpen(bool bOpen) override { m_bMarkWndOpen = bOpen; }

public:
    HyperlinkInternetTP(weld::Container* pParent, weld::DialogController* pController,
                        const SfxItemSet* pSet);

    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pParent, weld::DialogController* pController, const SfxItemSet* pSet);

    virtual void SetInitFocus() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
