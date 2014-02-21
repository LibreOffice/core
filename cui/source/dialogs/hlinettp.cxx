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
#include "hyperdlg.hrc"
#include "hlmarkwn_def.hxx"

sal_Char const sAnonymous[]    = "anonymous";
sal_Char const sHTTPScheme[]   = INET_HTTP_SCHEME;
sal_Char const sFTPScheme[]    = INET_FTP_SCHEME;

/*************************************************************************
|*
|* Constructor / Destructor
|*
|************************************************************************/

SvxHyperlinkInternetTp::SvxHyperlinkInternetTp ( Window *pParent,
                                                 const SfxItemSet& rItemSet)
:   SvxHyperlinkTabPageBase ( pParent, CUI_RES( RID_SVXPAGE_HYPERLINK_INTERNET ),
                              rItemSet ) ,
    maGrpLinkTyp           ( this, CUI_RES (GRP_LINKTYPE) ),
    maRbtLinktypInternet    ( this, CUI_RES (RB_LINKTYP_INTERNET) ),
    maRbtLinktypFTP         ( this, CUI_RES (RB_LINKTYP_FTP) ),
    maFtTarget              ( this, CUI_RES (FT_TARGET_HTML) ),
    maCbbTarget             ( this, INET_PROT_HTTP ),
    maBtBrowse              ( this, CUI_RES (BTN_BROWSE) ),
    maFtLogin               ( this, CUI_RES (FT_LOGIN) ),
    maEdLogin               ( this, CUI_RES (ED_LOGIN) ),
    maFtPassword            ( this, CUI_RES (FT_PASSWD) ),
    maEdPassword            ( this, CUI_RES (ED_PASSWD) ),
    maCbAnonymous           ( this, CUI_RES (CBX_ANONYMOUS) ),
    mbMarkWndOpen           ( sal_False )
{
    // Disable display of bitmap names.
    maBtBrowse.EnableTextDisplay (false);

    InitStdControls();
    FreeResource();

    // Init URL-Box (pos&size, Open-Handler)
    maCbbTarget.SetPosSizePixel ( LogicToPixel( Point( COL_2, 25 ), MAP_APPFONT ),
                                  LogicToPixel( Size ( 176 - COL_DIFF, 60), MAP_APPFONT ) );
    maCbbTarget.Show();
    maCbbTarget.SetHelpId( HID_HYPERDLG_INET_PATH );

    SetExchangeSupport ();

    ///////////////////////////////////////
    // set defaults
    maRbtLinktypInternet.Check ();
    maFtLogin.Show( false );
    maFtPassword.Show( false );
    maEdLogin.Show( false );
    maEdPassword.Show( false );
    maCbAnonymous.Show( false );
    maBtBrowse.Enable( true );

    ///////////////////////////////////////
    // overload handlers
    Link aLink( LINK ( this, SvxHyperlinkInternetTp, Click_SmartProtocol_Impl ) );
    maRbtLinktypInternet.SetClickHdl( aLink );
    maRbtLinktypFTP.SetClickHdl     ( aLink );
    maCbAnonymous.SetClickHdl       ( LINK ( this, SvxHyperlinkInternetTp, ClickAnonymousHdl_Impl ) );
    maBtBrowse.SetClickHdl          ( LINK ( this, SvxHyperlinkInternetTp, ClickBrowseHdl_Impl ) );
    maEdLogin.SetModifyHdl          ( LINK ( this, SvxHyperlinkInternetTp, ModifiedLoginHdl_Impl ) );
    maCbbTarget.SetLoseFocusHdl     ( LINK ( this, SvxHyperlinkInternetTp, LostFocusTargetHdl_Impl ) );
    maCbbTarget.SetModifyHdl        ( LINK ( this, SvxHyperlinkInternetTp, ModifiedTargetHdl_Impl ) );
    maTimer.SetTimeoutHdl           ( LINK ( this, SvxHyperlinkInternetTp, TimeoutHdl_Impl ) );

    maFtTarget.SetAccessibleRelationMemberOf( &maGrpLinkTyp );
    maCbbTarget.SetAccessibleRelationMemberOf( &maGrpLinkTyp );
    maBtBrowse.SetAccessibleRelationMemberOf( &maGrpLinkTyp );
    maBtBrowse.SetAccessibleRelationLabeledBy( &maFtTarget );
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
        maCbbTarget.SetText( aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ) );
    else
        maCbbTarget.SetText(rStrURL); // #77696#

    SetScheme(aStrScheme);
}

void SvxHyperlinkInternetTp::setAnonymousFTPUser()
{
    maEdLogin.SetText(OUString(sAnonymous));
    SvAddressParser aAddress( SvtUserOptions().GetEmail() );
    maEdPassword.SetText( aAddress.Count() ? aAddress.GetEmailAddress(0) : OUString() );

    maFtLogin.Disable ();
    maFtPassword.Disable ();
    maEdLogin.Disable ();
    maEdPassword.Disable ();
    maCbAnonymous.Check();
}

void SvxHyperlinkInternetTp::setFTPUser(const OUString& rUser, const OUString& rPassword)
{
    maEdLogin.SetText ( rUser );
    maEdPassword.SetText ( rPassword );

    maFtLogin.Enable ();
    maFtPassword.Enable ();
    maEdLogin.Enable ();
    maEdPassword.Enable ();
    maCbAnonymous.Check(false);
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
    OUString aStrURL( maCbbTarget.GetText().trim() );

    INetURLObject aURL(aStrURL);

    if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        aURL.SetSmartProtocol( GetSmartProtocolFromButtons() );
        aURL.SetSmartURL(aStrURL);
    }

    // username and password for ftp-url
    if( aURL.GetProtocol() == INET_PROT_FTP && !maEdLogin.GetText().isEmpty() )
        aURL.SetUserAndPass ( maEdLogin.GetText(), maEdPassword.GetText() );

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

IconChoicePage* SvxHyperlinkInternetTp::Create( Window* pWindow, const SfxItemSet& rItemSet )
{
    return( new SvxHyperlinkInternetTp( pWindow, rItemSet ) );
}

/*************************************************************************
|*
|* Set initial focus
|*
|************************************************************************/

void SvxHyperlinkInternetTp::SetInitFocus()
{
    maCbbTarget.GrabFocus();
}

/*************************************************************************
|*
|* Contents of editfield "Target" modified
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkInternetTp, ModifiedTargetHdl_Impl)
{
    OUString aScheme = GetSchemeFromURL( maCbbTarget.GetText() );
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
    OUString aStrLogin ( maEdLogin.GetText() );
    if ( aStrLogin.equalsIgnoreAsciiCase( sAnonymous ) )
    {
        maCbAnonymous.Check();
        ClickAnonymousHdl_Impl(NULL);
    }

    return( 0L );
}

/*************************************************************************
|************************************************************************/

void SvxHyperlinkInternetTp::SetScheme(const OUString& rScheme)
{
    //if rScheme is empty or unknown the default beaviour is like it where HTTP
    sal_Bool bFTP = rScheme.startsWith(sFTPScheme);
    sal_Bool bInternet = !(bFTP);

    //update protocol button selection:
    maRbtLinktypFTP.Check(bFTP);
    maRbtLinktypInternet.Check(bInternet);

    //update target:
    RemoveImproperProtocol(rScheme);
    maCbbTarget.SetSmartProtocol( GetSmartProtocolFromButtons() );

    //show/hide  special fields for FTP:
    maFtLogin.Show( bFTP );
    maFtPassword.Show( bFTP );
    maEdLogin.Show( bFTP );
    maEdPassword.Show( bFTP );
    maCbAnonymous.Show( bFTP );

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
    OUString aStrURL ( maCbbTarget.GetText() );
    if ( aStrURL != aEmptyStr )
    {
        OUString aStrScheme(GetSchemeFromURL(aStrURL));
        if ( !aStrScheme.isEmpty() && aStrScheme != aProperScheme )
        {
            aStrURL = aStrURL.copy( aStrScheme.getLength() );
            maCbbTarget.SetText ( aStrURL );
        }
    }
}

OUString SvxHyperlinkInternetTp::GetSchemeFromButtons() const
{
    if( maRbtLinktypFTP.IsChecked() )
        return OUString(INET_FTP_SCHEME);
    return OUString(INET_HTTP_SCHEME);
}

INetProtocol SvxHyperlinkInternetTp::GetSmartProtocolFromButtons() const
{
    if( maRbtLinktypFTP.IsChecked() )
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
    if ( maCbAnonymous.IsChecked() )
    {
        if ( maEdLogin.GetText().toAsciiLowerCase().startsWith( sAnonymous ) )
        {
            maStrOldUser = aEmptyStr;
            maStrOldPassword = aEmptyStr;
        }
        else
        {
            maStrOldUser = maEdLogin.GetText();
            maStrOldPassword = maEdPassword.GetText();
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
    /////////////////////////////////////////////////
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
    if ( maRbtLinktypInternet.IsChecked() && IsMarkWndVisible() )
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
    OUString aStrURL ( maCbbTarget.GetText() );

    const sal_Unicode sUHash = '#';
    sal_Int32 nPos = aStrURL.lastIndexOf( sUHash );

    if( nPos != -1 )
        aStrURL = aStrURL.copy(0, nPos);

    aStrURL += OUString(sUHash) + aStrMark;

    maCbbTarget.SetText ( aStrURL );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
