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

#ifndef INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_BASERESULTSET_HXX
#define INCLUDED_CONNECTIVITY_SOURCE_DRIVERS_POSTGRESQL_PQ_BASERESULTSET_HXX

#include <cppuhelper/propshlp.hxx>
#include <cppuhelper/component.hxx>

#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/sdbc/XCloseable.hpp>
#include <com/sun/star/sdbc/XColumnLocate.hpp>
#include "pq_connection.hxx"

namespace pq_sdbc_driver
{

static const sal_Int32 BASERESULTSET_CURSOR_NAME = 0;
static const sal_Int32 BASERESULTSET_ESCAPE_PROCESSING = 1;
static const sal_Int32 BASERESULTSET_FETCH_DIRECTION = 2;
static const sal_Int32 BASERESULTSET_FETCH_SIZE = 3;
static const sal_Int32 BASERESULTSET_IS_BOOKMARKABLE = 4;
static const sal_Int32 BASERESULTSET_RESULT_SET_CONCURRENCY = 5;
static const sal_Int32 BASERESULTSET_RESULT_SET_TYPE = 6;

#define BASERESULTSET_SIZE 7

typedef ::cppu::WeakComponentImplHelper<    css::sdbc::XCloseable,
                                            css::sdbc::XResultSetMetaDataSupplier,
                                            css::sdbc::XResultSet,
                                            css::sdbc::XRow,
                                            css::sdbc::XColumnLocate
                                            > BaseResultSet_BASE;
class BaseResultSet : public BaseResultSet_BASE,
                      public cppu::OPropertySetHelper
{
protected:
    css::uno::Any m_props[BASERESULTSET_SIZE];
    css::uno::Reference< css::uno::XInterface > m_owner;
    css::uno::Reference< css::script::XTypeConverter > m_tc;
    ::rtl::Reference< RefCountedMutex > m_refMutex;
    sal_Int32 m_row;
    sal_Int32 m_rowCount;
    sal_Int32 m_fieldCount;
    bool  m_wasNull;

protected:
    /** mutex should be locked before called
     */
    virtual void checkClosed()
        throw ( css::sdbc::SQLException, css::uno::RuntimeException ) = 0;
    void checkColumnIndex( sal_Int32 index )
        throw ( css::sdbc::SQLException, css::uno::RuntimeException );
    void checkRowIndex();

    virtual css::uno::Any getValue( sal_Int32 columnIndex ) = 0;
    css::uno::Any convertTo(
        const css::uno::Any &str, const css::uno::Type &type );

protected:
    BaseResultSet(
        const ::rtl::Reference< RefCountedMutex > & mutex,
        const css::uno::Reference< css::uno::XInterface > &owner,
        sal_Int32 rowCount,
        sal_Int32 columnCount,
        const css::uno::Reference< css::script::XTypeConverter > &tc );
    virtual ~BaseResultSet();

public: // XInterface
    virtual void SAL_CALL acquire() throw() override { BaseResultSet_BASE::acquire(); }
    virtual void SAL_CALL release() throw() override { BaseResultSet_BASE::release(); }
    virtual css::uno::Any  SAL_CALL queryInterface(
        const css::uno::Type & reqType )
        throw (css::uno::RuntimeException, std::exception) override;

public: // XCloseable
//      virtual void SAL_CALL close(  )
//          throw (css::sdbc::SQLException, css::uno::RuntimeException) = 0;

public: // XTypeProvider, first implemented by OPropertySetHelper
    virtual css::uno::Sequence< css::uno::Type > SAL_CALL getTypes()
        throw( css::uno::RuntimeException, std::exception ) override;
    virtual css::uno::Sequence< sal_Int8> SAL_CALL getImplementationId()
        throw( css::uno::RuntimeException, std::exception ) override;

public: // XResultSetMetaDataSupplier
//      virtual css::uno::Reference< css::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  )
//          throw (css::sdbc::SQLException, css::uno::RuntimeException) = 0;

public: // XResultSet
    // Methods
    virtual sal_Bool SAL_CALL next(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isBeforeFirst(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAfterLast(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isFirst(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isLast(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL beforeFirst(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL afterLast(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL first(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL last(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getRow(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL absolute( sal_Int32 row )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL relative( sal_Int32 rows )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL previous(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL refreshRow(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL rowUpdated(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL rowInserted(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL rowDeleted(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::uno::XInterface > SAL_CALL getStatement()
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;


public: // XRow
    virtual sal_Bool SAL_CALL wasNull(  )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getString( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual float SAL_CALL getFloat( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getDouble( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::util::Date SAL_CALL getDate( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::util::Time SAL_CALL getTime( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Any SAL_CALL getObject(
        sal_Int32 columnIndex,
        const css::uno::Reference< css::container::XNameAccess >& typeMap )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;
    virtual css::uno::Reference< css::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex )
        throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception) override;

public: // XColumnLocate
//      virtual sal_Int32 SAL_CALL findColumn( const OUString& columnName )
//          throw (css::sdbc::SQLException, css::uno::RuntimeException) = 0;

public: // OPropertySetHelper
    virtual cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        css::uno::Any & rConvertedValue,
        css::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const css::uno::Any& rValue )
        throw (css::lang::IllegalArgumentException) override;

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const css::uno::Any& rValue )
        throw (css::uno::Exception, std::exception) override;

    using ::cppu::OPropertySetHelper::getFastPropertyValue;

    void SAL_CALL getFastPropertyValue(
        css::uno::Any& rValue,
        sal_Int32 nHandle ) const override;

    // XPropertySet
    css::uno::Reference < css::beans::XPropertySetInfo >  SAL_CALL getPropertySetInfo()
        throw(css::uno::RuntimeException, std::exception) override;

public: // OComponentHelper
    virtual void SAL_CALL disposing() override;


};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
