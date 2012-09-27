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

#include "accessibility/extended/AccessibleBrowseBoxHeaderBar.hxx"
#include <svtools/accessibletableprovider.hxx>
#include <comphelper/servicehelper.hxx>

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

AccessibleBrowseBoxHeaderBar::AccessibleBrowseBoxHeaderBar(
        const Reference< XAccessible >& rxParent,
        IAccessibleTableProvider&                      rBrowseBox,
        AccessibleBrowseBoxObjType      eObjType ) :
    AccessibleBrowseBoxTableBase( rxParent, rBrowseBox,eObjType )
{
    OSL_ENSURE( isRowBar() || isColumnBar(),
        "accessibility/extended/AccessibleBrowseBoxHeaderBar - invalid object type" );
}

AccessibleBrowseBoxHeaderBar::~AccessibleBrowseBoxHeaderBar()
{
}

// XAccessibleContext ---------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleBrowseBoxHeaderBar::getAccessibleChild( sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidHeaderIndex( nChildIndex );
    return implGetChild( nChildIndex, implToVCLColumnPos( nChildIndex ) );
}

sal_Int32 SAL_CALL AccessibleBrowseBoxHeaderBar::getAccessibleIndexInParent()
    throw ( uno::RuntimeException )
{
    return isRowBar() ? BBINDEX_ROWHEADERBAR : BBINDEX_COLUMNHEADERBAR;
}

// XAccessibleComponent -------------------------------------------------------

Reference< XAccessible > SAL_CALL
AccessibleBrowseBoxHeaderBar::getAccessibleAtPoint( const awt::Point& rPoint )
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    sal_Int32 nRow = 0;
    sal_uInt16 nColumnPos = 0;
    sal_Bool bConverted = isRowBar() ?
        mpBrowseBox->ConvertPointToRowHeader( nRow, VCLPoint( rPoint ) ) :
        mpBrowseBox->ConvertPointToColumnHeader( nColumnPos, VCLPoint( rPoint ) );

    return bConverted ? implGetChild( nRow, nColumnPos ) : Reference< XAccessible >();
}

void SAL_CALL AccessibleBrowseBoxHeaderBar::grabFocus()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    // focus on header not supported
}

Any SAL_CALL AccessibleBrowseBoxHeaderBar::getAccessibleKeyBinding()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    return Any();   // no special key bindings for header
}

// XAccessibleTable -----------------------------------------------------------

OUString SAL_CALL AccessibleBrowseBoxHeaderBar::getAccessibleRowDescription( sal_Int32 nRow )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidRow( nRow );
    return OUString();  // no headers in headers
}

OUString SAL_CALL AccessibleBrowseBoxHeaderBar::getAccessibleColumnDescription( sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidColumn( nColumn );
    return OUString();  // no headers in headers
}

Reference< XAccessibleTable > SAL_CALL AccessibleBrowseBoxHeaderBar::getAccessibleRowHeaders()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    return NULL;        // no headers in headers
}

Reference< XAccessibleTable > SAL_CALL AccessibleBrowseBoxHeaderBar::getAccessibleColumnHeaders()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    return NULL;        // no headers in headers
}

Sequence< sal_Int32 > SAL_CALL AccessibleBrowseBoxHeaderBar::getSelectedAccessibleRows()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    Sequence< sal_Int32 > aSelSeq;
    // row of column header bar not selectable
    if( isRowBar() )
        implGetSelectedRows( aSelSeq );
    return aSelSeq;
}

Sequence< sal_Int32 > SAL_CALL AccessibleBrowseBoxHeaderBar::getSelectedAccessibleColumns()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    Sequence< sal_Int32 > aSelSeq;
    // column of row header bar ("handle column") not selectable
    if( isColumnBar() )
        implGetSelectedColumns( aSelSeq );
    return aSelSeq;
}

sal_Bool SAL_CALL AccessibleBrowseBoxHeaderBar::isAccessibleRowSelected( sal_Int32 nRow )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidRow( nRow );
    return isRowBar() ? implIsRowSelected( nRow ) : sal_False;
}

sal_Bool SAL_CALL AccessibleBrowseBoxHeaderBar::isAccessibleColumnSelected( sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidColumn( nColumn );
    return isColumnBar() ? implIsColumnSelected( nColumn ) : sal_False;
}

Reference< XAccessible > SAL_CALL AccessibleBrowseBoxHeaderBar::getAccessibleCellAt(
        sal_Int32 nRow, sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    return implGetChild( nRow, implToVCLColumnPos( nColumn ) );
}

sal_Bool SAL_CALL AccessibleBrowseBoxHeaderBar::isAccessibleSelected(
        sal_Int32 nRow, sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    return isRowBar() ? implIsRowSelected( nRow ) : implIsColumnSelected( nColumn );
}

// XAccessibleSelection -------------------------------------------------------

void SAL_CALL AccessibleBrowseBoxHeaderBar::selectAccessibleChild( sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidHeaderIndex( nChildIndex );
    if( isRowBar() )
        implSelectRow( nChildIndex, sal_True );
    else
        implSelectColumn( implToVCLColumnPos( nChildIndex ), sal_True );
}

sal_Bool SAL_CALL AccessibleBrowseBoxHeaderBar::isAccessibleChildSelected( sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    // using interface methods - no mutex
    return isRowBar() ?
        isAccessibleRowSelected( nChildIndex ) :
        isAccessibleColumnSelected( nChildIndex );
}

void SAL_CALL AccessibleBrowseBoxHeaderBar::clearAccessibleSelection()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    mpBrowseBox->SetNoSelection();
}

void SAL_CALL AccessibleBrowseBoxHeaderBar::selectAllAccessibleChildren()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    // no multiselection of columns possible
    if( isRowBar() )
        mpBrowseBox->SelectAll();
    else
        implSelectColumn( implToVCLColumnPos( 0 ), sal_True );
}

sal_Int32 SAL_CALL AccessibleBrowseBoxHeaderBar::getSelectedAccessibleChildCount()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return isRowBar() ? implGetSelectedRowCount() : implGetSelectedColumnCount();
}

Reference< XAccessible > SAL_CALL
AccessibleBrowseBoxHeaderBar::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    // method may throw lang::IndexOutOfBoundsException
    sal_Int32 nIndex = implGetChildIndexFromSelectedIndex( nSelectedChildIndex );
    return implGetChild( nIndex, implToVCLColumnPos( nIndex ) );
}

void SAL_CALL AccessibleBrowseBoxHeaderBar::deselectAccessibleChild(
        sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();

    // method may throw lang::IndexOutOfBoundsException
    if ( isAccessibleChildSelected(nSelectedChildIndex) )
    {
        if( isRowBar() )
            implSelectRow( nSelectedChildIndex, sal_False );
        else
            implSelectColumn( implToVCLColumnPos( nSelectedChildIndex ), sal_False );
    }
}

// XInterface -----------------------------------------------------------------

Any SAL_CALL AccessibleBrowseBoxHeaderBar::queryInterface( const uno::Type& rType )
    throw ( uno::RuntimeException )
{
    Any aAny( AccessibleBrowseBoxTableBase::queryInterface( rType ) );
    return aAny.hasValue() ?
        aAny : AccessibleBrowseBoxHeaderBarImplHelper::queryInterface( rType );
}

void SAL_CALL AccessibleBrowseBoxHeaderBar::acquire() throw ()
{
    AccessibleBrowseBoxTableBase::acquire();
}

void SAL_CALL AccessibleBrowseBoxHeaderBar::release() throw ()
{
    AccessibleBrowseBoxTableBase::release();
}

// XServiceInfo ---------------------------------------------------------------

OUString SAL_CALL AccessibleBrowseBoxHeaderBar::getImplementationName()
    throw ( uno::RuntimeException )
{
    return OUString( "com.sun.star.comp.svtools.AccessibleBrowseBoxHeaderBar" );
}

namespace
{
    class theAccessibleBrowseBoxHeaderBarImplementationId : public rtl::Static< UnoTunnelIdInit, theAccessibleBrowseBoxHeaderBarImplementationId > {};
}

Sequence< sal_Int8 > SAL_CALL AccessibleBrowseBoxHeaderBar::getImplementationId()
    throw ( uno::RuntimeException )
{
    return theAccessibleBrowseBoxHeaderBarImplementationId::get().getSeq();
}

// internal virtual methods ---------------------------------------------------

Rectangle AccessibleBrowseBoxHeaderBar::implGetBoundingBox()
{
    return mpBrowseBox->calcHeaderRect(isColumnBar(),sal_False);
}

Rectangle AccessibleBrowseBoxHeaderBar::implGetBoundingBoxOnScreen()
{
    return mpBrowseBox->calcHeaderRect(isColumnBar(),sal_True);
}

sal_Int32 AccessibleBrowseBoxHeaderBar::implGetRowCount() const
{
    // column header bar: only 1 row
    return isRowBar() ? AccessibleBrowseBoxTableBase::implGetRowCount() : 1;
}

sal_Int32 AccessibleBrowseBoxHeaderBar::implGetColumnCount() const
{
    // row header bar ("handle column"): only 1 column
    return isColumnBar() ? AccessibleBrowseBoxTableBase::implGetColumnCount() : 1;
}

// internal helper methods ----------------------------------------------------

Reference< XAccessible > AccessibleBrowseBoxHeaderBar::implGetChild(
        sal_Int32 nRow, sal_uInt16 nColumnPos )
{
    return isRowBar() ?
        mpBrowseBox->CreateAccessibleRowHeader( nRow ) :
        mpBrowseBox->CreateAccessibleColumnHeader( nColumnPos );
}

sal_Int32 AccessibleBrowseBoxHeaderBar::implGetChildIndexFromSelectedIndex(
        sal_Int32 nSelectedChildIndex )
    throw ( lang::IndexOutOfBoundsException )
{
    Sequence< sal_Int32 > aSelSeq;
    if( isRowBar() )
        implGetSelectedRows( aSelSeq );
    else
        implGetSelectedColumns( aSelSeq );

    if( (nSelectedChildIndex < 0) || (nSelectedChildIndex >= aSelSeq.getLength()) )
        throw lang::IndexOutOfBoundsException();

    return aSelSeq[ nSelectedChildIndex ];
}

void AccessibleBrowseBoxHeaderBar::ensureIsValidHeaderIndex( sal_Int32 nIndex )
    throw ( lang::IndexOutOfBoundsException )
{
    if( isRowBar() )
        ensureIsValidRow( nIndex );
    else
        ensureIsValidColumn( nIndex );
}

// ============================================================================

} // namespace accessibility

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
