/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include "AccessibleCell.hxx"
#include "scitems.hxx"
#include <editeng/eeitem.hxx>


#include "AccessibleText.hxx"
#include "AccessibleDocument.hxx"
#include "tabvwsh.hxx"
#include "document.hxx"
#include "attrib.hxx"
#include "miscuno.hxx"
#include "editsrc.hxx"
#include "dociter.hxx"
#include "cell.hxx"
#include "markdata.hxx"

#include <unotools/accessiblestatesethelper.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/XAccessibleTable.hpp>
#include <editeng/brshitem.hxx>
#include <comphelper/sequence.hxx>
#include <float.h>
#include <vcl/svapp.hxx>

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
    mpAccDoc(pAccDoc),
    meSplitPos(eSplitPos)
{
    if (pViewShell)
        pViewShell->AddAccessibilityObject(*this);
}

ScAccessibleCell::~ScAccessibleCell()
{
    if (!ScAccessibleContextBase::IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_atomic_increment( &m_refCount );
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
    SolarMutexGuard aGuard;
    // dispose in AccessibleStaticTextBase
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

IMPLEMENT_FORWARD_XINTERFACE2( ScAccessibleCell, ScAccessibleCellBase, AccessibleStaticTextBase )

    //=====  XTypeProvider  ===================================================

IMPLEMENT_FORWARD_XTYPEPROVIDER2( ScAccessibleCell, ScAccessibleCellBase, AccessibleStaticTextBase )

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
     SolarMutexGuard aGuard;
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
        long nSizeX, nSizeY;
        mpViewShell->GetViewData()->GetMergeSizePixel(
            maCellAddress.Col(), maCellAddress.Row(), nSizeX, nSizeY);
        aCellRect.SetSize(Size(nSizeX, nSizeY));
        aCellRect.SetPos(mpViewShell->GetViewData()->GetScrPos(maCellAddress.Col(), maCellAddress.Row(), meSplitPos, sal_True));

        Window* pWindow = mpViewShell->GetWindowByPos(meSplitPos);
        if (pWindow)
        {
            Rectangle aRect(pWindow->GetWindowExtentsRelative(pWindow->GetAccessibleParentWindow()));
            aRect.Move(-aRect.Left(), -aRect.Top());
            aCellRect = aRect.Intersection(aCellRect);
        }

        /*  #i19430# Gnopernicus reads text partly if it sticks out of the cell
            boundaries. This leads to wrong results in cases where the cell
            text is rotated, because rotation is not taken into account when
            calculating the visible part of the text. In these cases we will
            simply expand the cell size to the width of the unrotated text. */
        if (mpDoc)
        {
            const SfxInt32Item* pItem = static_cast< const SfxInt32Item* >(
                mpDoc->GetAttr( maCellAddress.Col(), maCellAddress.Row(), maCellAddress.Tab(), ATTR_ROTATE_VALUE ) );
            if( pItem && (pItem->GetValue() != 0) )
            {
                Rectangle aParaRect = GetParagraphBoundingBox();
                if( !aParaRect.IsEmpty() && (aCellRect.GetWidth() < aParaRect.GetWidth()) )
                    aCellRect.SetSize( Size( aParaRect.GetWidth(), aCellRect.GetHeight() ) );
            }
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
    SolarMutexGuard aGuard;
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
    const uno::Reference<XAccessibleStateSet>& /* rxParentStates */)
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
    sal_Bool bResult(false);
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

SAL_WNODEPRECATED_DECLARATIONS_PUSH
::std::auto_ptr< SvxEditSource > ScAccessibleCell::CreateEditSource(ScTabViewShell* pViewShell, ScAddress aCell, ScSplitPos eSplitPos)
{
    ::std::auto_ptr < ScAccessibleTextData > pAccessibleCellTextData
        ( new ScAccessibleCellTextData( pViewShell, aCell, eSplitPos, this ) );
    ::std::auto_ptr< SvxEditSource > pEditSource (new ScAccessibilityEditSource(pAccessibleCellTextData));

    return pEditSource;
}
SAL_WNODEPRECATED_DECLARATIONS_POP

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
                sal_Bool bFound(false);
                ScDetectiveRefIter aIter( (ScFormulaCell*) pCell );
                ScRange aRef;
                while ( !bFound && aIter.GetNextRef( aRef ) )
                {
                    if (aRef.In(maCellAddress))
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
            ScRange aRef;
            while ( aIter.GetNextRef( aRef ) )
            {
                AddRelation( aRef, AccessibleRelationType::CONTROLLED_BY, pRelationSet);
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
        sal_uInt32 nCount(static_cast<sal_uInt32>(rRange.aEnd.Col() -
                    rRange.aStart.Col() + 1) * (rRange.aEnd.Row() -
                    rRange.aStart.Row() + 1));
        uno::Sequence < uno::Reference < uno::XInterface > > aTargetSet( nCount );
        uno::Reference < uno::XInterface >* pTargetSet = aTargetSet.getArray();
        if (pTargetSet)
        {
            sal_uInt32 nPos(0);
            for (sal_uInt32 nRow = rRange.aStart.Row(); nRow <= sal::static_int_cast<sal_uInt32>(rRange.aEnd.Row()); ++nRow)
            {
                for (sal_uInt32 nCol = rRange.aStart.Col(); nCol <= sal::static_int_cast<sal_uInt32>(rRange.aEnd.Col()); ++nCol)
                {
                    pTargetSet[nPos] = xTable->getAccessibleCellAt(nRow, nCol);
                    ++nPos;
                }
            }
            OSL_ENSURE(nCount == nPos, "something wents wrong");
        }
        AccessibleRelation aRelation;
        aRelation.RelationType = aRelationType;
        aRelation.TargetSet = aTargetSet;
        pRelationSet->AddRelation(aRelation);
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
