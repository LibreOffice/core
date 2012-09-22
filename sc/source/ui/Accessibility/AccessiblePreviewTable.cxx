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


#include "scitems.hxx"
#include "AccessiblePreviewTable.hxx"
#include "AccessiblePreviewCell.hxx"
#include "AccessiblePreviewHeaderCell.hxx"
#include "AccessibilityHints.hxx"
#include "prevwsh.hxx"
#include "miscuno.hxx"
#include "prevloc.hxx"
#include "attrib.hxx"
#include "document.hxx"
#include "scresid.hxx"
#include "sc.hrc"

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>

#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <svl/smplhint.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessiblePreviewTable::ScAccessiblePreviewTable( const ::com::sun::star::uno::Reference<
                                ::com::sun::star::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell, sal_Int32 nIndex ) :
    ScAccessibleContextBase( rxParent, AccessibleRole::TABLE ),
    mpViewShell( pViewShell ),
    mnIndex( nIndex ),
    mpTableInfo( NULL )
{
    if (mpViewShell)
        mpViewShell->AddAccessibilityObject(*this);
}

ScAccessiblePreviewTable::~ScAccessiblePreviewTable()
{
    if (!ScAccessibleContextBase::IsDefunc() && !rBHelper.bInDispose)
    {
        // increment refcount to prevent double call off dtor
        osl_atomic_increment( &m_refCount );
        dispose();
    }
}

void SAL_CALL ScAccessiblePreviewTable::disposing()
{
    SolarMutexGuard aGuard;
    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = NULL;
    }

    if (mpTableInfo)
        DELETEZ (mpTableInfo);

    ScAccessibleContextBase::disposing();
}

//=====  SfxListener  =====================================================

void ScAccessiblePreviewTable::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.ISA( SfxSimpleHint ))
    {
        const SfxSimpleHint& rRef = (const SfxSimpleHint&)rHint;
        sal_uLong nId = rRef.GetId();
        if ( nId == SFX_HINT_DATACHANGED )
        {
            //  column / row layout may change with any document change,
            //  so it must be invalidated
            DELETEZ( mpTableInfo );
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

//=====  XInterface  =====================================================

uno::Any SAL_CALL ScAccessiblePreviewTable::queryInterface( uno::Type const & rType )
    throw (uno::RuntimeException)
{
    uno::Any aAny (ScAccessiblePreviewTableImpl::queryInterface(rType));
    return aAny.hasValue() ? aAny : ScAccessibleContextBase::queryInterface(rType);
}

void SAL_CALL ScAccessiblePreviewTable::acquire()
    throw ()
{
    ScAccessibleContextBase::acquire();
}

void SAL_CALL ScAccessiblePreviewTable::release()
    throw ()
{
    ScAccessibleContextBase::release();
}

//=====  XAccessibleTable  ================================================

sal_Int32 SAL_CALL ScAccessiblePreviewTable::getAccessibleRowCount() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    FillTableInfo();

    sal_Int32 nRet = 0;
    if ( mpTableInfo )
        nRet = mpTableInfo->GetRows();
    return nRet;
}

sal_Int32 SAL_CALL ScAccessiblePreviewTable::getAccessibleColumnCount() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    FillTableInfo();

    sal_Int32 nRet = 0;
    if ( mpTableInfo )
        nRet = mpTableInfo->GetCols();
    return nRet;
}

rtl::OUString SAL_CALL ScAccessiblePreviewTable::getAccessibleRowDescription( sal_Int32 nRow )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    FillTableInfo();
    if ( nRow < 0 || (mpTableInfo && nRow >= mpTableInfo->GetRows()) )
        throw lang::IndexOutOfBoundsException();

    return rtl::OUString();
}

rtl::OUString SAL_CALL ScAccessiblePreviewTable::getAccessibleColumnDescription( sal_Int32 nColumn )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    FillTableInfo();
    if ( nColumn < 0 || (mpTableInfo && nColumn >= mpTableInfo->GetCols()) )
        throw lang::IndexOutOfBoundsException();

    return rtl::OUString();
}

sal_Int32 SAL_CALL ScAccessiblePreviewTable::getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    FillTableInfo();

    sal_Int32 nRows = 1;
    if ( mpViewShell && mpTableInfo && nColumn >= 0 && nRow >= 0 &&
            nColumn < mpTableInfo->GetCols() && nRow < mpTableInfo->GetRows() )
    {
        const ScPreviewColRowInfo& rColInfo = mpTableInfo->GetColInfo()[nColumn];
        const ScPreviewColRowInfo& rRowInfo = mpTableInfo->GetRowInfo()[nRow];

        if ( rColInfo.bIsHeader || rRowInfo.bIsHeader )
        {
            //  header cells only span a single cell
        }
        else
        {
            ScDocument* pDoc = mpViewShell->GetDocument();
            const ScMergeAttr* pItem = (const ScMergeAttr*)pDoc->GetAttr(
                static_cast<SCCOL>(rColInfo.nDocIndex), static_cast<SCROW>(rRowInfo.nDocIndex), mpTableInfo->GetTab(), ATTR_MERGE );
            if ( pItem && pItem->GetRowMerge() > 0 )
                nRows = pItem->GetRowMerge();
        }
    }
    else
        throw lang::IndexOutOfBoundsException();

    return nRows;
}

sal_Int32 SAL_CALL ScAccessiblePreviewTable::getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    FillTableInfo();

    sal_Int32 nColumns = 1;
    if ( mpViewShell && mpTableInfo && nColumn >= 0 && nRow >= 0 &&
            nColumn < mpTableInfo->GetCols() && nRow < mpTableInfo->GetRows() )
    {
        const ScPreviewColRowInfo& rColInfo = mpTableInfo->GetColInfo()[nColumn];
        const ScPreviewColRowInfo& rRowInfo = mpTableInfo->GetRowInfo()[nRow];

        if ( rColInfo.bIsHeader || rRowInfo.bIsHeader )
        {
            //  header cells only span a single cell
        }
        else
        {
            ScDocument* pDoc = mpViewShell->GetDocument();
            const ScMergeAttr* pItem = (const ScMergeAttr*)pDoc->GetAttr(
                static_cast<SCCOL>(rColInfo.nDocIndex), static_cast<SCROW>(rRowInfo.nDocIndex), mpTableInfo->GetTab(), ATTR_MERGE );
            if ( pItem && pItem->GetColMerge() > 0 )
                nColumns = pItem->GetColMerge();
        }
    }
    else
        throw lang::IndexOutOfBoundsException();

    return nColumns;
}

uno::Reference< XAccessibleTable > SAL_CALL ScAccessiblePreviewTable::getAccessibleRowHeaders() throw (uno::RuntimeException)
{
    //! missing
    return NULL;
}

uno::Reference< XAccessibleTable > SAL_CALL ScAccessiblePreviewTable::getAccessibleColumnHeaders() throw (uno::RuntimeException)
{
    //! missing
    return NULL;
}

uno::Sequence< sal_Int32 > SAL_CALL ScAccessiblePreviewTable::getSelectedAccessibleRows() throw (uno::RuntimeException)
{
    //  in the page preview, there is no selection
    return uno::Sequence<sal_Int32>(0);
}

uno::Sequence< sal_Int32 > SAL_CALL ScAccessiblePreviewTable::getSelectedAccessibleColumns() throw (uno::RuntimeException)
{
    //  in the page preview, there is no selection
    return uno::Sequence<sal_Int32>(0);
}

sal_Bool SAL_CALL ScAccessiblePreviewTable::isAccessibleRowSelected( sal_Int32 nRow )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    //  in the page preview, there is no selection

    SolarMutexGuard aGuard;
    FillTableInfo();
    if ( nRow < 0 || (mpTableInfo && nRow >= mpTableInfo->GetRows()) )
        throw lang::IndexOutOfBoundsException();

    return false;
}

sal_Bool SAL_CALL ScAccessiblePreviewTable::isAccessibleColumnSelected( sal_Int32 nColumn )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    //  in the page preview, there is no selection

    SolarMutexGuard aGuard;
    FillTableInfo();
    if ( nColumn < 0 || (mpTableInfo && nColumn >= mpTableInfo->GetCols()) )
        throw lang::IndexOutOfBoundsException();

    return false;
}

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewTable::getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    FillTableInfo();

    uno::Reference<XAccessible> xRet;
    if ( mpTableInfo && nColumn >= 0 && nRow >= 0 && nColumn < mpTableInfo->GetCols() && nRow < mpTableInfo->GetRows() )
    {
        //  index iterates horizontally
        long nNewIndex = nRow * mpTableInfo->GetCols() + nColumn;

        const ScPreviewColRowInfo& rColInfo = mpTableInfo->GetColInfo()[nColumn];
        const ScPreviewColRowInfo& rRowInfo = mpTableInfo->GetRowInfo()[nRow];

        ScAddress aCellPos( static_cast<SCCOL>(rColInfo.nDocIndex), static_cast<SCROW>(rRowInfo.nDocIndex), mpTableInfo->GetTab() );
        if ( rColInfo.bIsHeader || rRowInfo.bIsHeader )
        {
            ScAccessiblePreviewHeaderCell* pHeaderCell = new ScAccessiblePreviewHeaderCell( this, mpViewShell, aCellPos,
                                        rRowInfo.bIsHeader, rColInfo.bIsHeader, nNewIndex );
            xRet = pHeaderCell;
            pHeaderCell->Init();
        }
        else
        {
            ScAccessiblePreviewCell* pCell = new ScAccessiblePreviewCell( this, mpViewShell, aCellPos, nNewIndex );
            xRet = pCell;
            pCell->Init();
        }
    }

    if ( !xRet.is() )
        throw lang::IndexOutOfBoundsException();

    return xRet;
}

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewTable::getAccessibleCaption() throw (uno::RuntimeException)
{
    //! missing
    return NULL;
}

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewTable::getAccessibleSummary() throw (uno::RuntimeException)
{
    //! missing
    return NULL;
}

sal_Bool SAL_CALL ScAccessiblePreviewTable::isAccessibleSelected( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    //  in the page preview, there is no selection
    SolarMutexGuard aGuard;
    IsObjectValid();

    FillTableInfo();

    if ( mpTableInfo && nColumn >= 0 && nRow >= 0 && nColumn < mpTableInfo->GetCols() && nRow < mpTableInfo->GetRows() )
    {
        //  index iterates horizontally
    }
    else
        throw lang::IndexOutOfBoundsException();

    return false;
}

sal_Int32 SAL_CALL ScAccessiblePreviewTable::getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    FillTableInfo();

    sal_Int32 nRet = 0;
    if ( mpTableInfo && nColumn >= 0 && nRow >= 0 && nColumn < mpTableInfo->GetCols() && nRow < mpTableInfo->GetRows() )
    {
        //  index iterates horizontally
        nRet = nRow * mpTableInfo->GetCols() + nColumn;
    }
    else
        throw lang::IndexOutOfBoundsException();

    return nRet;
}

sal_Int32 SAL_CALL ScAccessiblePreviewTable::getAccessibleRow( sal_Int32 nChildIndex )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    FillTableInfo();

    sal_Int32 nRow = 0;
    if ( mpTableInfo && nChildIndex >= 0 && nChildIndex < static_cast<sal_Int32>(mpTableInfo->GetRows()) * mpTableInfo->GetCols() )
    {
        nRow = nChildIndex / mpTableInfo->GetCols();
    }
    else
        throw lang::IndexOutOfBoundsException();

    return nRow;
}

sal_Int32 SAL_CALL ScAccessiblePreviewTable::getAccessibleColumn( sal_Int32 nChildIndex )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    FillTableInfo();

    sal_Int32 nCol = 0;
    if ( mpTableInfo && nChildIndex >= 0 && nChildIndex < static_cast<sal_Int32>(mpTableInfo->GetRows()) * mpTableInfo->GetCols() )
    {
        nCol = nChildIndex % static_cast<sal_Int32>(mpTableInfo->GetCols());
    }
    else
        throw lang::IndexOutOfBoundsException();

    return nCol;
}

//=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewTable::getAccessibleAtPoint( const awt::Point& aPoint )
                                throw (uno::RuntimeException)
{
    uno::Reference<XAccessible> xRet;
    if (containsPoint(aPoint))
    {
        SolarMutexGuard aGuard;
        IsObjectValid();

        FillTableInfo();

        if ( mpTableInfo )
        {
            SCCOL nCols = mpTableInfo->GetCols();
            SCROW nRows = mpTableInfo->GetRows();
            const ScPreviewColRowInfo* pColInfo = mpTableInfo->GetColInfo();
            const ScPreviewColRowInfo* pRowInfo = mpTableInfo->GetRowInfo();

            Rectangle aScreenRect(GetBoundingBox());

            awt::Point aMovedPoint = aPoint;
            aMovedPoint.X += aScreenRect.Left();
            aMovedPoint.Y += aScreenRect.Top();

            if ( nCols > 0 && nRows > 0 && aMovedPoint.X >= pColInfo[0].nPixelStart && aMovedPoint.Y >= pRowInfo[0].nPixelStart )
            {
                SCCOL nColIndex = 0;
                while ( nColIndex < nCols && aMovedPoint.X > pColInfo[nColIndex].nPixelEnd )
                    ++nColIndex;
                SCROW nRowIndex = 0;
                while ( nRowIndex < nRows && aMovedPoint.Y > pRowInfo[nRowIndex].nPixelEnd )
                    ++nRowIndex;
                if ( nColIndex < nCols && nRowIndex < nRows )
                {
                    try
                    {
                        xRet = getAccessibleCellAt( nRowIndex, nColIndex );
                    }
                    catch (uno::Exception&)
                    {
                    }
                }
            }
        }
    }

    return xRet;
}

void SAL_CALL ScAccessiblePreviewTable::grabFocus() throw (uno::RuntimeException)
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

sal_Int32 SAL_CALL ScAccessiblePreviewTable::getAccessibleChildCount() throw (uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    FillTableInfo();

    long nRet = 0;
    if ( mpTableInfo )
        nRet = static_cast<sal_Int32>(mpTableInfo->GetCols()) * mpTableInfo->GetRows();
    return nRet;
}

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewTable::getAccessibleChild( sal_Int32 nIndex )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    IsObjectValid();

    FillTableInfo();

    uno::Reference<XAccessible> xRet;
    if ( mpTableInfo )
    {
        long nColumns = mpTableInfo->GetCols();
        if ( nColumns > 0 )
        {
            // nCol, nRow are within the visible table, not the document
            long nCol = nIndex % nColumns;
            long nRow = nIndex / nColumns;

            xRet = getAccessibleCellAt( nRow, nCol );
        }
    }

    if ( !xRet.is() )
        throw lang::IndexOutOfBoundsException();

    return xRet;
}

sal_Int32 SAL_CALL ScAccessiblePreviewTable::getAccessibleIndexInParent() throw (uno::RuntimeException)
{
    return mnIndex;
}

uno::Reference< XAccessibleStateSet > SAL_CALL ScAccessiblePreviewTable::getAccessibleStateSet()
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
        pStateSet->AddState(AccessibleStateType::MANAGES_DESCENDANTS);
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

rtl::OUString SAL_CALL ScAccessiblePreviewTable::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ScAccessiblePreviewTable"));
}

uno::Sequence<rtl::OUString> SAL_CALL ScAccessiblePreviewTable::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSequence = ScAccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nOldSize(aSequence.getLength());
    aSequence.realloc(nOldSize + 1);
    ::rtl::OUString* pNames = aSequence.getArray();

    pNames[nOldSize] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.table.AccessibleTableView"));

    return aSequence;
}

//=====  XTypeProvider  ===================================================

uno::Sequence< uno::Type > SAL_CALL ScAccessiblePreviewTable::getTypes()
        throw (uno::RuntimeException)
{
    return comphelper::concatSequences(ScAccessiblePreviewTableImpl::getTypes(), ScAccessibleContextBase::getTypes());
}

namespace
{
    class theScAccessiblePreviewTableImplementationId : public rtl::Static< UnoTunnelIdInit, theScAccessiblePreviewTableImplementationId > {};
}

uno::Sequence<sal_Int8> SAL_CALL ScAccessiblePreviewTable::getImplementationId()
                                                    throw(uno::RuntimeException)
{
    return theScAccessiblePreviewTableImplementationId::get().getSeq();
}

//====  internal  =========================================================

::rtl::OUString SAL_CALL ScAccessiblePreviewTable::createAccessibleDescription(void)
                    throw (uno::RuntimeException)
{
    String sDesc(ScResId(STR_ACC_TABLE_DESCR));
    return rtl::OUString(sDesc);
}

::rtl::OUString SAL_CALL ScAccessiblePreviewTable::createAccessibleName(void)
                    throw (uno::RuntimeException)
{
    String sName(ScResId(STR_ACC_TABLE_NAME));

    if (mpViewShell && mpViewShell->GetDocument())
    {
        FillTableInfo();

        if ( mpTableInfo )
        {
            rtl::OUString sCoreName;
            if (mpViewShell->GetDocument()->GetName( mpTableInfo->GetTab(), sCoreName ))
                sName.SearchAndReplaceAscii("%1", sCoreName);
        }
    }

    return rtl::OUString(sName);
}

Rectangle ScAccessiblePreviewTable::GetBoundingBoxOnScreen() const throw (uno::RuntimeException)
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

Rectangle ScAccessiblePreviewTable::GetBoundingBox() const throw (uno::RuntimeException)
{
    FillTableInfo();

    Rectangle aRect;
    if ( mpTableInfo )
    {
        SCCOL nColumns = mpTableInfo->GetCols();
        SCROW nRows = mpTableInfo->GetRows();
        if ( nColumns > 0 && nRows > 0 )
        {
            const ScPreviewColRowInfo* pColInfo = mpTableInfo->GetColInfo();
            const ScPreviewColRowInfo* pRowInfo = mpTableInfo->GetRowInfo();

            aRect = Rectangle( pColInfo[0].nPixelStart,
                               pRowInfo[0].nPixelStart,
                               pColInfo[nColumns-1].nPixelEnd,
                               pRowInfo[nRows-1].nPixelEnd );
        }
    }
    return aRect;
}

sal_Bool ScAccessiblePreviewTable::IsDefunc( const uno::Reference<XAccessibleStateSet>& rxParentStates )
{
    return ScAccessibleContextBase::IsDefunc() || (mpViewShell == NULL) || !getAccessibleParent().is() ||
        (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

void ScAccessiblePreviewTable::FillTableInfo() const
{
    if ( mpViewShell && !mpTableInfo )
    {
        Size aOutputSize;
        Window* pWindow = mpViewShell->GetWindow();
        if ( pWindow )
            aOutputSize = pWindow->GetOutputSizePixel();
        Point aPoint;
        Rectangle aVisRect( aPoint, aOutputSize );

        mpTableInfo = new ScPreviewTableInfo;
        mpViewShell->GetLocationData().GetTableInfo( aVisRect, *mpTableInfo );
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
