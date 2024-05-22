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
#include <comphelper/sequence.hxx>
#include <editeng/editobj.hxx>
#include <svx/AccessibleTextHelper.hxx>
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

sal_Int64 SAL_CALL
    ScAccessiblePageHeaderArea::getAccessibleChildCount()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChildCount();
}

uno::Reference< XAccessible > SAL_CALL
    ScAccessiblePageHeaderArea::getAccessibleChild(sal_Int64 nIndex)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChild(nIndex);
}

sal_Int64 SAL_CALL ScAccessiblePageHeaderArea::getAccessibleStateSet()
{
    SolarMutexGuard aGuard;
    sal_Int64 nStateSet = 0;
    if (IsDefunc())
        nStateSet |= AccessibleStateType::DEFUNC;
    else
    {
        nStateSet |= AccessibleStateType::ENABLED;
        nStateSet |= AccessibleStateType::MULTI_LINE;
        if (isShowing())
            nStateSet |= AccessibleStateType::SHOWING;
        if (isVisible())
            nStateSet |= AccessibleStateType::VISIBLE;
    }
    return nStateSet;
}

// XServiceInfo

OUString SAL_CALL
       ScAccessiblePageHeaderArea::getImplementationName()
{
    return u"ScAccessiblePageHeaderArea"_ustr;
}

uno::Sequence< OUString> SAL_CALL
       ScAccessiblePageHeaderArea::getSupportedServiceNames()
{
    const css::uno::Sequence<OUString> vals { u"com.sun.star.sheet.AccessiblePageHeaderFooterAreasView"_ustr };
    return comphelper::concatSequences(ScAccessibleContextBase::getSupportedServiceNames(), vals);
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

AbsoluteScreenPixelRectangle ScAccessiblePageHeaderArea::GetBoundingBoxOnScreen() const
{
    AbsoluteScreenPixelRectangle aRect;
    if (mxParent.is())
    {
        uno::Reference<XAccessibleContext> xContext = mxParent->getAccessibleContext();
        uno::Reference<XAccessibleComponent> xComp(xContext, uno::UNO_QUERY);
        if (xComp.is())
        {
            // has the same size and position on screen like the parent
            aRect = AbsoluteScreenPixelRectangle(
                        AbsoluteScreenPixelPoint(VCLPoint(xComp->getLocationOnScreen())),
                        AbsoluteScreenPixelSize(VCLRectangle(xComp->getBounds()).GetSize()));
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
