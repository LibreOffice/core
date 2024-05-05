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

/**
 * AccTable.cpp : Implementation of CAccTable.
 */
#include "stdafx.h"
#include  <UAccCOM.h>
#include "AccTable.h"

#include <sal/log.hxx>
#include <vcl/svapp.hxx>
#include <o3tl/char16_t2wchar_t.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include "MAccessible.h"

#include <com/sun/star/accessibility/XAccessibleTableSelection.hpp>

using namespace com::sun::star::accessibility;
using namespace com::sun::star::uno;
/**
  * Gets accessible table cell.
  *
  * @param    row        the row of the specified cell.
  * @param    column     the column of the specified cell.
  * @param    accessible the accessible object of the cell.
  */

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_accessibleAt(long row, long column, IUnknown * * accessible)
{
    SolarMutexGuard g;

    try {

    if(accessible == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessible> pRAcc = pRXTable->getAccessibleCellAt(row, column);

    if(!pRAcc.is())
    {
        *accessible = nullptr;
        return E_FAIL;
    }

    IAccessible* pRet = CMAccessible::get_IAccessibleFromXAccessible(pRAcc.get());
    if (pRet)
    {
        *accessible = pRet;
        pRet->AddRef();
        return S_OK;
    }
    else if(pRAcc.is())
    {
        Reference<XAccessible> pxTable(pRXTable, UNO_QUERY);

        CMAccessible::g_pAccObjectManager->InsertAccObj(pRAcc.get(),pxTable.get());
        pRet = CMAccessible::get_IAccessibleFromXAccessible(pRAcc.get());

        if (pRet)
        {
            *accessible = pRet;
            pRet->AddRef();
            return S_OK;
        }
    }
    return E_FAIL;

    } catch(...) { return E_FAIL; }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_cellAt(long row, long column, IUnknown * * cell)
{
    return get_accessibleAt(row, column, cell);
}

/**
  * Gets accessible table caption.
  *
  * @param    accessible    the accessible object of table caption.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_caption(IUnknown * *)
{
    return E_NOTIMPL;
}

/**
  * Gets accessible column description (as string).
  *
  * @param    column        the column index.
  * @param    description   the description of the specified column.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_columnDescription(long column, BSTR * description)
{
    SolarMutexGuard g;

    try {

    if(description == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    const OUString& ouStr = pRXTable->getAccessibleColumnDescription(column);

    SysFreeString(*description);
    *description = SysAllocString(o3tl::toW(ouStr.getStr()));
    if (*description==nullptr)
        return E_FAIL;
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
  * Gets number of columns spanned by table cell.
  *
  * @param    row            the row of the specified cell.
  * @param    column         the column of the specified cell.
  * @param    spanColumns    the column span of the specified cell.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_columnExtentAt(long row, long column, long * nColumnsSpanned)
{
    SolarMutexGuard g;

    try {

    // Check pointer.
    if(nColumnsSpanned == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    *nColumnsSpanned = pRXTable->getAccessibleColumnExtentAt(row, column);
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
  * Gets accessible column header.
  *
  * @param    column        the column index.
  * @param    accessible    the accessible object of the specified column.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_columnHeader(IAccessibleTable __RPC_FAR *__RPC_FAR *accessibleTable, long *startingRowIndex)
{
    SolarMutexGuard g;

    try {

    if(accessibleTable == nullptr || startingRowIndex == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleTable> pRColumnHeaderTable = pRXTable->getAccessibleColumnHeaders();
    if(!pRColumnHeaderTable.is())
    {
        *accessibleTable = nullptr;
        return E_FAIL;
    }

    Reference<XAccessible> pRXColumnHeader(pRColumnHeaderTable,UNO_QUERY);

    if(!pRXColumnHeader.is())
    {
        *accessibleTable = nullptr;
        return E_FAIL;
    }
    *startingRowIndex = 0 ;

    IMAccessible* pIMacc = nullptr;
    HRESULT hr = createInstance<CMAccessible>(IID_IMAccessible, &pIMacc);
    if (!SUCCEEDED(hr))
    {
        return E_FAIL;
    }
    pIMacc->SetXAccessible(
        reinterpret_cast<hyper>(pRXColumnHeader.get()));
    pIMacc->QueryInterface(IID_IAccessibleTable,reinterpret_cast<void **>(accessibleTable));

    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
  * Gets total number of columns in table.
  *
  * @param    columnCount    the number of columns in table.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_nColumns(long * columnCount)
{
    SolarMutexGuard g;

    try {

    if(columnCount == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    *columnCount = pRXTable->getAccessibleColumnCount();
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
  * Gets total number of rows in table.
  *
  * @param    rowCount    the number of rows in table.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_nRows(long * rowCount)
{
    SolarMutexGuard g;

    try {

    if(rowCount == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    *rowCount = pRXTable->getAccessibleRowCount();
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
  * Gets total number of selected columns.
  *
  * @param    columnCount    the number of selected columns.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_nSelectedColumns(long * columnCount)
{
    SolarMutexGuard g;

    try {

    if(columnCount == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    Sequence<long> pSelected = pRXTable->getSelectedAccessibleColumns();
    *columnCount = pSelected.getLength();
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
  * Gets total number of selected rows.
  *
  * @param    rowCount    the number of selected rows.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_nSelectedRows(long * rowCount)
{
    SolarMutexGuard g;

    try {

    if(rowCount == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    Sequence<long> pSelected = pRXTable->getSelectedAccessibleRows();
    *rowCount = pSelected.getLength();
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
  * Gets accessible row description (as string).
  *
  * @param    row            the row index.
  * @param    description    the description of the specified row.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_rowDescription(long row, BSTR * description)
{
    SolarMutexGuard g;

    try {

    if(description == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    const OUString& ouStr = pRXTable->getAccessibleRowDescription(row);

    SysFreeString(*description);
    *description = SysAllocString(o3tl::toW(ouStr.getStr()));
    if (*description==nullptr)
        return E_FAIL;
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
  * Gets number of rows spanned by a table cell.
  *
  * @param    row            the row of the specified cell.
  * @param    column         the column of the specified cell.
  * @param    spanRows       the row span of the specified cell.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_rowExtentAt(long row, long column, long * nRowsSpanned)
{
    SolarMutexGuard g;

    try {

    // Check pointer.
    if(nRowsSpanned == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    *nRowsSpanned= pRXTable->getAccessibleRowExtentAt(row, column);

    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
  * Gets accessible row header.
  *
  * @param    row        the row index.
  * @param    accessible the accessible object of the row header.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_rowHeader(IAccessibleTable __RPC_FAR *__RPC_FAR *accessibleTable, long *startingColumnIndex)
{
    SolarMutexGuard g;

    try {

    if(accessibleTable == nullptr || startingColumnIndex == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleTable> pRRowHeaderTable = pRXTable->getAccessibleRowHeaders();
    if(!pRRowHeaderTable.is())
    {
        *accessibleTable = nullptr;
        return E_FAIL;
    }

    Reference<XAccessible> pRXRowHeader(pRRowHeaderTable,UNO_QUERY);

    if(!pRXRowHeader.is())
    {
        *accessibleTable = nullptr;
        return E_FAIL;
    }
    *startingColumnIndex = 0 ;

    IMAccessible* pIMacc = nullptr;
    HRESULT hr = createInstance<CMAccessible>(IID_IMAccessible, &pIMacc);
    if (!SUCCEEDED(hr))
    {
        return E_FAIL;
    }
    pIMacc->SetXAccessible(
        reinterpret_cast<hyper>(pRXRowHeader.get()));
    pIMacc->QueryInterface(IID_IAccessibleTable,reinterpret_cast<void **>(accessibleTable));

    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
  * Gets list of row indexes currently selected (0-based).
  *
  * @param    accessible     the accessible object array of the selected rows.
  * @param    nRows          the actual size of the accessible object array.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_selectedRows(long** rows, long* nRows)
{
    SolarMutexGuard g;

    try {

    if(rows == nullptr || nRows == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    Sequence<long> pSelected = pRXTable->getSelectedAccessibleRows();
    long count = pSelected.getLength() ;
    *nRows = count;

    *rows = static_cast<long*>(CoTaskMemAlloc(count * sizeof(long)));
    // #CHECK Memory Allocation#
    if(*rows == nullptr)
    {
        return E_FAIL;
    }
    for(int i=0; i<count; i++)
        (*rows)[i] = pSelected[i];

    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
  * Gets list of row indexes currently selected (0-based).
  *
  * @param    maxRows        This parameter is ignored.
  * @param    accessible     the accessible object array of the selected rows.
  * @param    nRows          the actual size of the accessible object array.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_selectedRows(long, long ** rows, long * nRows)
{
    return get_selectedRows(rows, nRows);
}

/**
  * Gets list of column indexes currently selected (0-based).
  *
  * @param    accessible    the accessible object array of the selected columns.
  * @param    numColumns    the actual size of accessible object array.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_selectedColumns(long ** columns, long * numColumns)
{
    SolarMutexGuard g;

    try {

    if(columns == nullptr || numColumns == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    Sequence<long> pSelected = pRXTable->getSelectedAccessibleColumns();
    long count = pSelected.getLength() ;
    *numColumns = count;

    *columns = static_cast<long*>(CoTaskMemAlloc(count * sizeof(long)));
    // #CHECK Memory Allocation#
    if(*columns == nullptr)
    {
        return E_FAIL;
    }
    for(int i=0; i<count; i++)
        (*columns)[i] = pSelected[i];

    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
  * Gets list of column indexes currently selected (0-based).
  *
  * @param    maxColumns    This parameter is ignored
  * @param    accessible    the accessible object array of the selected columns.
  * @param    numColumns    the actual size of accessible object array.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_selectedColumns(long, long ** columns, long * numColumns)
{
    return get_selectedColumns(columns, numColumns);
}

/**
  * Gets accessible table summary.
  *
  * @param    accessible   the accessible object of the summary.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_summary(IUnknown * * accessible)
{
    SolarMutexGuard g;

    try {

    if(accessible == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessible> pRAcc = pRXTable->getAccessibleSummary();

    IAccessible* pRet = CMAccessible::get_IAccessibleFromXAccessible(pRAcc.get());

    if(pRet)
    {
        *accessible = pRet;
        pRet->AddRef();
        return S_OK;
    }

    return E_FAIL;

    } catch(...) { return E_FAIL; }
}

/**
  * Determines if table column is selected.
  *
  * @param    column        the column index.
  * @param    isSelected    the result.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_isColumnSelected(long column, boolean * isSelected)
{
    SolarMutexGuard g;

    try {

    if(isSelected == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    *isSelected = pRXTable->isAccessibleColumnSelected(column);
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
  * Determines if table row is selected.
  *
  * @param    row           the row index.
  * @param    isSelected    the result.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_isRowSelected(long row, boolean * isSelected)
{
    SolarMutexGuard g;

    try {

    if(isSelected == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    *isSelected = pRXTable->isAccessibleRowSelected(row);
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
  * Determines if table cell is selected.
  *
  * @param    row            the row index.
  * @param    column         the column index.
  * @param    isSelected     the result.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_isSelected(long row, long column, boolean * isSelected)
{
    SolarMutexGuard g;

    try {

    if(isSelected == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    *isSelected = pRXTable->isAccessibleSelected(row, column);
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
  * Selects a row and unselect all previously selected rows.
  *
  * @param    row        the row index.
  * @param    success    the result.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::selectRow(long row)
{
    SolarMutexGuard g;

    try {

    // Check XAccessibleTable reference.
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleTableSelection>        pRTableExtent(pRXTable, UNO_QUERY);
    if(pRTableExtent.is())
    {
        pRTableExtent->selectRow(row);
        return S_OK;
    }
    else
    {
        // Get XAccessibleSelection.
        Reference<XAccessibleSelection>     pRSelection(pRXTable, UNO_QUERY);
        if(!pRSelection.is())
            return E_FAIL;

        // Select row.
        long            lCol, lColumnCount;
        lColumnCount = pRXTable->getAccessibleColumnCount();
        for(lCol = 0; lCol < lColumnCount; lCol ++)
        {
            sal_Int64 nChildIndex = pRXTable->getAccessibleIndex(row, lCol);
            pRSelection->selectAccessibleChild(nChildIndex);
        }

        return S_OK;
    }

    } catch(...) { return E_FAIL; }
}

/**
  * Selects a column and unselect all previously selected columns.
  *
  * @param    column    the column index.
  * @param    success   the result.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::selectColumn(long column)
{
    SolarMutexGuard g;

    try {

    // Check XAccessibleTable reference.
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleTableSelection>        pRTableExtent(pRXTable, UNO_QUERY);
    if(pRTableExtent.is())
    {
        pRTableExtent->selectColumn(column);
        return S_OK;
    }
    else
    {
        // Get XAccessibleSelection.
        Reference<XAccessibleSelection>     pRSelection(pRXTable, UNO_QUERY);
        if(!pRSelection.is())
            return E_FAIL;

        // Select column.
        long            lRow, lRowCount;
        lRowCount = pRXTable->getAccessibleRowCount();
        for(lRow = 0; lRow < lRowCount; lRow ++)
        {
            sal_Int64 nChildIndex = pRXTable->getAccessibleIndex(lRow, column);
            pRSelection->selectAccessibleChild(nChildIndex);
        }

        return S_OK;
    }
    // End of added.

    } catch(...) { return E_FAIL; }
}

/**
  * Unselects one row, leaving other selected rows selected (if any).
  *
  * @param    row        the row index.
  * @param    success    the result.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::unselectRow(long row)
{
    SolarMutexGuard g;

    try {

    // Check XAccessibleTable reference.
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleTableSelection>        pRTableExtent(pRXTable, UNO_QUERY);
    if(pRTableExtent.is())
    {
        if(pRTableExtent->unselectRow(row))
            return S_OK;
        else
            return E_FAIL;
    }
    else
    {
        // Get XAccessibleSelection.
        Reference<XAccessibleSelection>     pRSelection(pRXTable, UNO_QUERY);
        if(!pRSelection.is())
            return E_FAIL;

        // Select column.
        long            lColumn, lColumnCount;
        lColumnCount = pRXTable->getAccessibleColumnCount();
        for(lColumn = 0; lColumn < lColumnCount; lColumn ++)
        {
            sal_Int64 nChildIndex = pRXTable->getAccessibleIndex(row, lColumn);
            pRSelection->deselectAccessibleChild(nChildIndex);
        }

        return S_OK;
    }
    // End of added.

    } catch(...) { return E_FAIL; }
}

/**
  * Unselects one column, leaving other selected columns selected (if any).
  *
  * @param    column    the column index.
  * @param    success   the result.
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::unselectColumn(long column)
{
    SolarMutexGuard g;

    try {

    // Check XAccessibleTable reference.
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleTableSelection>        pRTableExtent(pRXTable, UNO_QUERY);
    if(pRTableExtent.is())
    {
        if(pRTableExtent->unselectColumn(column))
            return S_OK;
        else
            return E_FAIL;
    }
    else
    {
        // Get XAccessibleSelection.
        Reference<XAccessibleSelection>     pRSelection(pRXTable, UNO_QUERY);
        if(!pRSelection.is())
            return E_FAIL;

        // Unselect columns.
        long            lRow, lRowCount;
        lRowCount = pRXTable->getAccessibleRowCount();

        for(lRow = 0; lRow < lRowCount; lRow ++)
        {
            sal_Int64 nChildIndex = pRXTable->getAccessibleIndex(lRow, column);
            pRSelection->deselectAccessibleChild(nChildIndex);
        }
        return S_OK;
    }

    } catch(...) { return E_FAIL; }
}

/**
 * Override of IUNOXWrapper.
 *
 * @param    pXInterface    the pointer of UNO interface.
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::put_XInterface(hyper pXInterface)
{
    // internal IUNOXWrapper - no mutex meeded

    try {

    CUNOXWrapper::put_XInterface(pXInterface);
    //special query.
    if(pUNOInterface == nullptr)
        return E_INVALIDARG;

    Reference<XAccessibleContext> pRContext = pUNOInterface->getAccessibleContext();
    if( !pRContext.is() )
        return E_FAIL;

    Reference<XAccessibleTable> pRXI(pRContext,UNO_QUERY);
    if( !pRXI.is() )
        pRXTable = nullptr;
    else
        pRXTable = pRXI.get();
    return S_OK;

    } catch(...) { return E_FAIL; }
}


/**
  * Gets columnIndex of childIndex.
  *
  * @param    childIndex    childIndex
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_columnIndex(long childIndex, long * columnIndex)
{
    SolarMutexGuard g;

    try {

    if(columnIndex == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    *columnIndex = pRXTable->getAccessibleColumn(childIndex);
    return S_OK;

    } catch(...) { return E_FAIL; }
}
/**
  * Gets rowIndex of childIndex.
  *
  * @param    childIndex    childIndex
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_rowIndex(long childIndex, long * rowIndex)
{
    SolarMutexGuard g;

    try {

    if(rowIndex == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    *rowIndex = pRXTable->getAccessibleRow(childIndex);
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
  * Gets childIndex of childIndex.
  *
  * @param    childIndex    childIndex
  */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_childIndex(long RowIndex , long columnIndex, long * childIndex )
{
    SolarMutexGuard g;

    try {

    if(childIndex == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    sal_Int64 nIndex = pRXTable->getAccessibleIndex(RowIndex, columnIndex);
    if (nIndex > std::numeric_limits<long>::max())
    {
        // use -2 when the child index is too large to fit into 32 bit to neither use the
        // valid index of another child nor -1, which is more commonly used to indicate that
        // a child is no more inside of a parent or invalid otherwise
        SAL_WARN("vcl.qt", "CAccTable::get_childIndex: Child index exceeds maximum long value, "
                           "returning -2.");
        nIndex = -2;
    }
    *childIndex = nIndex;
    return S_OK;

    } catch(...) { return E_FAIL; }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_rowColumnExtentsAtIndex(long,
        long  *,
        long  *,
        long  *,
        long  *,
        boolean  *)
{
    return E_NOTIMPL;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_modelChange(IA2TableModelChange  *)
{
    return E_NOTIMPL;
}

// @brief Returns the total number of selected children
//   @param [out] childCount
//    Number of children currently selected
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_nSelectedChildren(long *childCount)
{
    SolarMutexGuard g;

    try {

    if(childCount == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleSelection> pRSelection(pRXTable, UNO_QUERY);
    if(!pRSelection.is())
        return E_FAIL;

    sal_Int64 nSelected = pRSelection->getSelectedAccessibleChildCount();
    if (nSelected > std::numeric_limits<long>::max())
    {
        SAL_WARN("iacc2", "CAccTable::get_nSelectedChildren: Selected item count exceeds maximum long value, "
                          "using max long.");
        nSelected = std::numeric_limits<long>::max();
    }
    *childCount = nSelected;
    return S_OK;

    } catch(...) { return E_FAIL; }
}



COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_nSelectedCells(long *cellCount)
{
    return get_nSelectedChildren(cellCount);
}

// @brief Returns a list of child indexes currently selected (0-based).
//   @param [in] maxChildren
//    Max children requested (possibly from IAccessibleTable::nSelectedChildren)
//   @param [out] children
//    array of indexes of selected children (each index is 0-based)
//   @param [out] nChildren
//    Length of array (not more than maxChildren)
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_selectedChildren(long, long **children, long *nChildren)
{
    SolarMutexGuard g;

    try {

    if(children == nullptr || nChildren == nullptr)
        return E_INVALIDARG;

    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleSelection> pRSelection(pRXTable, UNO_QUERY);
    if(!pRSelection.is())
        return E_FAIL;

    sal_Int64 nChildCount = pRSelection->getSelectedAccessibleChildCount();
    if (nChildCount > std::numeric_limits<long>::max())
    {
        SAL_WARN("iacc2", "CAccTable::get_selectedChildren: Selected child count exceeds maximum long value, "
                          "using max long.");
        nChildCount = std::numeric_limits<long>::max();
    }

    *nChildren = nChildCount;
    *children = static_cast<long*>(CoTaskMemAlloc(nChildCount * sizeof(long)));
    assert(*children && "Don't handle OOM conditions");

    for( sal_Int64 i = 0; i< nChildCount; i++)
    {
        Reference<XAccessible> pRAcc = pRSelection->getSelectedAccessibleChild(i);
        if(pRAcc.is())
        {
            Reference<XAccessibleContext> pRContext(pRAcc, UNO_QUERY);
            if( !pRContext.is() )
                return E_FAIL;


            sal_Int64 nChildIndex =  pRContext->getAccessibleIndexInParent();
            if (nChildIndex > std::numeric_limits<long>::max())
            {
                SAL_WARN("iacc2", "CAccTable::get_selectedChildren: Child index exceeds maximum long value, "
                                  "using max long.");
                nChildIndex = std::numeric_limits<long>::max();
            }
            (*children)[i] = nChildIndex;
        }
    }

    return S_OK;

    } catch(...) { return E_FAIL; }

}

/**
 * @brief Returns a list of accessibles currently selected.
 * @param cells Pointer to an array of references to selected accessibles.
 *        The array is allocated by the server with CoTaskMemAlloc and
 *        freed by the client with CoTaskMemFree.
 * @param nSelectedCells The number of accessibles returned; the size of the returned array.
 * @return S_FALSE if there are none, [out] values are NULL and 0 respectively, otherwise S_OK
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTable::get_selectedCells(IUnknown * * * cells, long *nSelectedCells)
{
    SolarMutexGuard g;

    try {

    if (cells == nullptr || nSelectedCells == nullptr)
        return E_INVALIDARG;

    if (!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleSelection> xSelection(pRXTable, UNO_QUERY);
    if (!xSelection.is())
        return E_FAIL;

    sal_Int64 nSelected = xSelection->getSelectedAccessibleChildCount();
    if (nSelected > std::numeric_limits<long>::max())
    {
        SAL_WARN("iacc2", "CAccTable::get_selectedCells: Selected cell count exceeds maximum long value, "
                          "using max long.");
        nSelected = std::numeric_limits<long>::max();
    }
    *nSelectedCells = nSelected;

    *cells = static_cast<IUnknown**>(CoTaskMemAlloc(nSelected * sizeof(IUnknown*)));
    assert(*cells && "Don't handle OOM conditions");

    for (sal_Int64 i = 0; i < nSelected; i++)
    {
        Reference<XAccessible> xAcc = xSelection->getSelectedAccessibleChild(i);
        assert(xAcc.is());

        IAccessible* pIAccessible = CMAccessible::get_IAccessibleFromXAccessible(xAcc.get());

        if (!pIAccessible)
        {
            Reference<XAccessible> xTable(pRXTable, UNO_QUERY);
            CMAccessible::g_pAccObjectManager->InsertAccObj(xAcc.get(), xTable.get());
            pIAccessible = CMAccessible::get_IAccessibleFromXAccessible(xAcc.get());
        }

        assert(pIAccessible && "Couldn't retrieve IAccessible object");

        pIAccessible->AddRef();
        (*cells)[i] = pIAccessible;
    }

    return S_OK;

    } catch(...) { return E_FAIL; }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
