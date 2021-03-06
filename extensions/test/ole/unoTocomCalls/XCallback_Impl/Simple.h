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
// Simple.h : Declaration of the CSimple

#pragma once

#include "resource.h"


// CSimple
class ATL_NO_VTABLE CSimple :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CSimple, &CLSID_Simple>,
    public IDispatchImpl<ISimple, &IID_ISimple, &LIBID_XCALLBACK_IMPLLib>
{
public:
    CSimple()
    {
    }

DECLARE_REGISTRY_RESOURCEID(IDR_SIMPLE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSimple)
    COM_INTERFACE_ENTRY(ISimple)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

// ISimple
public:

    STDMETHOD(get__implementedInterfaces)(/*[out, retval]*/ LPSAFEARRAY *pVal);
    STDMETHOD(func3)(/*[in]*/ BSTR message);
    STDMETHOD(func2)(/*[in]*/ BSTR message);
    STDMETHOD(func)( BSTR message);
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
