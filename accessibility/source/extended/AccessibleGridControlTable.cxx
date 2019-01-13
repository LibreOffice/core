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

#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <extended/AccessibleGridControlTable.hxx>
#include <extended/AccessibleGridControlTableCell.hxx>
#include <vcl/accessibletable.hxx>
#include <tools/debug.hxx>

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::vcl;
using namespace ::vcl::table;


namespace accessibility {


AccessibleGridControlTable::AccessibleGridControlTable(
        const Reference< XAccessible >& rxParent,
        IAccessibleTable& rTable) :
    AccessibleGridControlTableBase( rxParent, rTable, TCTYPE_TABLE )
{
}

// XAccessibleContext ---------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleGridControlTable::getAccessibleChild( sal_Int32 nChildIndex )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidIndex( nChildIndex );
    sal_Int32 nCount = getAccessibleChildCount();
    if(m_pAccessCellVector.empty() || m_pAccessCellVector.size() != static_cast<unsigned>(nCount))
    {
        m_pAccessCellVector.resize(nCount);
        m_pCellVector.resize(nCount);
    }
    if(!m_pAccessCellVector[nChildIndex].is())
    {
        AccessibleGridControlTableCell* pCell = new AccessibleGridControlTableCell(this, m_aTable, nChildIndex/m_aTable.GetColumnCount(), nChildIndex%m_aTable.GetColumnCount());
        m_pCellVector[nChildIndex] = pCell;
        m_pAccessCellVector[nChildIndex] = pCell;
    }
    return m_pAccessCellVector[nChildIndex];
}

sal_Int32 SAL_CALL AccessibleGridControlTable::getAccessibleIndexInParent()
{
    ensureIsAlive();
    if(m_aTable.HasRowHeader() && m_aTable.HasColHeader())
        return 0;
    else if((!m_aTable.HasRowHeader() && m_aTable.HasColHeader()) || (m_aTable.HasRowHeader() && !m_aTable.HasColHeader()) )
        return 1;
    else
        return 2;
}

// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleGridControlTable::getAccessibleAtPoint( const awt::Point& rPoint )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();

    Reference< XAccessible > xChild;
    sal_Int32 nRow = 0;
    sal_Int32 nColumnPos = 0;
    if( m_aTable.ConvertPointToCellAddress( nRow, nColumnPos, VCLPoint( rPoint ) ) )
        xChild = new AccessibleGridControlTableCell(this, m_aTable, nRow, nColumnPos);
    return xChild;
}

void SAL_CALL AccessibleGridControlTable::grabFocus()
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    m_aTable.GrabFocus();
}

// XAccessibleTable -----------------------------------------------------------

OUString SAL_CALL AccessibleGridControlTable::getAccessibleRowDescription( sal_Int32 nRow )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidRow( nRow );
    return m_aTable.GetRowDescription( nRow );
}

OUString SAL_CALL AccessibleGridControlTable::getAccessibleColumnDescription( sal_Int32 nColumn )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidColumn( nColumn );
    return m_aTable.GetColumnDescription( static_cast<sal_uInt16>(nColumn) );
}

Reference< XAccessibleTable > SAL_CALL AccessibleGridControlTable::getAccessibleRowHeaders()
{
    SolarMutexGuard g;

    ensureIsAlive();
    if(m_aTable.HasColHeader())
        return implGetHeaderBar( 1 );
    else
        return implGetHeaderBar( 0 );
}

Reference< XAccessibleTable > SAL_CALL AccessibleGridControlTable::getAccessibleColumnHeaders()
{
    SolarMutexGuard g;

    ensureIsAlive();
    return implGetHeaderBar( 0 );
}

Sequence< sal_Int32 > SAL_CALL AccessibleGridControlTable::getSelectedAccessibleRows()
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    Sequence< sal_Int32 > aSelSeq;
    implGetSelectedRows( aSelSeq );
    return aSelSeq;
}

//columns aren't selectable
Sequence< sal_Int32 > SAL_CALL AccessibleGridControlTable::getSelectedAccessibleColumns()
{
    Sequence< sal_Int32 > aSelSeq(0);
    return aSelSeq;
}

sal_Bool SAL_CALL AccessibleGridControlTable::isAccessibleRowSelected( sal_Int32 nRow )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidRow( nRow );
    bool bSelected = false;
    Sequence< sal_Int32 > selectedRows = getSelectedAccessibleRows();
    for(int i=0; i<selectedRows.getLength(); i++)
    {
        if(nRow == selectedRows[i])
        {
            bSelected = true;
            continue;
        }
    }
    return bSelected;
}

//columns aren't selectable
sal_Bool SAL_CALL AccessibleGridControlTable::isAccessibleColumnSelected( sal_Int32 )
{
    return false;
}

Reference< XAccessible > SAL_CALL AccessibleGridControlTable::getAccessibleCellAt(
        sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    sal_Int32 nCount = getAccessibleChildCount();
    sal_Int32 nChildIndex = nRow*m_aTable.GetColumnCount() + nColumn;
    if(m_pAccessCellVector.empty() || m_pAccessCellVector.size() != static_cast<unsigned>(nCount))
    {
        m_pAccessCellVector.resize(nCount);
        m_pCellVector.resize(nCount);
    }
    if(!m_pAccessCellVector[nChildIndex].is())
    {
        AccessibleGridControlTableCell* pCell = new AccessibleGridControlTableCell(this, m_aTable, nRow, nColumn);
        m_pCellVector[nChildIndex] = pCell;
        m_pAccessCellVector[nChildIndex] = pCell;
    }
    return m_pAccessCellVector[nChildIndex];
}

sal_Bool SAL_CALL AccessibleGridControlTable::isAccessibleSelected(
        sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    //selection of single cells not possible, so if row is selected, the cell will be selected too
    return isAccessibleRowSelected(nRow);
}
void SAL_CALL AccessibleGridControlTable::selectAccessibleChild( sal_Int32 nChildIndex )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidIndex( nChildIndex );
    sal_Int32 nColumns = m_aTable.GetColumnCount();
    sal_Int32 nRow = nChildIndex / nColumns;
    m_aTable.SelectRow( nRow, true );
}
sal_Bool SAL_CALL AccessibleGridControlTable::isAccessibleChildSelected( sal_Int32 nChildIndex )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidIndex( nChildIndex );
    sal_Int32 nColumns = m_aTable.GetColumnCount();
    sal_Int32 nRow = nChildIndex / nColumns;
    return isAccessibleRowSelected(nRow);
}
void SAL_CALL AccessibleGridControlTable::clearAccessibleSelection()
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    m_aTable.SelectAllRows( false );
}
void SAL_CALL AccessibleGridControlTable::selectAllAccessibleChildren()
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    Sequence< sal_Int32 > selectedRows = getSelectedAccessibleRows();
    for(long i=0; i<m_aTable.GetRowCount(); i++)
        selectedRows[i]=i;
}
sal_Int32 SAL_CALL AccessibleGridControlTable::getSelectedAccessibleChildCount()
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    Sequence< sal_Int32 > selectedRows = getSelectedAccessibleRows();
    sal_Int32 nColumns = m_aTable.GetColumnCount();
    return selectedRows.getLength()*nColumns;
}
Reference< XAccessible > SAL_CALL
AccessibleGridControlTable::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    if(isAccessibleChildSelected(nSelectedChildIndex))
        return getAccessibleChild(nSelectedChildIndex);
    else
        return nullptr;
}
//not implemented yet, because only row selection possible
void SAL_CALL AccessibleGridControlTable::deselectAccessibleChild(
        sal_Int32 )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
}
// XInterface -----------------------------------------------------------------

Any SAL_CALL AccessibleGridControlTable::queryInterface( const uno::Type& rType )
{
    Any aAny( AccessibleGridControlTableBase::queryInterface( rType ) );
    return aAny.hasValue() ?
        aAny : AccessibleGridControlTableSelectionImplHelper::queryInterface( rType );
}

void SAL_CALL AccessibleGridControlTable::acquire() throw ()
{
    AccessibleGridControlTableBase::acquire();
}

void SAL_CALL AccessibleGridControlTable::release() throw ()
{
    AccessibleGridControlTableBase::release();
}
// XServiceInfo ---------------------------------------------------------------

OUString SAL_CALL AccessibleGridControlTable::getImplementationName()
{
    return OUString( "com.sun.star.accessibility.AccessibleGridControlTable" );
}

// internal virtual methods ---------------------------------------------------

tools::Rectangle AccessibleGridControlTable::implGetBoundingBox()
{
    vcl::Window* pParent = m_aTable.GetAccessibleParentWindow();
    DBG_ASSERT( pParent, "implGetBoundingBox - missing parent window" );
    tools::Rectangle aGridRect( m_aTable.GetWindowExtentsRelative( pParent ));
    tools::Rectangle aTableRect( m_aTable.calcTableRect() );
    long nX = aGridRect.Left() + aTableRect.Left();
    long nY = aGridRect.Top() + aTableRect.Top();
    long nWidth = aGridRect.GetSize().Width()-aTableRect.Left();
    long nHeight = aGridRect.GetSize().Height()-aTableRect.Top();
    tools::Rectangle aTable( Point( nX, nY ), Size( nWidth, nHeight ));
    return aTable;
}

tools::Rectangle AccessibleGridControlTable::implGetBoundingBoxOnScreen()
{
    tools::Rectangle aGridRect( m_aTable.GetWindowExtentsRelative( nullptr ));
    tools::Rectangle aTableRect( m_aTable.calcTableRect() );
    long nX = aGridRect.Left() + aTableRect.Left();
    long nY = aGridRect.Top() + aTableRect.Top();
    long nWidth = aGridRect.GetSize().Width()-aTableRect.Left();
    long nHeight = aGridRect.GetSize().Height()-aTableRect.Top();
    tools::Rectangle aTable( Point( nX, nY ), Size( nWidth, nHeight ));
    return aTable;
}
// internal helper methods ----------------------------------------------------
Reference< XAccessibleTable > AccessibleGridControlTable::implGetHeaderBar(
        sal_Int32 nChildIndex )
{
    Reference< XAccessible > xRet;
    Reference< XAccessibleContext > xContext( m_xParent, uno::UNO_QUERY );
    if( xContext.is() )
    {
        try
        {
            xRet = xContext->getAccessibleChild( nChildIndex );
        }
        catch (const lang::IndexOutOfBoundsException&)
        {
            OSL_FAIL( "implGetHeaderBar - wrong child index" );
        }
        // RuntimeException goes to caller
    }
    return Reference< XAccessibleTable >( xRet, uno::UNO_QUERY );
}


} // namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
