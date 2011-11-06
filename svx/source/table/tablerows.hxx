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



#ifndef _SVX_TABLEROWS_HXX_
#define _SVX_TABLEROWS_HXX_

#include <com/sun/star/table/XTableRows.hpp>
#include <cppuhelper/implbase1.hxx>

#include "tablemodel.hxx"

// -----------------------------------------------------------------------------

namespace sdr { namespace table {

// -----------------------------------------------------------------------------
// TableRows
// -----------------------------------------------------------------------------

class TableRows : public ::cppu::WeakAggImplHelper1< ::com::sun::star::table::XTableRows >
{
public:
    TableRows( const TableModelRef& xTableModel );
    virtual ~TableRows();

    void dispose();
    void throwIfDisposed() const throw (::com::sun::star::uno::RuntimeException);

    // XTableRows
    virtual void SAL_CALL insertByIndex( sal_Int32 nIndex, sal_Int32 nCount ) throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL removeByIndex( sal_Int32 nIndex, sal_Int32 nCount ) throw (::com::sun::star::uno::RuntimeException);

    // XIndexAccess
    virtual sal_Int32 SAL_CALL getCount(  ) throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Any SAL_CALL getByIndex( sal_Int32 Index ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

    // Methods
    virtual ::com::sun::star::uno::Type SAL_CALL getElementType() throw (::com::sun::star::uno::RuntimeException);
    virtual ::sal_Bool SAL_CALL hasElements() throw (::com::sun::star::uno::RuntimeException);

private:
    TableModelRef   mxTableModel;
};

} }

#endif
