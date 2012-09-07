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

#include "accessibility/extended/AccessibleBrowseBoxTableBase.hxx"
#include <svtools/accessibletableprovider.hxx>
#include <tools/multisel.hxx>
#include <comphelper/sequence.hxx>
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

AccessibleBrowseBoxTableBase::AccessibleBrowseBoxTableBase(
        const Reference< XAccessible >& rxParent,
        IAccessibleTableProvider&                      rBrowseBox,
        AccessibleBrowseBoxObjType      eObjType ) :
    BrowseBoxAccessibleElement( rxParent, rBrowseBox,NULL, eObjType )
{
}

AccessibleBrowseBoxTableBase::~AccessibleBrowseBoxTableBase()
{
}

// XAccessibleContext ---------------------------------------------------------

sal_Int32 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleChildCount()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return implGetChildCount();
}

sal_Int16 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleRole()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    return AccessibleRole::TABLE;
}

// XAccessibleTable -----------------------------------------------------------

sal_Int32 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleRowCount()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return implGetRowCount();
}

sal_Int32 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleColumnCount()
    throw ( uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    return implGetColumnCount();
}

sal_Int32 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleRowExtentAt(
        sal_Int32 nRow, sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    return 1;   // merged cells not supported
}

sal_Int32 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleColumnExtentAt(
        sal_Int32 nRow, sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    return 1;   // merged cells not supported
}

Reference< XAccessible > SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleCaption()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    return NULL;    // not supported
}

Reference< XAccessible > SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleSummary()
    throw ( uno::RuntimeException )
{
    ensureIsAlive();
    return NULL;    // not supported
}

sal_Int32 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleIndex(
        sal_Int32 nRow, sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    return implGetChildIndex( nRow, nColumn );
}

sal_Int32 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleRow( sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidIndex( nChildIndex );
    return implGetRow( nChildIndex );
}

sal_Int32 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleColumn( sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException, uno::RuntimeException )
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( getOslMutex() );
    ensureIsAlive();
    ensureIsValidIndex( nChildIndex );
    return implGetColumn( nChildIndex );
}

// XInterface -----------------------------------------------------------------

Any SAL_CALL AccessibleBrowseBoxTableBase::queryInterface( const uno::Type& rType )
    throw ( uno::RuntimeException )
{
    Any aAny( BrowseBoxAccessibleElement::queryInterface( rType ) );
    return aAny.hasValue() ?
        aAny : AccessibleBrowseBoxTableImplHelper::queryInterface( rType );
}

void SAL_CALL AccessibleBrowseBoxTableBase::acquire() throw ()
{
    BrowseBoxAccessibleElement::acquire();
}

void SAL_CALL AccessibleBrowseBoxTableBase::release() throw ()
{
    BrowseBoxAccessibleElement::release();
}

// XTypeProvider --------------------------------------------------------------

Sequence< uno::Type > SAL_CALL AccessibleBrowseBoxTableBase::getTypes()
    throw ( uno::RuntimeException )
{
    return ::comphelper::concatSequences(
        BrowseBoxAccessibleElement::getTypes(),
        AccessibleBrowseBoxTableImplHelper::getTypes() );
}

namespace
{
    class theAccessibleBrowseBoxTableBaseImplementationId : public rtl::Static< UnoTunnelIdInit, theAccessibleBrowseBoxTableBaseImplementationId > {};
}

Sequence< sal_Int8 > SAL_CALL AccessibleBrowseBoxTableBase::getImplementationId()
    throw ( uno::RuntimeException )
{
    return theAccessibleBrowseBoxTableBaseImplementationId::get().getSeq();
}

// internal virtual methods ---------------------------------------------------

sal_Int32 AccessibleBrowseBoxTableBase::implGetRowCount() const
{
    return mpBrowseBox->GetRowCount();
}

sal_Int32 AccessibleBrowseBoxTableBase::implGetColumnCount() const
{
    sal_uInt16 nColumns = mpBrowseBox->GetColumnCount();
    // do not count the "handle column"
    if( nColumns && implHasHandleColumn() )
        --nColumns;
    return nColumns;
}

// internal helper methods ----------------------------------------------------

sal_Bool AccessibleBrowseBoxTableBase::implHasHandleColumn() const
{
    return mpBrowseBox->HasRowHeader();
}

sal_uInt16 AccessibleBrowseBoxTableBase::implToVCLColumnPos( sal_Int32 nColumn ) const
{
    sal_uInt16 nVCLPos = 0;
    if( (0 <= nColumn) && (nColumn < implGetColumnCount()) )
    {
        // regard "handle column"
        if( implHasHandleColumn() )
            ++nColumn;
        nVCLPos = static_cast< sal_uInt16 >( nColumn );
    }
    return nVCLPos;
}

sal_Int32 AccessibleBrowseBoxTableBase::implGetChildCount() const
{
    return implGetRowCount() * implGetColumnCount();
}

sal_Int32 AccessibleBrowseBoxTableBase::implGetRow( sal_Int32 nChildIndex ) const
{
    sal_Int32 nColumns = implGetColumnCount();
    return nColumns ? (nChildIndex / nColumns) : 0;
}

sal_Int32 AccessibleBrowseBoxTableBase::implGetColumn( sal_Int32 nChildIndex ) const
{
    sal_Int32 nColumns = implGetColumnCount();
    return nColumns ? (nChildIndex % nColumns) : 0;
}

sal_Int32 AccessibleBrowseBoxTableBase::implGetChildIndex(
        sal_Int32 nRow, sal_Int32 nColumn ) const
{
    return nRow * implGetColumnCount() + nColumn;
}

sal_Bool AccessibleBrowseBoxTableBase::implIsRowSelected( sal_Int32 nRow ) const
{
    return mpBrowseBox->IsRowSelected( nRow );
}

sal_Bool AccessibleBrowseBoxTableBase::implIsColumnSelected( sal_Int32 nColumn ) const
{
    if( implHasHandleColumn() )
        --nColumn;
    return mpBrowseBox->IsColumnSelected( nColumn );
}

void AccessibleBrowseBoxTableBase::implSelectRow( sal_Int32 nRow, sal_Bool bSelect )
{
    mpBrowseBox->SelectRow( nRow, bSelect, sal_True );
}

void AccessibleBrowseBoxTableBase::implSelectColumn( sal_Int32 nColumnPos, sal_Bool bSelect )
{
    mpBrowseBox->SelectColumn( (sal_uInt16)nColumnPos, bSelect );
}

sal_Int32 AccessibleBrowseBoxTableBase::implGetSelectedRowCount() const
{
    return mpBrowseBox->GetSelectedRowCount();
}

sal_Int32 AccessibleBrowseBoxTableBase::implGetSelectedColumnCount() const
{
    return mpBrowseBox->GetSelectedColumnCount();
}

void AccessibleBrowseBoxTableBase::implGetSelectedRows( Sequence< sal_Int32 >& rSeq )
{
    mpBrowseBox->GetAllSelectedRows( rSeq );
}

void AccessibleBrowseBoxTableBase::implGetSelectedColumns( Sequence< sal_Int32 >& rSeq )
{
    mpBrowseBox->GetAllSelectedColumns( rSeq );
}

void AccessibleBrowseBoxTableBase::ensureIsValidRow( sal_Int32 nRow )
    throw ( lang::IndexOutOfBoundsException )
{
    if( nRow >= implGetRowCount() )
        throw lang::IndexOutOfBoundsException(
            OUString( "row index is invalid" ), *this );
}

void AccessibleBrowseBoxTableBase::ensureIsValidColumn( sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException )
{
    if( nColumn >= implGetColumnCount() )
        throw lang::IndexOutOfBoundsException(
            OUString( "column index is invalid" ), *this );
}

void AccessibleBrowseBoxTableBase::ensureIsValidAddress(
        sal_Int32 nRow, sal_Int32 nColumn )
    throw ( lang::IndexOutOfBoundsException )
{
    ensureIsValidRow( nRow );
    ensureIsValidColumn( nColumn );
}

void AccessibleBrowseBoxTableBase::ensureIsValidIndex( sal_Int32 nChildIndex )
    throw ( lang::IndexOutOfBoundsException )
{
    if( nChildIndex >= implGetChildCount() )
        throw lang::IndexOutOfBoundsException(
            OUString( "child index is invalid" ), *this );
}

// ============================================================================

} // namespace accessibility

// ============================================================================

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
