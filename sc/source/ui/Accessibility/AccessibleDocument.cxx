/*************************************************************************
 *
 *  $RCSfile: AccessibleDocument.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: sab $ $Date: 2002-02-14 16:49:28 $
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


#ifndef _SC_ACCESSIBLEDOCUMENT_HXX
#include "AccessibleDocument.hxx"
#endif
#ifndef _SC_ACCESSIBLESPREADSHEET_HXX
#include "AccessibleSpreadsheet.hxx"
#endif
#ifndef SC_UNONAMES_HXX
#include "unonames.hxx"
#endif
#ifndef SC_TABVWSH_HXX
#include "tabvwsh.hxx"
#endif
#ifndef SC_ACCESSIBILITYHINTS_HXX
#include "AccessibilityHints.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_DRWLAYER_HXX
#include "drwlayer.hxx"
#endif

#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEROLE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLEEVENTID_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

    //=====  internal  ========================================================

ScAccessibleDocument::ScAccessibleDocument(
        const uno::Reference<XAccessible>& rxParent,
        ScTabViewShell* pViewShell,
        ScSplitPos eSplitPos)
    : ScAccessibleContextBase(rxParent, AccessibleRole::DOCUMENT),
    mpViewShell(pViewShell),
    meSplitPos(eSplitPos),
    mpAccessibleSpreadsheet(NULL)
{
    if (pViewShell)
        pViewShell->AddAccessibilityObject(*this);
}

ScAccessibleDocument::~ScAccessibleDocument(void)
{
    FreeAccessibleSpreadsheet();
    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
    }
}

void ScAccessibleDocument::SetDefunc()
{
    FreeAccessibleSpreadsheet();
    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = NULL;
    }

    ScAccessibleContextBase::SetDefunc();
}

    //=====  SfxListener  =====================================================

void ScAccessibleDocument::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.ISA( ScAccGridViewChangeHint ) )
    {
        const ScAccGridViewChangeHint& rRef = (const ScAccGridViewChangeHint&)rHint;
        if ((rRef.GetOldGridWin() == meSplitPos) ||
            (rRef.GetNewGridWin() == meSplitPos))
        {
            awt::FocusEvent aFocusEvent;
            aFocusEvent.Temporary = sal_False;
            if (rRef.GetOldGridWin() == meSplitPos)
            {
                aFocusEvent.NextFocus = rRef.GetNewAccessible();
                CommitFocusLost(aFocusEvent);
            }
            else
            {
                aFocusEvent.NextFocus = rRef.GetOldAccessible();
                CommitFocusGained(aFocusEvent);
            }
        }
    }
    else if (rHint.ISA( SfxSimpleHint ))
    {
        const SfxSimpleHint& rRef = (const SfxSimpleHint&)rHint;
        // only notify if child exist, otherwise it is not necessary
        if ((rRef.GetId() == SC_HINT_ACC_TABLECHANGED) &&
            mpAccessibleSpreadsheet)
        {
            AccessibleEventObject aEvent;
            aEvent.EventId = AccessibleEventId::ACCESSIBLE_CHILD_EVENT;
            aEvent.OldValue <<= GetAccessibleSpreadsheet();

            CommitChange(aEvent); // child is gone - event

            aEvent.OldValue = uno::Any();
            FreeAccessibleSpreadsheet(); // free the spreadsheet after free the reference on this object
            aEvent.NewValue <<= GetAccessibleSpreadsheet();

            CommitChange(aEvent); // there is a new child - event
        }
        else if (rRef.GetId() == SFX_HINT_DYING)
        {
            // it seems the Broadcaster is dying, since the view is dying
            SetDefunc();
        }
    }
}

    //=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleDocument::getAccessibleAt(
        const awt::Point& rPoint )
        throw (uno::RuntimeException)
{
    uno::Reference<XAccessible> xAccessible = NULL;
    SdrPage* pDrawPage = GetDrawPage();
    if (pDrawPage)
    {
        DBG_ERROR("not implemented");
    }
    else
        xAccessible = GetAccessibleSpreadsheet();
    return xAccessible;
}

void SAL_CALL ScAccessibleDocument::grabFocus(  )
        throw (uno::RuntimeException)
{
    // grab only focus if it does not have the focus and it is not hidden
    if (mpViewShell &&
        (mpViewShell->GetViewData()->GetActivePart() != meSplitPos) &&
        mpViewShell->GetWindowByPos(meSplitPos)->IsVisible())
    {
        mpViewShell->ActivatePart(meSplitPos);
    }
}

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
long SAL_CALL
    ScAccessibleDocument::getAccessibleChildCount (void)
    throw (uno::RuntimeException)
{
    sal_Int32 nShapes (0);
    SdrPage* pDrawPage = GetDrawPage();
    if (pDrawPage)
    {
        sal_uInt32 nObjCount(pDrawPage->GetObjCount());
        for (sal_uInt32 i = 0; i < nObjCount; i++)
        {
            SdrObject* pObj = pDrawPage->GetObj(i);
            if (pObj && (pObj->GetLayer != SC_LAYER_INTERN))
                nShapes++;
        }
    }
    return nShapes + 1;
}

    /// Return the specified child or NULL if index is invalid.
uno::Reference<XAccessible> SAL_CALL
    ScAccessibleDocument::getAccessibleChild (long nIndex)
    throw (uno::RuntimeException/*,
        lang::IndexOutOfBoundsException*/)
{
    uno::Reference<XAccessible> xAccessible;// = GetChild(nIndex);
    if (!xAccessible.is())
    {
        if (nIndex == 0)
            xAccessible = GetAccessibleSpreadsheet();
        else
        {
            DBG_ERROR("should return other childs here");
            // there is no child with this index at the moment
            throw lang::IndexOutOfBoundsException();
        }

        //SetChild(nIndex, xAccessible);
    }
    return xAccessible;
}

    /// Return the set of current states.
uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessibleDocument::getAccessibleStateSet (void)
    throw (uno::RuntimeException)
{
    uno::Reference<XAccessibleContext> xParentContext = getAccessibleParent()->getAccessibleContext();
    uno::Reference<XAccessibleStateSet> xParentStates = xParentContext->getAccessibleStateSet();
    utl::AccessibleStateSetHelper* pStateSet = new utl::AccessibleStateSetHelper();
    if (IsDefunc(xParentStates))
        pStateSet->AddState(AccessibleStateType::DEFUNC);
    if (IsEditable(xParentStates))
        pStateSet->AddState(AccessibleStateType::EDITABLE);
    pStateSet->AddState(AccessibleStateType::ENABLED);
    pStateSet->AddState(AccessibleStateType::OPAQUE);
    if (IsShowing(xParentStates))
        pStateSet->AddState(AccessibleStateType::SHOWING);
    if (IsVisible(xParentStates))
        pStateSet->AddState(AccessibleStateType::VISIBLE);
    return pStateSet;
}

    //=====  XServiceInfo  ====================================================

    /** Returns an identifier for the implementation of this object.
    */
::rtl::OUString SAL_CALL
    ScAccessibleDocument::getImplementationName (void)
    throw (uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleDocument"));
}


    //=====  internal  ========================================================

::rtl::OUString SAL_CALL
    ScAccessibleDocument::createAccessibleDescription (void)
    throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("This is a view of a Spreadsheet Document."));
}

::rtl::OUString SAL_CALL
    ScAccessibleDocument::createAccessibleName (void)
    throw (uno::RuntimeException)
{
    rtl::OUString sName(RTL_CONSTASCII_USTRINGPARAM ("Spreadsheet Document View "));
    sal_Int32 nNumber(sal_Int32(meSplitPos) + 1);
    sName += rtl::OUString::valueOf(nNumber);
    return sName;
}

Rectangle ScAccessibleDocument::GetBoundingBoxOnScreen()
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

Rectangle ScAccessibleDocument::GetBoundingBox()
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

sal_uInt16 ScAccessibleDocument::getVisibleTable()
{
    sal_uInt16 nVisibleTable(0);
    if (mpViewShell && mpViewShell->GetViewData())
        nVisibleTable = mpViewShell->GetViewData()->GetTabNo();
    return nVisibleTable;
}

uno::Reference < XAccessible >
    ScAccessibleDocument::GetAccessibleSpreadsheet()
{
    if (!mpAccessibleSpreadsheet && mpViewShell)
    {
        mpAccessibleSpreadsheet = new ScAccessibleSpreadsheet(this, mpViewShell, getVisibleTable(), meSplitPos);
        mpAccessibleSpreadsheet->acquire();
    }
    return mpAccessibleSpreadsheet;
}

void ScAccessibleDocument::FreeAccessibleSpreadsheet()
{
    if (mpAccessibleSpreadsheet)
    {
        mpAccessibleSpreadsheet->SetDefunc();
        mpAccessibleSpreadsheet->release();
        mpAccessibleSpreadsheet = NULL;
    }
}

SdrPage* ScAccessibleDocument::GetDrawPage()
{
    sal_uInt16 nTab(getVisibleTable());
    SdrPage* pDrawPage = NULL;
    if (mpViewShell && mpViewShell->GetViewData())
    {
        ScDocument* pDoc = mpViewShell->GetViewData()->GetDocument();
        if (pDoc && pDoc->GetDrawLayer())
        {
            ScDrawLayer* pDrawLayer = pDoc->GetDrawLayer();
            if (pDrawLayer->HasObjects() && (pDrawLayer->GetPageCount() > nTab))
                pDrawPage = pDrawLayer->GetPage(nTab);
        }
    }
    return pDrawPage;
}

sal_Bool ScAccessibleDocument::IsDefunc(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return (mpViewShell == NULL) ||
        (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

sal_Bool ScAccessibleDocument::IsEditable(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    // what is with document protection?
    return sal_True;
}

sal_Bool ScAccessibleDocument::IsShowing(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::SHOWING));
}

sal_Bool ScAccessibleDocument::IsVisible(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::VISIBLE));
}
