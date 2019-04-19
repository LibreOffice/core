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
// Basic.h : Declaration of the CBasic

#pragma once

#include "resource.h"
#import  "AxTestComponents.tlb" no_namespace no_implementation raw_interfaces_only named_guids


// CBasic
class ATL_NO_VTABLE CFoo :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CFoo, &CLSID_Foo>,
    public IFoo

//  public IDispatchImpl<IFoo, &__uuidof(IFoo), &LIBID_AXTESTCOMPONENTSLib, /* wMajor = */ 1, /* wMinor = */ 0>
{
public:
    CFoo();
    ~CFoo();

    DECLARE_REGISTRY_RESOURCEID(IDR_BASIC)

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CFoo)
        COM_INTERFACE_ENTRY(IFoo)
    END_COM_MAP()


STDMETHOD(Foo)(IUnknown* val);

    // IFoo Methods
public:
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
