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
// Simple.h : Declaration of the CSimple

#ifndef __SIMPLE_H_
#define __SIMPLE_H_

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
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

#endif //__SIMPLE_H_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
