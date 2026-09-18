/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <controls/table/TableFunctionSet.hxx>
#include <controls/table/tablecontrol_impl.hxx>

namespace svt::table
{
TableFunctionSet::TableFunctionSet(TableControl_Impl* _pTableControl)
    : m_pTableControl(_pTableControl)
    , m_nCurrentRow(ROW_INVALID)
{
}

TableFunctionSet::~TableFunctionSet() {}

void TableFunctionSet::BeginDrag() {}

void TableFunctionSet::CreateAnchor() { m_pTableControl->setAnchor(m_pTableControl->getCurRow()); }

void TableFunctionSet::DestroyAnchor() { m_pTableControl->setAnchor(ROW_INVALID); }

void TableFunctionSet::SetCursorAtPoint(const Point& rPoint, bool bDontSelectAtCursor)
{
    // newRow is the row which includes the point, getCurRow() is the last selected row, before the mouse click
    sal_Int32 newRow = m_pTableControl->getRowAtPoint(rPoint);
    if (newRow == ROW_COL_HEADERS)
        newRow = m_pTableControl->getTopRow();

    sal_Int32 newCol = m_pTableControl->getColAtPoint(rPoint);
    if (newCol == COL_ROW_HEADERS)
        newCol = m_pTableControl->getLeftColumn();

    if ((newRow == ROW_INVALID) || (newCol == COL_INVALID))
        return;

    if (bDontSelectAtCursor)
    {
        if (m_pTableControl->GetSelectedRowCount() > 1)
            m_pTableControl->getSelEngine()->AddAlways(true);
    }
    else if (m_pTableControl->getAnchor() == m_pTableControl->getCurRow())
    {
        //selected region lies above the last selection
        if (m_pTableControl->getCurRow() >= newRow)
        {
            //put selected rows in vector
            while (m_pTableControl->getAnchor() >= newRow)
            {
                m_pTableControl->markRowAsSelected(m_pTableControl->getAnchor());
                m_pTableControl->setAnchor(m_pTableControl->getAnchor() - 1);
            }
            m_pTableControl->setAnchor(m_pTableControl->getAnchor() + 1);
        }
        //selected region lies beneath the last selected row
        else
        {
            while (m_pTableControl->getAnchor() <= newRow)
            {
                m_pTableControl->markRowAsSelected(m_pTableControl->getAnchor());
                m_pTableControl->setAnchor(m_pTableControl->getAnchor() + 1);
            }
            m_pTableControl->setAnchor(m_pTableControl->getAnchor() - 1);
        }
        m_pTableControl->invalidateSelectedRegion(m_pTableControl->getCurRow(), newRow);
    }
    //no region selected
    else
    {
        if (!m_pTableControl->hasRowSelection())
            m_pTableControl->markRowAsSelected(newRow);
        else
        {
            if (m_pTableControl->getSelEngine()->GetSelectionMode() == SelectionMode::Single)
            {
                DeselectAll();
                m_pTableControl->markRowAsSelected(newRow);
            }
            else
            {
                m_pTableControl->markRowAsSelected(newRow);
            }
        }
        if (m_pTableControl->GetSelectedRowCount() > 1
            && m_pTableControl->getSelEngine()->GetSelectionMode() != SelectionMode::Single)
            m_pTableControl->getSelEngine()->AddAlways(true);

        m_pTableControl->invalidateRow(newRow);
    }
    m_pTableControl->GoToCell(newCol, newRow);
}

bool TableFunctionSet::IsSelectionAtPoint(const Point& rPoint)
{
    m_pTableControl->getSelEngine()->AddAlways(false);
    if (!m_pTableControl->hasRowSelection())
        return false;
    else
    {
        sal_Int32 curRow = m_pTableControl->getRowAtPoint(rPoint);
        m_pTableControl->setAnchor(ROW_INVALID);
        bool selected = m_pTableControl->IsRowSelected(curRow);
        m_nCurrentRow = curRow;
        return selected;
    }
}

void TableFunctionSet::DeselectAtPoint(const Point&)
{
    m_pTableControl->invalidateRow(m_nCurrentRow);
    m_pTableControl->markRowAsDeselected(m_nCurrentRow);
}

void TableFunctionSet::DeselectAll()
{
    if (m_pTableControl->hasRowSelection())
    {
        for (size_t i = 0; i < m_pTableControl->GetSelectedRowCount(); ++i)
        {
            sal_Int32 const rowIndex = m_pTableControl->GetSelectedRowIndex(i);
            m_pTableControl->invalidateRow(rowIndex);
        }

        m_pTableControl->markAllRowsAsDeselected();
    }
}

} // namespace svt::table

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
