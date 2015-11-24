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
#include "UAccCOM.h"
#include "AccValue.h"
#include "MAccessible.h"

#include <vcl/svapp.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

using namespace com::sun::star::accessibility;
using namespace com::sun::star::uno;

/**
   * Get current value.
   * @param  currentValue Variant that accepts current value.
   * @return Result.
   */

STDMETHODIMP CAccValue::get_currentValue(VARIANT * currentValue)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    if (currentValue == NULL)
        return E_INVALIDARG;
    if ( !pRXVal.is() )
        return E_FAIL;

    // Get Any type value from UNO.
    css::uno::Any  anyVal = GetXInterface()->getCurrentValue();
    // Convert Any to VARIANT.
    CMAccessible::ConvertAnyToVariant(anyVal, currentValue);

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Set current value.
   * @param  Value New value should be set.
   * @param  success If the method is successfully called.
   * @return Result.
   */
STDMETHODIMP CAccValue::setCurrentValue(VARIANT value)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    if ( !pRXVal.is() )
        return E_FAIL;

    HRESULT hRet = S_OK;
    css::uno::Any anyVal;

    // Set value according to value type.
    switch(value.vt)
    {
    case VT_UI1:
        {
            css::uno::Type     typeInfo(TypeClass_CHAR, (sal_Char *)"char");
            anyVal.setValue(&value.bVal, typeInfo);
        }
        break;

    case VT_BOOL:
        {
            css::uno::Type     typeInfo(TypeClass_BOOLEAN, (sal_Char *)"bool");
            anyVal.setValue(&value.boolVal, typeInfo);
        }
        break;

    case VT_I2:
        {
            css::uno::Type     typeInfo(TypeClass_SHORT, (sal_Char *)"short");
            anyVal.setValue(&value.iVal, typeInfo);
        }
        break;

    case VT_I4:
        {
            css::uno::Type     typeInfo(TypeClass_LONG, (sal_Char *)"long");
            anyVal.setValue(&value.lVal, typeInfo);
        }
        break;

    case VT_R4:
        {
            css::uno::Type     typeInfo(TypeClass_FLOAT, (sal_Char *)"float");
            anyVal.setValue(&value.fltVal, typeInfo);
        }
        break;

    case VT_R8:
        {
            css::uno::Type     typeInfo(TypeClass_DOUBLE, (sal_Char *)"double");
            anyVal.setValue(&value.dblVal, typeInfo);
        }
        break;

    default:
        {
            // Unsupport type conversion.
            hRet = E_FAIL;
        }
        break;
    }

    if(hRet == S_OK)
    {
        hRet = pRXVal->setCurrentValue(anyVal) ? S_OK : E_FAIL ;
    }

    return hRet;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Get maximum value.
   * @param  maximumValue Variant that accepts maximum value.
   * @return Result.
   */
STDMETHODIMP CAccValue::get_maximumValue(VARIANT *maximumValue)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    if (maximumValue == NULL)
        return E_INVALIDARG;
    if ( !pRXVal.is() )
        return E_FAIL;

    // Get Any type value from UNO.
    css::uno::Any  anyVal = GetXInterface()->getMaximumValue();
    // Convert Any to VARIANT.
    CMAccessible::ConvertAnyToVariant(anyVal, maximumValue);

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Get minimum value.
   * @param  mininumValue Variant that accepts minimum value.
   * @return Result.
   */
STDMETHODIMP CAccValue::get_minimumValue(VARIANT *mininumValue)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    if (mininumValue == NULL)
        return E_FAIL;
    if ( !pRXVal.is() )
        return E_FAIL;

    // Get Any type value from UNO.
    css::uno::Any  anyVal = GetXInterface()->getMinimumValue();
    // Convert Any to VARIANT.
    CMAccessible::ConvertAnyToVariant(anyVal, mininumValue);

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Put valid UNO interface into com class.
   * @param  pXInterface UNO interface.
   * @return Result.
   */
STDMETHODIMP CAccValue::put_XInterface(hyper pXInterface)
{
    // internal IUNOXWrapper - no mutex meeded

    ENTER_PROTECTED_BLOCK

    CUNOXWrapper::put_XInterface(pXInterface);
    //special query.
    if(pUNOInterface == NULL)
        return E_FAIL;
    Reference<XAccessibleContext> pRContext = pUNOInterface->getAccessibleContext();
    if( !pRContext.is() )
    {
        return E_FAIL;
    }
    Reference<XAccessibleValue> pRXI(pRContext,UNO_QUERY);
    if( !pRXI.is() )
        pRXVal = NULL;
    else
        pRXVal = pRXI.get();
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
