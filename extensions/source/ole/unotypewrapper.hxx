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

#include "wincrap.hxx"

#include "comifaces.hxx"

/* creates a UnoTypWrapper and sets the Name property to the value
   specified by sTypeName.
   Returns true if the object could be created and initialized.
 */
bool createUnoTypeWrapper(BSTR sTypeName, VARIANT * pVariant);
bool createUnoTypeWrapper(const OUString& sTypeName, VARIANT * pVar);

class UnoTypeWrapper:
      public CComObjectRootEx<CComMultiThreadModel>,
      public IUnoTypeWrapper,
      public IDispatch
{
public:
    UnoTypeWrapper();
    virtual ~UnoTypeWrapper();

    BEGIN_COM_MAP(UnoTypeWrapper)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IUnoTypeWrapper)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
    END_COM_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif

    // IDispatch -------------------------------------------
    STDMETHOD( GetTypeInfoCount)(UINT *pctinfo) override;

    STDMETHOD( GetTypeInfo)( UINT iTInfo,
                             LCID lcid,
                             ITypeInfo **ppTInfo) override;

    STDMETHOD( GetIDsOfNames)( REFIID riid,
                               LPOLESTR *rgszNames,
                               UINT cNames,
                               LCID lcid,
                               DISPID *rgDispId) override;

    STDMETHOD( Invoke)( DISPID dispIdMember,
                        REFIID riid,
                        LCID lcid,
                        WORD wFlags,
                        DISPPARAMS *pDispParams,
                        VARIANT *pVarResult,
                        EXCEPINFO *pExcepInfo,
                        UINT *puArgErr) override;
    // IUnoTypeWrapper --------------------------------------
    STDMETHOD(put_Name)(BSTR val) override;
    STDMETHOD(get_Name)(BSTR* pVal) override;

    CComBSTR m_sName;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
