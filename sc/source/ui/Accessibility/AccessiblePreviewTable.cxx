/*************************************************************************
 *
 *  $RCSfile: AccessiblePreviewTable.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nn $ $Date: 2002-02-27 19:41:47 $
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

#include "AccessiblePreviewTable.hxx"
#include "AccessiblePreviewCell.hxx"
#include "prevwsh.hxx"
#include "unoguard.hxx"
#include "miscuno.hxx"
#include "prevloc.hxx"

#include <drafts/com/sun/star/accessibility/AccessibleRole.hpp>
#include <drafts/com/sun/star/accessibility/AccessibleStateType.hpp>

#include <vcl/window.hxx>
#include <svtools/smplhint.hxx>
#include <unotools/accessiblestatesethelper.hxx>

using namespace ::com::sun::star;
using namespace ::drafts::com::sun::star::accessibility;

//=====  internal  ============================================================

ScAccessiblePreviewTable::ScAccessiblePreviewTable( const ::com::sun::star::uno::Reference<
                                ::drafts::com::sun::star::accessibility::XAccessible>& rxParent,
                            ScPreviewShell* pViewShell, sal_Int32 nIndex ) :
    ScAccessibleContextBase( rxParent, AccessibleRole::TABLE ),
    mpViewShell( pViewShell ),
    mpTableInfo( NULL ),
    mnIndex( nIndex )
{
    if (mpViewShell)
        mpViewShell->AddAccessibilityObject(*this);
}

ScAccessiblePreviewTable::~ScAccessiblePreviewTable()
{
    if (mpViewShell)
        mpViewShell->RemoveAccessibilityObject(*this);

    delete mpTableInfo;
}

void ScAccessiblePreviewTable::SetDefunc()
{
    if (mpViewShell)
    {
        mpViewShell->RemoveAccessibilityObject(*this);
        mpViewShell = NULL;
    }

    ScAccessibleContextBase::SetDefunc();
}

//=====  SfxListener  =====================================================

void ScAccessiblePreviewTable::Notify( SfxBroadcaster& rBC, const SfxHint& rHint )
{
    if (rHint.ISA( SfxSimpleHint ))
    {
        const SfxSimpleHint& rRef = (const SfxSimpleHint&)rHint;
        ULONG nId = rRef.GetId();
        if ( nId == SFX_HINT_DYING )
            SetDefunc();
        else if ( nId == SFX_HINT_DATACHANGED )
        {
            //  column / row layout may change with any document change,
            //  so it must be invalidated
            DELETEZ( mpTableInfo );
        }
    }
}

//=====  XInterface  ======================================================

uno::Any SAL_CALL ScAccessiblePreviewTable::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( XAccessibleTable )

    return ScAccessibleContextBase::queryInterface( rType );
}

void SAL_CALL ScAccessiblePreviewTable::acquire() throw()
{
    ScAccessibleContextBase::acquire();
}

void SAL_CALL ScAccessiblePreviewTable::release() throw()
{
    ScAccessibleContextBase::release();
}

//=====  XAccessibleTable  ================================================

sal_Int32 SAL_CALL ScAccessiblePreviewTable::getAccessibleRowCount() throw (uno::RuntimeException)
{
    //! missing
    return 0;
}

sal_Int32 SAL_CALL ScAccessiblePreviewTable::getAccessibleColumnCount() throw (uno::RuntimeException)
{
    //! missing
    return 0;
}

rtl::OUString SAL_CALL ScAccessiblePreviewTable::getAccessibleRowDescription( sal_Int32 nRow )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    //! missing
    return rtl::OUString();
}

rtl::OUString SAL_CALL ScAccessiblePreviewTable::getAccessibleColumnDescription( sal_Int32 nColumn )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    //! missing
    return rtl::OUString();
}

sal_Int32 SAL_CALL ScAccessiblePreviewTable::getAccessibleRowExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    //! missing
    return 0;
}

sal_Int32 SAL_CALL ScAccessiblePreviewTable::getAccessibleColumnExtentAt( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    //! missing
    return 0;
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
    return sal_False;
}

sal_Bool SAL_CALL ScAccessiblePreviewTable::isAccessibleColumnSelected( sal_Int32 nColumn )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    //  in the page preview, there is no selection
    return sal_False;
}

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewTable::getAccessibleCellAt( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

    FillTableInfo();

    uno::Reference<XAccessible> xRet;
    if ( mpTableInfo && nColumn >= 0 && nRow >= 0 && nColumn < mpTableInfo->GetCols() && nRow < mpTableInfo->GetRows() )
    {
        //  index iterates horizontally
        long nNewIndex = nRow * mpTableInfo->GetCols() + nColumn;

        const ScPreviewColRowInfo& rColInfo = mpTableInfo->GetColInfo()[nColumn];
        const ScPreviewColRowInfo& rRowInfo = mpTableInfo->GetRowInfo()[nRow];

        if ( rColInfo.bIsHeader || rRowInfo.bIsHeader )
        {
            //! header cell object...
        }
        else
        {
            ScAddress aCellPos( rColInfo.nDocIndex, rRowInfo.nDocIndex, mpTableInfo->GetTab() );
            xRet = new ScAccessiblePreviewCell( this, mpViewShell, aCellPos, nNewIndex );
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
    return sal_False;
}

sal_Int32 SAL_CALL ScAccessiblePreviewTable::getAccessibleIndex( sal_Int32 nRow, sal_Int32 nColumn )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    //! missing
    return 0;
}

sal_Int32 SAL_CALL ScAccessiblePreviewTable::getAccessibleRow( sal_Int32 nChildIndex )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    //! missing
    return 0;
}

sal_Int32 SAL_CALL ScAccessiblePreviewTable::getAccessibleColumn( sal_Int32 nChildIndex )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    //! missing
    return 0;
}

//=====  XAccessibleComponent  ============================================

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewTable::getAccessibleAt( const awt::Point& aPoint )
                                throw (uno::RuntimeException)
{
    ScUnoGuard aGuard;

    FillTableInfo();

    uno::Reference<XAccessible> xRet;
    if ( mpTableInfo )
    {
        USHORT nCols = mpTableInfo->GetCols();
        USHORT nRows = mpTableInfo->GetCols();
        const ScPreviewColRowInfo* pColInfo = mpTableInfo->GetColInfo();
        const ScPreviewColRowInfo* pRowInfo = mpTableInfo->GetRowInfo();

        if ( nCols > 0 && nRows > 0 && aPoint.X >= pColInfo[0].nPixelStart && aPoint.Y >= pRowInfo[0].nPixelStart )
        {
            USHORT nColIndex = 0;
            while ( nColIndex < nCols && aPoint.X > pColInfo[nColIndex].nPixelEnd )
                ++nColIndex;
            USHORT nRowIndex = 0;
            while ( nRowIndex < nRows && aPoint.Y > pRowInfo[nRowIndex].nPixelEnd )
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

    return xRet;
}

void SAL_CALL ScAccessiblePreviewTable::grabFocus() throw (uno::RuntimeException)
{
     ScUnoGuard aGuard;
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
    ScUnoGuard aGuard;

    FillTableInfo();

    long nRet = 0;
    if ( mpTableInfo )
        nRet = mpTableInfo->GetCols() * mpTableInfo->GetRows();
    return nRet;
}

uno::Reference< XAccessible > SAL_CALL ScAccessiblePreviewTable::getAccessibleChild( sal_Int32 nIndex )
                                throw (lang::IndexOutOfBoundsException, uno::RuntimeException)
{
    ScUnoGuard aGuard;

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

uno::Reference< XAccessibleRelationSet > SAL_CALL ScAccessiblePreviewTable::getAccessibleRelationSet()
                                throw (uno::RuntimeException)
{
    //! missing
    return NULL;
}

uno::Reference< XAccessibleStateSet > SAL_CALL ScAccessiblePreviewTable::getAccessibleStateSet()
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
    pStateSet->AddState(AccessibleStateType::ENABLED);
    pStateSet->AddState(AccessibleStateType::OPAQUE);
    if (isShowing())
        pStateSet->AddState(AccessibleStateType::SHOWING);
    if (isVisible())
        pStateSet->AddState(AccessibleStateType::VISIBLE);
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

    pNames[nOldSize] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("drafts.com.sun.star.table.AccessibleTableView"));

    return aSequence;
}

//====  internal  =========================================================

::rtl::OUString SAL_CALL ScAccessiblePreviewTable::createAccessibleDescription(void)
                    throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("This is a table in a page preview of a Spreadsheet Document."));
}

::rtl::OUString SAL_CALL ScAccessiblePreviewTable::createAccessibleName(void)
                    throw (uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Spreadsheet Page Preview Table"));
}

Rectangle ScAccessiblePreviewTable::GetBoundingBoxOnScreen() throw (uno::RuntimeException)
{
    Rectangle aRect;
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
            aRect = pWindow->GetWindowExtentsRelative(NULL);
    }
    return aRect;
}

Rectangle ScAccessiblePreviewTable::GetBoundingBox() throw (uno::RuntimeException)
{
    Rectangle aRect;
    if (mpViewShell)
    {
        Window* pWindow = mpViewShell->GetWindow();
        if (pWindow)
            aRect = pWindow->GetWindowExtentsRelative(pWindow->GetAccessibleParentWindow());
    }
    return aRect;
}

sal_Bool ScAccessiblePreviewTable::IsDefunc( const uno::Reference<XAccessibleStateSet>& rxParentStates )
{
    return (mpViewShell == NULL) || !getAccessibleParent().is() ||
        (rxParentStates.is() && rxParentStates->contains(AccessibleStateType::DEFUNC));
}

void ScAccessiblePreviewTable::FillTableInfo()
{
    if ( mpViewShell && !mpTableInfo )
    {
        Size aOutputSize;
        Window* pWindow = mpViewShell->GetWindow();
        if ( pWindow )
            aOutputSize = pWindow->GetOutputSizePixel();
        Rectangle aVisRect( Point(), aOutputSize );

        mpTableInfo = new ScPreviewTableInfo;
        mpViewShell->GetLocationData().GetTableInfo( aVisRect, *mpTableInfo );
    }
}

