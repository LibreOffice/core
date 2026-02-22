/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <hyperlinkmailtabpage.hxx>
#include <tools/urlobj.hxx>
#include <sfx2/viewfrm.hxx>
#include <comphelper/lok.hxx>
#include <sfx2/request.hxx>
#include <unotools/moduleoptions.hxx>

HyperlinkMailTabPage::HyperlinkMailTabPage(weld::Container* pParent,
                                           weld::DialogController* pController,
                                           const SfxItemSet* pSet)
    : HyperlinkTabPageBase(pParent, pController, u"cui/ui/hyperlinkmailpage.ui"_ustr,
                           u"HyperlinkMailPage"_ustr, pSet)
    , m_xCbbReceiver(std::make_unique<SvxHyperURLBox>(m_xBuilder->weld_combo_box(u"receiver"_ustr)))
    , m_xBtAdrBook(m_xBuilder->weld_button(u"addressbook"_ustr))
    , m_xEdSubject(m_xBuilder->weld_entry(u"subject"_ustr))
{
    m_xCbbReceiver->SetSmartProtocol(INetProtocol::Mailto);

    m_xBtAdrBook->connect_clicked(LINK(this, HyperlinkMailTabPage, ClickAdrBookHdl_Impl));
    m_xCbbReceiver->connect_changed(LINK(this, HyperlinkMailTabPage, ModifiedReceiverHdl_Impl));

    InitStdControls();

    if (!SvtModuleOptions().IsDataBaseInstalled() || comphelper::LibreOfficeKit::isActive())
        m_xBtAdrBook->hide();
}

std::unique_ptr<SfxTabPage> HyperlinkMailTabPage::Create(weld::Container* pParent,
                                                         weld::DialogController* pController,
                                                         const SfxItemSet* pSet)
{
    return std::make_unique<HyperlinkMailTabPage>(pParent, pController, pSet);
}

void HyperlinkMailTabPage::FillDlgFields(const OUString& rStrURL)
{
    OUString aStrScheme = GetSchemeFromURL(rStrURL);

    OUString aStrURLc(rStrURL);

    if (aStrScheme.startsWith(INET_MAILTO_SCHEME))
    {
        OUString aStrSubject, aStrTmp(aStrURLc);

        sal_Int32 nPos = aStrTmp.toAsciiLowerCase().indexOf("subject");

        if (nPos != -1)
            nPos = aStrTmp.indexOf('=', nPos);

        if (nPos != -1)
            aStrSubject = aStrURLc.copy(nPos + 1);

        nPos = aStrURLc.indexOf('?');

        if (nPos != -1)
            aStrURLc = aStrURLc.copy(0, nPos);

        m_xEdSubject->set_text(aStrSubject);
    }
    else
    {
        m_xEdSubject->set_text(u""_ustr);
    }

    m_xCbbReceiver->set_entry_text(aStrURLc);

    SetScheme(aStrScheme);
}

void HyperlinkMailTabPage::GetCurrentItemData(OUString& rStrURL, OUString& aStrName,
                                              OUString& aStrIntName, OUString& aStrFrame,
                                              SvxLinkInsertMode& eMode)
{
    rStrURL = CreateAbsoluteURL();
    GetDataFromCommonFields(aStrName, aStrIntName, aStrFrame, eMode);
}

OUString HyperlinkMailTabPage::CreateAbsoluteURL() const
{
    OUString aStrURL = m_xCbbReceiver->get_active_text();
    INetURLObject aURL(aStrURL, INetProtocol::Mailto);

    if (aURL.GetProtocol() == INetProtocol::Mailto)
    {
        if (!m_xEdSubject->get_text().isEmpty())
        {
            OUString aQuery = "subject=" + m_xEdSubject->get_text();
            aURL.SetParam(aQuery);
        }
    }

    if (aURL.GetProtocol() != INetProtocol::NotValid)
        return aURL.GetMainURL(INetURLObject::DecodeMechanism::WithCharset);
    else
        return aStrURL;
}

void HyperlinkMailTabPage::SetInitFocus() { m_xCbbReceiver->grab_focus(); }

void HyperlinkMailTabPage::SetScheme(std::u16string_view rScheme)
{
    RemoveImproperProtocol(rScheme);
    m_xCbbReceiver->SetSmartProtocol(INetProtocol::Mailto);

    m_xBtAdrBook->set_sensitive(true);
    m_xEdSubject->set_sensitive(true);
}

void HyperlinkMailTabPage::RemoveImproperProtocol(std::u16string_view aProperScheme)
{
    OUString aStrURL(m_xCbbReceiver->get_active_text());
    if (!aStrURL.isEmpty())
    {
        OUString aStrScheme = GetSchemeFromURL(aStrURL);
        if (!aStrScheme.isEmpty() && aStrScheme != aProperScheme)
        {
            aStrURL = aStrURL.copy(aStrScheme.getLength());
            m_xCbbReceiver->set_entry_text(aStrURL);
        }
    }
}

void HyperlinkMailTabPage::ClearPageSpecificControls()
{
    m_xCbbReceiver->set_entry_text(OUString());
    m_xEdSubject->set_text(OUString());
}

IMPL_LINK_NOARG(HyperlinkMailTabPage, ModifiedReceiverHdl_Impl, weld::ComboBox&, void)
{
    OUString aScheme = GetSchemeFromURL(m_xCbbReceiver->get_active_text());
    if (!aScheme.isEmpty())
        SetScheme(aScheme);
}

IMPL_STATIC_LINK_NOARG(HyperlinkMailTabPage, ClickAdrBookHdl_Impl, weld::Button&, void)
{
    if (SfxViewFrame* pViewFrame = SfxViewFrame::Current())
    {
        SfxItemPool& rPool = pViewFrame->GetPool();
        SfxRequest aReq(SID_VIEW_DATA_SOURCE_BROWSER, SfxCallMode::SLOT, rPool);
        pViewFrame->ExecuteSlot(aReq, true);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
