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

#include <vcl/errinf.hxx>
#include <tools/urlobj.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svl/eitem.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/module.hxx>
#include <sfx2/filedlghelper.hxx>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <svl/urihelper.hxx>
#include <svtools/ehdl.hxx>
#include <svtools/inettbc.hxx>
#include <svtools/sfxecode.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/objsh.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/weld.hxx>
#include <svx/imapdlg.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/svxids.hrc>
#include "imapwnd.hxx"
#include "imapimp.hxx"
#include <svx/svdopath.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>
#include <osl/diagnose.h>
#include "dlgunit.hxx"
#include <memory>

constexpr OUString SELF_TARGET = u"_self"_ustr;
constexpr OUString IMAP_CERN_FILTER = u"MAP - CERN"_ustr;
constexpr OUString IMAP_NCSA_FILTER = u"MAP - NCSA"_ustr;
constexpr OUString IMAP_BINARY_FILTER = u"SIP - StarView ImageMap"_ustr;
constexpr OUStringLiteral IMAP_ALL_TYPE = u"*.*";
constexpr OUString IMAP_BINARY_TYPE = u"*.sip"_ustr;
constexpr OUString IMAP_CERN_TYPE = u"*.map"_ustr;
constexpr OUString IMAP_NCSA_TYPE = u"*.map"_ustr;

SFX_IMPL_MODELESSDIALOGCONTOLLER_WITHID( SvxIMapDlgChildWindow, SID_IMAP );

// ControllerItem

SvxIMapDlgItem::SvxIMapDlgItem( SvxIMapDlg& rIMapDlg, SfxBindings& rBindings ) :
            SfxControllerItem   ( SID_IMAP_EXEC, rBindings ),
            rIMap               ( rIMapDlg )
{
}

void SvxIMapDlgItem::StateChangedAtToolBoxControl( sal_uInt16 nSID, SfxItemState /*eState*/,
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

SvxIMapDlgChildWindow::SvxIMapDlgChildWindow(vcl::Window* _pParent, sal_uInt16 nId,
                                             SfxBindings* pBindings,
                                             SfxChildWinInfo const * pInfo)
    : SfxChildWindow( _pParent, nId )
{
    SetController(std::make_shared<SvxIMapDlg>(pBindings, this, _pParent->GetFrameWeld()));
    SvxIMapDlg* pDlg = static_cast<SvxIMapDlg*>(GetController().get());
    pDlg->Initialize( pInfo );
}

void SvxIMapDlgChildWindow::UpdateIMapDlg( const Graphic& rGraphic, const ImageMap* pImageMap,
                                           const TargetList* pTargetList, void* pEditingObj )
{
    SvxIMapDlg* pDlg = GetIMapDlg();
    if (pDlg)
        pDlg->UpdateLink(rGraphic, pImageMap, pTargetList, pEditingObj);
}

SvxIMapDlg::SvxIMapDlg(SfxBindings *_pBindings, SfxChildWindow *pCW, weld::Window* _pParent)
    : SfxModelessDialogController(_pBindings, pCW, _pParent, u"svx/ui/imapdialog.ui"_ustr, u"ImapDialog"_ustr)
    , pCheckObj(nullptr)
    , aIMapItem(*this, *_pBindings)
    , m_xIMapWnd(new IMapWindow(_pBindings->GetActiveFrame(), m_xDialog.get()))
    , m_xTbxIMapDlg1(m_xBuilder->weld_toolbar(u"toolbar"_ustr))
    , m_xFtURL(m_xBuilder->weld_label(u"urlft"_ustr))
    , m_xURLBox(new SvtURLBox(m_xBuilder->weld_combo_box(u"url"_ustr)))
    , m_xFtText(m_xBuilder->weld_label(u"textft"_ustr))
    , m_xEdtText(m_xBuilder->weld_entry(u"text"_ustr))
    , m_xFtTarget(m_xBuilder->weld_label(u"targetft"_ustr))
    , m_xCbbTarget(m_xBuilder->weld_combo_box(u"target"_ustr))
    , m_xCancelBtn(m_xBuilder->weld_button(u"cancel"_ustr))
    , m_xStbStatus1(m_xBuilder->weld_label(u"statusurl"_ustr))
    , m_xStbStatus2(m_xBuilder->weld_label(u"statuspos"_ustr))
    , m_xStbStatus3(m_xBuilder->weld_label(u"statussize"_ustr))
    , m_xIMapWndWeld(new weld::CustomWeld(*m_xBuilder, u"container"_ustr, *m_xIMapWnd))

{
    m_xTbxIMapDlg1->insert_separator(4, u"sep1"_ustr);
    m_xTbxIMapDlg1->insert_separator(10, u"sep2"_ustr);
    m_xTbxIMapDlg1->insert_separator(15, u"sep3"_ustr);
    m_xTbxIMapDlg1->insert_separator(18, u"sel4"_ustr);

    //lock this down so it doesn't jump around in size
    //as entries are added later on
    TargetList aTmpList;
    SfxFrame::GetDefaultTargetList(aTmpList);
    for (const OUString & s : aTmpList)
        m_xCbbTarget->append_text(s);
    Size aPrefSize(m_xCbbTarget->get_preferred_size());
    m_xCbbTarget->set_size_request(aPrefSize.Width(), -1);
    m_xCbbTarget->clear();

    m_xIMapWnd->Show();

    pOwnData.reset(new IMapOwnData);

    m_xIMapWnd->SetInfoLink( LINK( this, SvxIMapDlg, InfoHdl ) );
    m_xIMapWnd->SetMousePosLink( LINK( this, SvxIMapDlg, MousePosHdl ) );
    m_xIMapWnd->SetGraphSizeLink( LINK( this, SvxIMapDlg, GraphSizeHdl ) );
    m_xIMapWnd->SetUpdateLink( LINK( this, SvxIMapDlg, StateHdl ) );

    m_xURLBox->connect_changed( LINK( this, SvxIMapDlg, URLModifyHdl ) );
    m_xURLBox->connect_focus_out( LINK( this, SvxIMapDlg, URLLoseFocusHdl ) );
    m_xEdtText->connect_changed( LINK( this, SvxIMapDlg, EntryModifyHdl ) );
    m_xCbbTarget->connect_focus_out( LINK( this, SvxIMapDlg, URLLoseFocusHdl ) );

    m_xTbxIMapDlg1->connect_clicked( LINK( this, SvxIMapDlg, TbxClickHdl ) );
    OUString sSelect(u"TBI_SELECT"_ustr);
    m_xTbxIMapDlg1->set_item_active(sSelect, true);
    TbxClickHdl(sSelect);

    m_xStbStatus1->set_size_request(120, -1);
    const int nWidth = m_xStbStatus1->get_pixel_size(u" 9999,99 cm / 9999,99 cm "_ustr).Width();
    m_xStbStatus2->set_size_request(nWidth, -1);
    m_xStbStatus3->set_size_request(nWidth, -1);

    m_xFtURL->set_sensitive(false);
    m_xURLBox->set_sensitive(false);
    m_xFtText->set_sensitive(false);
    m_xEdtText->set_sensitive(false);
    m_xFtTarget->set_sensitive(false);
    m_xCbbTarget->set_sensitive(false);
    pOwnData->bExecState = false;

    pOwnData->aIdle.SetInvokeHandler( LINK( this, SvxIMapDlg, UpdateHdl ) );

    m_xTbxIMapDlg1->set_item_sensitive(u"TBI_ACTIVE"_ustr, false);
    m_xTbxIMapDlg1->set_item_sensitive(u"TBI_MACRO"_ustr, false );
    m_xTbxIMapDlg1->set_item_sensitive(u"TBI_PROPERTY"_ustr, false );

    m_xCancelBtn->connect_clicked(LINK(this, SvxIMapDlg, CancelHdl));
}

SvxIMapDlg::~SvxIMapDlg()
{
    m_xIMapWnd->SetUpdateLink( Link<GraphCtrl*,void>() );
    m_xIMapWnd.reset();
}

IMPL_LINK_NOARG(SvxIMapDlg, CancelHdl, weld::Button&, void)
{
    bool bRet = true;

    if ( m_xTbxIMapDlg1->get_item_sensitive(u"TBI_APPLY"_ustr) )
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xDialog.get(), u"svx/ui/querymodifyimagemapchangesdialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xQBox(xBuilder->weld_message_dialog(u"QueryModifyImageMapChangesDialog"_ustr));
        const tools::Long nRet = xQBox->run();

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
    else if( m_xIMapWnd->IsChanged() )
    {
        std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xDialog.get(), u"svx/ui/querysaveimagemapchangesdialog.ui"_ustr));
        std::unique_ptr<weld::MessageDialog> xQBox(xBuilder->weld_message_dialog(u"QuerySaveImageMapChangesDialog"_ustr));
        const tools::Long nRet = xQBox->run();

        if( nRet == RET_YES )
            bRet = DoSave();
        else if( nRet == RET_CANCEL )
            bRet = false;
    }

    if (bRet)
        m_xDialog->response(RET_CANCEL);
}

// Enabled or disable all Controls

void SvxIMapDlg::SetExecState( bool bEnable )
{
    pOwnData->bExecState = bEnable;
}

const ImageMap& SvxIMapDlg::GetImageMap() const
{
    return m_xIMapWnd->GetImageMap();
}

void SvxIMapDlg::SetTargetList( const TargetList& rTargetList )
{
    m_xIMapWnd->SetTargetList( rTargetList );

    m_xCbbTarget->clear();

    for (const OUString & s : rTargetList)
        m_xCbbTarget->append_text(s);
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

    // TargetList must be copied, since it is owned by the caller and can be
    // deleted immediately after this call the copied list will be deleted
    // again in the handler
    if( pTargetList )
        pOwnData->aUpdateTargetList = *pTargetList;
    else
        pOwnData->aUpdateTargetList.clear();

    pOwnData->aIdle.Start();
}


// Click-handler for ToolBox

IMPL_LINK(SvxIMapDlg, TbxClickHdl, const OUString&, rNewItemId, void)
{
    if (rNewItemId == "TBI_APPLY")
    {
        URLLoseFocusHdl(*m_xCbbTarget);
        SfxBoolItem aBoolItem( SID_IMAP_EXEC, true );
        GetBindings().GetDispatcher()->ExecuteList(SID_IMAP_EXEC,
            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
            { &aBoolItem });
    }
    else if (rNewItemId == "TBI_OPEN")
        DoOpen();
    else if (rNewItemId == "TBI_SAVEAS")
        DoSave();
    else if (rNewItemId == "TBI_CLOSE")
        CancelHdl(*m_xCancelBtn);
    else if (rNewItemId == "TBI_SELECT")
    {
        SetActiveTool( rNewItemId );
        m_xIMapWnd->SetEditMode( true );
    }
    else if (rNewItemId == "TBI_RECT")
    {
        SetActiveTool( rNewItemId );
        m_xIMapWnd->SetObjKind( SdrObjKind::Rectangle );
    }
    else if (rNewItemId == "TBI_CIRCLE")
    {
        SetActiveTool( rNewItemId );
        m_xIMapWnd->SetObjKind( SdrObjKind::CircleOrEllipse );
    }
    else if (rNewItemId == "TBI_POLY")
    {
        SetActiveTool( rNewItemId );
        m_xIMapWnd->SetObjKind( SdrObjKind::Polygon );
    }
    else if (rNewItemId == "TBI_FREEPOLY")
    {
        SetActiveTool( rNewItemId );
        m_xIMapWnd->SetObjKind( SdrObjKind::FreehandFill );
    }
    else if (rNewItemId == "TBI_ACTIVE")
    {
        URLLoseFocusHdl(*m_xCbbTarget);
        bool bNewState = !m_xTbxIMapDlg1->get_item_active(rNewItemId);
        m_xTbxIMapDlg1->set_item_active(rNewItemId, bNewState);
        m_xIMapWnd->SetCurrentObjState( !bNewState );
    }
    else if (rNewItemId == "TBI_MACRO")
        m_xIMapWnd->DoMacroAssign();
    else if (rNewItemId == "TBI_PROPERTY")
        m_xIMapWnd->DoPropertyDialog();
    else if (rNewItemId == "TBI_POLYEDIT")
    {
        SetActiveTool( rNewItemId );
        m_xIMapWnd->SetPolyEditMode( m_xTbxIMapDlg1->get_item_active(rNewItemId) ? SID_BEZIER_MOVE : 0 );
    }
    else if (rNewItemId == "TBI_POLYMOVE")
    {
        SetActiveTool( rNewItemId );
        m_xIMapWnd->SetPolyEditMode( SID_BEZIER_MOVE );
    }
    else if (rNewItemId == "TBI_POLYINSERT")
    {
        SetActiveTool( rNewItemId );
        m_xIMapWnd->SetPolyEditMode( SID_BEZIER_INSERT );
    }
    else if (rNewItemId == "TBI_POLYDELETE")
    {
        SetActiveTool( rNewItemId );
        m_xIMapWnd->GetSdrView()->DeleteMarkedPoints();
    }
    else if (rNewItemId == "TBI_UNDO")
    {
        URLLoseFocusHdl(*m_xCbbTarget);
        m_xIMapWnd->GetSdrModel()->Undo();
    }
    else if (rNewItemId == "TBI_REDO")
    {
        URLLoseFocusHdl(*m_xCbbTarget);
        m_xIMapWnd->GetSdrModel()->Redo();
    }
}

void SvxIMapDlg::DoOpen()
{
    ::sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
        FileDialogFlags::NONE, m_xDialog.get());

    ImageMap        aLoadIMap;
    const OUString  aFilter(SvxResId(RID_SVXSTR_IMAP_ALL_FILTER));

    aDlg.AddFilter( aFilter, IMAP_ALL_TYPE );
    aDlg.AddFilter( IMAP_CERN_FILTER, IMAP_CERN_TYPE );
    aDlg.AddFilter( IMAP_NCSA_FILTER, IMAP_NCSA_TYPE );
    aDlg.AddFilter( IMAP_BINARY_FILTER, IMAP_BINARY_TYPE );

    aDlg.SetCurrentFilter( aFilter );
    aDlg.SetContext(sfx2::FileDialogHelper::ImageMap);

    if( aDlg.Execute() != ERRCODE_NONE )
        return;

    INetURLObject aURL( aDlg.GetPath() );
    DBG_ASSERT( aURL.GetProtocol() != INetProtocol::NotValid, "invalid URL" );
    std::unique_ptr<SvStream> pIStm(::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::READ ));

    if( pIStm )
    {
        aLoadIMap.Read( *pIStm, IMapFormat::Detect );

        if( pIStm->GetError() )
        {
            SfxErrorContext eEC(ERRCTX_ERROR, m_xDialog.get());
            ErrorHandler::HandleError( ERRCODE_IO_GENERAL );
        }
        else
            m_xIMapWnd->SetImageMap( aLoadIMap );
    }

    m_xIMapWnd->Invalidate();
}

bool SvxIMapDlg::DoSave()
{
    ::sfx2::FileDialogHelper aDlg(
        css::ui::dialogs::TemplateDescription::FILESAVE_SIMPLE,
        FileDialogFlags::NONE, m_xDialog.get());

    const OUString    aBinFilter( IMAP_BINARY_FILTER );
    const OUString    aCERNFilter( IMAP_CERN_FILTER );
    const OUString    aNCSAFilter( IMAP_NCSA_FILTER );
    SdrModel*       pModel = m_xIMapWnd->GetSdrModel();
    const bool bChanged = pModel->IsChanged();
    bool            bRet = false;

    aDlg.AddFilter( aCERNFilter, IMAP_CERN_TYPE );
    aDlg.AddFilter( aNCSAFilter, IMAP_NCSA_TYPE );
    aDlg.AddFilter( aBinFilter, IMAP_BINARY_TYPE );

    aDlg.SetCurrentFilter( aCERNFilter );
    aDlg.SetContext(sfx2::FileDialogHelper::ImageMap);

    if( aDlg.Execute() == ERRCODE_NONE )
    {
        const OUString    aFilter( aDlg.GetCurrentFilter() );
        OUString          aExt;
        IMapFormat        nFormat;

        if ( aFilter == aBinFilter )
        {
            nFormat = IMapFormat::Binary;
            aExt = "sip";
        }
        else if ( aFilter == aCERNFilter )
        {
            nFormat = IMapFormat::CERN;
            aExt = "map";
        }
        else if ( aFilter == aNCSAFilter )
        {
            nFormat = IMapFormat::NCSA;
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

            std::unique_ptr<SvStream> pOStm(::utl::UcbStreamHelper::CreateStream( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ), StreamMode::WRITE | StreamMode::TRUNC ));
            if( pOStm )
            {
                m_xIMapWnd->GetImageMap().Write( *pOStm, nFormat );

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

void SvxIMapDlg::SetActiveTool(std::u16string_view rId)
{
    m_xTbxIMapDlg1->set_item_active(u"TBI_SELECT"_ustr, rId == u"TBI_SELECT");
    m_xTbxIMapDlg1->set_item_active(u"TBI_RECT"_ustr, rId == u"TBI_RECT");
    m_xTbxIMapDlg1->set_item_active(u"TBI_CIRCLE"_ustr, rId == u"TBI_CIRCLE");
    m_xTbxIMapDlg1->set_item_active(u"TBI_POLY"_ustr, rId == u"TBI_POLY");
    m_xTbxIMapDlg1->set_item_active(u"TBI_FREEPOLY"_ustr, rId == u"TBI_FREEPOLY");

    m_xTbxIMapDlg1->set_item_active(u"TBI_POLYINSERT"_ustr, rId == u"TBI_POLYINSERT");
    m_xTbxIMapDlg1->set_item_active(u"TBI_POLYDELETE"_ustr, false);

    bool bMove = rId == u"TBI_POLYMOVE"
                || ( rId == u"TBI_POLYEDIT"
                && !m_xTbxIMapDlg1->get_item_active(u"TBI_POLYINSERT"_ustr)
                && !m_xTbxIMapDlg1->get_item_active(u"TBI_POLYDELETE"_ustr) );

    m_xTbxIMapDlg1->set_item_active(u"TBI_POLYMOVE"_ustr, bMove );

    bool bEditMode = ( rId == u"TBI_POLYEDIT" )
                    || ( rId == u"TBI_POLYMOVE")
                    || ( rId == u"TBI_POLYINSERT")
                    || ( rId == u"TBI_POLYDELETE" );

    m_xTbxIMapDlg1->set_item_active(u"TBI_POLYEDIT"_ustr, bEditMode);
}

IMPL_LINK( SvxIMapDlg, InfoHdl, IMapWindow&, rWnd, void )
{
    const NotifyInfo&   rInfo = rWnd.GetInfo();

    if ( rInfo.bNewObj )
    {
        if (!rInfo.aMarkURL.isEmpty() && ( m_xURLBox->find_text(rInfo.aMarkURL) == -1))
            m_xURLBox->append_text(rInfo.aMarkURL);

        m_xURLBox->set_entry_text(rInfo.aMarkURL);
        m_xEdtText->set_text(rInfo.aMarkAltText);

        if ( rInfo.aMarkTarget.isEmpty() )
            m_xCbbTarget->set_entry_text( SELF_TARGET );
        else
            m_xCbbTarget->set_entry_text( rInfo.aMarkTarget );
    }

    if ( !rInfo.bOneMarked )
    {
        m_xTbxIMapDlg1->set_item_active(u"TBI_ACTIVE"_ustr, false);
        m_xTbxIMapDlg1->set_item_sensitive(u"TBI_ACTIVE"_ustr, false);
        m_xTbxIMapDlg1->set_item_sensitive(u"TBI_MACRO"_ustr, false);
        m_xTbxIMapDlg1->set_item_sensitive(u"TBI_PROPERTY"_ustr, false);
        m_xStbStatus1->set_label(OUString());

        m_xFtURL->set_sensitive(false);
        m_xURLBox->set_sensitive(false);
        m_xFtText->set_sensitive(false);
        m_xEdtText->set_sensitive(false);
        m_xFtTarget->set_sensitive(false);
        m_xCbbTarget->set_sensitive(false);

        m_xURLBox->set_entry_text( u""_ustr );
        m_xEdtText->set_text( u""_ustr );
    }
    else
    {
        m_xTbxIMapDlg1->set_item_sensitive(u"TBI_ACTIVE"_ustr, true);
        m_xTbxIMapDlg1->set_item_active(u"TBI_ACTIVE"_ustr, !rInfo.bActivated );
        m_xTbxIMapDlg1->set_item_sensitive(u"TBI_MACRO"_ustr, true);
        m_xTbxIMapDlg1->set_item_sensitive(u"TBI_PROPERTY"_ustr, true);

        m_xFtURL->set_sensitive(true);
        m_xURLBox->set_sensitive(true);
        m_xFtText->set_sensitive(true);
        m_xEdtText->set_sensitive(true);
        m_xFtTarget->set_sensitive(true);
        m_xCbbTarget->set_sensitive(true);

        m_xStbStatus1->set_label(rInfo.aMarkURL);

        if ( m_xURLBox->get_active_text() != rInfo.aMarkURL )
            m_xURLBox->set_entry_text( rInfo.aMarkURL );

        if ( m_xEdtText->get_text() != rInfo.aMarkAltText )
            m_xEdtText->set_text( rInfo.aMarkAltText );

        if ( rInfo.aMarkTarget.isEmpty() )
            m_xCbbTarget->set_entry_text( SELF_TARGET );
        else
            m_xCbbTarget->set_entry_text(  rInfo.aMarkTarget );
    }
}

IMPL_LINK( SvxIMapDlg, MousePosHdl, GraphCtrl*, pWnd, void )
{
    const FieldUnit eFieldUnit = GetBindings().GetDispatcher()->GetModule()->GetFieldUnit();
    const Point& rMousePos = pWnd->GetMousePos();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    OUString aStr = GetUnitString( rMousePos.X(), eFieldUnit, cSep ) +
                    " / " + GetUnitString( rMousePos.Y(), eFieldUnit, cSep );

    m_xStbStatus2->set_label(aStr);
}

IMPL_LINK( SvxIMapDlg, GraphSizeHdl, GraphCtrl*, pWnd, void )
{
    const FieldUnit eFieldUnit = GetBindings().GetDispatcher()->GetModule()->GetFieldUnit();
    const Size& rSize = pWnd->GetGraphicSize();
    const LocaleDataWrapper& rLocaleWrapper( Application::GetSettings().GetLocaleDataWrapper() );
    const sal_Unicode cSep = rLocaleWrapper.getNumDecimalSep()[0];

    OUString aStr = GetUnitString( rSize.Width(), eFieldUnit, cSep ) +
                    " x " + GetUnitString( rSize.Height(), eFieldUnit, cSep );

    m_xStbStatus3->set_label(aStr);
}

void SvxIMapDlg::URLModify()
{
    NotifyInfo  aNewInfo;

    aNewInfo.aMarkURL = m_xURLBox->get_active_text();
    aNewInfo.aMarkAltText = m_xEdtText->get_text();
    aNewInfo.aMarkTarget = m_xCbbTarget->get_active_text();

    m_xIMapWnd->ReplaceActualIMapInfo( aNewInfo );
}

IMPL_LINK_NOARG(SvxIMapDlg, URLModifyHdl, weld::ComboBox&, void)
{
    URLModify();
}

IMPL_LINK_NOARG(SvxIMapDlg, EntryModifyHdl, weld::Entry&, void)
{
    URLModify();
}

IMPL_LINK_NOARG(SvxIMapDlg, URLLoseFocusHdl, weld::Widget&, void)
{
    NotifyInfo        aNewInfo;
    const OUString    aURLText( m_xURLBox->get_active_text() );
    const OUString    aTargetText( m_xCbbTarget->get_active_text() );

    if ( !aURLText.isEmpty() )
    {
        OUString aBase = GetBindings().GetDispatcher()->GetFrame()->GetObjectShell()->GetMedium()->GetBaseURL();
        aNewInfo.aMarkURL = ::URIHelper::SmartRel2Abs( INetURLObject(aBase), aURLText, URIHelper::GetMaybeFileHdl(), true, false,
                                                        INetURLObject::EncodeMechanism::WasEncoded,
                                                        INetURLObject::DecodeMechanism::Unambiguous );
    }
    else
        aNewInfo.aMarkURL = aURLText;

    aNewInfo.aMarkAltText = m_xEdtText->get_text();

    if ( aTargetText.isEmpty() )
        aNewInfo.aMarkTarget = SELF_TARGET;
    else
        aNewInfo.aMarkTarget = aTargetText;

    m_xIMapWnd->ReplaceActualIMapInfo( aNewInfo );
}

IMPL_LINK_NOARG(SvxIMapDlg, UpdateHdl, Timer *, void)
{
    pOwnData->aIdle.Stop();

    if ( pOwnData->pUpdateEditingObject != pCheckObj )
    {
        if (m_xIMapWnd->IsChanged())
        {
            std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(m_xDialog.get(), u"svx/ui/querysaveimagemapchangesdialog.ui"_ustr));
            std::unique_ptr<weld::MessageDialog> xQBox(xBuilder->weld_message_dialog(u"QuerySaveImageMapChangesDialog"_ustr));
            if (xQBox->run() == RET_YES)
            {
                DoSave();
            }
        }

        m_xIMapWnd->SetGraphic( pOwnData->aUpdateGraphic );
        m_xIMapWnd->SetImageMap( pOwnData->aUpdateImageMap );
        SetTargetList( pOwnData->aUpdateTargetList );
        pCheckObj = pOwnData->pUpdateEditingObject;

        // After changes => default selection
        m_xTbxIMapDlg1->set_item_active(u"TBI_SELECT"_ustr, true);
        m_xIMapWnd->SetEditMode( true );
    }

    // Delete the copied list again in the Update method
    pOwnData->aUpdateTargetList.clear();

    GetBindings().Invalidate( SID_IMAP_EXEC );
    m_xIMapWnd->QueueIdleUpdate();
}

IMPL_LINK( SvxIMapDlg, StateHdl, GraphCtrl*, pWnd, void )
{
    const SdrObject*    pObj = pWnd->GetSelectedSdrObject();
    const SdrModel*     pModel = pWnd->GetSdrModel();
    const SdrView*      pView = pWnd->GetSdrView();
    const bool          bPolyEdit = ( pObj != nullptr ) && dynamic_cast<const SdrPathObj*>( pObj) !=  nullptr;
    const bool          bDrawEnabled = !( bPolyEdit && m_xTbxIMapDlg1->get_item_active(u"TBI_POLYEDIT"_ustr) );

    m_xTbxIMapDlg1->set_item_sensitive(u"TBI_APPLY"_ustr, pOwnData->bExecState && pWnd->IsChanged() );

    m_xTbxIMapDlg1->set_item_sensitive(u"TBI_SELECT"_ustr, bDrawEnabled);
    m_xTbxIMapDlg1->set_item_sensitive(u"TBI_RECT"_ustr, bDrawEnabled);
    m_xTbxIMapDlg1->set_item_sensitive(u"TBI_CIRCLE"_ustr, bDrawEnabled);
    m_xTbxIMapDlg1->set_item_sensitive(u"TBI_POLY"_ustr, bDrawEnabled);
    m_xTbxIMapDlg1->set_item_sensitive(u"TBI_FREEPOLY"_ustr, bDrawEnabled);

    // BezierEditor State
    m_xTbxIMapDlg1->set_item_sensitive( u"TBI_POLYEDIT"_ustr, bPolyEdit );
    m_xTbxIMapDlg1->set_item_sensitive( u"TBI_POLYMOVE"_ustr, !bDrawEnabled );
    m_xTbxIMapDlg1->set_item_sensitive( u"TBI_POLYINSERT"_ustr, !bDrawEnabled );
    m_xTbxIMapDlg1->set_item_sensitive( u"TBI_POLYDELETE"_ustr, !bDrawEnabled && pView->IsDeleteMarkedPointsPossible() );

    // Undo/Redo
    m_xTbxIMapDlg1->set_item_sensitive( u"TBI_UNDO"_ustr, pModel->HasUndoActions() );
    m_xTbxIMapDlg1->set_item_sensitive( u"TBI_REDO"_ustr, pModel->HasRedoActions() );

    if ( bPolyEdit )
    {
        switch( pWnd->GetPolyEditMode() )
        {
            case SID_BEZIER_MOVE:
                m_xTbxIMapDlg1->set_item_active(u"TBI_POLYMOVE"_ustr, true);
                m_xTbxIMapDlg1->set_item_active(u"TBI_POLYINSERT"_ustr, false);
                break;
            case SID_BEZIER_INSERT:
                m_xTbxIMapDlg1->set_item_active(u"TBI_POLYINSERT"_ustr, true);
                m_xTbxIMapDlg1->set_item_active(u"TBI_POLYMOVE"_ustr, false);
                break;
            default:
                break;
        }
    }
    else
    {
        m_xTbxIMapDlg1->set_item_active( u"TBI_POLYEDIT"_ustr, false );
        m_xTbxIMapDlg1->set_item_active( u"TBI_POLYMOVE"_ustr, true);
        m_xTbxIMapDlg1->set_item_active( u"TBI_POLYINSERT"_ustr, false );
        pWnd->SetPolyEditMode( 0 );
    }

    m_xIMapWnd->QueueIdleUpdate();
}

SvxIMapDlg* GetIMapDlg()
{
    SfxChildWindow* pWnd = nullptr;
    SfxViewFrame* pViewFrm = SfxViewFrame::Current();
    if (pViewFrm && pViewFrm->HasChildWindow(SvxIMapDlgChildWindow::GetChildWindowId()))
        pWnd = pViewFrm->GetChildWindow(SvxIMapDlgChildWindow::GetChildWindowId());
    return pWnd ? static_cast<SvxIMapDlg*>(pWnd->GetController().get()) : nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
