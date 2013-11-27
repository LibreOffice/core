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

#ifndef __ENUMVARIANT_H_
#define __ENUMVARIANT_H_

#define WNT

#include "resource.h"       // main symbols
#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleSelection.hpp>
#include "UAccCOM2.h"
#include <AccObjectManagerAgent.hxx>

using namespace com::sun::star::accessibility;
using namespace com::sun::star::uno;

#include <vector>
#include <algorithm>
using namespace std;

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
            pUNOInterface(NULL),
            m_pXAccessibleSelection(NULL)
    {
        m_lCurrent = m_lLBound;
    }

    virtual ~CEnumVariant() {};

    DECLARE_REGISTRY_RESOURCEID(IDR_ENUMVARIANT)

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CEnumVariant)
    COM_INTERFACE_ENTRY(IEnumVariant)
    COM_INTERFACE_ENTRY(IEnumVARIANT)
    END_COM_MAP()

    // IEnumVariant
public:

    STDMETHOD(ClearEnumeration)();

    // IEnumVARIANT

    //
    HRESULT STDMETHODCALLTYPE Next(ULONG cElements,VARIANT __RPC_FAR *pvar,ULONG __RPC_FAR *pcElementFetched);

    //
    HRESULT STDMETHODCALLTYPE Skip(ULONG cElements);

    //
    HRESULT STDMETHODCALLTYPE Reset( void);

    //
    HRESULT STDMETHODCALLTYPE Clone(IEnumVARIANT __RPC_FAR *__RPC_FAR *ppenum);

    // IEnumVariant

    //
    HRESULT STDMETHODCALLTYPE PutSelection(long pXSelection);

    //
    static HRESULT STDMETHODCALLTYPE Create(CEnumVariant __RPC_FAR *__RPC_FAR *ppenum);

    //
    long GetCountOfElements();

private:

    long m_lCurrent;
    long m_lLBound;
    XAccessible* pUNOInterface;                     // XAccessible.
    Reference<XAccessibleSelection> m_pXAccessibleSelection;    // Selection.
};

#endif //__ENUMVARIANT_H_
