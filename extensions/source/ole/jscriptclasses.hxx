/*************************************************************************
 *
 *  $RCSfile: jscriptclasses.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:16:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef __JSCRIPTCLASSES_HXX
#define __JSCRIPTCLASSES_HXX
#include <tools/presys.h>
#include "stdafx.h"
#include <tools/postsys.h>

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
