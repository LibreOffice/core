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

#include <stdlib.h>
#include <tools/errinf.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>
#include <svl/eitem.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/module.hxx>
#include <sfx2/filedlghelper.hxx>
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include <svl/urihelper.hxx>
#include <svtools/miscopt.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/sfxecode.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/localedatawrapper.hxx>
#include <comphelper/processfactory.hxx>
#include <vcl/layout.hxx>
#include <svx/imapdlg.hxx>
#include <svx/dialmgr.hxx>
#include <svx/dialogs.hrc>
#include <svx/svxids.hrc>
#include "imapdlg.hrc"
#include "imapwnd.hxx"
#include "imapimp.hxx"
#include "svx/dlgutil.hxx"
#include <svx/svdtrans.hxx>
#include <svx/svdopath.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <vcl/builderfactory.hxx>
#include "dlgunit.hxx"
#include <memory>

#define SELF_TARGET         "_self"
#define IMAP_ALL_FILTER     OUString("<Alle>")
#define IMAP_CERN_FILTER    "MAP - CERN"
#define IMAP_NCSA_FILTER    "MAP - NCSA"
#define IMAP_BINARY_FILTER  "SIP - StarView ImageMap"
#define IMAP_ALL_TYPE       "*.*"
#define IMAP_BINARY_TYPE    "*.sip"
#define IMAP_CERN_TYPE      "*.map"
#define IMAP_NCSA_TYPE      "*.map"

SFX_IMPL_MODELESSDIALOG_WITHID( SvxIMapDlgChildWindow, SID_IMAP );

// ControllerItem

SvxIMapDlgItem::SvxIMapDlgItem( SvxIMapDlg& rIMapDlg, SfxBindings& rBindings ) :
            SfxControllerItem   ( SID_IMAP_EXEC, rBindings ),
            rIMap               ( rIMapDlg )
{
}

void SvxIMapDlgItem::StateChanged( sal_uInt16 nSID, SfxItemState /*eState*/,
                                   const SfxPoolItem* pItem )
{
    if ( ( nSID == SID_IMAP_EXEC ) && pItem )
    {
        const SfxBoolItem* pStateItem = dynamic_cast<const SfxBoolItem*>( pItem  );
        assert(pStateItem); //SfxBoolItem expected
        if (pStateItem)
        {
            // Disable Float if possible
            rIMap.SetExecState( !pStateItem->GetValue() );
        }
    }
}

SvxIMapDlgChildWindow::SvxIMapDlgChildWindow( vcl::Window* _pParent, sal_uInt16 nId,
                                              SfxBindings* pBindings,
                                              SfxChildWinInfo* pInfo ) :
            SfxChildWindow( _pParent, nId )
{
    SetWindow(VclPtr<SvxIMapDlg>::Create( pBindings, this, _pParent ));
    SvxIMapDlg* pDlg = static_cast<SvxIMapDlg*>(GetWindow());

    if ( pInfo->nFlags & SfxChildWindowFlags::ZOOMIN )
        pDlg->RollUp();

    pDlg->Initialize( pInfo );
}

void SvxIMapDlgChildWindow::UpdateIMapDlg( const Graphic& rGraphic, const ImageMap* pImageMap,
                                           const TargetList* pTargetList, void* pEditingObj )
{
    SvxIMapDlg* pDlg = GetIMapDlg();
    if (pDlg)
        pDlg->UpdateLink(rGraphic, pImageMap, pTargetList, pEditingObj);
}

VCL_BUILDER_FACTORY(StatusBar)

SvxIMapDlg::SvxIMapDlg(SfxBindings *_pBindings, SfxChildWindow *pCW, vcl::Window* _pParent)
    : SfxModelessDialog(_pBindings, pCW, _pParent, "ImapDialog", "svx/ui/imapdialog.ui")
    , pCheckObj(nullptr)
    , aIMapItem(*this, *_pBindings)
{
    get(m_pTbxIMapDlg1, "toolbar");
    m_pTbxIMapDlg1->InsertSeparator(3, 5);
    m_pTbxIMapDlg1->InsertSeparator(9, 5);
    m_pTbxIMapDlg1->InsertSeparator(14, 5);
    m_pTbxIMapDlg1->InsertSeparator(17, 5);

    mnApplyId = m_pTbxIMapDlg1->GetItemId("TBI_APPLY");
    mnOpenId = m_pTbxIMapDlg1->GetItemId("TBI_OPEN");
    mnSaveAsId = m_pTbxIMapDlg1->GetItemId("TBI_SAVEAS");
    mnSelectId = m_pTbxIMapDlg1->GetItemId("TBI_SELECT");
    mnRectId = m_pTbxIMapDlg1->GetItemId("TBI_RECT");
    mnCircleId = m_pTbxIMapDlg1->GetItemId("TBI_CIRCLE");
    mnPolyId = m_pTbxIMapDlg1->GetItemId("TBI_POLY");
    mnFreePolyId = m_pTbxIMapDlg1->GetItemId("TBI_FREEPOLY");
    mnPolyEditId = m_pTbxIMapDlg1->GetItemId("TBI_POLYEDIT");
    mnPolyMoveId = m_pTbxIMapDlg1->GetItemId("TBI_POLYMOVE");
    mnPolyInsertId = m_pTbxIMapDlg1->GetItemId("TBI_POLYINSERT");
    mnPolyDeleteId = m_pTbxIMapDlg1->GetItemId("TBI_POLYDELETE");
    mnUndoId = m_pTbxIMapDlg1->GetItemId("TBI_UNDO");
    mnRedoId = m_pTbxIMapDlg1->GetItemId("TBI_REDO");
    mnActiveId = m_pTbxIMapDlg1->GetItemId("TBI_ACTIVE");
    mnMacroId = m_pTbxIMapDlg1->GetItemId("TBI_MACRO");
    mnPropertyId = m_pTbxIMapDlg1->GetItemId("TBI_PROPERTY");

    get(m_pFtURL, "urlft");
    get(m_pURLBox, "url");
    get(m_pFtText, "textft");
    get(m_pEdtText, "text");
    get(m_pFtTarget, "targetft");
    get(m_pCbbTarget, "target");

    //lock this down so it doesn't jump around in size
    //as entries are added later on
    TargetList aTmpList;
    SfxFrame::GetDefaultTargetList(aTmpList);
    for (const OUString & s : aTmpList)
        m_pCbbTarget->InsertEntry(s);
    Size aPrefSize(m_pCbbTarget->get_preferred_size());
    m_pCbbTarget->set_width_request(aPrefSize.Width());
    m_pCbbTarget->Clear();

    get(m_pStbStatus, "statusbar");

    VclVBox* _pContainer = get<VclVBox>("container");
    pIMapWnd = VclPtr<IMapWindow>::Create( _pContainer, WB_BORDER, _pBindings->GetActiveFrame() );
    pIMapWnd->set_hexpand(true);
    pIMapWnd->set_vexpand(true);
    pIMapWnd->Show();

    pOwnData = new IMapOwnData;

    pIMapWnd->SetInfoLink( LINK( this, SvxIMapDlg, InfoHdl ) );
    pIMapWnd->SetMousePosLink( LINK( this, SvxIMapDlg, MousePosHdl ) );
    pIMapWnd->SetGraphSizeLink( LINK( this, SvxIMapDlg, GraphSizeHdl ) );
    pIMapWnd->SetUpdateLink( LINK( this, SvxIMapDlg, StateHdl ) );

    m_pURLBox->SetModifyHdl( LINK( this, SvxIMapDlg, URLModifyHdl ) );
    m_pURLBox->SetSelectHdl( LINK( this, SvxIMapDlg, URLModifyComboBoxHdl ) );
    m_pURLBox->SetLoseFocusHdl( LINK( this, SvxIMapDlg, URLLoseFocusHdl ) );
    m_pEdtText->SetModifyHdl( LINK( this, SvxIMapDlg, URLModifyHdl ) );
    m_pCbbTarget->SetLoseFocusHdl( LINK( this, SvxIMapDlg, URLLoseFocusHdl ) );

    SvtMiscOptions aMiscOptions;
    aMiscOptions.AddListenerLink( LINK( this, SvxIMapDlg, MiscHdl ) );

    m_pTbxIMapDlg1->SetSelectHdl( LINK( this, SvxIMapDlg, TbxClickHdl ) );
    m_pTbxIMapDlg1->CheckItem( mnSelectId );
    TbxClickHdl( m_pTbxIMapDlg1 );

    SetMinOutputSizePixel( aLastSize = GetOutputSizePixel() );

    m_pStbStatus->InsertItem( 1, 130, StatusBarItemBits::Left | StatusBarItemBits::In | StatusBarItemBits::AutoSize );
    m_pStbStatus->InsertItem( 2, 10 + GetTextWidth( " 9999,99 cm / 9999,99 cm " ) );
    m_pStbStatus->InsertItem( 3, 10 + GetTextWidth( " 9999,99 cm x 9999,99 cm " ) );

    m_pFtURL->Disable();
    m_pURLBox->Disable();
    m_pFtText->Disable();
    m_pEdtText->Disable();
    m_pFtTarget->Disable();
    m_pCbbTarget->Disable();
    pOwnData->bExecState = false;

    pOwnData->aIdle.SetIdleHdl( LINK( this, SvxIMapDlg, UpdateHdl ) );

    m_pTbxIMapDlg1->EnableItem( mnActiveId, false );
    m_pTbxIMapDlg1->EnableItem( mnMacroId, false );
    m_pTbxIMapDlg1->EnableItem( mnPropertyId, false );
}

SvxIMapDlg::~SvxIMapDlg()
{
    disposeOnce();
}

void SvxIMapDlg::dispose()
{
    pIMapWnd->SetUpdateLink( Link<GraphCtrl*,void>() );

    SvtMiscOptions aMiscOptions;
    aMiscOptions.RemoveListenerLink( LINK( this, SvxIMapDlg, MiscHdl ) );

    // Delete URL-List
    pIMapWnd.disposeAndClear();
    DELETEZ( pOwnData );
    m_pTbxIMapDlg1.clear();
    m_pFtURL.clear();
    m_pURLBox.clear();
    m_pFtText.clear();
    m_pEdtText.clear();
    m_pFtTarget.clear();
    m_pCbbTarget.clear();
    m_pStbStatus.clear();
    SfxModelessDialog::dispose();
    aIMapItem.dispose();
}

bool SvxIMapDlg::Close()
{
    bool bRet = true;

    if ( m_pTbxIMapDlg1->IsItemEnabled( mnApplyId ) )
    {
        ScopedVclPtrInstance< MessageDialog > aQBox(this,"QueryModifyImageMapChangesDialog","svx/ui/querymodifyimagemapchangesdialog.ui");
        const long  nRet = aQBox->Execute();

        if( nRet == RET_YES )
        {
            SfxBoolItem aBoolItem( SID_IMAP_EXEC, true );
            GetBindings().GetDispatcher()->ExecuteList(SID_IMAP_EXEC,
                SfxCallMode::SYNCHRON | SfxCallMode::RECORD,
                { &aBoolItem });
        }
        else if( nRet == RET_CANCEL )
            bRet = false;
    }
    else if( pIMapWnd->IsChanged() )
    {
        ScopedVclPtrInstance< MessageDialog > aQBox(this,"QuerySaveImageMapChangesDialog","svx/ui/querysaveimagemapchangesdialog.ui");
        const long  nRet = aQBox->Execute();

        if( nRet == RET_YES )
            bRet = DoSave();
        else if( nRet == RET_CANCEL )
            bRet = false;
    }

    return bRet && SfxModelessDialog::Close();
}

// Enabled or disable all Controls

void SvxIMapDlg::SetExecState( bool bEnable )
{
    pOwnData->bExecState = bEnable;
}

void SvxIMapDlg::SetGraphic( const Graphic& rGraphic )
{
    pIMapWnd->SetGraphic( rGraphic );
}

void SvxIMapDlg::SetImageMap( const ImageMap& rImageMap )
{
    pIMapWnd->SetImageMap( rImageMap );
}

const ImageMap& SvxIMapDlg::GetImageMap() const
{
    return pIMapWnd->GetImageMap();
}

void SvxIMapDlg::SetTargetList( const TargetList& rTargetList )
{
    TargetList aNewList( rTargetList );

    pIMapWnd->SetTargetList( aNewList );

    m_pCbbTarget->Clear();

    for (const OUString & s : aNewList)
        m_pCbbTarget->InsertEntry( s );
}

void SvxIMapDlg::UpdateLink( const Graphic& rGraphic, const ImageMap* pImageMap,
                         const TargetList* pTargetList, void* pEditingObj )
{
    pOwnData->aUpdateGraphic = rGraphic;

    if ( pImageMap )
        pOwnData->aUpdateImageMap = *pImageMap;
    else
        pOwnData->aUpdateImageMap.ClearImageMap();

    pOwnData->pUpdateEditingObject = pEditingObj;

    // Delete UpdateTargetList, because this method can still be called several
    // times before the update timer is turned on
    pOwnData->aUpdateTargetList.clear();

    // TargetList must be copied, since it is owned by the caller and can be
    // deleted immediately after this call the copied list will be deleted
    // again in the handler
    if( pTargetList )
    {
        TargetList aTargetList( *pTargetList );

        for (const OUString & s : aTargetList)
            pOwnData->aUpdateTargetList.push_back( s );
    }

    pOwnData->aIdle.Start();
}


void SvxIMapDlg::KeyInput( const KeyEvent& rKEvt )
{
        SfxModelessDialog::KeyInput( rKEvt );
}

// Click-handler for ToolBox

IMPL_LINK_TYPED( SvxIMapDlg, TbxClickHdl, ToolBox*, pTbx, void )
{
    sal_uInt16 nNewItemId = pTbx->GetCurItemId();

    if(nNewItemId == mnApplyId)
    {
        URLLoseFocusHdl( *m_pURLBox );
        SfxBoolItem aBoolItem( SID_IMAP_EXEC, true );
        GetBindings().GetDispatcher()->ExecuteList(SID_IMAP_EXEC,
            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
            { &aBoolItem });
    }
    else if(nNewItemId == mnOpenId)
        DoOpen();
    else if(nNewItemId == mnSaveAsId)
            DoSave();
    else if(nNewItemId == mnSelectId)
    {
        SetActiveTool( nNewItemId );
        pIMapWnd->SetEditMode( true );
        if( pTbx->IsKeyEvent() )
        {
            if((pTbx->GetKeyModifier() & KEY_MOD1) != 0)
                pIMapWnd->SelectFirstObject();
            else
                pIMapWnd->GrabFocus();
        }
    }
    else if(nNewItemId == mnRectId)
    {
        SetActiveTool( nNewItemId );
        pIMapWnd->SetObjKind( OBJ_RECT );
        if( pTbx->IsKeyEvent() && ((pTbx->GetKeyModifier() & KEY_MOD1) != 0) )
        {
            pIMapWnd->CreateDefaultObject();
            pIMapWnd->GrabFocus();
        }
    }
    else if(nNewItemId == mnCircleId)
    {
        SetActiveTool( nNewItemId );
        pIMapWnd->SetObjKind( OBJ_CIRC );
        if( pTbx->IsKeyEvent() && ((pTbx->GetKeyModifier() & KEY_MOD1) != 0) )
        {
            pIMapWnd->CreateDefaultObject();
            pIMapWnd->GrabFocus();
        }
    }
    else if(nNewItemId == mnPolyId)
    {
        SetActiveTool( nNewItemId );
        pIMapWnd->SetObjKind( OBJ_POLY );
        if( pTbx->IsKeyEvent() && ((pTbx->GetKeyModifier() & KEY_MOD1) != 0) )
        {
            pIMapWnd->CreateDefaultObject();
            pIMapWnd->GrabFocus();
        }
    }
    else if(nNewItemId == mnFreePolyId)
    {
        SetActiveTool( nNewItemId );
        pIMapWnd->SetObjKind( OBJ_FREEFILL );
        if( pTbx->IsKeyEvent() && ((pTbx->GetKeyModifier() & KEY_MOD1) != 0) )
        {
            pIMapWnd->CreateDefaultObject();
            pIMapWnd->GrabFocus();
        }
    }
    else if(nNewItemId == mnActiveId)
    {
        URLLoseFocusHdl( *m_pURLBox );
        bool bNewState = !pTbx->IsItemChecked( TBI_ACTIVE );
        pTbx->CheckItem( TBI_ACTIVE, bNewState );
        pIMapWnd->SetCurrentObjState( !bNewState );
    }
    else if(nNewItemId == mnMacroId)
        pIMapWnd->DoMacroAssign();
    else if(nNewItemId == mnPropertyId)
        pIMapWnd->DoPropertyDialog();
    else if(nNewItemId == mnPolyEditId)
    {
        SetActiveTool( nNewItemId );
        pIMapWnd->SetPolyEditMode( pTbx->IsItemChecked( TBI_POLYEDIT ) ? SID_BEZIER_MOVE : 0 );
        if( pTbx->IsKeyEvent() && pTbx->IsItemChecked( TBI_POLYEDIT ) )
            pIMapWnd->StartPolyEdit();
    }
    else if(nNewItemId == mnPolyMoveId)
    {
        SetActiveTool( nNewItemId );
        pIMapWnd->SetPolyEditMode( SID_BEZIER_MOVE );
    }
    else if(nNewItemId == mnPolyInsertId)
    {
        SetActiveTool( nNewItemId );
        pIMapWnd->SetPolyEditMode( SID_BEZIER_INSERT );
    }
    else if(nNewItemId == mnPolyDeleteId)
    {
        SetActiveTool( nNewItemId );
        pIMapWnd->GetSdrView()->DeleteMarkedPoints();
    }
    else if(nNewItemId == mnUndoId)
    {
        URLLoseFocusHdl( *m_pURLBox );
        pIMapWnd->GetSdrModel()->Undo();
    }
    else if(nNewItemId == mnRedoId)
    {
        URLLoseFocusHdl( *m_pURLBox );
        pIMapWnd->GetSdrModel()->Redo();
    }
}

void SvxIMapDlg::DoOpen()
{
    ::sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE );

    ImageMap        aLoadIMap;
    const OUString  aFilter( IMAP_ALL_FILTER );

    aDlg.AddFilter( aFilter, IMAP_ALL_TYPE );
    aDlg.AddFilter( IMAP_CERN_FILTER, IMAP_CERN_TYPE );
    aDlg.AddFilter( IMAP_NCSA_FILTER, IMAP_NCSA_TYPE );
    aDlg.AddFilter( IMAP_BINARY_FILTER, IMAP_BINARY_TYPE );

    aDlg.SetCurrentFilter( aFilter );
    aDlg.SetDisplayDirectory( SvtPathOptions().GetWorkPath() );

    if( aDlg.Execute() == ERRCODE_NONE )
    {
        INetURLObject aURL( aDlg.GetPath() );
        DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );
        std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), StreamMode::READ ));

        if( pIStm )
        {
            aLoadIMap.Read( *pIStm, IMAP_FORMAT_DETECT, "" );

            if( pIStm->GetError() )
            {
                SfxErrorContext eEC(ERRCODE_SFX_GENERAL,this);
                ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
            }
            else
                pIMapWnd->SetImageMap( aLoadIMap );
        }

        pIMapWnd->Invalidate();
    }
}

bool SvxIMapDlg::DoSave()
{
    ::sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE );

    const OUString    aBinFilter( IMAP_BINARY_FILTER );
    const OUString    aCERNFilter( IMAP_CERN_FILTER );
    const OUString    aNCSAFilter( IMAP_NCSA_FILTER );
    SdrModel*       pModel = pIMapWnd->GetSdrModel();
    const bool bChanged = pModel->IsChanged();
    bool            bRet = false;

    aDlg.AddFilter( aCERNFilter, IMAP_CERN_TYPE );
    aDlg.AddFilter( aNCSAFilter, IMAP_NCSA_TYPE );
    aDlg.AddFilter( aBinFilter, IMAP_BINARY_TYPE );

    aDlg.SetCurrentFilter( aCERNFilter );
    aDlg.SetDisplayDirectory( SvtPathOptions().GetWorkPath() );

    if( aDlg.Execute() == ERRCODE_NONE )
    {
        const OUString    aFilter( aDlg.GetCurrentFilter() );
        OUString          aExt;
        sal_uIntPtr       nFormat;

        if ( aFilter == aBinFilter )
        {
            nFormat = IMAP_FORMAT_BIN;
            aExt = "sip";
        }
        else if ( aFilter == aCERNFilter )
        {
            nFormat = IMAP_FORMAT_CERN;
            aExt = "map";
        }
        else if ( aFilter == aNCSAFilter )
        {
            nFormat = IMAP_FORMAT_NCSA;
            aExt = "map";
        }
        else
        {
            return false;
        }

        INetURLObject aURL( aDlg.GetPath() );

        if( aURL.GetProtocol() == INetProtocol::NotValid )
        {
            OSL_FAIL( "invalid URL" );
        }
        else
        {
            if( aURL.getExtension().isEmpty() )
                aURL.setExtension( aExt );

            std::unique_ptr<SvStream> pOStm(::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), StreamMode::WRITE | StreamMode::TRUNC ));
            if( pOStm )
            {
                pIMapWnd->GetImageMap().Write( *pOStm, nFormat, "" );

                if( pOStm->GetError() )
                    ErrorHandler::HandleError( ERRCODE_IO_GENERAL );

                pOStm.reset();
                pModel->SetChanged( bChanged );
                bRet = true;
            }
        }
    }

    return bRet;
}

void SvxIMapDlg::SetActiveTool( sal_uInt16 nId )
{
    m_pTbxIMapDlg1->CheckItem( mnSelectId, mnSelectId == nId );
    m_pTbxIMapDlg1->CheckItem( mnRectId, mnRectId == nId );
    m_pTbxIMapDlg1->CheckItem( mnCircleId, mnCircleId == nId );
    m_pTbxIMapDlg1->CheckItem( mnPolyId, mnPolyId == nId );
    m_pTbxIMapDlg1->CheckItem( mnFreePolyId, mnFreePolyId == nId );

    m_pTbxIMapDlg1->CheckItem( mnPolyInsertId, mnPolyInsertId == nId );
    m_pTbxIMapDlg1->CheckItem( mnPolyDeleteId, false );

    bool bMove = mnPolyMoveId == nId
                || ( mnPolyEditId == nId
                && !m_pTbxIMapDlg1->IsItemChecked( TBI_POLYINSERT )
                && !m_pTbxIMapDlg1->IsItemChecked( TBI_POLYDELETE ) );

    m_pTbxIMapDlg1->CheckItem( mnPolyMoveId, bMove );

    bool bEditMode = ( mnPolyEditId == nId )
                    || ( mnPolyMoveId == nId )
                    || ( mnPolyInsertId == nId )
                    || ( mnPolyDeleteId == nId );

    m_pTbxIMapDlg1->CheckItem( mnPolyEditId, bEditMode );
}

IMPL_LINK_TYPED( SvxIMapDlg, InfoHdl, IMapWindow&, rWnd, void )
{
    OUString            aStr;
    const NotifyInfo&   rInfo = rWnd.GetInfo();

    if ( rInfo.bNewObj )
    {
        if( !rInfo.aMarkURL.isEmpty() && ( m_pURLBox->GetEntryPos( rInfo.aMarkURL ) == LISTBOX_ENTRY_NOTFOUND ) )
            m_pURLBox->InsertEntry( rInfo.aMarkURL );

        m_pURLBox->SetText( rInfo.aMarkURL );
        m_pEdtText->SetText( rInfo.aMarkAltText );

        if ( rInfo.aMarkTarget.isEmpty() )
            m_pCbbTarget->SetText( SELF_TARGET );
        else
            m_pCbbTarget->SetText( rInfo.aMarkTarget );
    }

    if ( !rInfo.bOneMarked )
    {
        m_pTbxIMapDlg1->CheckItem( mnActiveId, false );
        m_pTbxIMapDlg1->EnableItem( mnActiveId, false );
        m_pTbxIMapDlg1->EnableItem( mnMacroId, false );
        m_pTbxIMapDlg1->EnableItem( mnPropertyId, false );
        m_pStbStatus->SetItemText( 1, aStr );

        m_pFtURL->Disable();
        m_pURLBox->Disable();
        m_pFtText->Disable();
        m_pEdtText->Disable();
        m_pFtTarget->Disable();
        m_pCbbTarget->Disable();

        m_pURLBox->SetText( "" );
        m_pEdtText->SetText( "" );
    }
    else
    {
        m_pTbxIMapDlg1->EnableItem( mnActiveId );
        m_pTbxIMapDlg1->CheckItem( mnActiveId, !rInfo.bActivated );
        m_pTbxIMapDlg1->EnableItem( mnMacroId );
        m_pTbxIMapDlg1->EnableItem( mnPropertyId );

        m_pFtURL->Enable();
        m_pURLBox->Enable();
        m_pFtText->Enable();
        m_pEdtText->Enable();
        m_pFtTarget->Enable();
        m_pCbbTarget->Enable();

        m_pStbStatus->SetItemText( 1, rInfo.aMarkURL );

        if ( m_pURLBox->GetText() != rInfo.aMarkURL )
            m_pURLBox->SetText( rInfo.aMarkURL );

        if ( m_pEdtText->GetText() != rInfo.aMarkAltText )
            m_pEdtText->SetText( rInfo.aMarkAltText );

        if ( rInfo.aMarkTarget.isEmpty() )
            m_pCbbTarget->SetText( SELF_TARGET );
        else
            m_pCbbTarget->SetText(  rInfo.aMarkTarget );
    }
}

IMPL_LINK_TYPED( SvxIMapDlg, MousePosHdl, GraphCtrl*, pWnd, void )
{
    const FieldUnit eFieldUnit = GetBindings().GetDispatcher()->GetModule()->GetFieldUnit();
    const Point& rMousePos = pWnd->GetMousePos();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    OUString aStr = GetUnitString( rMousePos.X(), eFieldUnit, cSep ) +
                    " / " + GetUnitString( rMousePos.Y(), eFieldUnit, cSep );

    m_pStbStatus->SetItemText( 2, aStr );
}

IMPL_LINK_TYPED( SvxIMapDlg, GraphSizeHdl, GraphCtrl*, pWnd, void )
{
    const FieldUnit eFieldUnit = GetBindings().GetDispatcher()->GetModule()->GetFieldUnit();
    const Size& rSize = pWnd->GetGraphicSize();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    OUString aStr = GetUnitString( rSize.Width(), eFieldUnit, cSep ) +
                    " x " + GetUnitString( rSize.Height(), eFieldUnit, cSep );

    m_pStbStatus->SetItemText( 3, aStr );
}


IMPL_LINK_NOARG_TYPED(SvxIMapDlg, URLModifyComboBoxHdl, ComboBox&, void)
{
    URLModifyHdl(*m_pURLBox);
}
IMPL_LINK_NOARG_TYPED(SvxIMapDlg, URLModifyHdl, Edit&, void)
{
    NotifyInfo  aNewInfo;

    aNewInfo.aMarkURL = m_pURLBox->GetText();
    aNewInfo.aMarkAltText = m_pEdtText->GetText();
    aNewInfo.aMarkTarget = m_pCbbTarget->GetText();

    pIMapWnd->ReplaceActualIMapInfo( aNewInfo );
}

IMPL_LINK_NOARG_TYPED(SvxIMapDlg, URLLoseFocusHdl, Control&, void)
{
    NotifyInfo        aNewInfo;
    const OUString    aURLText( m_pURLBox->GetText() );
    const OUString    aTargetText( m_pCbbTarget->GetText() );

    if ( !aURLText.isEmpty() )
    {
        OUString aBase = GetBindings().GetDispatcher()->GetFrame()->GetObjectShell()->GetMedium()->GetBaseURL();
        aNewInfo.aMarkURL = ::URIHelper::SmartRel2Abs( INetURLObject(aBase), aURLText, URIHelper::GetMaybeFileHdl(), true, false,
                                                        INetURLObject::WAS_ENCODED,
                                                        INetURLObject::DECODE_UNAMBIGUOUS );
    }
    else
        aNewInfo.aMarkURL = aURLText;

    aNewInfo.aMarkAltText = m_pEdtText->GetText();

    if ( aTargetText.isEmpty() )
        aNewInfo.aMarkTarget = SELF_TARGET;
    else
        aNewInfo.aMarkTarget = aTargetText;

    pIMapWnd->ReplaceActualIMapInfo( aNewInfo );
}

IMPL_LINK_NOARG_TYPED(SvxIMapDlg, UpdateHdl, Idle *, void)
{
    pOwnData->aIdle.Stop();

    if ( pOwnData->pUpdateEditingObject != pCheckObj )
    {
        if ( pIMapWnd->IsChanged() &&
             ( ScopedVclPtrInstance<MessageDialog>(this,"QuerySaveImageMapChangesDialog",
             "svx/ui/querysaveimagemapchangesdialog.ui")->Execute() == RET_YES ) )
        {
            DoSave();
        }

        SetGraphic( pOwnData->aUpdateGraphic );
        SetImageMap( pOwnData->aUpdateImageMap );
        SetTargetList( pOwnData->aUpdateTargetList );
        SetEditingObject( pOwnData->pUpdateEditingObject );

        // After changes => default selection
        m_pTbxIMapDlg1->CheckItem( mnSelectId );
        pIMapWnd->SetEditMode( true );
    }

    // Delete the copied list again in the Update method
    pOwnData->aUpdateTargetList.clear();

    GetBindings().Invalidate( SID_IMAP_EXEC );
}

IMPL_LINK_TYPED( SvxIMapDlg, StateHdl, GraphCtrl*, pWnd, void )
{
    const SdrObject*    pObj = pWnd->GetSelectedSdrObject();
    const SdrModel*     pModel = pWnd->GetSdrModel();
    const SdrView*      pView = pWnd->GetSdrView();
    const bool          bPolyEdit = ( pObj != nullptr ) && dynamic_cast<const SdrPathObj*>( pObj) !=  nullptr;
    const bool          bDrawEnabled = !( bPolyEdit && m_pTbxIMapDlg1->IsItemChecked( mnPolyEditId ) );

    m_pTbxIMapDlg1->EnableItem( mnApplyId, pOwnData->bExecState && pWnd->IsChanged() );

    m_pTbxIMapDlg1->EnableItem( mnSelectId, bDrawEnabled );
    m_pTbxIMapDlg1->EnableItem( mnRectId, bDrawEnabled );
    m_pTbxIMapDlg1->EnableItem( mnCircleId, bDrawEnabled );
    m_pTbxIMapDlg1->EnableItem( mnPolyId, bDrawEnabled );
    m_pTbxIMapDlg1->EnableItem( mnFreePolyId, bDrawEnabled );

    // BezierEditor State
    m_pTbxIMapDlg1->EnableItem( mnPolyEditId, bPolyEdit );
    m_pTbxIMapDlg1->EnableItem( mnPolyMoveId, !bDrawEnabled );
    m_pTbxIMapDlg1->EnableItem( mnPolyInsertId, !bDrawEnabled );
    m_pTbxIMapDlg1->EnableItem( mnPolyDeleteId, !bDrawEnabled && pView->IsDeleteMarkedPointsPossible() );

    // Undo/Redo
    m_pTbxIMapDlg1->EnableItem( mnUndoId, pModel->HasUndoActions() );
    m_pTbxIMapDlg1->EnableItem( mnRedoId, pModel->HasRedoActions() );

    if ( bPolyEdit )
    {
        sal_uInt16 nId = 0;

        switch( pWnd->GetPolyEditMode() )
        {
            case SID_BEZIER_MOVE: nId = mnPolyMoveId; break;
            case SID_BEZIER_INSERT: nId = mnPolyInsertId; break;

            default:
            break;
        }

        m_pTbxIMapDlg1->CheckItem( nId );
    }
    else
    {
        m_pTbxIMapDlg1->CheckItem( mnPolyEditId, false );
        m_pTbxIMapDlg1->CheckItem( mnPolyMoveId );
        m_pTbxIMapDlg1->CheckItem( mnPolyInsertId, false );
        pWnd->SetPolyEditMode( 0 );
    }
}

IMPL_LINK_NOARG_TYPED(SvxIMapDlg, MiscHdl, LinkParamNone*, void)
{
    if (m_pTbxIMapDlg1)
    {
        SvtMiscOptions aMiscOptions;
        m_pTbxIMapDlg1->SetOutStyle( aMiscOptions.GetToolboxStyle() );
    }
}

SvxIMapDlg* GetIMapDlg()
{
    SfxChildWindow* pWnd = nullptr;
    if (SfxViewFrame::Current() && SfxViewFrame::Current()->HasChildWindow(SvxIMapDlgChildWindow::GetChildWindowId()))
        pWnd = SfxViewFrame::Current()->GetChildWindow(SvxIMapDlgChildWindow::GetChildWindowId());
    return pWnd ? static_cast<SvxIMapDlg*>(pWnd->GetWindow()) : nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
