/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tablecolumns.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 10:03:27 $
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

#ifndef _SVX_TABLECOLUMNS_HXX_
#define _SVX_TABLECOLUMNS_HXX_

#include <com/sun/star/table/XTableColumns.hpp>
#include <cppuhelper/implbase1.hxx>

#include "tablemodel.hxx"

// -----------------------------------------------------------------------------

namespace sdr { namespace table {

// -----------------------------------------------------------------------------
// TableColumns
// -----------------------------------------------------------------------------

class TableColumns : public ::cppu::WeakAggImplHelper1< ::com::sun::star::table::XTableColumns >
{
public:
    TableColumns( const TableModelRef& xTableModel );
    virtual ~TableColumns();

    void dispose();
    void throwIfDisposed() const throw (::com::sun::star::uno::RuntimeException);

    // XTableColumns
    virtual void SAL_CALL insertByIndex( sal_Int32 nIndex, sal_Int32 nCount ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByIndex( sal_Int32 nIndex, sal_Int32 nCount ) throw (::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount() throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // Methods
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasElements() throw (::com::sun::star::uno::RuntimeException);

private:
    TableModelRef   mxTableModel;
};

} }

#endif
