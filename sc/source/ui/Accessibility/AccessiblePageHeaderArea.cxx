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

#include <sal/config.h>

#include <tools/gen.hxx>
#include <AccessiblePageHeaderArea.hxx>
#include <AccessibleText.hxx>
#include <editsrc.hxx>
#include <prevwsh.hxx>
#include <scresid.hxx>
#include <strings.hrc>
#include <strings.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <editeng/editobj.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

    //=====  internal  ========================================================

ScAccessiblePageHeaderArea::ScAccessiblePageHeaderArea(
        const uno::Reference<XAccessible>& rxParent,
        ScPreviewShell* pViewShell,
        const EditTextObject* pEditObj,
        SvxAdjust eAdjust)
        : ScAccessibleContextBase(rxParent, AccessibleRole::TEXT),
        mpEditObj(pEditObj->Clone()),
        mpViewShell(pViewShell),
        meAdjust(eAdjust)
{
    if (mpViewShell)
        mpViewShell->AddAccessibilityObject(*this);
}

ScAccessiblePageHeaderArea::~ScAccessiblePageHeaderArea()
{
    if (!ScAccessibleContextBase::IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_atomic_increment( &m_refCount );
        dispose();
    }
}

void SAL_CALL ScAccessiblePageHeaderArea::disposing()
{
    SolarMutexGuard aGuard;
    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = nullptr;
    }
    mpTextHelper.reset();
    mpEditObj.reset();
    ScAccessibleContextBase::disposing();
}

//=====  SfxListener  =====================================================

void ScAccessiblePageHeaderArea::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    // only notify if child exist, otherwise it is not necessary
    if (rHint.GetId() == SfxHintId::ScAccVisAreaChanged)
    {
        if (mpTextHelper)
            mpTextHelper->UpdateChildren();

        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::VISIBLE_DATA_CHANGED;
        aEvent.Source = uno::Reference< XAccessibleContext >(this);
        CommitChange(aEvent);
    }
    ScAccessibleContextBase::Notify(rBC, rHint);
}
    //=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessiblePageHeaderArea::getAccessibleAtPoint(
        const awt::Point& rPoint )
{
    uno::Reference<XAccessible> xRet;
    if (containsPoint(rPoint))
    {
        SolarMutexGuard aGuard;
        IsObjectValid();

        if(!mpTextHelper)
            CreateTextHelper();

        xRet = mpTextHelper->GetAt(rPoint);
    }

    return xRet;
}

    //=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL
    ScAccessiblePageHeaderArea::getAccessibleChildCount()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChildCount();
}

uno::Reference< XAccessible > SAL_CALL
    ScAccessiblePageHeaderArea::getAccessibleChild(sal_Int32 nIndex)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChild(nIndex);
}

uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessiblePageHeaderArea::getAccessibleStateSet()
{
    SolarMutexGuard aGuard;
    uno::Reference<XAccessibleStateSet> xParentStates;
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
        xParentStates = xParentContext->getAccessibleStateSet();
    }
    utl::AccessibleStateSetHelper* pStateSet = new utl::AccessibleStateSetHelper();
    if (IsDefunc())
        pStateSet->AddState(AccessibleStateType::DEFUNC);
    else
    {
        pStateSet->AddState(AccessibleStateType::ENABLED);
        pStateSet->AddState(AccessibleStateType::MULTI_LINE);
        if (isShowing())
            pStateSet->AddState(AccessibleStateType::SHOWING);
        if (isVisible())
            pStateSet->AddState(AccessibleStateType::VISIBLE);
    }
    return pStateSet;
}

// XServiceInfo

OUString SAL_CALL
       ScAccessiblePageHeaderArea::getImplementationName()
{
    return OUString("ScAccessiblePageHeaderArea");
}

uno::Sequence< OUString> SAL_CALL
       ScAccessiblePageHeaderArea::getSupportedServiceNames()
{
    uno::Sequence< OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);

    aSequence[nOldSize] = "com.sun.star.sheet.AccessiblePageHeaderFooterAreasView";

    return aSequence;
}

//=====  XTypeProvider  =======================================================

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessiblePageHeaderArea::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

//===== internal ==============================================================
OUString ScAccessiblePageHeaderArea::createAccessibleDescription()
{
    OUString sDesc;
    switch (meAdjust)
    {
    case SvxAdjust::Left :
        sDesc = STR_ACC_LEFTAREA_DESCR;
        break;
    case SvxAdjust::Right:
        sDesc = STR_ACC_RIGHTAREA_DESCR;
        break;
    case SvxAdjust::Center:
        sDesc = STR_ACC_CENTERAREA_DESCR;
        break;
    default:
        OSL_FAIL("wrong adjustment found");
    }

    return sDesc;
}

OUString ScAccessiblePageHeaderArea::createAccessibleName()
{
    OUString sName;
    switch (meAdjust)
    {
    case SvxAdjust::Left :
        sName = ScResId(STR_ACC_LEFTAREA_NAME);
        break;
    case SvxAdjust::Right:
        sName = ScResId(STR_ACC_RIGHTAREA_NAME);
        break;
    case SvxAdjust::Center:
        sName = ScResId(STR_ACC_CENTERAREA_NAME);
        break;
    default:
        OSL_FAIL("wrong adjustment found");
    }

    return sName;
}

tools::Rectangle ScAccessiblePageHeaderArea::GetBoundingBoxOnScreen() const
{
    tools::Rectangle aRect;
    if (mxParent.is())
    {
        uno::Reference<XAccessibleContext> xContext = mxParent->getAccessibleContext();
        uno::Reference<XAccessibleComponent> xComp(xContext, uno::UNO_QUERY);
        if (xComp.is())
        {
            // has the same size and position on screen like the parent
            aRect = tools::Rectangle(VCLPoint(xComp->getLocationOnScreen()), VCLRectangle(xComp->getBounds()).GetSize());
        }
    }
    return aRect;
}

tools::Rectangle ScAccessiblePageHeaderArea::GetBoundingBox() const
{
    tools::Rectangle aRect;
    if (mxParent.is())
    {
        uno::Reference<XAccessibleContext> xContext = mxParent->getAccessibleContext();
        uno::Reference<XAccessibleComponent> xComp(xContext, uno::UNO_QUERY);
        if (xComp.is())
        {
            // has the same size and position on screen like the parent and so the pos is (0, 0)
            tools::Rectangle aNewRect(Point(0, 0), VCLRectangle(xComp->getBounds()).GetSize());
            aRect = aNewRect;
        }
    }

    return aRect;
}

void ScAccessiblePageHeaderArea::CreateTextHelper()
{
    if (!mpTextHelper)
    {
        mpTextHelper.reset( new ::accessibility::AccessibleTextHelper(
            std::make_unique<ScAccessibilityEditSource>(
                std::make_unique<ScAccessibleHeaderTextData>(
                    mpViewShell, mpEditObj.get(), meAdjust))) );
        mpTextHelper->SetEventSource(this);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
