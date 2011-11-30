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


// Basic.h : Declaration of the CBasic

#ifndef __Foo_H_
#define __Foo_H_

#include "resource.h"       // main symbols
//#include "AxTestComponents.h"
#import  "AxTestComponents.tlb" no_namespace no_implementation raw_interfaces_only named_guids
//#include "AxTestComponents.h"
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

