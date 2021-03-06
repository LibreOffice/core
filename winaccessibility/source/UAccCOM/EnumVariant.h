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
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include <AccObjectManagerAgent.hxx>

#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnon-virtual-dtor"
#endif
#include  <UAccCOM.h>
#if defined __clang__
#pragma clang diagnostic pop
#endif

/**
 * CEnumVariant implements IEnumVARIANT interface.
 */
class ATL_NO_VTABLE CEnumVariant :
            public CComObjectRootEx<CComMultiThreadModel>,
            public CComCoClass<CEnumVariant, &CLSID_EnumVariant>,
            public IDispatchImpl<IEnumVariant, &IID_IEnumVariant, &LIBID_UACCCOMLib>
{
public:
    CEnumVariant()
            :m_lLBound(0),
            pUNOInterface(nullptr)
    {
        m_lCurrent = m_lLBound;
    }

    virtual ~CEnumVariant() {};

    DECLARE_NO_REGISTRY()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CEnumVariant)
    COM_INTERFACE_ENTRY(IEnumVariant)
    COM_INTERFACE_ENTRY(IEnumVARIANT)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
    END_COM_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif

    // IEnumVariant
public:

    STDMETHOD(ClearEnumeration)() override;

    // IEnumVARIANT


    HRESULT STDMETHODCALLTYPE Next(ULONG cElements,VARIANT __RPC_FAR *pvar,ULONG __RPC_FAR *pcElementFetched) override;


    HRESULT STDMETHODCALLTYPE Skip(ULONG cElements) override;


    HRESULT STDMETHODCALLTYPE Reset( void) override;


    HRESULT STDMETHODCALLTYPE Clone(IEnumVARIANT __RPC_FAR *__RPC_FAR *ppenum) override;

    // IEnumVariant


    HRESULT STDMETHODCALLTYPE PutSelection(hyper pXSelection) override;


    static HRESULT STDMETHODCALLTYPE Create(CEnumVariant __RPC_FAR *__RPC_FAR *ppenum);


    long GetCountOfElements();

private:

    long m_lCurrent;
    long m_lLBound;
    css::accessibility::XAccessible* pUNOInterface;
    css::uno::Reference<css::accessibility::XAccessibleSelection>
        m_pXAccessibleSelection;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
