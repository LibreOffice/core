/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <hyperlinkinternettabpage.hxx>
#include <tools/urlobj.hxx>
#include <o3tl/string_view.hxx>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <sot/exchange.hxx>
#include <hlmarkwn_def.hxx>

OUString HyperlinkInternetTP::CreateAbsoluteURL() const
{
    // erase leading and trailing whitespaces
    OUString aStrURL(o3tl::trim(m_xCbbTarget->get_active_text()));

    INetURLObject aURL(aStrURL, GetSmartProtocolFromButtons());

    if (aURL.GetProtocol() != INetProtocol::NotValid)
        return aURL.GetMainURL(INetURLObject::DecodeMechanism::ToIUri);
    else
        return aStrURL;
}

HyperlinkInternetTP::HyperlinkInternetTP(weld::Container* pParent,
                                         weld::DialogController* pController,
                                         const SfxItemSet* pSet)
    : HyperlinkTabPageBase(pParent, pController, u"cui/ui/hyperlinkinternetpage.ui"_ustr,
                           u"HyperlinkInternetPage"_ustr, pSet)
    , m_xCbbTarget(std::make_unique<SvxHyperURLBox>(m_xBuilder->weld_combo_box(u"target"_ustr)))
{
    m_xCbbTarget->SetSmartProtocol(GetSmartProtocolFromButtons());

    InitStdControls();

    // set handlers
    m_xCbbTarget->connect_focus_out(LINK(this, HyperlinkInternetTP, LostFocusTargetHdl_Impl));
    m_xCbbTarget->connect_changed(LINK(this, HyperlinkInternetTP, ModifiedTargetHdl_Impl));
    maTimer.SetInvokeHandler(LINK(this, HyperlinkInternetTP, TimeoutHdl_Impl));
}

std::unique_ptr<SfxTabPage> HyperlinkInternetTP::Create(weld::Container* pParent,
                                                        weld::DialogController* pController,
                                                        const SfxItemSet* pSet)
{
    return std::make_unique<HyperlinkInternetTP>(pParent, pController, pSet);
}

void HyperlinkInternetTP::FillDlgFields(const OUString& rStrURL)
{
    // tdf#146576 - propose clipboard content when inserting a hyperlink
    OUString aStrURL(rStrURL);
    if (aStrURL.isEmpty())
    {
        if (auto xClipboard = GetSystemClipboard())
        {
            if (auto xTransferable = xClipboard->getContents())
            {
                css::datatransfer::DataFlavor aFlavor;
                SotExchange::GetFormatDataFlavor(SotClipboardFormatId::STRING, aFlavor);
                if (xTransferable->isDataFlavorSupported(aFlavor))
                {
                    OUString aClipBoardContent;
                    try
                    {
                        if (xTransferable->getTransferData(aFlavor) >>= aClipBoardContent)
                        {
                            // tdf#162753 - allow only syntactically valid hyperlink targets
                            INetURLObject aURL(o3tl::trim(aClipBoardContent));
                            if (!aURL.HasError())
                                aStrURL
                                    = aURL.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous);
                        }
                    }
                    // tdf#158345: Opening Hyperlink dialog leads to crash
                    catch (const css::datatransfer::UnsupportedFlavorException&)
                    {
                    }
                }
            }
        }
    }

    INetURLObject aURL(aStrURL);
    OUString aStrScheme(GetSchemeFromURL(aStrURL));

    // set URL-field
    // Show the scheme, #72740
    if (aURL.GetProtocol() != INetProtocol::NotValid)
        m_xCbbTarget->set_entry_text(aURL.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous));
    else
        m_xCbbTarget->set_entry_text(rStrURL);

    SetScheme(aStrScheme);
}

void HyperlinkInternetTP::GetCurrentItemData(OUString& rStrURL, OUString& aStrName,
                                             OUString& aStrIntName, OUString& aStrFrame,
                                             SvxLinkInsertMode& eMode)
{
    rStrURL = CreateAbsoluteURL();
    GetDataFromCommonFields(aStrName, aStrIntName, aStrFrame, eMode);
}

void HyperlinkInternetTP::ClearPageSpecificControls() { m_xCbbTarget->set_entry_text(OUString()); }

IMPL_LINK_NOARG(HyperlinkInternetTP, ModifiedTargetHdl_Impl, weld::ComboBox&, void)
{
    OUString aScheme = GetSchemeFromURL(m_xCbbTarget->get_active_text());
    if (!aScheme.isEmpty())
        SetScheme(aScheme);

    // start timer
    maTimer.SetTimeout(2500);
    maTimer.Start();
}

IMPL_LINK_NOARG(HyperlinkInternetTP, TimeoutHdl_Impl, Timer*, void) { RefreshMarkWindow(); }

void HyperlinkInternetTP::SetScheme(std::u16string_view rScheme)
{
    // update target
    RemoveImproperProtocol(rScheme);
    m_xCbbTarget->SetSmartProtocol(GetSmartProtocolFromButtons());

    // update mark window visibility
    if (o3tl::starts_with(rScheme, INET_HTTP_SCHEME) || rScheme.empty())
    {
        if (m_bMarkWndOpen)
            ShowMarkWnd();
    }
    else
    {
        // disable for https and ftp
        if (m_bMarkWndOpen)
            HideMarkWnd();
    }
}

void HyperlinkInternetTP::RemoveImproperProtocol(std::u16string_view aProperScheme)
{
    OUString aStrURL(m_xCbbTarget->get_active_text());
    if (!aStrURL.isEmpty())
    {
        OUString aStrScheme(GetSchemeFromURL(aStrURL));
        if (!aStrScheme.isEmpty() && aStrScheme != aProperScheme)
        {
            aStrURL = aStrURL.copy(aStrScheme.getLength());
            m_xCbbTarget->set_entry_text(aStrURL);
        }
    }
}

INetProtocol HyperlinkInternetTP::GetSmartProtocolFromButtons() { return INetProtocol::Http; }

IMPL_LINK_NOARG(HyperlinkInternetTP, LostFocusTargetHdl_Impl, weld::Widget&, void)
{
    RefreshMarkWindow();
}

void HyperlinkInternetTP::RefreshMarkWindow()
{
    if (IsMarkWndVisible())
    {
        weld::WaitObject aWait(GetFrameWeld());
        OUString aStrURL(CreateAbsoluteURL());
        if (!aStrURL.isEmpty())
            mxMarkWnd->RefreshTree(aStrURL);
        else
            mxMarkWnd->SetError(LERR_DOCNOTOPEN);
    }
}

void HyperlinkInternetTP::SetInitFocus() { m_xCbbTarget->grab_focus(); }

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
