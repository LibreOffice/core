/*************************************************************************
 *
 *  $RCSfile: AccessibleSpreadsheet.cxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: sab $ $Date: 2002-05-24 15:09:52 $
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
#ifndef _SC_ACCESSIBLEDOCUMENT_HXX
#include "AccessibleDocument.hxx"
#endif
#ifndef SC_TABVWSH_HXX
#include "tabvwsh.hxx"
#endif
#ifndef SC_DOCUMENT_HXX
#include "document.hxx"
#endif
#ifndef SC_UNOGUARD_HXX
#include "unoguard.hxx"
#endif
#ifndef SC_HINTS_HXX
#include "hints.hxx"
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
#ifndef _DRAFTS_COM_SUN_STAR_ACCESSIBILITY_ACCESSIBLETABLEMODELCHANGETYPE_HPP_
#include <drafts/com/sun/star/accessibility/AccessibleTableModelChangeType.hpp>
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

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessibleSpreadsheet::ScAccessibleSpreadsheet(
        ScAccessibleDocument* pAccDoc,
        ScTabViewShell* pViewShell,
        sal_uInt16 nTab,
        ScSplitPos eSplitPos)
    :
    ScAccessibleTableBase (pAccDoc, GetDocument(pViewShell),
        ScRange(ScAddress(0, 0, nTab),ScAddress(MAXCOL, MAXROW, nTab))),
    mpViewShell(pViewShell),
    mpMarkedRanges(NULL),
    mpSortedMarkedCells(NULL),
    maVisCells(GetVisCells(GetVisArea(pViewShell, eSplitPos))),
    meSplitPos(eSplitPos),
    mbHasSelection(sal_False),
    mpAccDoc(pAccDoc)
{
    if (pViewShell)
    {
        pViewShell->AddAccessibilityObject(*this);

        maActiveCell = pViewShell->GetViewData()->GetCurPos();
        mbHasSelection = pViewShell->GetViewData()->GetMarkData().GetTableSelect(maActiveCell.Tab()) &&
                    (pViewShell->GetViewData()->GetMarkData().IsMarked() ||
                    pViewShell->GetViewData()->GetMarkData().IsMultiMarked());
    }
}

ScAccessibleSpreadsheet::~ScAccessibleSpreadsheet()
{
    if (mpMarkedRanges)
        delete mpMarkedRanges;
    if (mpSortedMarkedCells)
        delete mpSortedMarkedCells;
    if (mpViewShell)
        mpViewShell->RemoveAccessibilityObject(*this);
}

void SAL_CALL ScAccessibleSpreadsheet::disposing()
{
    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = NULL;
    }

    ScAccessibleTableBase::disposing();
}

void ScAccessibleSpreadsheet::CompleteSelectionChanged(sal_Bool bNewState)
{
    if (mpMarkedRanges)
        DELETEZ(mpMarkedRanges);
    if (mpSortedMarkedCells)
        DELETEZ(mpSortedMarkedCells);

    mbHasSelection = bNewState;

    AccessibleEventObject aEvent;
    aEvent.EventId = AccessibleEventId::ACCESSIBLE_STATE_EVENT;
    if (bNewState)
        aEvent.NewValue = uno::makeAny(AccessibleStateType::SELECTED);
    else
        aEvent.OldValue = uno::makeAny(AccessibleStateType::SELECTED);
    aEvent.Source = uno::Reference< XAccessible >(this);

    CommitChange(aEvent);
}

    //=====  SfxListener  =====================================================

void ScAccessibleSpreadsheet::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.ISA( SfxSimpleHint ) )
    {
        const SfxSimpleHint& rRef = (const SfxSimpleHint&)rHint;
        // only notify if child exist, otherwise it is not necessary
        if ((rRef.GetId() == SC_HINT_ACC_CURSORCHANGED))
        {
            if (mpViewShell)
            {
                ScAddress aNewCell = mpViewShell->GetViewData()->GetCurPos();
                sal_Bool bNewMarked(mpViewShell->GetViewData()->GetMarkData().GetTableSelect(aNewCell.Tab()) &&
                    (mpViewShell->GetViewData()->GetMarkData().IsMarked() ||
                    mpViewShell->GetViewData()->GetMarkData().IsMultiMarked()));
                sal_Bool bNewCellSelected(isAccessibleSelected(aNewCell.Row(), aNewCell.Col()));
                if ((bNewMarked != mbHasSelection) ||
                    (!bNewCellSelected && bNewMarked) ||
                    (bNewCellSelected && mbHasSelection))
                {
                    if (mpMarkedRanges)
                        DELETEZ(mpMarkedRanges);
                    if (mpSortedMarkedCells)
                        DELETEZ(mpSortedMarkedCells);
                    AccessibleEventObject aEvent;
                    aEvent.EventId = AccessibleEventId::ACCESSIBLE_SELECTION_EVENT;
                    aEvent.Source = uno::Reference< XAccessible >(this);

                    mbHasSelection = bNewMarked;

                    CommitChange(aEvent);
                }

                if ((aNewCell != maActiveCell) && (aNewCell.Tab() == maActiveCell.Tab()))
                {
                    AccessibleEventObject aEvent;
                    aEvent.EventId = AccessibleEventId::ACCESSIBLE_ACTIVE_DESCENDANT_EVENT;
                    aEvent.Source = uno::Reference< XAccessible >(this);
                    aEvent.OldValue <<= getAccessibleCellAt(maActiveCell.Row(), maActiveCell.Col());
                    aEvent.NewValue <<= getAccessibleCellAt(aNewCell.Row(), aNewCell.Col());

                    maActiveCell = aNewCell;

                    CommitChange(aEvent);
                }
            }
        }
        else if ((rRef.GetId() == SC_HINT_DATACHANGED))
        {
            if (!mbDelIns)
                CommitTableModelChange(maRange.aStart.Row(), maRange.aStart.Col(), maRange.aEnd.Row(), maRange.aEnd.Col(), AccessibleTableModelChangeType::UPDATE);
            else
                mbDelIns = sal_False;
        }
        // commented out, because to use a ModelChangeEvent is not the right way
        // at the moment there is no way, but the Java/Gnome Api should be extended sometime
/*      else if (rRef.GetId() == SC_HINT_ACC_VISAREACHANGED)
        {
            if (mpViewShell)
            {
                Rectangle aNewVisCells(GetVisCells(GetVisArea(mpViewShell, meSplitPos)));

                Rectangle aNewPos(aNewVisCells);

                if (aNewVisCells.IsOver(maVisCells))
                    aNewPos.Union(maVisCells);
                else
                    CommitTableModelChange(maVisCells.Top(), maVisCells.Left(), maVisCells.Bottom(), maVisCells.Right(), AccessibleTableModelChangeType::UPDATE);

                maVisCells = aNewVisCells;

                CommitTableModelChange(aNewPos.Top(), aNewPos.Left(), aNewPos.Bottom(), aNewPos.Right(), AccessibleTableModelChangeType::UPDATE);
            }
        }*/
    }
    else if (rHint.ISA( ScUpdateRefHint ))
    {
        const ScUpdateRefHint& rRef = (const ScUpdateRefHint&)rHint;
        if (rRef.GetMode() == URM_INSDEL)
        {
            if (((rRef.GetRange().aStart.Col() == maRange.aStart.Col()) &&
                (rRef.GetRange().aEnd.Col() == maRange.aEnd.Col())) ||
                ((rRef.GetRange().aStart.Row() == maRange.aStart.Row()) &&
                (rRef.GetRange().aEnd.Row() == maRange.aEnd.Row())))
            {
                // ignore next SC_HINT_DATACHANGED notification
                mbDelIns = sal_True;

                sal_Int16 nId(0);
                if ((rRef.GetDx() < 0) || (rRef.GetDy() < 0))
                    nId = AccessibleTableModelChangeType::DELETE;
                else if ((rRef.GetDx() > 0) || (rRef.GetDy() > 0))
                    nId = AccessibleTableModelChangeType::INSERT;
                else
                    DBG_ERROR("is it a deletion or a insertion?");

                CommitTableModelChange(rRef.GetRange().aStart.Row(), rRef.GetRange().aStart.Col(), rRef.GetRange().aEnd.Row(), rRef.GetRange().aEnd.Col(), nId);
            }
        }
    }

    ScAccessibleTableBase::Notify(rBC, rHint);
}

    //=====  XAccessibleTable  ================================================

uno::Sequence< sal_Int32 > SAL_CALL ScAccessibleSpreadsheet::getSelectedAccessibleRows(  )
                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Sequence<sal_Int32> aSequence;
    if (mpViewShell && mpViewShell->GetViewData())
    {
        aSequence.realloc(maRange.aEnd.Col() - maRange.aStart.Col() + 1);
        const ScMarkData& rMarkdata = mpViewShell->GetViewData()->GetMarkData();
        sal_Int32* pSequence = aSequence.getArray();
        sal_Int32 nCount(0);
        for (sal_uInt16 i = maRange.aStart.Row(); i <= maRange.aEnd.Row(); ++i)
        {
            if (rMarkdata.IsRowMarked(i))
            {
                pSequence[nCount] = i;
                ++nCount;
            }
        }
        aSequence.realloc(nCount);
    }
    else
        aSequence.realloc(0);
    return aSequence;
}

uno::Sequence< sal_Int32 > SAL_CALL ScAccessibleSpreadsheet::getSelectedAccessibleColumns(  )
                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Sequence<sal_Int32> aSequence;
    if (mpViewShell && mpViewShell->GetViewData())
    {
        aSequence.realloc(maRange.aEnd.Col() - maRange.aStart.Col() + 1);
        const ScMarkData& rMarkdata = mpViewShell->GetViewData()->GetMarkData();
        sal_Int32* pSequence = aSequence.getArray();
        sal_Int32 nCount(0);
        for (sal_uInt16 i = maRange.aStart.Col(); i <= maRange.aEnd.Col(); ++i)
        {
            if (rMarkdata.IsColumnMarked(i))
            {
                pSequence[nCount] = i;
                ++nCount;
            }
        }
        aSequence.realloc(nCount);
    }
    else
        aSequence.realloc(0);
    return aSequence;
}

sal_Bool SAL_CALL ScAccessibleSpreadsheet::isAccessibleRowSelected( sal_Int32 nRow )
                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    sal_Bool bResult(sal_False);
    if (mpViewShell && mpViewShell->GetViewData())
    {
        const ScMarkData& rMarkdata = mpViewShell->GetViewData()->GetMarkData();
        bResult = rMarkdata.IsRowMarked((sal_uInt16)nRow);
    }
    return bResult;
}

sal_Bool SAL_CALL ScAccessibleSpreadsheet::isAccessibleColumnSelected( sal_Int32 nColumn )
                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    sal_Bool bResult(sal_False);
    if (mpViewShell && mpViewShell->GetViewData())
    {
        const ScMarkData& rMarkdata = mpViewShell->GetViewData()->GetMarkData();
        bResult = rMarkdata.IsColumnMarked((sal_uInt16)nColumn);
    }
    return bResult;
}

uno::Reference< XAccessible > SAL_CALL ScAccessibleSpreadsheet::getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    ScAddress aCellAddress(static_cast<sal_uInt16>(maRange.aStart.Col() + nColumn),
        static_cast<sal_uInt16>(maRange.aStart.Row() + nRow), maRange.aStart.Tab());
    ScAccessibleCell* pAccessibleCell = new ScAccessibleCell(this, mpViewShell, aCellAddress, getAccessibleIndex(nRow, nColumn), meSplitPos, mpAccDoc);
    uno::Reference < XAccessible > xAccessible = pAccessibleCell;
    pAccessibleCell->Init();
    return xAccessible;
}

sal_Bool SAL_CALL ScAccessibleSpreadsheet::isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
                    throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    sal_Bool bResult(sal_False);
    if (mpViewShell)
    {
        const ScMarkData& rMarkdata = mpViewShell->GetViewData()->GetMarkData();
        bResult = rMarkdata.IsCellMarked(static_cast<sal_uInt16>(nColumn), static_cast<sal_uInt16>(nRow));
    }
    return bResult;
}

    //=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessibleSpreadsheet::getAccessibleAt(
    const awt::Point& rPoint )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference< XAccessible > xAccessible;
    if (mpViewShell)
    {
        sal_Int16 nX, nY;
        mpViewShell->GetViewData()->GetPosFromPixel( rPoint.X, rPoint.Y, meSplitPos, nX, nY);
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

uno::Reference<XAccessibleRelationSet> SAL_CALL ScAccessibleSpreadsheet::getAccessibleRelationSet(void)
        throw (::com::sun::star::uno::RuntimeException)
{
    utl::AccessibleRelationSetHelper* pRelationSet = NULL;
    if(mpAccDoc)
        pRelationSet = mpAccDoc->GetRelationSet(NULL);
    return pRelationSet;
}

uno::Reference<XAccessibleStateSet> SAL_CALL
    ScAccessibleSpreadsheet::getAccessibleStateSet(void)
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
    pStateSet->AddState(AccessibleStateType::MULTISELECTABLE);
    pStateSet->AddState(AccessibleStateType::OPAQUE);
    pStateSet->AddState(AccessibleStateType::SELECTABLE);
    if (IsCompleteSheetSelected())
        pStateSet->AddState(AccessibleStateType::SELECTED);
    if (isShowing())
        pStateSet->AddState(AccessibleStateType::SHOWING);
    if (isVisible())
        pStateSet->AddState(AccessibleStateType::VISIBLE);
    return pStateSet;
}

    ///=====  XAccessibleSelection  ===========================================

void SAL_CALL
        ScAccessibleSpreadsheet::selectAccessibleChild( sal_Int32 nChildIndex )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (mpViewShell)
    {
        sal_Int32 nCol(getAccessibleColumn(nChildIndex));
        sal_Int32 nRow(getAccessibleRow(nChildIndex));

        SelectCell(nRow, nCol, sal_False);
    }
}

void SAL_CALL
        ScAccessibleSpreadsheet::clearAccessibleSelection(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (mpViewShell)
    {
        mpViewShell->Unmark();
    }
}

void SAL_CALL
        ScAccessibleSpreadsheet::selectAllAccessible(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (mpViewShell)
    {
        mpViewShell->SelectAll();
    }
}

sal_Int32 SAL_CALL
        ScAccessibleSpreadsheet::getSelectedAccessibleChildCount(  )
        throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;
    sal_Int32 nResult(0);
    if (mpViewShell)
    {
        if (!mpMarkedRanges)
        {
            mpMarkedRanges = new ScRangeList();
            mpViewShell->GetViewData()->GetMarkData().FillRangeListWithMarks(mpMarkedRanges, sal_False);
        }
        // is possible, because there shouldn't be overlapped ranges in it
        if (mpMarkedRanges)
            nResult = mpMarkedRanges->GetCellCount();
    }
    return nResult;
}

uno::Reference<XAccessible > SAL_CALL
        ScAccessibleSpreadsheet::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    uno::Reference < XAccessible > xAccessible;
    if (mpViewShell)
    {
        if (!mpMarkedRanges)
        {
            mpMarkedRanges = new ScRangeList();
            mpViewShell->GetViewData()->GetMarkData().FillRangeListWithMarks(mpMarkedRanges, sal_False);
        }
        if (mpMarkedRanges)
        {
            if (!mpSortedMarkedCells)
                CreateSortedMarkedCells();
            if (mpSortedMarkedCells)
            {
                if ((nSelectedChildIndex < 0) ||
                    (mpSortedMarkedCells->size() <= static_cast<sal_uInt32>(nSelectedChildIndex)))
                    throw lang::IndexOutOfBoundsException();
                else
                    xAccessible = getAccessibleCellAt((*mpSortedMarkedCells)[nSelectedChildIndex].Row(), (*mpSortedMarkedCells)[nSelectedChildIndex].Col());
            }
        }
    }
    return xAccessible;
}

void SAL_CALL
        ScAccessibleSpreadsheet::deselectSelectedAccessibleChild( sal_Int32 nChildIndex )
        throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ScUnoGuard aGuard;
    if (mpViewShell)
    {
        sal_Int32 nCol(getAccessibleColumn(nChildIndex));
        sal_Int32 nRow(getAccessibleRow(nChildIndex));

        if (mpViewShell->GetViewData()->GetMarkData().IsCellMarked(static_cast<sal_uInt16>(nCol), static_cast<sal_uInt16>(nRow)))
            SelectCell(nRow, nCol, sal_True);
    }
}

void ScAccessibleSpreadsheet::SelectCell(sal_Int32 nRow, sal_Int32 nCol, sal_Bool bDeselect)
{
    mpViewShell->SetTabNo( maRange.aStart.Tab() );

    mpViewShell->DoneBlockMode( sal_True ); // continue selecting
    mpViewShell->InitBlockMode( static_cast<sal_uInt16>(nCol), static_cast<sal_uInt16>(nRow), maRange.aStart.Tab(), bDeselect, sal_False, sal_False );

    mpViewShell->SelectionChanged();
}

void ScAccessibleSpreadsheet::CreateSortedMarkedCells()
{
    mpSortedMarkedCells = new std::vector<ScMyAddress>();
    mpSortedMarkedCells->reserve(mpMarkedRanges->GetCellCount());
    ScRange* pRange = mpMarkedRanges->First();
    while (pRange)
    {
        if (pRange->aStart.Tab() != pRange->aEnd.Tab())
        {
            if ((maActiveCell.Tab() >= pRange->aStart.Tab()) ||
                maActiveCell.Tab() <= pRange->aEnd.Tab())
            {
                ScRange aRange(*pRange);
                aRange.aStart.SetTab(maActiveCell.Tab());
                aRange.aEnd.SetTab(maActiveCell.Tab());
                AddMarkedRange(aRange);
            }
            else
                DBG_ERROR("Range of wrong table");
        }
        else if(pRange->aStart.Tab() == maActiveCell.Tab())
            AddMarkedRange(*pRange);
        else
            DBG_ERROR("Range of wrong table");
        pRange = mpMarkedRanges->Next();
    }
    std::sort(mpSortedMarkedCells->begin(), mpSortedMarkedCells->end());
}

void ScAccessibleSpreadsheet::AddMarkedRange(const ScRange& rRange)
{
    sal_uInt32 nCount((rRange.aEnd.Col() - rRange.aStart.Col() + 1) * (rRange.aEnd.Row() - rRange.aStart.Row() + 1));
    sal_uInt32 nPos(0);
    for (sal_uInt16 nRow = rRange.aStart.Row(); nRow <= rRange.aEnd.Row(); ++nRow)
    {
        for (sal_uInt16 nCol = rRange.aStart.Col(); nCol <= rRange.aEnd.Col(); ++nCol)
        {
            ScMyAddress aCell(nCol, nRow, maActiveCell.Tab());
            mpSortedMarkedCells->push_back(aCell);
            ++nPos;
        }
    }
    DBG_ASSERT(nCount == nPos, "something wents wrong");
}

    //=====  XServiceInfo  ====================================================

::rtl::OUString SAL_CALL ScAccessibleSpreadsheet::getImplementationName(void)
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

//=====  XTypeProvider  =======================================================

uno::Sequence<sal_Int8> SAL_CALL
    ScAccessibleSpreadsheet::getImplementationId(void)
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

    //====  internal  =========================================================

Rectangle ScAccessibleSpreadsheet::GetBoundingBoxOnScreen() const
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

Rectangle ScAccessibleSpreadsheet::GetBoundingBox() const
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
    return ScAccessibleContextBase::IsDefunc() || (mpViewShell == NULL) || !getAccessibleParent().is() ||
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

sal_Bool ScAccessibleSpreadsheet::IsCompleteSheetSelected()
{
    sal_Bool bResult(sal_False);
    if(mpViewShell)
    {
        mpViewShell->GetViewData()->GetMarkData().MarkToMulti();
        if (mpViewShell->GetViewData()->GetMarkData().IsAllMarked(maRange))
            bResult = sal_True;
    }
    return bResult;
}

ScDocument* ScAccessibleSpreadsheet::GetDocument(ScTabViewShell* pViewShell)
{
    ScDocument* pDoc = NULL;
    if (pViewShell)
        pDoc = pViewShell->GetViewData()->GetDocument();
    return pDoc;
}

Rectangle ScAccessibleSpreadsheet::GetVisArea(ScTabViewShell* pViewShell, ScSplitPos eSplitPos)
{
    Rectangle aVisArea;
    if (pViewShell)
    {
        Window* pWindow = pViewShell->GetWindowByPos(eSplitPos);
        if (pWindow)
        {
            aVisArea.SetPos(pViewShell->GetViewData()->GetPixPos(eSplitPos));
            aVisArea.SetSize(pWindow->GetSizePixel());
        }
    }
    return aVisArea;
}

Rectangle ScAccessibleSpreadsheet::GetVisCells(const Rectangle& rVisArea)
{
    if (mpViewShell)
    {
        sal_Int16 nStartX, nStartY, nEndX, nEndY;

        mpViewShell->GetViewData()->GetPosFromPixel( 1, 1, meSplitPos, nStartX, nStartY);
        mpViewShell->GetViewData()->GetPosFromPixel( rVisArea.GetWidth(), rVisArea.GetHeight(), meSplitPos, nEndX, nEndY);

        return Rectangle(nStartX, nStartY, nEndX, nEndY);
    }
    else
        return Rectangle();
}