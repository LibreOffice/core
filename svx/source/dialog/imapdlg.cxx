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
#include <tools/shl.hxx>
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

#define _IMAPDLG_PRIVATE
#include "svx/imapdlg.hxx"

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
#include "dlgunit.hxx"

#define SELF_TARGET         "_self"
#define IMAP_ALL_FILTER     OUString("<Alle>")
#define IMAP_CERN_FILTER    OUString("MAP - CERN")
#define IMAP_NCSA_FILTER    OUString("MAP - NCSA")
#define IMAP_BINARY_FILTER  OUString("SIP - StarView ImageMap")
#define IMAP_ALL_TYPE       OUString("*.*")
#define IMAP_BINARY_TYPE    OUString("*.sip")
#define IMAP_CERN_TYPE      OUString("*.map")
#define IMAP_NCSA_TYPE      OUString("*.map")
#define IMAP_BINARY_EXT     OUString("sip")
#define IMAP_CERN_EXT       OUString("map")
#define IMAP_NCSA_EXT       OUString("map")

/******************************************************************************/

SFX_IMPL_MODELESSDIALOG_WITHID( SvxIMapDlgChildWindow, SID_IMAP );

// ControllerItem

SvxIMapDlgItem::SvxIMapDlgItem( sal_uInt16 _nId, SvxIMapDlg& rIMapDlg, SfxBindings& rBindings ) :
            SfxControllerItem   ( _nId, rBindings ),
            rIMap               ( rIMapDlg )
{
}

void SvxIMapDlgItem::StateChanged( sal_uInt16 nSID, SfxItemState /*eState*/,
                                   const SfxPoolItem* pItem )
{
    if ( ( nSID == SID_IMAP_EXEC ) && pItem )
    {
        const SfxBoolItem* pStateItem = PTR_CAST( SfxBoolItem, pItem );

        DBG_ASSERT( pStateItem || pItem == 0, "SfxBoolItem expected");

        // Disable Float if possible
        rIMap.SetExecState( !pStateItem->GetValue() );
    }
}

SvxIMapDlgChildWindow::SvxIMapDlgChildWindow( Window* _pParent, sal_uInt16 nId,
                                              SfxBindings* pBindings,
                                              SfxChildWinInfo* pInfo ) :
            SfxChildWindow( _pParent, nId )
{
    pWindow = new SvxIMapDlg( pBindings, this, _pParent, SVX_RES( RID_SVXDLG_IMAP ) );
    SvxIMapDlg* pDlg = (SvxIMapDlg*) pWindow;

    if ( pInfo->nFlags & SFX_CHILDWIN_ZOOMIN )
        pDlg->RollUp();

    eChildAlignment = SFX_ALIGN_NOALIGNMENT;

    pDlg->Initialize( pInfo );
}

void SvxIMapDlgChildWindow::UpdateIMapDlg( const Graphic& rGraphic, const ImageMap* pImageMap,
                                           const TargetList* pTargetList, void* pEditingObj )
{
    if ( SfxViewFrame::Current() &&
         SfxViewFrame::Current()->HasChildWindow( SvxIMapDlgChildWindow::GetChildWindowId() ) )
        SVXIMAPDLG()->Update( rGraphic, pImageMap, pTargetList, pEditingObj );
}

SvxIMapDlg::SvxIMapDlg( SfxBindings *_pBindings, SfxChildWindow *pCW,
                        Window* _pParent, const ResId& rResId ) :
        SfxModelessDialog   ( _pBindings, pCW, _pParent, rResId ),

        aTbxIMapDlg1        ( this, SVX_RES( TBX_IMAPDLG1 ) ),
        aFtURL              ( this, SVX_RES( FT_URL ) ),
        maURLBox            ( this, SVX_RES( CBB_URL ) ),
        aFtText             ( this, SVX_RES( FT_TEXT ) ),
        aEdtText            ( this, SVX_RES( EDT_TEXT ) ),
        maFtTarget          ( this, SVX_RES( RID_SVXCTL_FT_TARGET ) ),
        maCbbTarget         ( this, SVX_RES( RID_SVXCTL_CBB_TARGET ) ),
        aStbStatus          ( this, WB_BORDER | WB_3DLOOK | WB_LEFT ),
        maImageList         ( SVX_RES( IL_IMAPDLG ) ),
        pCheckObj           ( NULL ),
        aIMapItem           ( SID_IMAP_EXEC, *this, *_pBindings )
{
    pIMapWnd = new IMapWindow( this, SVX_RES( RID_SVXCTL_IMAP ), _pBindings->GetActiveFrame() );

    ApplyImageList();

    FreeResource();

    pOwnData = new IMapOwnData;

    pIMapWnd->SetInfoLink( LINK( this, SvxIMapDlg, InfoHdl ) );
    pIMapWnd->SetMousePosLink( LINK( this, SvxIMapDlg, MousePosHdl ) );
    pIMapWnd->SetGraphSizeLink( LINK( this, SvxIMapDlg, GraphSizeHdl ) );
    pIMapWnd->SetUpdateLink( LINK( this, SvxIMapDlg, StateHdl ) );

    maURLBox.SetModifyHdl( LINK( this, SvxIMapDlg, URLModifyHdl ) );
    maURLBox.SetSelectHdl( LINK( this, SvxIMapDlg, URLModifyHdl ) );
    maURLBox.SetLoseFocusHdl( LINK( this, SvxIMapDlg, URLLoseFocusHdl ) );
    aEdtText.SetModifyHdl( LINK( this, SvxIMapDlg, URLModifyHdl ) );
    maCbbTarget.SetLoseFocusHdl( LINK( this, SvxIMapDlg, URLLoseFocusHdl ) );

       SvtMiscOptions aMiscOptions;
    aMiscOptions.AddListenerLink( LINK( this, SvxIMapDlg, MiscHdl ) );

    aTbxIMapDlg1.SetOutStyle( aMiscOptions.GetToolboxStyle() );
    aTbxIMapDlg1.SetSizePixel( aTbxIMapDlg1.CalcWindowSizePixel() );
    aTbxIMapDlg1.SetSelectHdl( LINK( this, SvxIMapDlg, TbxClickHdl ) );
    aTbxIMapDlg1.CheckItem( TBI_SELECT, sal_True );
    TbxClickHdl( &aTbxIMapDlg1 );

    SetMinOutputSizePixel( aLastSize = GetOutputSizePixel() );

    aStbStatus.InsertItem( 1, 130, SIB_LEFT | SIB_IN | SIB_AUTOSIZE );
    aStbStatus.InsertItem( 2, 10 + GetTextWidth( OUString(" 9999,99 cm / 9999,99 cm ") ), SIB_CENTER | SIB_IN );
    aStbStatus.InsertItem( 3, 10 + GetTextWidth( OUString(" 9999,99 cm x 9999,99 cm ") ), SIB_CENTER | SIB_IN );

    aFtURL.Disable();
    maURLBox.Disable();
    aFtText.Disable();
    aEdtText.Disable();
    maFtTarget.Disable();
    maCbbTarget.Disable();
    pOwnData->bExecState = sal_False;

    Resize();

    pOwnData->aTimer.SetTimeout( 100 );
    pOwnData->aTimer.SetTimeoutHdl( LINK( this, SvxIMapDlg, UpdateHdl ) );

    aTbxIMapDlg1.EnableItem( TBI_ACTIVE, sal_False );
    aTbxIMapDlg1.EnableItem( TBI_MACRO, sal_False );
    aTbxIMapDlg1.EnableItem( TBI_PROPERTY, sal_False );
}

SvxIMapDlg::~SvxIMapDlg()
{
    // Delete URL-List
    delete pIMapWnd;
    delete pOwnData;
}

void SvxIMapDlg::Resize()
{
    SfxModelessDialog::Resize();

    Size aMinSize( GetMinOutputSizePixel() );
    Size aNewSize( GetOutputSizePixel() );

    if ( aNewSize.Height() >= aMinSize.Height() )
    {
        Size    _aSize( aStbStatus.GetSizePixel() );
        Point   aPoint( 0, aNewSize.Height() - _aSize.Height() );

        // Position the StatusBar
        aStbStatus.SetPosSizePixel( aPoint, Size( aNewSize.Width(), _aSize.Height() ) );
        aStbStatus.Show();

        // Position the EditWindow
        _aSize.Width() = aNewSize.Width() - 18;
        _aSize.Height() = aPoint.Y() - pIMapWnd->GetPosPixel().Y() - 6;
        pIMapWnd->SetSizePixel( _aSize );

        aLastSize = aNewSize;
    }
}

sal_Bool SvxIMapDlg::Close()
{
    sal_Bool bRet = sal_True;

    if ( aTbxIMapDlg1.IsItemEnabled( TBI_APPLY ) )
    {
        QueryBox    aQBox( this, WB_YES_NO_CANCEL | WB_DEF_YES,
                           SVX_RESSTR( STR_IMAPDLG_MODIFY ) );
        const long  nRet = aQBox.Execute();

        if( nRet == RET_YES )
        {
            SfxBoolItem aBoolItem( SID_IMAP_EXEC, sal_True );
            GetBindings().GetDispatcher()->Execute(
                SID_IMAP_EXEC, SFX_CALLMODE_SYNCHRON | SFX_CALLMODE_RECORD, &aBoolItem, 0L );
        }
        else if( nRet == RET_CANCEL )
            bRet = sal_False;
    }
    else if( pIMapWnd->IsChanged() )
    {
        QueryBox    aQBox( this, WB_YES_NO_CANCEL | WB_DEF_YES,
                           SVX_RESSTR( STR_IMAPDLG_SAVE ) );
        const long  nRet = aQBox.Execute();

        if( nRet == RET_YES )
            bRet = DoSave();
        else if( nRet == RET_CANCEL )
            bRet = sal_False;
    }

    return( bRet ? SfxModelessDialog::Close() : sal_False );
}

// Enabled or disable all Controls

void SvxIMapDlg::SetExecState( sal_Bool bEnable )
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
    TargetList  aNewList( rTargetList );

    pIMapWnd->SetTargetList( aNewList );

    maCbbTarget.Clear();

    for ( size_t i = 0, n = aNewList.size(); i < n; ++i )
        maCbbTarget.InsertEntry( aNewList[ i ] );
}

void SvxIMapDlg::Update( const Graphic& rGraphic, const ImageMap* pImageMap,
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

        for ( size_t i = 0, n = aTargetList.size(); i < n; ++i )
            pOwnData->aUpdateTargetList.push_back( aTargetList[ i ] );
    }

    pOwnData->aTimer.Start();
}


void SvxIMapDlg::KeyInput( const KeyEvent& rKEvt )
{
        SfxModelessDialog::KeyInput( rKEvt );
}

// Click-handler for ToolBox

IMPL_LINK( SvxIMapDlg, TbxClickHdl, ToolBox*, pTbx )
{
    sal_uInt16 nNewItemId = pTbx->GetCurItemId();

    switch( pTbx->GetCurItemId() )
    {
        case( TBI_APPLY ):
        {
            URLLoseFocusHdl( NULL );
            SfxBoolItem aBoolItem( SID_IMAP_EXEC, sal_True );
            GetBindings().GetDispatcher()->Execute(
                SID_IMAP_EXEC, SFX_CALLMODE_ASYNCHRON | SFX_CALLMODE_RECORD, &aBoolItem, 0L );
        }
        break;

        case( TBI_OPEN ):
            DoOpen();
        break;

        case( TBI_SAVEAS ):
            DoSave();
        break;

        case( TBI_SELECT ):
        {
            pTbx->CheckItem( nNewItemId, sal_True );
            pIMapWnd->SetEditMode( sal_True );
            if( pTbx->IsKeyEvent() )
            {
                if((pTbx->GetKeyModifier() & KEY_MOD1) != 0)
                    pIMapWnd->SelectFirstObject();
                else
                    pIMapWnd->GrabFocus();
            }
        }
        break;

        case( TBI_RECT ):
        {
            pTbx->CheckItem( nNewItemId, sal_True );
            pIMapWnd->SetObjKind( OBJ_RECT );
            if( pTbx->IsKeyEvent() && ((pTbx->GetKeyModifier() & KEY_MOD1) != 0) )
            {
                pIMapWnd->CreateDefaultObject();
                pIMapWnd->GrabFocus();
            }
        }
        break;

        case( TBI_CIRCLE ):
        {
            pTbx->CheckItem( nNewItemId, sal_True );
            pIMapWnd->SetObjKind( OBJ_CIRC );
            if( pTbx->IsKeyEvent() && ((pTbx->GetKeyModifier() & KEY_MOD1) != 0) )
            {
                pIMapWnd->CreateDefaultObject();
                pIMapWnd->GrabFocus();
            }
        }
        break;

        case( TBI_POLY ):
        {
            pTbx->CheckItem( nNewItemId, sal_True );
            pIMapWnd->SetObjKind( OBJ_POLY );
            if( pTbx->IsKeyEvent() && ((pTbx->GetKeyModifier() & KEY_MOD1) != 0) )
            {
                pIMapWnd->CreateDefaultObject();
                pIMapWnd->GrabFocus();
            }
        }
        break;

        case( TBI_FREEPOLY ):
        {
            pTbx->CheckItem( nNewItemId, sal_True );
            pIMapWnd->SetObjKind( OBJ_FREEFILL );
            if( pTbx->IsKeyEvent() && ((pTbx->GetKeyModifier() & KEY_MOD1) != 0) )
            {
                pIMapWnd->CreateDefaultObject();
                pIMapWnd->GrabFocus();
            }
        }
        break;

        case( TBI_ACTIVE ):
        {
            URLLoseFocusHdl( NULL );
            bool bNewState = !pTbx->IsItemChecked( TBI_ACTIVE );
            pTbx->CheckItem( TBI_ACTIVE, bNewState );
            pIMapWnd->SetCurrentObjState( !bNewState );
        }
        break;

        case( TBI_MACRO ):
            pIMapWnd->DoMacroAssign();
        break;

        case( TBI_PROPERTY ):
            pIMapWnd->DoPropertyDialog();
        break;

        case( TBI_POLYEDIT ):
            pIMapWnd->SetPolyEditMode( pTbx->IsItemChecked( TBI_POLYEDIT ) ? SID_BEZIER_MOVE : 0 );
            if( pTbx->IsKeyEvent() && pTbx->IsItemChecked( TBI_POLYEDIT ) )
                pIMapWnd->StartPolyEdit();
        break;

        case( TBI_POLYMOVE ):
            pIMapWnd->SetPolyEditMode( SID_BEZIER_MOVE );
        break;

        case( TBI_POLYINSERT ):
            pIMapWnd->SetPolyEditMode( SID_BEZIER_INSERT );
        break;

        case( TBI_POLYDELETE ):
            pIMapWnd->GetSdrView()->DeleteMarkedPoints();
        break;

        case( TBI_UNDO ):
        {
            URLLoseFocusHdl( NULL );
            pIMapWnd->GetSdrModel()->Undo();
        }
        break;

        case( TBI_REDO ):
        {
            URLLoseFocusHdl( NULL );
            pIMapWnd->GetSdrModel()->Redo();
        }
        break;

        default:
        break;
    }

    return 0;
}

void SvxIMapDlg::DoOpen()
{
    ::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );

    ImageMap        aLoadIMap;
    const String    aFilter( IMAP_ALL_FILTER );

    aDlg.AddFilter( aFilter, IMAP_ALL_TYPE );
    aDlg.AddFilter( IMAP_CERN_FILTER, IMAP_CERN_TYPE );
    aDlg.AddFilter( IMAP_NCSA_FILTER, IMAP_NCSA_TYPE );
    aDlg.AddFilter( IMAP_BINARY_FILTER, IMAP_BINARY_TYPE );

    aDlg.SetCurrentFilter( aFilter );
    aDlg.SetDisplayDirectory( SvtPathOptions().GetWorkPath() );

    if( aDlg.Execute() == ERRCODE_NONE )
    {
        INetURLObject aURL( aDlg.GetPath() );
        DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
        SvStream* pIStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ );

        if( pIStm )
        {
            aLoadIMap.Read( *pIStm, IMAP_FORMAT_DETECT, String() );

            if( pIStm->GetError() )
            {
                SfxErrorContext eEC(ERRCODE_SFX_GENERAL,this);
                ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
            }
            else
                pIMapWnd->SetImageMap( aLoadIMap );

            delete pIStm;
        }

        pIMapWnd->Invalidate();
    }
}

sal_Bool SvxIMapDlg::DoSave()
{
    ::sfx2::FileDialogHelper aDlg(
        com::sun::star::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE, 0 );

    const String    aBinFilter( IMAP_BINARY_FILTER );
    const String    aCERNFilter( IMAP_CERN_FILTER );
    const String    aNCSAFilter( IMAP_NCSA_FILTER );
    SdrModel*       pModel = pIMapWnd->GetSdrModel();
    const sal_Bool bChanged = pModel->IsChanged();
    sal_Bool            bRet = false;

    aDlg.AddFilter( aCERNFilter, IMAP_CERN_TYPE );
    aDlg.AddFilter( aNCSAFilter, IMAP_NCSA_TYPE );
    aDlg.AddFilter( aBinFilter, IMAP_BINARY_TYPE );

    aDlg.SetCurrentFilter( aCERNFilter );
    aDlg.SetDisplayDirectory( SvtPathOptions().GetWorkPath() );

    if( aDlg.Execute() == ERRCODE_NONE )
    {
        const String    aFilter( aDlg.GetCurrentFilter() );
        String          aExt;
        sal_uIntPtr         nFormat;

        if ( aFilter == aBinFilter )
        {
            nFormat = IMAP_FORMAT_BIN;
            aExt = IMAP_BINARY_EXT;
        }
        else if ( aFilter == aCERNFilter )
        {
            nFormat = IMAP_FORMAT_CERN;
            aExt = IMAP_CERN_EXT;
        }
        else if ( aFilter == aNCSAFilter )
        {
            nFormat = IMAP_FORMAT_NCSA;
            aExt = IMAP_NCSA_EXT;
        }
        else
        {
            return sal_False;
        }

        INetURLObject aURL( aDlg.GetPath() );

        if( aURL.GetProtocol() == INET_PROT_NOT_VALID )
        {
            OSL_FAIL( "invalid URL" );
        }
        else
        {
            if( aURL.getExtension().isEmpty() )
                aURL.setExtension( aExt );

            SvStream* pOStm = ::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::NO_DECODE ), STREAM_WRITE | STREAM_TRUNC );
            if( pOStm )
            {
                pIMapWnd->GetImageMap().Write( *pOStm, nFormat, String() );

                if( pOStm->GetError() )
                    ErrorHandler::HandleError( ERRCODE_IO_GENERAL );

                delete pOStm;
                pModel->SetChanged( bChanged );
                bRet = sal_True;
            }
        }
    }

    return bRet;
}

IMPL_LINK( SvxIMapDlg, InfoHdl, IMapWindow*, pWnd )
{
    String              aStr;
    const NotifyInfo&   rInfo = pWnd->GetInfo();

    if ( rInfo.bNewObj )
    {
        if( rInfo.aMarkURL.Len() && ( maURLBox.GetEntryPos( rInfo.aMarkURL ) == LISTBOX_ENTRY_NOTFOUND ) )
            maURLBox.InsertEntry( rInfo.aMarkURL );

        maURLBox.SetText( rInfo.aMarkURL );
        aEdtText.SetText( rInfo.aMarkAltText );

        if ( !rInfo.aMarkTarget.Len() )
            maCbbTarget.SetText( SELF_TARGET );
        else
            maCbbTarget.SetText( rInfo.aMarkTarget );
    }

    if ( !rInfo.bOneMarked )
    {
        aTbxIMapDlg1.CheckItem( TBI_ACTIVE, sal_False );
        aTbxIMapDlg1.EnableItem( TBI_ACTIVE, sal_False );
        aTbxIMapDlg1.EnableItem( TBI_MACRO, sal_False );
        aTbxIMapDlg1.EnableItem( TBI_PROPERTY, sal_False );
        aStbStatus.SetItemText( 1, aStr );

        aFtURL.Disable();
        maURLBox.Disable();
        aFtText.Disable();
        aEdtText.Disable();
        maFtTarget.Disable();
        maCbbTarget.Disable();

        maURLBox.SetText( String() );
        aEdtText.SetText( String() );
    }
    else
    {
        aTbxIMapDlg1.EnableItem( TBI_ACTIVE, sal_True );
        aTbxIMapDlg1.CheckItem( TBI_ACTIVE, !rInfo.bActivated );
        aTbxIMapDlg1.EnableItem( TBI_MACRO, sal_True );
        aTbxIMapDlg1.EnableItem( TBI_PROPERTY, sal_True );

        aFtURL.Enable();
        maURLBox.Enable();
        aFtText.Enable();
        aEdtText.Enable();
        maFtTarget.Enable();
        maCbbTarget.Enable();

        aStbStatus.SetItemText( 1, rInfo.aMarkURL );

        if ( maURLBox.GetText() != OUString(rInfo.aMarkURL) )
            maURLBox.SetText( rInfo.aMarkURL );

        if ( aEdtText.GetText() != OUString(rInfo.aMarkAltText) )
            aEdtText.SetText( rInfo.aMarkAltText );

        if ( !rInfo.aMarkTarget.Len() )
            maCbbTarget.SetText( SELF_TARGET );
        else
            maCbbTarget.SetText(  rInfo.aMarkTarget );
    }

    return 0;
}

IMPL_LINK( SvxIMapDlg, MousePosHdl, IMapWindow*, pWnd )
{
    const FieldUnit eFieldUnit = GetBindings().GetDispatcher()->GetModule()->GetFieldUnit();
    const Point& rMousePos = pWnd->GetMousePos();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    OUString aStr = GetUnitString( rMousePos.X(), eFieldUnit, cSep ) +
                    " / " + GetUnitString( rMousePos.Y(), eFieldUnit, cSep );

    aStbStatus.SetItemText( 2, aStr );

    return 0L;
}

IMPL_LINK( SvxIMapDlg, GraphSizeHdl, IMapWindow*, pWnd )
{
    const FieldUnit eFieldUnit = GetBindings().GetDispatcher()->GetModule()->GetFieldUnit();
    const Size& rSize = pWnd->GetGraphicSize();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    OUString aStr = GetUnitString( rSize.Width(), eFieldUnit, cSep ) +
                    " x " + GetUnitString( rSize.Height(), eFieldUnit, cSep );

    aStbStatus.SetItemText( 3, aStr );

    return 0L;
}

IMPL_LINK_NOARG(SvxIMapDlg, URLModifyHdl)
{
    NotifyInfo  aNewInfo;

    aNewInfo.aMarkURL = maURLBox.GetText();
    aNewInfo.aMarkAltText = aEdtText.GetText();
    aNewInfo.aMarkTarget = maCbbTarget.GetText();

    pIMapWnd->ReplaceActualIMapInfo( aNewInfo );

    return 0;
}

IMPL_LINK_NOARG(SvxIMapDlg, URLLoseFocusHdl)
{
    NotifyInfo      aNewInfo;
    const String    aURLText( maURLBox.GetText() );
    const String    aTargetText( maCbbTarget.GetText() );

    if ( aURLText.Len() )
    {
        String aBase = GetBindings().GetDispatcher()->GetFrame()->GetObjectShell()->GetMedium()->GetBaseURL();
        aNewInfo.aMarkURL = ::URIHelper::SmartRel2Abs( INetURLObject(aBase), aURLText, URIHelper::GetMaybeFileHdl(), true, false,
                                                        INetURLObject::WAS_ENCODED,
                                                        INetURLObject::DECODE_UNAMBIGUOUS );
    }
    else
        aNewInfo.aMarkURL = aURLText;

    aNewInfo.aMarkAltText = aEdtText.GetText();

    if ( !aTargetText.Len() )
        aNewInfo.aMarkTarget = SELF_TARGET;
    else
        aNewInfo.aMarkTarget = aTargetText;

    pIMapWnd->ReplaceActualIMapInfo( aNewInfo );

    return 0;
}

IMPL_LINK_NOARG(SvxIMapDlg, UpdateHdl)
{
    pOwnData->aTimer.Stop();

    if ( pOwnData->pUpdateEditingObject != pCheckObj )
    {
        if ( pIMapWnd->IsChanged() &&
             ( QueryBox( this, WB_YES_NO | WB_DEF_YES,
             SVX_RESSTR( STR_IMAPDLG_SAVE ) ).Execute() == RET_YES ) )
        {
            DoSave();
        }

        SetGraphic( pOwnData->aUpdateGraphic );
        SetImageMap( pOwnData->aUpdateImageMap );
        SetTargetList( pOwnData->aUpdateTargetList );
        SetEditingObject( pOwnData->pUpdateEditingObject );

        // After changes => default selection
        aTbxIMapDlg1.CheckItem( TBI_SELECT, sal_True );
        pIMapWnd->SetEditMode( sal_True );
    }

    // Delete the copied list again in the Update method
    pOwnData->aUpdateTargetList.clear();

    GetBindings().Invalidate( SID_IMAP_EXEC );

    return 0L;
}

IMPL_LINK( SvxIMapDlg, StateHdl, IMapWindow*, pWnd )
{
    const SdrObject*    pObj = pWnd->GetSelectedSdrObject();
    const SdrModel*     pModel = pWnd->GetSdrModel();
    const SdrView*      pView = pWnd->GetSdrView();
    const sal_Bool          bPolyEdit = ( pObj != NULL ) && pObj->ISA( SdrPathObj );
    const sal_Bool          bDrawEnabled = !( bPolyEdit && aTbxIMapDlg1.IsItemChecked( TBI_POLYEDIT ) );

    aTbxIMapDlg1.EnableItem( TBI_APPLY, pOwnData->bExecState && pWnd->IsChanged() );

    aTbxIMapDlg1.EnableItem( TBI_SELECT, bDrawEnabled );
    aTbxIMapDlg1.EnableItem( TBI_RECT, bDrawEnabled );
    aTbxIMapDlg1.EnableItem( TBI_CIRCLE, bDrawEnabled );
    aTbxIMapDlg1.EnableItem( TBI_POLY, bDrawEnabled );
    aTbxIMapDlg1.EnableItem( TBI_FREEPOLY, bDrawEnabled );

    // BezierEditor-Stati
    aTbxIMapDlg1.EnableItem( TBI_POLYEDIT, bPolyEdit );
    aTbxIMapDlg1.EnableItem( TBI_POLYMOVE, !bDrawEnabled );
    aTbxIMapDlg1.EnableItem( TBI_POLYINSERT, !bDrawEnabled );
    aTbxIMapDlg1.EnableItem( TBI_POLYDELETE, !bDrawEnabled && pView->IsDeleteMarkedPointsPossible() );

    // Undo/Redo
    aTbxIMapDlg1.EnableItem( TBI_UNDO, pModel->HasUndoActions() );
    aTbxIMapDlg1.EnableItem( TBI_REDO, pModel->HasRedoActions() );

    if ( bPolyEdit )
    {
        sal_uInt16 nId = 0;

        switch( pWnd->GetPolyEditMode() )
        {
            case( SID_BEZIER_MOVE ): nId = TBI_POLYMOVE; break;
            case( SID_BEZIER_INSERT ): nId = TBI_POLYINSERT; break;

            default:
            break;
        }

        aTbxIMapDlg1.CheckItem( nId, sal_True );
    }
    else
    {
        aTbxIMapDlg1.CheckItem( TBI_POLYEDIT, sal_False );
        aTbxIMapDlg1.CheckItem( TBI_POLYMOVE, sal_True );
        aTbxIMapDlg1.CheckItem( TBI_POLYINSERT, sal_False );
        pWnd->SetPolyEditMode( 0 );
    }

    return 0L;
}

IMPL_LINK_NOARG(SvxIMapDlg, MiscHdl)
{
       SvtMiscOptions aMiscOptions;
    aTbxIMapDlg1.SetOutStyle( aMiscOptions.GetToolboxStyle() );

    return 0L;
}

void SvxIMapDlg::ApplyImageList()
{
    ImageList& rImgLst = maImageList;

    aTbxIMapDlg1.SetImageList( rImgLst );
}

void SvxIMapDlg::DataChanged( const DataChangedEvent& rDCEvt )
{
    SfxModelessDialog::DataChanged( rDCEvt );

    if ( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
            ApplyImageList();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
