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


#ifndef __JSCRIPTCLASSES_HXX
#define __JSCRIPTCLASSES_HXX


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



// Sequences are represented by prepending "[]", e.g. []char, [][]byte, [][][]object, etc.

// To make a JScriptValue object to an out parameter, call
// "InitOutParam" and to make it a in/out parameter call
// "InitInOutParam"

// If the object represents an out parameter then the value can after the call
// be retrived by "Get".

// From JavaScript the functions Get, Set, InitOutParam and InitInOutParam are
// used, that is they are accessible through IDispatch. The functions are used
// by the bridge.

class JScriptValue:
      public CComObjectRootEx<CComMultiThreadModel>,
      public IJScriptValueObject,
      public IDispatch
{
public:
    JScriptValue();
    virtual ~JScriptValue();

    BEGIN_COM_MAP(JScriptValue)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IJScriptValueObject)
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
    // IJScriptOutParam --------------------------------------

    STDMETHOD( Set)( VARIANT type, VARIANT value);
    STDMETHOD( Get)( VARIANT *val);
    STDMETHOD( InitOutParam)();
    STDMETHOD( InitInOutParam)( VARIANT type, VARIANT value);
    STDMETHOD( IsOutParam)( VARIANT_BOOL * flag);
    STDMETHOD( IsInOutParam)( VARIANT_BOOL * flag);
    STDMETHOD( GetValue)( BSTR* type, VARIANT *value);


    CComVariant m_varValue;
    CComBSTR m_bstrType;
    unsigned m_bOutParam: 1;
    unsigned m_bInOutParam: 1;

};

// If a class is implemented in JScript, then its method
class JScriptOutParam:
      public CComObjectRootEx<CComMultiThreadModel>,
      public IDispatch
{
public:
    JScriptOutParam();
    virtual ~JScriptOutParam();

    BEGIN_COM_MAP(JScriptOutParam)
        COM_INTERFACE_ENTRY(IDispatch)
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


private:
    CComVariant m_varValue;
};

#endif
