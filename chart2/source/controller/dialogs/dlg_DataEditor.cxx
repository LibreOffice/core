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

#include "dlg_DataEditor.hxx"
#include "Strings.hrc"
#include "DataBrowser.hxx"

#include "ResId.hxx"
#include <sfx2/dispatch.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/taskpanelist.hxx>
#include <svtools/miscopt.hxx>
#include <unotools/pathoptions.hxx>
#include <svl/eitem.hxx>
#include <vcl/edit.hxx>

#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

namespace chart
{

DataEditor::DataEditor(vcl::Window* pParent,
    const Reference< chart2::XChartDocument > & xChartDoc,
    const Reference< uno::XComponentContext > & xContext)
    : ModalDialog(pParent, "ChartDataDialog",
        "modules/schart/ui/chartdatadialog.ui")
    , m_bReadOnly(false)
    , m_xChartDoc(xChartDoc)
    , m_xContext(xContext)
{
    m_xBrwData.reset(VclPtr<DataBrowser>::Create(get<vcl::Window>("datawindow"), WB_BORDER | WB_TABSTOP, true /* bLiveUpdate */));
    m_xBrwData->set_hexpand(true);
    m_xBrwData->set_vexpand(true);
    m_xBrwData->set_expand(true);
    Size aSize(m_xBrwData->LogicToPixel(Size(232, 121), MapUnit::MapAppFont));
    m_xBrwData->set_width_request(aSize.Width());
    m_xBrwData->set_height_request(aSize.Height());
    m_xBrwData->Show();

    get(m_pTbxData, "toolbar");

    TBI_DATA_INSERT_ROW = m_pTbxData->GetItemId("InsertRow");
    TBI_DATA_INSERT_COL = m_pTbxData->GetItemId("InsertColumn");
    TBI_DATA_INSERT_TEXT_COL = m_pTbxData->GetItemId("InsertTextColumn");
    TBI_DATA_DELETE_ROW = m_pTbxData->GetItemId("RemoveRow");
    TBI_DATA_DELETE_COL = m_pTbxData->GetItemId("RemoveColumn");
    TBI_DATA_MOVE_LEFT_COL = m_pTbxData->GetItemId("MoveLeftColumn");
    TBI_DATA_MOVE_RIGHT_COL = m_pTbxData->GetItemId("MoveRightColumn");
    TBI_DATA_MOVE_UP_ROW = m_pTbxData->GetItemId("MoveUpRow");
    TBI_DATA_MOVE_DOWN_ROW = m_pTbxData->GetItemId("MoveDownRow");

    m_pTbxData->SetSelectHdl( LINK( this, DataEditor, ToolboxHdl ));

    m_xBrwData->SetCursorMovedHdl( LINK( this, DataEditor, BrowserCursorMovedHdl ));

    m_xBrwData->SetDataFromModel( m_xChartDoc, m_xContext );
    GrabFocus();
    m_xBrwData->GrabFocus();

    bool bReadOnly = true;
    Reference< frame::XStorable > xStor( m_xChartDoc, uno::UNO_QUERY );
    if( xStor.is())
        bReadOnly = xStor->isReadonly();
    SetReadOnly( bReadOnly );

    // change buttons to flat-look if set so by user
    SvtMiscOptions aMiscOptions;
    const sal_Int16 nStyle( aMiscOptions.GetToolboxStyle() );
    // react on changes
    aMiscOptions.AddListenerLink( LINK( this, DataEditor, MiscHdl ) );
    m_pTbxData->SetOutStyle( nStyle );

    // allow travelling to toolbar with F6
    notifySystemWindow( this, m_pTbxData, ::comphelper::mem_fun( & TaskPaneList::AddWindow ));
}

DataEditor::~DataEditor()
{
    disposeOnce();
}

void DataEditor::dispose()
{
    notifySystemWindow( this, m_pTbxData, ::comphelper::mem_fun( & TaskPaneList::RemoveWindow ));

    SvtMiscOptions aMiscOptions;
    aMiscOptions.RemoveListenerLink( LINK( this, DataEditor, MiscHdl ) );

    m_pTbxData.clear();
    m_xBrwData.disposeAndClear();
    ModalDialog::dispose();
}

// react on click (or keypress) on toolbar icon
IMPL_LINK_NOARG(DataEditor, ToolboxHdl, ToolBox *, void)
{
    sal_uInt16 nId = m_pTbxData->GetCurItemId();

    if (nId == TBI_DATA_INSERT_ROW)
        m_xBrwData->InsertRow();
    else if (nId == TBI_DATA_INSERT_COL)
        m_xBrwData->InsertColumn();
    else if (nId == TBI_DATA_INSERT_TEXT_COL)
        m_xBrwData->InsertTextColumn();
    else if (nId == TBI_DATA_DELETE_ROW)
        m_xBrwData->RemoveRow();
    else if (nId == TBI_DATA_DELETE_COL)
        m_xBrwData->RemoveColumn();
    else if (nId == TBI_DATA_MOVE_LEFT_COL)
        m_xBrwData->MoveLeftColumn();
    else if (nId == TBI_DATA_MOVE_RIGHT_COL)
        m_xBrwData->MoveRightColumn();
    else if (nId == TBI_DATA_MOVE_UP_ROW)
        m_xBrwData->MoveUpRow();
    else if (nId == TBI_DATA_MOVE_DOWN_ROW)
        m_xBrwData->MoveDownRow();
}

// refresh toolbar icons according to currently selected cell in browse box
IMPL_LINK_NOARG(DataEditor, BrowserCursorMovedHdl, DataBrowser*, void)
{
    if( m_bReadOnly )
        return;

    bool bIsDataValid = m_xBrwData->IsEnableItem();

    m_pTbxData->EnableItem( TBI_DATA_INSERT_ROW, bIsDataValid && m_xBrwData->MayInsertRow() );
    m_pTbxData->EnableItem( TBI_DATA_INSERT_COL, bIsDataValid && m_xBrwData->MayInsertColumn() );
    m_pTbxData->EnableItem( TBI_DATA_INSERT_TEXT_COL, bIsDataValid && m_xBrwData->MayInsertColumn() );
    m_pTbxData->EnableItem( TBI_DATA_DELETE_ROW, m_xBrwData->MayDeleteRow() );
    m_pTbxData->EnableItem( TBI_DATA_DELETE_COL, m_xBrwData->MayDeleteColumn() );

    m_pTbxData->EnableItem( TBI_DATA_MOVE_LEFT_COL,   bIsDataValid && m_xBrwData->MayMoveLeftColumns() );
    m_pTbxData->EnableItem( TBI_DATA_MOVE_RIGHT_COL,   bIsDataValid && m_xBrwData->MayMoveRightColumns() );
    m_pTbxData->EnableItem( TBI_DATA_MOVE_DOWN_ROW,   bIsDataValid && m_xBrwData->MayMoveDownRows() );
    m_pTbxData->EnableItem( TBI_DATA_MOVE_UP_ROW,   bIsDataValid && m_xBrwData->MayMoveUpRows() );
}

// disable all modifying controls
void DataEditor::SetReadOnly( bool bReadOnly )
{
    m_bReadOnly = bReadOnly;
    if( m_bReadOnly )
    {
        m_pTbxData->EnableItem( TBI_DATA_INSERT_ROW, false );
        m_pTbxData->EnableItem( TBI_DATA_INSERT_COL, false );
        m_pTbxData->EnableItem( TBI_DATA_INSERT_TEXT_COL, false );
        m_pTbxData->EnableItem( TBI_DATA_DELETE_ROW, false );
        m_pTbxData->EnableItem( TBI_DATA_DELETE_COL, false );
        m_pTbxData->EnableItem( TBI_DATA_MOVE_LEFT_COL, false );
        m_pTbxData->EnableItem( TBI_DATA_MOVE_RIGHT_COL, false );
        m_pTbxData->EnableItem( TBI_DATA_MOVE_UP_ROW, false );
        m_pTbxData->EnableItem( TBI_DATA_MOVE_DOWN_ROW, false );
    }

    m_xBrwData->SetReadOnly( m_bReadOnly );
}

IMPL_LINK_NOARG(DataEditor, MiscHdl, LinkParamNone*, void)
{
    SvtMiscOptions aMiscOptions;
    sal_Int16 nStyle( aMiscOptions.GetToolboxStyle() );

    m_pTbxData->SetOutStyle( nStyle );
}

bool DataEditor::Close()
{
    if( ApplyChangesToModel() )
        return ModalDialog::Close();
    else
        return true;
}

bool DataEditor::ApplyChangesToModel()
{
    return m_xBrwData->EndEditing();
}

// add/remove a window (the toolbar) to/from the global list, so that F6
// travels/no longer travels over this window.  _rMemFunc may be
// TaskPaneList::AddWindow or TaskPaneList::RemoveWindow
void DataEditor::notifySystemWindow(
    vcl::Window* pWindow, vcl::Window* pToRegister,
    const ::comphelper::mem_fun1_t<TaskPaneList, vcl::Window*>& rMemFunc )
{
    OSL_ENSURE( pWindow, "Window must not be null!" );
    if( !pWindow )
        return;
    vcl::Window* pParent = pWindow->GetParent();
    while( pParent && ! pParent->IsSystemWindow() )
    {
        pParent = pParent->GetParent();
    }
    if ( pParent && pParent->IsSystemWindow())
    {
        SystemWindow* pSystemWindow = static_cast< SystemWindow* >( pParent );
        rMemFunc( pSystemWindow->GetTaskPaneList(), pToRegister );
    }
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
