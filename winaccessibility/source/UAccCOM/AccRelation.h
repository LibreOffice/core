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

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/accessibility/XAccessibleRelationSet.hpp>
#include "UNOXWrapper.h"

/**
 * CAccRelation implements IAccessibleRelation interface.
 */
class ATL_NO_VTABLE CAccRelation :
            public CComObjectRoot,
            public CComCoClass<CAccRelation, &CLSID_AccRelation>,
            public IAccessibleRelation,
            public CUNOXWrapper
{
public:
    CAccRelation()
    {
            }
    virtual ~CAccRelation()
    {
            }

    DECLARE_NO_REGISTRY()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CAccRelation)
    COM_INTERFACE_ENTRY(IAccessibleRelation)
    COM_INTERFACE_ENTRY(IUNOXWrapper)
#if defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winconsistent-missing-override"
#endif
    END_COM_MAP()
#if defined __clang__
#pragma clang diagnostic pop
#endif

    // IAccessibleRelation
public:
    // IAccessibleRelation

    // Gets what the type of relation is.
    STDMETHOD(get_relationType)(BSTR * relationType) override;

    // Gets what the type of localized relation is.
    STDMETHOD(get_localizedRelationType)(BSTR * relationType) override;

    // Gets how many targets this relation have.
    STDMETHOD(get_nTargets)(long * nTargets) override;

    // Gets one accessible relation target.
    STDMETHOD(get_target)(long targetIndex, IUnknown * * target) override;

    // Gets multiple accessible relation targets.
    STDMETHOD(get_targets)(long maxTargets, IUnknown * * target, long * nTargets) override;

    // Override of IUNOXWrapper.
    STDMETHOD(put_XSubInterface)(hyper pXSubInterface) override;

    //static OLECHAR* getRelationTypeOLECHAR(int type);
    static BSTR getRelationTypeBSTR(int type);

private:

    css::accessibility::AccessibleRelation relation;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
