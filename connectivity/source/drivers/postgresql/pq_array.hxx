/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  Effective License of whole file:
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
 *  Parts "Copyright by Sun Microsystems, Inc" prior to August 2011:
 *
 *    The Contents of this file are made available subject to the terms of
 *    the GNU Lesser General Public License Version 2.1
 *
 *    Copyright: 2000 by Sun Microsystems, Inc.
 *
 *    Contributor(s): Joerg Budischewski
 *
 *  All parts contributed on or after August 2011:
 *
 *    This Source Code Form is subject to the terms of the Mozilla Public
 *    License, v. 2.0. If a copy of the MPL was not distributed with this
 *    file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 ************************************************************************/

#pragma once
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/sdbc/XArray.hpp>

#include "pq_connection.hxx"
#include <vector>

namespace pq_sdbc_driver
{

class Array : public cppu::WeakImplHelper< css::sdbc::XArray >
{
    std::vector< css::uno::Any > m_data;
    css::uno::Reference< css::uno::XInterface > m_owner;
    css::uno::Reference< css::script::XTypeConverter > m_tc;
    rtl::Reference< comphelper::RefCountedMutex > m_xMutex;

public:
    Array(
        const rtl::Reference< comphelper::RefCountedMutex > & mutex,
        const std::vector< css::uno::Any > & data,
        const css::uno::Reference< css::uno::XInterface > & owner,
        const css::uno::Reference< css::script::XTypeConverter > &tc) :
        m_data( data ),
        m_owner( owner ),
        m_tc( tc ),
        m_xMutex( mutex )
    {}

public: // XArray

    // Methods
    virtual OUString SAL_CALL getBaseTypeName(  ) override;

    virtual sal_Int32 SAL_CALL getBaseType(  ) override;

    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getArray(
        const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;

    virtual css::uno::Sequence< css::uno::Any > SAL_CALL getArrayAtIndex(
        sal_Int32 index,
        sal_Int32 count,
        const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;

    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL
    getResultSet(
        const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;

    virtual css::uno::Reference< css::sdbc::XResultSet > SAL_CALL getResultSetAtIndex(
        sal_Int32 index,
        sal_Int32 count,
        const css::uno::Reference< css::container::XNameAccess >& typeMap ) override;

private:
    void checkRange( sal_Int32 index, sal_Int32 count );
};


};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
