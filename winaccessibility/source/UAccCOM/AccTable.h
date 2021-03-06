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

#pragma once

#include "Resource.h"       // main symbols

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include "UNOXWrapper.h"

/**
 * CAccTable implements IAccessibleTable interface.
 */
class ATL_NO_VTABLE CAccTable :
            public CComObjectRoot,
            public CComCoClass<CAccTable, &CLSID_AccTable>,
            public IAccessibleTable,
            public CUNOXWrapper

{
public:
    CAccTable()
    {
            }
    virtual ~CAccTable()
    {
            }

    BEGIN_COM_MAP(CAccTable)
    COM_INTERFACE_ENTRY(IAccessibleTable)
    COM_INTERFACE_ENTRY(IUNOXWrapper)
    COM_INTERFACE_ENTRY_FUNC_BLIND(NULL,SmartQI_)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
    END_COM_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif

    static HRESULT WINAPI SmartQI_(void* pv,
                                   REFIID iid, void** ppvObject, DWORD_PTR)
    {
        return static_cast<CAccTable*>(pv)->SmartQI(iid,ppvObject);
    }

    HRESULT SmartQI(REFIID iid, void** ppvObject)
    {
        if( m_pOuterUnknown )
            return OuterQueryInterface(iid,ppvObject);
        return E_FAIL;
    }

    DECLARE_NO_REGISTRY()

public:
    // IAccessibleTable

    // Gets accessible table cell.
    STDMETHOD(get_accessibleAt)(long row, long column, IUnknown * * accessible) override;

    // Gets accessible table caption.
    STDMETHOD(get_caption)(IUnknown * * accessible) override;

    // Gets accessible column description (as string).
    STDMETHOD(get_columnDescription)(long column, BSTR * description) override;

    // Gets number of columns spanned by table cell.
    STDMETHOD(get_columnExtentAt)(long row, long column, long * nColumnsSpanned) override;

    // Gets accessible column header.
    STDMETHOD(get_columnHeader)(IAccessibleTable __RPC_FAR *__RPC_FAR *accessibleTable, long *startingRowIndex) override;

    // Gets total number of columns in table.
    STDMETHOD(get_nColumns)(long * columnCount) override;

    // Gets total number of rows in table.
    STDMETHOD(get_nRows)(long * rowCount) override;

    // Gets total number of selected columns.
    STDMETHOD(get_nSelectedColumns)(long * columnCount) override;

    // Gets total number of selected rows.
    STDMETHOD(get_nSelectedRows)(long * rowCount) override;

    // Gets accessible row description (as string).
    STDMETHOD(get_rowDescription)(long row, BSTR * description) override;

    // Gets number of rows spanned by a table cell.
    STDMETHOD(get_rowExtentAt)(long row, long column, long * nRowsSpanned) override;

    // Gets accessible row header.
    STDMETHOD(get_rowHeader)(IAccessibleTable __RPC_FAR *__RPC_FAR *accessibleTable, long *startingColumnIndex) override;

    // Gets list of row indexes currently selected (0-based).
    STDMETHOD(get_selectedRows)(long maxRows, long **rows, long * nRows) override;

    // Gets list of column indexes currently selected (0-based).
    STDMETHOD(get_selectedColumns)(long maxColumns, long **columns, long * numColumns) override;

    // Gets accessible table summary.
    STDMETHOD(get_summary)(IUnknown * * accessible) override;

    // Determines if table column is selected.
    STDMETHOD(get_isColumnSelected)(long column, boolean * isSelected) override;

    // Determines if table row is selected.
    STDMETHOD(get_isRowSelected)(long row, boolean * isSelected) override;

    // Determines if table cell is selected.
    STDMETHOD(get_isSelected)(long row, long column, boolean * isSelected) override;

    // Selects a row and unselect all previously selected rows.
    STDMETHOD(selectRow)(long row ) override;


    // Selects a column and unselect all previously selected columns.

    STDMETHOD(selectColumn)(long column) override;

    // Unselects one row, leaving other selected rows selected (if any).
    STDMETHOD(unselectRow)(long row) override;

    // Unselects one column, leaving other selected columns selected (if any).
    STDMETHOD(unselectColumn)(long column) override;

    //get Column index
    STDMETHOD(get_columnIndex)(long childIndex, long * columnIndex) override;

    STDMETHOD(get_rowIndex)(long childIndex, long * rowIndex) override;

    STDMETHOD(get_childIndex)(long rowIndex,long columnIndex, long * childIndex) override;

    STDMETHOD(get_nSelectedChildren)(long *childCount) override;

    STDMETHOD(get_selectedChildren)(long maxChildren, long **children, long *nChildren) override;

    STDMETHOD(get_rowColumnExtentsAtIndex)( long index,
                                            long  *row,
                                            long  *column,
                                            long  *rowExtents,
                                            long  *columnExtents,
                                            boolean  *isSelected) override;

    STDMETHOD(get_modelChange)(IA2TableModelChange  *modelChange) override;

    // Override of IUNOXWrapper.
    STDMETHOD(put_XInterface)(hyper pXInterface) override;

private:

    css::uno::Reference<css::accessibility::XAccessibleTable> pRXTable;

    css::accessibility::XAccessibleTable* GetXInterface()
    {
        return pRXTable.get();
    }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
