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
 * AccAction.cpp : Implementation of CAccAction
 */
#include "stdafx.h"
#include "UAccCOM2.h"
#include "AccAction.h"

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
 * Overide of IUNOXWrapper.
 *
 * @param    pXInterface    the pointer of UNO interface.
 */
STDMETHODIMP CAccAction::put_XInterface(long pXInterface)
{

    return CAccActionBase::put_XInterface(pXInterface);
}
/**
   * Put UNO interface.
   * @param pXSubInterface XAccessibleHyperlink interface.
   * @return Result.
*/
STDMETHODIMP CAccAction::put_XSubInterface(long pXSubInterface)
{


    pRXAct = (XAccessibleAction*)pXSubInterface;

    return S_OK;
}
