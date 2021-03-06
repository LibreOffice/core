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

#pragma once

#include "Resource.h"       // main symbols

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

    BEGIN_COM_MAP(CAccComponent)
    COM_INTERFACE_ENTRY(IAccessibleComponent)
    COM_INTERFACE_ENTRY(IUNOXWrapper)
    COM_INTERFACE_ENTRY_FUNC_BLIND(NULL,SmartQI_)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
    END_COM_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif

    static HRESULT WINAPI SmartQI_(void* pv,
                                   REFIID iid, void** ppvObject, DWORD_PTR)
    {
        return static_cast<CAccComponent*>(pv)->SmartQI(iid,ppvObject);
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
    STDMETHOD(get_locationInParent)(long *x, long *y) override;

    // Returns the foreground color of this object.
    STDMETHOD(get_foreground)(IA2Color * foreground) override;

    // Returns the background color of this object.
    STDMETHOD(get_background)(IA2Color * background) override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
