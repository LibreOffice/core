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

#include <comphelper/string.hxx>
#include <unotools/pathoptions.hxx>
#include <unotools/useroptions.hxx>
#include <svl/adrparse.hxx>

#include "hlinettp.hxx"
#include "hyperdlg.hrc"
#include "hlmarkwn_def.hxx"

sal_Char const sAnonymous[]    = "anonymous";
sal_Char const sHTTPScheme[]   = INET_HTTP_SCHEME;
sal_Char const sHTTPSScheme[]  = INET_HTTPS_SCHEME;
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
    maBtTarget              ( this, CUI_RES (BTN_TARGET) ),
    maFtPassword            ( this, CUI_RES (FT_PASSWD) ),
    maEdPassword            ( this, CUI_RES (ED_PASSWD) ),
    maCbAnonymous           ( this, CUI_RES (CBX_ANONYMOUS) ),
    mbMarkWndOpen           ( sal_False )
{
    // Disable display of bitmap names.
    maBtBrowse.EnableTextDisplay (sal_False);
    maBtTarget.EnableTextDisplay (sal_False);

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
    maFtLogin.Show( sal_False );
    maFtPassword.Show( sal_False );
    maEdLogin.Show( sal_False );
    maEdPassword.Show( sal_False );
    maCbAnonymous.Show( sal_False );
    maBtTarget.Enable( sal_False );
    maBtBrowse.Enable( sal_True );

    ///////////////////////////////////////
    // overload handlers
    Link aLink( LINK ( this, SvxHyperlinkInternetTp, Click_SmartProtocol_Impl ) );
    maRbtLinktypInternet.SetClickHdl( aLink );
    maRbtLinktypFTP.SetClickHdl     ( aLink );
    maCbAnonymous.SetClickHdl       ( LINK ( this, SvxHyperlinkInternetTp, ClickAnonymousHdl_Impl ) );
    maBtBrowse.SetClickHdl          ( LINK ( this, SvxHyperlinkInternetTp, ClickBrowseHdl_Impl ) );
    maBtTarget.SetClickHdl          ( LINK ( this, SvxHyperlinkInternetTp, ClickTargetHdl_Impl ) );
    maEdLogin.SetModifyHdl          ( LINK ( this, SvxHyperlinkInternetTp, ModifiedLoginHdl_Impl ) );
    maCbbTarget.SetLoseFocusHdl     ( LINK ( this, SvxHyperlinkInternetTp, LostFocusTargetHdl_Impl ) );
    maCbbTarget.SetModifyHdl        ( LINK ( this, SvxHyperlinkInternetTp, ModifiedTargetHdl_Impl ) );
    maTimer.SetTimeoutHdl           ( LINK ( this, SvxHyperlinkInternetTp, TimeoutHdl_Impl ) );

    maFtTarget.SetAccessibleRelationMemberOf( &maGrpLinkTyp );
    maCbbTarget.SetAccessibleRelationMemberOf( &maGrpLinkTyp );
    maBtTarget.SetAccessibleRelationMemberOf( &maGrpLinkTyp );
    maBtTarget.SetAccessibleRelationLabeledBy( &maFtTarget );
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

void SvxHyperlinkInternetTp::FillDlgFields ( String& aStrURL )
{
    INetURLObject aURL( aStrURL );
    String aStrScheme = GetSchemeFromURL( aStrURL );

    // set additional controls for FTP: Username / Password
    if ( aStrScheme.SearchAscii( sFTPScheme ) == 0 )
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
        maCbbTarget.SetText( aStrURL ); // #77696#

    SetScheme( aStrScheme );
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

void SvxHyperlinkInternetTp::setFTPUser(const String& rUser, const String& rPassword)
{
    maEdLogin.SetText ( rUser );
    maEdPassword.SetText ( rPassword );

    maFtLogin.Enable ();
    maFtPassword.Enable ();
    maEdLogin.Enable ();
    maEdPassword.Enable ();
    maCbAnonymous.Check(sal_False);
}

/*************************************************************************
|*
|* retrieve and prepare data from dialog-fields
|*
|************************************************************************/

void SvxHyperlinkInternetTp::GetCurentItemData ( String& aStrURL, String& aStrName,
                                                 String& aStrIntName, String& aStrFrame,
                                                 SvxLinkInsertMode& eMode )
{
    aStrURL = CreateAbsoluteURL();
    GetDataFromCommonFields( aStrName, aStrIntName, aStrFrame, eMode );
}

String SvxHyperlinkInternetTp::CreateAbsoluteURL() const
{
    String aStrURL = maCbbTarget.GetText();

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
    String aScheme = GetSchemeFromURL( maCbbTarget.GetText() );
    if(aScheme.Len()!=0)
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
    String aStrLogin ( maEdLogin.GetText() );
    if ( aStrLogin.EqualsIgnoreCaseAscii( sAnonymous ) )
    {
        maCbAnonymous.Check();
        ClickAnonymousHdl_Impl(NULL);
    }

    return( 0L );
}

/*************************************************************************
|************************************************************************/

void SvxHyperlinkInternetTp::SetScheme( const String& aScheme )
{
    //if  aScheme is empty or unknown the default beaviour is like it where HTTP

    sal_Bool bFTP = aScheme.SearchAscii( sFTPScheme ) == 0;
    sal_Bool bInternet = !(bFTP);

    //update protocol button selection:
    maRbtLinktypFTP.Check(bFTP);
    maRbtLinktypInternet.Check(bInternet);

    //update target:
    RemoveImproperProtocol(aScheme);
    maCbbTarget.SetSmartProtocol( GetSmartProtocolFromButtons() );

    //show/hide  special fields for FTP:
    maFtLogin.Show( bFTP );
    maFtPassword.Show( bFTP );
    maEdLogin.Show( bFTP );
    maEdPassword.Show( bFTP );
    maCbAnonymous.Show( bFTP );

    //update 'link target in document'-window and opening-button
    if( aScheme.SearchAscii( sHTTPScheme ) == 0 || aScheme.Len() == 0 )
    {
        maBtTarget.Enable();
        if ( mbMarkWndOpen )
            ShowMarkWnd ();
    }
    else
    {
        //disable for https and ftp
        maBtTarget.Disable();
        if ( mbMarkWndOpen )
            HideMarkWnd ();
    }
}

/*************************************************************************
|*
|* Remove protocol if it does not fit to the current button selection
|*
|************************************************************************/

void SvxHyperlinkInternetTp::RemoveImproperProtocol(const String& aProperScheme)
{
    String aStrURL ( maCbbTarget.GetText() );
    if ( aStrURL != aEmptyStr )
    {
        String aStrScheme = GetSchemeFromURL( aStrURL );
        if ( aStrScheme != aEmptyStr && aStrScheme != aProperScheme )
        {
            aStrURL.Erase ( 0, aStrScheme.Len() );
            maCbbTarget.SetText ( aStrURL );
        }
    }
}

String SvxHyperlinkInternetTp::GetSchemeFromButtons() const
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
    String aScheme = GetSchemeFromButtons();
    SetScheme( aScheme );
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
        if ( maEdLogin.GetText().toAsciiLowerCase().indexOf ( sAnonymous ) == 0 )
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
    SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, sal_True );
    SfxBoolItem aSilent( SID_SILENT, sal_True );
    SfxBoolItem aReadOnly( SID_DOC_READONLY, sal_True );

    SfxBoolItem aBrowse( SID_BROWSE, sal_True );

    const SfxPoolItem *ppItems[] = { &aName, &aNewView, &aSilent, &aReadOnly, &aRefererItem, &aBrowse, NULL };
    (((SvxHpLinkDlg*)mpDialog)->GetBindings())->Execute( SID_OPENDOC, ppItems, 0, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );

    return( 0L );
}

/*************************************************************************
|*
|* Click on imagebutton : Target
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHyperlinkInternetTp, ClickTargetHdl_Impl)
{
    RefreshMarkWindow();
    ShowMarkWnd ();
    mbMarkWndOpen = IsMarkWndVisible ();

    return( 0L );
}

void SvxHyperlinkInternetTp::RefreshMarkWindow()
{
    if ( maRbtLinktypInternet.IsChecked() && IsMarkWndVisible() )
    {
        EnterWait();
        String aStrURL( CreateAbsoluteURL() );
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

void SvxHyperlinkInternetTp::SetMarkStr ( const String& aStrMark )
{
    String aStrURL ( maCbbTarget.GetText() );

    const sal_Unicode sUHash = '#';
    xub_StrLen nPos = aStrURL.SearchBackward( sUHash );

    if( nPos != STRING_NOTFOUND )
        aStrURL.Erase ( nPos );

    aStrURL += sUHash;
    aStrURL += aStrMark;

    maCbbTarget.SetText ( aStrURL );
}

/*************************************************************************
|*
|* Enable Browse-Button in subject to the office is in onlinemode
|*
|************************************************************************/

void SvxHyperlinkInternetTp::SetOnlineMode( sal_Bool /*bEnable*/ )
{
    // State of target-button in subject to the current url-string
    // ( Can't display any targets in an document, if there is no
    //   valid url to a document )
    String aStrCurrentTarget(comphelper::string::stripEnd(maCbbTarget.GetText(), ' '));

    if( aStrCurrentTarget == aEmptyStr                ||
        aStrCurrentTarget.EqualsIgnoreCaseAscii( sHTTPScheme )  ||
        aStrCurrentTarget.EqualsIgnoreCaseAscii( sHTTPSScheme ) )
        maBtTarget.Enable( sal_False );
    else
        maBtTarget.Enable( sal_True );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
