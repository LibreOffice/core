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
#include <comphelper/sequence.hxx>

#include <vcl/window.hxx>
#include <svl/hint.hxx>
#include <svl/itemset.hxx>
#include <vcl/svapp.hxx>
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
                    aEvent.OldValue <<= uno::Reference<XAccessible>(aOldAreas[i]);
                    aEvent.IndexHint = -1;

                    CommitChange(aEvent); // child gone - event
                    aOldAreas[i]->dispose();
                }
                if (maAreas[i].is() && maAreas[i]->GetEditTextObject())
                {
                    AccessibleEventObject aEvent;
                    aEvent.EventId = AccessibleEventId::CHILD;
                    aEvent.Source = uno::Reference< XAccessibleContext >(this);
                    aEvent.NewValue <<= uno::Reference<XAccessible>(maAreas[i]);
                    aEvent.IndexHint = -1;

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

        sal_Int64 nCount(getAccessibleChildCount()); // fill the areas

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

sal_Int64 SAL_CALL ScAccessiblePageHeader::getAccessibleChildCount()
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

uno::Reference< XAccessible > SAL_CALL ScAccessiblePageHeader::getAccessibleChild( sal_Int64 nIndex )
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

sal_Int64 SAL_CALL ScAccessiblePageHeader::getAccessibleIndexInParent()
{
    return mnIndex;
}

sal_Int64 SAL_CALL ScAccessiblePageHeader::getAccessibleStateSet()
{
    SolarMutexGuard aGuard;
    sal_Int64 nParentStates = 0;
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
        nParentStates = xParentContext->getAccessibleStateSet();
    }
    sal_Int64 nStateSet = 0;
    if (IsDefunc(nParentStates))
        nStateSet |= AccessibleStateType::DEFUNC;
    else
    {
        nStateSet |= AccessibleStateType::ENABLED;
        nStateSet |= AccessibleStateType::OPAQUE;
        if (isShowing())
            nStateSet |= AccessibleStateType::SHOWING;
        if (isVisible())
            nStateSet |= AccessibleStateType::VISIBLE;
    }
    return nStateSet;
}

//=====  XServiceInfo  ====================================================

OUString SAL_CALL ScAccessiblePageHeader::getImplementationName()
{
    return "ScAccessiblePageHeader";
}

uno::Sequence<OUString> SAL_CALL ScAccessiblePageHeader::getSupportedServiceNames()
{
    const css::uno::Sequence<OUString> vals { "com.sun.star.text.AccessibleHeaderFooterView" };
    return comphelper::concatSequences(ScAccessibleContextBase::getSupportedServiceNames(), vals);
}

//====  internal  =========================================================

OUString ScAccessiblePageHeader::createAccessibleDescription()
{
    OUString sDesc(mbHeader ? STR_ACC_HEADER_DESCR : STR_ACC_FOOTER_DESCR);
    return sDesc.replaceFirst("%1", ScResId(SCSTR_UNKNOWN));
}

OUString ScAccessiblePageHeader::createAccessibleName()
{
    OUString sName(ScResId(mbHeader ? STR_ACC_HEADER_NAME : STR_ACC_FOOTER_NAME));
    return sName.replaceFirst("%1", ScResId(SCSTR_UNKNOWN));
}

AbsoluteScreenPixelRectangle ScAccessiblePageHeader::GetBoundingBoxOnScreen() const
{
    tools::Rectangle aCellRect(GetBoundingBox());
    if (mpViewShell)
    {
        vcl::Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
        {
            AbsoluteScreenPixelRectangle aRect = pWindow->GetWindowExtentsAbsolute();
            aCellRect.Move(aRect.Left(), aRect.Top());
        }
    }
    return AbsoluteScreenPixelRectangle(aCellRect);
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
            aClipRect = pWindow->GetWindowExtentsRelative(*pWindow->GetAccessibleParentWindow());
        aRect = aClipRect.GetIntersection(aRect);
    }
    if (aRect.IsEmpty())
        aRect.SetSize(Size(-1, -1));

    return aRect;
}

bool ScAccessiblePageHeader::IsDefunc( sal_Int64 nParentStates )
{
    return ScAccessibleContextBase::IsDefunc() || (mpViewShell == nullptr) || !getAccessibleParent().is() ||
        (nParentStates & AccessibleStateType::DEFUNC);
}

void ScAccessiblePageHeader::AddChild(const EditTextObject* pArea, sal_uInt32 nIndex, SvxAdjust eAdjust)
{
    if (pArea && ((pArea->GetParagraphCount() > 1) || pArea->HasText(0)))
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
