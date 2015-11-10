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
#include <svx/svxids.hrc>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::frame::XFrame;


//#                                                                      #
//# Childwindow-Wrapper-Class                                            #
//#                                                                      #


SvxHlinkCtrl::SvxHlinkCtrl( sal_uInt16 _nId, SfxBindings & rBindings, SvxHpLinkDlg* pDlg )
    : SfxControllerItem ( _nId, rBindings )
    , aRdOnlyForwarder  ( SID_READONLY_MODE, *this )
{
    pParent = pDlg;
}

void SvxHlinkCtrl::dispose()
{
    aRdOnlyForwarder.dispose();
    ::SfxControllerItem::dispose();
}

void SvxHlinkCtrl::StateChanged( sal_uInt16 nSID, SfxItemState eState,
                                 const SfxPoolItem* pState )
{
    if ( eState == SfxItemState::DEFAULT && !pParent->IsDisposed() )
    {
        switch ( nSID )
        {
            case SID_HYPERLINK_GETLINK :
            {
                pParent->SetPage( const_cast<SvxHyperlinkItem*>(static_cast<const SvxHyperlinkItem*>(pState)) );
            }
            break;
            case SID_READONLY_MODE :
            {
                pParent->SetReadOnlyMode( static_cast<const SfxBoolItem*>(pState)->GetValue() );
            }
            break;
        }
    }
}








//#                                                                      #
//# Hyperlink - Dialog                                                   #
//#                                                                      #


/*************************************************************************
|*
|* Constructor / Destructor
|*
|************************************************************************/

SvxHpLinkDlg::SvxHpLinkDlg (vcl::Window* pParent, SfxBindings* pBindings)
:   IconChoiceDialog( pParent, "HyperlinkDialog", "cui/ui/hyperlinkdialog.ui" ),
    maCtrl          ( SID_HYPERLINK_GETLINK, *pBindings, this ),
    mpBindings      ( pBindings ),
    mbReadOnly      ( false ),
    mbIsHTMLDoc     ( false )
{
    SetUniqueId( HID_HYPERLINK_DIALOG );
    mbGrabFocus = true;
    // insert pages
    Image aImage;
    OUString aStrTitle;
    SvxIconChoiceCtrlEntry *pEntry;

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

    // set OK/Cancel - button
    GetCancelButton().SetText ( CUI_RESSTR(RID_SVXSTR_HYPDLG_CLOSEBUT) );

    // create itemset for tabpages
    mpItemSet = new SfxItemSet( SfxGetpApp()->GetPool(), SID_HYPERLINK_GETLINK,
                               SID_HYPERLINK_SETLINK );

    SvxHyperlinkItem aItem;
    mpItemSet->Put (aItem, SID_HYPERLINK_GETLINK);

    SetInputSet (mpItemSet);

    //loop through the pages and get their max bounds and lock that down
    ShowPage(RID_SVXPAGE_HYPERLINK_NEWDOCUMENT);
    VclBox *pBox = get_content_area();
    Size aMaxPrefSize(pBox->get_preferred_size());
    ShowPage(RID_SVXPAGE_HYPERLINK_DOCUMENT);
    Size aSize(pBox->get_preferred_size());
    aMaxPrefSize.Width() = std::max(aMaxPrefSize.Width(), aSize.Width());
    aMaxPrefSize.Height() = std::max(aMaxPrefSize.Height(), aSize.Height());
    ShowPage(RID_SVXPAGE_HYPERLINK_MAIL);
    aSize = pBox->get_preferred_size();
    aMaxPrefSize.Width() = std::max(aMaxPrefSize.Width(), aSize.Width());
    aMaxPrefSize.Height() = std::max(aMaxPrefSize.Height(), aSize.Height());
    ShowPage(RID_SVXPAGE_HYPERLINK_INTERNET);
    aSize = pBox->get_preferred_size();
    aMaxPrefSize.Width() = std::max(aMaxPrefSize.Width(), aSize.Width());
    aMaxPrefSize.Height() = std::max(aMaxPrefSize.Height(), aSize.Height());
    pBox->set_width_request(aMaxPrefSize.Width());
    pBox->set_height_request(aMaxPrefSize.Height());

    SetCurPageId(RID_SVXPAGE_HYPERLINK_INTERNET);

    // Init Dialog
    Start (false);

    pBindings->Update( SID_READONLY_MODE );

    GetOKButton().SetClickHdl    ( LINK ( this, SvxHpLinkDlg, ClickOkHdl_Impl ) );
    GetApplyButton().SetClickHdl ( LINK ( this, SvxHpLinkDlg, ClickApplyHdl_Impl ) );
    GetCancelButton().SetClickHdl( LINK ( this, SvxHpLinkDlg, ClickCloseHdl_Impl ) );
}

SvxHpLinkDlg::~SvxHpLinkDlg ()
{
    disposeOnce();
}

void SvxHpLinkDlg::dispose()
{
    // delete config item, so the base class (IconChoiceDialog) can not load it on the next start
    SvtViewOptions aViewOpt( E_TABDIALOG, OUString::number(SID_HYPERLINK_DIALOG) );
    aViewOpt.Delete();

    delete mpItemSet;
    mpItemSet = nullptr;

    maCtrl.dispose();

    IconChoiceDialog::dispose();
}

/*************************************************************************
|*
|* Close Dialog-Window
|*
|************************************************************************/

bool SvxHpLinkDlg::Close()
{
    GetDispatcher()->Execute( SID_HYPERLINK_DIALOG,
                              SfxCallMode::ASYNCHRON |
                              SfxCallMode::RECORD);
    return true;
}

void SvxHpLinkDlg::Apply()
{
    SfxItemSet aItemSet( SfxGetpApp()->GetPool(), SID_HYPERLINK_GETLINK,
                         SID_HYPERLINK_SETLINK );

    SvxHyperlinkTabPageBase* pCurrentPage = static_cast<SvxHyperlinkTabPageBase*>(
                                                GetTabPage( GetCurPageId() ) );

    if ( pCurrentPage->AskApply() )
    {
        pCurrentPage->FillItemSet( &aItemSet );

        const SvxHyperlinkItem *aItem = static_cast<const SvxHyperlinkItem *>(
                                      aItemSet.GetItem (SID_HYPERLINK_SETLINK));
        OUString aStrEmpty;
        if ( aItem->GetURL() != aStrEmpty )
            GetDispatcher()->Execute( SID_HYPERLINK_SETLINK, SfxCallMode::ASYNCHRON |
                                      SfxCallMode::RECORD, aItem, 0L);

        static_cast<SvxHyperlinkTabPageBase*>( GetTabPage( GetCurPageId() ) )->DoApply();
    }
}

/*************************************************************************
|*
|* When extra window is visible and its never moved by user, then move that
|* window, too.
|*
|************************************************************************/

void SvxHpLinkDlg::Move()
{
    SvxHyperlinkTabPageBase* pCurrentPage = static_cast<SvxHyperlinkTabPageBase*>(
                                              GetTabPage ( GetCurPageId() ) );

    if( pCurrentPage->IsMarkWndVisible () )
    {
        // Pos&Size of this dialog-window
        Point aDlgPos ( GetPosPixel () );
        Size aDlgSize ( GetSizePixel () );

        // Size of Office-Main-Window
        Size aWindowSize( SfxGetpApp()->GetTopWindow()->GetSizePixel() );

        // Size of Extrawindow
        Size aExtraWndSize( pCurrentPage->GetSizeExtraWnd() );

        bool bDoInvalid ;
        if( aDlgPos.X()+(1.02*aDlgSize.Width())+aExtraWndSize.Width() > aWindowSize.Width() )
        {
            if( aDlgPos.X() - ( 0.02*aDlgSize.Width() ) - aExtraWndSize.Width() < 0 )
            {
                // Pos Extrawindow anywhere
                bDoInvalid = pCurrentPage->MoveToExtraWnd( Point( 1, long(1.1*aDlgPos.Y()) ), true );
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
            Invalidate(InvalidateFlags::Transparent);
    }

    Window::Move();
}

/// Click on OK button
IMPL_LINK_NOARG_TYPED(SvxHpLinkDlg, ClickOkHdl_Impl, Button*, void)
{
    Apply();
    Close();
}

/*************************************************************************
|*
|* Click on Apply-button
|*
|************************************************************************/

IMPL_LINK_NOARG_TYPED(SvxHpLinkDlg, ClickApplyHdl_Impl, Button*, void)
{
    Apply();
}

/*************************************************************************
|*
|* Click on Close-button
|*
|************************************************************************/

IMPL_LINK_NOARG_TYPED(SvxHpLinkDlg, ClickCloseHdl_Impl, Button*, void)
{
    Close();
}

/*************************************************************************
|*
|* Set Page
|*
|************************************************************************/

sal_uInt16 SvxHpLinkDlg::SetPage ( SvxHyperlinkItem* pItem )
{
    sal_uInt16 nPageId = RID_SVXPAGE_HYPERLINK_INTERNET;

    OUString aStrURL(pItem->GetURL());
    INetURLObject aURL(aStrURL);
    INetProtocol eProtocolTyp = aURL.GetProtocol();

    switch ( eProtocolTyp )
    {
        case INetProtocol::Http :
        case INetProtocol::Ftp :
            nPageId = RID_SVXPAGE_HYPERLINK_INTERNET;
            break;
        case INetProtocol::File :
            nPageId = RID_SVXPAGE_HYPERLINK_DOCUMENT;
            break;
        case INetProtocol::Mailto :
            nPageId = RID_SVXPAGE_HYPERLINK_MAIL;
            break;
        default :
            if (aStrURL.startsWith("#"))
                nPageId = RID_SVXPAGE_HYPERLINK_DOCUMENT;
            else
            {
                eProtocolTyp = INetProtocol::NotValid;
                nPageId = GetCurPageId();
            }
            break;
    }

    ShowPage (nPageId);

    SvxHyperlinkTabPageBase* pCurrentPage = static_cast<SvxHyperlinkTabPageBase*>(GetTabPage( nPageId ));

    mbIsHTMLDoc = (pItem->GetInsertMode() & HLINK_HTMLMODE) != 0;

    IconChoicePage* pPage = GetTabPage (nPageId);
    if(pPage)
    {
        SfxItemSet& aPageSet =  (SfxItemSet&)pPage->GetItemSet ();
        aPageSet.Put ( *pItem );

        pCurrentPage->Reset( aPageSet );
        if ( mbGrabFocus )
        {
            pCurrentPage->SetInitFocus();   // #92535# grab the focus only once at initialization
            mbGrabFocus = false;
        }
    }
    return nPageId;
}

/*************************************************************************
|*
|* Enable/Disable ReadOnly mode
|*
|************************************************************************/

void SvxHpLinkDlg::SetReadOnlyMode( bool bRdOnly )
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
