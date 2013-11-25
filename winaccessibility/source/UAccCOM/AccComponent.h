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

#if !defined(AFX_ACCCOMPONENT_H__626D760C_3944_4B0E_BB4D_F0D20AEDF7DC__INCLUDED_)
#define AFX_ACCCOMPONENT_H__626D760C_3944_4B0E_BB4D_F0D20AEDF7DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"       // main symbols

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include <com/sun/star/accessibility/XAccessibleComponent.hpp>
#include "UNOXWrapper.h"
#include "AccComponentBase.h"


/**
 * CAccComponent implements IAccessibleComponent interface.
 */
class ATL_NO_VTABLE CAccComponent :
            public CComObjectRoot,
            public CComCoClass<CAccComponent,&CLSID_AccComponent>,
            public IAccessibleComponent,
            public CAccComponentBase
{
public:
    CAccComponent()
    {
            }
    ~CAccComponent()
    {
            }

    BEGIN_COM_MAP(CAccComponent)
    COM_INTERFACE_ENTRY(IAccessibleComponent)
    COM_INTERFACE_ENTRY(IUNOXWrapper)
    COM_INTERFACE_ENTRY_FUNC_BLIND(NULL,_SmartQI)
    END_COM_MAP()

    static HRESULT WINAPI _SmartQI(void* pv,
                                   REFIID iid, void** ppvObject, DWORD)
    {
        return ((CAccComponent*)pv)->SmartQI(iid,ppvObject);
    }

    HRESULT SmartQI(REFIID iid, void** ppvObject)
    {
        if( m_pOuterUnknown )
            return OuterQueryInterface(iid,ppvObject);
        return E_FAIL;
    }

    DECLARE_NO_REGISTRY()

public:
    // IAccessibleComponent

    // Returns the location of the upper left corner of the object's bounding
    // box relative to the parent.
    STDMETHOD(get_locationInParent)(long *x, long *y);

    // Returns the foreground color of this object.
    STDMETHOD(get_foreground)(IA2Color * foreground);

    // Returns the background color of this object.
    STDMETHOD(get_background)(IA2Color * background);

    // Overide of IUNOXWrapper.
    STDMETHOD(put_XInterface)(hyper pXInterface);

};

#endif // !defined(AFX_ACCCOMPONENT_H__626D760C_3944_4B0E_BB4D_F0D20AEDF7DC__INCLUDED_)

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
