/*************************************************************************
 *
 *  $RCSfile: hlinettp.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: pw $ $Date: 2000-11-22 13:40:45 $
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
sal_Char __READONLY_DATA sTelnet[]       = "telnet";
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
    InitStdControls();
    FreeResource();

    // Init URL-Box (pos&size, Open-Handler)
    maCbbTarget.SetPosSizePixel ( LogicToPixel( Point( 54, 25 ), MAP_APPFONT ),
                                  LogicToPixel( Size ( 176, 60), MAP_APPFONT ) );
    maCbbTarget.Show();
    maCbbTarget.SetHelpId( HID_HYPERDLG_INET_PATH );

    // Find Path to Std-Doc
    String aStrBasePaths( SvtPathOptions().GetTemplatePath() );
    INetURLObject aURL;
    aURL.SetSmartProtocol( INET_PROT_FILE );
    BOOL bFound = FALSE;
    for( xub_StrLen n = 0; n<aStrBasePaths.GetTokenCount() && !bFound; n++ )
    {
        String aTmp( aStrBasePaths.GetToken( n ) );
        aURL.SetSmartURL( aStrBasePaths.GetToken( n ) );

        aURL.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( STD_DOC_SUBPATH ) ) );
        aURL.Append( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( STD_DOC_NAME ) ) );
        bFound = FileExists( aURL );
    }
    if( bFound )
        maStrStdDocURL = aURL.GetFull();


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
    maBtBrowse.Enable( maStrStdDocURL==aEmptyStr?FALSE:TRUE);

    ///////////////////////////////////////
    // overload handlers
    maRbtLinktypInternet.SetClickHdl( LINK ( this, SvxHyperlinkInternetTp, ClickTypeInternetHdl_Impl ) );
    maRbtLinktypFTP.SetClickHdl     ( LINK ( this, SvxHyperlinkInternetTp, ClickTypeFTPHdl_Impl ) );
    maRbtLinktypTelnet.SetClickHdl  ( LINK ( this, SvxHyperlinkInternetTp, ClickTypeTelnetHdl_Impl ) );
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
    INetURLObject aURL ( aStrURL );
    String aStrScheme;

    // set protocoll-radiobuttons
    INetProtocol aProtocol = aURL.GetProtocol ();
    switch ( aProtocol )
    {
        case INET_PROT_HTTP :
            maRbtLinktypInternet.Check ();
            maRbtLinktypFTP.Check (FALSE);
            maRbtLinktypTelnet.Check (FALSE);

            maFtLogin.Show( FALSE );
            maFtPassword.Show( FALSE );
            maEdLogin.Show( FALSE );
            maEdPassword.Show( FALSE );
            maCbAnonymous.Show( FALSE );

            maBtTarget.Enable();

            if ( mbMarkWndOpen )
                ShowMarkWnd ();

            aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM( sHTTPScheme ) );

            maCbbTarget.SetSmartProtocol( INET_PROT_HTTP );

            break;
        case INET_PROT_HTTPS :
            maRbtLinktypInternet.Check ();
            maRbtLinktypFTP.Check (FALSE);
            maRbtLinktypTelnet.Check (FALSE);

            maFtLogin.Show( FALSE );
            maFtPassword.Show( FALSE );
            maEdLogin.Show( FALSE );
            maEdPassword.Show( FALSE );
            maCbAnonymous.Show( FALSE );

            maBtTarget.Enable();

            if ( mbMarkWndOpen )
                HideMarkWnd ();

            aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM( sHTTPSScheme ) );

            maCbbTarget.SetSmartProtocol( INET_PROT_HTTP );

            break;
        case INET_PROT_FTP :
            maRbtLinktypInternet.Check (FALSE);
            maRbtLinktypFTP.Check ();
            maRbtLinktypTelnet.Check (FALSE);

            maFtLogin.Show( TRUE );
            maFtPassword.Show( TRUE );
            maEdLogin.Show( TRUE );
            maEdPassword.Show( TRUE );
            maCbAnonymous.Show( TRUE );

            maBtTarget.Disable();

            if ( mbMarkWndOpen )
                HideMarkWnd ();

            aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM( sFTPScheme ) );

            maCbbTarget.SetSmartProtocol( INET_PROT_FTP );

            break;
        default :
            String aStrTmp ( aStrURL );
            if ( aStrTmp.ToLowerAscii().SearchAscii( sTelnet ) == 0 )
            {
                maRbtLinktypInternet.Check (FALSE);
                maRbtLinktypFTP.Check (FALSE);
                maRbtLinktypTelnet.Check ();

                maBtTarget.Disable();

            if ( mbMarkWndOpen )
                HideMarkWnd ();

                aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM( sTelnetScheme ) );

                maCbbTarget.SetSmartProtocol( INET_PROT_FILE );
            }
            else
            {   // as default : HTTP-Protocoll
                maRbtLinktypInternet.Check ();
                maRbtLinktypFTP.Check (FALSE);
                maRbtLinktypTelnet.Check (FALSE);
            }

            maFtLogin.Show( FALSE );
            maFtPassword.Show( FALSE );
            maEdLogin.Show( FALSE );
            maEdPassword.Show( FALSE );
            maCbAnonymous.Show( FALSE );
            break;
    }

    // Username / Password
    if ( aProtocol == INET_PROT_FTP )
    {
        String aUserName ( aURL.GetUser() );

        if ( aUserName.ToLowerAscii().SearchAscii ( sAnonymous ) == 0 )
        {
            maCbAnonymous.Check();
            maFtLogin.Disable ();
            maFtPassword.Disable ();

            maEdLogin.SetText ( String::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM( sAnonymous ) ) );
            SvAddressParser aAddress( SvtUserOptions().GetEmail() );
            maEdPassword.SetText( aAddress.Count() ? aAddress.GetEmailAddress(0) : String() );
        }
        else
        {
            maEdLogin.SetText ( aURL.GetUser() );
            maEdPassword.SetText ( aURL.GetPass() );
            maFtLogin.Enable ();
            maFtPassword.Enable ();
        }
    }

    // set URL-field
    if ( aStrScheme != aEmptyStr )
    {
        // Show the scheme, #72740
        maCbbTarget.SetText( aURL.GetMainURL() );
    }
    else
        maCbbTarget.SetText ( aEmptyStr );

    // State of target-button
    String aStrCurrentTarget( maCbbTarget.GetText() );
    aStrCurrentTarget.EraseTrailingChars();

    if( aStrCurrentTarget == aEmptyStr ||
        aStrCurrentTarget.EqualsIgnoreCaseAscii( sHTTPScheme ) ||
        aStrCurrentTarget.EqualsIgnoreCaseAscii( sHTTPSScheme ) )
        maBtTarget.Enable( FALSE );
    else
        maBtTarget.Enable( TRUE );

    maBtBrowse.Enable( maStrStdDocURL==aEmptyStr?FALSE:TRUE);
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
    String aStrScheme;

    // get data from dialog-controls
    aStrURL = maCbbTarget.GetText();

    if ( maRbtLinktypInternet.IsChecked() && aStrURL.SearchAscii( sHTTPScheme ) != 0 )
    {
        if ( aStrURL.SearchAscii( sHTTPSScheme ) != 0 )
            aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM ( sHTTPScheme ) );

    } else if ( maRbtLinktypFTP.IsChecked() && aStrURL.SearchAscii( sFTPScheme ) != 0 )
    {
        aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM ( sFTPScheme ) );

    } else if ( maRbtLinktypTelnet.IsChecked() && aStrURL.SearchAscii( sTelnetScheme ) != 0 )
    {
        aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM( sTelnetScheme ) );
    }

    String aStrTmp( aStrScheme );
    aStrTmp += aStrURL;
    INetURLObject aURL ( aStrTmp );

    // username and password for ftp-url
    if ( maRbtLinktypFTP.IsChecked() )
    {
        aURL.SetUserAndPass ( maEdLogin.GetText(), maEdPassword.GetText() );
    }

    // get data from standard-fields
    aStrIntName = mpEdText->GetText();
    aStrName    = mpEdIndication->GetText();
    aStrFrame   = mpCbbFrame->GetText();
    eMode       = (SvxLinkInsertMode) (mpLbForm->GetSelectEntryPos()+1);
    if( IsHTMLDoc() )
        eMode = (SvxLinkInsertMode) ( UINT16(eMode) | HLINK_HTMLMODE );

    if( aStrURL != aEmptyStr )
        aStrURL = aURL.GetMainURL();

    if( aStrName == aEmptyStr )
        aStrName = aStrURL;
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
|* Activate / Deactivate Tabpage
|*
|************************************************************************/

void SvxHyperlinkInternetTp::ActivatePage( const SfxItemSet& rItemSet )
{
    ///////////////////////////////////////
    // Set dialog-fields from input-itemset
    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       rItemSet.GetItem (SID_HYPERLINK_GETLINK);

    if ( pHyperlinkItem )
    {
        // standard-fields
        FillStandardDlgFields (pHyperlinkItem);

        maStrOldUser = maEdLogin.GetText();
        maStrOldPassword = maEdPassword.GetText();

        mbNewName = ( pHyperlinkItem->GetName() == aEmptyStr );
    }

    // show mark-window if it was open before
    if ( mbMarkWndOpen && maRbtLinktypInternet.IsChecked() )
        ShowMarkWnd ();

    maBtBrowse.Enable( maStrStdDocURL==aEmptyStr?FALSE:TRUE);
}

int SvxHyperlinkInternetTp::DeactivatePage( SfxItemSet* pSet)
{
    mbMarkWndOpen = IsMarkWndVisible ();
    HideMarkWnd ();

    String aStrURL, aStrName, aStrIntName, aStrFrame;
    SvxLinkInsertMode eMode;

    GetCurentItemData ( aStrURL, aStrName, aStrIntName, aStrFrame, eMode);

    USHORT nEvents = GetMacroEvents();
    SvxMacroTableDtor* pTable = GetMacroTable();

    SvxHyperlinkItem aItem( SID_HYPERLINK_GETLINK, aStrName, aStrURL, aStrFrame,
                            aStrIntName, eMode, nEvents, pTable );
    pSet->Put (aItem);

    return( LEAVE_PAGE );
}

/*************************************************************************
|*
|* Fill output-ItemSet
|*
|************************************************************************/

BOOL SvxHyperlinkInternetTp::FillItemSet( SfxItemSet& rOut)
{
    String aStrURL, aStrName, aStrIntName, aStrFrame;
    SvxLinkInsertMode eMode;

    GetCurentItemData ( aStrURL, aStrName, aStrIntName, aStrFrame, eMode);

    USHORT nEvents = GetMacroEvents();
    SvxMacroTableDtor* pTable = GetMacroTable();

    SvxHyperlinkItem aItem( SID_HYPERLINK_SETLINK, aStrName, aStrURL, aStrFrame,
                            aStrIntName, eMode, nEvents, pTable );
    rOut.Put (aItem);

    return TRUE;
}

/*************************************************************************
|*
|* Reset dialogfields
|*
|************************************************************************/

void SvxHyperlinkInternetTp::Reset( const SfxItemSet& rItemSet)
{
    ///////////////////////////////////////
    // Set dialog-fields from create-itemset
    maStrInitURL = aEmptyStr;

    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       rItemSet.GetItem (SID_HYPERLINK_GETLINK);

    if ( pHyperlinkItem )
    {
        // set dialog-fields
        FillStandardDlgFields (pHyperlinkItem);

        // set all other fields
        FillDlgFields ( (String&)pHyperlinkItem->GetURL() );

        // Store initial URL
        maStrInitURL = pHyperlinkItem->GetURL();

        mbNewName = ( pHyperlinkItem->GetName() == aEmptyStr );
    }
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
    String aStrCurrentTarget( maCbbTarget.GetText() );
    aStrCurrentTarget.EraseTrailingChars();

    if ( mbNewName )
    {
        mpEdIndication->SetText ( aStrCurrentTarget );
    }

    if( aStrCurrentTarget == aEmptyStr                ||
        aStrCurrentTarget.EqualsIgnoreCaseAscii( sHTTPScheme )  ||
        aStrCurrentTarget.EqualsIgnoreCaseAscii( sHTTPSScheme ) )
        maBtTarget.Enable( FALSE );
    else
        maBtTarget.Enable( TRUE );

    // changed scheme ? - Then change radiobutton-settings
    if( ( aStrCurrentTarget.SearchAscii( sHTTPScheme ) == 0 &&
          !maRbtLinktypInternet.IsChecked() ) ||
        ( aStrCurrentTarget.SearchAscii( sHTTPSScheme ) == 0 &&
          !maRbtLinktypInternet.IsChecked() )  )
    {
        maRbtLinktypInternet.Check();
        maRbtLinktypFTP.Check(FALSE);
        maRbtLinktypTelnet.Check(FALSE);

        maFtLogin.Show( FALSE );
        maFtPassword.Show( FALSE );
        maEdLogin.Show( FALSE );
        maEdPassword.Show( FALSE );
        maCbAnonymous.Show( FALSE );

        maBtTarget.Enable();

        if ( mbMarkWndOpen )
            ShowMarkWnd ();
    }
    else if( aStrCurrentTarget.SearchAscii( sFTPScheme ) == 0 &&
             !maRbtLinktypFTP.IsChecked() )
    {
        maRbtLinktypInternet.Check(FALSE);
        maRbtLinktypFTP.Check();
        maRbtLinktypTelnet.Check(FALSE);

        maFtLogin.Show( TRUE );
        maFtPassword.Show( TRUE );
        maEdLogin.Show( TRUE );
        maEdPassword.Show( TRUE );
        maCbAnonymous.Show( TRUE );

        maBtTarget.Disable();

        if ( mbMarkWndOpen )
            HideMarkWnd ();
    }
    else if( aStrCurrentTarget.SearchAscii( sTelnetScheme ) == 0 &&
             !maRbtLinktypTelnet.IsChecked() )
    {
        maRbtLinktypInternet.Check(FALSE);
        maRbtLinktypFTP.Check(FALSE);
        maRbtLinktypTelnet.Check();

        maFtLogin.Show( FALSE );
        maFtPassword.Show( FALSE );
        maEdLogin.Show( FALSE );
        maEdPassword.Show( FALSE );
        maCbAnonymous.Show( FALSE );

        maBtTarget.Disable();

        if ( mbMarkWndOpen )
            HideMarkWnd ();
    }

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
    if ( maRbtLinktypInternet.IsChecked() && IsMarkWndVisible() )
    {
        String aStrURL( maCbbTarget.GetText() );

        if ( !aStrURL.EqualsIgnoreCaseAscii( sHTTPScheme ) &&
             !aStrURL.EqualsIgnoreCaseAscii( sHTTPSScheme ) )
        {
            EnterWait();
            mpMarkWnd->RefreshTree ( aStrURL );
            LeaveWait();
        }
    }

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

    ModifiedTargetHdl_Impl (NULL);

    return( 0L );
}

/*************************************************************************
|*
|* Change Scheme-String
|*
|************************************************************************/

void SvxHyperlinkInternetTp::ChangeScheme ( String& aStrURL, String aStrNewScheme )
{
    if ( aStrURL != aEmptyStr )
    {
        INetURLObject aURL ( aStrURL );
        String aStrScheme;

        // set protocoll-radiobuttons
        INetProtocol aProtocol = aURL.GetProtocol ();
        switch ( aProtocol )
        {
            case INET_PROT_HTTP :
                aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM ( sHTTPScheme ) );
                break;
            case INET_PROT_HTTPS :
                aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM (sHTTPSScheme ) );
                break;
            case INET_PROT_FTP :
                aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM ( sFTPScheme ) );
                break;
            default :
                String aStrTmp ( aStrURL );

                if ( aStrTmp.ToLowerAscii().SearchAscii( sTelnet ) == 0 )
                {
                    aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM ( sTelnetScheme ) );
                }
        }

        if ( aStrScheme != aEmptyStr )
        {
            String aStrTmp( aStrURL.Erase ( 0, aStrScheme.Len() ) );
            aStrURL = aStrNewScheme;
            aStrURL += aStrTmp;
        }
    }
}

/*************************************************************************
|*
|* Click on Radiobutton : Type Internet
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, ClickTypeInternetHdl_Impl, void *, EMPTYARG )
{
    if ( maRbtLinktypInternet.IsChecked() )
    {
        maFtLogin.Show( FALSE );
        maFtPassword.Show( FALSE );
        maEdLogin.Show( FALSE );
        maEdPassword.Show( FALSE );
        maCbAnonymous.Show( FALSE );

        maBtTarget.Enable();

        if ( mbMarkWndOpen )
            ShowMarkWnd ();

        String aStrURL ( maCbbTarget.GetText() );
        ChangeScheme ( aStrURL, UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM ( sHTTPScheme ) ) );
        maCbbTarget.SetText ( aStrURL );
    }

    maCbbTarget.SetSmartProtocol( INET_PROT_HTTP );
    ModifiedTargetHdl_Impl (NULL);

    return( 0L );
}

/*************************************************************************
|*
|* Click on Radiobutton : Type FTP
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, ClickTypeFTPHdl_Impl, void *, EMPTYARG )
{
    if ( maRbtLinktypFTP.IsChecked() )
    {
        maFtLogin.Show( TRUE );
        maFtPassword.Show( TRUE );
        maEdLogin.Show( TRUE );
        maEdPassword.Show( TRUE );
        maCbAnonymous.Show( TRUE );

        maBtTarget.Disable();

        if ( mbMarkWndOpen )
            HideMarkWnd ();

        ClickAnonymousHdl_Impl(NULL);

        String aStrURL ( maCbbTarget.GetText() );
        ChangeScheme ( aStrURL,  UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM ( sFTPScheme ) ) );
        maCbbTarget.SetText ( aStrURL );
    }

    maCbbTarget.SetSmartProtocol( INET_PROT_FTP );
    ModifiedTargetHdl_Impl (NULL);

    return( 0L );
}

/*************************************************************************
|*
|* Click on Radiobutton : Type Telnet
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, ClickTypeTelnetHdl_Impl, void *, EMPTYARG )
{
    if ( maRbtLinktypTelnet.IsChecked() )
    {
        maFtLogin.Show( FALSE );
        maFtPassword.Show( FALSE );
        maEdLogin.Show( FALSE );
        maEdPassword.Show( FALSE );
        maCbAnonymous.Show( FALSE );

        maBtTarget.Disable();

        if ( mbMarkWndOpen )
            HideMarkWnd ();

        String aStrURL ( maCbbTarget.GetText() );
        ChangeScheme ( aStrURL,  UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM ( sTelnetScheme ) ) );
        maCbbTarget.SetText ( aStrURL );
    }

    maCbbTarget.SetSmartProtocol( INET_PROT_FILE );
    ModifiedTargetHdl_Impl(NULL);

    return( 0L );
}

/*************************************************************************
|*
|* Click on Checkbox : Anonymous user
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, ClickAnonymousHdl_Impl, void *, EMPTYARG )
{
    // change username & password
    String aStrUser ( maEdLogin.GetText() );
    String aStrPassword ( maEdPassword.GetText() );

    if( maStrOldUser.EqualsIgnoreCaseAscii( sAnonymous ) )
    {
        maEdLogin.SetText( aEmptyStr );
        maEdPassword.SetText( aEmptyStr );
    }
    else
    {
        maEdLogin.SetText ( maStrOldUser );
        maEdPassword.SetText ( maStrOldPassword );
    }

    maStrOldUser = aStrUser;
    maStrOldPassword = aStrPassword;

    // disable login-editfields if checked
    if ( maCbAnonymous.IsChecked() )
    {
        SvAddressParser aAddress( SvtUserOptions().GetEmail() );
        maEdLogin.SetText( UniString::CreateFromAscii( RTL_CONSTASCII_STRINGPARAM ( sAnonymous ) ) );
        maEdPassword.SetText( aAddress.Count() ? aAddress.GetEmailAddress(0) : String() );

        maFtLogin.Disable ();
        maFtPassword.Disable ();
        maEdLogin.Disable ();
        maEdPassword.Disable ();
    }
    else
    {
        maFtLogin.Enable ();
        maFtPassword.Enable ();
        maEdLogin.Enable ();
        maEdPassword.Enable ();
    }

    ModifiedTargetHdl_Impl(NULL);

    return( 0L );
}

/*************************************************************************
|*
|* Combobox Target lost the focus
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, LostFocusTargetHdl_Impl, void *, EMPTYARG )
{
    String aStrURL ( maCbbTarget.GetText() );
    String aStrScheme;

    if( maRbtLinktypInternet.IsChecked() &&
         aStrURL.SearchAscii( sHTTPScheme ) != 0 )
    {
        if( aStrURL.SearchAscii( sHTTPSScheme ) != 0 )
            aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM ( sHTTPScheme ) );
    } else if( maRbtLinktypFTP.IsChecked() &&
               aStrURL.SearchAscii( sFTPScheme ) != 0 )
    {
        aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM ( sFTPScheme ) );
    } else if( maRbtLinktypTelnet.IsChecked() &&
               aStrURL.SearchAscii( sTelnetScheme ) != 0 )
    {
        aStrScheme.AssignAscii( RTL_CONSTASCII_STRINGPARAM ( sTelnetScheme ) );
    }

    if( aStrURL != aEmptyStr )
    {
        String aStrTarget ( aStrScheme );
        aStrTarget += aStrURL;
        maCbbTarget.SetText ( aStrTarget );
    }

    if ( maRbtLinktypInternet.IsChecked() && aStrURL.Len()!=0 && IsMarkWndVisible() )
    {
        EnterWait();
        mpMarkWnd->RefreshTree ( maCbbTarget.GetText() );
        LeaveWait();
    }

    ModifiedTargetHdl_Impl ( NULL );

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

    GetDispatcher()->Execute(   SID_OPENDOC,
                                SFX_CALLMODE_ASYNCHRON |
                                SFX_CALLMODE_RECORD,
                                &aName,
                                &aNewView,
                                &aSilent,
                                &aReadOnly,
                                &aRefererItem,
                                0L );

    return( 0L );
}

/*************************************************************************
|*
|* Click on imagebutton : Target
|*
|************************************************************************/

IMPL_LINK ( SvxHyperlinkInternetTp, ClickTargetHdl_Impl, void *, EMPTYARG )
{
    if ( maRbtLinktypInternet.IsChecked() )
    {
        String aStrURL( maCbbTarget.GetText() );
        EnterWait();
        if ( !aStrURL.EqualsIgnoreCaseAscii( sHTTPScheme )  &&
             !aStrURL.EqualsIgnoreCaseAscii(sHTTPSScheme ) &&
             aStrURL != aEmptyStr )
            mpMarkWnd->RefreshTree ( aStrURL );
        else
            mpMarkWnd->SetError( LERR_DOCNOTOPEN );
        LeaveWait();
    }

    ShowMarkWnd ();

    mbMarkWndOpen = IsMarkWndVisible ();

    return( 0L );
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


