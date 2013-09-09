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


#include "AccessiblePageHeader.hxx"
#include "AccessiblePageHeaderArea.hxx"
#include "AccessibilityHints.hxx"
#include "prevwsh.hxx"
#include "miscuno.hxx"
#include "prevloc.hxx"
#include "document.hxx"
#include "stlpool.hxx"
#include "scitems.hxx"
#include "attrib.hxx"
#include "scresid.hxx"
#include "sc.hrc"

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>

#include <vcl/window.hxx>
#include <svl/smplhint.hxx>
#include <vcl/svapp.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <svl/style.hxx>
#include <svl/itempool.hxx>
#include <editeng/editobj.hxx>
#include <toolkit/helper/convert.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

const sal_uInt8     MAX_AREAS = 3;

//=====  internal  ============================================================
struct Acquire
{
    void operator() (ScAccessiblePageHeaderArea* pArea)
    {
        if (pArea)
            pArea->acquire();
    }
};

struct Release
{
    void operator() (ScAccessiblePageHeaderArea*& pArea)
    {
        if (pArea)
            pArea->release();
    }
};

struct Dispose
{
    void operator() (ScAccessiblePageHeaderArea*& pArea)
    {
        if (pArea)
        {
            pArea->dispose();
            pArea->release();
        }
        pArea = NULL;
    }
};

ScAccessiblePageHeader::ScAccessiblePageHeader( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell, sal_Bool bHeader, sal_Int32 nIndex ) :
ScAccessibleContextBase( rxParent, bHeader ? AccessibleRole::HEADER : AccessibleRole::FOOTER ),
    mpViewShell( pViewShell ),
    mnIndex( nIndex ),
    mbHeader( bHeader ),
    maAreas(MAX_AREAS, NULL),
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
        mpViewShell = NULL;
    }
    std::for_each(maAreas.begin(), maAreas.end(), Dispose());

    ScAccessibleContextBase::disposing();
}

//=====  SfxListener  =====================================================

void ScAccessiblePageHeader::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.ISA( SfxSimpleHint ) )
    {
        const SfxSimpleHint& rRef = (const SfxSimpleHint&)rHint;
        // only notify if child exist, otherwise it is not necessary
        if ((rRef.GetId() == SC_HINT_DATACHANGED))
        {
            ScHFAreas aOldAreas(maAreas);
            std::for_each(aOldAreas.begin(), aOldAreas.end(), Acquire());
            mnChildCount = -1;
            getAccessibleChildCount();
            for (sal_uInt8 i = 0; i < MAX_AREAS; ++i)
            {
                if ((aOldAreas[i] && maAreas[i] && !ScGlobal::EETextObjEqual(aOldAreas[i]->GetEditTextObject(), maAreas[i]->GetEditTextObject())) ||
                    (aOldAreas[i] && !maAreas[i]) || (!aOldAreas[i] && maAreas[i]))
                {
                    if (aOldAreas[i] && aOldAreas[i]->GetEditTextObject())
                    {
                        AccessibleEventObject aEvent;
                        aEvent.EventId = AccessibleEventId::CHILD;
                        aEvent.Source = uno::Reference< XAccessibleContext >(this);
                        aEvent.OldValue = uno::makeAny(uno::Reference<XAccessible>(aOldAreas[i]));

                        CommitChange(aEvent); // child gone - event
                        aOldAreas[i]->dispose();
                    }
                    if (maAreas[i] && maAreas[i]->GetEditTextObject())
                    {
                        AccessibleEventObject aEvent;
                        aEvent.EventId = AccessibleEventId::CHILD;
                        aEvent.Source = uno::Reference< XAccessibleContext >(this);
                        aEvent.NewValue = uno::makeAny(uno::Reference<XAccessible>(maAreas[i]));

                        CommitChange(aEvent); // new child - event
                    }
                }
            }
            std::for_each(aOldAreas.begin(), aOldAreas.end(), Release());
        }
        else if (rRef.GetId() == SC_HINT_ACC_VISAREACHANGED)
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::VISIBLE_DATA_CHANGED;
            aEvent.Source = uno::Reference< XAccessibleContext >(this);
            CommitChange(aEvent);
        }
    }

    ScAccessibleContextBase::Notify(rBC, rHint);
}

//=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessiblePageHeader::getAccessibleAtPoint( const awt::Point& aPoint )
                                throw (uno::RuntimeException)
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
                if (maAreas[i])
                    xRet = maAreas[i];
                else
                    ++i;
            }
        }
    }

    return xRet;
}

void SAL_CALL ScAccessiblePageHeader::grabFocus() throw (uno::RuntimeException)
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

sal_Int32 SAL_CALL ScAccessiblePageHeader::getAccessibleChildCount() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    if((mnChildCount < 0) && mpViewShell)
    {
        mnChildCount = 0;
        ScDocument* pDoc = mpViewShell->GetDocument();
        if (pDoc)
        {
            // find out how many regions (left,center, right) are with content

            SfxStyleSheetBase* pStyle = pDoc->GetStyleSheetPool()->Find(pDoc->GetPageStyle(mpViewShell->GetLocationData().GetPrintTab()), SFX_STYLE_FAMILY_PAGE);
            if (pStyle)
            {
                sal_uInt16 nPageWhichId(0);
                if (mbHeader)
                    nPageWhichId = mpViewShell->GetLocationData().IsHeaderLeft() ? ATTR_PAGE_HEADERLEFT : ATTR_PAGE_HEADERRIGHT;
                else
                    nPageWhichId = mpViewShell->GetLocationData().IsFooterLeft() ? ATTR_PAGE_FOOTERLEFT : ATTR_PAGE_FOOTERRIGHT;

                const ScPageHFItem& rPageItem = static_cast<const ScPageHFItem&>(pStyle->GetItemSet().Get(nPageWhichId));
                AddChild(rPageItem.GetLeftArea(), 0, SVX_ADJUST_LEFT);
                AddChild(rPageItem.GetCenterArea(), 1, SVX_ADJUST_CENTER);
                AddChild(rPageItem.GetRightArea(), 2, SVX_ADJUST_RIGHT);
            }
        }
    }

    return mnChildCount;
}

uno::Reference< XAccessible > SAL_CALL ScAccessiblePageHeader::getAccessibleChild( sal_Int32 nIndex )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    uno::Reference<XAccessible> xRet;

    if(mnChildCount < 0)
        getAccessibleChildCount();

    ScHFAreas::iterator aItr = maAreas.begin();
    ScHFAreas::iterator aEndItr = maAreas.end();
    while (!xRet.is() && (nIndex >= 0) && (aItr != aEndItr))
    {
        if (*aItr)
        {
            if (nIndex == 0)
                xRet = *aItr;
            else
                --nIndex;
        }
        else
            ++aItr;
    }

    if ( !xRet.is() )
        throw lang::IndexOutOfBoundsException();

    return xRet;
}

sal_Int32 SAL_CALL ScAccessiblePageHeader::getAccessibleIndexInParent() throw (uno::RuntimeException)
{
    return mnIndex;
}

uno::Reference< XAccessibleStateSet > SAL_CALL ScAccessiblePageHeader::getAccessibleStateSet()
                                throw (uno::RuntimeException)
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

OUString SAL_CALL ScAccessiblePageHeader::getImplementationName() throw(uno::RuntimeException)
{
    return OUString("ScAccessiblePageHeader");
}

uno::Sequence<OUString> SAL_CALL ScAccessiblePageHeader::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);
    OUString* pNames = aSequence.getArray();

    pNames[nOldSize] = OUString("com.sun.star.text.AccessibleHeaderFooterView");

    return aSequence;
}

//====  internal  =========================================================

OUString SAL_CALL ScAccessiblePageHeader::createAccessibleDescription(void)
                    throw (uno::RuntimeException)
{
    String sDesc(ScResId(mbHeader ? STR_ACC_HEADER_DESCR : STR_ACC_FOOTER_DESCR));
    sDesc.SearchAndReplaceAscii("%1", String(ScResId(SCSTR_UNKNOWN)));
    return OUString( sDesc );
}

OUString SAL_CALL ScAccessiblePageHeader::createAccessibleName(void)
                    throw (uno::RuntimeException)
{
    String sName(ScResId(mbHeader ? STR_ACC_HEADER_NAME : STR_ACC_FOOTER_NAME));
    sName.SearchAndReplaceAscii("%1", String(ScResId(SCSTR_UNKNOWN)));
    return OUString( sName );
}

Rectangle ScAccessiblePageHeader::GetBoundingBoxOnScreen() const throw (uno::RuntimeException)
{
    Rectangle aCellRect(GetBoundingBox());
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
        {
            Rectangle aRect = pWindow->GetWindowExtentsRelative(NULL);
            aCellRect.setX(aCellRect.getX() + aRect.getX());
            aCellRect.setY(aCellRect.getY() + aRect.getY());
        }
    }
    return aCellRect;
}

Rectangle ScAccessiblePageHeader::GetBoundingBox() const throw (uno::RuntimeException)
{
    Rectangle aRect;
    if (mpViewShell)
    {
        const ScPreviewLocationData& rData = mpViewShell->GetLocationData();
        if ( mbHeader )
            rData.GetHeaderPosition( aRect );
        else
            rData.GetFooterPosition( aRect );

        // the Rectangle could contain negative coordinates so it should be cliped
        Rectangle aClipRect(Point(0, 0), aRect.GetSize());
        Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
            aClipRect = pWindow->GetWindowExtentsRelative(pWindow->GetAccessibleParentWindow());
        aRect = aClipRect.GetIntersection(aRect);
    }
    if (aRect.IsEmpty())
        aRect.SetSize(Size(-1, -1));

    return aRect;
}

sal_Bool ScAccessiblePageHeader::IsDefunc( const uno::Reference<XAccessibleStateSet>& rxParentStates )
{
    return ScAccessibleContextBase::IsDefunc() || (mpViewShell == NULL) || !getAccessibleParent().is() ||
        (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

void ScAccessiblePageHeader::AddChild(const EditTextObject* pArea, sal_uInt32 nIndex, SvxAdjust eAdjust)
{
    if (pArea && (!pArea->GetText(0).isEmpty() || (pArea->GetParagraphCount() > 1)))
    {
        if (maAreas[nIndex])
        {
            if (!ScGlobal::EETextObjEqual(maAreas[nIndex]->GetEditTextObject(), pArea))
            {
                maAreas[nIndex]->release();
                maAreas[nIndex] = new ScAccessiblePageHeaderArea(this, mpViewShell, pArea, mbHeader, eAdjust);
                maAreas[nIndex]->acquire();
            }
        }
        else
        {
            maAreas[nIndex] = new ScAccessiblePageHeaderArea(this, mpViewShell, pArea, mbHeader, eAdjust);
            maAreas[nIndex]->acquire();
        }
        ++mnChildCount;
    }
    else
    {
        if (maAreas[nIndex])
        {
            maAreas[nIndex]->release();
            maAreas[nIndex] = NULL;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
