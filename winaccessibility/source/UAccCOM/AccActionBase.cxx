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


// AccActionBase.cpp: implementation of the CAccActionBase class.

#include "stdafx.h"

#include "AccActionBase.h"
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>

#include <vcl/svapp.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <comphelper/AccessibleImplementationHelper.hxx>

#include "acccommon.h"

using namespace com::sun::star::accessibility::AccessibleRole;
using namespace com::sun::star::accessibility;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;


// Construction/Destruction


CAccActionBase::CAccActionBase()
{}

CAccActionBase::~CAccActionBase()
{}

/**
 * Returns the number of action.
 *
 * @param    nActions    the number of action.
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccActionBase::nActions(/*[out,retval]*/long* nActions)
{
    SolarMutexGuard g;

    if (!nActions)
        return E_INVALIDARG;

    try {

    if (pRXAct.is())
    {
        *nActions = pRXAct->getAccessibleActionCount();
        return S_OK;
    }

    *nActions = 0;
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
 * Performs specified action on the object.
 *
 * @param    actionIndex    the index of action.
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccActionBase::doAction(/* [in] */ long actionIndex)
{
    SolarMutexGuard g;

    try {

    if( pRXAct.is() )
    {
        return pRXAct->doAccessibleAction(actionIndex) ? S_OK : E_FAIL;
    }
    return E_FAIL;

    } catch(...) { return E_FAIL; }
}

/**
 * Gets description of specified action.
 *
 * @param    actionIndex    the index of action.
 * @param    description    the description string of the specified action.
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccActionBase::get_description(long actionIndex,BSTR __RPC_FAR *description)
{
    SolarMutexGuard g;

    try {

    if(description == nullptr)
        return E_INVALIDARG;

    if(!pRXAct.is())
        return E_FAIL;

    OUString ouStr = pRXAct->getAccessibleActionDescription(actionIndex);

    SysFreeString(*description);
    *description = SysAllocString(o3tl::toW(ouStr.getStr()));

    return S_OK;

    } catch(...) { return E_FAIL; }
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccActionBase::get_name( long, BSTR __RPC_FAR *)
{
    return E_NOTIMPL;
}

COM_DECLSPEC_NOTHROW STDMETHODIMP CAccActionBase::get_localizedName( long, BSTR __RPC_FAR *)
{
    return E_NOTIMPL;
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
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccActionBase::get_keyBinding(
    /* [in] */ long actionIndex,
    /* [in] */ long,
    /* [length_is][length_is][size_is][size_is][out] */ BSTR __RPC_FAR *__RPC_FAR *keyBinding,
    /* [retval][out] */ long __RPC_FAR *nBinding)
{
    SolarMutexGuard g;

    try {

    if( !keyBinding || !nBinding)
        return E_INVALIDARG;

    if( !pRXAct.is() )
        return E_FAIL;

    Reference< XAccessibleKeyBinding > binding = pRXAct->getAccessibleActionKeyBinding(actionIndex);
    if( !binding.is() )
        return E_FAIL;

    sal_Int32 nCount = binding->getAccessibleKeyBindingCount();

    *keyBinding = static_cast<BSTR*>(::CoTaskMemAlloc(nCount*sizeof(BSTR)));

    // #CHECK Memory Allocation#
    if(*keyBinding == nullptr)
        return E_FAIL;

    for( sal_Int32 index = 0;index < nCount;index++ )
    {
        auto const wString = comphelper::GetkeyBindingStrByXkeyBinding(
            binding->getAccessibleKeyBinding(index));

        (*keyBinding)[index] = SysAllocString(o3tl::toW(wString.getStr()));
    }

    *nBinding = nCount;
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/**
 * Override of IUNOXWrapper.
 *
 * @param    pXInterface    the pointer of UNO interface.
 */
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccActionBase::put_XInterface(hyper pXInterface)
{
    // internal IUNOXWrapper - no mutex meeded

    try {

    CUNOXWrapper::put_XInterface(pXInterface);

    //special query.
    if(pUNOInterface == nullptr)
        return E_FAIL;
    Reference<XAccessibleContext> pRContext = pUNOInterface->getAccessibleContext();
    if( !pRContext.is() )
        return E_FAIL;

    Reference<XAccessibleAction> pRXI(pRContext,UNO_QUERY);
    if( !pRXI.is() )
        pRXAct = nullptr;
    else
        pRXAct = pRXI.get();
    return S_OK;

    } catch(...) { return E_FAIL; }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
