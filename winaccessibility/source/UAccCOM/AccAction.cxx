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
 * AccAction.cpp : Implementation of CAccAction
 */
#include "stdafx.h"
#include "AccAction.h"

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
#include  "UAccCOM.h"
#if defined __clang__
#pragma clang diagnostic pop
#endif

using namespace com::sun::star::accessibility;
using namespace com::sun::star::uno;

/**
 * Returns the number of action.
 *
 * @param    nActions    the number of action.
 */
STDMETHODIMP CAccAction::nActions(/*[out,retval]*/long* nActions)
{

    return CAccActionBase::nActions(nActions);
}

/**
 * Performs specified action on the object.
 *
 * @param    actionIndex    the index of action.
 */
STDMETHODIMP CAccAction::doAction(/* [in] */ long actionIndex)
{

    return CAccActionBase::doAction(actionIndex);
}

/**
 * Gets description of specified action.
 *
 * @param    actionIndex    the index of action.
 * @param    description    the description string of the specified action.
 */
STDMETHODIMP CAccAction::get_description(long actionIndex,BSTR __RPC_FAR *description)
{

    return CAccActionBase::get_description(actionIndex, description);
}

STDMETHODIMP CAccAction::get_name( long actionIndex, BSTR __RPC_FAR *name)
{

    return CAccActionBase::get_name(actionIndex, name);
}

STDMETHODIMP CAccAction::get_localizedName( long actionIndex, BSTR __RPC_FAR *localizedName)
{

    return CAccActionBase::get_localizedName(actionIndex, localizedName);
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
STDMETHODIMP CAccAction::get_keyBinding(
    /* [in] */ long actionIndex,
    /* [in] */ long nMaxBinding,
    /* [length_is][length_is][size_is][size_is][out] */ BSTR __RPC_FAR *__RPC_FAR *keyBinding,
    /* [retval][out] */ long __RPC_FAR *nBinding)
{

    return CAccActionBase::get_keyBinding(actionIndex, nMaxBinding, keyBinding, nBinding);
}

/**
 * Override of IUNOXWrapper.
 *
 * @param    pXInterface    the pointer of UNO interface.
 */
STDMETHODIMP CAccAction::put_XInterface(hyper pXInterface)
{

    return CAccActionBase::put_XInterface(pXInterface);
}
/**
   * Put UNO interface.
   * @param pXSubInterface XAccessibleHyperlink interface.
   * @return Result.
*/
STDMETHODIMP CAccAction::put_XSubInterface(hyper pXSubInterface)
{


    pRXAct = reinterpret_cast<XAccessibleAction*>(pXSubInterface);

    return S_OK;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
