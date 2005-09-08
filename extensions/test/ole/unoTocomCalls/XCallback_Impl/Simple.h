/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Simple.h,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 20:55:39 $
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

