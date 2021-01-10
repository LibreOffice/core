/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <unotools/useroptions.hxx>
#include <svl/adrparse.hxx>

#include <hlinettp.hxx>
#include <hlmarkwn_def.hxx>

constexpr OUStringLiteral sAnonymous = u"anonymous";
char const sFTPScheme[]    = INET_FTP_SCHEME;

/*************************************************************************
|*
|* Constructor / Destructor
|*
|************************************************************************/
SvxHyperlinkInternetTp::SvxHyperlinkInternetTp(weld::Container* pParent,
                                               SvxHpLinkDlg* pDlg,
                                               const SfxItemSet* pItemSet)
    : SvxHyperlinkTabPageBase(pParent, pDlg, "cui/ui/hyperlinkinternetpage.ui", "HyperlinkInternetPage",
                              pItemSet)
    , m_bMarkWndOpen(false)
    , m_xRbtLinktypInternet(xBuilder->weld_radio_button("linktyp_internet"))
    , m_xRbtLinktypFTP(xBuilder->weld_radio_button("linktyp_ftp"))
    , m_xCbbTarget(new SvxHyperURLBox(xBuilder->weld_combo_box("target")))
    , m_xFtTarget(xBuilder->weld_label("target_label"))
    , m_xFtLogin(xBuilder->weld_label("login_label"))
    , m_xEdLogin(xBuilder->weld_entry("login"))
    , m_xFtPassword(xBuilder->weld_label("password_label"))
    , m_xEdPassword(xBuilder->weld_entry("password"))
    , m_xCbAnonymous(xBuilder->weld_check_button("anonymous"))
{
    // gtk_size_group_set_ignore_hidden, "Measuring the size of hidden widgets
    // ...  they will report a size of 0 nowadays, and thus, their size will
    // not affect the other size group members", which is unfortunate. So here
    // before we hide the labels, take the size group width and set it as
    // explicit preferred size on a label that won't be hidden
    auto nLabelWidth = m_xFtTarget->get_preferred_size().Width();
    m_xFtTarget->set_size_request(nLabelWidth, -1);

    m_xCbbTarget->SetSmartProtocol(INetProtocol::Http);

    InitStdControls();

    m_xCbbTarget->show();

    SetExchangeSupport ();

    // set defaults
    m_xRbtLinktypInternet->set_active(true);

    // set handlers
    Link<weld::Button&, void> aLink( LINK ( this, SvxHyperlinkInternetTp, Click_SmartProtocol_Impl ) );
    m_xRbtLinktypInternet->connect_clicked( aLink );
    m_xRbtLinktypFTP->connect_clicked( aLink );
    m_xCbAnonymous->connect_clicked( LINK ( this, SvxHyperlinkInternetTp, ClickAnonymousHdl_Impl ) );
    m_xEdLogin->connect_changed( LINK ( this, SvxHyperlinkInternetTp, ModifiedLoginHdl_Impl ) );
    m_xCbbTarget->connect_focus_out( LINK ( this, SvxHyperlinkInternetTp, LostFocusTargetHdl_Impl ) );
    m_xCbbTarget->connect_changed( LINK ( this, SvxHyperlinkInternetTp, ModifiedTargetHdl_Impl ) );
    maTimer.SetInvokeHandler ( LINK ( this, SvxHyperlinkInternetTp, TimeoutHdl_Impl ) );
}

SvxHyperlinkInternetTp::~SvxHyperlinkInternetTp()
{
}

/*************************************************************************
|*
|* Fill the all dialog-controls except controls in groupbox "more..."
|*
|************************************************************************/
void SvxHyperlinkInternetTp::FillDlgFields(const OUString& rStrURL)
{
    INetURLObject aURL(rStrURL);
    OUString aStrScheme(GetSchemeFromURL(rStrURL));

    // set additional controls for FTP: Username / Password
    if (aStrScheme.startsWith(sFTPScheme))
    {
        if ( aURL.GetUser().toAsciiLowerCase().startsWith( sAnonymous ) )
            setAnonymousFTPUser();
        else
            setFTPUser(aURL.GetUser(), aURL.GetPass());

        //do not show password and user in url
        if(!aURL.GetUser().isEmpty() || !aURL.GetPass().isEmpty() )
            aURL.SetUserAndPass("", "");
    }

    // set URL-field
    // Show the scheme, #72740
    if ( aURL.GetProtocol() != INetProtocol::NotValid )
        m_xCbbTarget->set_entry_text( aURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous ) );
    else
        m_xCbbTarget->set_entry_text(rStrURL);

    SetScheme(aStrScheme);
}

void SvxHyperlinkInternetTp::setAnonymousFTPUser()
{
    m_xEdLogin->set_text(sAnonymous);
    SvAddressParser aAddress(SvtUserOptions().GetEmail());
    m_xEdPassword->set_text(aAddress.Count() ? aAddress.GetEmailAddress(0) : OUString());

    m_xFtLogin->set_sensitive(false);
    m_xFtPassword->set_sensitive(false);
    m_xEdLogin->set_sensitive(false);
    m_xEdPassword->set_sensitive(false);
    m_xCbAnonymous->set_active(true);
}

void SvxHyperlinkInternetTp::setFTPUser(const OUString& rUser, const OUString& rPassword)
{
    m_xEdLogin->set_text(rUser);
    m_xEdPassword->set_text(rPassword);

    m_xFtLogin->set_sensitive(true);
    m_xFtPassword->set_sensitive(true);
    m_xEdLogin->set_sensitive(true);
    m_xEdPassword->set_sensitive(true);
    m_xCbAnonymous->set_active(false);
}

/*************************************************************************
|*
|* retrieve and prepare data from dialog-fields
|*
|************************************************************************/

void SvxHyperlinkInternetTp::GetCurentItemData ( OUString& rStrURL, OUString& aStrName,
                                                 OUString& aStrIntName, OUString& aStrFrame,
                                                 SvxLinkInsertMode& eMode )
{
    rStrURL = CreateAbsoluteURL();
    GetDataFromCommonFields( aStrName, aStrIntName, aStrFrame, eMode );
}

OUString SvxHyperlinkInternetTp::CreateAbsoluteURL() const
{
    // erase leading and trailing whitespaces
    OUString aStrURL(m_xCbbTarget->get_active_text().trim());

    INetURLObject aURL(aStrURL);

    if( aURL.GetProtocol() == INetProtocol::NotValid )
    {
        aURL.SetSmartProtocol( GetSmartProtocolFromButtons() );
        aURL.SetSmartURL(aStrURL);
    }

    // username and password for ftp-url
    if( aURL.GetProtocol() == INetProtocol::Ftp && !m_xEdLogin->get_text().isEmpty() )
        aURL.SetUserAndPass ( m_xEdLogin->get_text(), m_xEdPassword->get_text() );

    if ( aURL.GetProtocol() != INetProtocol::NotValid )
        return aURL.GetMainURL( INetURLObject::DecodeMechanism::ToIUri );
    else //#105788# always create a URL even if it is not valid
        return aStrURL;
}

/*************************************************************************
|*
|* static method to create Tabpage
|*
|************************************************************************/

std::unique_ptr<IconChoicePage> SvxHyperlinkInternetTp::Create(weld::Container* pWindow, SvxHpLinkDlg* pDlg, const SfxItemSet* pItemSet)
{
    return std::make_unique<SvxHyperlinkInternetTp>(pWindow, pDlg, pItemSet);
}

/*************************************************************************
|*
|* Set initial focus
|*
|************************************************************************/
void SvxHyperlinkInternetTp::SetInitFocus()
{
    m_xCbbTarget->grab_focus();
}

/*************************************************************************
|*
|* Contents of editfield "Target" modified
|*
|************************************************************************/
IMPL_LINK_NOARG(SvxHyperlinkInternetTp, ModifiedTargetHdl_Impl, weld::ComboBox&, void)
{
    OUString aScheme = GetSchemeFromURL( m_xCbbTarget->get_active_text() );
    if( !aScheme.isEmpty() )
        SetScheme( aScheme );

    // start timer
    maTimer.SetTimeout( 2500 );
    maTimer.Start();
}

/*************************************************************************
|*
|* If target-field was modify, to browse the new doc after timeout
|*
|************************************************************************/
IMPL_LINK_NOARG(SvxHyperlinkInternetTp, TimeoutHdl_Impl, Timer *, void)
{
    RefreshMarkWindow();
}

/*************************************************************************
|*
|* Contents of editfield "Login" modified
|*
|************************************************************************/
IMPL_LINK_NOARG(SvxHyperlinkInternetTp, ModifiedLoginHdl_Impl, weld::Entry&, void)
{
    OUString aStrLogin ( m_xEdLogin->get_text() );
    if ( aStrLogin.equalsIgnoreAsciiCase( sAnonymous ) )
    {
        m_xCbAnonymous->set_active(true);
        ClickAnonymousHdl_Impl(*m_xCbAnonymous);
    }
}

void SvxHyperlinkInternetTp::SetScheme(const OUString& rScheme)
{
    //if rScheme is empty or unknown the default behaviour is like it where HTTP
    bool bFTP = rScheme.startsWith(sFTPScheme);
    bool bInternet = !bFTP;

    //update protocol button selection:
    m_xRbtLinktypFTP->set_active(bFTP);
    m_xRbtLinktypInternet->set_active(bInternet);

    //update target:
    RemoveImproperProtocol(rScheme);
    m_xCbbTarget->SetSmartProtocol( GetSmartProtocolFromButtons() );

    //show/hide  special fields for FTP:
    m_xFtLogin->set_visible( bFTP );
    m_xFtPassword->set_visible( bFTP );
    m_xEdLogin->set_visible( bFTP );
    m_xEdPassword->set_visible( bFTP );
    m_xCbAnonymous->set_visible( bFTP );

    //update 'link target in document'-window and opening-button
    if (rScheme.startsWith(INET_HTTP_SCHEME) || rScheme.isEmpty())
    {
        if ( m_bMarkWndOpen )
            ShowMarkWnd ();
    }
    else
    {
        //disable for https and ftp
        if ( m_bMarkWndOpen )
            HideMarkWnd ();
    }
}

/*************************************************************************
|*
|* Remove protocol if it does not fit to the current button selection
|*
|************************************************************************/

void SvxHyperlinkInternetTp::RemoveImproperProtocol(std::u16string_view aProperScheme)
{
    OUString aStrURL ( m_xCbbTarget->get_active_text() );
    if ( !aStrURL.isEmpty() )
    {
        OUString aStrScheme(GetSchemeFromURL(aStrURL));
        if ( !aStrScheme.isEmpty() && aStrScheme != aProperScheme )
        {
            aStrURL = aStrURL.copy( aStrScheme.getLength() );
            m_xCbbTarget->set_entry_text( aStrURL );
        }
    }
}

OUString SvxHyperlinkInternetTp::GetSchemeFromButtons() const
{
    if( m_xRbtLinktypFTP->get_active() )
        return INET_FTP_SCHEME;
    return INET_HTTP_SCHEME;
}

INetProtocol SvxHyperlinkInternetTp::GetSmartProtocolFromButtons() const
{
    if( m_xRbtLinktypFTP->get_active() )
    {
        return INetProtocol::Ftp;
    }
    return INetProtocol::Http;
}

/*************************************************************************
|*
|* Click on Radiobutton : Internet or FTP
|*
|************************************************************************/
IMPL_LINK_NOARG(SvxHyperlinkInternetTp, Click_SmartProtocol_Impl, weld::Button&, void)
{
    OUString aScheme = GetSchemeFromButtons();
    SetScheme(aScheme);
}

/*************************************************************************
|*
|* Click on Checkbox : Anonymous user
|*
|************************************************************************/
IMPL_LINK_NOARG(SvxHyperlinkInternetTp, ClickAnonymousHdl_Impl, weld::Button&, void)
{
    // disable login-editfields if checked
    if ( m_xCbAnonymous->get_active() )
    {
        if ( m_xEdLogin->get_text().toAsciiLowerCase().startsWith( sAnonymous ) )
        {
            maStrOldUser.clear();
            maStrOldPassword.clear();
        }
        else
        {
            maStrOldUser = m_xEdLogin->get_text();
            maStrOldPassword = m_xEdPassword->get_text();
        }

        setAnonymousFTPUser();
    }
    else
        setFTPUser(maStrOldUser, maStrOldPassword);
}

/*************************************************************************
|*
|* Combobox Target lost the focus
|*
|************************************************************************/
IMPL_LINK_NOARG(SvxHyperlinkInternetTp, LostFocusTargetHdl_Impl, weld::Widget&, void)
{
    RefreshMarkWindow();
}

void SvxHyperlinkInternetTp::RefreshMarkWindow()
{
    if (m_xRbtLinktypInternet->get_active() && IsMarkWndVisible())
    {
        weld::WaitObject aWait(mpDialog->getDialog());
        OUString aStrURL( CreateAbsoluteURL() );
        if ( !aStrURL.isEmpty() )
            mxMarkWnd->RefreshTree ( aStrURL );
        else
            mxMarkWnd->SetError( LERR_DOCNOTOPEN );
    }
}

/*************************************************************************
|*
|* Get String from Bookmark-Wnd
|*
|************************************************************************/
void SvxHyperlinkInternetTp::SetMarkStr ( const OUString& aStrMark )
{
    OUString aStrURL(m_xCbbTarget->get_active_text());

    const sal_Unicode sUHash = '#';
    sal_Int32 nPos = aStrURL.lastIndexOf( sUHash );

    if( nPos != -1 )
        aStrURL = aStrURL.copy(0, nPos);

    aStrURL += OUStringChar(sUHash) + aStrMark;

    m_xCbbTarget->set_entry_text(aStrURL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
