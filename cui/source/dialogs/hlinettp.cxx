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

#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <svl/adrparse.hxx>

#include "hlinettp.hxx"
#include "hlmarkwn_def.hxx"

sal_Char const sAnonymous[]    = "anonymous";
sal_Char const sHTTPScheme[]   = INET_HTTP_SCHEME;
sal_Char const sFTPScheme[]    = INET_FTP_SCHEME;

/*************************************************************************
|*
|* Constructor / Destructor
|*
|************************************************************************/

SvxHyperlinkInternetTp::SvxHyperlinkInternetTp ( vcl::Window *pParent,
                                                 IconChoiceDialog* pDlg,
                                                 const SfxItemSet& rItemSet)
:   SvxHyperlinkTabPageBase ( pParent, pDlg, "HyperlinkInternetPage", "cui/ui/hyperlinkinternetpage.ui",
                              rItemSet ) ,
    mbMarkWndOpen           ( false )
{
    get(m_pRbtLinktypInternet, "linktyp_internet");
    get(m_pRbtLinktypFTP, "linktyp_ftp");
    get(m_pCbbTarget, "target");
    m_pCbbTarget->SetSmartProtocol(INET_PROT_HTTP);
    get(m_pBtBrowse, "browse");
    m_pBtBrowse->SetModeImage(Image(CUI_RES (RID_SVXBMP_BROWSE)));
    get(m_pFtLogin, "login_label");
    get(m_pEdLogin, "login");
    get(m_pFtPassword, "password_label");
    get(m_pEdPassword, "password");
    get(m_pCbAnonymous, "anonymous");

    // Disable display of bitmap names.
    m_pBtBrowse->EnableTextDisplay (false);

    InitStdControls();

    m_pCbbTarget->Show();
    m_pCbbTarget->SetHelpId( HID_HYPERDLG_INET_PATH );

    SetExchangeSupport ();


    // set defaults
    m_pRbtLinktypInternet->Check ();
    m_pBtBrowse->Enable( true );


    // overload handlers
    Link aLink( LINK ( this, SvxHyperlinkInternetTp, Click_SmartProtocol_Impl ) );
    m_pRbtLinktypInternet->SetClickHdl( aLink );
    m_pRbtLinktypFTP->SetClickHdl     ( aLink );
    m_pCbAnonymous->SetClickHdl       ( LINK ( this, SvxHyperlinkInternetTp, ClickAnonymousHdl_Impl ) );
    m_pBtBrowse->SetClickHdl          ( LINK ( this, SvxHyperlinkInternetTp, ClickBrowseHdl_Impl ) );
    m_pEdLogin->SetModifyHdl          ( LINK ( this, SvxHyperlinkInternetTp, ModifiedLoginHdl_Impl ) );
    m_pCbbTarget->SetLoseFocusHdl     ( LINK ( this, SvxHyperlinkInternetTp, LostFocusTargetHdl_Impl ) );
    m_pCbbTarget->SetModifyHdl        ( LINK ( this, SvxHyperlinkInternetTp, ModifiedTargetHdl_Impl ) );
    maTimer.SetTimeoutHdl           ( LINK ( this, SvxHyperlinkInternetTp, TimeoutHdl_Impl ) );
}

SvxHyperlinkInternetTp::~SvxHyperlinkInternetTp ()
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
            aURL.SetUserAndPass(aEmptyStr,aEmptyStr);
    }

    // set URL-field
    // Show the scheme, #72740
    if ( aURL.GetProtocol() != INET_PROT_NOT_VALID )
        m_pCbbTarget->SetText( aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ) );
    else
        m_pCbbTarget->SetText(rStrURL); // #77696#

    SetScheme(aStrScheme);
}

void SvxHyperlinkInternetTp::setAnonymousFTPUser()
{
    m_pEdLogin->SetText(OUString(sAnonymous));
    SvAddressParser aAddress( SvtUserOptions().GetEmail() );
    m_pEdPassword->SetText( aAddress.Count() ? aAddress.GetEmailAddress(0) : OUString() );

    m_pFtLogin->Disable ();
    m_pFtPassword->Disable ();
    m_pEdLogin->Disable ();
    m_pEdPassword->Disable ();
    m_pCbAnonymous->Check();
}

void SvxHyperlinkInternetTp::setFTPUser(const OUString& rUser, const OUString& rPassword)
{
    m_pEdLogin->SetText ( rUser );
    m_pEdPassword->SetText ( rPassword );

    m_pFtLogin->Enable ();
    m_pFtPassword->Enable ();
    m_pEdLogin->Enable ();
    m_pEdPassword->Enable ();
    m_pCbAnonymous->Check(false);
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
    OUString aStrURL( m_pCbbTarget->GetText().trim() );

    INetURLObject aURL(aStrURL);

    if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        aURL.SetSmartProtocol( GetSmartProtocolFromButtons() );
        aURL.SetSmartURL(aStrURL);
    }

    // username and password for ftp-url
    if( aURL.GetProtocol() == INET_PROT_FTP && !m_pEdLogin->GetText().isEmpty() )
        aURL.SetUserAndPass ( m_pEdLogin->GetText(), m_pEdPassword->GetText() );

    if ( aURL.GetProtocol() != INET_PROT_NOT_VALID )
        return aURL.GetMainURL( INetURLObject::DECODE_WITH_CHARSET );
    else //#105788# always create a URL even if it is not valid
        return aStrURL;
}

/*************************************************************************
|*
|* static method to create Tabpage
|*
|************************************************************************/

IconChoicePage* SvxHyperlinkInternetTp::Create( vcl::Window* pWindow, IconChoiceDialog* pDlg, const SfxItemSet& rItemSet )
{
    return( new SvxHyperlinkInternetTp( pWindow, pDlg, rItemSet ) );
}

/*************************************************************************
|*
|* Set initial focus
|*
|************************************************************************/

void SvxHyperlinkInternetTp::SetInitFocus()
{
    m_pCbbTarget->GrabFocus();
}

/*************************************************************************
|*
|* Contents of editfield "Target" modified
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkInternetTp, ModifiedTargetHdl_Impl)
{
    OUString aScheme = GetSchemeFromURL( m_pCbbTarget->GetText() );
    if( !aScheme.isEmpty() )
        SetScheme( aScheme );

    // start timer
    maTimer.SetTimeout( 2500 );
    maTimer.Start();

    return( 0L );
}

/*************************************************************************
|*
|* If target-field was modify, to browse the new doc after timeout
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkInternetTp, TimeoutHdl_Impl)
{
    RefreshMarkWindow();
    return( 0L );
}

/*************************************************************************
|*
|* Contents of editfield "Login" modified
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkInternetTp, ModifiedLoginHdl_Impl)
{
    OUString aStrLogin ( m_pEdLogin->GetText() );
    if ( aStrLogin.equalsIgnoreAsciiCase( sAnonymous ) )
    {
        m_pCbAnonymous->Check();
        ClickAnonymousHdl_Impl(NULL);
    }

    return( 0L );
}

/*************************************************************************
|************************************************************************/

void SvxHyperlinkInternetTp::SetScheme(const OUString& rScheme)
{
    //if rScheme is empty or unknown the default beaviour is like it where HTTP
    bool bFTP = rScheme.startsWith(sFTPScheme);
    bool bInternet = !(bFTP);

    //update protocol button selection:
    m_pRbtLinktypFTP->Check(bFTP);
    m_pRbtLinktypInternet->Check(bInternet);

    //update target:
    RemoveImproperProtocol(rScheme);
    m_pCbbTarget->SetSmartProtocol( GetSmartProtocolFromButtons() );

    //show/hide  special fields for FTP:
    m_pFtLogin->Show( bFTP );
    m_pFtPassword->Show( bFTP );
    m_pEdLogin->Show( bFTP );
    m_pEdPassword->Show( bFTP );
    m_pCbAnonymous->Show( bFTP );

    //update 'link target in document'-window and opening-button
    if (rScheme.startsWith(sHTTPScheme) || rScheme.isEmpty())
    {
        if ( mbMarkWndOpen )
            ShowMarkWnd ();
    }
    else
    {
        //disable for https and ftp
        if ( mbMarkWndOpen )
            HideMarkWnd ();
    }
}

/*************************************************************************
|*
|* Remove protocol if it does not fit to the current button selection
|*
|************************************************************************/

void SvxHyperlinkInternetTp::RemoveImproperProtocol(const OUString& aProperScheme)
{
    OUString aStrURL ( m_pCbbTarget->GetText() );
    if ( aStrURL != aEmptyStr )
    {
        OUString aStrScheme(GetSchemeFromURL(aStrURL));
        if ( !aStrScheme.isEmpty() && aStrScheme != aProperScheme )
        {
            aStrURL = aStrURL.copy( aStrScheme.getLength() );
            m_pCbbTarget->SetText ( aStrURL );
        }
    }
}

OUString SvxHyperlinkInternetTp::GetSchemeFromButtons() const
{
    if( m_pRbtLinktypFTP->IsChecked() )
        return OUString(INET_FTP_SCHEME);
    return OUString(INET_HTTP_SCHEME);
}

INetProtocol SvxHyperlinkInternetTp::GetSmartProtocolFromButtons() const
{
    if( m_pRbtLinktypFTP->IsChecked() )
    {
        return INET_PROT_FTP;
    }
    return INET_PROT_HTTP;
}

/*************************************************************************
|*
|* Click on Radiobutton : Internet or FTP
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkInternetTp, Click_SmartProtocol_Impl)
{
    OUString aScheme = GetSchemeFromButtons();
    SetScheme(aScheme);
    return( 0L );
}

/*************************************************************************
|*
|* Click on Checkbox : Anonymous user
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkInternetTp, ClickAnonymousHdl_Impl)
{
    // disable login-editfields if checked
    if ( m_pCbAnonymous->IsChecked() )
    {
        if ( m_pEdLogin->GetText().toAsciiLowerCase().startsWith( sAnonymous ) )
        {
            maStrOldUser = aEmptyStr;
            maStrOldPassword = aEmptyStr;
        }
        else
        {
            maStrOldUser = m_pEdLogin->GetText();
            maStrOldPassword = m_pEdPassword->GetText();
        }

        setAnonymousFTPUser();
    }
    else
        setFTPUser(maStrOldUser, maStrOldPassword);

    return( 0L );
}

/*************************************************************************
|*
|* Combobox Target lost the focus
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkInternetTp, LostFocusTargetHdl_Impl)
{
    RefreshMarkWindow();
    return (0L);
}

/*************************************************************************
|*
|* Click on imagebutton : Browse
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkInternetTp, ClickBrowseHdl_Impl)
{

    // Open URL if available

    SfxStringItem aName( SID_FILE_NAME, OUString("http://") );
    SfxStringItem aRefererItem( SID_REFERER, OUString("private:user") );
    SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, true );
    SfxBoolItem aSilent( SID_SILENT, true );
    SfxBoolItem aReadOnly( SID_DOC_READONLY, true );

    SfxBoolItem aBrowse( SID_BROWSE, true );

    const SfxPoolItem *ppItems[] = { &aName, &aNewView, &aSilent, &aReadOnly, &aRefererItem, &aBrowse, NULL };
    (((SvxHpLinkDlg*)mpDialog)->GetBindings())->Execute( SID_OPENDOC, ppItems, 0, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );

    return( 0L );
}

void SvxHyperlinkInternetTp::RefreshMarkWindow()
{
    if ( m_pRbtLinktypInternet->IsChecked() && IsMarkWndVisible() )
    {
        EnterWait();
        OUString aStrURL( CreateAbsoluteURL() );
        if ( aStrURL != aEmptyStr )
            mpMarkWnd->RefreshTree ( aStrURL );
        else
            mpMarkWnd->SetError( LERR_DOCNOTOPEN );
        LeaveWait();
    }

}

/*************************************************************************
|*
|* Get String from Bookmark-Wnd
|*
|************************************************************************/

void SvxHyperlinkInternetTp::SetMarkStr ( const OUString& aStrMark )
{
    OUString aStrURL ( m_pCbbTarget->GetText() );

    const sal_Unicode sUHash = '#';
    sal_Int32 nPos = aStrURL.lastIndexOf( sUHash );

    if( nPos != -1 )
        aStrURL = aStrURL.copy(0, nPos);

    aStrURL += OUString(sUHash) + aStrMark;

    m_pCbbTarget->SetText ( aStrURL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
