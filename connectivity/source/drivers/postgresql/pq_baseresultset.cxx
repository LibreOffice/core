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
using com::sun::star::beans::XPropertySet;
using com::sun::star::beans::XMultiPropertySet;
using com::sun::star::beans::XFastPropertySet;

using com::sun::star::uno::Any;
using com::sun::star::uno::makeAny;
using com::sun::star::uno::Type;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Exception;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;

using com::sun::star::lang::IllegalArgumentException;

using com::sun::star::sdbc::XWarningsSupplier;
using com::sun::star::sdbc::XCloseable;
using com::sun::star::sdbc::XStatement;
using com::sun::star::sdbc::XResultSet;
using com::sun::star::sdbc::XConnection;
using com::sun::star::sdbc::SQLException;
using com::sun::star::sdbc::XRow;
using com::sun::star::sdbc::XColumnLocate;
using com::sun::star::sdbc::XResultSetMetaData;
using com::sun::star::sdbc::XResultSetMetaDataSupplier;


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
                        OUString("CursorName"), 0,
                        ::cppu::UnoType<OUString>::get() , 0 ),
                    Property(
                        OUString("EscapeProcessing"), 1,
                        cppu::UnoType<bool>::get() , 0 ),
                    Property(
                        OUString("FetchDirection"), 2,
                        ::cppu::UnoType<sal_Int32>::get() , 0 ),
                    Property(
                        OUString("FetchSize"), 3,
                        ::cppu::UnoType<sal_Int32>::get() , 0 ),
                    Property(
                        OUString("IsBookmarkable"), 4,
                        cppu::UnoType<bool>::get() , 0 ),
                    Property(
                        OUString("ResultSetConcurrency"), 5,
                        ::cppu::UnoType<sal_Int32>::get() , 0 ),
                    Property(
                        OUString("ResultSetType"), 6,
                        ::cppu::UnoType<sal_Int32>::get() , 0 )
                };
            OSL_ASSERT( sizeof(aTable) / sizeof(Property) == BASERESULTSET_SIZE );
            static ::cppu::OPropertyArrayHelper arrayHelper( aTable, BASERESULTSET_SIZE, sal_True );
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
    const Reference< com::sun::star::script::XTypeConverter > & tc )
    : OComponentHelper( refMutex->mutex )
    , OPropertySetHelper( OComponentHelper::rBHelper )
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

Any BaseResultSet::queryInterface( const Type & reqType ) throw (RuntimeException, std::exception)
{
    Any ret;

    ret = OComponentHelper::queryInterface( reqType );
    if( ! ret.hasValue() )
        ret = ::cppu::queryInterface( reqType,
                                    static_cast< XResultSet * > ( this  ),
                                    static_cast< XResultSetMetaDataSupplier * > ( this ),
                                    static_cast< XRow * > ( this ),
                                    static_cast< XColumnLocate * > ( this ),
                                    static_cast< XCloseable * > ( this ),
                                    static_cast< XPropertySet * > ( this ),
                                    static_cast< XMultiPropertySet * > ( this ),
                                    static_cast< XFastPropertySet * > ( this ) );
    return ret;
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

Sequence<Type > BaseResultSet::getTypes() throw( RuntimeException, std::exception )
{
    static cppu::OTypeCollection *pCollection;
    if( ! pCollection )
    {
        MutexGuard guard( osl::Mutex::getGlobalMutex() );
        if( !pCollection )
        {
            static cppu::OTypeCollection collection(
                cppu::UnoType<XResultSet>::get(),
                cppu::UnoType<XResultSetMetaDataSupplier>::get(),
                cppu::UnoType<XRow>::get(),
                cppu::UnoType<XColumnLocate>::get(),
                cppu::UnoType<XCloseable>::get(),
                cppu::UnoType<XPropertySet>::get(),
                cppu::UnoType<XFastPropertySet>::get(),
                cppu::UnoType<XMultiPropertySet>::get(),
                OComponentHelper::getTypes());
            pCollection = &collection;
        }
    }
    return pCollection->getTypes();
}

Sequence< sal_Int8> BaseResultSet::getImplementationId() throw( RuntimeException, std::exception )
{
    return css::uno::Sequence<sal_Int8>();
}

// Reference< XResultSetMetaData > BaseResultSet::getMetaData(  ) throw (SQLException, RuntimeException)
// {
//     ResultSetGuard guard(*this);
//     checkClosed();
//     return new ResultSetMetaData( m_refMutex, this, &m_result );
// }

sal_Bool BaseResultSet::next(  ) throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    m_row ++;
    return m_row < m_rowCount;
}

sal_Bool BaseResultSet::isBeforeFirst(  ) throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return m_row == -1;
}

sal_Bool BaseResultSet::isAfterLast(  ) throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return m_row >= m_rowCount;
}

sal_Bool BaseResultSet::isFirst(  ) throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return m_row == 0 && m_rowCount;
}

sal_Bool BaseResultSet::isLast(  ) throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return m_row >= 0 && m_row + 1 == m_rowCount;
}

void BaseResultSet::beforeFirst(  ) throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    m_row = -1;
}

void BaseResultSet::afterLast(  ) throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    m_row = m_rowCount;
}

sal_Bool BaseResultSet::first(  ) throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    bool bRet = ( m_rowCount > 0 );
    if( bRet )
        m_row = 0;
    return bRet;
}

sal_Bool BaseResultSet::last(  ) throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    bool bRet = ( m_rowCount > 0 );
    if( bRet )
        m_row = m_rowCount -1;
    return bRet;
}

sal_Int32 BaseResultSet::getRow(  ) throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return m_row +1;
}

sal_Bool BaseResultSet::absolute( sal_Int32 row ) throw (SQLException, RuntimeException, std::exception)
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
    return sal_True;
}

sal_Bool BaseResultSet::relative( sal_Int32 rows ) throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    m_row += rows;

    if( m_row > m_rowCount )
        m_row = m_rowCount;
    else if ( m_row < -1 )
        m_row = -1;
    return sal_True;
}

sal_Bool BaseResultSet::previous(  ) throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    bool bRet = ( m_row != -1 );
    if( bRet )
        m_row --;
    return bRet;
}

void BaseResultSet::refreshRow(  ) throw (SQLException, RuntimeException, std::exception)
{
    // TODO: not supported for now
}

sal_Bool BaseResultSet::rowUpdated(  ) throw (SQLException, RuntimeException, std::exception)
{
    return sal_False;
}

sal_Bool BaseResultSet::rowInserted(  ) throw (SQLException, RuntimeException, std::exception)
{
    return sal_False;
}

sal_Bool BaseResultSet::rowDeleted(  ) throw (SQLException, RuntimeException, std::exception)
{
    return sal_False;
}

Reference< XInterface > BaseResultSet::getStatement() throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    return m_owner;
}


//----------------- XRow interface ----------------------------------------------------

sal_Bool BaseResultSet::wasNull(  ) throw (SQLException, RuntimeException, std::exception)
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
    catch( com::sun::star::lang::IllegalArgumentException & )
    {}
    catch( com::sun::star::script::CannotConvertException & )
    {}
    return aRet;
}

sal_Bool BaseResultSet::getBoolean( sal_Int32 columnIndex ) throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( columnIndex );
    checkRowIndex( true /* must be on row */ );

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

            return sal_True;
        }
    }
    return sal_False;
}

sal_Int8 BaseResultSet::getByte( sal_Int32 columnIndex )
        throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( columnIndex );
    checkRowIndex( true /* must be on row */ );
    sal_Int8 b = 0;
    convertTo( getValue( columnIndex ), cppu::UnoType<decltype(b)>::get()) >>= b;
    return b;
}

sal_Int16 BaseResultSet::getShort( sal_Int32 columnIndex )
        throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( columnIndex );
    checkRowIndex( true /* must be on row */ );
    sal_Int16 i = 0;
    convertTo( getValue( columnIndex ), cppu::UnoType<decltype(i)>::get()) >>= i;
    return i;
}

OUString BaseResultSet::getString( sal_Int32 columnIndex ) throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard(m_refMutex->mutex);
    checkClosed();
    checkColumnIndex( columnIndex );
    checkRowIndex( true /* must be on row */ );
    OUString ret;
    convertTo( getValue(  columnIndex ), cppu::UnoType<decltype(ret)>::get() ) >>= ret;
//     printf( "BaseResultSet::getString() %s\n" , OUStringToOString( ret, RTL_TEXTENCODING_ASCII_US ).getStr() );
    return ret;
}

sal_Int32 BaseResultSet::getInt( sal_Int32 columnIndex )
        throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( columnIndex );
    checkRowIndex( true /* must be on row */ );
    sal_Int32 i = 0;
    convertTo( getValue( columnIndex ), cppu::UnoType<decltype(i)>::get()) >>= i;
    return i;
}

sal_Int64 BaseResultSet::getLong( sal_Int32 columnIndex )
        throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( columnIndex );
    checkRowIndex( true /* must be on row */ );
    sal_Int64 i = 0;
    convertTo( getValue( columnIndex ), cppu::UnoType<decltype(i)>::get()) >>= i;
    return i;
}

float BaseResultSet::getFloat( sal_Int32 columnIndex )
        throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( columnIndex );
    checkRowIndex( true /* must be on row */ );
    float f = 0.;
    convertTo( getValue( columnIndex ), cppu::UnoType<decltype(f)>::get()) >>= f;
    return f;
}

double BaseResultSet::getDouble( sal_Int32 columnIndex )
        throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( columnIndex );
    double d = 0.;
    convertTo( getValue( columnIndex ), cppu::UnoType<decltype(d)>::get()) >>= d;
    return d;
}

Sequence< sal_Int8 > BaseResultSet::getBytes( sal_Int32 columnIndex )
        throw (SQLException, RuntimeException, std::exception)
{
    MutexGuard guard( m_refMutex->mutex );
    checkClosed();
    checkColumnIndex( columnIndex );
    checkRowIndex( true /* must be on row */ );

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


::com::sun::star::util::Date BaseResultSet::getDate( sal_Int32 columnIndex )
        throw (SQLException, RuntimeException, std::exception)
{
    return DBTypeConversion::toDate( getString( columnIndex ) );
}

::com::sun::star::util::Time BaseResultSet::getTime( sal_Int32 columnIndex )
        throw (SQLException, RuntimeException, std::exception)
{
    return DBTypeConversion::toTime( getString( columnIndex ) );
}

::com::sun::star::util::DateTime BaseResultSet::getTimestamp( sal_Int32 columnIndex )
        throw (SQLException, RuntimeException, std::exception)
{
    return DBTypeConversion::toDateTime( getString( columnIndex ) );
}

  // LEM TODO: these look like they are missing an actual implementation
Reference< ::com::sun::star::io::XInputStream > BaseResultSet::getBinaryStream( sal_Int32 /* columnIndex */ )
        throw (SQLException, RuntimeException, std::exception)
{
    return nullptr;
}

Reference< ::com::sun::star::io::XInputStream > BaseResultSet::getCharacterStream( sal_Int32 /* columnIndex */ )
        throw (SQLException, RuntimeException, std::exception)
{
    return nullptr;
}

Any BaseResultSet::getObject(
        sal_Int32 /* columnIndex */,
        const Reference< ::com::sun::star::container::XNameAccess >& /* typeMap */ )
        throw (SQLException, RuntimeException, std::exception)
{
    return Any();
}

Reference< ::com::sun::star::sdbc::XRef > BaseResultSet::getRef( sal_Int32 /* columnIndex */ )
        throw (SQLException, RuntimeException, std::exception)
{
    return Reference< com::sun::star::sdbc::XRef > ();
}

Reference< ::com::sun::star::sdbc::XBlob > BaseResultSet::getBlob( sal_Int32 /* columnIndex */ )
        throw (SQLException, RuntimeException, std::exception)
{
    return Reference< com::sun::star::sdbc::XBlob > ();
}

Reference< ::com::sun::star::sdbc::XClob > BaseResultSet::getClob( sal_Int32 /* columnIndex */ )
        throw (SQLException, RuntimeException, std::exception)
{
    return Reference< com::sun::star::sdbc::XClob > ();
}

Reference< ::com::sun::star::sdbc::XArray > BaseResultSet::getArray( sal_Int32 columnIndex )
        throw (SQLException, RuntimeException, std::exception)
{
    return new Array( m_refMutex, parseArray( getString( columnIndex ) ), *this, m_tc );
}

::cppu::IPropertyArrayHelper & BaseResultSet::getInfoHelper()
{
    return getResultSetPropertyArrayHelper();
}

sal_Bool BaseResultSet::convertFastPropertyValue(
        Any & /* rConvertedValue */, Any & /* rOldValue */, sal_Int32 nHandle, const Any& rValue )
        throw (IllegalArgumentException)
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
        OUStringBuffer buf(128);
        buf.append( "pq_resultset: Invalid property handle (" );
        buf.append( nHandle );
        buf.append( ")" );
        throw IllegalArgumentException( buf.makeStringAndClear(), *this, 2 );
    }
    }
    return bRet;
}


void BaseResultSet::setFastPropertyValue_NoBroadcast(
    sal_Int32 nHandle,const Any& rValue ) throw (Exception, std::exception)
{
    m_props[nHandle] = rValue;
}

void BaseResultSet::getFastPropertyValue( Any& rValue, sal_Int32 nHandle ) const
{
    rValue = m_props[nHandle];
}

Reference < XPropertySetInfo >  BaseResultSet::getPropertySetInfo()
        throw(RuntimeException, std::exception)
{
    return OPropertySetHelper::createPropertySetInfo( getResultSetPropertyArrayHelper() );
}

void BaseResultSet::disposing()
{
    close();
}

void BaseResultSet::checkColumnIndex(sal_Int32 index ) throw ( SQLException, RuntimeException )
{
    if( index < 1 || index > m_fieldCount )
    {
        OUStringBuffer buf(128);
        buf.append( "pq_resultset: index out of range (" );
        buf.append( index );
        buf.append( ", allowed range is 1 to " );
        buf.append( m_fieldCount );
        buf.append( ")" );
        throw SQLException( buf.makeStringAndClear(), *this, OUString(), 1, Any() );
    }

}

void BaseResultSet::checkRowIndex( bool mustBeOnValidRow )
{
    OUStringBuffer buf( 128 );
    buf.append( "pq_baseresultset: row index out of range, allowed is " );
    if( mustBeOnValidRow )
    {
        if( m_row < 0 || m_row >= m_rowCount )
        {
            buf.append( "0 to " );
            buf.append( ((sal_Int32)(m_rowCount -1)) );
            buf.append( ", got " );
            buf.append( m_row );
            throw SQLException( buf.makeStringAndClear(), *this, OUString(),1, Any() );
        }
    }
    else
    {
        if( m_row < -1 || m_row > m_rowCount )
        {
            buf.append( "-1 to " );
            buf.append( m_rowCount );
            buf.append( ", got " );
            buf.append( m_row );
            throw SQLException( buf.makeStringAndClear(), *this, OUString(),1, Any() );
        }
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
