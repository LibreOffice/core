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

#include "accessibility/extended/AccessibleGridControlTable.hxx"
#include "accessibility/extended/AccessibleGridControlTableCell.hxx"
#include <svtools/accessibletable.hxx>

// ============================================================================

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::svt;
using namespace ::svt::table;
// ============================================================================

namespace accessibility {

// ============================================================================

AccessibleGridControlTable::AccessibleGridControlTable(
        const Reference< XAccessible >& rxParent,
        IAccessibleTable& rTable,
        AccessibleTableControlObjType _eType) :
    AccessibleGridControlTableBase( rxParent, rTable, _eType )
{
}

AccessibleGridControlTable::~AccessibleGridControlTable()
{
}

// XAccessibleContext ---------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleGridControlTable::getAccessibleChild( sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidIndex( nChildIndex );
    return new AccessibleGridControlTableCell(this, m_aTable, nChildIndex/m_aTable.GetColumnCount(), nChildIndex%m_aTable.GetColumnCount(), TCTYPE_TABLECELL);
}

sal_Int32 SAL_CALL AccessibleGridControlTable::getAccessibleIndexInParent()
    throw ( uno::RuntimeException )
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
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    Reference< XAccessible > xChild;
    sal_Int32 nRow = 0;
    sal_Int32 nColumnPos = 0;
    if( m_aTable.ConvertPointToCellAddress( nRow, nColumnPos, VCLPoint( rPoint ) ) )
        xChild = new AccessibleGridControlTableCell(this, m_aTable, nRow, nColumnPos, TCTYPE_TABLECELL);

    return xChild;
}

void SAL_CALL AccessibleGridControlTable::grabFocus()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    m_aTable.GrabFocus();
}

Any SAL_CALL AccessibleGridControlTable::getAccessibleKeyBinding()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    return Any();   // no special key bindings for data table
}

// XAccessibleTable -----------------------------------------------------------

OUString SAL_CALL AccessibleGridControlTable::getAccessibleRowDescription( sal_Int32 nRow )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidRow( nRow );
    return m_aTable.GetRowDescription( nRow );
}

OUString SAL_CALL AccessibleGridControlTable::getAccessibleColumnDescription( sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidColumn( nColumn );
    return m_aTable.GetColumnDescription( (sal_uInt16)nColumn );
}

Reference< XAccessibleTable > SAL_CALL AccessibleGridControlTable::getAccessibleRowHeaders()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    if(m_aTable.HasColHeader())
        return implGetHeaderBar( 1 );
    else
        return implGetHeaderBar( 0 );
}

Reference< XAccessibleTable > SAL_CALL AccessibleGridControlTable::getAccessibleColumnHeaders()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return implGetHeaderBar( 0 );
}

Sequence< sal_Int32 > SAL_CALL AccessibleGridControlTable::getSelectedAccessibleRows()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    Sequence< sal_Int32 > aSelSeq;
    implGetSelectedRows( aSelSeq );
    return aSelSeq;
}

//columns aren't selectable
Sequence< sal_Int32 > SAL_CALL AccessibleGridControlTable::getSelectedAccessibleColumns()
    throw ( uno::RuntimeException )
{
    Sequence< sal_Int32 > aSelSeq(0);
    return aSelSeq;
}

sal_Bool SAL_CALL AccessibleGridControlTable::isAccessibleRowSelected( sal_Int32 nRow )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidRow( nRow );
    sal_Bool bSelected = sal_False;
    Sequence< sal_Int32 > selectedRows = getSelectedAccessibleRows();
    for(int i=0; i<selectedRows.getLength(); i++)
    {
        if(nRow == selectedRows[i])
        {
            bSelected = sal_True;
            continue;
        }
    }
    return bSelected;
}

//columns aren't selectable
sal_Bool SAL_CALL AccessibleGridControlTable::isAccessibleColumnSelected( sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    (void) nColumn;
    return sal_False;
}

Reference< XAccessible > SAL_CALL AccessibleGridControlTable::getAccessibleCellAt(
        sal_Int32 nRow, sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    return new AccessibleGridControlTableCell(this, m_aTable, nRow, nColumn, TCTYPE_TABLECELL);
}

sal_Bool SAL_CALL AccessibleGridControlTable::isAccessibleSelected(
        sal_Int32 nRow, sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    (void) nColumn;
    //selection of single cells not possible, so if row is selected, the cell will be selected too
    return isAccessibleRowSelected(nRow);
}
void SAL_CALL AccessibleGridControlTable::selectAccessibleChild( sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidIndex( nChildIndex );
    sal_Int32 nColumns = m_aTable.GetColumnCount();
    sal_Int32 nRow = (nChildIndex / nColumns);
    m_aTable.SelectRow( nRow, sal_True );
}
sal_Bool SAL_CALL AccessibleGridControlTable::isAccessibleChildSelected( sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidIndex( nChildIndex );
    sal_Int32 nColumns = m_aTable.GetColumnCount();
    sal_Int32 nRow = (nChildIndex / nColumns);
    return isAccessibleRowSelected(nRow);
}
void SAL_CALL AccessibleGridControlTable::clearAccessibleSelection()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    m_aTable.SelectAllRows( false );
}
void SAL_CALL AccessibleGridControlTable::selectAllAccessibleChildren()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    Sequence< sal_Int32 > selectedRows = getSelectedAccessibleRows();
    for(int i=0;i<m_aTable.GetRowCount();i++)
        selectedRows[i]=i;
}
sal_Int32 SAL_CALL AccessibleGridControlTable::getSelectedAccessibleChildCount()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    Sequence< sal_Int32 > selectedRows = getSelectedAccessibleRows();
    sal_Int32 nColumns = m_aTable.GetColumnCount();
    return selectedRows.getLength()*nColumns;
}
Reference< XAccessible > SAL_CALL
AccessibleGridControlTable::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    if(isAccessibleChildSelected(nSelectedChildIndex))
        return getAccessibleChild(nSelectedChildIndex);
    else
        return NULL;
}
//not implemented yet, because only row selection possible
void SAL_CALL AccessibleGridControlTable::deselectAccessibleChild(
        sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    (void)nSelectedChildIndex;
}
// XInterface -----------------------------------------------------------------

Any SAL_CALL AccessibleGridControlTable::queryInterface( const uno::Type& rType )
    throw ( uno::RuntimeException )
{
    Any aAny( AccessibleGridControlTableBase::queryInterface( rType ) );
    return aAny.hasValue() ?
        aAny : AccessibleGridControlTableImplHelper1::queryInterface( rType );
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
    throw ( uno::RuntimeException )
{
    return OUString( "com.sun.star.accessibility.AccessibleGridControlTable" );
}

// internal virtual methods ---------------------------------------------------

Rectangle AccessibleGridControlTable::implGetBoundingBox()
{
    return m_aTable.calcTableRect();
}

Rectangle AccessibleGridControlTable::implGetBoundingBoxOnScreen()
{
    return m_aTable.calcTableRect();
}
// internal helper methods ----------------------------------------------------
Reference< XAccessibleTable > AccessibleGridControlTable::implGetHeaderBar(
        sal_Int32 nChildIndex )
    throw ( uno::RuntimeException )
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

// ============================================================================

} // namespace accessibility

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
