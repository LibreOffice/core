/*************************************************************************
 *
 *  $RCSfile: resultsetmetadata.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 - Search for @@@ in this file to find out what's to do

 *************************************************************************/

#ifndef _COM_SUN_STAR_BEANS_PROPERTY_HPP_
#include <com/sun/star/beans/Property.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSETINFO_HPP_
#include <com/sun/star/beans/XPropertySetInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNVALUE_HPP_
#include <com/sun/star/sdbc/ColumnValue.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XARRAY_HPP_
#include <com/sun/star/sdbc/XArray.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XBLOB_HPP_
#include <com/sun/star/sdbc/XBlob.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCLOB_HPP_
#include <com/sun/star/sdbc/XClob.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XREF_HPP_
#include <com/sun/star/sdbc/XRef.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_TIME_HPP_
#include <com/sun/star/util/Time.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_DATETIME_HPP_
#include <com/sun/star/util/DateTime.hpp>
#endif

#ifndef _UCBHELPER_RESULTSETMETADATA_HXX
#include <ucbhelper/resultsetmetadata.hxx>
#endif

using namespace com::sun::star::beans;
using namespace com::sun::star::io;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::util;
using namespace rtl;
using namespace ucb;

namespace ucb
{

struct ResultSetMetaData_Impl
{
    osl::Mutex  m_aMutex;
    sal_Bool    m_bObtainedTypes;

    ResultSetMetaData_Impl() : m_bObtainedTypes( sal_False ) {}
};

}

//=========================================================================
//=========================================================================
//
// ResultSetMetaData Implementation.
//
//=========================================================================
//=========================================================================

ResultSetMetaData::ResultSetMetaData(
                        const Reference< XMultiServiceFactory >& rxSMgr,
                        const Sequence< Property >& rProps,
                        sal_Bool bReadOnly )
: m_pImpl( new ResultSetMetaData_Impl ),
  m_xSMgr( rxSMgr ),
  m_aProps( rProps ),
  m_bReadOnly( bReadOnly )
{
}

//=========================================================================
// virtual
ResultSetMetaData::~ResultSetMetaData()
{
    delete m_pImpl;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_2( ResultSetMetaData,
                   XTypeProvider,
                   XResultSetMetaData );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_2( ResultSetMetaData,
                      XTypeProvider,
                      XResultSetMetaData );

//=========================================================================
//
// XResultSetMetaData methods.
//
//=========================================================================

// virtual
sal_Int32 SAL_CALL ResultSetMetaData::getColumnCount()
    throw( SQLException, RuntimeException )
{
    return m_aProps.getLength();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSetMetaData::isAutoIncrement( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    /*
        Checks whether column is automatically numbered, which makes it
        read-only.
     */

    return m_bReadOnly;
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSetMetaData::isCaseSensitive( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    // @@@
    return sal_False;
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSetMetaData::isSearchable( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    /*
        Checks whether the value stored in column can be used in a
        WHERE clause.
     */

    // @@@
    return sal_False;
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSetMetaData::isCurrency( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    /*
        Checks whether column is a cash value.
     */

    // @@@
    return sal_False;
}

//=========================================================================
// virtual
sal_Int32 SAL_CALL ResultSetMetaData::isNullable( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    /*
        Checks whether a NULL can be stored in column.
        Possible values: see com/sun/star/sdbc/ColumnValue.idl
     */

    // All columns may contain NULL. Think of result sets containing
    // UCB contents of different types...
    return ColumnValue::NULLABLE;
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSetMetaData::isSigned( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    /*
        Checks whether the value stored in column is a signed number.
     */

    // @@@
    return sal_False;
}

//=========================================================================
// virtual
sal_Int32 SAL_CALL ResultSetMetaData::getColumnDisplaySize( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    /*
        Gets the normal maximum width in characters for column.
     */

    // @@@
    return 16;
}

//=========================================================================
// virtual
OUString SAL_CALL ResultSetMetaData::getColumnLabel( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    /*
        Gets the suggested column title for column, to be used in print-
        outs and displays.
     */

    if ( ( column < 1 ) || ( column > m_aProps.getLength() ) )
        return OUString();

    return m_aProps.getConstArray()[ column - 1 ].Name;
}

//=========================================================================
// virtual
OUString SAL_CALL ResultSetMetaData::getColumnName( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    /*
        Gets the name of column.
     */

    if ( ( column < 1 ) || ( column > m_aProps.getLength() ) )
        return OUString();

    return m_aProps.getConstArray()[ column - 1 ].Name;
}

//=========================================================================
// virtual
OUString SAL_CALL ResultSetMetaData::getSchemaName( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    /*
        Gets the schema name for the table from which column of this
        result set was derived.
        Because this feature is not widely supported, the return value
        for many DBMSs will be an empty string.
     */

    return OUString();
}

//=========================================================================
// virtual
sal_Int32 SAL_CALL ResultSetMetaData::getPrecision( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    /*
        For number types, getprecision gets the number of decimal digits
        in column.
        For character types, it gets the maximum length in characters for
        column.
        For binary types, it gets the maximum length in bytes for column.
     */

    // @@@
    return -1;
}

//=========================================================================
// virtual
sal_Int32 SAL_CALL ResultSetMetaData::getScale( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    /*
        Gets the number of digits to the right of the decimal point for
        values in column.
     */

    // @@@
    return 0;
}

//=========================================================================
// virtual
OUString SAL_CALL ResultSetMetaData::getTableName( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    /*
        Gets the name of the table from which column of this result set
        was derived or "" if there is none (for example, for a join).
        Because this feature is not widely supported, the return value
        for many DBMSs will be an empty string.
     */

    return OUString();
}

//=========================================================================
// virtual
OUString SAL_CALL ResultSetMetaData::getCatalogName( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    /*
        Gets the catalog name for the table from which column of this
        result set was derived.
        Because this feature is not widely supported, the return value
        for many DBMSs will be an empty string.
     */

    return OUString();
}

//=========================================================================
// virtual
sal_Int32 SAL_CALL ResultSetMetaData::getColumnType( sal_Int32 column )
    throw( SQLException, RuntimeException )
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
            == getCppuVoidType() )
    {
        // No type given. Try UCB's Properties Manager...

        osl::Guard< osl::Mutex > aGuard( m_pImpl->m_aMutex );

        if ( !m_pImpl->m_bObtainedTypes )
        {
            try
            {
                Reference< XPropertySetInfo > xInfo(
                            m_xSMgr->createInstance(
                                OUString::createFromAscii(
                                    "com.sun.star.ucb.PropertiesManager" ) ),
                            UNO_QUERY );
                if ( xInfo.is() )
                {
#if 0
    // Convenient...

                    sal_Int32 nCount = m_pImpl->m_aProps.getLength();
                    Property* pProps = m_pImpl->m_aProps.getArray();
                    for ( sal_Int32 n = 0; n < nCount; ++n )
                    {
                        Property& rProp = pProps[ n ];

                        try
                        {
                            Property aProp
                                = xInfo->getPropertyByName( rProp.Name );
                            rProp.Type = aProp.Type;
                        }
                        catch ( UnknownPropertyException& )
                        {
                            // getPropertyByName
                        }
                    }
#else
    // Less (remote) calls...

                    Sequence< Property > aProps = xInfo->getProperties();
                    const Property* pProps1 = aProps.getConstArray();
                    sal_Int32 nCount1 = aProps.getLength();

                    sal_Int32 nCount = m_aProps.getLength();
                    Property* pProps = m_aProps.getArray();
                    for ( sal_Int32 n = 0; n < nCount; ++n )
                    {
                        Property& rProp = pProps[ n ];

                        for ( sal_Int32 m = 0; m < nCount1; ++m )
                        {
                            const Property& rProp1 = pProps1[ m ];
                            if ( rProp.Name == rProp1.Name )
                            {
                                // Found...
                                rProp.Type = rProp1.Type;
                                break;
                            }
                        }
                    }
#endif
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

            m_pImpl->m_bObtainedTypes = sal_True;
        }
    }

    const Type& rType = m_aProps.getConstArray()[ column - 1 ].Type;
    sal_Int32 nType = DataType::OTHER;

    if ( rType == getCppuType( static_cast< const rtl::OUString * >( 0 ) ) )
        nType = DataType::VARCHAR;  // XRow::getString
    else if ( rType == getCppuBooleanType() )
        nType = DataType::BIT;      // XRow::getBoolean
    else if ( rType == getCppuType( static_cast< const sal_Int32 * >( 0 ) ) )
        nType = DataType::INTEGER;  // XRow::getInt
    else if ( rType == getCppuType( static_cast< const sal_Int64 * >( 0 ) ) )
        nType = DataType::BIGINT;   // XRow::getLong
    else if ( rType == getCppuType( static_cast< const sal_Int16 * >( 0 ) ) )
        nType = DataType::SMALLINT; // XRow::getShort
    else if ( rType == getCppuType( static_cast< const sal_Int8 * >( 0 ) ) )
        nType = DataType::TINYINT;  // XRow::getByte
    else if ( rType == getCppuType( static_cast< const float * >( 0 ) ) )
        nType = DataType::REAL;     // XRow::getFloat
    else if ( rType == getCppuType( static_cast< const double * >( 0 ) ) )
        nType = DataType::DOUBLE;   // XRow::getDouble
    else if ( rType == getCppuType( static_cast< const Sequence< sal_Int8 > * >( 0 ) ) )
        nType = DataType::VARBINARY;// XRow::getBytes
    else if ( rType == getCppuType( static_cast< const Date * >( 0 ) ) )
        nType = DataType::DATE;     // XRow::getDate
    else if ( rType == getCppuType( static_cast< const Time * >( 0 ) ) )
        nType = DataType::TIME;     // XRow::getTime
    else if ( rType == getCppuType( static_cast< const DateTime * >( 0 ) ) )
        nType = DataType::TIMESTAMP;// XRow::getTimestamp
    else if ( rType == getCppuType( static_cast< Reference< XInputStream > * >( 0 ) ) )
        nType = DataType::LONGVARBINARY;    // XRow::getBinaryStream
//      nType = DataType::LONGVARCHAR;      // XRow::getCharacterStream
    else if ( rType == getCppuType( static_cast< Reference< XClob > * >( 0 ) ) )
        nType = DataType::CLOB; // XRow::getClob
    else if ( rType == getCppuType( static_cast< Reference< XBlob > * >( 0 ) ) )
        nType = DataType::BLOB; // XRow::getBlob
    else if ( rType == getCppuType( static_cast< Reference< XArray > * >( 0 ) ) )
        nType = DataType::ARRAY;// XRow::getArray
    else if ( rType == getCppuType( static_cast< Reference< XRef > * >( 0 ) ) )
        nType = DataType::REF;// XRow::getRef
    else
        nType = DataType::OBJECT;// XRow::getObject

    return nType;
}

//=========================================================================
// virtual
OUString SAL_CALL ResultSetMetaData::getColumnTypeName( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    /*
        Gets the type name used by this particular data source for the
        values stored in column. If the type code for the type of value
        stored in column is STRUCT, DISTINCT or JAVA_OBJECT, this method
        returns a fully-qualified SQL type name.
     */

    if ( ( column < 1 ) || ( column > m_aProps.getLength() ) )
        return OUString();

    // @@@ According to DG, the type name may be empty.
    return OUString();
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSetMetaData::isReadOnly( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    // ContentResultSet's are completely read-only.
    return m_bReadOnly;
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSetMetaData::isWritable( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    // ContentResultSet's are completely read-only.
    return !m_bReadOnly;
}

//=========================================================================
// virtual
sal_Bool SAL_CALL ResultSetMetaData::isDefinitelyWritable( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    // ContentResultSet's are completely read-only.
    return !m_bReadOnly;
}

//=========================================================================
// virtual
OUString SAL_CALL ResultSetMetaData::getColumnServiceName( sal_Int32 column )
    throw( SQLException, RuntimeException )
{
    /*
          Returns the fully-qualified name of the service whose instances
         are manufactured if XResultSet::getObject is called to retrieve
        a value from the column.
     */

    return OUString();
}

