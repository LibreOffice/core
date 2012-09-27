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
