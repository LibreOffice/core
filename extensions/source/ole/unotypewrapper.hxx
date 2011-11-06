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


#ifndef UNO_TYPE_WRAPPER_HXX
#define UNO_TYPE_WRAPPER_HXX

#pragma warning (push,1)
#pragma warning (disable:4548)

#include <tools/presys.h>
//#include "stdafx.h"
#define STRICT
#define _WIN32_WINNT 0x0400
#define _WIN32_DCOM
#if OSL_DEBUG_LEVEL > 0
//#define _ATL_DEBUG_INTERFACES
#endif
#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
#include <tools/postsys.h>

#pragma warning (pop)
#pragma warning (disable:4505)
    // disable "unreferenced local function has been removed" globally

#include "comifaces.hxx"

/* creates an UnoTypWrapper and sets the Name property to the value
   specified by sTypeName.
   Returns true if the object could be created and initialized.
 */
bool createUnoTypeWrapper(BSTR sTypeName, VARIANT * pVariant);
bool createUnoTypeWrapper(const rtl::OUString& sTypeName, VARIANT * pVar);

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
    END_COM_MAP()

    // IDispatch -------------------------------------------
    STDMETHOD( GetTypeInfoCount)(UINT *pctinfo);

    STDMETHOD( GetTypeInfo)( UINT iTInfo,
                             LCID lcid,
                             ITypeInfo **ppTInfo);

    STDMETHOD( GetIDsOfNames)( REFIID riid,
                               LPOLESTR *rgszNames,
                               UINT cNames,
                               LCID lcid,
                               DISPID *rgDispId);

    STDMETHOD( Invoke)( DISPID dispIdMember,
                        REFIID riid,
                        LCID lcid,
                        WORD wFlags,
                        DISPPARAMS *pDispParams,
                        VARIANT *pVarResult,
                        EXCEPINFO *pExcepInfo,
                        UINT *puArgErr);
    // IUnoTypeWrapper --------------------------------------
    STDMETHOD(put_Name)(BSTR val);
    STDMETHOD(get_Name)(BSTR* pVal);

    CComBSTR m_sName;
};

#endif
