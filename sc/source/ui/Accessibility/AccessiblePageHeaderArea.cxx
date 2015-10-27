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

#include <memory>
#include <utility>

#include <tools/gen.hxx>
#include "AccessiblePageHeaderArea.hxx"
#include "AccessibleText.hxx"
#include "AccessibilityHints.hxx"
#include "editsrc.hxx"
#include "prevwsh.hxx"
#include "prevloc.hxx"
#include "scresid.hxx"
#include "sc.hrc"

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <editeng/editobj.hxx>
#include <svx/AccessibleTextHelper.hxx>
#include <comphelper/servicehelper.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <rtl/ustrbuf.hxx>
#include <toolkit/helper/convert.hxx>
#include <vcl/svapp.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

    //=====  internal  ========================================================

ScAccessiblePageHeaderArea::ScAccessiblePageHeaderArea(
        const uno::Reference<XAccessible>& rxParent,
        ScPreviewShell* pViewShell,
        const EditTextObject* pEditObj,
        bool bHeader,
        SvxAdjust eAdjust)
        : ScAccessibleContextBase(rxParent, AccessibleRole::TEXT),
        mpEditObj(pEditObj->Clone()),
        mpTextHelper(NULL),
        mpViewShell(pViewShell),
        mbHeader(bHeader),
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
        mpViewShell = NULL;
    }
    if (mpTextHelper)
        DELETEZ(mpTextHelper);
    if (mpEditObj)
        DELETEZ(mpEditObj);

    ScAccessibleContextBase::disposing();
}

//=====  SfxListener  =====================================================

void ScAccessiblePageHeaderArea::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
    if (pSimpleHint)
    {
        // only notify if child exist, otherwise it is not necessary
        if (pSimpleHint->GetId() == SC_HINT_ACC_VISAREACHANGED)
        {
            if (mpTextHelper)
                mpTextHelper->UpdateChildren();

            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::VISIBLE_DATA_CHANGED;
            aEvent.Source = uno::Reference< XAccessibleContext >(this);
            CommitChange(aEvent);
        }
    }
    ScAccessibleContextBase::Notify(rBC, rHint);
}
    //=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessiblePageHeaderArea::getAccessibleAtPoint(
        const awt::Point& rPoint )
        throw (uno::RuntimeException, std::exception)
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
                    throw (uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChildCount();
}

uno::Reference< XAccessible > SAL_CALL
    ScAccessiblePageHeaderArea::getAccessibleChild(sal_Int32 nIndex)
        throw (uno::RuntimeException,
        lang::IndexOutOfBoundsException, std::exception)
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (!mpTextHelper)
        CreateTextHelper();
    return mpTextHelper->GetChild(nIndex);
}

uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessiblePageHeaderArea::getAccessibleStateSet()
    throw (uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
{
    return OUString("ScAccessiblePageHeaderArea");
}

uno::Sequence< OUString> SAL_CALL
       ScAccessiblePageHeaderArea::getSupportedServiceNames()
    throw (uno::RuntimeException, std::exception)
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
    throw (uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

//===== internal ==============================================================
OUString SAL_CALL ScAccessiblePageHeaderArea::createAccessibleDescription()
    throw(uno::RuntimeException, std::exception)
{
    OUString sDesc;
    switch (meAdjust)
    {
    case SVX_ADJUST_LEFT :
        sDesc = OUString(ScResId(STR_ACC_LEFTAREA_DESCR));
        break;
    case SVX_ADJUST_RIGHT:
        sDesc = OUString(ScResId(STR_ACC_RIGHTAREA_DESCR));
        break;
    case SVX_ADJUST_CENTER:
        sDesc = OUString(ScResId(STR_ACC_CENTERAREA_DESCR));
        break;
    default:
        OSL_FAIL("wrong adjustment found");
    }

    return sDesc;
}

OUString SAL_CALL ScAccessiblePageHeaderArea::createAccessibleName()
    throw (uno::RuntimeException, std::exception)
{
    OUString sName;
    switch (meAdjust)
    {
    case SVX_ADJUST_LEFT :
        sName = OUString(ScResId(STR_ACC_LEFTAREA_NAME));
        break;
    case SVX_ADJUST_RIGHT:
        sName = OUString(ScResId(STR_ACC_RIGHTAREA_NAME));
        break;
    case SVX_ADJUST_CENTER:
        sName = OUString(ScResId(STR_ACC_CENTERAREA_NAME));
        break;
    default:
        OSL_FAIL("wrong adjustment found");
    }

    return sName;
}

Rectangle ScAccessiblePageHeaderArea::GetBoundingBoxOnScreen() const
    throw(css::uno::RuntimeException, std::exception)
{
    Rectangle aRect;
    if (mxParent.is())
    {
        uno::Reference<XAccessibleContext> xContext = mxParent->getAccessibleContext();
        uno::Reference<XAccessibleComponent> xComp(xContext, uno::UNO_QUERY);
        if (xComp.is())
        {
            // has the same size and position on screen like the parent
            aRect = Rectangle(VCLPoint(xComp->getLocationOnScreen()), VCLRectangle(xComp->getBounds()).GetSize());
        }
    }
    return aRect;
}

Rectangle ScAccessiblePageHeaderArea::GetBoundingBox() const
    throw (css::uno::RuntimeException, std::exception)
{
    Rectangle aRect;
    if (mxParent.is())
    {
        uno::Reference<XAccessibleContext> xContext = mxParent->getAccessibleContext();
        uno::Reference<XAccessibleComponent> xComp(xContext, uno::UNO_QUERY);
        if (xComp.is())
        {
            // has the same size and position on screen like the parent and so the pos is (0, 0)
            Rectangle aNewRect(Point(0, 0), VCLRectangle(xComp->getBounds()).GetSize());
            aRect = aNewRect;
        }
    }

    return aRect;
}

void ScAccessiblePageHeaderArea::CreateTextHelper()
{
    if (!mpTextHelper)
    {
        ::std::unique_ptr < ScAccessibleTextData > pAccessibleHeaderTextData
            (new ScAccessibleHeaderTextData(mpViewShell, mpEditObj, mbHeader, meAdjust));
        ::std::unique_ptr< SvxEditSource > pEditSource (new ScAccessibilityEditSource(std::move(pAccessibleHeaderTextData)));

        mpTextHelper = new ::accessibility::AccessibleTextHelper(std::move(pEditSource));
        mpTextHelper->SetEventSource(this);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
