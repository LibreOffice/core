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

#include "AccTableCell.h"
#include "MAccessible.h"

#include <vcl/svapp.hxx>
#include <com/sun/star/accessibility/XAccessible.hpp>

using namespace com::sun::star::accessibility;
using namespace com::sun::star::uno;

CAccTableCell::CAccTableCell()
    : m_nIndexInParent(0)
{
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTableCell::put_XInterface(hyper pXInterface)
{
    // internal IUNOXWrapper - no mutex meeded

    try
    {
        CUNOXWrapper::put_XInterface(pXInterface);
        if (pUNOInterface == nullptr)
            return E_INVALIDARG;

        Reference<XAccessibleContext> xContext = pUNOInterface->getAccessibleContext();
        if (!xContext.is())
            return E_FAIL;

        // retrieve reference to table (parent of the cell)
        Reference<XAccessibleContext> xParentContext
            = xContext->getAccessibleParent()->getAccessibleContext();
        Reference<XAccessibleTable> xTable(xParentContext, UNO_QUERY);

        if (!xTable.is())
        {
            m_xTable.clear();
            return E_FAIL;
        }

        m_xTable = xTable;
        m_nIndexInParent = xContext->getAccessibleIndexInParent();
        return S_OK;
    }
    catch (...)
    {
        return E_FAIL;
    }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTableCell::get_columnExtent(long* pColumnsSpanned)
{
    SolarMutexGuard g;

    try
    {
        if (pColumnsSpanned == nullptr)
            return E_INVALIDARG;

        if (!m_xTable.is())
            return E_FAIL;

        long nRow = 0, nColumn = 0;
        get_rowIndex(&nRow);
        get_columnIndex(&nColumn);

        *pColumnsSpanned = m_xTable->getAccessibleColumnExtentAt(nRow, nColumn);
        return S_OK;
    }
    catch (...)
    {
        return E_FAIL;
    }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTableCell::get_columnHeaderCells(IUnknown*** cellAccessibles,
                                                                       long* pColumnHeaderCellCount)
{
    SolarMutexGuard g;

    if (!cellAccessibles || !pColumnHeaderCellCount)
        return E_INVALIDARG;

    if (!m_xTable.is())
        return E_FAIL;

    Reference<XAccessibleTable> xHeaders = m_xTable->getAccessibleColumnHeaders();
    if (!xHeaders.is())
        return E_FAIL;

    const sal_Int32 nCount = xHeaders->getAccessibleRowCount();
    *pColumnHeaderCellCount = nCount;
    *cellAccessibles = static_cast<IUnknown**>(CoTaskMemAlloc(nCount * sizeof(IUnknown*)));
    assert(*cellAccessibles && "Don't handle OOM conditions");
    sal_Int32 nCol = 0;
    get_columnIndex(&nCol);
    for (sal_Int32 nRow = 0; nRow < nCount; nRow++)
    {
        Reference<XAccessible> xCell = xHeaders->getAccessibleCellAt(nRow, nCol);
        assert(xCell.is());

        IAccessible* pIAccessible = CMAccessible::get_IAccessibleFromXAccessible(xCell.get());
        if (!pIAccessible)
        {
            Reference<XAccessible> xTableAcc(m_xTable, UNO_QUERY);
            CMAccessible::g_pAccObjectManager->InsertAccObj(xCell.get(), xTableAcc.get());
            pIAccessible = CMAccessible::get_IAccessibleFromXAccessible(xCell.get());
        }
        assert(pIAccessible && "Couldn't retrieve IAccessible object for cell.");

        pIAccessible->AddRef();
        (*cellAccessibles)[nRow] = pIAccessible;
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTableCell::get_columnIndex(long* pColumnIndex)
{
    SolarMutexGuard g;

    try
    {
        if (pColumnIndex == nullptr)
            return E_INVALIDARG;

        if (!m_xTable.is())
            return E_FAIL;

        *pColumnIndex = m_xTable->getAccessibleColumn(m_nIndexInParent);
        return S_OK;
    }
    catch (...)
    {
        return E_FAIL;
    }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTableCell::get_rowExtent(long* pRowsSpanned)
{
    SolarMutexGuard g;

    try
    {
        if (pRowsSpanned == nullptr)
            return E_INVALIDARG;

        if (!m_xTable.is())
            return E_FAIL;

        long nRow = 0, nColumn = 0;
        get_rowIndex(&nRow);
        get_columnIndex(&nColumn);

        *pRowsSpanned = m_xTable->getAccessibleRowExtentAt(nRow, nColumn);

        return S_OK;
    }
    catch (...)
    {
        return E_FAIL;
    }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTableCell::get_rowHeaderCells(IUnknown*** cellAccessibles,
                                                                    long* pRowHeaderCellCount)
{
    SolarMutexGuard g;

    if (!cellAccessibles || !pRowHeaderCellCount)
        return E_INVALIDARG;

    if (!m_xTable.is())
        return E_FAIL;

    Reference<XAccessibleTable> xHeaders = m_xTable->getAccessibleRowHeaders();
    if (!xHeaders.is())
        return E_FAIL;

    const sal_Int32 nCount = xHeaders->getAccessibleColumnCount();
    *pRowHeaderCellCount = nCount;
    *cellAccessibles = static_cast<IUnknown**>(CoTaskMemAlloc(nCount * sizeof(IUnknown*)));
    assert(*cellAccessibles && "Don't handle OOM conditions");
    sal_Int32 nRow = 0;
    get_rowIndex(&nRow);
    for (sal_Int32 nCol = 0; nCol < nCount; nCol++)
    {
        Reference<XAccessible> xCell = xHeaders->getAccessibleCellAt(nRow, nCol);
        assert(xCell.is());

        IAccessible* pIAccessible = CMAccessible::get_IAccessibleFromXAccessible(xCell.get());
        if (!pIAccessible)
        {
            Reference<XAccessible> xTableAcc(m_xTable, UNO_QUERY);
            CMAccessible::g_pAccObjectManager->InsertAccObj(xCell.get(), xTableAcc.get());
            pIAccessible = CMAccessible::get_IAccessibleFromXAccessible(xCell.get());
        }
        assert(pIAccessible && "Couldn't retrieve IAccessible object for cell.");

        pIAccessible->AddRef();
        (*cellAccessibles)[nCol] = pIAccessible;
    }
    return S_OK;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTableCell::get_rowIndex(long* pRowIndex)
{
    SolarMutexGuard g;

    try
    {
        if (pRowIndex == nullptr)
            return E_INVALIDARG;

        if (!m_xTable.is())
            return E_FAIL;

        *pRowIndex = m_xTable->getAccessibleRow(m_nIndexInParent);
        return S_OK;
    }
    catch (...)
    {
        return E_FAIL;
    }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTableCell::get_isSelected(boolean* pIsSelected)
{
    SolarMutexGuard g;

    try
    {
        if (pIsSelected == nullptr)
            return E_INVALIDARG;

        if (!m_xTable.is())
            return E_FAIL;

        long nRow = 0, nColumn = 0;
        get_rowIndex(&nRow);
        get_columnIndex(&nColumn);

        *pIsSelected = m_xTable->isAccessibleSelected(nRow, nColumn);
        return S_OK;
    }
    catch (...)
    {
        return E_FAIL;
    }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTableCell::get_rowColumnExtents(long* pRow, long* pColumn,
                                                                      long* pRowExtents,
                                                                      long* pColumnExtents,
                                                                      boolean* pIsSelected)
{
    SolarMutexGuard g;

    if (!pRow || !pColumn || !pRowExtents || !pColumnExtents || !pIsSelected)
        return E_INVALIDARG;

    if (get_rowIndex(pRow) != S_OK)
        return E_FAIL;
    if (get_columnIndex(pColumn) != S_OK)
        return E_FAIL;
    if (get_rowExtent(pRowExtents) != S_OK)
        return E_FAIL;
    if (get_columnExtent(pColumnExtents) != S_OK)
        return E_FAIL;
    if (get_isSelected(pIsSelected) != S_OK)
        return E_FAIL;
    return S_OK;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccTableCell::get_table(IUnknown** ppTable)
{
    if (!ppTable)
        return E_INVALIDARG;

    if (!m_xTable.is())
        return E_FAIL;

    Reference<XAccessible> xAcc(m_xTable, UNO_QUERY);
    if (!xAcc.is())
        return E_FAIL;

    IAccessible* pRet = CMAccessible::get_IAccessibleFromXAccessible(xAcc.get());
    if (!pRet)
        return E_FAIL;

    *ppTable = pRet;
    pRet->AddRef();
    return S_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
