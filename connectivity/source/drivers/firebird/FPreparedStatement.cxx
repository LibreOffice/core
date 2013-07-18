/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

#include <stdio.h>
#include <osl/diagnose.h>
#include "FPreparedStatement.hxx"
#include <com/sun/star/sdbc/DataType.hpp>
#include "FResultSetMetaData.hxx"
#include "FResultSet.hxx"
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include "propertyids.hxx"
#include <comphelper/sequence.hxx>

using namespace ::comphelper;
using namespace connectivity::firebird;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

typedef struct vary {
    short vary_length;
    char vary_string[1];
} VARY;

IMPLEMENT_SERVICE_INFO(OPreparedStatement,"com.sun.star.sdbcx.firebird.PreparedStatement","com.sun.star.sdbc.PreparedStatement");


OPreparedStatement::OPreparedStatement( OConnection* _pConnection,
                                        const TTypeInfoVector& _TypeInfo,
                                        const OUString& sql)
    :OStatement_BASE2(_pConnection)
    ,m_aTypeInfo(_TypeInfo)
    ,m_nNumParams(0)
    ,m_sSqlStatement(sql)
    ,m_bPrepared(sal_False)
{
    SAL_INFO("connectivity.firebird", "OPreparedStatement_BASE(). "
             "sql: " << sql);

//     prepareQuery(m_sSqlStatement);
    (void) sql;
}

// -----------------------------------------------------------------------------
OPreparedStatement::~OPreparedStatement()
{
}
// -----------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::acquire() throw()
{
    OStatement_BASE2::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::release() throw()
{
    OStatement_BASE2::release();
}
// -----------------------------------------------------------------------------
Any SAL_CALL OPreparedStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_BASE2::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OPreparedStatement_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OPreparedStatement::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return concatSequences(OPreparedStatement_BASE::getTypes(),OStatement_BASE2::getTypes());
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(getOwnConnection());
    return m_xMetaData;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::close(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "close()");

    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    // Reset last warning message

    try {
        clearWarnings ();
        OStatement_BASE2::close();
    }
    catch (SQLException &) {
        // If we get an error, ignore
    }

    // Remove this Statement object from the Connection object's
    // list
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OPreparedStatement::execute(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


    // same as in statement with the difference that this statement also can contain parameter
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OPreparedStatement::executeUpdate(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    // same as in statement with the difference that this statement also can contain parameter
    return 0;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setString( sal_Int32 parameterIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "setString(). "
             "parameterIndex: " << parameterIndex << " , "
             "x: " << x);

    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    if (NULL == m_INsqlda)
    {
        SAL_WARN("connectivity.firebird", "setString(). "
                 "The query has not input parameters.");
        return;
    }

    OString str = OUStringToOString(x , RTL_TEXTENCODING_UTF8 );
    SAL_INFO("connectivity.firebird", "setString(). "
             "Setting parameter as: " << str);

    XSQLVAR *var = m_INsqlda->sqlvar + (parameterIndex - 1);

    int dtype = (var->sqltype & ~1); // drop flag bit for now
    switch(dtype) {
    case SQL_VARYING:
        var->sqltype = SQL_TEXT;
    case SQL_TEXT:
        var->sqllen = str.getLength();
        var->sqldata = (char *)malloc(sizeof(char)*var->sqllen);
        sprintf(var->sqldata , "%s", str.getStr());
        break;
    default:
        OSL_ASSERT( false );
    }

    sprintf(var->sqldata , "%s", OUStringToOString(x , RTL_TEXTENCODING_UTF8 ).getStr());
}
// -------------------------------------------------------------------------

Reference< XConnection > SAL_CALL OPreparedStatement::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery(  ) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.firebird", "executeQuery(). "
             "Got called with sql: " <<  m_sSqlStatement);

    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

//     ISC_STATUS_ARRAY status; /* status vector */

//     if (isc_dsql_execute(status, &m_pConnection->getTransaction(), &m_statementHandle, 1, m_INsqlda))
//         if (pr_error(status, "execute query"))
//             return NULL;

    isc_stmt_handle aHandle = 0;
    Reference< OResultSet > pResult( new OResultSet( this, aHandle, 0) );
    //initializeResultSet( pResult.get() );
    Reference< XResultSet > xRS = pResult.get();

    SAL_INFO("connectivity.firebird", "executeQuery(). "
             "Query executed.");

    return xRS;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);


}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDate( sal_Int32 parameterIndex, const Date& aData ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) aData;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setTime( sal_Int32 parameterIndex, const Time& aVal ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) aVal;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setTimestamp( sal_Int32 parameterIndex, const DateTime& aVal ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) aVal;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}

// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 aVal ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) aVal;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) sqlType;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setClob( sal_Int32 parameterIndex, const Reference< XClob >& x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBlob( sal_Int32 parameterIndex, const Reference< XBlob >& x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setArray( sal_Int32 parameterIndex, const Reference< XArray >& x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setRef( sal_Int32 parameterIndex, const Reference< XRef >& x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    (void) sqlType;
    (void) scale;
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) sqlType;
    (void) typeName;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    (void) length;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    (void) parameterIndex;
    (void) x;
    (void) length;
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OStatement_BASE::rBHelper.bDisposed);

}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::clearParameters(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::clearBatch(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::addBatch( ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------

Sequence< sal_Int32 > SAL_CALL OPreparedStatement::executeBatch(  ) throw(SQLException, RuntimeException)
{
    return Sequence< sal_Int32 > ();
}
// -------------------------------------------------------------------------
void OPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            break;
        case PROPERTY_ID_USEBOOKMARKS:
            break;
        default:
            OStatement_Base::setFastPropertyValue_NoBroadcast(nHandle,rValue);
    }
}
// -----------------------------------------------------------------------------
void OPreparedStatement::checkParameterIndex(sal_Int32 _parameterIndex)
{
    if( !_parameterIndex || _parameterIndex > m_nNumParams)
        throw SQLException();
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
