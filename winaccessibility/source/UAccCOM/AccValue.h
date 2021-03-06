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

#include "Resource.h"           // main symbols

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/XAccessibleValue.hpp>
#include "UNOXWrapper.h"

/**
 * CAccValue implements IAccessibleValue interface.
 */
class CAccValue :
            public CComObjectRoot,
            public CComCoClass<CAccValue,&CLSID_AccValue>,
            public IAccessibleValue,
            public CUNOXWrapper
{
public:
    CAccValue()
    {
            }
    virtual ~CAccValue()
    {
            }

    BEGIN_COM_MAP(CAccValue)
    COM_INTERFACE_ENTRY(IAccessibleValue)
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
        return static_cast<CAccValue*>(pv)->SmartQI(iid,ppvObject);
    }

    HRESULT SmartQI(REFIID iid, void** ppvObject)
    {
        if( m_pOuterUnknown )
            return OuterQueryInterface(iid,ppvObject);
        return E_FAIL;
    }

    DECLARE_NO_REGISTRY()

public:
    // IAccessibleValue

    // Returns the value of this object as a number.
    STDMETHOD(get_currentValue)(VARIANT *currentValue) override;

    // Sets the value of this object to the given number.
    STDMETHOD(setCurrentValue)(VARIANT value) override;

    // Returns the maximal value that can be represented by this object.
    STDMETHOD(get_maximumValue)(VARIANT *maximumValue) override;

    // Returns the minimal value that can be represented by this object.
    STDMETHOD(get_minimumValue)(VARIANT *minimumValue) override;

    // Override of IUNOXWrapper.
    STDMETHOD(put_XInterface)(hyper pXInterface) override;

private:

    css::uno::Reference<css::accessibility::XAccessibleValue> pRXVal;

    css::accessibility::XAccessibleValue* GetXInterface()
    {
        return pRXVal.get();
    }

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
