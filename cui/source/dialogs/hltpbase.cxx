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

#include <sal/config.h>

#include <osl/file.hxx>
#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>
#include <sot/formats.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/macitem.hxx>
#include <ucbhelper/content.hxx>
#include "cuihyperdlg.hxx"
#include "hltpbase.hxx"
#include "macroass.hxx"
#include <svx/svxdlg.hxx>
#include <cuires.hrc>
#include <vcl/builderfactory.hxx>

using namespace ::ucbhelper;

namespace {

OUString CreateUiNameFromURL( const OUString& aStrURL )
{
    OUString          aStrUiURL;
    INetURLObject   aURLObj( aStrURL );

    switch(aURLObj.GetProtocol())
    {
        case INetProtocol::File:
            osl::FileBase::getSystemPathFromFileURL(aURLObj.GetMainURL(INetURLObject::NO_DECODE), aStrUiURL);
            break;
        case INetProtocol::Ftp :
            {
                //remove password from name
                INetURLObject   aTmpURL(aURLObj);
                aTmpURL.SetPass("");
                aStrUiURL = aTmpURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );
            }
            break;
        default :
            {
                aStrUiURL = aURLObj.GetMainURL(INetURLObject::DECODE_UNAMBIGUOUS);
            }
    }
    if(aStrUiURL.isEmpty())
        return aStrURL;
    return aStrUiURL;
}

}

//# ComboBox-Control for URL's with History and Autocompletion           #

SvxHyperURLBox::SvxHyperURLBox( vcl::Window* pParent, INetProtocol eSmart )
: SvtURLBox         ( pParent, eSmart ),
  DropTargetHelper  ( this )
{
}

VCL_BUILDER_FACTORY_ARGS(SvxHyperURLBox, INetProtocol::Http)

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
        SetText( aString );
        nRet = DND_ACTION_COPY;
    }

    return nRet;
}

void SvxHyperURLBox::Select()
{
    SvtURLBox::Select();
}
void SvxHyperURLBox::Modify()
{
    SvtURLBox::Modify();
}
bool SvxHyperURLBox::Notify( NotifyEvent& rNEvt )
{
    return SvtURLBox::Notify( rNEvt );
}
bool SvxHyperURLBox::PreNotify( NotifyEvent& rNEvt )
{
    return SvtURLBox::PreNotify( rNEvt );
}

//# Hyperlink-Dialog: Tabpages-Baseclass                                 #

SvxHyperlinkTabPageBase::SvxHyperlinkTabPageBase ( vcl::Window *pParent,
                                                   IconChoiceDialog* pDlg,
                                                   const OString& rID,
                                                   const OUString& rUIXMLDescription,
                                                   const SfxItemSet& rItemSet )
:   IconChoicePage          ( pParent, rID, rUIXMLDescription, rItemSet ),
    mpCbbFrame              ( NULL ),
    mpLbForm                ( NULL ),
    mpEdIndication          ( NULL ),
    mpEdText                ( NULL ),
    mpBtScript              ( NULL ),
    mbIsCloseDisabled       ( false ),
    mpDialog                ( pDlg ),
    mbStdControlsInit       ( false )
{
    // create bookmark-window
    mpMarkWnd = VclPtr<SvxHlinkDlgMarkWnd>::Create( this );
}

SvxHyperlinkTabPageBase::~SvxHyperlinkTabPageBase()
{
    disposeOnce();
}

void SvxHyperlinkTabPageBase::dispose()
{
    maTimer.Stop();

    mpMarkWnd.disposeAndClear();

    mpCbbFrame.clear();
    mpLbForm.clear();
    mpEdIndication.clear();
    mpEdText.clear();
    mpBtScript.clear();
    mpDialog.clear();

    IconChoicePage::dispose();
}

void SvxHyperlinkTabPageBase::ActivatePage()
{
    TabPage::ActivatePage();
}

void SvxHyperlinkTabPageBase::DeactivatePage()
{
    TabPage::DeactivatePage();
}

bool SvxHyperlinkTabPageBase::QueryClose()
{
    return !mbIsCloseDisabled;
}

void SvxHyperlinkTabPageBase::InitStdControls ()
{
    if ( !mbStdControlsInit )
    {
        get(mpCbbFrame, "frame");

        SfxDispatcher* pDispatch = GetDispatcher();
        SfxViewFrame* pViewFrame = pDispatch ? pDispatch->GetFrame() : 0;
        SfxFrame* pFrame = pViewFrame ? &pViewFrame->GetTopFrame() : 0;
        if ( pFrame )
        {
            std::unique_ptr<TargetList> pList(new TargetList);
            pFrame->GetTargetList(*pList);
            if( !pList->empty() )
            {
                size_t nCount = pList->size();
                size_t i;
                for ( i = 0; i < nCount; i++ )
                {
                    mpCbbFrame->InsertEntry( pList->at( i ) );
                }
            }
        }

        get(mpLbForm, "form");
        get(mpEdIndication, "indication");
        get(mpEdText, "name");
        get(mpBtScript, "script");
        mpBtScript->SetModeImage(Image(CUI_RES (RID_SVXBMP_SCRIPT)));

        mpBtScript->SetClickHdl ( LINK ( this, SvxHyperlinkTabPageBase, ClickScriptHdl_Impl ) );
        mpBtScript->EnableTextDisplay (false);
    }

    mbStdControlsInit = true;
}

// Move Extra-Window
bool SvxHyperlinkTabPageBase::MoveToExtraWnd( Point aNewPos, bool bDisConnectDlg )
{
    bool bReturn =  mpMarkWnd->MoveTo ( aNewPos );

    if( bDisConnectDlg )
        mpMarkWnd->ConnectToDialog( false );

    return ( !bReturn && IsMarkWndVisible() );
}

// Show Extra-Window
void SvxHyperlinkTabPageBase::ShowMarkWnd ()
{
    static_cast<vcl::Window*>(mpMarkWnd)->Show();

    // Size of dialog-window in screen pixels
    Rectangle aDlgRect( mpDialog->GetWindowExtentsRelative( NULL ) );
    Point aDlgPos ( aDlgRect.TopLeft() );
    Size aDlgSize ( mpDialog->GetSizePixel () );

    // Absolute size of the screen
    Rectangle aScreen( mpDialog->GetDesktopRectPixel() );

    // Size of Extrawindow
    Size aExtraWndSize( mpMarkWnd->GetSizePixel () );

    // mpMarkWnd is a child of mpDialog, so coordinates for positioning must be relative to mpDialog
    if( aDlgPos.X()+(1.05*aDlgSize.Width())+aExtraWndSize.Width() > aScreen.Right() )
    {
        if( aDlgPos.X() - ( 0.05*aDlgSize.Width() ) - aExtraWndSize.Width() < 0 )
        {
            // Pos Extrawindow anywhere
            MoveToExtraWnd( Point(10,10) );  // very unlikely
            mpMarkWnd->ConnectToDialog( false );
        }
        else
        {
            // Pos Extrawindow on the left side of Dialog
            MoveToExtraWnd( Point(0,0) - Point( long(0.05*aDlgSize.Width()), 0 ) - Point( aExtraWndSize.Width(), 0 ) );
        }
    }
    else
    {
        // Pos Extrawindow on the right side of Dialog
        MoveToExtraWnd ( Point( long(1.05*aDlgSize.getWidth()), 0 ) );
    }

    // Set size of Extra-Window
    mpMarkWnd->SetSizePixel( Size( aExtraWndSize.Width(), aDlgSize.Height() ) );
}

// Fill Dialogfields
void SvxHyperlinkTabPageBase::FillStandardDlgFields ( const SvxHyperlinkItem* pHyperlinkItem )
{
    // Frame
    sal_Int32 nPos = mpCbbFrame->GetEntryPos ( pHyperlinkItem->GetTargetFrame() );
    if ( nPos != COMBOBOX_ENTRY_NOTFOUND)
        mpCbbFrame->SetText ( pHyperlinkItem->GetTargetFrame() );

    // Form
    OUString aStrFormText = CUI_RESSTR( RID_SVXSTR_HYPERDLG_FROM_TEXT );
    OUString aStrFormButton = CUI_RESSTR( RID_SVXSTR_HYPERDLG_FORM_BUTTON );

    if( pHyperlinkItem->GetInsertMode() & HLINK_HTMLMODE )
    {
        mpLbForm->Clear();
        mpLbForm->InsertEntry( aStrFormText );
        mpLbForm->SelectEntryPos ( 0 );
    }
    else
    {
        mpLbForm->Clear();
        mpLbForm->InsertEntry( aStrFormText );
        mpLbForm->InsertEntry( aStrFormButton );
        mpLbForm->SelectEntryPos ( pHyperlinkItem->GetInsertMode() == HLINK_BUTTON ? 1 : 0 );
    }

    // URL
    mpEdIndication->SetText ( pHyperlinkItem->GetName() );

    // Name
    mpEdText->SetText ( pHyperlinkItem->GetIntName() );

    // Script-button
    if ( !pHyperlinkItem->GetMacroEvents() )
        mpBtScript->Disable();
    else
        mpBtScript->Enable();
}

// Any action to do after apply-button is pressed
void SvxHyperlinkTabPageBase::DoApply ()
{
    // default-implemtation : do nothing
}

// Ask page whether an insert is possible
bool SvxHyperlinkTabPageBase::AskApply ()
{
    // default-implementation
    return true;
}

// This method would be called from bookmark-window to set new mark-string
void SvxHyperlinkTabPageBase::SetMarkStr ( const OUString& /*aStrMark*/ )
{
    // default-implemtation : do nothing
}

// Set initial focus
void SvxHyperlinkTabPageBase::SetInitFocus()
{
    GrabFocus();
}

// Ask dialog whether the current doc is a HTML-doc
bool SvxHyperlinkTabPageBase::IsHTMLDoc() const
{
    return static_cast<SvxHpLinkDlg*>(mpDialog.get())->IsHTMLDoc();
}

// retrieve dispatcher
SfxDispatcher* SvxHyperlinkTabPageBase::GetDispatcher() const
{
    return static_cast<SvxHpLinkDlg*>(mpDialog.get())->GetDispatcher();
}

// Click on imagebutton : Script
IMPL_LINK_NOARG_TYPED(SvxHyperlinkTabPageBase, ClickScriptHdl_Impl, Button*, void)
{
    SvxHyperlinkItem *pHyperlinkItem = const_cast<SvxHyperlinkItem*>(static_cast<const SvxHyperlinkItem *>(
                                       GetItemSet().GetItem (SID_HYPERLINK_GETLINK)));

    if ( pHyperlinkItem->GetMacroEvents() )
    {
        // get macros from itemset
        const SvxMacroTableDtor* pMacroTbl = pHyperlinkItem->GetMacroTable();
        SvxMacroItem aItem ( GetWhich(SID_ATTR_MACROITEM) );
        if( pMacroTbl )
            aItem.SetMacroTable( *pMacroTbl );

        // create empty itemset for macro-dlg
        SfxItemSet* pItemSet = new SfxItemSet(SfxGetpApp()->GetPool(),
                                              SID_ATTR_MACROITEM,
                                              SID_ATTR_MACROITEM );
        pItemSet->Put ( aItem, SID_ATTR_MACROITEM );

        /*  disable HyperLinkDlg for input while the MacroAssignDlg is working
            because if no JAVA is installed an error box occurs and then it is possible
            to close the HyperLinkDlg before its child (MacroAssignDlg) -> GPF
         */
        bool bIsInputEnabled = GetParent()->IsInputEnabled();
        if ( bIsInputEnabled )
            GetParent()->EnableInput( false );
        ScopedVclPtrInstance< SfxMacroAssignDlg > aDlg( this, mxDocumentFrame, *pItemSet );

        // add events
        SfxMacroTabPage *pMacroPage = static_cast<SfxMacroTabPage*>( aDlg->GetTabPage() );

        if ( pHyperlinkItem->GetMacroEvents() & HYPERDLG_EVENT_MOUSEOVER_OBJECT )
            pMacroPage->AddEvent( OUString( CUI_RESSTR(RID_SVXSTR_HYPDLG_MACROACT1) ),
                                  SFX_EVENT_MOUSEOVER_OBJECT );
        if ( pHyperlinkItem->GetMacroEvents() & HYPERDLG_EVENT_MOUSECLICK_OBJECT )
            pMacroPage->AddEvent( OUString( CUI_RESSTR(RID_SVXSTR_HYPDLG_MACROACT2) ),
                                  SFX_EVENT_MOUSECLICK_OBJECT);
        if ( pHyperlinkItem->GetMacroEvents() & HYPERDLG_EVENT_MOUSEOUT_OBJECT )
            pMacroPage->AddEvent( OUString( CUI_RESSTR(RID_SVXSTR_HYPDLG_MACROACT3) ),
                                  SFX_EVENT_MOUSEOUT_OBJECT);

        if ( bIsInputEnabled )
            GetParent()->EnableInput( true );
        // execute dlg
        DisableClose( true );
        short nRet = aDlg->Execute();
        DisableClose( false );
        if ( RET_OK == nRet )
        {
            const SfxItemSet* pOutSet = aDlg->GetOutputItemSet();
            const SfxPoolItem* pItem;
            if( SfxItemState::SET == pOutSet->GetItemState( SID_ATTR_MACROITEM, false, &pItem ))
            {
                pHyperlinkItem->SetMacroTable( static_cast<const SvxMacroItem*>(pItem)->GetMacroTable() );
            }
        }
        delete pItemSet;
    }
}

// Get Macro-Infos
sal_uInt16 SvxHyperlinkTabPageBase::GetMacroEvents()
{
    const SvxHyperlinkItem *pHyperlinkItem = static_cast<const SvxHyperlinkItem *>(
                                       GetItemSet().GetItem (SID_HYPERLINK_GETLINK));

    return pHyperlinkItem->GetMacroEvents();
}

SvxMacroTableDtor* SvxHyperlinkTabPageBase::GetMacroTable()
{
    const SvxHyperlinkItem *pHyperlinkItem = static_cast<const SvxHyperlinkItem *>(
                                       GetItemSet().GetItem (SID_HYPERLINK_GETLINK));

    return const_cast<SvxMacroTableDtor*>(pHyperlinkItem->GetMacroTable());
}

// try to detect the current protocol that is used in rStrURL
OUString SvxHyperlinkTabPageBase::GetSchemeFromURL( const OUString& rStrURL )
{
    OUString aStrScheme;

    INetURLObject aURL( rStrURL );
    INetProtocol aProtocol = aURL.GetProtocol();

    // #77696#
    // our new INetUrlObject now has the ability
    // to detect if an Url is valid or not :-(
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
    aStrIntName = mpEdText->GetText();
    aStrName    = mpEdIndication->GetText();
    aStrFrame   = mpCbbFrame->GetText();
    eMode       = (SvxLinkInsertMode) (mpLbForm->GetSelectEntryPos()+1);
    if( IsHTMLDoc() )
        eMode = (SvxLinkInsertMode) ( sal_uInt16(eMode) | HLINK_HTMLMODE );
}

// reset dialog-fields
void SvxHyperlinkTabPageBase::Reset( const SfxItemSet& rItemSet)
{

    // Set dialog-fields from create-itemset
    maStrInitURL.clear();

    const SvxHyperlinkItem *pHyperlinkItem = static_cast<const SvxHyperlinkItem *>(
                                       rItemSet.GetItem (SID_HYPERLINK_GETLINK));

    if ( pHyperlinkItem )
    {
        // set dialog-fields
        FillStandardDlgFields (pHyperlinkItem);

        // set all other fields
        FillDlgFields ( (OUString&)pHyperlinkItem->GetURL() );

        // Store initial URL
        maStrInitURL = pHyperlinkItem->GetURL();
    }
}

// Fill output-ItemSet
bool SvxHyperlinkTabPageBase::FillItemSet( SfxItemSet* rOut)
{
    OUString aStrURL, aStrName, aStrIntName, aStrFrame;
    SvxLinkInsertMode eMode;

    GetCurentItemData ( aStrURL, aStrName, aStrIntName, aStrFrame, eMode);
    if ( aStrName.isEmpty() ) //automatically create a visible name if the link is created without name
        aStrName = CreateUiNameFromURL(aStrURL);

    sal_uInt16 nEvents = GetMacroEvents();
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
    const SvxHyperlinkItem *pHyperlinkItem = static_cast<const SvxHyperlinkItem *>(
                                       rItemSet.GetItem (SID_HYPERLINK_GETLINK));

    if ( pHyperlinkItem )
    {
        // standard-fields
        FillStandardDlgFields (pHyperlinkItem);
    }

    // show mark-window if it was open before
    if ( ShouldOpenMarkWnd () )
        ShowMarkWnd ();
}

int SvxHyperlinkTabPageBase::DeactivatePage( SfxItemSet* _pSet)
{
    // hide mark-wnd
    SetMarkWndShouldOpen( IsMarkWndVisible () );
    HideMarkWnd ();

    // retrieve data of dialog
    OUString aStrURL, aStrName, aStrIntName, aStrFrame;
    SvxLinkInsertMode eMode;

    GetCurentItemData ( aStrURL, aStrName, aStrIntName, aStrFrame, eMode);

    sal_uInt16 nEvents = GetMacroEvents();
    SvxMacroTableDtor* pTable = GetMacroTable();

    if( _pSet )
    {
        SvxHyperlinkItem aItem( SID_HYPERLINK_GETLINK, aStrName, aStrURL, aStrFrame,
                                aStrIntName, eMode, nEvents, pTable );
        _pSet->Put( aItem );
    }

    return LEAVE_PAGE;
}

bool SvxHyperlinkTabPageBase::ShouldOpenMarkWnd()
{
    return false;
}

void SvxHyperlinkTabPageBase::SetMarkWndShouldOpen(bool)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
