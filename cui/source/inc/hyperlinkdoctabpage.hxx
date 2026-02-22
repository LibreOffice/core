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

class HyperlinkDocTabPage final : public HyperlinkTabPageBase
{
private:
    std::unique_ptr<SvxHyperURLBox> m_xCbbPath;
    std::unique_ptr<weld::Button> m_xBtFileopen;

    std::unique_ptr<weld::Entry> m_xEdTarget;
    std::unique_ptr<weld::Label> m_xFtFullURL;
    std::unique_ptr<weld::Button> m_xBtBrowse;

    OUString maStrURL;

    DECL_LINK(ClickFileopenHdl_Impl, weld::Button&, void);
    DECL_LINK(ClickTargetHdl_Impl, weld::Button&, void);
    DECL_LINK(ModifiedPathHdl_Impl, weld::ComboBox&, void);
    DECL_LINK(ModifiedTargetHdl_Impl, weld::Entry&, void);
    DECL_LINK(LostFocusPathHdl_Impl, weld::Widget&, void);
    DECL_LINK(TimeoutHdl_Impl, Timer*, void);

    enum class EPathType
    {
        Invalid,
        ExistsFile
    };
    static EPathType GetPathType(std::u16string_view rStrPath);

    virtual void FillDlgFields(const OUString& rStrURL) override;
    virtual void GetCurrentItemData(OUString& rStrURL, OUString& aStrName, OUString& aStrIntName,
                                    OUString& aStrFrame, SvxLinkInsertMode& eMode) override;
    virtual void ClearPageSpecificControls() override;

    OUString GetCurrentURL() const;

    virtual bool ShouldOpenMarkWnd() override;
    virtual void SetMarkWndShouldOpen(bool bOpen) override;

public:
    HyperlinkDocTabPage(weld::Container* pParent, weld::DialogController* pController,
                        const SfxItemSet* pSet);
    virtual ~HyperlinkDocTabPage() override;

    static std::unique_ptr<SfxTabPage>
    Create(weld::Container* pParent, weld::DialogController* pController, const SfxItemSet* pSet);

    virtual void SetInitFocus() override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
