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

#include <memory>
#include <sal/config.h>

#include <comphelper/lok.hxx>
#include <osl/file.hxx>
#include <sfx2/app.hxx>
#include <sfx2/event.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>
#include <sot/formats.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/macitem.hxx>
#include <ucbhelper/content.hxx>
#include <cuihyperdlg.hxx>
#include <hltpbase.hxx>
#include <macroass.hxx>
#include <svx/svxdlg.hxx>
#include <strings.hrc>
#include <dialmgr.hxx>
#include <bitmaps.hlst>
#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>

namespace {

OUString CreateUiNameFromURL( const OUString& aStrURL )
{
    OUString          aStrUiURL;
    INetURLObject   aURLObj( aStrURL );

    switch(aURLObj.GetProtocol())
    {
        case INetProtocol::File:
            osl::FileBase::getSystemPathFromFileURL(aURLObj.GetMainURL(INetURLObject::DecodeMechanism::NONE), aStrUiURL);
            break;
        case INetProtocol::Ftp :
            {
                //remove password from name
                INetURLObject   aTmpURL(aURLObj);
                aTmpURL.SetPass(u"");
                aStrUiURL = aTmpURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous );
            }
            break;
        default :
            {
                aStrUiURL = aURLObj.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous);
            }
    }
    if(aStrUiURL.isEmpty())
        return aStrURL;
    return aStrUiURL;
}

}

// ComboBox-Control for URL's with History and Autocompletion
SvxHyperURLBox::SvxHyperURLBox(std::unique_ptr<weld::ComboBox> xControl)
    : SvtURLBox(std::move(xControl))
    , DropTargetHelper(getWidget()->get_drop_target())
{
    SetSmartProtocol(INetProtocol::Http);
}

sal_Int8 SvxHyperURLBox::AcceptDrop( const AcceptDropEvent& /* rEvt */ )
{
    return IsDropFormatSupported( SotClipboardFormatId::STRING ) ? DND_ACTION_COPY : DND_ACTION_NONE;
}

sal_Int8 SvxHyperURLBox::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    TransferableDataHelper  aDataHelper( rEvt.maDropEvent.Transferable );
    OUString                aString;
    sal_Int8                nRet = DND_ACTION_NONE;

    if( aDataHelper.GetString( SotClipboardFormatId::STRING, aString ) )
    {
        set_entry_text(aString);
        nRet = DND_ACTION_COPY;
    }

    return nRet;
}

//# Hyperlink-Dialog: Tabpages-Baseclass                                 #

SvxHyperlinkTabPageBase::SvxHyperlinkTabPageBase(weld::Container* pParent,
                                                 SvxHpLinkDlg* pDlg,
                                                 const OUString& rUIXMLDescription,
                                                 const OUString& rID,
                                                 const SfxItemSet* pItemSet)
  : IconChoicePage(pParent, rUIXMLDescription, rID, pItemSet)
  , mxCbbFrame(xBuilder->weld_combo_box("frame"))
  , mxLbForm(xBuilder->weld_combo_box("form"))
  , mxEdIndication(xBuilder->weld_entry("indication"))
  , mxEdText(xBuilder->weld_entry("name"))
  , mxBtScript(xBuilder->weld_button("script"))
  , mxFormLabel(xBuilder->weld_label("form_label"))
  , mxFrameLabel(xBuilder->weld_label("frame_label"))
  , mbIsCloseDisabled( false )
  , mpDialog( pDlg )
  , mbStdControlsInit( false )
  , maTimer("cui SvxHyperlinkTabPageBase maTimer")
{
    // create bookmark-window
}

SvxHyperlinkTabPageBase::~SvxHyperlinkTabPageBase()
{
    maTimer.Stop();

    HideMarkWnd();
}

bool SvxHyperlinkTabPageBase::QueryClose()
{
    return !mbIsCloseDisabled;
}

void SvxHyperlinkTabPageBase::InitStdControls ()
{
    if ( !mbStdControlsInit )
    {
        SfxDispatcher* pDispatch = GetDispatcher();
        SfxViewFrame* pViewFrame = pDispatch ? pDispatch->GetFrame() : nullptr;
        SfxFrame* pFrame = pViewFrame ? &pViewFrame->GetFrame() : nullptr;
        if ( pFrame )
        {
            TargetList aList;
            SfxFrame::GetDefaultTargetList(aList);
            if( !aList.empty() )
            {
                size_t nCount = aList.size();
                size_t i;
                for ( i = 0; i < nCount; i++ )
                {
                    mxCbbFrame->append_text( aList.at( i ) );
                }
            }
        }

        mxBtScript->set_from_icon_name(RID_SVXBMP_SCRIPT);

        mxBtScript->connect_clicked ( LINK ( this, SvxHyperlinkTabPageBase, ClickScriptHdl_Impl ) );
    }

    mbStdControlsInit = true;
}

// Move Extra-Window
void SvxHyperlinkTabPageBase::MoveToExtraWnd( Point aNewPos )
{
    mxMarkWnd->MoveTo(aNewPos);
}

// Show Extra-Window
void SvxHyperlinkTabPageBase::ShowMarkWnd()
{
    if (mxMarkWnd)
    {
        mxMarkWnd->getDialog()->present();
        return;
    }

    weld::Dialog* pDialog = mpDialog->getDialog();

    mxMarkWnd = std::make_shared<SvxHlinkDlgMarkWnd>(pDialog, this);

    // Size of dialog-window in screen pixels
    Point aDlgPos(pDialog->get_position());
    Size aDlgSize(pDialog->get_size());

    // Absolute size of the screen
    ::tools::Rectangle aScreen(pDialog->get_monitor_workarea());

    // Size of Extrawindow
    Size aExtraWndSize(mxMarkWnd->getDialog()->get_preferred_size());

    // mxMarkWnd is a child of mpDialog, so coordinates for positioning must be relative to mpDialog
    if( aDlgPos.X()+(1.05*aDlgSize.Width())+aExtraWndSize.Width() > aScreen.Right() )
    {
        if( aDlgPos.X() - ( 0.05*aDlgSize.Width() ) - aExtraWndSize.Width() < 0 )
        {
            // Pos Extrawindow anywhere
            MoveToExtraWnd( Point(10,10) );  // very unlikely
        }
        else
        {
            // Pos Extrawindow on the left side of Dialog
            MoveToExtraWnd( Point(0,0) - Point( tools::Long(0.05*aDlgSize.Width()), 0 ) - Point( aExtraWndSize.Width(), 0 ) );
        }
    }
    else
    {
        // Pos Extrawindow on the right side of Dialog
        MoveToExtraWnd ( Point( tools::Long(1.05*aDlgSize.getWidth()), 0 ) );
    }

    // Set size of Extra-Window
    mxMarkWnd->getDialog()->set_size_request(aExtraWndSize.Width(), aDlgSize.Height());

    weld::DialogController::runAsync(mxMarkWnd, [this](sal_Int32 /*nResult*/) { mxMarkWnd.reset(); } );
}

void SvxHyperlinkTabPageBase::HideMarkWnd()
{
    if (mxMarkWnd)
    {
        mxMarkWnd->response(RET_CANCEL);
        mxMarkWnd.reset();
    }
}

// Fill Dialogfields
void SvxHyperlinkTabPageBase::FillStandardDlgFields ( const SvxHyperlinkItem* pHyperlinkItem )
{
    if (!comphelper::LibreOfficeKit::isActive())
    {
        // Frame
        sal_Int32 nPos = mxCbbFrame->find_text(pHyperlinkItem->GetTargetFrame());
        if (nPos != -1)
            mxCbbFrame->set_active(nPos);

        // Form
        OUString aStrFormText = CuiResId( RID_CUISTR_HYPERDLG_FROM_TEXT );

        OUString aStrFormButton = CuiResId( RID_CUISTR_HYPERDLG_FORM_BUTTON );

        if( pHyperlinkItem->GetInsertMode() & HLINK_HTMLMODE )
        {
            mxLbForm->clear();
            mxLbForm->append_text( aStrFormText );
            mxLbForm->set_active( 0 );
        }
        else
        {
            mxLbForm->clear();
            mxLbForm->append_text( aStrFormText );
            mxLbForm->append_text( aStrFormButton );
            mxLbForm->set_active( pHyperlinkItem->GetInsertMode() == HLINK_BUTTON ? 1 : 0 );
        }
    }
    else
    {
        mxCbbFrame->hide();
        mxLbForm->hide();
        mxFormLabel->hide();
        mxFrameLabel->hide();
    }

    // URL
    mxEdIndication->set_text( pHyperlinkItem->GetName() );

    // Name
    mxEdText->set_text( pHyperlinkItem->GetIntName() );

    // Script-button
    if (!comphelper::LibreOfficeKit::isActive())
    {
        if ( pHyperlinkItem->GetMacroEvents() == HyperDialogEvent::NONE )
            mxBtScript->set_sensitive(false);
        else
            mxBtScript->set_sensitive(true);
    }
    else
    {
        mxBtScript->hide();
    }
}

// Any action to do after apply-button is pressed
void SvxHyperlinkTabPageBase::DoApply ()
{
    // default-implementation : do nothing
}

// This method would be called from bookmark-window to set new mark-string
void SvxHyperlinkTabPageBase::SetMarkStr ( const OUString& /*aStrMark*/ )
{
    // default-implementation : do nothing
}

// Set initial focus
void SvxHyperlinkTabPageBase::SetInitFocus()
{
    xContainer->grab_focus();
}

// retrieve dispatcher
SfxDispatcher* SvxHyperlinkTabPageBase::GetDispatcher() const
{
    return mpDialog->GetDispatcher();
}

void SvxHyperlinkTabPageBase::DisableClose(bool _bDisable)
{
    mbIsCloseDisabled = _bDisable;
    if (mbIsCloseDisabled)
        maBusy.incBusy(mpDialog->getDialog());
    else
        maBusy.decBusy();
}

// Click on imagebutton : Script
IMPL_LINK_NOARG(SvxHyperlinkTabPageBase, ClickScriptHdl_Impl, weld::Button&, void)
{
    SvxHyperlinkItem *pHyperlinkItem = const_cast<SvxHyperlinkItem*>(
                                       GetItemSet().GetItem (SID_HYPERLINK_GETLINK));

    if (!pHyperlinkItem || pHyperlinkItem->GetMacroEvents() == HyperDialogEvent::NONE)
        return;

    // get macros from itemset
    const SvxMacroTableDtor* pMacroTbl = pHyperlinkItem->GetMacroTable();
    SvxMacroItem aItem ( SID_ATTR_MACROITEM );
    if( pMacroTbl )
        aItem.SetMacroTable( *pMacroTbl );

    // create empty itemset for macro-dlg
    SfxItemSetFixed<SID_ATTR_MACROITEM, SID_ATTR_MACROITEM> aItemSet( SfxGetpApp()->GetPool() );
    aItemSet.Put ( aItem );

    DisableClose( true );

    SfxMacroAssignDlg aDlg(mpDialog->getDialog(), mxDocumentFrame, aItemSet);

    // add events
    SfxMacroTabPage *pMacroPage = aDlg.GetTabPage();

    if ( pHyperlinkItem->GetMacroEvents() & HyperDialogEvent::MouseOverObject )
        pMacroPage->AddEvent( CuiResId(RID_CUISTR_HYPDLG_MACROACT1),
                              SvMacroItemId::OnMouseOver );
    if ( pHyperlinkItem->GetMacroEvents() & HyperDialogEvent::MouseClickObject )
        pMacroPage->AddEvent( CuiResId(RID_CUISTR_HYPDLG_MACROACT2),
                              SvMacroItemId::OnClick);
    if ( pHyperlinkItem->GetMacroEvents() & HyperDialogEvent::MouseOutObject )
        pMacroPage->AddEvent( CuiResId(RID_CUISTR_HYPDLG_MACROACT3),
                              SvMacroItemId::OnMouseOut);
    // execute dlg
    short nRet = aDlg.run();
    DisableClose( false );
    if ( RET_OK == nRet )
    {
        const SfxItemSet* pOutSet = aDlg.GetOutputItemSet();
        const SfxPoolItem* pItem;
        if( SfxItemState::SET == pOutSet->GetItemState( SID_ATTR_MACROITEM, false, &pItem ))
        {
            pHyperlinkItem->SetMacroTable( static_cast<const SvxMacroItem*>(pItem)->GetMacroTable() );
        }
    }
}

// Get Macro-Infos
HyperDialogEvent SvxHyperlinkTabPageBase::GetMacroEvents() const
{
    const SvxHyperlinkItem *pHyperlinkItem =
                                       GetItemSet().GetItem (SID_HYPERLINK_GETLINK);

    return pHyperlinkItem ? pHyperlinkItem->GetMacroEvents() : HyperDialogEvent();
}

SvxMacroTableDtor* SvxHyperlinkTabPageBase::GetMacroTable()
{
    const SvxHyperlinkItem *pHyperlinkItem =
                                       GetItemSet().GetItem (SID_HYPERLINK_GETLINK);

    return const_cast<SvxMacroTableDtor*>(pHyperlinkItem->GetMacroTable());
}

// try to detect the current protocol that is used in rStrURL
OUString SvxHyperlinkTabPageBase::GetSchemeFromURL( const OUString& rStrURL )
{
    OUString aStrScheme;

    INetURLObject aURL( rStrURL );
    INetProtocol aProtocol = aURL.GetProtocol();

    // our new INetUrlObject now has the ability
    // to detect if a Url is valid or not :-(
    if ( aProtocol == INetProtocol::NotValid )
    {
        if ( rStrURL.startsWithIgnoreAsciiCase( INET_HTTP_SCHEME ) )
        {
            aStrScheme = INET_HTTP_SCHEME;
        }
        else if ( rStrURL.startsWithIgnoreAsciiCase( INET_HTTPS_SCHEME ) )
        {
            aStrScheme = INET_HTTPS_SCHEME;
        }
        else if ( rStrURL.startsWithIgnoreAsciiCase( INET_FTP_SCHEME ) )
        {
            aStrScheme = INET_FTP_SCHEME;
        }
        else if ( rStrURL.startsWithIgnoreAsciiCase( INET_MAILTO_SCHEME ) )
        {
            aStrScheme = INET_MAILTO_SCHEME;
        }
    }
    else
        aStrScheme = INetURLObject::GetScheme( aProtocol );
    return aStrScheme;
}

void SvxHyperlinkTabPageBase::GetDataFromCommonFields( OUString& aStrName,
                                             OUString& aStrIntName, OUString& aStrFrame,
                                             SvxLinkInsertMode& eMode )
{
    aStrIntName = mxEdText->get_text();
    aStrName    = mxEdIndication->get_text();
    aStrFrame   = mxCbbFrame->get_active_text();

    sal_Int32 nPos = mxLbForm->get_active();
    if (nPos == -1)
        // This happens when FillStandardDlgFields() hides mpLbForm.
        nPos = 0;
    eMode = static_cast<SvxLinkInsertMode>(nPos + 1);

    // Ask dialog whether the current doc is a HTML-doc
    if (mpDialog->IsHTMLDoc())
        eMode = static_cast<SvxLinkInsertMode>( sal_uInt16(eMode) | HLINK_HTMLMODE );
}

// reset dialog-fields
void SvxHyperlinkTabPageBase::Reset( const SfxItemSet& rItemSet)
{

    // Set dialog-fields from create-itemset
    maStrInitURL.clear();

    const SvxHyperlinkItem *pHyperlinkItem =
                                       rItemSet.GetItem (SID_HYPERLINK_GETLINK);

    if ( pHyperlinkItem )
    {
        // tdf#146576 - propose clipboard content when inserting a hyperlink
        OUString aStrURL(pHyperlinkItem->GetURL());
        // Store initial URL
        maStrInitURL = aStrURL;
        if (aStrURL.isEmpty())
        {
            if (auto xClipboard = GetSystemClipboard())
            {
                if (auto xTransferable = xClipboard->getContents())
                {
                    css::datatransfer::DataFlavor aFlavor;
                    SotExchange::GetFormatDataFlavor(SotClipboardFormatId::STRING, aFlavor);
                    if (xTransferable->isDataFlavorSupported(aFlavor))
                    {
                        OUString aClipBoardContent;
                        try
                        {
                            if (xTransferable->getTransferData(aFlavor) >>= aClipBoardContent)
                            {
                                INetURLObject aURL;
                                aURL.SetSmartURL(aClipBoardContent);
                                if (!aURL.HasError())
                                    aStrURL
                                        = aURL.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous);
                            }
                        }
                        // tdf#158345: Opening Hyperlink dialog leads to crash
                        // MimeType = "text/plain;charset=utf-16"
                        catch(const css::datatransfer::UnsupportedFlavorException&)
                        {
                        }
                    }
                }
            }
        }

        // set dialog-fields
        FillStandardDlgFields (pHyperlinkItem);

        // set all other fields
        FillDlgFields(aStrURL);
    }
}

// Fill output-ItemSet
bool SvxHyperlinkTabPageBase::FillItemSet( SfxItemSet* rOut)
{
    OUString aStrURL, aStrName, aStrIntName, aStrFrame;
    SvxLinkInsertMode eMode;

    GetCurrentItemData ( aStrURL, aStrName, aStrIntName, aStrFrame, eMode);
    if ( aStrName.isEmpty() ) //automatically create a visible name if the link is created without name
        aStrName = CreateUiNameFromURL(aStrURL);

    HyperDialogEvent nEvents = GetMacroEvents();
    SvxMacroTableDtor* pTable = GetMacroTable();

    SvxHyperlinkItem aItem( SID_HYPERLINK_SETLINK, aStrName, aStrURL, aStrFrame,
                            aStrIntName, eMode, nEvents, pTable );
    rOut->Put (aItem);

    return true;
}

// Activate / Deactivate Tabpage
void SvxHyperlinkTabPageBase::ActivatePage( const SfxItemSet& rItemSet )
{

    // Set dialog-fields from input-itemset
    const SvxHyperlinkItem *pHyperlinkItem =
                                       rItemSet.GetItem (SID_HYPERLINK_GETLINK);

    if ( pHyperlinkItem )
    {
        // standard-fields
        FillStandardDlgFields (pHyperlinkItem);
    }

    // show mark-window if it was open before
    if ( ShouldOpenMarkWnd () )
        ShowMarkWnd ();
}

DeactivateRC SvxHyperlinkTabPageBase::DeactivatePage( SfxItemSet* _pSet)
{
    // hide mark-wnd
    SetMarkWndShouldOpen( IsMarkWndVisible () );
    HideMarkWnd ();

    // retrieve data of dialog
    OUString aStrURL, aStrName, aStrIntName, aStrFrame;
    SvxLinkInsertMode eMode;

    GetCurrentItemData ( aStrURL, aStrName, aStrIntName, aStrFrame, eMode);

    HyperDialogEvent nEvents = GetMacroEvents();
    SvxMacroTableDtor* pTable = GetMacroTable();

    if( _pSet )
    {
        SvxHyperlinkItem aItem( SID_HYPERLINK_GETLINK, aStrName, aStrURL, aStrFrame,
                                aStrIntName, eMode, nEvents, pTable );
        _pSet->Put( aItem );
    }

    return DeactivateRC::LeavePage;
}

bool SvxHyperlinkTabPageBase::ShouldOpenMarkWnd()
{
    return false;
}

void SvxHyperlinkTabPageBase::SetMarkWndShouldOpen(bool)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
