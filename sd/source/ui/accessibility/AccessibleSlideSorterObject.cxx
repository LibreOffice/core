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

#include <AccessibleSlideSorterObject.hxx>

#include <SlideSorter.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsPageSelector.hxx>
#include <controller/SlsFocusManager.hxx>
#include <model/SlideSorterModel.hxx>
#include <model/SlsPageDescriptor.hxx>
#include <view/SlideSorterView.hxx>
#include <view/SlsLayouter.hxx>
#include <view/SlsPageObjectLayouter.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/IllegalAccessibleComponentStateException.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <comphelper/accessibleeventnotifier.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>

#include <sdpage.hxx>
#include <sdresid.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <strings.hrc>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::accessibility;

namespace accessibility {

AccessibleSlideSorterObject::AccessibleSlideSorterObject(
    const rtl::Reference<AccessibleSlideSorterView>& rxParent,
    ::sd::slidesorter::SlideSorter& rSlideSorter,
    sal_uInt16 nPageNumber)
    : mxParent(rxParent),
      mnPageNumber(nPageNumber),
      mrSlideSorter(rSlideSorter)
{
}

AccessibleSlideSorterObject::~AccessibleSlideSorterObject()
{
}

void AccessibleSlideSorterObject::FireAccessibleEvent (
    short nEventId,
    const uno::Any& rOldValue,
    const uno::Any& rNewValue)
{
    NotifyAccessibleEvent(nEventId, rOldValue, rNewValue);
}

//===== XAccessible ===========================================================

Reference<XAccessibleContext> SAL_CALL
    AccessibleSlideSorterObject::getAccessibleContext()
{
    ensureAlive();
    return this;
}

//===== XAccessibleContext ====================================================

sal_Int64 SAL_CALL AccessibleSlideSorterObject::getAccessibleChildCount()
{
    ensureAlive();
    return 0;
}

Reference<XAccessible> SAL_CALL AccessibleSlideSorterObject::getAccessibleChild (sal_Int64 )
{
    ensureAlive();
    throw lang::IndexOutOfBoundsException();
}

Reference<XAccessible> SAL_CALL AccessibleSlideSorterObject::getAccessibleParent()
{
    ensureAlive();
    return mxParent;
}

sal_Int64 SAL_CALL AccessibleSlideSorterObject::getAccessibleIndexInParent()
{
    ensureAlive();
    const SolarMutexGuard aSolarGuard;
    sal_Int64 nIndexInParent(-1);

    if (mxParent.is())
    {
        sal_Int64 nChildCount (mxParent->getAccessibleChildCount());
        for (sal_Int64 i=0; i<nChildCount; ++i)
            if (mxParent->getAccessibleChild(i).get() == static_cast<XAccessible*>(this))
            {
                nIndexInParent = i;
                break;
            }
    }

    return nIndexInParent;
}

sal_Int16 SAL_CALL AccessibleSlideSorterObject::getAccessibleRole()
{
    ensureAlive();
    return AccessibleRole::SHAPE;
}

OUString SAL_CALL AccessibleSlideSorterObject::getAccessibleDescription()
{
    ensureAlive();
    return SdResId(STR_PAGE);
}

OUString SAL_CALL AccessibleSlideSorterObject::getAccessibleName()
{
    ensureAlive();
    const SolarMutexGuard aSolarGuard;

    SdPage* pPage = GetPage();
    if (pPage != nullptr)
        return pPage->GetName();
    else
        return OUString();
}

Reference<XAccessibleRelationSet> SAL_CALL
    AccessibleSlideSorterObject::getAccessibleRelationSet()
{
    ensureAlive();
    return Reference<XAccessibleRelationSet>();
}

sal_Int64 SAL_CALL
    AccessibleSlideSorterObject::getAccessibleStateSet()
{
    ensureAlive();
    const SolarMutexGuard aSolarGuard;
    sal_Int64 nStateSet = 0;

    if (mxParent.is())
    {
        // Unconditional states.
        nStateSet |= AccessibleStateType::SELECTABLE;
        nStateSet |= AccessibleStateType::FOCUSABLE;
        nStateSet |= AccessibleStateType::ENABLED;
        nStateSet |= AccessibleStateType::VISIBLE;
        nStateSet |= AccessibleStateType::SHOWING;
        nStateSet |= AccessibleStateType::ACTIVE;
        nStateSet |= AccessibleStateType::SENSITIVE;

        // Conditional states.
        if (mrSlideSorter.GetController().GetPageSelector().IsPageSelected(mnPageNumber))
            nStateSet |= AccessibleStateType::SELECTED;
        if (mrSlideSorter.GetController().GetFocusManager().GetFocusedPageIndex() == mnPageNumber)
            if (mrSlideSorter.GetController().GetFocusManager().IsFocusShowing())
                nStateSet |= AccessibleStateType::FOCUSED;
    }

    return nStateSet;
}

lang::Locale SAL_CALL AccessibleSlideSorterObject::getLocale()
{
    ensureAlive();
    // Delegate request to parent.
    if (mxParent.is())
    {
        return mxParent->getLocale ();
    }

    //  No locale and no parent.  Therefore throw exception to indicate this
    //  cluelessness.
    throw IllegalAccessibleComponentStateException();
}

//===== XAccessibleComponent ==================================================

Reference<XAccessible> SAL_CALL
    AccessibleSlideSorterObject::getAccessibleAtPoint(const awt::Point& )
{
    return nullptr;
}

awt::Rectangle AccessibleSlideSorterObject::implGetBounds()
{
    ::tools::Rectangle aBBox (
        mrSlideSorter.GetView().GetLayouter().GetPageObjectLayouter()->GetBoundingBox(
            mrSlideSorter.GetModel().GetPageDescriptor(mnPageNumber),
            ::sd::slidesorter::view::PageObjectLayouter::Part::PageObject,
            ::sd::slidesorter::view::PageObjectLayouter::WindowCoordinateSystem));

    if (mxParent.is())
    {
        awt::Rectangle aParentBBox (mxParent->getBounds());
        aBBox.Intersection(::tools::Rectangle(
            aParentBBox.X,
            aParentBBox.Y,
            aParentBBox.Width,
            aParentBBox.Height));
    }

    return awt::Rectangle(
        aBBox.Left(),
        aBBox.Top(),
        aBBox.GetWidth(),
        aBBox.GetHeight());
}

void SAL_CALL AccessibleSlideSorterObject::grabFocus()
{
    // nothing to do
}

sal_Int32 SAL_CALL AccessibleSlideSorterObject::getForeground()
{
    ensureAlive();
    svtools::ColorConfig aColorConfig;
    Color nColor = aColorConfig.GetColorValue( svtools::FONTCOLOR ).nColor;
    return static_cast<sal_Int32>(nColor);
}

sal_Int32 SAL_CALL AccessibleSlideSorterObject::getBackground()
{
    ensureAlive();
    Color nColor = Application::GetSettings().GetStyleSettings().GetWindowColor();
    return sal_Int32(nColor);
}

// XServiceInfo
OUString SAL_CALL
       AccessibleSlideSorterObject::getImplementationName()
{
    return u"AccessibleSlideSorterObject"_ustr;
}

sal_Bool SAL_CALL AccessibleSlideSorterObject::supportsService (const OUString& sServiceName)
{
    return cppu::supportsService(this, sServiceName);
}

uno::Sequence< OUString> SAL_CALL
       AccessibleSlideSorterObject::getSupportedServiceNames()
{
    ensureAlive();

    return uno::Sequence<OUString> {
        u"com.sun.star.accessibility.Accessible"_ustr,
        u"com.sun.star.accessibility.AccessibleContext"_ustr
    };
}

SdPage* AccessibleSlideSorterObject::GetPage() const
{
    ::sd::slidesorter::model::SharedPageDescriptor pDescriptor(
        mrSlideSorter.GetModel().GetPageDescriptor(mnPageNumber));
    if (pDescriptor)
        return pDescriptor->GetPage();
    else
        return nullptr;
}

} // end of namespace ::accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
