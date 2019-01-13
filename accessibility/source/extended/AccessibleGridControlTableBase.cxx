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
#include <extended/AccessibleGridControlTableBase.hxx>
#include <vcl/accessibletable.hxx>
#include <tools/multisel.hxx>
#include <comphelper/sequence.hxx>

using css::uno::Reference;
using css::uno::Sequence;
using css::uno::Any;

using namespace ::com::sun::star;
using namespace ::com::sun::star::accessibility;
using namespace ::vcl;
using namespace ::vcl::table;


namespace accessibility {


AccessibleGridControlTableBase::AccessibleGridControlTableBase(
        const Reference< XAccessible >& rxParent,
        IAccessibleTable& rTable,
        AccessibleTableControlObjType eObjType ) :
    GridControlAccessibleElement( rxParent, rTable, eObjType )
{
}

// XAccessibleContext ---------------------------------------------------------

sal_Int32 SAL_CALL AccessibleGridControlTableBase::getAccessibleChildCount()
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    sal_Int32 nChildren = 0;
    if(m_eObjType == TCTYPE_ROWHEADERBAR)
        nChildren = m_aTable.GetRowCount();
    else if(m_eObjType == TCTYPE_TABLE)
        nChildren = m_aTable.GetRowCount()*m_aTable.GetColumnCount();
    else if(m_eObjType == TCTYPE_COLUMNHEADERBAR)
        nChildren = m_aTable.GetColumnCount();
    return nChildren;
}

sal_Int16 SAL_CALL AccessibleGridControlTableBase::getAccessibleRole()
{
    SolarMutexGuard g;

    ensureIsAlive();
    return AccessibleRole::TABLE;
}

// XAccessibleTable -----------------------------------------------------------

sal_Int32 SAL_CALL AccessibleGridControlTableBase::getAccessibleRowCount()
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    return  m_aTable.GetRowCount();
}

sal_Int32 SAL_CALL AccessibleGridControlTableBase::getAccessibleColumnCount()
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    return m_aTable.GetColumnCount();
}

sal_Int32 SAL_CALL AccessibleGridControlTableBase::getAccessibleRowExtentAt(
        sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    return 1;   // merged cells not supported
}

sal_Int32 SAL_CALL AccessibleGridControlTableBase::getAccessibleColumnExtentAt(
        sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    return 1;   // merged cells not supported
}

Reference< XAccessible > SAL_CALL AccessibleGridControlTableBase::getAccessibleCaption()
{
    SolarMutexGuard g;

    ensureIsAlive();
    return nullptr;    // not supported
}

Reference< XAccessible > SAL_CALL AccessibleGridControlTableBase::getAccessibleSummary()
{
    SolarMutexGuard g;

    ensureIsAlive();
    return nullptr;    // not supported
}

sal_Int32 SAL_CALL AccessibleGridControlTableBase::getAccessibleIndex(
        sal_Int32 nRow, sal_Int32 nColumn )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidAddress( nRow, nColumn );
    return nRow * m_aTable.GetColumnCount() + nColumn;
}

sal_Int32 SAL_CALL AccessibleGridControlTableBase::getAccessibleRow( sal_Int32 nChildIndex )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidIndex( nChildIndex );
    return implGetRow( nChildIndex );
}

sal_Int32 SAL_CALL AccessibleGridControlTableBase::getAccessibleColumn( sal_Int32 nChildIndex )
{
    SolarMutexGuard aSolarGuard;

    ensureIsAlive();
    ensureIsValidIndex( nChildIndex );
    return implGetColumn( nChildIndex );
}

// XInterface -----------------------------------------------------------------

Any SAL_CALL AccessibleGridControlTableBase::queryInterface( const uno::Type& rType )
{
    Any aAny( GridControlAccessibleElement::queryInterface( rType ) );
    return aAny.hasValue() ?
        aAny : AccessibleGridControlTableImplHelper::queryInterface( rType );
}

void SAL_CALL AccessibleGridControlTableBase::acquire() throw ()
{
    GridControlAccessibleElement::acquire();
}

void SAL_CALL AccessibleGridControlTableBase::release() throw ()
{
    GridControlAccessibleElement::release();
}

// XTypeProvider --------------------------------------------------------------

Sequence< uno::Type > SAL_CALL AccessibleGridControlTableBase::getTypes()
{
    return ::comphelper::concatSequences(
        GridControlAccessibleElement::getTypes(),
        AccessibleGridControlTableImplHelper::getTypes() );
}

Sequence< sal_Int8 > SAL_CALL AccessibleGridControlTableBase::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// internal helper methods ----------------------------------------------------

sal_Int32 AccessibleGridControlTableBase::implGetRow( sal_Int32 nChildIndex ) const
{
    sal_Int32 nColumns = m_aTable.GetColumnCount();
    return nColumns ? (nChildIndex / nColumns) : 0;
}

sal_Int32 AccessibleGridControlTableBase::implGetColumn( sal_Int32 nChildIndex ) const
{
    sal_Int32 nColumns = m_aTable.GetColumnCount();
    return nColumns ? (nChildIndex % nColumns) : 0;
}

void AccessibleGridControlTableBase::implGetSelectedRows( Sequence< sal_Int32 >& rSeq )
{
    sal_Int32 const selectionCount( m_aTable.GetSelectedRowCount() );
    rSeq.realloc( selectionCount );
    for ( sal_Int32 i=0; i<selectionCount; ++i )
        rSeq[i] = m_aTable.GetSelectedRowIndex(i);
}

void AccessibleGridControlTableBase::ensureIsValidRow( sal_Int32 nRow )
{
    if( nRow >= m_aTable.GetRowCount() )
        throw lang::IndexOutOfBoundsException( "row index is invalid", *this );
}

void AccessibleGridControlTableBase::ensureIsValidColumn( sal_Int32 nColumn )
{
    if( nColumn >= m_aTable.GetColumnCount() )
        throw lang::IndexOutOfBoundsException( "column index is invalid", *this );
}

void AccessibleGridControlTableBase::ensureIsValidAddress(
        sal_Int32 nRow, sal_Int32 nColumn )
{
    ensureIsValidRow( nRow );
    ensureIsValidColumn( nColumn );
}

void AccessibleGridControlTableBase::ensureIsValidIndex( sal_Int32 nChildIndex )
{
    if( nChildIndex >= m_aTable.GetRowCount()*m_aTable.GetColumnCount() )
        throw lang::IndexOutOfBoundsException( "child index is invalid", *this );
}


} // namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
