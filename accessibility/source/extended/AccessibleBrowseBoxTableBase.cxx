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

#include <extended/AccessibleBrowseBoxTableBase.hxx>
#include <vcl/accessibletableprovider.hxx>
#include <comphelper/sequence.hxx>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>


using css::uno::Reference;
using css::uno::Sequence;
using css::uno::Any;

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;


namespace accessibility {


// Ctor/Dtor/disposing --------------------------------------------------------

AccessibleBrowseBoxTableBase::AccessibleBrowseBoxTableBase(
        const Reference< XAccessible >& rxParent,
        vcl::IAccessibleTableProvider& rBrowseBox,
        AccessibleBrowseBoxObjType eObjType ) :
    BrowseBoxAccessibleElement( rxParent, rBrowseBox,nullptr, eObjType )
{
}

// XAccessibleContext ---------------------------------------------------------

sal_Int64 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleChildCount()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();
    return implGetChildCount();
}

sal_Int16 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleRole()
{
    osl::MutexGuard aGuard( getMutex() );
    ensureIsAlive();
    return AccessibleRole::TABLE;
}

// XAccessibleTable -----------------------------------------------------------

sal_Int32 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleRowCount()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();
    return implGetRowCount();
}

sal_Int32 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleColumnCount()
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();
    return implGetColumnCount();
}

sal_Int32 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleRowExtentAt(
        sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    return 1;   // merged cells not supported
}

sal_Int32 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleColumnExtentAt(
        sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    return 1;   // merged cells not supported
}

Reference< XAccessible > SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleCaption()
{
    ensureIsAlive();
    return nullptr;    // not supported
}

Reference< XAccessible > SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleSummary()
{
    ensureIsAlive();
    return nullptr;    // not supported
}

sal_Int64 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleIndex(
        sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    return static_cast<sal_Int64>(nRow) * static_cast<sal_Int64>(implGetColumnCount()) + nColumn;
}

sal_Int32 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleRow( sal_Int64 nChildIndex )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();
    ensureIsValidIndex( nChildIndex );
    return implGetRow( nChildIndex );
}

sal_Int32 SAL_CALL AccessibleBrowseBoxTableBase::getAccessibleColumn( sal_Int64 nChildIndex )
{
    SolarMethodGuard aGuard(getMutex());
    ensureIsAlive();
    ensureIsValidIndex( nChildIndex );
    return implGetColumn( nChildIndex );
}

// XInterface -----------------------------------------------------------------

Any SAL_CALL AccessibleBrowseBoxTableBase::queryInterface( const uno::Type& rType )
{
    Any aAny( BrowseBoxAccessibleElement::queryInterface( rType ) );
    return aAny.hasValue() ?
        aAny : AccessibleBrowseBoxTableImplHelper::queryInterface( rType );
}

void SAL_CALL AccessibleBrowseBoxTableBase::acquire() noexcept
{
    BrowseBoxAccessibleElement::acquire();
}

void SAL_CALL AccessibleBrowseBoxTableBase::release() noexcept
{
    BrowseBoxAccessibleElement::release();
}

// XTypeProvider --------------------------------------------------------------

Sequence< uno::Type > SAL_CALL AccessibleBrowseBoxTableBase::getTypes()
{
    return ::comphelper::concatSequences(
        BrowseBoxAccessibleElement::getTypes(),
        AccessibleBrowseBoxTableImplHelper::getTypes() );
}

Sequence< sal_Int8 > SAL_CALL AccessibleBrowseBoxTableBase::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
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

bool AccessibleBrowseBoxTableBase::implHasHandleColumn() const
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

sal_Int64 AccessibleBrowseBoxTableBase::implGetChildCount() const
{
    return static_cast<sal_Int64>(implGetRowCount()) * static_cast<sal_Int64>(implGetColumnCount());
}

sal_Int32 AccessibleBrowseBoxTableBase::implGetRow( sal_Int64 nChildIndex ) const
{
    sal_Int32 nColumns = implGetColumnCount();
    return nColumns ? (nChildIndex / nColumns) : 0;
}

sal_Int32 AccessibleBrowseBoxTableBase::implGetColumn( sal_Int64 nChildIndex ) const
{
    sal_Int32 nColumns = implGetColumnCount();
    return nColumns ? (nChildIndex % nColumns) : 0;
}

bool AccessibleBrowseBoxTableBase::implIsRowSelected( sal_Int32 nRow ) const
{
    return mpBrowseBox->IsRowSelected( nRow );
}

bool AccessibleBrowseBoxTableBase::implIsColumnSelected( sal_Int32 nColumn ) const
{
    if( implHasHandleColumn() )
        --nColumn;
    return mpBrowseBox->IsColumnSelected( nColumn );
}

void AccessibleBrowseBoxTableBase::implSelectRow( sal_Int32 nRow, bool bSelect )
{
    mpBrowseBox->SelectRow( nRow, bSelect );
}

void AccessibleBrowseBoxTableBase::implSelectColumn( sal_Int32 nColumnPos, bool bSelect )
{
    mpBrowseBox->SelectColumn( static_cast<sal_uInt16>(nColumnPos), bSelect );
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
{
    if( nRow >= implGetRowCount() )
        throw lang::IndexOutOfBoundsException( u"row index is invalid"_ustr, *this );
}

void AccessibleBrowseBoxTableBase::ensureIsValidColumn( sal_Int32 nColumn )
{
    if( nColumn >= implGetColumnCount() )
        throw lang::IndexOutOfBoundsException( u"column index is invalid"_ustr, *this );
}

void AccessibleBrowseBoxTableBase::ensureIsValidAddress(
        sal_Int32 nRow, sal_Int32 nColumn )
{
    ensureIsValidRow( nRow );
    ensureIsValidColumn( nColumn );
}

void AccessibleBrowseBoxTableBase::ensureIsValidIndex( sal_Int64 nChildIndex )
{
    if( nChildIndex >= implGetChildCount() )
        throw lang::IndexOutOfBoundsException( u"child index is invalid"_ustr, *this );
}


} // namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
