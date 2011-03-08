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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <com/sun/star/lang/DisposedException.hpp>

#include "cell.hxx"
#include "tablerow.hxx"
#include "tablerows.hxx"

// -----------------------------------------------------------------------------

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::table;

// -----------------------------------------------------------------------------

namespace sdr { namespace table {

// -----------------------------------------------------------------------------
// TableRows
// -----------------------------------------------------------------------------

TableRows::TableRows( const TableModelRef& xTableModel )
: mxTableModel( xTableModel )
{
}

// -----------------------------------------------------------------------------

TableRows::~TableRows()
{
    dispose();
}

// -----------------------------------------------------------------------------

void TableRows::dispose()
{
    mxTableModel.clear();
}

// -----------------------------------------------------------------------------

void TableRows::throwIfDisposed() const throw (::com::sun::star::uno::RuntimeException)
{
    if( !mxTableModel.is() )
        throw DisposedException();
}

// -----------------------------------------------------------------------------
// XTableRows
// -----------------------------------------------------------------------------

void SAL_CALL TableRows::insertByIndex( sal_Int32 nIndex, sal_Int32 nCount ) throw (RuntimeException)
{
    throwIfDisposed();
    mxTableModel->insertRows( nIndex, nCount );
}

// -----------------------------------------------------------------------------

void SAL_CALL TableRows::removeByIndex( sal_Int32 nIndex, sal_Int32 nCount ) throw (RuntimeException)
{
    throwIfDisposed();
    mxTableModel->removeRows( nIndex, nCount );
}

// -----------------------------------------------------------------------------
// XIndexAccess
// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL TableRows::getCount() throw (RuntimeException)
{
    throwIfDisposed();
    return mxTableModel->getRowCount();
}

// -----------------------------------------------------------------------------

Any SAL_CALL TableRows::getByIndex( sal_Int32 Index ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    throwIfDisposed();
    return Any( Reference< XCellRange >( static_cast< XCellRange* >( mxTableModel->getRow( Index ).get() ) ) );
}

// -----------------------------------------------------------------------------
// XElementAccess
// -----------------------------------------------------------------------------

Type SAL_CALL TableRows::getElementType() throw (RuntimeException)
{
    throwIfDisposed();
    return XCellRange::static_type();
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL TableRows::hasElements() throw (RuntimeException)
{
    throwIfDisposed();
    return mxTableModel->getRowCount() != 0;
}

// -----------------------------------------------------------------------------

} }

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
