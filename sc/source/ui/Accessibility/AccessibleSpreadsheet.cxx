/*************************************************************************
 *
 *  $RCSfile: AccessibleSpreadsheet.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: sab $ $Date: 2002-02-19 08:26:13 $
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


#include "AccessibleSpreadsheet.hxx"
#ifndef SC_ACCESSIBILITYHINTS_HXX
#include "AccessibilityHints.hxx"
#endif
#ifndef _SC_ACCESSIBLECELL_HXX
#include "AccessibleCell.hxx"
#endif
#ifndef SC_TABVWSH_HXX
#include "tabvwsh.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessibleSpreadsheet::ScAccessibleSpreadsheet (
        const uno::Reference<XAccessible>& rxParent,
        ScTabViewShell* pViewShell,
        sal_uInt16 nTab,
        ScSplitPos eSplitPos)
    :
    ScAccessibleTableBase (rxParent, GetDocument(pViewShell),
        ScRange(ScAddress(0, 0, nTab),ScAddress(MAXCOL + 1, MAXROW + 1, nTab))),
    mpViewShell(pViewShell),
    meSplitPos(eSplitPos)
{
    if (pViewShell)
    {
        maActiveCell = pViewShell->GetViewData()->GetCurPos();
        mpViewShell->AddAccessibilityObject(*this);
    }
}

ScAccessibleSpreadsheet::~ScAccessibleSpreadsheet ()
{
    if (mpViewShell)
        mpViewShell->RemoveAccessibilityObject(*this);
}

void ScAccessibleSpreadsheet::SetDefunc()
{
    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = NULL;
    }

    ScAccessibleTableBase::SetDefunc();
}

    //=====  SfxListener  =====================================================

void ScAccessibleSpreadsheet::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.ISA( ScAccActiveCellChangeHint ) )
    {
        const ScAccActiveCellChangeHint& rRef = (const ScAccActiveCellChangeHint&)rHint;
        ScAddress aNewCell(rRef.GetNewCellAddress());
        if (aNewCell != maActiveCell)
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::ACCESSIBLE_ACTIVE_DESCENDANT_EVENT;
            aEvent.OldValue <<= getAccessibleCellAt(maActiveCell.Row(), maActiveCell.Col());
            aEvent.NewValue <<= getAccessibleCellAt(aNewCell.Row(), aNewCell.Col());

            maActiveCell = aNewCell;

            CommitChange(aEvent);
        }
    }
}

    //=====  XAccessibleTable  ================================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleSpreadsheet::getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    ScAddress aCellAddress(static_cast<sal_uInt16>(maRange.aStart.Col() + nColumn),
        static_cast<sal_uInt16>(maRange.aStart.Row() + nRow), maRange.aStart.Tab());
    ScAccessibleCell* pAccessibleCell = new ScAccessibleCell(this, mpViewShell, aCellAddress, getAccessibleIndex(nRow, nColumn), meSplitPos);
    return pAccessibleCell;
}

    //=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleSpreadsheet::getAccessibleAt(
    const awt::Point& rPoint )
        throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    uno::Reference< XAccessible > xAccessible;
    if (mpViewShell)
    {
        sal_Int16 nX, nY;
        if (mpViewShell->GetViewData()->GetPosFromPixel( rPoint.X, rPoint.Y, meSplitPos, nX, nY))
            xAccessible = getAccessibleCellAt(nY, nX);
    }
    return xAccessible;
}

void SAL_CALL ScAccessibleSpreadsheet::grabFocus(  )
        throw (uno::RuntimeException)
{
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleComponent> xAccessibleComponent(getAccessibleParent()->getAccessibleContext(), uno::UNO_QUERY);
        if (xAccessibleComponent.is())
            xAccessibleComponent->grabFocus();
    }
}

    //=====  XAccessibleContext  ==============================================

uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessibleSpreadsheet::getAccessibleStateSet (void)
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard (maMutex);
    uno::Reference<XAccessibleStateSet> xParentStates;
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
        xParentStates = xParentContext->getAccessibleStateSet();
    }
    utl::AccessibleStateSetHelper* pStateSet = new utl::AccessibleStateSetHelper();
    if (IsDefunc(xParentStates))
        pStateSet->AddState(AccessibleStateType::DEFUNC);
    if (IsEditable(xParentStates))
        pStateSet->AddState(AccessibleStateType::EDITABLE);
    pStateSet->AddState(AccessibleStateType::ENABLED);
    pStateSet->AddState(AccessibleStateType::MULTISELECTABLE);
    pStateSet->AddState(AccessibleStateType::OPAQUE);
    pStateSet->AddState(AccessibleStateType::SELECTABLE);
    if (IsCompleteSheetSelected(xParentStates))
        pStateSet->AddState(AccessibleStateType::SELECTED);
    if (IsShowing(xParentStates))
        pStateSet->AddState(AccessibleStateType::SHOWING);
    if (IsVisible(xParentStates))
        pStateSet->AddState(AccessibleStateType::VISIBLE);
    return pStateSet;
}

    //=====  XServiceInfo  ====================================================

::rtl::OUString SAL_CALL ScAccessibleSpreadsheet::getImplementationName (void)
        throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleSpreadsheet"));
}

uno::Sequence< ::rtl::OUString> SAL_CALL
    ScAccessibleSpreadsheet::getSupportedServiceNames (void)
        throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSequence = ScAccessibleTableBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);
    ::rtl::OUString* pNames = aSequence.getArray();

    pNames[nOldSize] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("drafts.com.sun.star.AccessibleSpreadsheet"));

    return aSequence;
}

    //====  internal  =========================================================

Rectangle ScAccessibleSpreadsheet::GetBoundingBoxOnScreen()
    throw (uno::RuntimeException)
{
    Rectangle aRect;
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
        if (pWindow)
            aRect = pWindow->GetWindowExtentsRelative(NULL);
    }
    return aRect;
}

Rectangle ScAccessibleSpreadsheet::GetBoundingBox()
    throw (uno::RuntimeException)
{
    Rectangle aRect;
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
        if (pWindow)
            aRect = pWindow->GetWindowExtentsRelative(pWindow->GetAccessibleParentWindow());
    }
    return aRect;
}

sal_Bool ScAccessibleSpreadsheet::IsDefunc(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return (mpViewShell == NULL) || !getAccessibleParent().is() ||
        (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

sal_Bool ScAccessibleSpreadsheet::IsEditable(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    sal_Bool bProtected(sal_False);
    if (mpDoc && mpDoc->IsTabProtected(maRange.aStart.Tab()))
        bProtected = sal_True;
    return !bProtected;
}

sal_Bool ScAccessibleSpreadsheet::IsCompleteSheetSelected(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return sal_False;
}

sal_Bool ScAccessibleSpreadsheet::IsShowing(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::SHOWING));
}

sal_Bool ScAccessibleSpreadsheet::IsVisible(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::VISIBLE));
}

ScDocument* ScAccessibleSpreadsheet::GetDocument(ScTabViewShell* pViewShell)
{
    ScDocument* pDoc = NULL;
    if (pViewShell && pViewShell->GetViewData())
        pDoc = pViewShell->GetViewData()->GetDocument();
    return pDoc;
}

