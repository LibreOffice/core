/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <hyperlinkdlg.hxx>
#include <hyperlinkinternettabpage.hxx>
#include <hyperlinkmailtabpage.hxx>
#include <hyperlinkdoctabpage.hxx>
#include <hyperlinknewdoctabpage.hxx>
#include <hyperlinktabpagebase.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/app.hxx>
#include <sfx2/basedlgs.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/bindings.hxx>
#include <svl/urihelper.hxx>
#include <svx/hlnkitem.hxx>
#include <vcl/tabs.hrc>

HyperlinkDialog::HyperlinkDialog(weld::Window* pParent, SfxChildWindow* /*pChildWindow*/)
    : SfxTabDialogController(pParent, u"cui/ui/hyperlinkdlg.ui"_ustr, u"HyperlinkDialog"_ustr)
{
    AddTabPage("Internet", TabResId(RID_TAB_HLINTERNET.aLabel), HyperlinkInternetTP::Create,
               RID_TAB_HLINTERNET.sIconName);

    AddTabPage("Mail", TabResId(RID_TAB_HLMAIL.aLabel), HyperlinkMailTabPage::Create,
               RID_TAB_HLMAIL.sIconName);

    AddTabPage("Document", TabResId(RID_TAB_HLDOC.aLabel), HyperlinkDocTabPage::Create,
               RID_TAB_HLDOC.sIconName);

    AddTabPage("New Document", TabResId(RID_TAB_HLDOCN.aLabel), HyperlinkDocNewTabPage::Create,
               RID_TAB_HLDOCN.sIconName);

    SetApplyHandler(LINK(this, HyperlinkDialog, ClickApplyHdl));
}

SfxItemSet* HyperlinkDialog::CreateInputItemSet(const OUString&)
{
    m_pInputSet = std::make_unique<SfxItemSetFixed<SID_HYPERLINK_GETLINK, SID_HYPERLINK_SETLINK>>(
        SfxGetpApp()->GetPool());
    return m_pInputSet.get();
}

OUString HyperlinkDialog::DetermineTabPageId(const SvxHyperlinkItem* pHyperItem)
{
    if (comphelper::LibreOfficeKit::isActive())
        return "Internet";

    const OUString& aStrURL = pHyperItem->GetURL();
    INetURLObject aURL(aStrURL);
    INetProtocol eProtocolTyp = aURL.GetProtocol();

    if (eProtocolTyp == INetProtocol::Http || eProtocolTyp == INetProtocol::Https
        || eProtocolTyp == INetProtocol::Ftp)
        return "Internet";

    if (eProtocolTyp == INetProtocol::Mailto)
        return "Mail";

    if (eProtocolTyp == INetProtocol::File || aStrURL.startsWith("#"))
        return "Document";

    return "Internet";
}

void HyperlinkDialog::Apply()
{
    auto aSet = SfxItemSet::makeFixedSfxItemSet<SID_HYPERLINK_GETLINK, SID_HYPERLINK_SETLINK>(
        SfxGetpApp()->GetPool());

    SfxTabPage* pCurrentTabPage = GetTabPage(GetCurPageId());
    if (!pCurrentTabPage)
        return;

    if (!pCurrentTabPage->FillItemSet(&aSet))
        return;

    const SvxHyperlinkItem* pItem = aSet.GetItem(SID_HYPERLINK_SETLINK);

    if (!pItem || pItem->GetURL().isEmpty())
        return;

    // create the document first
    HyperlinkDocNewTabPage* pNewDocTab = dynamic_cast<HyperlinkDocNewTabPage*>(pCurrentTabPage);
    if (pNewDocTab)
    {
        pNewDocTab->DoApply();
    }

    SfxViewFrame* pFrame = SfxViewFrame::Current();
    if (!pFrame)
        return;

    SfxDispatcher* pDisp = pFrame->GetDispatcher();
    if (!pDisp)
        return;

    pDisp->ExecuteList(SID_HYPERLINK_SETLINK, SfxCallMode::RECORD, { pItem });
}

void HyperlinkDialog::CheckAndInitializeHyperlinkData()
{
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    if (!pViewFrame)
        return;

    std::unique_ptr<SfxPoolItem> pItem;
    SfxItemState eState = pViewFrame->GetBindings().QueryState(SID_HYPERLINK_GETLINK, pItem);

    if (eState >= SfxItemState::DEFAULT && pItem)
    {
        const SvxHyperlinkItem* pHyperItem = dynamic_cast<SvxHyperlinkItem*>(pItem.get());
        if (!pHyperItem)
            return;

        if (m_pInputSet)
        {
            m_pInputSet->Put(*pHyperItem);
        }

        // Only override the page if there's a valid URL to determine the appropriate tab
        // holds the last page when dialog is opened without hyperlink data (tdf#90496)
        const OUString& aStrURL = pHyperItem->GetURL();
        if (!aStrURL.isEmpty())
        {
            OUString sPageId = DetermineTabPageId(pHyperItem);
            SetCurPageId(sPageId);
        }

        SfxTabPage* pCurrentTabPage = GetTabPage(GetCurPageId());
        if (pCurrentTabPage)
        {
            pCurrentTabPage->Reset(m_pInputSet.get());
        }
    }
}

void HyperlinkDialog::Activate()
{
    SfxTabDialogController::Activate();

    // Set initial focus to the current tab page
    SfxTabPage* pCurrentTabPage = GetTabPage(GetCurPageId());
    if (pCurrentTabPage)
    {
        static_cast<HyperlinkTabPageBase*>(pCurrentTabPage)->SetInitFocus();
    }

    // Check for existing hyperlink data only once when dialog is first activated
    if (!m_bInitialized)
    {
        CheckAndInitializeHyperlinkData();
        m_bInitialized = true;
    }
}

IMPL_LINK_NOARG(HyperlinkDialog, ClickApplyHdl, weld::Button&, void) { Apply(); }

short HyperlinkDialog::Ok()
{
    Apply();
    return SfxTabDialogController::Ok();
}

void HyperlinkDialog::PageCreated(const OUString&, SfxTabPage& rTabPage)
{
    SfxViewFrame* pFrame = SfxViewFrame::Current();
    if (pFrame)
    {
        css::uno::Reference<css::frame::XFrame> xFrame = pFrame->GetFrame().GetFrameInterface();
        rTabPage.SetFrame(xFrame);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
