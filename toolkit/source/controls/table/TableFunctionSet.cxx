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
#include <controls/table/TableControl.hxx>

namespace svt::table
{
TableFunctionSet::TableFunctionSet(TableControl& rTableControl)
    : m_rTableControl(rTableControl)
    , m_nCurrentRow(ROW_INVALID)
{
}

TableFunctionSet::~TableFunctionSet() {}

void TableFunctionSet::BeginDrag() {}

void TableFunctionSet::CreateAnchor() { m_rTableControl.setAnchor(m_rTableControl.getCurRow()); }

void TableFunctionSet::DestroyAnchor() { m_rTableControl.setAnchor(ROW_INVALID); }

void TableFunctionSet::SetCursorAtPoint(const Point& rPoint, bool bDontSelectAtCursor)
{
    // newRow is the row which includes the point, getCurRow() is the last selected row, before the mouse click
    sal_Int32 newRow = m_rTableControl.getRowAtPoint(rPoint);
    if (newRow == ROW_COL_HEADERS)
        newRow = m_rTableControl.getTopRow();

    sal_Int32 newCol = m_rTableControl.getColAtPoint(rPoint);
    if (newCol == COL_ROW_HEADERS)
        newCol = m_rTableControl.getLeftColumn();

    if ((newRow == ROW_INVALID) || (newCol == COL_INVALID))
        return;

    if (bDontSelectAtCursor)
    {
        if (m_rTableControl.GetSelectedRowCount() > 1)
            m_rTableControl.getSelEngine()->AddAlways(true);
    }
    else if (m_rTableControl.getAnchor() == m_rTableControl.getCurRow())
    {
        //selected region lies above the last selection
        if (m_rTableControl.getCurRow() >= newRow)
        {
            //put selected rows in vector
            while (m_rTableControl.getAnchor() >= newRow)
            {
                m_rTableControl.markRowAsSelected(m_rTableControl.getAnchor());
                m_rTableControl.setAnchor(m_rTableControl.getAnchor() - 1);
            }
            m_rTableControl.setAnchor(m_rTableControl.getAnchor() + 1);
        }
        //selected region lies beneath the last selected row
        else
        {
            while (m_rTableControl.getAnchor() <= newRow)
            {
                m_rTableControl.markRowAsSelected(m_rTableControl.getAnchor());
                m_rTableControl.setAnchor(m_rTableControl.getAnchor() + 1);
            }
            m_rTableControl.setAnchor(m_rTableControl.getAnchor() - 1);
        }
        m_rTableControl.invalidateSelectedRegion(m_rTableControl.getCurRow(), newRow);
    }
    //no region selected
    else
    {
        if (!m_rTableControl.hasRowSelection())
            m_rTableControl.markRowAsSelected(newRow);
        else
        {
            if (m_rTableControl.getSelEngine()->GetSelectionMode() == SelectionMode::Single)
            {
                DeselectAll();
                m_rTableControl.markRowAsSelected(newRow);
            }
            else
            {
                m_rTableControl.markRowAsSelected(newRow);
            }
        }
        if (m_rTableControl.GetSelectedRowCount() > 1
            && m_rTableControl.getSelEngine()->GetSelectionMode() != SelectionMode::Single)
            m_rTableControl.getSelEngine()->AddAlways(true);

        m_rTableControl.invalidateRow(newRow);
    }
    m_rTableControl.GoToCell(newCol, newRow);
}

bool TableFunctionSet::IsSelectionAtPoint(const Point& rPoint)
{
    m_rTableControl.getSelEngine()->AddAlways(false);
    if (!m_rTableControl.hasRowSelection())
        return false;
    else
    {
        sal_Int32 curRow = m_rTableControl.getRowAtPoint(rPoint);
        m_rTableControl.setAnchor(ROW_INVALID);
        bool selected = m_rTableControl.IsRowSelected(curRow);
        m_nCurrentRow = curRow;
        return selected;
    }
}

void TableFunctionSet::DeselectAtPoint(const Point&)
{
    m_rTableControl.invalidateRow(m_nCurrentRow);
    m_rTableControl.markRowAsDeselected(m_nCurrentRow);
}

void TableFunctionSet::DeselectAll()
{
    if (m_rTableControl.hasRowSelection())
    {
        for (size_t i = 0; i < m_rTableControl.GetSelectedRowCount(); ++i)
        {
            sal_Int32 const rowIndex = m_rTableControl.GetSelectedRowIndex(i);
            m_rTableControl.invalidateRow(rowIndex);
        }

        m_rTableControl.markAllRowsAsDeselected();
    }
}

} // namespace svt::table

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
