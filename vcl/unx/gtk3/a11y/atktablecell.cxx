/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "atkwrapper.hxx"

#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <com/sun/star/accessibility/XAccessibleTableSelection.hpp>
#include <sal/log.hxx>

static css::uno::Reference<css::accessibility::XAccessibleContext>
getContext(AtkTableCell* pTableCell)
{
    AtkObjectWrapper* pWrap = ATK_OBJECT_WRAPPER(pTableCell);
    if (pWrap)
    {
        return pWrap->mpContext;
    }

    return css::uno::Reference<css::accessibility::XAccessibleContext>();
}

static css::uno::Reference<css::accessibility::XAccessibleTable>
getTableParent(AtkTableCell* pTableCell)
{
    AtkObject* pParent = atk_object_get_parent(ATK_OBJECT(pTableCell));
    if (!pParent)
        return css::uno::Reference<css::accessibility::XAccessibleTable>();

    AtkObjectWrapper* pWrap = ATK_OBJECT_WRAPPER(pParent);
    if (pWrap)
    {
        if (!pWrap->mpTable.is())
        {
            pWrap->mpTable.set(pWrap->mpContext, css::uno::UNO_QUERY);
        }

        return pWrap->mpTable;
    }

    return css::uno::Reference<css::accessibility::XAccessibleTable>();
}

extern "C" {

static int tablecell_wrapper_get_column_span(AtkTableCell* cell)
{
    int nColumnExtent = -1;
    try
    {
        css::uno::Reference<css::accessibility::XAccessibleContext> xContext = getContext(cell);
        if (!xContext.is())
            return -1;

        css::uno::Reference<css::accessibility::XAccessibleTable> xTable = getTableParent(cell);
        if (xTable.is())
        {
            const sal_Int64 nChildIndex = xContext->getAccessibleIndexInParent();
            const sal_Int32 nRow = xTable->getAccessibleRow(nChildIndex);
            const sal_Int32 nColumn = xTable->getAccessibleColumn(nChildIndex);
            nColumnExtent = xTable->getAccessibleColumnExtentAt(nRow, nColumn);
        }
    }
    catch (const css::uno::Exception&)
    {
        g_warning("Exception in tablecell_wrapper_get_column_span");
    }

    return nColumnExtent;
}

static GPtrArray* tablecell_wrapper_get_column_header_cells(AtkTableCell* cell)
{
    GPtrArray* pHeaderCells = g_ptr_array_new();
    try
    {
        css::uno::Reference<css::accessibility::XAccessibleContext> xContext = getContext(cell);
        if (!xContext.is())
            return pHeaderCells;

        css::uno::Reference<css::accessibility::XAccessibleTable> xTable = getTableParent(cell);
        if (xTable.is())
        {
            const sal_Int64 nChildIndex = xContext->getAccessibleIndexInParent();
            const sal_Int32 nCol = xTable->getAccessibleColumn(nChildIndex);
            css::uno::Reference<css::accessibility::XAccessibleTable> xHeaders
                = xTable->getAccessibleColumnHeaders();
            if (!xHeaders.is())
                return pHeaderCells;

            for (sal_Int32 nRow = 0; nRow < xHeaders->getAccessibleRowCount(); nRow++)
            {
                css::uno::Reference<css::accessibility::XAccessible> xCell
                    = xHeaders->getAccessibleCellAt(nRow, nCol);
                AtkObject* pCell = atk_object_wrapper_ref(xCell);
                g_ptr_array_add(pHeaderCells, pCell);
            }
        }
    }
    catch (const css::uno::Exception&)
    {
        g_warning("Exception in tablecell_wrapper_get_column_header_cells");
    }

    return pHeaderCells;
}

static gboolean tablecell_wrapper_get_position(AtkTableCell* cell, gint* row, gint* column)
{
    try
    {
        css::uno::Reference<css::accessibility::XAccessibleContext> xContext = getContext(cell);
        if (!xContext.is())
            return false;

        css::uno::Reference<css::accessibility::XAccessibleTable> xTable = getTableParent(cell);
        if (xTable.is())
        {
            const sal_Int64 nChildIndex = xContext->getAccessibleIndexInParent();
            *row = xTable->getAccessibleRow(nChildIndex);
            *column = xTable->getAccessibleColumn(nChildIndex);
            return true;
        }
    }
    catch (const css::uno::Exception&)
    {
        g_warning("Exception in tablecell_wrapper_get_position()");
    }

    return false;
}

static gint tablecell_wrapper_get_row_span(AtkTableCell* cell)
{
    int nRowExtent = -1;
    try
    {
        css::uno::Reference<css::accessibility::XAccessibleContext> xContext = getContext(cell);
        if (!xContext.is())
            return -1;

        css::uno::Reference<css::accessibility::XAccessibleTable> xTable = getTableParent(cell);
        if (xTable.is())
        {
            const sal_Int64 nChildIndex = xContext->getAccessibleIndexInParent();
            const sal_Int32 nRow = xTable->getAccessibleRow(nChildIndex);
            const sal_Int32 nColumn = xTable->getAccessibleColumn(nChildIndex);
            nRowExtent = xTable->getAccessibleRowExtentAt(nRow, nColumn);
        }
    }
    catch (const css::uno::Exception&)
    {
        g_warning("Exception in tablecell_wrapper_get_row_span");
    }

    return nRowExtent;
}

static GPtrArray* tablecell_wrapper_get_row_header_cells(AtkTableCell* cell)
{
    GPtrArray* pHeaderCells = g_ptr_array_new();
    try
    {
        css::uno::Reference<css::accessibility::XAccessibleContext> xContext = getContext(cell);
        if (!xContext.is())
            return pHeaderCells;

        css::uno::Reference<css::accessibility::XAccessibleTable> xTable = getTableParent(cell);
        if (xTable.is())
        {
            const sal_Int64 nChildIndex = xContext->getAccessibleIndexInParent();
            const sal_Int32 nRow = xTable->getAccessibleRow(nChildIndex);
            css::uno::Reference<css::accessibility::XAccessibleTable> xHeaders
                = xTable->getAccessibleRowHeaders();
            if (!xHeaders.is())
                return pHeaderCells;

            for (sal_Int32 nCol = 0; nCol < xHeaders->getAccessibleColumnCount(); nCol++)
            {
                css::uno::Reference<css::accessibility::XAccessible> xCell
                    = xHeaders->getAccessibleCellAt(nRow, nCol);
                AtkObject* pCell = atk_object_wrapper_ref(xCell);
                g_ptr_array_add(pHeaderCells, pCell);
            }
        }
    }
    catch (const css::uno::Exception&)
    {
        g_warning("Exception in tablecell_wrapper_get_row_header_cells");
    }

    return pHeaderCells;
}

static gboolean tablecell_wrapper_get_row_column_span(AtkTableCell* cell, gint* row, gint* column,
                                                      gint* row_span, gint* column_span)
{
    try
    {
        css::uno::Reference<css::accessibility::XAccessibleContext> xContext = getContext(cell);
        if (!xContext.is())
            return -1;

        css::uno::Reference<css::accessibility::XAccessibleTable> xTable = getTableParent(cell);
        if (xTable.is())
        {
            const sal_Int64 nChildIndex = xContext->getAccessibleIndexInParent();
            const sal_Int32 nRow = xTable->getAccessibleRow(nChildIndex);
            const sal_Int32 nColumn = xTable->getAccessibleColumn(nChildIndex);
            *row = nRow;
            *column = nColumn;
            *row_span = xTable->getAccessibleRowExtentAt(nRow, nColumn);
            *column_span = xTable->getAccessibleColumnExtentAt(nRow, nColumn);
            return true;
        }
    }
    catch (const css::uno::Exception&)
    {
        g_warning("Exception in tablecell_wrapper_get_row_column_span");
    }

    return false;
}

static AtkObject* tablecell_wrapper_get_table(AtkTableCell* cell)
{
    try
    {
        css::uno::Reference<css::accessibility::XAccessibleContext> xContext = getContext(cell);
        if (!xContext.is())
            return nullptr;

        css::uno::Reference<css::accessibility::XAccessible> xParent
            = getContext(cell)->getAccessibleParent();
        if (!xParent.is())
            return nullptr;

        return atk_object_wrapper_ref(xParent);
    }

    catch (const css::uno::Exception&)
    {
        g_warning("Exception in tablecell_wrapper_get_table()");
    }

    return nullptr;
}

} // extern "C"

void tablecellIfaceInit(gpointer iface_, gpointer)
{
    auto const iface = static_cast<AtkTableCellIface*>(iface_);
    g_return_if_fail(iface != nullptr);

    iface->get_column_span = tablecell_wrapper_get_column_span;
    iface->get_column_header_cells = tablecell_wrapper_get_column_header_cells;
    iface->get_position = tablecell_wrapper_get_position;
    iface->get_row_span = tablecell_wrapper_get_row_span;
    iface->get_row_header_cells = tablecell_wrapper_get_row_header_cells;
    iface->get_row_column_span = tablecell_wrapper_get_row_column_span;
    iface->get_table = tablecell_wrapper_get_table;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
