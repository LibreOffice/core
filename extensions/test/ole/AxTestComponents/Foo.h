/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
// Basic.h : Declaration of the CBasic

#ifndef __Foo_H_
#define __Foo_H_

#include "resource.h"       // main symbols
#import  "AxTestComponents.tlb" no_namespace no_implementation raw_interfaces_only named_guids

/////////////////////////////////////////////////////////////////////////////
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

#endif //__BASIC_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
