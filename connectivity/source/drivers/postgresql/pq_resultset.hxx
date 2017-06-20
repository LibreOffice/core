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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_RESULTSET_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_RESULTSET_HXX

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/component.hxx>

#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include "pq_connection.hxx"
#include "pq_baseresultset.hxx"

namespace pq_sdbc_driver
{

class ResultSet : public BaseResultSet
{
protected:
    PGresult *m_result;
    OUString m_schema;
    OUString m_table;
    ConnectionSettings **m_ppSettings;

protected:
    /** mutex should be locked before called
     */
    virtual void checkClosed() override;

    /** unchecked, acquire mutex before calling
     */
    virtual css::uno::Any getValue( sal_Int32 columnIndex ) override;

public:
    ResultSet(
        const ::rtl::Reference< comphelper::RefCountedMutex > & mutex,
        const css::uno::Reference< css::uno::XInterface > &owner,
        ConnectionSettings **pSettings,
        PGresult *result,
        const OUString &schema,
        const OUString &table );
    virtual ~ResultSet() override;

public: // XCloseable
    virtual void SAL_CALL close(  ) override;

public: // XResultSetMetaDataSupplier
    virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  ) override;

public: // XColumnLocate
    virtual sal_Int32 SAL_CALL findColumn( const OUString& columnName ) override;

public:
    sal_Int32 guessDataType( sal_Int32 column );
};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
