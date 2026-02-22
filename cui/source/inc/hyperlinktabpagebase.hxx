/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <memory>
#include <vcl/weld/weld.hxx>
#include <vcl/weld/ComboBox.hxx>
#include <vcl/weld/Entry.hxx>
#include <vcl/weld/Builder.hxx>
#include <vcl/vclenum.hxx>
#include <vcl/timer.hxx>
#include <vcl/transfer.hxx>
#include <sfx2/tabdlg.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/hlnkitem.hxx>
#include <svl/macitem.hxx>
#include <svtools/inettbc.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include "hlmarkwn.hxx"

/// ComboBox-Control for URL's with History and Autocompletion
class SvxHyperURLBox : public SvtURLBox, public DropTargetHelper
{
protected:
    virtual sal_Int8 AcceptDrop(const AcceptDropEvent& rEvt) override;
    virtual sal_Int8 ExecuteDrop(const ExecuteDropEvent& rEvt) override;

public:
    SvxHyperURLBox(std::unique_ptr<weld::ComboBox> xWidget);
};

/// Base class for hyperlink tab pages containing common controls and functionality
class HyperlinkTabPageBase : public SfxTabPage
{
protected:
    std::unique_ptr<weld::ComboBox> m_xFrame;
    std::unique_ptr<weld::Entry> m_xIndication;
    std::unique_ptr<weld::Entry> m_xName;
    std::unique_ptr<weld::ComboBox> m_xForm;
    std::unique_ptr<weld::Button> m_xScript;

    bool m_bMacroEventsEnabled = true;
    HyperDialogEvent m_nMacroEvents = HyperDialogEvent::NONE;
    SvxMacroTableDtor m_aMacroTable;

    Timer maTimer;

    std::shared_ptr<SvxHlinkDlgMarkWnd> mxMarkWnd;

    bool m_bMarkWndOpen = false;

    DECL_LINK(ClickScriptHdl, weld::Button&, void);

    void InitStdControls();

    virtual void FillDlgFields(const OUString& rStrURL) = 0;
    virtual void GetCurrentItemData(OUString& rStrURL, OUString& aStrName, OUString& aStrIntName,
                                    OUString& aStrFrame, SvxLinkInsertMode& eMode)
        = 0;

    virtual void ClearPageSpecificControls() = 0;

    void GetDataFromCommonFields(OUString& aStrName, OUString& aStrIntName, OUString& aStrFrame,
                                 SvxLinkInsertMode& eMode);

    static OUString GetSchemeFromURL(std::u16string_view rStrUrl);

    virtual bool ShouldOpenMarkWnd() { return false; }
    virtual void SetMarkWndShouldOpen(bool bOpen) { m_bMarkWndOpen = bOpen; }

    void ShowMarkWnd();
    void HideMarkWnd();

public:
    HyperlinkTabPageBase(weld::Container* pParent, weld::DialogController* pController,
                         const OUString& rUIXMLDescription, const OUString& rID,
                         const SfxItemSet* pItemSet);

    virtual ~HyperlinkTabPageBase();

    void Reset(const SfxItemSet* pItemSet) override;
    bool FillItemSet(SfxItemSet* pItemSet) override;

    virtual void SetInitFocus();

    bool IsMarkWndVisible() const { return static_cast<bool>(mxMarkWnd); }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
