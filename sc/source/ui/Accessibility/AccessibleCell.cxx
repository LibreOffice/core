/*************************************************************************
 *
 *  $RCSfile: AccessibleCell.cxx,v $
 *
 *  $Revision: 1.24 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-24 17:10:07 $
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


#include "AccessibleCell.hxx"

#ifndef SC_ITEMS_HXX
#include "scitems.hxx"
#endif
#include <svx/eeitem.hxx>
#define ITEMID_FIELD EE_FEATURE_FIELD

#ifndef _SC_ACCESSIBLETEXT_HXX
#include "AccessibleText.hxx"
#endif
#ifndef _SC_ACCESSIBLEDOCUMENT_HXX
#include "AccessibleDocument.hxx"
#endif
#ifndef SC_TABVWSH_HXX
#include "tabvwsh.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_SCATTR_HXX
#include "attrib.hxx"
#endif
#ifndef SC_MISCUNO_HXX
#include "miscuno.hxx"
#endif
#ifndef SC_UNOGUARD_HXX
#include "unoguard.hxx"
#endif
#ifndef SC_EDITSRC_HXX
#include "editsrc.hxx"
#endif
#ifndef SC_DOCITER_HXX
#include "dociter.hxx"
#endif
#ifndef SC_CELL_HXX
#include "cell.hxx"
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLERELATIONTYPE_HPP_
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_XACCESSIBLETABLE_HPP_
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX
#include <svx/brshitem.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#include <float.h>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessibleCell::ScAccessibleCell(
        const uno::Reference<XAccessible>& rxParent,
        ScTabViewShell* pViewShell,
        ScAddress& rCellAddress,
        sal_Int32 nIndex,
        ScSplitPos eSplitPos,
        ScAccessibleDocument* pAccDoc)
    :
    ScAccessibleCellBase(rxParent, GetDocument(pViewShell), rCellAddress, nIndex),
        ::accessibility::AccessibleStaticTextBase(CreateEditSource(pViewShell, rCellAddress, eSplitPos)),
    mpViewShell(pViewShell),
    meSplitPos(eSplitPos),
    mpAccDoc(pAccDoc)
{
    if (pViewShell)
        pViewShell->AddAccessibilityObject(*this);
}

ScAccessibleCell::~ScAccessibleCell()
{
    if (!ScAccessibleContextBase::IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_incrementInterlockedCount( &m_refCount );
        // call dispose to inform object wich have a weak reference to this object
        dispose();
    }
}

void ScAccessibleCell::Init()
{
    ScAccessibleCellBase::Init();

    SetEventSource(this);
}

void SAL_CALL ScAccessibleCell::disposing()
{
    ScUnoGuard aGuard;
    // #100593# dispose in AccessibleStaticTextBase
    Dispose();

    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = NULL;
    }
    mpAccDoc = NULL;

    ScAccessibleCellBase::disposing();
}

    //=====  XInterface  =====================================================

uno::Any SAL_CALL ScAccessibleCell::queryInterface( uno::Type const & rType )
    throw (uno::RuntimeException)
{
    SC_QUERYINTERFACE( XAccessibleText )
    return ScAccessibleCellBase::queryInterface(rType);
}

void SAL_CALL ScAccessibleCell::acquire()
    throw ()
{
    ScAccessibleCellBase::acquire();
}

void SAL_CALL ScAccessibleCell::release()
    throw ()
{
    ScAccessibleCellBase::release();
}

    //=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleCell::getAccessibleAtPoint(
        const awt::Point& rPoint )
        throw (uno::RuntimeException)
{
    return AccessibleStaticTextBase::getAccessibleAtPoint(rPoint);
}

void SAL_CALL ScAccessibleCell::grabFocus(  )
        throw (uno::RuntimeException)
{
     ScUnoGuard aGuard;
    IsObjectValid();
    if (getAccessibleParent().is() && mpViewShell)
    {
        uno::Reference<XAccessibleComponent> xAccessibleComponent(getAccessibleParent()->getAccessibleContext(), uno::UNO_QUERY);
        if (xAccessibleComponent.is())
        {
            xAccessibleComponent->grabFocus();
            mpViewShell->SetCursor(maCellAddress.Col(), maCellAddress.Row());
        }
    }
}

Rectangle ScAccessibleCell::GetBoundingBoxOnScreen(void) const
        throw (uno::RuntimeException)
{
    Rectangle aCellRect(GetBoundingBox());
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
        if (pWindow)
        {
            Rectangle aRect = pWindow->GetWindowExtentsRelative(NULL);
            aCellRect.setX(aCellRect.getX() + aRect.getX());
            aCellRect.setY(aCellRect.getY() + aRect.getY());
        }
    }
    return aCellRect;
}

Rectangle ScAccessibleCell::GetBoundingBox(void) const
        throw (uno::RuntimeException)
{
    Rectangle aCellRect;
    if (mpViewShell)
    {
        sal_Int32 nSizeX, nSizeY;
        mpViewShell->GetViewData()->GetMergeSizePixel(
            maCellAddress.Col(), maCellAddress.Row(), nSizeX, nSizeY);
        aCellRect.SetSize(Size(nSizeX, nSizeY));
        aCellRect.SetPos(mpViewShell->GetViewData()->GetScrPos(maCellAddress.Col(), maCellAddress.Row(), meSplitPos));

        Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
        if (pWindow)
        {
            Rectangle aRect(pWindow->GetWindowExtentsRelative(pWindow->GetAccessibleParentWindow()));
            aRect.Move(-aRect.Left(), -aRect.Top());
            aCellRect = aRect.Intersection(aCellRect);
        }

    }
    if (aCellRect.IsEmpty())
        aCellRect.SetPos(Point(-1, -1));
    return aCellRect;
}

    //=====  XAccessibleContext  ==============================================

sal_Int32 SAL_CALL
    ScAccessibleCell::getAccessibleChildCount(void)
                    throw (uno::RuntimeException)
{
    return AccessibleStaticTextBase::getAccessibleChildCount();
}

uno::Reference< XAccessible > SAL_CALL
    ScAccessibleCell::getAccessibleChild(sal_Int32 nIndex)
        throw (uno::RuntimeException,
        lang::IndexOutOfBoundsException)
{
    return AccessibleStaticTextBase::getAccessibleChild(nIndex);
}

uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessibleCell::getAccessibleStateSet(void)
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
        if (IsEditable(xParentStates))
        {
            pStateSet->AddState(AccessibleStateType::EDITABLE);
            pStateSet->AddState(AccessibleStateType::RESIZABLE);
        }
        pStateSet->AddState(AccessibleStateType::ENABLED);
        pStateSet->AddState(AccessibleStateType::MULTI_LINE);
        pStateSet->AddState(AccessibleStateType::MULTI_SELECTABLE);
        if (IsOpaque(xParentStates))
            pStateSet->AddState(AccessibleStateType::OPAQUE);
        pStateSet->AddState(AccessibleStateType::SELECTABLE);
        if (IsSelected())
            pStateSet->AddState(AccessibleStateType::SELECTED);
        if (isShowing())
            pStateSet->AddState(AccessibleStateType::SHOWING);
        pStateSet->AddState(AccessibleStateType::TRANSIENT);
        if (isVisible())
            pStateSet->AddState(AccessibleStateType::VISIBLE);
    }
    return pStateSet;
}

uno::Reference<XAccessibleRelationSet> SAL_CALL
       ScAccessibleCell::getAccessibleRelationSet(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    utl::AccessibleRelationSetHelper* pRelationSet = NULL;
    if (mpAccDoc)
        pRelationSet = mpAccDoc->GetRelationSet(&maCellAddress);
    if (!pRelationSet)
        pRelationSet = new utl::AccessibleRelationSetHelper();
    FillDependends(pRelationSet);
    FillPrecedents(pRelationSet);
    return pRelationSet;
}

    //=====  XServiceInfo  ====================================================

::rtl::OUString SAL_CALL ScAccessibleCell::getImplementationName(void)
        throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM ("ScAccessibleCell"));
}

uno::Sequence< ::rtl::OUString> SAL_CALL
    ScAccessibleCell::getSupportedServiceNames(void)
        throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);
    ::rtl::OUString* pNames = aSequence.getArray();

    pNames[nOldSize] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sheet.AccessibleCell"));

    return aSequence;
}

//=====  XTypeProvider  =======================================================

uno::Sequence< uno::Type > SAL_CALL ScAccessibleCell::getTypes()
        throw (uno::RuntimeException)
{
    uno::Sequence< uno::Type > aSeq;
    aSeq.realloc(1);
    aSeq[0] = getCppuType((const uno::Reference<XAccessibleText>*)0);
    return comphelper::concatSequences(ScAccessibleCellBase::getTypes(), aSeq);
}

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleCell::getImplementationId(void)
    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    IsObjectValid();
    static uno::Sequence<sal_Int8> aId;
    if (aId.getLength() == 0)
    {
        aId.realloc (16);
        rtl_createUuid (reinterpret_cast<sal_uInt8 *>(aId.getArray()), 0, sal_True);
    }
    return aId;
}

    //====  internal  =========================================================

sal_Bool ScAccessibleCell::IsDefunc(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    return ScAccessibleContextBase::IsDefunc() || (mpDoc == NULL) || (mpViewShell == NULL) || !getAccessibleParent().is() ||
         (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

sal_Bool ScAccessibleCell::IsEditable(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    sal_Bool bEditable(sal_True);
    if (rxParentStates.is() && !rxParentStates->contains(AccessibleStateType::EDITABLE) &&
        mpDoc)
    {
        // here I have to test whether the protection of the table should influence this cell.
        const ScProtectionAttr* pItem = (const ScProtectionAttr*)mpDoc->GetAttr(
            maCellAddress.Col(), maCellAddress.Row(),
            maCellAddress.Tab(), ATTR_PROTECTION);
        if (pItem)
            bEditable = !pItem->GetProtection();
    }
    return bEditable;
}

sal_Bool ScAccessibleCell::IsOpaque(
    const uno::Reference<XAccessibleStateSet>& rxParentStates)
{
    // test whether there is a background color
    sal_Bool bOpaque(sal_True);
    if (mpDoc)
    {
        const SvxBrushItem* pItem = (const SvxBrushItem*)mpDoc->GetAttr(
            maCellAddress.Col(), maCellAddress.Row(),
            maCellAddress.Tab(), ATTR_BACKGROUND);
        if (pItem)
            bOpaque = pItem->GetColor() != COL_TRANSPARENT;
    }
    return bOpaque;
}

sal_Bool ScAccessibleCell::IsSelected()
{
    sal_Bool bResult(sal_False);
    if (mpViewShell && mpViewShell->GetViewData())
    {
        const ScMarkData& rMarkdata = mpViewShell->GetViewData()->GetMarkData();
        bResult = rMarkdata.IsCellMarked(maCellAddress.Col(), maCellAddress.Row());
    }
    return bResult;
}

ScDocument* ScAccessibleCell::GetDocument(ScTabViewShell* pViewShell)
{
    ScDocument* pDoc = NULL;
    if (pViewShell && pViewShell->GetViewData())
        pDoc = pViewShell->GetViewData()->GetDocument();
    return pDoc;
}

::std::auto_ptr< SvxEditSource > ScAccessibleCell::CreateEditSource(ScTabViewShell* pViewShell, ScAddress aCell, ScSplitPos eSplitPos)
{
    ::std::auto_ptr < ScAccessibleTextData > pAccessibleCellTextData
        (new ScAccessibleCellTextData(pViewShell, aCell, eSplitPos));
    ::std::auto_ptr< SvxEditSource > pEditSource (new ScAccessibilityEditSource(pAccessibleCellTextData));

    return pEditSource;
}

void ScAccessibleCell::FillDependends(utl::AccessibleRelationSetHelper* pRelationSet)
{
    if (mpDoc)
    {
        ScCellIterator aCellIter( mpDoc, 0,0, maCellAddress.Tab(), MAXCOL,MAXROW, maCellAddress.Tab() );
        ScBaseCell* pCell = aCellIter.GetFirst();
        while (pCell)
        {
            if (pCell->GetCellType() == CELLTYPE_FORMULA)
            {
                sal_Bool bFound(sal_False);
                ScDetectiveRefIter aIter( (ScFormulaCell*) pCell );
                ScTripel aRefStart;
                ScTripel aRefEnd;
                while ( !bFound && aIter.GetNextRef( aRefStart, aRefEnd ) )
                {
                    ScRange aRefRange( aRefStart, aRefEnd );
                    if (aRefRange.In(maCellAddress))
                        bFound = sal_True;
                }
                if (bFound)
                    AddRelation(ScAddress(aCellIter.GetCol(), aCellIter.GetRow(), aCellIter.GetTab()), AccessibleRelationType::CONTROLLER_FOR, pRelationSet);
            }
            pCell = aCellIter.GetNext();
        }
    }
}

void ScAccessibleCell::FillPrecedents(utl::AccessibleRelationSetHelper* pRelationSet)
{
    if (mpDoc)
    {
        ScBaseCell* pBaseCell = mpDoc->GetCell(maCellAddress);
        if (pBaseCell && (pBaseCell->GetCellType() == CELLTYPE_FORMULA))
        {
            ScFormulaCell* pFCell = (ScFormulaCell*) pBaseCell;

            ScDetectiveRefIter aIter( pFCell );
            ScTripel aRefStart;
            ScTripel aRefEnd;
            while ( aIter.GetNextRef( aRefStart, aRefEnd ) )
            {
                AddRelation(ScRange( aRefStart, aRefEnd ), AccessibleRelationType::CONTROLLED_BY, pRelationSet);
            }
        }
    }
}

void ScAccessibleCell::AddRelation(const ScAddress& rCell,
    const sal_uInt16 aRelationType,
    utl::AccessibleRelationSetHelper* pRelationSet)
{
    AddRelation(ScRange(rCell, rCell), aRelationType, pRelationSet);
}

void ScAccessibleCell::AddRelation(const ScRange& rRange,
    const sal_uInt16 aRelationType,
    utl::AccessibleRelationSetHelper* pRelationSet)
{
    uno::Reference < XAccessibleTable > xTable ( getAccessibleParent()->getAccessibleContext(), uno::UNO_QUERY );
    if (xTable.is())
    {
        sal_uInt32 nCount((rRange.aEnd.Col() - rRange.aStart.Col() + 1) * (rRange.aEnd.Row() - rRange.aStart.Row() + 1));
        uno::Sequence < uno::Reference < uno::XInterface > > aTargetSet( nCount );
        uno::Reference < uno::XInterface >* pTargetSet = aTargetSet.getArray();
        if (pTargetSet)
        {
            sal_uInt32 nPos(0);
            for (sal_uInt32 nRow = rRange.aStart.Row(); nRow <= rRange.aEnd.Row(); ++nRow)
            {
                for (sal_uInt32 nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
                {
                    pTargetSet[nPos] = xTable->getAccessibleCellAt(nRow, nCol);
                    ++nPos;
                }
            }
            DBG_ASSERT(nCount == nPos, "something wents wrong");
        }
        AccessibleRelation aRelation;
        aRelation.RelationType = aRelationType;
        aRelation.TargetSet = aTargetSet;
        pRelationSet->AddRelation(aRelation);
    }
}
