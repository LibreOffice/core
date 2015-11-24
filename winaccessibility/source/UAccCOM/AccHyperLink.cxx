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
#include "AccHyperLink.h"

#include <vcl/svapp.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleAction.hpp>
#include "MAccessible.h"

using namespace com::sun::star::accessibility;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;

/**
 * Returns the number of action.
 *
 * @param    nActions    the number of action.
 */
STDMETHODIMP CAccHyperLink::nActions(/*[out,retval]*/long* nActions)
{

    return CAccActionBase::nActions(nActions);
}

/**
 * Performs specified action on the object.
 *
 * @param    actionIndex    the index of action.
 */
STDMETHODIMP CAccHyperLink::doAction(/* [in] */ long actionIndex)
{

    return CAccActionBase::doAction(actionIndex);
}

/**
 * Gets description of specified action.
 *
 * @param    actionIndex    the index of action.
 * @param    description    the description string of the specified action.
 */
STDMETHODIMP CAccHyperLink::get_description(long actionIndex,BSTR __RPC_FAR *description)
{

    return CAccActionBase::get_description(actionIndex, description);
}

STDMETHODIMP CAccHyperLink::get_name( long actionIndex, BSTR __RPC_FAR *name)
{

    return CAccActionBase::get_name(actionIndex, name);
}

STDMETHODIMP CAccHyperLink::get_localizedName( long actionIndex, BSTR __RPC_FAR *localizedName)
{

    return CAccActionBase::get_name(actionIndex, localizedName);
}

/**
 * Returns key binding object (if any) associated with specified action
 * key binding is string.
 * e.g. "alt+d" (like IAccessible::get_accKeyboardShortcut).
 *
 * @param    actionIndex    the index of action.
 * @param    nMaxBinding    the max number of key binding.
 * @param    keyBinding     the key binding array.
 * @param    nBinding       the actual number of key binding returned.
 */
STDMETHODIMP CAccHyperLink::get_keyBinding(
    /* [in] */ long actionIndex,
    /* [in] */ long nMaxBinding,
    /* [length_is][length_is][size_is][size_is][out] */ BSTR __RPC_FAR *__RPC_FAR *keyBinding,
    /* [retval][out] */ long __RPC_FAR *nBinding)
{

    return CAccActionBase::get_keyBinding(actionIndex, nMaxBinding, keyBinding, nBinding);
}

/**
   * get an object
   * @param
   * @return Result.
*/
STDMETHODIMP CAccHyperLink::get_anchor(/* [in] */ long index,
        /* [retval][out] */ VARIANT __RPC_FAR *anchor)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(anchor == NULL)
    {
        return E_INVALIDARG;
    }
    // #CHECK XInterface#
    if(!pRXLink.is())
    {
        return E_FAIL;
    }
    // Get Any type value via pRXLink.
    css::uno::Any  anyVal = GetXInterface()->getAccessibleActionAnchor(index);
    // Convert Any to VARIANT.
    CMAccessible::ConvertAnyToVariant(anyVal, anchor);

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * get an object
   * @param
   * @return Result.
*/
STDMETHODIMP CAccHyperLink::get_anchorTarget(/* [in] */ long index,
        /* [retval][out] */ VARIANT __RPC_FAR *anchorTarget)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(anchorTarget == NULL)
    {
        return E_INVALIDARG;
    }
    // #CHECK XInterface#
    if(!pRXLink.is())
    {
        return E_FAIL;
    }
    // Get Any type value via pRXLink.
    css::uno::Any  anyVal = GetXInterface()->getAccessibleActionObject(index);
    // Convert Any to VARIANT.
    CMAccessible::ConvertAnyToVariant(anyVal, anchorTarget);

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}


/**
   * Get start index.
   * @param index Variant to get start index.
   * @return Result.
*/
STDMETHODIMP CAccHyperLink::get_startIndex(/* [retval][out] */ long __RPC_FAR *index)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(index == NULL)
    {
        return E_INVALIDARG;
    }
    *index = GetXInterface()->getStartIndex();

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Get start index.
   * @param index Variant to get end index.
   * @return Result.
*/
STDMETHODIMP CAccHyperLink::get_endIndex(/* [retval][out] */ long __RPC_FAR *index)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(index == NULL)
    {
        return E_INVALIDARG;
    }
    // #CHECK XInterface#
    if(!pRXLink.is())
    {
        return E_FAIL;
    }
    *index = GetXInterface()->getEndIndex();

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Judge if the hyperlink is valid.
   * @param valid Variant to get validity.
   * @return Result.
*/
STDMETHODIMP CAccHyperLink::get_valid(/* [retval][out] */ boolean __RPC_FAR *valid)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(valid == NULL)
    {
        return E_INVALIDARG;
    }
    // #CHECK XInterface#
    if(!pRXLink.is())
    {
        return E_FAIL;
    }
    *valid = GetXInterface()->isValid();

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Put UNO interface.
   * @param pXInterface XAccessibleContext interface.
   * @return Result.
*/
STDMETHODIMP CAccHyperLink::put_XInterface(hyper pXInterface)
{
    // internal IUNOXWrapper - no mutex meeded

    ENTER_PROTECTED_BLOCK

    CAccActionBase::put_XInterface(pXInterface);
    //special query.
    if(pUNOInterface != NULL)
    {
        Reference<XAccessibleContext> pRContext = pUNOInterface->getAccessibleContext();
        if( !pRContext.is() )
        {
            return E_FAIL;
        }
        Reference<XAccessibleHyperlink> pRXI(pRContext,UNO_QUERY);
        if( !pRXI.is() )
        {
            pRXLink = NULL;
        }
        else
            pRXLink = pRXI.get();
    }
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Put UNO interface.
   * @param pXSubInterface XAccessibleHyperlink interface.
   * @return Result.
*/
STDMETHODIMP CAccHyperLink::put_XSubInterface(hyper pXSubInterface)
{
    // internal IUNOXWrapper - no mutex meeded

    pRXLink = reinterpret_cast<XAccessibleHyperlink*>(pXSubInterface);
    pRXAct = reinterpret_cast<XAccessibleAction*>(pXSubInterface);

    return S_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
