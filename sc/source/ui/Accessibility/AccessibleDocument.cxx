/*************************************************************************
 *
 *  $RCSfile: AccessibleDocument.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: vg $ $Date: 2002-04-02 14:59:56 $
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
#ifndef SC_UNOGUARD_HXX
#include "unoguard.hxx"
#endif
#ifndef SC_SHAPEUNO_HXX
#include "shapeuno.hxx"
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
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_SHAPE_TYPE_HANDLER_HXX
#include <svx/ShapeTypeHandler.hxx>
#endif
#ifndef _SVX_ACCESSIBILITY_ACCESSIBLE_SHAPE_HXX
#include <svx/AccessibleShape.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

    //=====  internal  ========================================================

ScAccessibleDocument::ScAccessibleDocument(
        const uno::Reference<XAccessible>& rxParent,
        ScTabViewShell* pViewShell,
        ScSplitPos eSplitPos)
    : ScAccessibleDocumentBase(rxParent),
    mpViewShell(pViewShell),
    meSplitPos(eSplitPos),
    mpAccessibleSpreadsheet(NULL)
{
    if (pViewShell)
        pViewShell->AddAccessibilityObject(*this);
}

ScAccessibleDocument::~ScAccessibleDocument(void)
{
    if (!ScAccessibleContextBase::IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_incrementInterlockedCount( &m_refCount );
        dispose();
    }
}

void SAL_CALL ScAccessibleDocument::disposing()
{
    FreeAccessibleSpreadsheet();
    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = NULL;
    }

    ScAccessibleDocumentBase::disposing();
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
            aEvent.Source = uno::Reference< XAccessible >(this);
            aEvent.OldValue <<= GetAccessibleSpreadsheet();

            CommitChange(aEvent); // child is gone - event

            aEvent.OldValue = uno::Any();
            FreeAccessibleSpreadsheet(); // free the spreadsheet after free the reference on this object
            aEvent.NewValue <<= GetAccessibleSpreadsheet();

            CommitChange(aEvent); // there is a new child - event
        }
    }

    ScAccessibleDocumentBase::Notify(rBC, rHint);
}

    //=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleDocument::getAccessibleAt(
        const awt::Point& rPoint )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;
    if (getAccessibleParent().is())
    {
        uno::Reference<XAccessibleComponent> xAccessibleComponent(getAccessibleParent()->getAccessibleContext(), uno::UNO_QUERY);
        if (xAccessibleComponent.is())
        {
            xAccessibleComponent->grabFocus();
            // grab only focus if it does not have the focus and it is not hidden
            if (mpViewShell && mpViewShell->GetViewData() &&
                (mpViewShell->GetViewData()->GetActivePart() != meSplitPos) &&
                mpViewShell->GetWindowByPos(meSplitPos)->IsVisible())
            {
                mpViewShell->ActivatePart(meSplitPos);
            }
        }
    }
}

    //=====  XAccessibleContext  ==============================================

    /// Return the number of currently visible children.
sal_Int32 SAL_CALL
    ScAccessibleDocument::getAccessibleChildCount(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    return GetShapesCount() + 1;
}

    /// Return the specified child or NULL if index is invalid.
uno::Reference<XAccessible> SAL_CALL
    ScAccessibleDocument::getAccessibleChild(sal_Int32 nIndex)
    throw (uno::RuntimeException,
        lang::IndexOutOfBoundsException)
{
    ScUnoGuard aGuard;
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
    ScAccessibleDocument::getAccessibleStateSet(void)
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
    if (IsEditable(xParentStates))
        pStateSet->AddState(AccessibleStateType::EDITABLE);
    pStateSet->AddState(AccessibleStateType::ENABLED);
    pStateSet->AddState(AccessibleStateType::OPAQUE);
    if (isShowing())
        pStateSet->AddState(AccessibleStateType::SHOWING);
    if (isVisible())
        pStateSet->AddState(AccessibleStateType::VISIBLE);
    return pStateSet;
}

    //=====  XServiceInfo  ====================================================

::rtl::OUString SAL_CALL
    ScAccessibleDocument::getImplementationName(void)
    throw (uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleDocument"));
}

uno::Sequence< ::rtl::OUString> SAL_CALL
    ScAccessibleDocument::getSupportedServiceNames(void)
        throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);
    ::rtl::OUString* pNames = aSequence.getArray();

    pNames[nOldSize] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("drafts.com.sun.star.AccessibleSpreadsheetDocumentView"));

    return aSequence;
}

//=====  XTypeProvider  =======================================================

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleDocument::getImplementationId(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    static uno::Sequence<sal_Int8> aId;
    if (aId.getLength() == 0)
    {
        aId.realloc (16);
        rtl_createUuid (reinterpret_cast<sal_uInt8 *>(aId.getArray()), 0, sal_True);
    }
    return aId;
}

    //=====  internal  ========================================================

::rtl::OUString SAL_CALL
    ScAccessibleDocument::createAccessibleDescription(void)
    throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("This is a view of a Spreadsheet Document."));
}

::rtl::OUString SAL_CALL
    ScAccessibleDocument::createAccessibleName(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
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
        mpAccessibleSpreadsheet->Init();
    }
    return mpAccessibleSpreadsheet;
}

void ScAccessibleDocument::FreeAccessibleSpreadsheet()
{
    if (mpAccessibleSpreadsheet)
    {
        mpAccessibleSpreadsheet->dispose();
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

sal_Int32 ScAccessibleDocument::GetShapesCount()
{
    SdrPage* pDrawPage = GetDrawPage();
    if (pDrawPage && maShapes.empty())
    {
        sal_Int32 nObjCount(pDrawPage->GetObjCount());
        maShapes.reserve(nObjCount);
        for (sal_Int32 i = 0; i < nObjCount; ++i)
        {
            SdrObject* pObj = pDrawPage->GetObj(i);
            if (pObj && (pObj->GetLayer() != SC_LAYER_INTERN))
            {
                ScAccessibleShapeData aShape;
                aShape.nIndex = i;
                maShapes.push_back(aShape);
            }
        }
    }
    return maShapes.size();
}

uno::Reference< XAccessible > ScAccessibleDocument::GetShape(sal_Int32 nIndex)
{
    if (maShapes.empty())
        GetShapesCount(); // fill list with filtered shapes (no internal shapes)

    if (static_cast<sal_uInt32>(nIndex) >= maShapes.size())
        throw lang::IndexOutOfBoundsException();

    if (!maShapes[nIndex].pAccShape)
    {
        SdrPage* pDrawPage = GetDrawPage();
        if (pDrawPage)
        {
            SdrObject* pObj = pDrawPage->GetObj(nIndex);
            if (pObj)
            {
                uno::Reference< drawing::XShape > xShape (pObj->getUnoShape(), uno::UNO_QUERY);
                new ScShapeObj(xShape);
                accessibility::ShapeTypeHandler& rShapeHandler = accessibility::ShapeTypeHandler::Instance();
                //maShapes[nIndex].pAccShape = rShapeHandler.CreateAccessibleObject(
                //  xShape, this, maShapeTreeInfo);
            }
        }
    }
    return maShapes[nIndex].pAccShape;
}

sal_Bool ScAccessibleDocument::IsDefunc(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return ScAccessibleContextBase::IsDefunc() || (mpViewShell == NULL) || !getAccessibleParent().is() ||
        (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

sal_Bool ScAccessibleDocument::IsEditable(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    // what is with document protection or readonly documents?
    return sal_True;
}
