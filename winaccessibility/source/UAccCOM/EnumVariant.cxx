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

#include "stdafx.h"
#include  <UAccCOM.h>
#include "EnumVariant.h"
#include "MAccessible.h"

#include <sal/log.hxx>
#include <vcl/svapp.hxx>

using namespace com::sun::star::uno;
using namespace com::sun::star::accessibility;


// CEnumVariant


/**
   * enumerate method,get next element
   * @param  cElements The number of elements to be returned.
   * @param  pvar      An array of at least size celt in which the elements are to be returned.
   * @param  pcElementFetched Pointer to the number of elements returned in rgVar, or Null
   * @return Result.
   */
HRESULT STDMETHODCALLTYPE CEnumVariant::Next(ULONG cElements,VARIANT __RPC_FAR *pvar,ULONG __RPC_FAR *pcElementFetched)
{
    SolarMutexGuard g;

    ULONG l2;

    if (pvar == nullptr)
        return E_INVALIDARG;

    if (pcElementFetched != nullptr)
        *pcElementFetched = 0;

    sal_Int64 nChildCount = m_pXAccessibleSelection->getSelectedAccessibleChildCount();
    if (nChildCount > std::numeric_limits<long>::max())
    {
        SAL_WARN("iacc2", "CEnumVariant::Next: Child count exceeds maximum long value, "
                          "using max long.");
        nChildCount = std::numeric_limits<long>::max();
    }

    // Retrieve the next cElements.
    sal_Int64 l1;
    for (l1 = m_nCurrent, l2 = 0; l1 < nChildCount && l2 < cElements; l1++, l2++)
    {
        Reference< XAccessible > pRXAcc = m_pXAccessibleSelection->getSelectedAccessibleChild(l1);
        IAccessible* pChild = nullptr;
        bool isGet = CMAccessible::get_IAccessibleFromXAccessible(pRXAcc.get(),
                        &pChild);
        if(isGet)
        {
            pvar[l2].vt = VT_DISPATCH;
            pvar[l2].pdispVal = pChild;
            pChild->AddRef();
        }
        else if(pRXAcc.is())
        {
            if(CMAccessible::g_pAgent)
                CMAccessible::g_pAgent->InsertAccObj(pRXAcc.get(),pUNOInterface);
            isGet = CMAccessible::get_IAccessibleFromXAccessible(
                            pRXAcc.get(), &pChild);
            if(isGet)
            {
                pvar[l2].vt = VT_DISPATCH;
                pvar[l2].pdispVal = pChild;
                pChild->AddRef();
            }
        }
    }
    // Set count of elements retrieved.
    if (pcElementFetched != nullptr)
        *pcElementFetched = l2;
    m_nCurrent = l1;

    return (l2 < cElements) ? S_FALSE : NOERROR;
}

/**
   * skip the elements in the given range when enumerate elements
   * @param  cElements The number of elements to skip.
   * @return Result.
   */
HRESULT STDMETHODCALLTYPE CEnumVariant::Skip(ULONG cElements)
{
    SolarMutexGuard g;

    m_nCurrent += cElements;
    sal_Int64 nChildCount = m_pXAccessibleSelection->getSelectedAccessibleChildCount();
    if (nChildCount > std::numeric_limits<long>::max())
    {
        SAL_WARN("iacc2", "CEnumVariant::Skip: Child count exceeds maximum long value, "
                          "using max long.");
        nChildCount = std::numeric_limits<long>::max();
    }
    if (m_nCurrent > nChildCount)
    {
        m_nCurrent = nChildCount;
        return E_FAIL;
    }
    else
        return NOERROR;
}


/**
   * reset the enumaration position to initial value
   * @param
   * @return Result.
   */
HRESULT STDMETHODCALLTYPE CEnumVariant::Reset()
{
    SolarMutexGuard g;

    m_nCurrent = 0;
    return NOERROR;
}


/**
   *create a new IEnumVariant object,
   *copy current enumaration container and its state to
   *the new object
   *AT will use the copy object to get elements
   * @param ppenum On return, pointer to the location of the clone enumerator
   * @return Result.
   */
HRESULT STDMETHODCALLTYPE CEnumVariant::Clone(IEnumVARIANT __RPC_FAR *__RPC_FAR *ppenum)
{
    SolarMutexGuard g;

    CEnumVariant * penum = nullptr;
    HRESULT hr;
    if (ppenum == nullptr)
        return E_INVALIDARG;

    *ppenum = nullptr;

    hr = Create(&penum);
    if( hr == S_OK )
    {
        penum->PutSelection(reinterpret_cast<hyper>(pUNOInterface));
        *ppenum = penum;
    }
    else
    {
        if (penum)
            penum->Release();
    }
    return hr;
}

/**
   *Static public method to create a CLSID_EnumVariant com object.
   * @param ppenum Pointer to accept com object.
   * @return Result.
   */
HRESULT STDMETHODCALLTYPE CEnumVariant::Create(CEnumVariant __RPC_FAR *__RPC_FAR *ppenum)
{
    SolarMutexGuard g;

    HRESULT hr = createInstance<CEnumVariant>(IID_IEnumVariant, ppenum);
    if (S_OK != hr)
    {
        return E_FAIL;
    }

    return S_OK;
}

/**
   *Return count of elements in current container
   * @param.
   * @return count of elements in current container.
   */
long CEnumVariant::GetCountOfElements()
{
    if(m_pXAccessibleSelection.is())
    {
        sal_Int64 nCount = m_pXAccessibleSelection->getSelectedAccessibleChildCount();
        if (nCount > std::numeric_limits<long>::max())
        {
            SAL_WARN("iacc2", "CEnumVariant::GetCountOfElements: Count exceeds maximum long value, "
                              "using max long.");
            nCount = std::numeric_limits<long>::max();
        }
        return nCount;
    }
    return 0;
}

/**
   * Set member m_pXAccessibleSelection to NULL and m_nCurrent to 0.
   * @param.
   * @return Result
   */
COM_DECLSPEC_NOTHROW STDMETHODIMP CEnumVariant::ClearEnumeration()
{
    // internal IEnumVariant - no mutex meeded

    pUNOInterface = nullptr;
    m_pXAccessibleSelection = nullptr;
    m_nCurrent = 0;
    return S_OK;
}

/**
   *Static method to fetch XAccessibleSelection
   * @param pXAcc XAccessible interface.
   * @return XAccessibleSelection interface.
   */
static Reference<XAccessibleSelection> GetXAccessibleSelection(XAccessible* pXAcc)
{
    if( pXAcc == nullptr)
        return nullptr;

    Reference< XAccessibleContext > pRContext = pXAcc->getAccessibleContext();
    if( !pRContext.is() )
        return nullptr;

    Reference< XAccessibleSelection > pRSelection(pRContext,UNO_QUERY);
    if( !pRSelection.is() )
        return nullptr;

    return pRSelection;
}

/**
   * Put valid UNO XAccessible interface.
   * @param pXSelection XAccessible interface.
   * @return Result...
   */
STDMETHODIMP CEnumVariant::PutSelection(hyper pXSelection)
{
    // internal IEnumVariant - no mutex meeded

    pUNOInterface = reinterpret_cast<XAccessible*>(pXSelection);
    m_pXAccessibleSelection = GetXAccessibleSelection(pUNOInterface);
    return S_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
