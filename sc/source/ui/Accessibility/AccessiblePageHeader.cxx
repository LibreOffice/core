/*************************************************************************
 *
 *  $RCSfile: AccessiblePageHeader.cxx,v $
 *
 *  $Revision: 1.17 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:11:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "AccessiblePageHeader.hxx"
#ifndef _SC_ACCESSIBLEPAGEHEADERAREA_HXX
#include "AccessiblePageHeaderArea.hxx"
#endif
#ifndef SC_ACCESSIBILITYHINTS_HXX
#include "AccessibilityHints.hxx"
#endif
#include "prevwsh.hxx"
#include "unoguard.hxx"
#include "miscuno.hxx"
#include "prevloc.hxx"
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_STLPOOL_HXX
#include "stlpool.hxx"
#endif
#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#ifndef SC_SCATTR_HXX
#include "attrib.hxx"
#endif
#ifndef SC_SCRESID_HXX
#include "scresid.hxx"
#endif
#ifndef SC_SC_HRC
#include "sc.hrc"
#endif

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif

#include <vcl/window.hxx>
#include <svtools/smplhint.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#ifndef _SFXSTYLE_HXX
#include <svtools/style.hxx>
#endif
#ifndef _SFXITEMPOOL_HXX
#include <svtools/itempool.hxx>
#endif
#ifndef _EDITOBJ_HXX
#include <svx/editobj.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif

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
        osl_incrementInterlockedCount( &m_refCount );
        dispose();
    }
}

void SAL_CALL ScAccessiblePageHeader::disposing()
{
    ScUnoGuard aGuard;
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
            sal_Int32 nOldCount(mnChildCount);
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
                        aEvent.Source = uno::Reference< XAccessible >(this);
                        aEvent.OldValue = uno::makeAny(uno::Reference<XAccessible>(aOldAreas[i]));

                        CommitChange(aEvent); // child gone - event
                        aOldAreas[i]->dispose();
                    }
                    if (maAreas[i] && maAreas[i]->GetEditTextObject())
                    {
                        AccessibleEventObject aEvent;
                        aEvent.EventId = AccessibleEventId::CHILD;
                        aEvent.Source = uno::Reference< XAccessible >(this);
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
            aEvent.Source = uno::Reference< XAccessible >(this);
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
        ScUnoGuard aGuard;
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
     ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
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

rtl::OUString SAL_CALL ScAccessiblePageHeader::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScAccessiblePageHeader"));
}

uno::Sequence<rtl::OUString> SAL_CALL ScAccessiblePageHeader::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);
    ::rtl::OUString* pNames = aSequence.getArray();

    pNames[nOldSize] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.AccessibleHeaderFooterView"));

    return aSequence;
}

//====  internal  =========================================================

::rtl::OUString SAL_CALL ScAccessiblePageHeader::createAccessibleDescription(void)
                    throw (uno::RuntimeException)
{
    String sDesc(ScResId(mbHeader ? STR_ACC_HEADER_DESCR : STR_ACC_FOOTER_DESCR));
    sDesc.SearchAndReplaceAscii("%1", String(ScResId(SCSTR_UNKNOWN)));
    return rtl::OUString( sDesc );
}

::rtl::OUString SAL_CALL ScAccessiblePageHeader::createAccessibleName(void)
                    throw (uno::RuntimeException)
{
    String sName(ScResId(mbHeader ? STR_ACC_HEADER_NAME : STR_ACC_FOOTER_NAME));
    sName.SearchAndReplaceAscii("%1", String(ScResId(SCSTR_UNKNOWN)));
    return rtl::OUString( sName );
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
    if (pArea && (pArea->GetText(0).Len() || (pArea->GetParagraphCount() > 1)))
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
