/*************************************************************************
 *
 *  $RCSfile: hyperdlg.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pw $ $Date: 2000-10-17 08:50:08 $
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

// include ---------------------------------------------------------------

#ifndef _SV_SETTINGS_HXX
#include <vcl/settings.hxx>
#endif

#include "hyperdlg.hxx"

#include "hlinettp.hxx"
#include "hlmailtp.hxx"
#include "hldoctp.hxx"
#include "hldocntp.hxx"

#include "hyperdlg.hrc"

#define INET_PROT_TELNET_PRIVATE    INET_PROT_HTTP

//########################################################################
//#                                                                      #
//# Childwindow-Wrapper-Class                                            #
//#                                                                      #
//########################################################################

SvxHlinkCtrl::SvxHlinkCtrl( USHORT nId, SfxBindings & rBindings, SvxHpLinkDlg* pDlg)
: SfxControllerItem (nId, rBindings),
  aForwarder        ( SID_INTERNET_ONLINE, *this )
{
    pParent = pDlg;
}

void SvxHlinkCtrl::StateChanged( USHORT nSID, SfxItemState eState,
                                 const SfxPoolItem* pState )
{
    if ( nSID == SID_INTERNET_ONLINE && SFX_ITEM_AVAILABLE == eState )
    {
        pParent->EnableInetBrowse( !( (SfxBoolItem*)pState)->GetValue() );
    }
    else
    {
        if ( nSID == SID_HYPERLINK_GETLINK && SFX_ITEM_AVAILABLE == eState )
        {
            pParent->SetPage ( (SvxHyperlinkItem*)pState);
        }
    }
}

//########################################################################
//#                                                                      #
//# Childwindow-Wrapper-Class                                            #
//#                                                                      #
//########################################################################

SFX_IMPL_CHILDWINDOW(SvxHlinkDlgWrapper, SID_HYPERLINK_DIALOG)

// -----------------------------------------------------------------------

struct MyStruct
{
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > xFrame;
    SfxChildWinFactory* pFact;
    sal_Bool                bHideNotDelete;
    sal_Bool                bVisible;
    sal_Bool                bHideAtToggle;
    SfxModule*          pContextModule;
    SfxWorkWindow*      pWorkWin;
};

SvxHlinkDlgWrapper::SvxHlinkDlgWrapper( Window* pParent, USHORT nId,
                                                SfxBindings* pBindings,
                                                SfxChildWinInfo* pInfo ) :
    SfxChildWindow( pParent, nId )

{
    pWindow = new SvxHpLinkDlg( pParent, pBindings );
    ((MyStruct*)pImp)->bVisible = FALSE;

    if ( pInfo->aSize.Width() != 0 && pInfo->aSize.Height() != 0 )
    {
        Size aParentSize( SFX_APP()->GetTopWindow()->GetSizePixel() );
        Size aDlgSize ( GetSizePixel () );

        if( aParentSize.Width() < pInfo->aPos.X() )
            pInfo->aPos.setX( aParentSize.Width()-aDlgSize.Width() < 0.1*aParentSize.Width() ?
                              0.1*aParentSize.Width() : aParentSize.Width()-aDlgSize.Width() );
        if( aParentSize.Height() < pInfo->aPos. Y() )
            pInfo->aPos.setY( aParentSize.Height()-aDlgSize.Height() < 0.1*aParentSize.Height() ?
                              0.1*aParentSize.Height() : aParentSize.Height()-aDlgSize.Height() );

        pWindow->SetPosPixel( pInfo->aPos );
    }

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    SetHideNotDelete( TRUE );
}

SfxChildWinInfo SvxHlinkDlgWrapper::GetInfo() const
{
    return SfxChildWindow::GetInfo();
}

//########################################################################
//#                                                                      #
//# Hyperlink - Dialog                                                   #
//#                                                                      #
//########################################################################

/*************************************************************************
|*
|* Contructor / Destructor
|*
|************************************************************************/

SvxHpLinkDlg::SvxHpLinkDlg (Window* pParent, SfxBindings* pBindings)
:   IconChoiceDialog( pParent, SVX_RES ( RID_SVXDLG_NEWHYPERLINK ) ),
    maCtrl          ( SID_HYPERLINK_GETLINK, *pBindings, this ),
    mpBindings      ( pBindings ),
    mbIsHTMLDoc     ( FALSE )
{
    // set background of iconchoicectrl
    const StyleSettings& rStyles = Application::GetSettings().GetStyleSettings();
    SetCtrlColor ( rStyles.GetWorkspaceColor() );

    // insert pages
    Image aImage;
    String aStrTitle;

    aStrTitle = SVX_RESSTR( RID_SVXSTR_HYPERDLG_HLINETTP );
    aImage = Image( SVX_RES ( RID_SVXBMP_HLINETTP ) );
    AddTabPage ( RID_SVXPAGE_HYPERLINK_INTERNET, aStrTitle, aImage, SvxHyperlinkInternetTp::Create );
    aStrTitle = SVX_RESSTR( RID_SVXSTR_HYPERDLG_HLMAILTP );
    aImage = Image( SVX_RES ( RID_SVXBMP_HLMAILTP ) );
    AddTabPage ( RID_SVXPAGE_HYPERLINK_MAIL, aStrTitle, aImage, SvxHyperlinkMailTp::Create );
    aStrTitle = SVX_RESSTR( RID_SVXSTR_HYPERDLG_HLDOCTP );
    aImage = Image( SVX_RES ( RID_SVXBMP_HLDOCTP ) );
    AddTabPage ( RID_SVXPAGE_HYPERLINK_DOCUMENT, aStrTitle, aImage, SvxHyperlinkDocTp::Create );
    aStrTitle = SVX_RESSTR( RID_SVXSTR_HYPERDLG_HLDOCNTP );
    aImage = Image( SVX_RES ( RID_SVXBMP_HLDOCNTP ) );
    AddTabPage ( RID_SVXPAGE_HYPERLINK_NEWDOCUMENT, aStrTitle, aImage, SvxHyperlinkNewDocTp::Create );

    // create itemset for tabpages
    mpItemSet = new SfxItemSet( SFX_APP()->GetPool(), SID_HYPERLINK_GETLINK,
                               SID_HYPERLINK_SETLINK );

    SvxHyperlinkItem aItem;
    mpItemSet->Put (aItem, SID_HYPERLINK_GETLINK);

    SetInputSet (mpItemSet);

    // Init Dialog
    Start (FALSE);

    // set OK/Cancel - button
    GetOKButton().SetText ( SVX_RESSTR(RID_SVXSTR_HYPDLG_APPLYBUT) );
    GetCancelButton().SetText ( SVX_RESSTR(RID_SVXSTR_HYPDLG_CLOSEBUT) );

    GetOKButton().SetClickHdl    ( LINK ( this, SvxHpLinkDlg, ClickApplyHdl_Impl ) );
    GetCancelButton().SetClickHdl( LINK ( this, SvxHpLinkDlg, ClickCloseHdl_Impl ) );
}

SvxHpLinkDlg::~SvxHpLinkDlg ()
{
    String aStrEntry = UniString::CreateFromInt32( SID_HYPERLINK_DIALOG );
    aStrEntry += String ( RTL_CONSTASCII_USTRINGPARAM( "Window" ) );
    SFX_APP()->GetIniManager()->Delete( SFX_GROUP_VIEW, aStrEntry );

    delete mpItemSet;
}

/*************************************************************************
|*
|* Close Dialog-Window
|*
|************************************************************************/

BOOL SvxHpLinkDlg::Close()
{
    GetDispatcher()->Execute( SID_HYPERLINK_DIALOG,
                              SFX_CALLMODE_ASYNCHRON |
                              SFX_CALLMODE_RECORD);
    return TRUE;
}

/*************************************************************************
|*
|* When extrawindow is visible and its never moved by user, then move that
|* window, too.
|*
|************************************************************************/

void SvxHpLinkDlg::Move()
{
    SvxHyperlinkTabPageBase* pCurrentPage = ( SvxHyperlinkTabPageBase* )
                                            GetTabPage ( GetCurPageId() );

    if( pCurrentPage->IsMarkWndVisible () )
    {
        // Pos&Size of this dialog-window
        Point aDlgPos ( GetPosPixel () );
        Size aDlgSize ( GetSizePixel () );

        // Size of Office-Main-Window
        Size aWindowSize( SFX_APP()->GetTopWindow()->GetSizePixel() );

        // Size of Extrawindow
        Size aExtraWndSize( pCurrentPage->GetSizeExtraWnd() );

        BOOL bDoInvalid ;
        if( aDlgPos.X()+(1.02*aDlgSize.Width())+aExtraWndSize.Width() > aWindowSize.Width() )
        {
            if( aDlgPos.X() - ( 0.02*aDlgSize.Width() ) - aExtraWndSize.Width() < 0 )
            {
                // Pos Extrawindow anywhere
                bDoInvalid = pCurrentPage->MoveToExtraWnd( Point( 1, 1.1*aDlgPos.Y() ), TRUE );
            }
            else
            {
                // Pos Extrawindow on the left side of Dialog
                bDoInvalid = pCurrentPage->MoveToExtraWnd( aDlgPos -
                                                           Point( 0.02*aDlgSize.Width(), 0 ) -
                                                           Point( aExtraWndSize.Width(), 0 ) );
            }
        }
        else
        {
            // Pos Extrawindow on the right side of Dialog
            bDoInvalid = pCurrentPage->MoveToExtraWnd ( aDlgPos + Point( 1.02*aDlgSize.Width(), 0 ) );
        }

        if ( bDoInvalid )
            Invalidate();
    }

    Window::Move();
}

/*************************************************************************
|*
|* Click on Apply-button
|*
|************************************************************************/

IMPL_LINK ( SvxHpLinkDlg, ClickApplyHdl_Impl, void *, EMPTYARG )
{
    SfxItemSet aItemSet( SFX_APP()->GetPool(), SID_HYPERLINK_GETLINK,
                         SID_HYPERLINK_SETLINK );

    SvxHyperlinkTabPageBase* pCurrentPage = (SvxHyperlinkTabPageBase*)
                                            GetTabPage ( GetCurPageId() );

    if ( pCurrentPage->AskApply() )
    {
        pCurrentPage->FillItemSet( aItemSet );

        SvxHyperlinkItem *aItem = (SvxHyperlinkItem *)
                                  aItemSet.GetItem (SID_HYPERLINK_SETLINK);

        String aStrEmpty;
        if ( aItem->GetURL() != aStrEmpty )
            GetDispatcher()->Execute( SID_HYPERLINK_SETLINK, SFX_CALLMODE_ASYNCHRON |
                                      SFX_CALLMODE_RECORD, aItem, 0L);

        ( (SvxHyperlinkTabPageBase*)GetTabPage ( GetCurPageId() ) )->DoApply();
    }

    return( 0L );
}

/*************************************************************************
|*
|* Click on Close-button
|*
|************************************************************************/

IMPL_LINK ( SvxHpLinkDlg, ClickCloseHdl_Impl, void *, EMPTYARG )
{
    Close();

    return( 0L );
}

/*************************************************************************
|*
|* Set Page
|*
|************************************************************************/

USHORT SvxHpLinkDlg::SetPage ( SvxHyperlinkItem* pItem )
{
    USHORT nPageId = RID_SVXPAGE_HYPERLINK_INTERNET;

    String aStrURL ( pItem->GetURL() );
    INetURLObject aURL ( aStrURL );
    INetProtocol eProtocolTyp = aURL.GetProtocol();

    switch ( eProtocolTyp )
    {
        case INET_PROT_HTTP :
        case INET_PROT_FTP :
            nPageId = RID_SVXPAGE_HYPERLINK_INTERNET;
            break;
        case INET_PROT_FILE :
        case INET_PROT_POP3 :
        case INET_PROT_IMAP :
            nPageId = RID_SVXPAGE_HYPERLINK_DOCUMENT;
            break;
        case INET_PROT_MAILTO :
        case INET_PROT_NEWS :
            nPageId = RID_SVXPAGE_HYPERLINK_MAIL;
            break;
        default :
            sal_Char const sTelnet[] = "telnet";
            sal_Char const sNewsSrvScheme[] = INET_NEWS_SRV_SCHEME;

            if ( aStrURL.ToLowerAscii().SearchAscii( sTelnet ) == 0 )
            {
                nPageId = RID_SVXPAGE_HYPERLINK_INTERNET;
                eProtocolTyp = INET_PROT_TELNET_PRIVATE;
            }
            else if ( aStrURL.SearchAscii( sNewsSrvScheme ) == 0 )
                nPageId = RID_SVXPAGE_HYPERLINK_DOCUMENT;
            else
            {
                sal_Char const sHash[] = "#";
                if( aStrURL.SearchAscii( sHash ) == 0 )
                    nPageId = RID_SVXPAGE_HYPERLINK_DOCUMENT;
                else
                {
                    eProtocolTyp = INET_PROT_NOT_VALID;
                    nPageId = GetCurPageId();
                }
            }
            break;
    }

    ShowPage (nPageId);

    SvxHyperlinkTabPageBase* pCurrentPage = (SvxHyperlinkTabPageBase*)GetTabPage( nPageId );

    mbIsHTMLDoc = BOOL( pItem->GetInsertMode() & HLINK_HTMLMODE );

    SfxItemSet& aPageSet =  (SfxItemSet&)GetTabPage (nPageId)->GetItemSet ();
    aPageSet.Put ( *pItem );

    pCurrentPage->Reset( aPageSet );
    pCurrentPage->SetInitFocus();

    return nPageId;
}

/*************************************************************************
|*
|* Enable/Disable to browse targets in a html-doc
|*
|************************************************************************/

void SvxHpLinkDlg::EnableInetBrowse( BOOL bEnable )
{
    SvxHyperlinkTabPageBase* pCurrentPage = ( SvxHyperlinkTabPageBase* )
                                            GetTabPage ( GetCurPageId() );
    pCurrentPage->SetOnlineMode( bEnable );
}

