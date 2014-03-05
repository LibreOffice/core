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

#include <sfx2/frame.hxx>
#include <sfx2/viewfrm.hxx>
#include <sot/formats.hxx>
#include <sfx2/sfxsids.hrc>
#include <svl/macitem.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/localfilehelper.hxx>
#include "hyperdlg.hrc"
#include "cuihyperdlg.hxx"
#include "hltpbase.hxx"
#include "macroass.hxx"
#include <svx/svxdlg.hxx>
#include <cuires.hrc>
#include <boost/scoped_ptr.hpp>

using namespace ::ucbhelper;

//# ComboBox-Control, which is filled with all current framenames        #

/*************************************************************************
|*
|* Contructor / Destructor
|*
|************************************************************************/

SvxFramesComboBox::SvxFramesComboBox ( Window* pParent, const ResId& rResId,
                                        SfxDispatcher* pDispatch )
:   ComboBox (pParent, rResId)
{
    SfxViewFrame* pViewFrame = pDispatch ? pDispatch->GetFrame() : 0;
    SfxFrame* pFrame = pViewFrame ? &pViewFrame->GetTopFrame() : 0;
    if ( pFrame )
    {
        boost::scoped_ptr<TargetList> pList(new TargetList);
        pFrame->GetTargetList(*pList);
        if( !pList->empty() )
        {
            size_t nCount = pList->size();
            size_t i;
            for ( i = 0; i < nCount; i++ )
            {
                InsertEntry( pList->at( i ) );
            }
        }
    }
}

SvxFramesComboBox::~SvxFramesComboBox ()
{
}

//# ComboBox-Control for URL's with History and Autocompletion           #

SvxHyperURLBox::SvxHyperURLBox( Window* pParent, INetProtocol eSmart )
: SvtURLBox         ( pParent, eSmart ),
  DropTargetHelper  ( this )
{
}

sal_Int8 SvxHyperURLBox::AcceptDrop( const AcceptDropEvent& /* rEvt */ )
{
    return( IsDropFormatSupported( FORMAT_STRING ) ? DND_ACTION_COPY : DND_ACTION_NONE );
}

sal_Int8 SvxHyperURLBox::ExecuteDrop( const ExecuteDropEvent& rEvt )
{
    TransferableDataHelper  aDataHelper( rEvt.maDropEvent.Transferable );
    OUString                aString;
    sal_Int8                nRet = DND_ACTION_NONE;

    if( aDataHelper.GetString( FORMAT_STRING, aString ) )
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

SvxHyperlinkTabPageBase::SvxHyperlinkTabPageBase ( Window *pParent,
                                                   const ResId &rResId,
                                                   const SfxItemSet& rItemSet )
:   IconChoicePage          ( pParent, rResId, rItemSet ),
    mpGrpMore               ( NULL ),
    mpFtFrame               ( NULL ),
    mpCbbFrame              ( NULL ),
    mpFtForm                ( NULL ),
    mpLbForm                ( NULL ),
    mpFtIndication          ( NULL ),
    mpEdIndication          ( NULL ),
    mpFtText                ( NULL ),
    mpEdText                ( NULL ),
    mpBtScript              ( NULL ),
    mbIsCloseDisabled       ( sal_False ),
    mpDialog                ( pParent ),
    mbStdControlsInit       ( sal_False ),
    aEmptyStr()
{
    // create bookmark-window
    mpMarkWnd = new SvxHlinkDlgMarkWnd ( this );
}

SvxHyperlinkTabPageBase::~SvxHyperlinkTabPageBase ()
{
    maTimer.Stop();

    if ( mbStdControlsInit )
    {
        delete mpGrpMore;
        delete mpFtFrame;
        delete mpCbbFrame;
        delete mpFtForm;
        delete mpLbForm;
        delete mpFtIndication;
        delete mpEdIndication;
        delete mpFtText;
        delete mpEdText ;
        delete mpBtScript;
    }

    delete mpMarkWnd;
}

void SvxHyperlinkTabPageBase::ActivatePage()
{
    TabPage::ActivatePage();
}

void SvxHyperlinkTabPageBase::DeactivatePage()
{
    TabPage::DeactivatePage();
}

sal_Bool SvxHyperlinkTabPageBase::QueryClose()
{
    return !mbIsCloseDisabled;
}

void SvxHyperlinkTabPageBase::InitStdControls ()
{
    if ( !mbStdControlsInit )
    {
        mpGrpMore     = new FixedLine           ( this, ResId (GRP_MORE, *m_pResMgr) );
        mpFtFrame     = new FixedText           ( this, ResId (FT_FRAME, *m_pResMgr) );
        mpCbbFrame    = new SvxFramesComboBox   ( this, ResId (CB_FRAME, *m_pResMgr), GetDispatcher() );
        mpFtForm      = new FixedText           ( this, ResId (FT_FORM, *m_pResMgr) );
        mpLbForm      = new ListBox             ( this, ResId (LB_FORM, *m_pResMgr) );
        mpFtIndication= new FixedText           ( this, ResId (FT_INDICATION, *m_pResMgr) );
        mpEdIndication= new Edit                ( this, ResId (ED_INDICATION, *m_pResMgr) );
        mpFtText      = new FixedText           ( this, ResId (FT_TEXT, *m_pResMgr) );
        mpEdText      = new Edit                ( this, ResId (ED_TEXT, *m_pResMgr) );
        mpBtScript    = new ImageButton         ( this, ResId (BTN_SCRIPT, *m_pResMgr) );

        mpBtScript->SetClickHdl ( LINK ( this, SvxHyperlinkTabPageBase, ClickScriptHdl_Impl ) );
        mpBtScript->EnableTextDisplay (false);

        mpBtScript->SetAccessibleRelationMemberOf( mpGrpMore );
        mpBtScript->SetAccessibleRelationLabeledBy( mpFtForm );
    }

    mbStdControlsInit = sal_True;
}

// Move Extra-Window
sal_Bool SvxHyperlinkTabPageBase::MoveToExtraWnd( Point aNewPos, sal_Bool bDisConnectDlg )
{
    sal_Bool bReturn =  mpMarkWnd->MoveTo ( aNewPos );

    if( bDisConnectDlg )
        mpMarkWnd->ConnectToDialog( sal_False );

    return ( !bReturn && IsMarkWndVisible() );
}

// Show Extra-Window
void SvxHyperlinkTabPageBase::ShowMarkWnd ()
{
    ( ( Window* ) mpMarkWnd )->Show();

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
            mpMarkWnd->ConnectToDialog( sal_False );
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
void SvxHyperlinkTabPageBase::FillStandardDlgFields ( SvxHyperlinkItem* pHyperlinkItem )
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
sal_Bool SvxHyperlinkTabPageBase::AskApply ()
{
    // default-implementation
    return sal_True;
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

// Ask dialog whether the curretn doc is a HTML-doc
sal_Bool SvxHyperlinkTabPageBase::IsHTMLDoc() const
{
    return ((SvxHpLinkDlg*)mpDialog)->IsHTMLDoc();
}

// retrieve dispatcher
SfxDispatcher* SvxHyperlinkTabPageBase::GetDispatcher() const
{
    return ((SvxHpLinkDlg*)mpDialog)->GetDispatcher();
}

// Click on imagebutton : Script
IMPL_LINK_NOARG(SvxHyperlinkTabPageBase, ClickScriptHdl_Impl)
{
    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       GetItemSet().GetItem (SID_HYPERLINK_GETLINK);

    if ( pHyperlinkItem->GetMacroEvents() )
    {
        // get macros from itemset
        const SvxMacroTableDtor* pMacroTbl = pHyperlinkItem->GetMacroTbl();
        SvxMacroItem aItem ( GetWhich(SID_ATTR_MACROITEM) );
        if( pMacroTbl )
            aItem.SetMacroTable( *pMacroTbl );

        // create empty itemset for macro-dlg
        SfxItemSet* pItemSet = new SfxItemSet(SFX_APP()->GetPool(),
                                              SID_ATTR_MACROITEM,
                                              SID_ATTR_MACROITEM );
        pItemSet->Put ( aItem, SID_ATTR_MACROITEM );

        /*  disable HyperLinkDlg for input while the MacroAssignDlg is working
            because if no JAVA is installed an error box occurs and then it is possible
            to close the HyperLinkDlg before its child (MacroAssignDlg) -> GPF
         */
        sal_Bool bIsInputEnabled = GetParent()->IsInputEnabled();
        if ( bIsInputEnabled )
            GetParent()->EnableInput( false );
        SfxMacroAssignDlg aDlg( this, mxDocumentFrame, *pItemSet );

        // add events
        SfxMacroTabPage *pMacroPage = (SfxMacroTabPage*) aDlg.GetTabPage();

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
        DisableClose( sal_True );
        short nRet = aDlg.Execute();
        DisableClose( sal_False );
        if ( RET_OK == nRet )
        {
            const SfxItemSet* pOutSet = aDlg.GetOutputItemSet();
            const SfxPoolItem* pItem;
            if( SFX_ITEM_SET == pOutSet->GetItemState( SID_ATTR_MACROITEM, false, &pItem ))
            {
                pHyperlinkItem->SetMacroTable( ((SvxMacroItem*)pItem)->GetMacroTable() );
            }
        }
        delete pItemSet;
    }

    return( 0L );
}

// Get Macro-Infos
sal_uInt16 SvxHyperlinkTabPageBase::GetMacroEvents()
{
    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       GetItemSet().GetItem (SID_HYPERLINK_GETLINK);

    return pHyperlinkItem->GetMacroEvents();
}

SvxMacroTableDtor* SvxHyperlinkTabPageBase::GetMacroTable()
{
    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       GetItemSet().GetItem (SID_HYPERLINK_GETLINK);

    return ( (SvxMacroTableDtor*)pHyperlinkItem->GetMacroTbl() );
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
    if ( aProtocol == INET_PROT_NOT_VALID )
    {
        if ( rStrURL.startsWithIgnoreAsciiCase( INET_HTTP_SCHEME ) )
        {
            aStrScheme = OUString( INET_HTTP_SCHEME );
        }
        else if ( rStrURL.startsWithIgnoreAsciiCase( INET_HTTPS_SCHEME ) )
        {
            aStrScheme = OUString( INET_HTTPS_SCHEME );
        }
        else if ( rStrURL.startsWithIgnoreAsciiCase( INET_FTP_SCHEME ) )
        {
            aStrScheme = OUString( INET_FTP_SCHEME );
        }
        else if ( rStrURL.startsWithIgnoreAsciiCase( INET_MAILTO_SCHEME ) )
        {
            aStrScheme = OUString( INET_MAILTO_SCHEME );
        }
        else if ( rStrURL.startsWithIgnoreAsciiCase( INET_NEWS_SCHEME ) )
        {
            aStrScheme = OUString( INET_NEWS_SCHEME );
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
    maStrInitURL = aEmptyStr;

    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
                                       rItemSet.GetItem (SID_HYPERLINK_GETLINK);

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
sal_Bool SvxHyperlinkTabPageBase::FillItemSet( SfxItemSet& rOut)
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
    rOut.Put (aItem);

    return sal_True;
}

OUString SvxHyperlinkTabPageBase::CreateUiNameFromURL( const OUString& aStrURL )
{
    OUString          aStrUiURL;
    INetURLObject   aURLObj( aStrURL );

    switch(aURLObj.GetProtocol())
    {
        case INET_PROT_FILE:
            utl::LocalFileHelper::ConvertURLToSystemPath( aURLObj.GetMainURL(INetURLObject::NO_DECODE), aStrUiURL );
            break;
        case INET_PROT_FTP :
            {
                //remove password from name
                INetURLObject   aTmpURL(aURLObj);
                aTmpURL.SetPass(aEmptyStr);
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

// Activate / Deactivate Tabpage
void SvxHyperlinkTabPageBase::ActivatePage( const SfxItemSet& rItemSet )
{

    // Set dialog-fields from input-itemset
    SvxHyperlinkItem *pHyperlinkItem = (SvxHyperlinkItem *)
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

    return( LEAVE_PAGE );
}

sal_Bool SvxHyperlinkTabPageBase::ShouldOpenMarkWnd()
{
    return sal_False;
}

void SvxHyperlinkTabPageBase::SetMarkWndShouldOpen(sal_Bool)
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
