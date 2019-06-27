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


/**************************************************************************
                                TODO
 **************************************************************************

 *************************************************************************/

#include <osl/diagnose.h>
#include <com/sun/star/beans/Property.hpp>
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XArray.hpp>
#include <com/sun/star/sdbc/XBlob.hpp>
#include <com/sun/star/sdbc/XClob.hpp>
#include <com/sun/star/sdbc/XRef.hpp>
#include <com/sun/star/util/Date.hpp>
#include <com/sun/star/util/Time.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <com/sun/star/ucb/PropertiesManager.hpp>
#include <ucbhelper/macros.hxx>
#include <ucbhelper/resultsetmetadata.hxx>
#include <cppuhelper/queryinterface.hxx>

using namespace com::sun::star::beans;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;


namespace ucbhelper_impl {

struct ResultSetMetaData_Impl
{
    osl::Mutex                                      m_aMutex;
    std::vector< ::ucbhelper::ResultSetColumnData > m_aColumnData;
    bool                                        m_bObtainedTypes;

    explicit ResultSetMetaData_Impl( sal_Int32 nSize )
    : m_aColumnData( nSize ), m_bObtainedTypes( false ) {}

    explicit ResultSetMetaData_Impl(
        const std::vector< ::ucbhelper::ResultSetColumnData >& rColumnData )
    : m_aColumnData( rColumnData ), m_bObtainedTypes( false ) {}
};

}

using namespace ucbhelper_impl;

namespace ucbhelper {


// ResultSetMetaData Implementation.


ResultSetMetaData::ResultSetMetaData(
                        const Reference< XComponentContext >& rxContext,
                        const Sequence< Property >& rProps )
: m_pImpl( new ResultSetMetaData_Impl( rProps.getLength() ) ),
  m_xContext( rxContext ),
  m_aProps( rProps )
{
}


ResultSetMetaData::ResultSetMetaData(
                        const Reference< XComponentContext >& rxContext,
                        const Sequence< Property >& rProps,
                        const std::vector< ResultSetColumnData >& rColumnData )
: m_pImpl( new ResultSetMetaData_Impl( rColumnData ) ),
  m_xContext( rxContext ),
  m_aProps( rProps )
{
    OSL_ENSURE( rColumnData.size() == sal_uInt32( rProps.getLength() ),
                "ResultSetMetaData ctor - different array sizes!" );
}


// virtual
ResultSetMetaData::~ResultSetMetaData()
{
}


// XInterface methods.

void SAL_CALL ResultSetMetaData::acquire()
    throw()
{
    OWeakObject::acquire();
}

void SAL_CALL ResultSetMetaData::release()
    throw()
{
    OWeakObject::release();
}

css::uno::Any SAL_CALL ResultSetMetaData::queryInterface( const css::uno::Type & rType )
{
    css::uno::Any aRet = cppu::queryInterface( rType,
                                               static_cast< XTypeProvider* >(this),
                                               static_cast< XResultSetMetaData* >(this)
                                               );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}

// XTypeProvider methods.


XTYPEPROVIDER_IMPL_2( ResultSetMetaData,
                      XTypeProvider,
                      XResultSetMetaData );


// XResultSetMetaData methods.


// virtual
sal_Int32 SAL_CALL ResultSetMetaData::getColumnCount()
{
    return m_aProps.getLength();
}


// virtual
sal_Bool SAL_CALL ResultSetMetaData::isAutoIncrement( sal_Int32 /*column*/ )
{
    /*
        Checks whether column is automatically numbered, which makes it
        read-only.
     */
    return false;
}


// virtual
sal_Bool SAL_CALL ResultSetMetaData::isCaseSensitive( sal_Int32 column )
{
    if ( ( column < 1 ) || ( column > m_aProps.getLength() ) )
        return false;

    return m_pImpl->m_aColumnData[ column - 1 ].isCaseSensitive;
}


// virtual
sal_Bool SAL_CALL ResultSetMetaData::isSearchable( sal_Int32 /*column*/ )
{
    return false;
}


// virtual
sal_Bool SAL_CALL ResultSetMetaData::isCurrency( sal_Int32 /*column*/ )
{
    return false;
}


// virtual
sal_Int32 SAL_CALL ResultSetMetaData::isNullable( sal_Int32 /*column*/ )
{
    return ColumnValue::NULLABLE;
}


// virtual
sal_Bool SAL_CALL ResultSetMetaData::isSigned( sal_Int32 /*column*/ )
{
    return false;
}


// virtual
sal_Int32 SAL_CALL ResultSetMetaData::getColumnDisplaySize( sal_Int32 /*column*/ )
{
    /*
        Gets the normal maximum width in characters for column.
     */
    return 16;
}


// virtual
OUString SAL_CALL ResultSetMetaData::getColumnLabel( sal_Int32 column )
{
    /*
        Gets the suggested column title for column, to be used in print-
        outs and displays.
     */

    if ( ( column < 1 ) || ( column > m_aProps.getLength() ) )
        return OUString();

    return m_aProps.getConstArray()[ column - 1 ].Name;
}


// virtual
OUString SAL_CALL ResultSetMetaData::getColumnName( sal_Int32 column )
{
    /*
        Gets the name of column.
     */

    if ( ( column < 1 ) || ( column > m_aProps.getLength() ) )
        return OUString();

    return m_aProps.getConstArray()[ column - 1 ].Name;
}


// virtual
OUString SAL_CALL ResultSetMetaData::getSchemaName( sal_Int32 /*column*/ )
{
    /*
        Gets the schema name for the table from which column of this
        result set was derived.
        Because this feature is not widely supported, the return value
        for many DBMSs will be an empty string.
     */
    return OUString();
}


// virtual
sal_Int32 SAL_CALL ResultSetMetaData::getPrecision( sal_Int32 /*column*/ )
{
    return -1;
}


// virtual
sal_Int32 SAL_CALL ResultSetMetaData::getScale( sal_Int32 /*column*/ )
{
    return 0;
}


// virtual
OUString SAL_CALL ResultSetMetaData::getTableName( sal_Int32 /*column*/ )
{
    /*
        Gets the name of the table from which column of this result set
        was derived or "" if there is none (for example, for a join).
        Because this feature is not widely supported, the return value
        for many DBMSs will be an empty string.
     */
    return OUString();
}


// virtual
OUString SAL_CALL ResultSetMetaData::getCatalogName( sal_Int32 /*column*/ )
{
    /*
        Gets the catalog name for the table from which column of this
        result set was derived.
        Because this feature is not widely supported, the return value
        for many DBMSs will be an empty string.
     */
    return OUString();
}


// virtual
sal_Int32 SAL_CALL ResultSetMetaData::getColumnType( sal_Int32 column )
{
    /*
        Gets the JDBC type for the value stored in column. ... The STRUCT
        and DISTINCT type codes are always returned for structured and
        distinct types, regardless of whether the value will be mapped
        according to the standard mapping or be a custom mapping.
     */

    if ( ( column < 1 ) || ( column > m_aProps.getLength() ) )
        return DataType::SQLNULL;

    if ( m_aProps.getConstArray()[ column - 1 ].Type
            == cppu::UnoType<void>::get() )
    {
        // No type given. Try UCB's Properties Manager...

        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

        if ( !m_pImpl->m_bObtainedTypes )
        {
            try
            {
                Reference< XPropertySetInfo > xInfo = PropertiesManager::create( m_xContext );
    // Less (remote) calls...

                Sequence< Property > aProps = xInfo->getProperties();

                for ( Property& rProp : m_aProps )
                {
                    auto pProp = std::find_if(aProps.begin(), aProps.end(),
                        [&rProp](const Property& rProp1) { return rProp.Name == rProp1.Name; });
                    if (pProp != aProps.end())
                    {
                        // Found...
                        rProp.Type = pProp->Type;
                    }
                }
            }
            catch ( RuntimeException& )
            {
                throw;
            }
            catch ( Exception& )
            {
                // createInstance
            }

            m_pImpl->m_bObtainedTypes = true;
        }
    }

    const Type& rType = m_aProps.getConstArray()[ column - 1 ].Type;
    sal_Int32 nType = DataType::OTHER;

    if ( rType == cppu::UnoType<OUString>::get() )
        nType = DataType::VARCHAR;  // XRow::getString
    else if ( rType == cppu::UnoType<bool>::get() )
        nType = DataType::BIT;      // XRow::getBoolean
    else if ( rType == cppu::UnoType<sal_Int32>::get() )
        nType = DataType::INTEGER;  // XRow::getInt
    else if ( rType == cppu::UnoType<sal_Int64>::get() )
        nType = DataType::BIGINT;   // XRow::getLong
    else if ( rType == cppu::UnoType<sal_Int16>::get() )
        nType = DataType::SMALLINT; // XRow::getShort
    else if ( rType == cppu::UnoType<sal_Int8>::get() )
        nType = DataType::TINYINT;  // XRow::getByte
    else if ( rType == cppu::UnoType<float>::get() )
        nType = DataType::REAL;     // XRow::getFloat
    else if ( rType == cppu::UnoType<double>::get() )
        nType = DataType::DOUBLE;   // XRow::getDouble
    else if ( rType == cppu::UnoType<Sequence<sal_Int8>>::get() )
        nType = DataType::VARBINARY;// XRow::getBytes
    else if ( rType == cppu::UnoType<Date>::get() )
        nType = DataType::DATE;     // XRow::getDate
    else if ( rType == cppu::UnoType<Time>::get() )
        nType = DataType::TIME;     // XRow::getTime
    else if ( rType == cppu::UnoType<DateTime>::get() )
        nType = DataType::TIMESTAMP;// XRow::getTimestamp
    else if ( rType == cppu::UnoType<XInputStream>::get() )
        nType = DataType::LONGVARBINARY;    // XRow::getBinaryStream
//      nType = DataType::LONGVARCHAR;      // XRow::getCharacterStream
    else if ( rType == cppu::UnoType<XClob>::get() )
        nType = DataType::CLOB; // XRow::getClob
    else if ( rType == cppu::UnoType<XBlob>::get() )
        nType = DataType::BLOB; // XRow::getBlob
    else if ( rType == cppu::UnoType<XArray>::get() )
        nType = DataType::ARRAY;// XRow::getArray
    else if ( rType == cppu::UnoType<XRef>::get() )
        nType = DataType::REF;// XRow::getRef
    else
        nType = DataType::OBJECT;// XRow::getObject

    return nType;
}


// virtual
OUString SAL_CALL ResultSetMetaData::getColumnTypeName( sal_Int32 /*column*/ )
{
    /*
        Gets the type name used by this particular data source for the
        values stored in column. If the type code for the type of value
        stored in column is STRUCT, DISTINCT or JAVA_OBJECT, this method
        returns a fully-qualified SQL type name.
     */
    return OUString();
}


// virtual
sal_Bool SAL_CALL ResultSetMetaData::isReadOnly( sal_Int32 /*column*/ )
{
    return true;
}


// virtual
sal_Bool SAL_CALL ResultSetMetaData::isWritable( sal_Int32 /*column*/ )
{
    return false;
}


// virtual
sal_Bool SAL_CALL ResultSetMetaData::isDefinitelyWritable( sal_Int32 /*column*/ )
{
    return false;
}


// virtual
OUString SAL_CALL ResultSetMetaData::getColumnServiceName( sal_Int32 /*column*/ )
{
    /*
          Returns the fully-qualified name of the service whose instances
         are manufactured if XResultSet::getObject is called to retrieve
        a value from the column.
     */
    return OUString();
}

} // namespace ucbhelper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
