/*************************************************************************
 *
 *  $RCSfile: Foo.h,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-03-17 13:12:25 $
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

