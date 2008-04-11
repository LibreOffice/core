/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: comifaces.hxx,v $
 * $Revision: 1.5 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef __COMIFACES_HXX
#define __COMIFACES_HXX

#include <com/sun/star/uno/XInterface.hpp>

using namespace com::sun::star::uno;

#ifdef __MINGW32__
DEFINE_GUID(IID_IJScriptValueObject, 0xe40a2331, 0x3bc1, 0x11d4, 0x83, 0x21, 0x00, 0x50, 0x04, 0x52, 0x6a, 0xb4);
DEFINE_GUID(IID_IUnoObjectWrapper, 0x7B5C3410, 0x66FA, 0x11d4, 0x83, 0x2A, 0x00, 0x50, 0x04, 0x52, 0x6A, 0xB4);
DEFINE_GUID(IID_IUnoTypeWrapper, 0x8BB66591, 0xA544, 0x4de9, 0x82, 0x2C, 0x57, 0xAB, 0x57, 0xBC, 0xED, 0x1C);
#endif
MIDL_INTERFACE("e40a2331-3bc1-11d4-8321-005004526ab4")
IJScriptValueObject: public IUnknown
{
    STDMETHOD( Set)( VARIANT type,  VARIANT value)= 0;
    STDMETHOD( Get)( VARIANT *val)= 0;
    STDMETHOD( InitOutParam)()= 0;
    STDMETHOD( InitInOutParam)( VARIANT type, VARIANT value)= 0;
    STDMETHOD( IsOutParam)( VARIANT_BOOL * flag)= 0;
    STDMETHOD( IsInOutParam)( VARIANT_BOOL * flag)= 0;
    STDMETHOD( GetValue)( BSTR* type, VARIANT *value)= 0;

};

MIDL_INTERFACE("7B5C3410-66FA-11d4-832A-005004526AB4")
IUnoObjectWrapper: public IUnknown
{
    STDMETHOD( getWrapperXInterface)( Reference<XInterface>* pInt)=0;
    STDMETHOD( getOriginalUnoObject)( Reference<XInterface>* pInt)=0;
    STDMETHOD( getOriginalUnoStruct)( Any * pStruct)=0;

};

MIDL_INTERFACE("8BB66591-A544-4de9-822C-57AB57BCED1C")
IUnoTypeWrapper: public IUnknown
{
    STDMETHOD(put_Name)(BSTR val) = 0;
    STDMETHOD(get_Name)(BSTR* pVal) = 0;
};


#endif
