/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

/**
 * AccTable.cpp : Implementation of CAccTable.
 */
#include "stdafx.h"
#include "UAccCOM2.h"
#include "AccTable.h"
#include <com/sun/star/accessibility/XAccessible.hpp>
#include "MAccessible.h"

#include "act.hxx"

#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLETABLEEXTENT_HPP_
#include <com/sun/star/accessibility/XAccessibleTableSelection.hpp>
#endif

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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(accessible == NULL)
        return E_INVALIDARG;
    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessible> pRAcc = GetXInterface()->getAccessibleCellAt(row,column);

    if(!pRAcc.is())
    {
        *accessible = NULL;
        return E_FAIL;
    }

    IAccessible* pRet = NULL;

    BOOL isTRUE = CMAccessible::get_IAccessibleFromXAccessible((long)pRAcc.get(),&pRet);
    if(isTRUE)
    {
        *accessible = (IAccessible2 *)pRet;
        pRet->AddRef();
        return S_OK;
    }
    else if(pRAcc.is())
    {
        Reference<XAccessible> pxTable(GetXInterface(),UNO_QUERY);

        CMAccessible::g_pAgent->InsertAccObj(pRAcc.get(),pxTable.get());
        isTRUE = CMAccessible::get_IAccessibleFromXAccessible((long)pRAcc.get(),&pRet);

        if(isTRUE)
        {
            *accessible = (IAccessible2 *)pRet;
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
  * @param    accessible    the accessible object of table cpation.
  */
STDMETHODIMP CAccTable::get_caption(IUnknown * *)
{


    ENTER_PROTECTED_BLOCK

    return E_NOTIMPL;

    LEAVE_PROTECTED_BLOCK
}

/**
  * Gets accessible column description (as string).
  *
  * @param    column        the column index.
  * @param    description   the description of the specified column.
  */
STDMETHODIMP CAccTable::get_columnDescription(long column, BSTR * description)
{

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(description == NULL)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    const ::rtl::OUString& ouStr = GetXInterface()->getAccessibleColumnDescription(column);
    // #CHECK#

    SAFE_SYSFREESTRING(*description);//??
    *description = SysAllocString((OLECHAR*)ouStr.getStr());
    if(description==NULL)
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    XAccessibleTable    *pXAccTable = GetXInterface();

    // Check pointer.
    if(nColumnsSpanned == NULL)
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(accessibleTable == NULL || startingRowIndex == NULL)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleTable> pRColumnHeaderTable = GetXInterface()->getAccessibleColumnHeaders();
    if(!pRColumnHeaderTable.is())
    {
        *accessibleTable = NULL;
        return E_FAIL;
    }

    Reference<XAccessible> pRXColumnHeader(pRColumnHeaderTable,UNO_QUERY);

    if(!pRXColumnHeader.is())
    {
        *accessibleTable = NULL;
        return E_FAIL;
    }
    *startingRowIndex = 0 ;

    IAccessible* m_pIMacc = NULL;
    ActivateActContext();
    HRESULT hr = CoCreateInstance( CLSID_MAccessible, NULL, CLSCTX_ALL ,
                                    IID_IMAccessible,
                                    (void **)&m_pIMacc
                                  );
    DeactivateActContext();
    ((CMAccessible*)m_pIMacc)->SetXAccessible((long)pRXColumnHeader.get());
    m_pIMacc->QueryInterface(IID_IAccessibleTable,(void **)accessibleTable);
    if( SUCCEEDED(hr) )
    {
        return S_OK;
    }

    return E_FAIL;

    LEAVE_PROTECTED_BLOCK
}

/**
  * Gets total number of columns in table.
  *
  * @param    columnCount    the number of columns in table.
  */
STDMETHODIMP CAccTable::get_nColumns(long * columnCount)
{

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(columnCount == NULL)
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(rowCount == NULL)
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(columnCount == NULL)
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(rowCount == NULL)
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(description == NULL)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    const ::rtl::OUString& ouStr = GetXInterface()->getAccessibleRowDescription(row);
    // #CHECK#

    SAFE_SYSFREESTRING(*description);
    *description = SysAllocString((OLECHAR*)ouStr.getStr());
    if(description==NULL)
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    XAccessibleTable    *pXAccTable = GetXInterface();

    // Check pointer.
    if(nRowsSpanned == NULL)
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(accessibleTable == NULL || startingColumnIndex == NULL)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleTable> pRRowHeaderTable = GetXInterface()->getAccessibleRowHeaders();
    if(!pRRowHeaderTable.is())
    {
        *accessibleTable = NULL;
        return E_FAIL;
    }

    Reference<XAccessible> pRXRowHeader(pRRowHeaderTable,UNO_QUERY);

    if(!pRXRowHeader.is())
    {
        *accessibleTable = NULL;
        return E_FAIL;
    }
    *startingColumnIndex = 0 ;

    IAccessible* m_pIMacc = NULL;
    ActivateActContext();
    HRESULT hr = CoCreateInstance( CLSID_MAccessible, NULL, CLSCTX_ALL ,
                                    IID_IMAccessible,
                                    (void **)&m_pIMacc
                                  );
    DeactivateActContext();
    ((CMAccessible*)m_pIMacc)->SetXAccessible((long)pRXRowHeader.get());
    m_pIMacc->QueryInterface(IID_IAccessibleTable,(void **)accessibleTable);
    if( SUCCEEDED(hr) )
    {
        return S_OK;
    }

    return E_FAIL;

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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(rows == NULL || nRows == NULL)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    Sequence<long> pSelected = GetXInterface()->getSelectedAccessibleRows();
    long count = pSelected.getLength() ;
    *nRows = count;

    *rows = reinterpret_cast<long*>(CoTaskMemAlloc((count) * sizeof(long)));
    // #CHECK Memory Allocation#
    if(*rows == NULL)
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(columns == NULL || numColumns == NULL)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    Sequence<long> pSelected = GetXInterface()->getSelectedAccessibleColumns();
    long count = pSelected.getLength() ;
    *numColumns = count;

    *columns = reinterpret_cast<long*>(CoTaskMemAlloc((count) * sizeof(long)));
    // #CHECK Memory Allocation#
    if(*columns == NULL)
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(accessible == NULL)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
    {
        return E_FAIL;
    }
    Reference<XAccessible> pRAcc = GetXInterface()->getAccessibleSummary();

    IAccessible* pRet = NULL;
    BOOL isTRUE = CMAccessible::get_IAccessibleFromXAccessible((long)pRAcc.get(),&pRet);

    if(pRet)
    {
        *accessible = (IAccessible2 *)pRet;
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(isSelected == NULL)
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(isSelected == NULL)
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(isSelected == NULL)
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

    CHECK_ENABLE_INF

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
        long            lCol, lColumnCount, lChildIndex;
        lColumnCount = GetXInterface()->getAccessibleColumnCount();
        for(lCol = 0; lCol < lColumnCount; lCol ++)
        {
            lChildIndex = GetXInterface()->getAccessibleIndex(row, lCol);
            pRSelection.get()->selectAccessibleChild(lChildIndex);
        }

        return S_OK;
    }
    return S_OK;

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

    CHECK_ENABLE_INF

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
        long            lRow, lRowCount, lChildIndex;
        lRowCount = GetXInterface()->getAccessibleRowCount();
        for(lRow = 0; lRow < lRowCount; lRow ++)
        {
            lChildIndex = GetXInterface()->getAccessibleIndex(lRow, column);
            pRSelection.get()->selectAccessibleChild(lChildIndex);
        }

        return S_OK;
    }
    return S_OK;
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

    CHECK_ENABLE_INF

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
        long            lColumn, lColumnCount, lChildIndex;
        lColumnCount = GetXInterface()->getAccessibleColumnCount();
        for(lColumn = 0; lColumn < lColumnCount; lColumn ++)
        {
            lChildIndex = GetXInterface()->getAccessibleIndex(row,lColumn);
            pRSelection.get()->deselectAccessibleChild(lChildIndex);
        }

        return S_OK;
    }
    return S_OK;
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

    CHECK_ENABLE_INF

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
        long            lRow, lRowCount, lChildIndex;
        lRowCount = GetXInterface()->getAccessibleRowCount();

        for(lRow = 0; lRow < lRowCount; lRow ++)
        {
            lChildIndex = GetXInterface()->getAccessibleIndex(lRow, column);
            pRSelection.get()->deselectAccessibleChild(lChildIndex);
        }
        return S_OK;
    }

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
 * Overide of IUNOXWrapper.
 *
 * @param    pXInterface    the pointer of UNO interface.
 */
STDMETHODIMP CAccTable::put_XInterface(long pXInterface)
{

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    CUNOXWrapper::put_XInterface(pXInterface);
    //special query.
    if(pUNOInterface == NULL)
        return E_INVALIDARG;

    Reference<XAccessibleContext> pRContext = pUNOInterface->getAccessibleContext();
    if( !pRContext.is() )
        return E_FAIL;

    Reference<XAccessibleTable> pRXI(pRContext,UNO_QUERY);
    if( !pRXI.is() )
        pRXTable = NULL;
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(columnIndex == NULL)
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(rowIndex == NULL)
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(childIndex == NULL)
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    return E_NOTIMPL;

    LEAVE_PROTECTED_BLOCK
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(childCount == NULL)
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

    CHECK_ENABLE_INF

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(children == NULL || nChildren == NULL)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXTable.is())
        return E_FAIL;

    Reference<XAccessibleSelection>     pRSelection(GetXInterface(), UNO_QUERY);
    if(!pRSelection.is())
        return E_FAIL;

    long childCount = pRSelection->getSelectedAccessibleChildCount() ;

    *nChildren = childCount;

    *children = reinterpret_cast<long*>(CoTaskMemAlloc((childCount) * sizeof(long)));

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
