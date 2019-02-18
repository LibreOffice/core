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

#include <AccessiblePageHeader.hxx>
#include <AccessiblePageHeaderArea.hxx>
#include <prevwsh.hxx>
#include <prevloc.hxx>
#include <document.hxx>
#include <stlpool.hxx>
#include <scitems.hxx>
#include <attrib.hxx>
#include <scresid.hxx>
#include <strings.hrc>
#include <strings.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <vcl/window.hxx>
#include <svl/hint.hxx>
#include <vcl/svapp.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <svl/style.hxx>
#include <editeng/editobj.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

const sal_uInt8     MAX_AREAS = 3;

ScAccessiblePageHeader::ScAccessiblePageHeader( const css::uno::Reference<css::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell, bool bHeader, sal_Int32 nIndex ) :
ScAccessibleContextBase( rxParent, bHeader ? AccessibleRole::HEADER : AccessibleRole::FOOTER ),
    mpViewShell( pViewShell ),
    mnIndex( nIndex ),
    mbHeader( bHeader ),
    maAreas(MAX_AREAS, rtl::Reference<ScAccessiblePageHeaderArea>()),
    mnChildCount(-1)
{
    if (mpViewShell)
        mpViewShell->AddAccessibilityObject(*this);
}

ScAccessiblePageHeader::~ScAccessiblePageHeader()
{
    if (!ScAccessibleContextBase::IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_atomic_increment( &m_refCount );
        dispose();
    }
}

void SAL_CALL ScAccessiblePageHeader::disposing()
{
    SolarMutexGuard aGuard;
    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = nullptr;
    }
    for (auto & i : maAreas)
    {
        if (i.is())
        {
            i->dispose();
            i.clear();
        }
    }

    ScAccessibleContextBase::disposing();
}

//=====  SfxListener  =====================================================

void ScAccessiblePageHeader::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    // only notify if child exist, otherwise it is not necessary
    if (rHint.GetId() == SfxHintId::ScDataChanged)
    {
        std::vector<rtl::Reference<ScAccessiblePageHeaderArea>> aOldAreas(maAreas);
        mnChildCount = -1;
        getAccessibleChildCount();
        for (sal_uInt8 i = 0; i < MAX_AREAS; ++i)
        {
            if ((aOldAreas[i].is() && maAreas[i].is() && !ScGlobal::EETextObjEqual(aOldAreas[i]->GetEditTextObject(), maAreas[i]->GetEditTextObject())) ||
                    (aOldAreas[i].is() && !maAreas[i].is()) || (!aOldAreas[i].is() && maAreas[i].is()))
            {
                if (aOldAreas[i].is() && aOldAreas[i]->GetEditTextObject())
                {
                    AccessibleEventObject aEvent;
                    aEvent.EventId = AccessibleEventId::CHILD;
                    aEvent.Source = uno::Reference< XAccessibleContext >(this);
                    aEvent.OldValue <<= uno::Reference<XAccessible>(aOldAreas[i].get());

                    CommitChange(aEvent); // child gone - event
                    aOldAreas[i]->dispose();
                }
                if (maAreas[i].is() && maAreas[i]->GetEditTextObject())
                {
                    AccessibleEventObject aEvent;
                    aEvent.EventId = AccessibleEventId::CHILD;
                    aEvent.Source = uno::Reference< XAccessibleContext >(this);
                    aEvent.NewValue <<= uno::Reference<XAccessible>(maAreas[i].get());

                    CommitChange(aEvent); // new child - event
                }
            }
        }
    }
    else if (rHint.GetId() == SfxHintId::ScAccVisAreaChanged)
    {
        AccessibleEventObject aEvent;
        aEvent.EventId = AccessibleEventId::VISIBLE_DATA_CHANGED;
        aEvent.Source = uno::Reference< XAccessibleContext >(this);
        CommitChange(aEvent);
    }

    ScAccessibleContextBase::Notify(rBC, rHint);
}

//=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessiblePageHeader::getAccessibleAtPoint( const awt::Point& aPoint )
{
    uno::Reference<XAccessible> xRet;

    if (containsPoint(aPoint))
    {
        SolarMutexGuard aGuard;
        IsObjectValid();

        sal_Int32 nCount(getAccessibleChildCount()); // fill the areas

        if (nCount)
        {
            // return the first with content, because they have all the same Bounding Box
            sal_uInt8 i(0);
            while(!xRet.is() && i < MAX_AREAS)
            {
                if (maAreas[i].is())
                    xRet = maAreas[i].get();
                else
                    ++i;
            }
        }
    }

    return xRet;
}

void SAL_CALL ScAccessiblePageHeader::grabFocus()
{
    SolarMutexGuard aGuard;
    IsObjectValid();
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleComponent> xAccessibleComponent(getAccessibleParent()->getAccessibleContext(), uno::UNO_QUERY);
        if (xAccessibleComponent.is())
            xAccessibleComponent->grabFocus();
    }
}

//=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL ScAccessiblePageHeader::getAccessibleChildCount()
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    if((mnChildCount < 0) && mpViewShell)
    {
        mnChildCount = 0;
        ScDocument& rDoc = mpViewShell->GetDocument();
        // find out how many regions (left,center, right) are with content

        SfxStyleSheetBase* pStyle = rDoc.GetStyleSheetPool()->Find(rDoc.GetPageStyle(mpViewShell->GetLocationData().GetPrintTab()), SfxStyleFamily::Page);
        if (pStyle)
        {
            sal_uInt16 nPageWhichId(0);
            if (mbHeader)
                nPageWhichId = mpViewShell->GetLocationData().IsHeaderLeft() ? ATTR_PAGE_HEADERLEFT : ATTR_PAGE_HEADERRIGHT;
            else
                nPageWhichId = mpViewShell->GetLocationData().IsFooterLeft() ? ATTR_PAGE_FOOTERLEFT : ATTR_PAGE_FOOTERRIGHT;

            const ScPageHFItem& rPageItem = static_cast<const ScPageHFItem&>(pStyle->GetItemSet().Get(nPageWhichId));
            AddChild(rPageItem.GetLeftArea(), 0, SvxAdjust::Left);
            AddChild(rPageItem.GetCenterArea(), 1, SvxAdjust::Center);
            AddChild(rPageItem.GetRightArea(), 2, SvxAdjust::Right);
        }
    }

    return mnChildCount;
}

uno::Reference< XAccessible > SAL_CALL ScAccessiblePageHeader::getAccessibleChild( sal_Int32 nIndex )
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    uno::Reference<XAccessible> xRet;

    if(mnChildCount < 0)
        getAccessibleChildCount();

    if (nIndex >= 0)
        for (const auto& rxArea : maAreas)
        {
            if (rxArea.is())
            {
                if (nIndex == 0)
                {
                    xRet = rxArea.get();
                    break;
                }
                else
                    --nIndex;
            }
        }

    if ( !xRet.is() )
        throw lang::IndexOutOfBoundsException();

    return xRet;
}

sal_Int32 SAL_CALL ScAccessiblePageHeader::getAccessibleIndexInParent()
{
    return mnIndex;
}

uno::Reference< XAccessibleStateSet > SAL_CALL ScAccessiblePageHeader::getAccessibleStateSet()
{
    SolarMutexGuard aGuard;
    uno::Reference<XAccessibleStateSet> xParentStates;
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
        xParentStates = xParentContext->getAccessibleStateSet();
    }
    utl::AccessibleStateSetHelper* pStateSet = new utl::AccessibleStateSetHelper();
    if (IsDefunc(xParentStates))
        pStateSet->AddState(AccessibleStateType::DEFUNC);
    else
    {
        pStateSet->AddState(AccessibleStateType::ENABLED);
        pStateSet->AddState(AccessibleStateType::OPAQUE);
        if (isShowing())
            pStateSet->AddState(AccessibleStateType::SHOWING);
        if (isVisible())
            pStateSet->AddState(AccessibleStateType::VISIBLE);
    }
    return pStateSet;
}

//=====  XServiceInfo  ====================================================

OUString SAL_CALL ScAccessiblePageHeader::getImplementationName()
{
    return OUString("ScAccessiblePageHeader");
}

uno::Sequence<OUString> SAL_CALL ScAccessiblePageHeader::getSupportedServiceNames()
{
    uno::Sequence< OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);

    aSequence[nOldSize] = "com.sun.star.text.AccessibleHeaderFooterView";

    return aSequence;
}

//====  internal  =========================================================

OUString ScAccessiblePageHeader::createAccessibleDescription()
{
    OUString sDesc(mbHeader ? OUString(STR_ACC_HEADER_DESCR) : OUString(STR_ACC_FOOTER_DESCR));
    return sDesc.replaceFirst("%1", ScResId(SCSTR_UNKNOWN));
}

OUString ScAccessiblePageHeader::createAccessibleName()
{
    OUString sName(ScResId(mbHeader ? STR_ACC_HEADER_NAME : STR_ACC_FOOTER_NAME));
    return sName.replaceFirst("%1", ScResId(SCSTR_UNKNOWN));
}

tools::Rectangle ScAccessiblePageHeader::GetBoundingBoxOnScreen() const
{
    tools::Rectangle aCellRect(GetBoundingBox());
    if (mpViewShell)
    {
        vcl::Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
        {
            tools::Rectangle aRect = pWindow->GetWindowExtentsRelative(nullptr);
            aCellRect.setX(aCellRect.getX() + aRect.getX());
            aCellRect.setY(aCellRect.getY() + aRect.getY());
        }
    }
    return aCellRect;
}

tools::Rectangle ScAccessiblePageHeader::GetBoundingBox() const
{
    tools::Rectangle aRect;
    if (mpViewShell)
    {
        const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
        if ( mbHeader )
            rData.GetHeaderPosition( aRect );
        else
            rData.GetFooterPosition( aRect );

        // the Rectangle could contain negative coordinates so it should be clipped
        tools::Rectangle aClipRect(Point(0, 0), aRect.GetSize());
        vcl::Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
            aClipRect = pWindow->GetWindowExtentsRelative(pWindow->GetAccessibleParentWindow());
        aRect = aClipRect.GetIntersection(aRect);
    }
    if (aRect.IsEmpty())
        aRect.SetSize(Size(-1, -1));

    return aRect;
}

bool ScAccessiblePageHeader::IsDefunc( const uno::Reference<XAccessibleStateSet>& rxParentStates )
{
    return ScAccessibleContextBase::IsDefunc() || (mpViewShell == nullptr) || !getAccessibleParent().is() ||
        (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

void ScAccessiblePageHeader::AddChild(const EditTextObject* pArea, sal_uInt32 nIndex, SvxAdjust eAdjust)
{
    if (pArea && (!pArea->GetText(0).isEmpty() || (pArea->GetParagraphCount() > 1)))
    {
        if (maAreas[nIndex].is())
        {
            if (!ScGlobal::EETextObjEqual(maAreas[nIndex]->GetEditTextObject(), pArea))
            {
                maAreas[nIndex] = new ScAccessiblePageHeaderArea(this, mpViewShell, pArea, eAdjust);
            }
        }
        else
        {
            maAreas[nIndex] = new ScAccessiblePageHeaderArea(this, mpViewShell, pArea, eAdjust);
        }
        ++mnChildCount;
    }
    else
    {
        maAreas[nIndex].clear();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
