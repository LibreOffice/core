/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: unotypewrapper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 14:49:08 $
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
