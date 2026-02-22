/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <hyperlinktabpagebase.hxx>
#include <sot/formats.hxx>
#include <sfx2/app.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/sfxsids.hrc>
#include <vcl/weld/Dialog.hxx>
#include <macroass.hxx>
#include <dialmgr.hxx>
#include <strings.hrc>
#include <tools/urlobj.hxx>
#include <com/sun/star/frame/XFrame.hpp>
#include <bitmaps.hlst>

// SvxHyperURLBox implementation
// ComboBox-Control for URL's with History and Autocompletion
SvxHyperURLBox::SvxHyperURLBox(std::unique_ptr<weld::ComboBox> xControl)
    : SvtURLBox(std::move(xControl))
    , DropTargetHelper(getWidget()->get_drop_target())
{
    SetSmartProtocol(INetProtocol::Http);
}

sal_Int8 SvxHyperURLBox::AcceptDrop(const AcceptDropEvent& /* rEvt */)
{
    return IsDropFormatSupported(SotClipboardFormatId::STRING) ? DND_ACTION_COPY : DND_ACTION_NONE;
}

sal_Int8 SvxHyperURLBox::ExecuteDrop(const ExecuteDropEvent& rEvt)
{
    TransferableDataHelper aDataHelper(rEvt.maDropEvent.Transferable);
    OUString aString;
    sal_Int8 nRet = DND_ACTION_NONE;

    if (aDataHelper.GetString(SotClipboardFormatId::STRING, aString))
    {
        set_entry_text(aString);
        nRet = DND_ACTION_COPY;
    }

    return nRet;
}

HyperlinkTabPageBase::HyperlinkTabPageBase(weld::Container* pParent,
                                           weld::DialogController* pController,
                                           const OUString& rUIXMLDescription, const OUString& rID,
                                           const SfxItemSet* pItemSet)
    : SfxTabPage(pParent, pController, rUIXMLDescription, rID, pItemSet)
    , m_xFrame(m_xBuilder->weld_combo_box(u"frame"_ustr))
    , m_xIndication(m_xBuilder->weld_entry(u"indication"_ustr))
    , m_xName(m_xBuilder->weld_entry(u"name"_ustr))
    , m_xForm(m_xBuilder->weld_combo_box(u"form"_ustr))
    , m_xScript(m_xBuilder->weld_button(u"script"_ustr))
    , maTimer("cui HyperlinkTabPageBase maTimer")
{
}

HyperlinkTabPageBase::~HyperlinkTabPageBase()
{
    maTimer.Stop();
    HideMarkWnd();
}

void HyperlinkTabPageBase::InitStdControls()
{
    SfxViewFrame* pViewFrame = SfxViewFrame::Current();
    SfxFrame* pFrame = pViewFrame ? &pViewFrame->GetFrame() : nullptr;
    if (pFrame)
    {
        TargetList aList;
        SfxFrame::GetDefaultTargetList(aList);
        if (!aList.empty())
        {
            for (const auto& rTarget : aList)
                m_xFrame->append_text(rTarget);
        }
    }

    m_xForm->append_text(u"Text"_ustr);
    m_xForm->append_text(u"Button"_ustr);
    m_xForm->set_active(0);

    m_xScript->set_from_icon_name(RID_SVXBMP_SCRIPT);
    m_xScript->connect_clicked(LINK(this, HyperlinkTabPageBase, ClickScriptHdl));
}

void HyperlinkTabPageBase::GetDataFromCommonFields(OUString& aStrName, OUString& aStrIntName,
                                                   OUString& aStrFrame, SvxLinkInsertMode& eMode)
{
    aStrName = m_xIndication->get_text();
    aStrIntName = m_xName->get_text();
    aStrFrame = m_xFrame->get_active_text();
    eMode = (m_xForm->get_active() == 1) ? HLINK_BUTTON : HLINK_FIELD;
}

OUString HyperlinkTabPageBase::GetSchemeFromURL(std::u16string_view rStrUrl)
{
    INetURLObject aURL(rStrUrl);
    if (aURL.GetProtocol() != INetProtocol::NotValid)
        return INetURLObject::GetScheme(aURL.GetProtocol());
    return OUString();
}

void HyperlinkTabPageBase::ShowMarkWnd()
{
    if (mxMarkWnd)
    {
        mxMarkWnd->getDialog()->present();
        return;
    }

    weld::Window* pDialog = GetFrameWeld();
    if (!pDialog)
        return;

    mxMarkWnd = std::make_shared<SvxHlinkDlgMarkWnd>(pDialog);

    // Size of dialog-window in screen pixels
    Size aDlgSize(pDialog->get_size());

    // Size of Extrawindow
    Size aExtraWndSize(mxMarkWnd->getDialog()->get_preferred_size());

    mxMarkWnd->RestorePosSize();

    // Set size of Extra-Window
    mxMarkWnd->getDialog()->set_size_request(aExtraWndSize.Width(), aDlgSize.Height());

    // Don't capture 'this' - cleanup is handled by HideMarkWnd() in the destructor
    // Capturing 'this' causes a dangling reference if the tab page is destroyed
    // before the async callback fires (e.g., when closing LibreOffice with dialog open)
    weld::DialogController::runAsync(mxMarkWnd, [](sal_Int32 /*nResult*/) {});
}

void HyperlinkTabPageBase::HideMarkWnd()
{
    if (mxMarkWnd)
    {
        mxMarkWnd->response(RET_CANCEL);
        mxMarkWnd.reset();
    }
}

void HyperlinkTabPageBase::SetInitFocus()
{
    if (m_xIndication)
        m_xIndication->grab_focus();
}

void HyperlinkTabPageBase::Reset(const SfxItemSet* pItemSet)
{
    // Get hyperlink item from the item set
    const SvxHyperlinkItem* pHyperlinkItem = nullptr;
    if (pItemSet)
    {
        pHyperlinkItem = pItemSet->GetItem<SvxHyperlinkItem>(SID_HYPERLINK_GETLINK);
    }

    if (pHyperlinkItem)
    {
        FillDlgFields(pHyperlinkItem->GetURL());

        m_xIndication->set_text(pHyperlinkItem->GetName());
        m_xName->set_text(pHyperlinkItem->GetIntName());

        OUString aFrame = pHyperlinkItem->GetTargetFrame();
        if (!aFrame.isEmpty())
        {
            sal_Int32 nPos = m_xFrame->find_text(aFrame);
            if (nPos != -1)
                m_xFrame->set_active(nPos);
            else
                m_xFrame->set_entry_text(aFrame);
        }
        else if (m_xFrame->get_count() > 0)
        {
            m_xFrame->set_active(0);
        }

        SvxLinkInsertMode eMode = pHyperlinkItem->GetInsertMode();
        if ((eMode & HLINK_BUTTON) != 0)
            m_xForm->set_active(1);
        else
            m_xForm->set_active(0);

        if (m_bMacroEventsEnabled)
        {
            m_nMacroEvents = pHyperlinkItem->GetMacroEvents();
            const SvxMacroTableDtor* pMacroTbl = pHyperlinkItem->GetMacroTable();
            if (pMacroTbl)
                m_aMacroTable = *pMacroTbl;
            m_xScript->set_sensitive(m_nMacroEvents != HyperDialogEvent::NONE);
        }

        if (ShouldOpenMarkWnd())
            ShowMarkWnd();
    }
    else
    {
        // Clear page-specific controls first
        ClearPageSpecificControls();

        // Clear common controls
        m_xIndication->set_text(OUString());
        m_xName->set_text(OUString());
        if (m_xFrame->get_count() > 0)
            m_xFrame->set_active(0);
        m_xForm->set_active(0);
        m_xScript->set_sensitive(m_bMacroEventsEnabled);
        m_nMacroEvents = HyperDialogEvent::NONE;
        m_aMacroTable = SvxMacroTableDtor();
    }

    SetMarkWndShouldOpen(IsMarkWndVisible());
    HideMarkWnd();
}

bool HyperlinkTabPageBase::FillItemSet(SfxItemSet* pItemSet)
{
    OUString aStrName = m_xIndication->get_text();
    OUString aStrIntName = m_xName->get_text();
    OUString aStrFrame = m_xFrame->get_active_text();
    SvxLinkInsertMode eMode = (m_xForm->get_active() == 1) ? HLINK_BUTTON : HLINK_FIELD;

    OUString aStrURL;
    GetCurrentItemData(aStrURL, aStrName, aStrIntName, aStrFrame, eMode);

    if (aStrURL.isEmpty())
        return false;

    if (aStrName.isEmpty())
        aStrName = aStrURL;

    SvxHyperlinkItem aItem(SID_HYPERLINK_SETLINK);
    aItem.SetName(aStrName);
    aItem.SetURL(aStrURL);
    aItem.SetIntName(aStrIntName);
    aItem.SetTargetFrame(aStrFrame);
    aItem.SetInsertMode(eMode);

    if (m_bMacroEventsEnabled)
    {
        aItem.SetMacroEvents(m_nMacroEvents);
        aItem.SetMacroTable(m_aMacroTable);
    }

    pItemSet->Put(aItem);
    return true;
}

IMPL_LINK_NOARG(HyperlinkTabPageBase, ClickScriptHdl, weld::Button&, void)
{
    if (!m_bMacroEventsEnabled)
        return;

    SvxMacroItem aItem(SID_ATTR_MACROITEM);
    if (!m_aMacroTable.empty())
        aItem.SetMacroTable(m_aMacroTable);

    auto xItemSet = std::make_unique<SfxItemSetFixed<SID_ATTR_MACROITEM, SID_ATTR_MACROITEM>>(
        SfxGetpApp()->GetPool());
    xItemSet->Put(aItem);

    weld::Window* pWin = GetFrameWeld();
    css::uno::Reference<css::frame::XFrame> xFrame = GetFrame();

    SfxMacroAssignDlg aDlg(pWin, xFrame, std::move(xItemSet));

    SfxMacroTabPage* pMacroPage = aDlg.GetTabPage();
    pMacroPage->AddEvent(CuiResId(RID_CUISTR_HYPDLG_MACROACT1), static_cast<SvMacroItemId>(5100));
    pMacroPage->AddEvent(CuiResId(RID_CUISTR_HYPDLG_MACROACT2), static_cast<SvMacroItemId>(5101));
    pMacroPage->AddEvent(CuiResId(RID_CUISTR_HYPDLG_MACROACT3), static_cast<SvMacroItemId>(5102));

    short nRet = aDlg.run();
    if (RET_OK == nRet)
    {
        const SfxItemSet* pOutSet = aDlg.GetOutputItemSet();
        const SfxPoolItem* pItem;
        if (SfxItemState::SET == pOutSet->GetItemState(SID_ATTR_MACROITEM, false, &pItem))
        {
            const SvxMacroItem* pMacroItem = static_cast<const SvxMacroItem*>(pItem);
            m_aMacroTable = pMacroItem->GetMacroTable();
            m_xScript->set_sensitive(!m_aMacroTable.empty());
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
