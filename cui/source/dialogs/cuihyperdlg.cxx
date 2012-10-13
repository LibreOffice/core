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

#include <vcl/settings.hxx>
#include <unotools/viewoptions.hxx>
#include "cuihyperdlg.hxx"
#include "hlinettp.hxx"
#include "hlmailtp.hxx"
#include "hldoctp.hxx"
#include "hldocntp.hxx"
#include "hyperdlg.hrc"
#include <svx/svxids.hrc> // SID_READONLY_MODE

using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XFrame;

//########################################################################
//#                                                                      #
//# Childwindow-Wrapper-Class                                            #
//#                                                                      #
//########################################################################

SvxHlinkCtrl::SvxHlinkCtrl( sal_uInt16 _nId, SfxBindings & rBindings, SvxHpLinkDlg* pDlg )
: SfxControllerItem ( _nId, rBindings )
  ,aOnlineForwarder  ( SID_INTERNET_ONLINE , *this )
  ,aRdOnlyForwarder  ( SID_READONLY_MODE, *this )
{
    pParent = pDlg;
}

void SvxHlinkCtrl::StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                 const SfxPoolItem* pState )
{
    if ( eState == SFX_ITEM_AVAILABLE )
    {
        switch ( nSID )
        {
            case SID_INTERNET_ONLINE :
            {
                pParent->EnableInetBrowse( !( (SfxBoolItem*)pState)->GetValue() );
            }
            break;
            case SID_HYPERLINK_GETLINK :
            {
                pParent->SetPage ( (SvxHyperlinkItem*)pState);
            }
            break;
            case SID_READONLY_MODE :
            {
                pParent->SetReadOnlyMode( ( (SfxBoolItem*)pState)->GetValue() == sal_True );
            }
            break;
        }
    }
}



// -----------------------------------------------------------------------



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
:   IconChoiceDialog( pParent, CUI_RES ( RID_SVXDLG_NEWHYPERLINK ) ),
    maCtrl          ( SID_HYPERLINK_GETLINK, *pBindings, this ),
    mpBindings      ( pBindings ),
    mbReadOnly      ( sal_False ),
    mbIsHTMLDoc     ( sal_False )
{
    SetUniqueId( HID_HYPERLINK_DIALOG );
    mbGrabFocus = sal_True;
    // insert pages
    Image aImage;
    String aStrTitle;
    SvxIconChoiceCtrlEntry* pEntry = NULL;

    aStrTitle = CUI_RESSTR( RID_SVXSTR_HYPERDLG_HLINETTP );
    aImage = Image( CUI_RES ( RID_SVXBMP_HLINETTP ) );
    pEntry = AddTabPage ( RID_SVXPAGE_HYPERLINK_INTERNET, aStrTitle, aImage, SvxHyperlinkInternetTp::Create );
    pEntry->SetQuickHelpText( CUI_RESSTR( RID_SVXSTR_HYPERDLG_HLINETTP_HELP ) );
    aStrTitle = CUI_RESSTR( RID_SVXSTR_HYPERDLG_HLMAILTP );
    aImage = Image( CUI_RES ( RID_SVXBMP_HLMAILTP ) );
    pEntry = AddTabPage ( RID_SVXPAGE_HYPERLINK_MAIL, aStrTitle, aImage, SvxHyperlinkMailTp::Create );
    pEntry->SetQuickHelpText( CUI_RESSTR( RID_SVXSTR_HYPERDLG_HLMAILTP_HELP ) );
    aStrTitle = CUI_RESSTR( RID_SVXSTR_HYPERDLG_HLDOCTP );
    aImage = Image( CUI_RES ( RID_SVXBMP_HLDOCTP ) );
    pEntry = AddTabPage ( RID_SVXPAGE_HYPERLINK_DOCUMENT, aStrTitle, aImage, SvxHyperlinkDocTp::Create );
    pEntry->SetQuickHelpText( CUI_RESSTR( RID_SVXSTR_HYPERDLG_HLDOCTP_HELP ) );
    aStrTitle = CUI_RESSTR( RID_SVXSTR_HYPERDLG_HLDOCNTP );
    aImage = Image( CUI_RES ( RID_SVXBMP_HLDOCNTP ) );
    pEntry = AddTabPage ( RID_SVXPAGE_HYPERLINK_NEWDOCUMENT, aStrTitle, aImage, SvxHyperlinkNewDocTp::Create );
    pEntry->SetQuickHelpText( CUI_RESSTR( RID_SVXSTR_HYPERDLG_HLDOCNTP_HELP ) );

    // create itemset for tabpages
    mpItemSet = new SfxItemSet( SFX_APP()->GetPool(), SID_HYPERLINK_GETLINK,
                               SID_HYPERLINK_SETLINK );

    SvxHyperlinkItem aItem;
    mpItemSet->Put (aItem, SID_HYPERLINK_GETLINK);

    SetInputSet (mpItemSet);

    // Init Dialog
    Start (sal_False);

    pBindings->Update( SID_READONLY_MODE );

    // set OK/Cancel - button
    GetOKButton().SetText ( CUI_RESSTR(RID_SVXSTR_HYPDLG_APPLYBUT) );
    GetCancelButton().SetText ( CUI_RESSTR(RID_SVXSTR_HYPDLG_CLOSEBUT) );

    GetOKButton().SetClickHdl    ( LINK ( this, SvxHpLinkDlg, ClickApplyHdl_Impl ) );
    GetCancelButton().SetClickHdl( LINK ( this, SvxHpLinkDlg, ClickCloseHdl_Impl ) );
}

SvxHpLinkDlg::~SvxHpLinkDlg ()
{
    // delete config item, so the base class (IconChoiceDialog) can not load it on the next start
    SvtViewOptions aViewOpt( E_TABDIALOG, String::CreateFromInt32( SID_HYPERLINK_DIALOG ) );
    aViewOpt.Delete();

    delete mpItemSet;
}

/*************************************************************************
|*
|* Close Dialog-Window
|*
|************************************************************************/

sal_Bool SvxHpLinkDlg::Close()
{
    GetDispatcher()->Execute( SID_HYPERLINK_DIALOG,
                              SFX_CALLMODE_ASYNCHRON |
                              SFX_CALLMODE_RECORD);
    return sal_True;
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

        sal_Bool bDoInvalid ;
        if( aDlgPos.X()+(1.02*aDlgSize.Width())+aExtraWndSize.Width() > aWindowSize.Width() )
        {
            if( aDlgPos.X() - ( 0.02*aDlgSize.Width() ) - aExtraWndSize.Width() < 0 )
            {
                // Pos Extrawindow anywhere
                bDoInvalid = pCurrentPage->MoveToExtraWnd( Point( 1, long(1.1*aDlgPos.Y()) ), sal_True );
            }
            else
            {
                // Pos Extrawindow on the left side of Dialog
                bDoInvalid = pCurrentPage->MoveToExtraWnd( aDlgPos -
                                                           Point( long(0.02*aDlgSize.Width()), 0 ) -
                                                           Point( aExtraWndSize.Width(), 0 ) );
            }
        }
        else
        {
            // Pos Extrawindow on the right side of Dialog
            bDoInvalid = pCurrentPage->MoveToExtraWnd ( aDlgPos + Point( long(1.02*aDlgSize.Width()), 0 ) );
        }

        if ( bDoInvalid )
            Invalidate(INVALIDATE_BACKGROUND);
    }

    Window::Move();
}

/*************************************************************************
|*
|* Click on Apply-button
|*
|************************************************************************/

IMPL_LINK_NOARG(SvxHpLinkDlg, ClickApplyHdl_Impl)
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

IMPL_LINK_NOARG(SvxHpLinkDlg, ClickCloseHdl_Impl)
{
    Close();

    return( 0L );
}

/*************************************************************************
|*
|* Set Page
|*
|************************************************************************/

sal_uInt16 SvxHpLinkDlg::SetPage ( SvxHyperlinkItem* pItem )
{
    sal_uInt16 nPageId = RID_SVXPAGE_HYPERLINK_INTERNET;

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
            sal_Char const sNewsSrvScheme[] = "news://";
                // TODO news:// is nonsense

            if ( aStrURL.SearchAscii( sNewsSrvScheme ) == 0 )
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

    mbIsHTMLDoc = (pItem->GetInsertMode() & HLINK_HTMLMODE) ? true : false;

    SfxItemSet& aPageSet =  (SfxItemSet&)GetTabPage (nPageId)->GetItemSet ();
    aPageSet.Put ( *pItem );

    pCurrentPage->Reset( aPageSet );
    if ( mbGrabFocus )
    {
        pCurrentPage->SetInitFocus();   // #92535# grab the focus only once at initialization
        mbGrabFocus = sal_False;
    }
    return nPageId;
}

/*************************************************************************
|*
|* Enable/Disable to browse targets in a html-doc
|*
|************************************************************************/

void SvxHpLinkDlg::EnableInetBrowse( sal_Bool bEnable )
{
    SvxHyperlinkTabPageBase* pCurrentPage = ( SvxHyperlinkTabPageBase* )
                                            GetTabPage ( GetCurPageId() );
    pCurrentPage->SetOnlineMode( bEnable );
}

/*************************************************************************
|*
|* Enable/Disable ReadOnly mode
|*
|************************************************************************/

void SvxHpLinkDlg::SetReadOnlyMode( sal_Bool bRdOnly )
{
    mbReadOnly = bRdOnly;
    if ( bRdOnly )
        GetOKButton().Disable();
    else
        GetOKButton().Enable();
}

/*************************************************************************
|*
|* late-initialization of newly created pages
|*
|************************************************************************/

void SvxHpLinkDlg::PageCreated( sal_uInt16 /*nId*/, IconChoicePage& rPage )
{
    SvxHyperlinkTabPageBase& rHyperlinkPage = dynamic_cast< SvxHyperlinkTabPageBase& >( rPage );
    Reference< XFrame > xDocumentFrame;
    if ( mpBindings )
        xDocumentFrame = mpBindings->GetActiveFrame();
    OSL_ENSURE( xDocumentFrame.is(), "SvxHpLinkDlg::PageCreated: macro assignment functionality won't work with a proper frame!" );
    rHyperlinkPage.SetDocumentFrame( xDocumentFrame );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
