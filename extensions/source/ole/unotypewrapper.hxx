/*************************************************************************
 *
 *  $RCSfile: unotypewrapper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-08-02 09:47:11 $
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
#ifndef UNO_TYPE_WRAPPER_HXX
#define UNO_TYPE_WRAPPER_HXX
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
