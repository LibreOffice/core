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
#include "AccTable.h"

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
#include  "UAccCOM.h"
#if defined __clang__
#pragma clang diagnostic pop
#endif

#include <vcl/svapp.hxx>

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

STDMETHODIMP CAccTable::get_accessibleAt(long row, long column, IUnknown * * accessible)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(accessible == nullptr)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessible> pRAcc = GetXInterface()->getAccessibleCellAt(row,column);

    if(!pRAcc.is())
    {
        *accessible = nullptr;
        return E_FAIL;
    }

    IAccessible* pRet = nullptr;

    BOOL isTRUE = CMAccessible::get_IAccessibleFromXAccessible(pRAcc.get(), &pRet);
    if(isTRUE)
    {
        *accessible = static_cast<IAccessible2 *>(pRet);
        pRet->AddRef();
        return S_OK;
    }
    else if(pRAcc.is())
    {
        Reference<XAccessible> pxTable(GetXInterface(),UNO_QUERY);

        CMAccessible::g_pAgent->InsertAccObj(pRAcc.get(),pxTable.get());
        isTRUE = CMAccessible::get_IAccessibleFromXAccessible(pRAcc.get(), &pRet);

        if(isTRUE)
        {
            *accessible = static_cast<IAccessible2 *>(pRet);
            pRet->AddRef();
            return S_OK;
        }
    }
    return E_FAIL;

    LEAVE_PROTECTED_BLOCK
}

/**
  * Gets accessible table caption.
  *
  * @param    accessible    the accessible object of table caption.
  */
STDMETHODIMP CAccTable::get_caption(IUnknown * *)
{
    return E_NOTIMPL;
}

/**
  * Gets accessible column description (as string).
  *
  * @param    column        the column index.
  * @param    description   the description of the specified column.
  */
STDMETHODIMP CAccTable::get_columnDescription(long column, BSTR * description)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(description == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    const ::rtl::OUString& ouStr = GetXInterface()->getAccessibleColumnDescription(column);
    // #CHECK#

    SAFE_SYSFREESTRING(*description);//??
    *description = SysAllocString(SAL_W(ouStr.getStr()));
    if(description==nullptr)
        return E_FAIL;
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
  * Gets number of columns spanned by table cell.
  *
  * @param    row            the row of the specified cell.
  * @param    column         the column of the specified cell.
  * @param    spanColumns    the column span of the specified cell.
  */
STDMETHODIMP CAccTable::get_columnExtentAt(long row, long column, long * nColumnsSpanned)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    XAccessibleTable    *pXAccTable = GetXInterface();

    // Check pointer.
    if(nColumnsSpanned == nullptr)
        return E_INVALIDARG;

    // Get Extent.
    if(pXAccTable)
    {
        long lExt = pXAccTable->getAccessibleColumnExtentAt(row,column);

        // Fill Extent struct.
        *nColumnsSpanned = lExt;
        return S_OK;
    }

    return E_FAIL;

    LEAVE_PROTECTED_BLOCK
}

/**
  * Gets accessible column header.
  *
  * @param    column        the column index.
  * @param    accessible    the accessible object of the specified column.
  */
STDMETHODIMP CAccTable::get_columnHeader(IAccessibleTable __RPC_FAR *__RPC_FAR *accessibleTable, long *startingRowIndex)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(accessibleTable == nullptr || startingRowIndex == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleTable> pRColumnHeaderTable = GetXInterface()->getAccessibleColumnHeaders();
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

    LEAVE_PROTECTED_BLOCK
}

/**
  * Gets total number of columns in table.
  *
  * @param    columnCount    the number of columns in table.
  */
STDMETHODIMP CAccTable::get_nColumns(long * columnCount)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(columnCount == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    *columnCount = GetXInterface()->getAccessibleColumnCount();
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
  * Gets total number of rows in table.
  *
  * @param    rowCount    the number of rows in table.
  */
STDMETHODIMP CAccTable::get_nRows(long * rowCount)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(rowCount == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    *rowCount = GetXInterface()->getAccessibleRowCount();
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
  * Gets total number of selected columns.
  *
  * @param    columnCount    the number of selected columns.
  */
STDMETHODIMP CAccTable::get_nSelectedColumns(long * columnCount)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(columnCount == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    Sequence<long> pSelected = GetXInterface()->getSelectedAccessibleColumns();
    *columnCount = pSelected.getLength();
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
  * Gets total number of selected rows.
  *
  * @param    rowCount    the number of selected rows.
  */
STDMETHODIMP CAccTable::get_nSelectedRows(long * rowCount)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(rowCount == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    Sequence<long> pSelected = GetXInterface()->getSelectedAccessibleRows();
    *rowCount = pSelected.getLength();
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
  * Gets accessible row description (as string).
  *
  * @param    row            the row index.
  * @param    description    the description of the specified row.
  */
STDMETHODIMP CAccTable::get_rowDescription(long row, BSTR * description)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(description == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    const ::rtl::OUString& ouStr = GetXInterface()->getAccessibleRowDescription(row);
    // #CHECK#

    SAFE_SYSFREESTRING(*description);
    *description = SysAllocString(SAL_W(ouStr.getStr()));
    if(description==nullptr)
        return E_FAIL;

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
  * Gets number of rows spanned by a table cell.
  *
  * @param    row            the row of the specified cell.
  * @param    column         the column of the specified cell.
  * @param    spanRows       the row span of the specified cell.
  */
STDMETHODIMP CAccTable::get_rowExtentAt(long row, long column, long * nRowsSpanned)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    XAccessibleTable    *pXAccTable = GetXInterface();

    // Check pointer.
    if(nRowsSpanned == nullptr)
        return E_INVALIDARG;

    // Get Extent.
    if(pXAccTable)
    {
        long lExt = GetXInterface()->getAccessibleRowExtentAt(row,column);

        // Fill Extent struct.
        *nRowsSpanned= lExt;

        return S_OK;
    }

    return E_FAIL;

    LEAVE_PROTECTED_BLOCK
}

/**
  * Gets accessible row header.
  *
  * @param    row        the row index.
  * @param    accessible the accessible object of the row header.
  */
STDMETHODIMP CAccTable::get_rowHeader(IAccessibleTable __RPC_FAR *__RPC_FAR *accessibleTable, long *startingColumnIndex)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(accessibleTable == nullptr || startingColumnIndex == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleTable> pRRowHeaderTable = GetXInterface()->getAccessibleRowHeaders();
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

    LEAVE_PROTECTED_BLOCK
}

/**
  * Gets list of row indexes currently selected (0-based).
  *
  * @param    maxRows        the max number of the rows.
  * @param    accessible     the accessible object array of the selected rows.
  * @param    nRows          the actual size of the accessible object array.
  */
STDMETHODIMP CAccTable::get_selectedRows(long, long ** rows, long * nRows)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(rows == nullptr || nRows == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    Sequence<long> pSelected = GetXInterface()->getSelectedAccessibleRows();
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

    LEAVE_PROTECTED_BLOCK
}

/**
  * Gets list of column indexes currently selected (0-based).
  *
  * @param    maxColumns    the max number of the columns.
  * @param    accessible    the accessible object array of the selected columns.
  * @param    numColumns    the actual size of accessible object array.
  */
STDMETHODIMP CAccTable::get_selectedColumns(long, long ** columns, long * numColumns)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(columns == nullptr || numColumns == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    Sequence<long> pSelected = GetXInterface()->getSelectedAccessibleColumns();
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

    LEAVE_PROTECTED_BLOCK
}

/**
  * Gets accessible table summary.
  *
  * @param    accessible   the accessible object of the summary.
  */
STDMETHODIMP CAccTable::get_summary(IUnknown * * accessible)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(accessible == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
    {
        return E_FAIL;
    }
    Reference<XAccessible> pRAcc = GetXInterface()->getAccessibleSummary();

    IAccessible* pRet = nullptr;
    CMAccessible::get_IAccessibleFromXAccessible(pRAcc.get(), &pRet);

    if(pRet)
    {
        *accessible = static_cast<IAccessible2 *>(pRet);
        pRet->AddRef();
        return S_OK;
    }

    return E_FAIL;

    LEAVE_PROTECTED_BLOCK
}

/**
  * Determines if table column is selected.
  *
  * @param    column        the column index.
  * @param    isSelected    the result.
  */
STDMETHODIMP CAccTable::get_isColumnSelected(long column, unsigned char * isSelected)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(isSelected == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    *isSelected = GetXInterface()->isAccessibleColumnSelected(column);
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
  * Determines if table row is selected.
  *
  * @param    row           the row index.
  * @param    isSelected    the result.
  */
STDMETHODIMP CAccTable::get_isRowSelected(long row, unsigned char * isSelected)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(isSelected == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
    {
        return E_FAIL;
    }
    *isSelected = GetXInterface()->isAccessibleRowSelected(row);
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
  * Determines if table cell is selected.
  *
  * @param    row            the row index.
  * @param    column         the column index.
  * @param    isSelected     the result.
  */
STDMETHODIMP CAccTable::get_isSelected(long row, long column, unsigned char * isSelected)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(isSelected == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    *isSelected = GetXInterface()->isAccessibleSelected(row,column);
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
  * Selects a row and unselect all previously selected rows.
  *
  * @param    row        the row index.
  * @param    success    the result.
  */
STDMETHODIMP CAccTable::selectRow(long row)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // Check XAccessibleTable reference.
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleTableSelection>        pRTableExtent(pRXTable, UNO_QUERY);
    if(pRTableExtent.is())
    {
        pRTableExtent.get()->selectRow(row);
        return S_OK;
    }
    else
    {
        // Get XAccessibleSelection.
        Reference<XAccessibleSelection>     pRSelection(GetXInterface(), UNO_QUERY);
        if(!pRSelection.is())
            return E_FAIL;

        // Select row.
        long            lCol, lColumnCount;
        lColumnCount = GetXInterface()->getAccessibleColumnCount();
        for(lCol = 0; lCol < lColumnCount; lCol ++)
        {
            long lChildIndex = GetXInterface()->getAccessibleIndex(row, lCol);
            pRSelection.get()->selectAccessibleChild(lChildIndex);
        }

        return S_OK;
    }

    LEAVE_PROTECTED_BLOCK
}

/**
  * Selects a column and unselect all previously selected columns.
  *
  * @param    column    the column index.
  * @param    success   the result.
  */
STDMETHODIMP CAccTable::selectColumn(long column)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // Check XAccessibleTable reference.
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleTableSelection>        pRTableExtent(GetXInterface(), UNO_QUERY);
    if(pRTableExtent.is())
    {
        pRTableExtent.get()->selectColumn(column);
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
        lRowCount = GetXInterface()->getAccessibleRowCount();
        for(lRow = 0; lRow < lRowCount; lRow ++)
        {
            long lChildIndex = GetXInterface()->getAccessibleIndex(lRow, column);
            pRSelection.get()->selectAccessibleChild(lChildIndex);
        }

        return S_OK;
    }
    // End of added.

    LEAVE_PROTECTED_BLOCK
}

/**
  * Unselects one row, leaving other selected rows selected (if any).
  *
  * @param    row        the row index.
  * @param    success    the result.
  */
STDMETHODIMP CAccTable::unselectRow(long row)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // Check XAccessibleTable reference.
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleTableSelection>        pRTableExtent(GetXInterface(), UNO_QUERY);
    if(pRTableExtent.is())
    {
        if(pRTableExtent.get()->unselectRow(row))
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
        lColumnCount = GetXInterface()->getAccessibleColumnCount();
        for(lColumn = 0; lColumn < lColumnCount; lColumn ++)
        {
            long lChildIndex = GetXInterface()->getAccessibleIndex(row,lColumn);
            pRSelection.get()->deselectAccessibleChild(lChildIndex);
        }

        return S_OK;
    }
    // End of added.

    LEAVE_PROTECTED_BLOCK
}

/**
  * Unselects one column, leaving other selected columns selected (if any).
  *
  * @param    column    the column index.
  * @param    success   the result.
  */
STDMETHODIMP CAccTable::unselectColumn(long column)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // Check XAccessibleTable reference.
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleTableSelection>        pRTableExtent(GetXInterface(), UNO_QUERY);
    if(pRTableExtent.is())
    {
        if(pRTableExtent.get()->unselectColumn(column))
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
        lRowCount = GetXInterface()->getAccessibleRowCount();

        for(lRow = 0; lRow < lRowCount; lRow ++)
        {
            long lChildIndex = GetXInterface()->getAccessibleIndex(lRow, column);
            pRSelection.get()->deselectAccessibleChild(lChildIndex);
        }
        return S_OK;
    }

    LEAVE_PROTECTED_BLOCK
}

/**
 * Override of IUNOXWrapper.
 *
 * @param    pXInterface    the pointer of UNO interface.
 */
STDMETHODIMP CAccTable::put_XInterface(hyper pXInterface)
{
    // internal IUNOXWrapper - no mutex meeded

    ENTER_PROTECTED_BLOCK

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

    LEAVE_PROTECTED_BLOCK
}

/**
  * Gets columnIndex of childIndex.
  *
  * @param    childIndex    childIndex
  */
STDMETHODIMP CAccTable::get_columnIndex(long childIndex, long * columnIndex)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(columnIndex == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    *columnIndex = GetXInterface()->getAccessibleColumn(childIndex);
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}
/**
  * Gets rowIndex of childIndex.
  *
  * @param    childIndex    childIndex
  */
STDMETHODIMP CAccTable::get_rowIndex(long childIndex, long * rowIndex)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(rowIndex == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    *rowIndex = GetXInterface()->getAccessibleRow(childIndex);
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}
/**
  * Gets childIndex of childIndex.
  *
  * @param    childIndex    childIndex
  */
STDMETHODIMP CAccTable::get_childIndex(long RowIndex , long columnIndex, long * childIndex )
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(childIndex == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    *childIndex = GetXInterface()->getAccessibleIndex(RowIndex, columnIndex);
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

STDMETHODIMP CAccTable::get_rowColumnExtentsAtIndex(long,
        long  *,
        long  *,
        long  *,
        long  *,
        boolean  *)
{
    return E_NOTIMPL;
}

STDMETHODIMP CAccTable::get_modelChange(IA2TableModelChange  *)
{
    return E_NOTIMPL;
}

// @brief Returns the total number of selected children
//   @param [out] childCount
//    Number of children currently selected
STDMETHODIMP CAccTable::get_nSelectedChildren(long *childCount)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(childCount == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleSelection>     pRSelection(GetXInterface(), UNO_QUERY);
    if(!pRSelection.is())
        return E_FAIL;

    *childCount = pRSelection->getSelectedAccessibleChildCount();
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

// @brief Returns a list of child indexes currently selected (0-based).
//   @param [in] maxChildren
//    Max children requested (possibly from IAccessibleTable::nSelectedChildren)
//   @param [out] children
//    array of indexes of selected children (each index is 0-based)
//   @param [out] nChildren
//    Length of array (not more than maxChildren)
STDMETHODIMP CAccTable::get_selectedChildren(long, long **children, long *nChildren)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(children == nullptr || nChildren == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleSelection>     pRSelection(GetXInterface(), UNO_QUERY);
    if(!pRSelection.is())
        return E_FAIL;

    long childCount = pRSelection->getSelectedAccessibleChildCount() ;

    *nChildren = childCount;

    *children = static_cast<long*>(CoTaskMemAlloc(childCount * sizeof(long)));

    for( long i = 0; i< childCount; i++)
    {
        Reference<XAccessible> pRAcc = pRSelection->getSelectedAccessibleChild(i);
        if(pRAcc.is())
        {
            Reference<XAccessibleContext> pRContext(pRAcc, UNO_QUERY);
            if( !pRContext.is() )
                return E_FAIL;

            long childIndex = pRContext->getAccessibleIndexInParent();
            (*children)[i] = childIndex;
        }
    }

    return S_OK;

    LEAVE_PROTECTED_BLOCK

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
