/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: jscriptclasses.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:46:18 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
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
