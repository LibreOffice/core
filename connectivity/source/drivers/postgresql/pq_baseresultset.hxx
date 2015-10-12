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

class BaseResultSet : public cppu::OComponentHelper,
                      public cppu::OPropertySetHelper,
                      public com::sun::star::sdbc::XCloseable,
                      public com::sun::star::sdbc::XResultSetMetaDataSupplier,
                      public com::sun::star::sdbc::XResultSet,
                      public com::sun::star::sdbc::XRow,
                      public com::sun::star::sdbc::XColumnLocate
{
protected:
    com::sun::star::uno::Any m_props[BASERESULTSET_SIZE];
    com::sun::star::uno::Reference< com::sun::star::uno::XInterface > m_owner;
    com::sun::star::uno::Reference< com::sun::star::script::XTypeConverter > m_tc;
    ::rtl::Reference< RefCountedMutex > m_refMutex;
    sal_Int32 m_row;
    sal_Int32 m_rowCount;
    sal_Int32 m_fieldCount;
    bool  m_wasNull;

protected:
    /** mutex should be locked before called
     */
    virtual void checkClosed()
        throw ( com::sun::star::sdbc::SQLException, com::sun::star::uno::RuntimeException ) = 0;
    void checkColumnIndex( sal_Int32 index )
        throw ( com::sun::star::sdbc::SQLException, com::sun::star::uno::RuntimeException );
    void checkRowIndex( bool mustBeOnValidRow );

    virtual ::com::sun::star::uno::Any getValue( sal_Int32 columnIndex ) = 0;
    com::sun::star::uno::Any convertTo(
        const ::com::sun::star::uno::Any &str, const com::sun::star::uno::Type &type );

protected:
    BaseResultSet(
        const ::rtl::Reference< RefCountedMutex > & mutex,
        const com::sun::star::uno::Reference< com::sun::star::uno::XInterface > &owner,
        sal_Int32 rowCount,
        sal_Int32 columnCount,
        const ::com::sun::star::uno::Reference< ::com::sun::star::script::XTypeConverter > &tc );
    virtual ~BaseResultSet();

public: // XInterface
    virtual void SAL_CALL acquire() throw() override { OComponentHelper::acquire(); }
    virtual void SAL_CALL release() throw() override { OComponentHelper::release(); }
    virtual com::sun::star::uno::Any  SAL_CALL queryInterface(
        const com::sun::star::uno::Type & reqType )
        throw (com::sun::star::uno::RuntimeException, std::exception) override;

public: // XCloseable
//      virtual void SAL_CALL close(  )
//          throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) = 0;

public: // XTypeProvider, first implemented by OPropertySetHelper
    virtual com::sun::star::uno::Sequence< com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;
    virtual com::sun::star::uno::Sequence< sal_Int8> SAL_CALL getImplementationId()
        throw( com::sun::star::uno::RuntimeException, std::exception ) override;

public: // XResultSetMetaDataSupplier
//      virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetMetaData > SAL_CALL getMetaData(  )
//          throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) = 0;

public: // XResultSet
    // Methods
    virtual sal_Bool SAL_CALL next(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isBeforeFirst(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isAfterLast(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isFirst(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL isLast(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL beforeFirst(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL afterLast(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL first(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL last(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getRow(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL absolute( sal_Int32 row )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL relative( sal_Int32 rows )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL previous(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual void SAL_CALL refreshRow(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL rowUpdated(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL rowInserted(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL rowDeleted(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getStatement()
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;


public: // XRow
    virtual sal_Bool SAL_CALL wasNull(  )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual OUString SAL_CALL getString( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Bool SAL_CALL getBoolean( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int8 SAL_CALL getByte( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int16 SAL_CALL getShort( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int32 SAL_CALL getInt( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual sal_Int64 SAL_CALL getLong( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual float SAL_CALL getFloat( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual double SAL_CALL getDouble( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Sequence< sal_Int8 > SAL_CALL getBytes( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::util::Date SAL_CALL getDate( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::util::Time SAL_CALL getTime( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::util::DateTime SAL_CALL getTimestamp( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getBinaryStream( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > SAL_CALL getCharacterStream( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Any SAL_CALL getObject(
        sal_Int32 columnIndex,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >& typeMap )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRef > SAL_CALL getRef( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XBlob > SAL_CALL getBlob( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XClob > SAL_CALL getClob( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XArray > SAL_CALL getArray( sal_Int32 columnIndex )
        throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException, std::exception) override;

public: // XColumnLocate
//      virtual sal_Int32 SAL_CALL findColumn( const OUString& columnName )
//          throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException) = 0;

public: // OPropertySetHelper
    virtual cppu::IPropertyArrayHelper & SAL_CALL getInfoHelper() override;

    virtual sal_Bool SAL_CALL convertFastPropertyValue(
        ::com::sun::star::uno::Any & rConvertedValue,
        ::com::sun::star::uno::Any & rOldValue,
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::lang::IllegalArgumentException) override;

    virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
        sal_Int32 nHandle,
        const ::com::sun::star::uno::Any& rValue )
        throw (::com::sun::star::uno::Exception, std::exception) override;

    using ::cppu::OPropertySetHelper::getFastPropertyValue;

    void SAL_CALL getFastPropertyValue(
        ::com::sun::star::uno::Any& rValue,
        sal_Int32 nHandle ) const override;

    // XPropertySet
    ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySetInfo >  SAL_CALL getPropertySetInfo()
        throw(com::sun::star::uno::RuntimeException, std::exception) override;

public: // OComponentHelper
    virtual void SAL_CALL disposing() override;


};

}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
