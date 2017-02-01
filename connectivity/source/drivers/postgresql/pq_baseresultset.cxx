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

#include <osl/thread.h>

#include <rtl/ustrbuf.hxx>

#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/queryinterface.hxx>

#include <comphelper/sequence.hxx>

#include "pq_tools.hxx"
#include "pq_array.hxx"
#include "pq_statement.hxx"
#include "pq_baseresultset.hxx"
#include "pq_resultsetmetadata.hxx"

#include <com/sun/star/lang/DisposedException.hpp>

#include <connectivity/dbconversion.hxx>

using osl::Mutex;
using osl::MutexGuard;


using com::sun::star::beans::XPropertySetInfo;

using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::Type;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Exception;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;

using com::sun::star::lang::IllegalArgumentException;

using com::sun::star::sdbc::SQLException;
using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XResultSetMetaData;


using com::sun::star::beans::Property;

using namespace dbtools;

namespace pq_sdbc_driver
{
static ::cppu::IPropertyArrayHelper & getResultSetPropertyArrayHelper()
{
    static ::cppu::IPropertyArrayHelper *pArrayHelper;
    if( ! pArrayHelper )
    {
        MutexGuard guard( Mutex::getGlobalMutex() );
        if( ! pArrayHelper )
        {
            static Property aTable[] =
                {
                    // LEM TODO: this needs to be kept in sync with other, e.g. pq_statics.css:508
                    // Should really share!
                    // At least use for the handles the #define'd values in .hxx file...
                    Property(
                        "CursorName", 0,
                        ::cppu::UnoType<OUString>::get() , 0 ),
                    Property(
                        "EscapeProcessing", 1,
                        cppu::UnoType<bool>::get() , 0 ),
                    Property(
                        "FetchDirection", 2,
                        ::cppu::UnoType<sal_Int32>::get() , 0 ),
                    Property(
                        "FetchSize", 3,
                        ::cppu::UnoType<sal_Int32>::get() , 0 ),
                    Property(
                        "IsBookmarkable", 4,
                        cppu::UnoType<bool>::get() , 0 ),
                    Property(
                        "ResultSetConcurrency", 5,
                        ::cppu::UnoType<sal_Int32>::get() , 0 ),
                    Property(
                        "ResultSetType", 6,
                        ::cppu::UnoType<sal_Int32>::get() , 0 )
                };
            static_assert( SAL_N_ELEMENTS(aTable) == BASERESULTSET_SIZE, "wrong number of elements" );
            static ::cppu::OPropertyArrayHelper arrayHelper( aTable, BASERESULTSET_SIZE, true );
            pArrayHelper = &arrayHelper;
        }
    }
    return *pArrayHelper;
}

BaseResultSet::BaseResultSet(
    const ::rtl::Reference< RefCountedMutex > & refMutex,
    const Reference< XInterface > & owner,
    sal_Int32 rowCount,
    sal_Int32 colCount,
    const Reference< css::script::XTypeConverter > & tc )
    : BaseResultSet_BASE( refMutex->mutex )
    , OPropertySetHelper( BaseResultSet_BASE::rBHelper )
    , m_owner( owner )
    , m_tc( tc )
    , m_refMutex( refMutex )
    , m_row( -1 )
    , m_rowCount( rowCount )
    , m_fieldCount( colCount )
    , m_wasNull(false)
{
    POSTGRE_TRACE( "ctor BaseResultSet" );
}

// LEM TODO: refMutex->mutex should live longer than OComponentHelper,
// but calling OComponentHelper::dispose explicitly here calls
// BaseResultSet::~BaseResultSet in an infinite loop :(
BaseResultSet::~BaseResultSet()
{
    POSTGRE_TRACE( "dtor BaseResultSet" );
}

Any BaseResultSet::queryInterface( const Type & rType )
{
    Any aRet = BaseResultSet_BASE::queryInterface(rType);
    return aRet.hasValue() ? aRet : OPropertySetHelper::queryInterface(rType);
}

// void BaseResultSet::close(  ) throw (SQLException, RuntimeException)
// {
//     Reference< XInterface > owner;
//     {
//         ResultSetGuard guard(*this);
//         if( m_result )
//         {
//             PQclear(m_result );
//             m_result = 0;
//             m_row = -1;
//         }
//         owner = m_owner;
//         m_owner.clear();
//     }
// }

Sequence<Type > BaseResultSet::getTypes()
{
    static Sequence< Type > *pCollection;
    if( ! pCollection )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( !pCollection )
        {
            static Sequence< Type > collection(
                ::comphelper::concatSequences(
                    OPropertySetHelper::getTypes(),
                    BaseResultSet_BASE::getTypes()));
            pCollection = &collection;
        }
    }
    return *pCollection;
}

Sequence< sal_Int8> BaseResultSet::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// Reference< XResultSetMetaData > BaseResultSet::getMetaData(  ) throw (SQLException, RuntimeException)
// {
//     ResultSetGuard guard(*this);
//     checkClosed();
//     return new ResultSetMetaData( m_refMutex, this, &m_result );
// }

sal_Bool BaseResultSet::next(  )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    m_row ++;
    return m_row < m_rowCount;
}

sal_Bool BaseResultSet::isBeforeFirst(  )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return m_row == -1;
}

sal_Bool BaseResultSet::isAfterLast(  )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return m_row >= m_rowCount;
}

sal_Bool BaseResultSet::isFirst(  )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return m_row == 0 && m_rowCount;
}

sal_Bool BaseResultSet::isLast(  )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return m_row >= 0 && m_row + 1 == m_rowCount;
}

void BaseResultSet::beforeFirst(  )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    m_row = -1;
}

void BaseResultSet::afterLast(  )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    m_row = m_rowCount;
}

sal_Bool BaseResultSet::first(  )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    bool bRet = ( m_rowCount > 0 );
    if( bRet )
        m_row = 0;
    return bRet;
}

sal_Bool BaseResultSet::last(  )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    bool bRet = ( m_rowCount > 0 );
    if( bRet )
        m_row = m_rowCount -1;
    return bRet;
}

sal_Int32 BaseResultSet::getRow(  )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return m_row +1;
}

sal_Bool BaseResultSet::absolute( sal_Int32 row )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    if( row > 0 )
    {
        m_row = row -1;
        if( m_row > m_rowCount )
            m_row = m_rowCount;
    }
    else
    {
        m_row = m_rowCount + row;
        if( m_row < -1 )
            m_row = -1;
    }
    return true;
}

sal_Bool BaseResultSet::relative( sal_Int32 rows )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    m_row += rows;

    if( m_row > m_rowCount )
        m_row = m_rowCount;
    else if ( m_row < -1 )
        m_row = -1;
    return true;
}

sal_Bool BaseResultSet::previous(  )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    bool bRet = ( m_row != -1 );
    if( bRet )
        m_row --;
    return bRet;
}

void BaseResultSet::refreshRow(  )
{
    // TODO: not supported for now
}

sal_Bool BaseResultSet::rowUpdated(  )
{
    return false;
}

sal_Bool BaseResultSet::rowInserted(  )
{
    return false;
}

sal_Bool BaseResultSet::rowDeleted(  )
{
    return false;
}

Reference< XInterface > BaseResultSet::getStatement()
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return m_owner;
}


//----------------- XRow interface ----------------------------------------------------

sal_Bool BaseResultSet::wasNull(  )
{
    return m_wasNull;
}

Any BaseResultSet::convertTo( const Any & val , const Type & type )
{
    Any aRet;
    try
    {
        aRet = m_tc->convertTo( val , type );
    }
    catch( css::lang::IllegalArgumentException & )
    {}
    catch( css::script::CannotConvertException & )
    {}
    return aRet;
}

sal_Bool BaseResultSet::getBoolean( sal_Int32 columnIndex )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( columnIndex );
    checkRowIndex();

    OUString str = getString( columnIndex );

    if( str.getLength() > 0 )
    {
        switch(str[0])
        {
        case '1':
        case 't':
        case 'T':
        case 'y':
        case 'Y':

            return true;
        }
    }
    return false;
}

sal_Int8 BaseResultSet::getByte( sal_Int32 columnIndex )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( columnIndex );
    checkRowIndex();
    sal_Int8 b = 0;
    convertTo( getValue( columnIndex ), cppu::UnoType<decltype(b)>::get()) >>= b;
    return b;
}

sal_Int16 BaseResultSet::getShort( sal_Int32 columnIndex )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( columnIndex );
    checkRowIndex();
    sal_Int16 i = 0;
    convertTo( getValue( columnIndex ), cppu::UnoType<decltype(i)>::get()) >>= i;
    return i;
}

OUString BaseResultSet::getString( sal_Int32 columnIndex )
{
    MutexGuard guard(m_refMutex->mutex);
    checkClosed();
    checkColumnIndex( columnIndex );
    checkRowIndex();
    OUString ret;
    convertTo( getValue(  columnIndex ), cppu::UnoType<decltype(ret)>::get() ) >>= ret;
//     printf( "BaseResultSet::getString() %s\n" , OUStringToOString( ret, RTL_TEXTENCODING_ASCII_US ).getStr() );
    return ret;
}

sal_Int32 BaseResultSet::getInt( sal_Int32 columnIndex )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( columnIndex );
    checkRowIndex();
    sal_Int32 i = 0;
    convertTo( getValue( columnIndex ), cppu::UnoType<decltype(i)>::get()) >>= i;
    return i;
}

sal_Int64 BaseResultSet::getLong( sal_Int32 columnIndex )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( columnIndex );
    checkRowIndex();
    sal_Int64 i = 0;
    convertTo( getValue( columnIndex ), cppu::UnoType<decltype(i)>::get()) >>= i;
    return i;
}

float BaseResultSet::getFloat( sal_Int32 columnIndex )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( columnIndex );
    checkRowIndex();
    float f = 0.;
    convertTo( getValue( columnIndex ), cppu::UnoType<decltype(f)>::get()) >>= f;
    return f;
}

double BaseResultSet::getDouble( sal_Int32 columnIndex )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( columnIndex );
    double d = 0.;
    convertTo( getValue( columnIndex ), cppu::UnoType<decltype(d)>::get()) >>= d;
    return d;
}

Sequence< sal_Int8 > BaseResultSet::getBytes( sal_Int32 columnIndex )
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( columnIndex );
    checkRowIndex();

    Sequence< sal_Int8 > ret;
    OUString ustr;
    if( ! (getValue( columnIndex ) >>= ustr) )
        m_wasNull = true;
    else
    {
        // if this is a binary, it must contain escaped data !
        OString val = OUStringToOString( ustr, RTL_TEXTENCODING_ASCII_US );

        size_t length;
        char * res = reinterpret_cast<char*>(PQunescapeBytea( reinterpret_cast<unsigned char const *>(val.getStr()), &length));
        ret = Sequence< sal_Int8 > ( reinterpret_cast<sal_Int8*>(res), length );
        if( res )
            free( res );
    }
    return ret;
}


css::util::Date BaseResultSet::getDate( sal_Int32 columnIndex )
{
    return DBTypeConversion::toDate( getString( columnIndex ) );
}

css::util::Time BaseResultSet::getTime( sal_Int32 columnIndex )
{
    return DBTypeConversion::toTime( getString( columnIndex ) );
}

css::util::DateTime BaseResultSet::getTimestamp( sal_Int32 columnIndex )
{
    return DBTypeConversion::toDateTime( getString( columnIndex ) );
}

  // LEM TODO: these look like they are missing an actual implementation
Reference< css::io::XInputStream > BaseResultSet::getBinaryStream( sal_Int32 /* columnIndex */ )
{
    return nullptr;
}

Reference< css::io::XInputStream > BaseResultSet::getCharacterStream( sal_Int32 /* columnIndex */ )
{
    return nullptr;
}

Any BaseResultSet::getObject(
        sal_Int32 /* columnIndex */,
        const Reference< css::container::XNameAccess >& /* typeMap */ )
{
    return Any();
}

Reference< css::sdbc::XRef > BaseResultSet::getRef( sal_Int32 /* columnIndex */ )
{
    return Reference< css::sdbc::XRef > ();
}

Reference< css::sdbc::XBlob > BaseResultSet::getBlob( sal_Int32 /* columnIndex */ )
{
    return Reference< css::sdbc::XBlob > ();
}

Reference< css::sdbc::XClob > BaseResultSet::getClob( sal_Int32 /* columnIndex */ )
{
    return Reference< css::sdbc::XClob > ();
}

Reference< css::sdbc::XArray > BaseResultSet::getArray( sal_Int32 columnIndex )
{
    return new Array( m_refMutex, parseArray( getString( columnIndex ) ), *this, m_tc );
}

::cppu::IPropertyArrayHelper & BaseResultSet::getInfoHelper()
{
    return getResultSetPropertyArrayHelper();
}

sal_Bool BaseResultSet::convertFastPropertyValue(
        Any & /* rConvertedValue */, Any & /* rOldValue */, sal_Int32 nHandle, const Any& rValue )
{
    bool bRet;
    switch( nHandle )
    {
    case BASERESULTSET_CURSOR_NAME:
    {
        OUString val;
        bRet = ( rValue >>= val );
        m_props[nHandle] = makeAny( val );
        break;
    }
    case BASERESULTSET_ESCAPE_PROCESSING:
    case BASERESULTSET_IS_BOOKMARKABLE:
    {
        bool val(false);
        bRet = ( rValue >>= val );
        m_props[nHandle] = makeAny( val );
        break;
    }
    case BASERESULTSET_FETCH_DIRECTION:
    case BASERESULTSET_FETCH_SIZE:
    case BASERESULTSET_RESULT_SET_CONCURRENCY:
    case BASERESULTSET_RESULT_SET_TYPE:
    {
        sal_Int32 val;
        bRet = ( rValue >>= val );
        m_props[nHandle] = makeAny( val );
        break;
    }
    default:
    {
        throw IllegalArgumentException(
            "pq_resultset: Invalid property handle (" + OUString::number( nHandle ) + ")",
            *this, 2 );
    }
    }
    return bRet;
}


void BaseResultSet::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle,const Any& rValue )
{
    m_props[nHandle] = rValue;
}

void BaseResultSet::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    rValue = m_props[nHandle];
}

Reference < XPropertySetInfo >  BaseResultSet::getPropertySetInfo()
{
    return OPropertySetHelper::createPropertySetInfo( getResultSetPropertyArrayHelper() );
}

void BaseResultSet::disposing()
{
    close();
}

void BaseResultSet::checkColumnIndex(sal_Int32 index )
{
    if( index < 1 || index > m_fieldCount )
    {
        throw SQLException(
            "pq_resultset: index out of range ("
            + OUString::number( index )
            + ", allowed range is 1 to " + OUString::number( m_fieldCount )
            + ")",
            *this, OUString(), 1, Any() );
    }

}

void BaseResultSet::checkRowIndex()
{
    if( m_row < 0 || m_row >= m_rowCount )
    {
        throw SQLException(
            "pq_baseresultset: row index out of range, allowed is 0 to " + OUString::number( m_rowCount -1 )
            + ", got " + OUString::number( m_row ),
            *this, OUString(),1, Any() );
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
