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

#include "stdafx.h"
#include "UAccCOM.h"
#include "AccRelation.h"

#include <vcl/svapp.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/XAccessibleContext.hpp>
#include "MAccessible.h"

using namespace com::sun::star::accessibility;
using namespace com::sun::star::uno;

/**
   * Get relation type.
   * @param relationType Variant to get relation type.
   * @return Result.
*/
STDMETHODIMP CAccRelation::get_relationType(BSTR * relationType)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    if (relationType == NULL)
        return E_INVALIDARG;

    int type = relation.RelationType;
    SAFE_SYSFREESTRING(*relationType);

    *relationType = getRelationTypeBSTR(type);
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

// Gets what the type of localized relation is.
STDMETHODIMP CAccRelation::get_localizedRelationType(BSTR *)
{


    ENTER_PROTECTED_BLOCK

    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Get targets length.
   * @param nTargets Variant to get targets length.
   * @return Result.
*/
STDMETHODIMP CAccRelation::get_nTargets(long * nTargets)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    if (nTargets == NULL)
        return E_INVALIDARG;

    Sequence< Reference< XInterface > > xTargets = relation.TargetSet;
    *nTargets = xTargets.getLength();
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Get special target.
   * @param targetIndex target index.
   * @param target      Variant to get special target.
   * @return Result.
*/
STDMETHODIMP CAccRelation::get_target(long targetIndex, IUnknown * * target)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    if (target == NULL)
        return E_FAIL;

    Sequence< Reference< XInterface > > xTargets = relation.TargetSet;
    int nCount = xTargets.getLength();
    if( targetIndex >= nCount )
        return E_FAIL;

    Reference<XAccessible> xRAcc(xTargets[targetIndex], UNO_QUERY);
    IAccessible* pRet = NULL;

    BOOL isGet = CMAccessible::get_IAccessibleFromXAccessible(xRAcc.get(), &pRet);
    if(isGet)
    {
        *target = /*(IAccessible2 *)*/(IUnknown*)pRet;
        pRet->AddRef();
        return S_OK;
    }

    return E_FAIL;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Get special targets.
   * @param maxTargets Special targets count.
   * @param target Variant to get special target.
   * @param nTargets Variant to accept actual target length.
   * @return Result.
*/
STDMETHODIMP CAccRelation::get_targets(long, IUnknown * * target, long * nTargets)
{
    SolarMutexGuard g;

    ENTER_PROTECTED_BLOCK

    // #CHECK#
    if(target == NULL)
        return E_INVALIDARG;
    if (nTargets == NULL)
        return E_INVALIDARG;

    Sequence< Reference< XInterface > > xTargets = relation.TargetSet;
    int nCount = xTargets.getLength();

    *target = (IUnknown*)::CoTaskMemAlloc(nCount*sizeof(IUnknown));

    // #CHECK Memory Allocation#
    if(*target == NULL)
    {
        return E_FAIL;
    }

    for(int i=0; i<nCount ; i++)
    {
        IUnknown* pAcc = NULL;
        HRESULT hr = get_target(i,&pAcc);
        if(SUCCEEDED(hr))
            target[i] = pAcc;
    }

    *nTargets = nCount;
    return S_OK;

    LEAVE_PROTECTED_BLOCK
}

/**
   * Put UNO interface.
   * @param pXSubInterface AccessibleRelation pointer.
   * @return Result.
*/
STDMETHODIMP CAccRelation::put_XSubInterface(hyper pXSubInterface)
{
    // internal IUNOXWrapper - no mutex meeded

    relation = *reinterpret_cast<AccessibleRelation*>(pXSubInterface);
    return S_OK;
}

/**
   * Get relation type string by type.
   * @param type Relation type.
   * @return relation type string.
*/
BSTR CAccRelation::getRelationTypeBSTR(int type)
{
    static struct TYPE_BSTR_MAP
    {
        LPCTSTR string;
        int type;
    }
    map[] =
        {
            {_T("INVALID")              ,   0},
            {IA2_RELATION_FLOWS_FROM    ,   1},
            {IA2_RELATION_FLOWS_TO      ,   2},
            {IA2_RELATION_CONTROLLED_BY ,   3},
            {IA2_RELATION_CONTROLLER_FOR,   4},
            {IA2_RELATION_LABEL_FOR     ,   5},
            {IA2_RELATION_LABELED_BY    ,   6},
            {IA2_RELATION_MEMBER_OF     ,   7},
            {IA2_RELATION_SUBWINDOW_OF  ,   8},
            {IA2_RELATION_NODE_CHILD_OF ,   9},
            {IA2_RELATION_DESCRIBED_BY  ,  10},
        };

    USES_CONVERSION;

    return (type >= 0 && type <= 10) ? map[type].string : _T("");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
