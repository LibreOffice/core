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
#include <UAccCOM.h>
#include "AccRelation.h"

#include <vcl/svapp.hxx>

#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
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
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccRelation::get_relationType(BSTR* relationType)
{
    SolarMutexGuard g;

    try
    {
        if (relationType == nullptr)
            return E_INVALIDARG;

        SysFreeString(*relationType);
        *relationType = SysAllocString(mapToIA2RelationType(relation.RelationType));
        return S_OK;
    }
    catch (...)
    {
        return E_FAIL;
    }
}

// Gets what the type of localized relation is.
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccRelation::get_localizedRelationType(BSTR*) { return S_OK; }

/**
   * Get targets length.
   * @param nTargets Variant to get targets length.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccRelation::get_nTargets(long* nTargets)
{
    SolarMutexGuard g;

    try
    {
        if (nTargets == nullptr)
            return E_INVALIDARG;

        Sequence<Reference<XAccessible>> xTargets = relation.TargetSet;
        *nTargets = xTargets.getLength();
        return S_OK;
    }
    catch (...)
    {
        return E_FAIL;
    }
}

/**
   * Get special target.
   * @param targetIndex target index.
   * @param target      Variant to get special target.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccRelation::get_target(long targetIndex, IUnknown** target)
{
    SolarMutexGuard g;

    try
    {
        if (target == nullptr)
            return E_FAIL;

        Sequence<Reference<XAccessible>> xTargets = relation.TargetSet;
        int nCount = xTargets.getLength();
        if (targetIndex >= nCount)
            return E_FAIL;

        Reference<XAccessible> xTarget = xTargets[targetIndex];
        if (!xTarget.is())
            return E_FAIL;

        IAccessible* pRet = CMAccessible::get_IAccessibleFromXAccessible(xTarget.get());
        if (!pRet)
        {
            Reference<XAccessibleContext> xTargetContext = xTarget->getAccessibleContext();
            if (!xTargetContext.is())
                return E_FAIL;

            Reference<XAccessible> xParent = xTargetContext->getAccessibleParent();
            CMAccessible::g_pAccObjectManager->InsertAccObj(xTarget.get(), xParent.get());
            pRet = CMAccessible::get_IAccessibleFromXAccessible(xTarget.get());
        }

        if (pRet)
        {
            *target = pRet;
            pRet->AddRef();
            return S_OK;
        }

        return E_FAIL;
    }
    catch (...)
    {
        return E_FAIL;
    }
}

/**
   * Get special targets.
   * @param maxTargets Special targets count.
   * @param target Variant to get special target.
   * @param nTargets Variant to accept actual target length.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccRelation::get_targets(long, IUnknown** target, long* nTargets)
{
    SolarMutexGuard g;

    try
    {
        if (target == nullptr)
            return E_INVALIDARG;
        if (nTargets == nullptr)
            return E_INVALIDARG;

        Sequence<Reference<XAccessible>> xTargets = relation.TargetSet;
        int nCount = xTargets.getLength();

        *target = static_cast<IUnknown*>(::CoTaskMemAlloc(nCount * sizeof(IUnknown)));

        // #CHECK Memory Allocation#
        if (*target == nullptr)
        {
            return E_FAIL;
        }

        for (int i = 0; i < nCount; i++)
        {
            IUnknown* pAcc = nullptr;
            HRESULT hr = get_target(i, &pAcc);
            if (SUCCEEDED(hr))
                target[i] = pAcc;
        }

        *nTargets = nCount;
        return S_OK;
    }
    catch (...)
    {
        return E_FAIL;
    }
}

/**
   * Put UNO interface.
   * @param pXSubInterface AccessibleRelation pointer.
   * @return Result.
*/
COM_DECLSPEC_NOTHROW STDMETHODIMP CAccRelation::put_XSubInterface(hyper pXSubInterface)
{
    // internal IUNOXWrapper - no mutex meeded

    relation = *reinterpret_cast<AccessibleRelation*>(pXSubInterface);
    return S_OK;
}

const wchar_t* CAccRelation::mapToIA2RelationType(sal_Int16 nUnoRelationType)
{
    switch (nUnoRelationType)
    {
        case AccessibleRelationType::CONTENT_FLOWS_FROM:
            return IA2_RELATION_FLOWS_FROM;
        case AccessibleRelationType::CONTENT_FLOWS_TO:
            return IA2_RELATION_FLOWS_TO;
        case AccessibleRelationType::CONTROLLED_BY:
            return IA2_RELATION_CONTROLLED_BY;
        case AccessibleRelationType::CONTROLLER_FOR:
            return IA2_RELATION_CONTROLLER_FOR;
        case AccessibleRelationType::LABEL_FOR:
            return IA2_RELATION_LABEL_FOR;
        case AccessibleRelationType::LABELED_BY:
            return IA2_RELATION_LABELED_BY;
        case AccessibleRelationType::MEMBER_OF:
            return IA2_RELATION_MEMBER_OF;
        case AccessibleRelationType::SUB_WINDOW_OF:
            return IA2_RELATION_SUBWINDOW_OF;
        case AccessibleRelationType::NODE_CHILD_OF:
            return IA2_RELATION_NODE_CHILD_OF;
        case AccessibleRelationType::DESCRIBED_BY:
            return IA2_RELATION_DESCRIBED_BY;
        case AccessibleRelationType::INVALID:
            return L"INVALID";
        default:
            assert(false && "unhandled AccessibleRelationType");
            return L"";
    }
}

sal_Int16 CAccRelation::mapToUnoRelationType(const BSTR aIA2RelationType)
{
    if (wcscmp(aIA2RelationType, IA2_RELATION_FLOWS_FROM) == 0)
        return AccessibleRelationType::CONTENT_FLOWS_FROM;
    if (wcscmp(aIA2RelationType, IA2_RELATION_FLOWS_TO) == 0)
        return AccessibleRelationType::CONTENT_FLOWS_TO;
    if (wcscmp(aIA2RelationType, IA2_RELATION_CONTROLLED_BY) == 0)
        return AccessibleRelationType::CONTROLLED_BY;
    if (wcscmp(aIA2RelationType, IA2_RELATION_CONTROLLER_FOR) == 0)
        return AccessibleRelationType::CONTROLLER_FOR;
    if (wcscmp(aIA2RelationType, IA2_RELATION_LABEL_FOR) == 0)
        return AccessibleRelationType::LABEL_FOR;
    if (wcscmp(aIA2RelationType, IA2_RELATION_LABELED_BY) == 0)
        return AccessibleRelationType::LABELED_BY;
    if (wcscmp(aIA2RelationType, IA2_RELATION_MEMBER_OF) == 0)
        return AccessibleRelationType::MEMBER_OF;
    if (wcscmp(aIA2RelationType, IA2_RELATION_SUBWINDOW_OF) == 0)
        return AccessibleRelationType::SUB_WINDOW_OF;
    if (wcscmp(aIA2RelationType, IA2_RELATION_NODE_CHILD_OF) == 0)
        return AccessibleRelationType::NODE_CHILD_OF;
    if (wcscmp(aIA2RelationType, IA2_RELATION_DESCRIBED_BY) == 0)
        return AccessibleRelationType::DESCRIBED_BY;

    // not all IAccessible2 relation types have a UNO equivalent
    return AccessibleRelationType::INVALID;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
