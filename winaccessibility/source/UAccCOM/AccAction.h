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
#include "AccActionBase.h"

/**
 * CAccAction implements IAccessibleAction interface.
 */
class ATL_NO_VTABLE CAccAction :
            public CComObjectRoot,
            public CComCoClass<CAccAction, &CLSID_AccAction>,
            public IAccessibleAction,
            public CAccActionBase
{
public:
    CAccAction()
    {

    }

    DECLARE_NO_REGISTRY()

    BEGIN_COM_MAP(CAccAction)
    COM_INTERFACE_ENTRY(IAccessibleAction)
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
        return static_cast<CAccAction*>(pv)->SmartQI(iid,ppvObject);
    }

    HRESULT SmartQI(REFIID iid, void** ppvObject)
    {
        if( m_pOuterUnknown )
            return OuterQueryInterface(iid,ppvObject);
        return E_FAIL;
    }

    // IAccessibleAction
public:
    // IAccessibleAction

    // Returns the number of action.
    STDMETHOD(nActions)(/*[out,retval]*/long* nActions) override;

    // Performs specified action on the object.
    STDMETHOD(doAction)(/* [in] */ long actionIndex) override;

    // Gets description of specified action.
    STDMETHOD(get_description)(long actionIndex,BSTR __RPC_FAR *description) override;

    // added , 2006/06/28, for driver 07/11
    // get the action name
    STDMETHOD(get_name)( long actionIndex, BSTR __RPC_FAR *name) override;

    // get the localized action name
    STDMETHOD(get_localizedName)( long actionIndex, BSTR __RPC_FAR *localizedName) override;

    // Returns key binding object (if any) associated with specified action
    // key binding is string.
    // e.g. "alt+d" (like IAccessible::get_accKeyboardShortcut).
    STDMETHOD(get_keyBinding)(
        /* [in] */ long actionIndex,
        /* [in] */ long nMaxBinding,
        /* [length_is][length_is][size_is][size_is][out] */ BSTR __RPC_FAR *__RPC_FAR *keyBinding,
        /* [retval][out] */ long __RPC_FAR *nBinding) override;

    // Override of IUNOXWrapper.
    STDMETHOD(put_XSubInterface)(hyper pXSubInterface) override;

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
