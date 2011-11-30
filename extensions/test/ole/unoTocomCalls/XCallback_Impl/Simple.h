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

