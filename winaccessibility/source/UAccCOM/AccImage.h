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

#ifndef INCLUDED_WINACCESSIBILITY_SOURCE_UACCCOM_ACCIMAGE_H
#define INCLUDED_WINACCESSIBILITY_SOURCE_UACCCOM_ACCIMAGE_H

#include "resource.h"        // main symbols

#include <com/sun/star/uno/reference.hxx>
#include <com/sun/star/accessibility/XAccessibleImage.hpp>
#include "UNOXWrapper.h"

/**
 * CAccImage implements IAccessibleImage interface.
 */
class ATL_NO_VTABLE CAccImage :
            public CComObjectRoot,
            public CComCoClass<CAccImage,&CLSID_AccImage>,
            public IAccessibleImage,
            public CUNOXWrapper
{
public:
    CAccImage()
    {
            }
    virtual ~CAccImage()
    {
            }
    BEGIN_COM_MAP(CAccImage)
    COM_INTERFACE_ENTRY(IAccessibleImage)
    COM_INTERFACE_ENTRY(IUNOXWrapper)
    COM_INTERFACE_ENTRY_FUNC_BLIND(NULL,_SmartQI)
    END_COM_MAP()

    static HRESULT WINAPI _SmartQI(void* pv,
                                   REFIID iid, void** ppvObject, DWORD_PTR)
    {
        return ((CAccImage*)pv)->SmartQI(iid,ppvObject);
    }

    HRESULT SmartQI(REFIID iid, void** ppvObject)
    {
        if( m_pOuterUnknown )
            return OuterQueryInterface(iid,ppvObject);
        return E_FAIL;
    }

    DECLARE_NO_REGISTRY()

public:
    // IAccessibleImage

    // Gets the description of the image.
    STDMETHOD(get_description)(BSTR * description);

    STDMETHOD(get_imagePosition)( enum IA2CoordinateType coordinateType,
                                  long __RPC_FAR *x,
                                  long __RPC_FAR *y);

    STDMETHOD(get_imageSize)(
        long __RPC_FAR *height,
        long __RPC_FAR *width);

    // Override of IUNOXWrapper.
    STDMETHOD(put_XInterface)(hyper pXInterface);

private:

    css::uno::Reference<css::accessibility::XAccessibleImage> pRXImg;

    inline css::accessibility::XAccessibleImage* GetXInterface()
    {
        return pRXImg.get();
    }
};

#endif // INCLUDED_WINACCESSIBILITY_SOURCE_UACCCOM_ACCIMAGE_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
