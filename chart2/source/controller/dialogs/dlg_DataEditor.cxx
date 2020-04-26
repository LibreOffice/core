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

#include <dlg_DataEditor.hxx>
#include "DataBrowser.hxx"
#include <comphelper/stl_types.hxx>

#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/chart2/XChartDocument.hpp>
#include <com/sun/star/frame/XStorable.hpp>

using namespace ::com::sun::star;
using ::com::sun::star::uno::Reference;

namespace chart
{

DataEditor::DataEditor(weld::Window* pParent,
    const Reference< chart2::XChartDocument > & xChartDoc,
    const Reference< uno::XComponentContext > & xContext)
    : GenericDialogController(pParent, "modules/schart/ui/chartdatadialog.ui", "ChartDataDialog")
    , m_bReadOnly(false)
    , m_xChartDoc(xChartDoc)
    , m_xContext(xContext)
    , m_xTbxData(m_xBuilder->weld_toolbar("toolbar"))
    , m_xCloseBtn(m_xBuilder->weld_button("close"))
    , m_xTable(m_xBuilder->weld_container("datawindow"))
    , m_xColumns(m_xBuilder->weld_container("columns"))
    , m_xColors(m_xBuilder->weld_container("colorcolumns"))
    , m_xTableCtrlParent(m_xTable->CreateChildFrame())
    , m_xBrwData(VclPtr<DataBrowser>::Create(m_xTableCtrlParent, m_xColumns.get(), m_xColors.get()))
{
    m_xCloseBtn->connect_clicked(LINK(this, DataEditor, CloseHdl));

    Size aSize(m_xTable->get_approximate_digit_width() * 75, m_xTable->get_text_height() * 15);
    m_xTable->set_size_request(aSize.Width(), aSize.Height());

    m_xBrwData->Show();

    m_xTbxData->connect_clicked(LINK(this, DataEditor, ToolboxHdl));

    m_xBrwData->SetCursorMovedHdl( LINK( this, DataEditor, BrowserCursorMovedHdl ));

    m_xBrwData->SetDataFromModel( m_xChartDoc, m_xContext );
    m_xBrwData->GrabFocus();

    bool bReadOnly = true;
    Reference< frame::XStorable > xStor( m_xChartDoc, uno::UNO_QUERY );
    if( xStor.is())
        bReadOnly = xStor->isReadonly();
    SetReadOnly( bReadOnly );
}

DataEditor::~DataEditor()
{
    m_xBrwData.disposeAndClear();
    m_xTableCtrlParent->dispose();
    m_xTableCtrlParent.clear();
}

// react on click (or keypress) on toolbar icon
IMPL_LINK(DataEditor, ToolboxHdl, const OString&, rId, void)
{
    if (rId == "InsertRow")
        m_xBrwData->InsertRow();
    else if (rId == "InsertColumn")
        m_xBrwData->InsertColumn();
    else if (rId == "InsertTextColumn")
        m_xBrwData->InsertTextColumn();
    else if (rId == "RemoveRow")
        m_xBrwData->RemoveRow();
    else if (rId == "RemoveColumn")
        m_xBrwData->RemoveColumn();
    else if (rId == "MoveLeftColumn")
        m_xBrwData->MoveLeftColumn();
    else if (rId == "MoveRightColumn")
        m_xBrwData->MoveRightColumn();
    else if (rId == "MoveUpRow")
        m_xBrwData->MoveUpRow();
    else if (rId == "MoveDownRow")
        m_xBrwData->MoveDownRow();
}

// refresh toolbar icons according to currently selected cell in browse box
IMPL_LINK_NOARG(DataEditor, BrowserCursorMovedHdl, DataBrowser*, void)
{
    if( m_bReadOnly )
        return;

    bool bIsDataValid = m_xBrwData->IsEnableItem();

    m_xTbxData->set_item_sensitive("InsertRow", bIsDataValid && m_xBrwData->MayInsertRow() );
    m_xTbxData->set_item_sensitive("InsertColumn", bIsDataValid && m_xBrwData->MayInsertColumn() );
    m_xTbxData->set_item_sensitive("InsertTextColumn", bIsDataValid && m_xBrwData->MayInsertColumn() );
    m_xTbxData->set_item_sensitive("RemoveRow", m_xBrwData->MayDeleteRow() );
    m_xTbxData->set_item_sensitive("RemoveColumn", m_xBrwData->MayDeleteColumn() );

    m_xTbxData->set_item_sensitive("MoveLeftColumn", bIsDataValid && m_xBrwData->MayMoveLeftColumns() );
    m_xTbxData->set_item_sensitive("MoveRightColumn", bIsDataValid && m_xBrwData->MayMoveRightColumns() );
    m_xTbxData->set_item_sensitive("MoveUpRow", bIsDataValid && m_xBrwData->MayMoveDownRows() );
    m_xTbxData->set_item_sensitive("MoveDownRow", bIsDataValid && m_xBrwData->MayMoveUpRows() );
}

// disable all modifying controls
void DataEditor::SetReadOnly( bool bReadOnly )
{
    m_bReadOnly = bReadOnly;
    if( m_bReadOnly )
    {
        m_xTbxData->set_item_sensitive("InsertRow", false);
        m_xTbxData->set_item_sensitive("InsertColumn", false);
        m_xTbxData->set_item_sensitive("InsertTextColumn", false);
        m_xTbxData->set_item_sensitive("RemoveRow", false);
        m_xTbxData->set_item_sensitive("RemoveColumn", false);
        m_xTbxData->set_item_sensitive("MoveLeftColumn", false);
        m_xTbxData->set_item_sensitive("MoveRightColumn", false);
        m_xTbxData->set_item_sensitive("MoveUpRow", false);
        m_xTbxData->set_item_sensitive("MoveDownRow", false);
    }

    m_xBrwData->SetReadOnly( m_bReadOnly );
}

IMPL_LINK_NOARG(DataEditor, CloseHdl, weld::Button&, void)
{
    bool bApplied = m_xBrwData->EndEditing(); // apply changes to model
    if (bApplied)
        m_xDialog->response(RET_CLOSE);
}

} // namespace chart

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
