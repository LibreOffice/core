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

#include "accessibility/extended/AccessibleBrowseBoxTable.hxx"
#include <svtools/accessibletableprovider.hxx>

// ============================================================================

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::Any;

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::svt;

// ============================================================================

namespace accessibility {

// ============================================================================

// Ctor/Dtor/disposing --------------------------------------------------------

AccessibleBrowseBoxTable::AccessibleBrowseBoxTable(
        const Reference< XAccessible >& rxParent,
        IAccessibleTableProvider&                      rBrowseBox ) :
    AccessibleBrowseBoxTableBase( rxParent, rBrowseBox, BBTYPE_TABLE )
{
}

AccessibleBrowseBoxTable::~AccessibleBrowseBoxTable()
{
}

// XAccessibleContext ---------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleBrowseBoxTable::getAccessibleChild( sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidIndex( nChildIndex );
    return mpBrowseBox->CreateAccessibleCell(
        implGetRow( nChildIndex ), (sal_Int16)implGetColumn( nChildIndex ) );
}

sal_Int32 SAL_CALL AccessibleBrowseBoxTable::getAccessibleIndexInParent()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    return BBINDEX_TABLE;
}

// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleBrowseBoxTable::getAccessibleAtPoint( const awt::Point& rPoint )
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    Reference< XAccessible > xChild;
    sal_Int32 nRow = 0;
    sal_uInt16 nColumnPos = 0;
    if( mpBrowseBox->ConvertPointToCellAddress( nRow, nColumnPos, VCLPoint( rPoint ) ) )
        xChild = mpBrowseBox->CreateAccessibleCell( nRow, nColumnPos );

    return xChild;
}

void SAL_CALL AccessibleBrowseBoxTable::grabFocus()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    mpBrowseBox->GrabTableFocus();
}

Any SAL_CALL AccessibleBrowseBoxTable::getAccessibleKeyBinding()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    return Any();   // no special key bindings for data table
}

// XAccessibleTable -----------------------------------------------------------

OUString SAL_CALL AccessibleBrowseBoxTable::getAccessibleRowDescription( sal_Int32 nRow )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidRow( nRow );
    return mpBrowseBox->GetRowDescription( nRow );
}

OUString SAL_CALL AccessibleBrowseBoxTable::getAccessibleColumnDescription( sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidColumn( nColumn );
    return mpBrowseBox->GetColumnDescription( (sal_uInt16)nColumn );
}

Reference< XAccessibleTable > SAL_CALL AccessibleBrowseBoxTable::getAccessibleRowHeaders()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return implGetHeaderBar( BBINDEX_ROWHEADERBAR );
}

Reference< XAccessibleTable > SAL_CALL AccessibleBrowseBoxTable::getAccessibleColumnHeaders()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return implGetHeaderBar( BBINDEX_COLUMNHEADERBAR );
}

Sequence< sal_Int32 > SAL_CALL AccessibleBrowseBoxTable::getSelectedAccessibleRows()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    Sequence< sal_Int32 > aSelSeq;
    implGetSelectedRows( aSelSeq );
    return aSelSeq;
}

Sequence< sal_Int32 > SAL_CALL AccessibleBrowseBoxTable::getSelectedAccessibleColumns()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    Sequence< sal_Int32 > aSelSeq;
    implGetSelectedColumns( aSelSeq );
    return aSelSeq;
}

sal_Bool SAL_CALL AccessibleBrowseBoxTable::isAccessibleRowSelected( sal_Int32 nRow )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidRow( nRow );
    return implIsRowSelected( nRow );
}

sal_Bool SAL_CALL AccessibleBrowseBoxTable::isAccessibleColumnSelected( sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidColumn( nColumn );
    return implIsColumnSelected( nColumn );
}

Reference< XAccessible > SAL_CALL AccessibleBrowseBoxTable::getAccessibleCellAt(
        sal_Int32 nRow, sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    return mpBrowseBox->CreateAccessibleCell( nRow, (sal_Int16)nColumn );
}

sal_Bool SAL_CALL AccessibleBrowseBoxTable::isAccessibleSelected(
        sal_Int32 nRow, sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    return implIsRowSelected( nRow ) || implIsColumnSelected( nColumn );
}

// XServiceInfo ---------------------------------------------------------------

OUString SAL_CALL AccessibleBrowseBoxTable::getImplementationName()
    throw ( uno::RuntimeException )
{
    return OUString( "com.sun.star.comp.svtools.AccessibleBrowseBoxTable" );
}

// internal virtual methods ---------------------------------------------------

Rectangle AccessibleBrowseBoxTable::implGetBoundingBox()
{
    return mpBrowseBox->calcTableRect(sal_False);
}

Rectangle AccessibleBrowseBoxTable::implGetBoundingBoxOnScreen()
{
    return mpBrowseBox->calcTableRect();
}

// internal helper methods ----------------------------------------------------

Reference< XAccessibleTable > AccessibleBrowseBoxTable::implGetHeaderBar(
        sal_Int32 nChildIndex )
    throw ( uno::RuntimeException )
{
    Reference< XAccessible > xRet;
    Reference< XAccessibleContext > xContext( mxParent, uno::UNO_QUERY );
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
