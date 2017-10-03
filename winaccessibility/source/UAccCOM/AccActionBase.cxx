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

#include "AccessibleKeyStroke.h"

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
STDMETHODIMP CAccActionBase::nActions(/*[out,retval]*/long* nActions)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if( pRXAct.is() && nActions != nullptr )
    {
        *nActions = GetXInterface()->getAccessibleActionCount();
        return S_OK;
    }
    *nActions = 0;

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
 * Performs specified action on the object.
 *
 * @param    actionIndex    the index of action.
 */
STDMETHODIMP CAccActionBase::doAction(/* [in] */ long actionIndex)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    if( pRXAct.is() )
    {
        return GetXInterface()->doAccessibleAction( actionIndex )?S_OK:E_FAIL;
    }
    return E_FAIL;

    LEAVE_PROTECTED_BLOCK
}

/**
 * Gets description of specified action.
 *
 * @param    actionIndex    the index of action.
 * @param    description    the description string of the specified action.
 */
STDMETHODIMP CAccActionBase::get_description(long actionIndex,BSTR __RPC_FAR *description)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(description == nullptr)
        return E_INVALIDARG;

    // #CHECK XInterface#
    if(!pRXAct.is())
        return E_FAIL;

    ::rtl::OUString ouStr = GetXInterface()->getAccessibleActionDescription(actionIndex);
    // #CHECK#

    SAFE_SYSFREESTRING(*description);
    *description = SysAllocString(SAL_W(ouStr.getStr()));

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

STDMETHODIMP CAccActionBase::get_name( long, BSTR __RPC_FAR *)
{
    return E_NOTIMPL;
}

STDMETHODIMP CAccActionBase::get_localizedName( long, BSTR __RPC_FAR *)
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
STDMETHODIMP CAccActionBase::get_keyBinding(
    /* [in] */ long actionIndex,
    /* [in] */ long,
    /* [length_is][length_is][size_is][size_is][out] */ BSTR __RPC_FAR *__RPC_FAR *keyBinding,
    /* [retval][out] */ long __RPC_FAR *nBinding)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    if( !keyBinding || !nBinding)
        return E_INVALIDARG;

    if( !pRXAct.is() )
        return E_FAIL;

    Reference< XAccessibleKeyBinding > binding = GetXInterface()->getAccessibleActionKeyBinding(actionIndex);
    if( !binding.is() )
        return E_FAIL;

    long nCount = (binding.get())->getAccessibleKeyBindingCount();

    *keyBinding = static_cast<BSTR*>(::CoTaskMemAlloc(nCount*sizeof(BSTR)));

    // #CHECK Memory Allocation#
    if(*keyBinding == nullptr)
        return E_FAIL;

    for( int index = 0;index < nCount;index++ )
    {
        auto const wString = GetkeyBindingStrByXkeyBinding( (binding.get())->getAccessibleKeyBinding(index) );

        (*keyBinding)[index] = SysAllocString(SAL_W(wString.getStr()));
    }

    *nBinding = nCount;
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
 * Override of IUNOXWrapper.
 *
 * @param    pXInterface    the pointer of UNO interface.
 */
STDMETHODIMP CAccActionBase::put_XInterface(hyper pXInterface)
{
    // internal IUNOXWrapper - no mutex meeded

    ENTER_PROTECTED_BLOCK

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

    LEAVE_PROTECTED_BLOCK
}

/**
 * Helper function used for converting keybinding to string.
 *
 * @param    keySet    the key stroke sequence.
 */
OUString CAccActionBase::GetkeyBindingStrByXkeyBinding( const Sequence< KeyStroke > &keySet )
{
    OUStringBuffer buf;
    for( int iIndex = 0;iIndex < keySet.getLength();iIndex++ )
    {
        KeyStroke stroke = keySet[iIndex];
        buf.append('\n');
        buf.append(stroke.KeyChar);
    }
    return buf.makeStringAndClear();
}

/**
 * Helper function used for converting key code to ole string.
 *
 * @param    key    the key code.
 */
OLECHAR const * CAccActionBase::getOLECHARFromKeyCode(long key)
{
    static struct keyMap
    {
        int keyCode;
        OLECHAR const * key;
    }
    map[] =
        {
#define CODEENTRY(key)   {KEYCODE_##key, L#key}
            {MODIFIER_SHIFT, L"SHIFT" },
            {MODIFIER_CTRL, L"CTRL" },
            {MODIFIER_ALT, L"ALT" },
            CODEENTRY(NUM0),CODEENTRY(NUM1),CODEENTRY(NUM2),CODEENTRY(NUM3),CODEENTRY(NUM4),CODEENTRY(NUM5),
            CODEENTRY(NUM6),CODEENTRY(NUM7),CODEENTRY(NUM8),CODEENTRY(NUM9),
            CODEENTRY(A),CODEENTRY(B),CODEENTRY(C),CODEENTRY(D),CODEENTRY(E),CODEENTRY(F),
            CODEENTRY(G),CODEENTRY(H),CODEENTRY(I),CODEENTRY(J),CODEENTRY(K),CODEENTRY(L),
            CODEENTRY(M),CODEENTRY(N),CODEENTRY(O),CODEENTRY(P),CODEENTRY(Q),CODEENTRY(R),
            CODEENTRY(S),CODEENTRY(T),CODEENTRY(U),CODEENTRY(V),CODEENTRY(W),CODEENTRY(X),
            CODEENTRY(Y),CODEENTRY(Z),
            CODEENTRY(F1),CODEENTRY(F2),CODEENTRY(F3),CODEENTRY(F4),CODEENTRY(F5),CODEENTRY(F6),
            CODEENTRY(F7),CODEENTRY(F8),CODEENTRY(F9),CODEENTRY(F10),CODEENTRY(F11),CODEENTRY(F12),
            CODEENTRY(F13),CODEENTRY(F14),CODEENTRY(F15),CODEENTRY(F16),CODEENTRY(F17),CODEENTRY(F18),
            CODEENTRY(F19),CODEENTRY(F20),CODEENTRY(F21),CODEENTRY(F22),CODEENTRY(F23),CODEENTRY(F24),
            CODEENTRY(F25),CODEENTRY(F26),

            { KEYCODE_DOWN, L"DOWN" },
            { KEYCODE_UP, L"UP" },
            { KEYCODE_LEFT, L"LEFT" },
            { KEYCODE_RIGHT, L"RIGHT" },
            { KEYCODE_HOME, L"HOME" },
            { KEYCODE_END, L"END" },
            { KEYCODE_PAGEUP, L"PAGEUP" },
            { KEYCODE_PAGEDOWN, L"PAGEDOWN" },
            { KEYCODE_RETURN, L"RETURN" },
            { KEYCODE_ESCAPE, L"ESCAPE" },
            { KEYCODE_TAB, L"TAB" },
            { KEYCODE_BACKSPACE, L"BACKSPACE" },
            { KEYCODE_SPACE, L"SPACE" },
            { KEYCODE_INSERT, L"INSERT" },
            { KEYCODE_DELETE, L"DELETE" },
            { KEYCODE_ADD, L"ADD" },
            { KEYCODE_SUBTRACT, L"SUBTRACT" },
            { KEYCODE_MULTIPLY, L"MULTIPLY" },
            { KEYCODE_DIVIDE, L"DIVIDE" },
            { KEYCODE_POINT, L"POINT" },
            { KEYCODE_COMMA, L"COMMA" },
            { KEYCODE_LESS, L"LESS" },
            { KEYCODE_GREATER, L"GREATER" },
            { KEYCODE_EQUAL, L"EQUAL" },
            { KEYCODE_OPEN, L"OPEN" },
            { KEYCODE_CUT, L"CUT" },
            { KEYCODE_COPY, L"COPY" },
            { KEYCODE_PASTE, L"PASTE" },
            { KEYCODE_UNDO, L"UNDO" },
            { KEYCODE_REPEAT, L"REPEAT" },
            { KEYCODE_FIND, L"FIND" },
            { KEYCODE_PROPERTIES, L"PROPERTIES" },
            { KEYCODE_FRONT, L"FRONT" },
            { KEYCODE_CONTEXTMENU, L"CONTEXTMENU" },
            { KEYCODE_HELP, L"HELP" },
        };
    static long nCount = SAL_N_ELEMENTS(map);

    long min = 0;
    long max = nCount-1;
    long mid = nCount/2;
    while(min<max)
    {
        if(key<map[mid].keyCode)
            max = mid-1;
        else if(key>map[mid].keyCode)
            min = mid+1;
        else
            break;
        mid = (min+max)/2;
    }

    if(key == map[mid].keyCode)
    {
        return map[mid].key;
    }
    else
    {
        return nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
