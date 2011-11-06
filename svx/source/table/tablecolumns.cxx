/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"

#include <com/sun/star/lang/DisposedException.hpp>

#include "tablecolumns.hxx"
#include "tablecolumn.hxx"

// -----------------------------------------------------------------------------

using ::rtl::OUString;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::table;

// -----------------------------------------------------------------------------

namespace sdr { namespace table {

// -----------------------------------------------------------------------------
// TableColumns
// -----------------------------------------------------------------------------

TableColumns::TableColumns( const TableModelRef& xTableModel )
: mxTableModel( xTableModel )
{
}

// -----------------------------------------------------------------------------

TableColumns::~TableColumns()
{
    dispose();
}

// -----------------------------------------------------------------------------

void TableColumns::dispose()
{
    mxTableModel.clear();
}

// -----------------------------------------------------------------------------

void TableColumns::throwIfDisposed() const throw (::com::sun::star::uno::RuntimeException)
{
    if( !mxTableModel.is() )
        throw DisposedException();
}

// -----------------------------------------------------------------------------
// XTableRows
// -----------------------------------------------------------------------------

void SAL_CALL TableColumns::insertByIndex( sal_Int32 nIndex, sal_Int32 nCount ) throw (RuntimeException)
{
    throwIfDisposed();
    mxTableModel->insertColumns( nIndex, nCount );
}

// -----------------------------------------------------------------------------

void SAL_CALL TableColumns::removeByIndex( sal_Int32 nIndex, sal_Int32 nCount ) throw (RuntimeException)
{
    throwIfDisposed();
    mxTableModel->removeColumns( nIndex, nCount );
}

// -----------------------------------------------------------------------------
// XIndexAccess
// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL TableColumns::getCount() throw (RuntimeException)
{
    throwIfDisposed();
    return mxTableModel->getColumnCount();
}

// -----------------------------------------------------------------------------

Any SAL_CALL TableColumns::getByIndex( sal_Int32 Index ) throw (IndexOutOfBoundsException, WrappedTargetException, RuntimeException)
{
    throwIfDisposed();

    if( ( Index < 0 ) || ( Index >= mxTableModel->getColumnCount() ) )
        throw IndexOutOfBoundsException();

    return Any( Reference< XCellRange >( mxTableModel->getColumn( Index ).get() ) );
}

// -----------------------------------------------------------------------------
// XElementAccess
// -----------------------------------------------------------------------------

Type SAL_CALL TableColumns::getElementType() throw (RuntimeException)
{
    throwIfDisposed();

    return XCellRange::static_type();
}

// -----------------------------------------------------------------------------

sal_Bool SAL_CALL TableColumns::hasElements() throw (RuntimeException)
{
    throwIfDisposed();

    return mxTableModel->getColumnCount() != 0;
}

// -----------------------------------------------------------------------------

} }
