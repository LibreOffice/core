/*************************************************************************
 *
 *  $RCSfile: hlinettp.cxx,v $
 *
 *  $Revision: 1.19 $
 *
 *  last change: $Author: rt $ $Date: 2003-04-24 14:46:33 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _ADRPARSE_HXX
#include <svtools/adrparse.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <svtools/pathoptions.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_USEROPTIONS_HXX
#include <svtools/useroptions.hxx>
#endif

#include "hlinettp.hxx"
#include "hyperdlg.hrc"

#define STD_DOC_SUBPATH     "internal"
#define STD_DOC_NAME        "url_transfer.htm"

sal_Char __READONLY_DATA sAnonymous[]    = "anonymous";
sal_Char __READONLY_DATA sHTTPScheme[]   = INET_HTTP_SCHEME;
sal_Char __READONLY_DATA sHTTPSScheme[]  = INET_HTTPS_SCHEME;
sal_Char __READONLY_DATA sFTPScheme[]    = INET_FTP_SCHEME;
sal_Char __READONLY_DATA sTelnetScheme[] = INET_TELNET_SCHEME;

/*************************************************************************
|*
|* Contructor / Destructor
|*
|************************************************************************/

SvxHyperlinkInternetTp::SvxHyperlinkInternetTp ( Window *pParent,
                                                 const SfxItemSet& rItemSet)
:   maGrpLinkTyp            ( this, ResId (GRP_LINKTYPE) ),
    maRbtLinktypInternet    ( this, ResId (RB_LINKTYP_INTERNET) ),
    maRbtLinktypFTP         ( this, ResId (RB_LINKTYP_FTP) ),
    maRbtLinktypTelnet      ( this, ResId (RB_LINKTYP_TELNET) ),
    maFtTarget              ( this, ResId (FT_TARGET_HTML) ),
    maCbbTarget             ( this, INET_PROT_HTTP ),
    maFtLogin               ( this, ResId (FT_LOGIN) ),
    maEdLogin               ( this, ResId (ED_LOGIN) ),
    maFtPassword            ( this, ResId (FT_PASSWD) ),
    maEdPassword            ( this, ResId (ED_PASSWD) ),
    maCbAnonymous           ( this, ResId (CBX_ANONYMOUS) ),
    maBtBrowse              ( this, ResId (BTN_BROWSE) ),
    maBtTarget              ( this, ResId (BTN_TARGET) ),
    SvxHyperlinkTabPageBase ( pParent, SVX_RES( RID_SVXPAGE_HYPERLINK_INTERNET ),
                              rItemSet ) ,
    mbMarkWndOpen           ( FALSE ),
    maStrStdDocURL          ()
{
    // Set HC bitmaps and display display of bitmap names.
    maBtBrowse.SetModeImage( Image( ResId( IMG_BROWSE_HC ) ), BMP_COLOR_HIGHCONTRAST );
    maBtBrowse.EnableTextDisplay (FALSE);
    maBtTarget.SetModeImage( Image( ResId( IMG_TARGET_HC ) ), BMP_COLOR_HIGHCONTRAST );
    maBtTarget.EnableTextDisplay (FALSE);

    InitStdControls();
    FreeResource();

    // Init URL-Box (pos&size, Open-Handler)
    maCbbTarget.SetPosSizePixel ( LogicToPixel( Point( COL_2, 25 ), MAP_APPFONT ),
                                  LogicToPixel( Size ( 176 - COL_DIFF, 60), MAP_APPFONT ) );
    maCbbTarget.Show();
    maCbbTarget.SetHelpId( HID_HYPERDLG_INET_PATH );

    // Find Path to Std-Doc
    String aStrBasePaths( SvtPathOptions().GetTemplatePath() );
    for( xub_StrLen n = 0; n < aStrBasePaths.GetTokenCount(); n++ )
    {
        INetURLObject aURL( aStrBasePaths.GetToken( n ) );
        aURL.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( STD_DOC_SUBPATH ) ) );
        aURL.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( STD_DOC_NAME ) ) );
        if ( FileExists( aURL ) )
        {
            maStrStdDocURL = aURL.GetMainURL( INetURLObject::NO_DECODE );
            break;
        }
    }
    SetExchangeSupport ();

    ///////////////////////////////////////
    // set defaults
    maRbtLinktypInternet.Check ();
    maFtLogin.Show( FALSE );
    maFtPassword.Show( FALSE );
    maEdLogin.Show( FALSE );
    maEdPassword.Show( FALSE );
    maCbAnonymous.Show( FALSE );
    maBtTarget.Enable( FALSE );
    maBtBrowse.Enable( maStrStdDocURL != aEmptyStr );

    ///////////////////////////////////////
    // overload handlers
    maRbtLinktypInternet.SetClickHdl( LINK ( this, SvxHyperlinkInternetTp, Click_SmartProtocol_Impl ) );
    maRbtLinktypFTP.SetClickHdl     ( LINK ( this, SvxHyperlinkInternetTp, Click_SmartProtocol_Impl ) );
    maRbtLinktypTelnet.SetClickHdl  ( LINK ( this, SvxHyperlinkInternetTp, Click_SmartProtocol_Impl ) );
    maCbAnonymous.SetClickHdl       ( LINK ( this, SvxHyperlinkInternetTp, ClickAnonymousHdl_Impl ) );
    maBtBrowse.SetClickHdl          ( LINK ( this, SvxHyperlinkInternetTp, ClickBrowseHdl_Impl ) );
    maBtTarget.SetClickHdl          ( LINK ( this, SvxHyperlinkInternetTp, ClickTargetHdl_Impl ) );
    maEdLogin.SetModifyHdl          ( LINK ( this, SvxHyperlinkInternetTp, ModifiedLoginHdl_Impl ) );

    maCbbTarget.SetLoseFocusHdl     ( LINK ( this, SvxHyperlinkInternetTp, LostFocusTargetHdl_Impl ) );
    maCbbTarget.SetModifyHdl        ( LINK ( this, SvxHyperlinkInternetTp, ModifiedTargetHdl_Impl ) );

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

void SvxHyperlinkInternetTp::FillDlgFields ( String& aStrURL )
{
    INetURLObject aURL( aStrURL );
    String aStrScheme = GetSchemeFromURL( aStrURL );

    // set additional controls for FTP: Username / Password
    if ( aStrScheme.SearchAscii( sFTPScheme ) == 0 )
    {
        if ( aURL.GetUser().ToLowerAscii().SearchAscii ( sAnonymous ) == 0 )
            setAnonymousFTPUser();
        else
            setFTPUser(aURL.GetUser(), aURL.GetPass());

        //do not show password and user in url
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
    maEdLogin.SetText( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM ( sAnonymous ) ) );
    SvAddressParser aAddress( SvtUserOptions().GetEmail() );
    maEdPassword.SetText( aAddress.Count() ? aAddress.GetEmailAddress(0) : String() );

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
    maCbAnonymous.Check(FALSE);
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
    String aScheme = GetSchemeFromURL(aStrURL);

    INetURLObject aURL(aStrURL);

    if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
    {
        aURL.SetSmartProtocol( GetSmartProtocolFromButtons() );
        aURL.SetSmartURL(aStrURL);
    }

    // username and password for ftp-url
    if( aURL.GetProtocol() == INET_PROT_FTP )
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
|* Contens of editfield "Taregt" modified
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, ModifiedTargetHdl_Impl, void *, EMPTYARG )
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
|* If target-field was modify, to browse the new doc afeter timeout
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, TimeoutHdl_Impl, Timer *, EMPTYARG )
{
    RefreshMarkWindow();
    return( 0L );
}

/*************************************************************************
|*
|* Contens of editfield "Login" modified
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, ModifiedLoginHdl_Impl, void *, EMPTYARG )
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

    BOOL bFTP = aScheme.SearchAscii( sFTPScheme ) == 0;
    BOOL bTelnet = FALSE;
    if( !bFTP )
        bTelnet = aScheme.SearchAscii( sTelnetScheme ) == 0;
    BOOL bInternet = !(bFTP || bTelnet);

    //update protocol button selection:
    maRbtLinktypFTP.Check(bFTP);
    maRbtLinktypTelnet.Check(bTelnet);
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
    if( aScheme.SearchAscii( sHTTPScheme ) == 0 )
    {
        maBtTarget.Enable();
        if ( mbMarkWndOpen )
            ShowMarkWnd ();
    }
    else
    {
        //disable for https, ftp and telnet
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
    {
        return String::CreateFromAscii( INET_FTP_SCHEME );
    }
    else if( maRbtLinktypTelnet.IsChecked() )
    {
        return String::CreateFromAscii( INET_TELNET_SCHEME );
    }
    return String::CreateFromAscii( INET_HTTP_SCHEME );
}

INetProtocol SvxHyperlinkInternetTp::GetSmartProtocolFromButtons() const
{
    if( maRbtLinktypFTP.IsChecked() )
    {
        return INET_PROT_FTP;
    }
    else if( maRbtLinktypTelnet.IsChecked() )
    {
        return INET_PROT_TELNET;
    }
    return INET_PROT_HTTP;
}

/*************************************************************************
|*
|* Click on Radiobutton : Internet, FTP or Telnet
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, Click_SmartProtocol_Impl, void*, EMPTYARG )
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

IMPL_LINK ( SvxHyperlinkInternetTp, ClickAnonymousHdl_Impl, void *, EMPTYARG )
{
    // disable login-editfields if checked
    if ( maCbAnonymous.IsChecked() )
    {
        if ( maEdLogin.GetText().ToLowerAscii().SearchAscii ( sAnonymous ) == 0 )
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

IMPL_LINK ( SvxHyperlinkInternetTp, LostFocusTargetHdl_Impl, void *, EMPTYARG )
{
    RefreshMarkWindow();
    return (0L);
}

/*************************************************************************
|*
|* Click on imagebutton : Browse
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, ClickBrowseHdl_Impl, void *, EMPTYARG )
{
    /////////////////////////////////////////////////
    // Open URL if available

    SfxStringItem aName( SID_FILE_NAME, maStrStdDocURL );
    SfxStringItem aRefererItem( SID_REFERER, UniString::CreateFromAscii(
                                RTL_CONSTASCII_STRINGPARAM( "private:user" ) ) );
    SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, TRUE );
    SfxBoolItem aSilent( SID_SILENT, TRUE );
    SfxBoolItem aReadOnly( SID_DOC_READONLY, TRUE );

    SfxBoolItem aBrowse( SID_BROWSE, TRUE );

    const SfxPoolItem *ppItems[] = { &aName, &aNewView, &aSilent, &aReadOnly, &aRefererItem, &aBrowse, NULL };
    (((SvxHpLinkDlg*)mpDialog)->GetBindings())->Execute( SID_OPENDOC, ppItems, 0, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD );

    return( 0L );
}

/*************************************************************************
|*
|* Click on imagebutton : Target
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, ClickTargetHdl_Impl, void *, EMPTYARG )
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

void SvxHyperlinkInternetTp::SetMarkStr ( String& aStrMark )
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

void SvxHyperlinkInternetTp::SetOnlineMode( BOOL bEnable )
{
    // State of target-button in subject to the current url-string
    // ( Can't display any targets in an document, if there is no
    //   valid url to a document )
    String aStrCurrentTarget( maCbbTarget.GetText() );
    aStrCurrentTarget.EraseTrailingChars();

    if( aStrCurrentTarget == aEmptyStr                ||
        aStrCurrentTarget.EqualsIgnoreCaseAscii( sHTTPScheme )  ||
        aStrCurrentTarget.EqualsIgnoreCaseAscii( sHTTPSScheme ) )
        maBtTarget.Enable( FALSE );
    else
        maBtTarget.Enable( TRUE );
}
