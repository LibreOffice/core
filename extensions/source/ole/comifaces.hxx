/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_EXTENSIONS_SOURCE_OLE_COMIFACES_HXX
#define INCLUDED_EXTENSIONS_SOURCE_OLE_COMIFACES_HXX

#include <com/sun/star/uno/XInterface.hpp>

using namespace com::sun::star::uno;

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

protected:
    ~IJScriptValueObject() {}
};

MIDL_INTERFACE("7B5C3410-66FA-11d4-832A-005004526AB4")
IUnoObjectWrapper: public IUnknown
{
    STDMETHOD( getWrapperXInterface)( Reference<XInterface>* pInt)=0;
    STDMETHOD( getOriginalUnoObject)( Reference<XInterface>* pInt)=0;
    STDMETHOD( getOriginalUnoStruct)( Any * pStruct)=0;

protected:
    ~IUnoObjectWrapper() {}
};

MIDL_INTERFACE("8BB66591-A544-4de9-822C-57AB57BCED1C")
IUnoTypeWrapper: public IUnknown
{
    STDMETHOD(put_Name)(BSTR val) = 0;
    STDMETHOD(get_Name)(BSTR* pVal) = 0;

protected:
    ~IUnoTypeWrapper() {}
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
