/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tablecolumns.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:03:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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
