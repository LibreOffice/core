/*************************************************************************
 *
 *  $RCSfile: OPreparedStatement.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: jl $ $Date: 2001-03-20 16:54:34 $
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
 *  This library is free software
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

#include <stdio.h>

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CONNECTIVITY_ODBC_OPREPAREDSTATEMENT_HXX_
#include "odbc/OPreparedStatement.hxx"
#endif
#ifndef _CONNECTIVITY_OBOUNPARAM_HXX_
#include "odbc/OBoundParam.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _CONNECTIVITY_OTOOLS_HXX_
#include "odbc/OTools.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_ORESULTSETMETADATA_HXX_
#include "odbc/OResultSetMetaData.hxx"
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE odbc
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

using namespace connectivity;
using namespace connectivity::odbc;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

int OBoundParam::ASCII   = 1;
int OBoundParam::UNICODE = 2;
int OBoundParam::BINARY  = 3;

IMPLEMENT_SERVICE_INFO(OPreparedStatement,"com.sun.star.sdbcx.OPreparedStatement","com.sun.star.sdbc.PreparedStatement");

OPreparedStatement::OPreparedStatement( OConnection* _pConnection,const ::std::vector<OTypeInfo>& _TypeInfo,const ::rtl::OUString& sql)
    :OStatement_BASE2(_pConnection)
    ,m_aTypeInfo(_TypeInfo)
    ,boundParams(NULL)
    ,m_bPrepared(sal_False)
    ,m_sSqlStatement(sql)
{
}
// -------------------------------------------------------------------------

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
    return ::comphelper::concatSequences(OPreparedStatement_BASE::getTypes(),OStatement_BASE2::getTypes());
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_aStatementHandle);
    return m_xMetaData;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::close(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    // Close/clear our result set
    clearMyResultSet ();

    // Reset last warning message

    try {
        clearWarnings ();
        OStatement_BASE2::close();
        FreeParams();
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
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Bool hasResultSet = sal_False;
    SQLWarning  warning;
    sal_Bool needData = sal_False;

    // Reset warnings

    clearWarnings ();

    // Reset the statement handle, warning and saved Resultset

    reset();

    // Call SQLExecute
    if(!isPrepared())
        prepareStatement();

    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    try
    {
        SQLRETURN nReturn = N3SQLExecute(m_aStatementHandle);

        OTools::ThrowException(nReturn,m_aStatementHandle,SQL_HANDLE_STMT,*this);
        needData = nReturn == SQL_NEED_DATA;

        // Now loop while more data is needed (i.e. a data-at-
        // execution parameter was given).  For each parameter
        // that needs data, put the data from the input stream.

        while (needData) {

            // Get the parameter number that requires data

            sal_Int32* paramIndex = 0;
            nReturn = N3SQLParamData(m_aStatementHandle,(SQLPOINTER*)&paramIndex);

            // If the parameter index is -1, there is no
            // more data required

            if (*paramIndex == -1)
                needData = sal_False;
            else
            {
                // Now we have the proper parameter
                // index, get the data from the input
                // stream and do a SQLPutData
                putParamData (*paramIndex);
            }
        }

    }
    catch (SQLWarning& ex)
    {

        // Save pointer to warning and save with ResultSet
        // object once it is created.

        warning = ex;
    }

    // Now loop while more data is needed (i.e. a data-at-
    // execution parameter was given).  For each parameter
    // that needs data, put the data from the input stream.

    while (needData) {

        // Get the parameter number that requires data

        sal_Int32* paramIndex = 0;
        N3SQLParamData (m_aStatementHandle,(SQLPOINTER*)&paramIndex);

        // If the parameter index is -1, there is no more
        // data required

        if (*paramIndex == -1) {
            needData = sal_False;
        }
        else {
            // Now we have the proper parameter index,
            // get the data from the input stream
            // and do a SQLPutData
            putParamData(*paramIndex);
        }
    }

    // Now determine if there is a result set associated with
    // the SQL statement that was executed.  Get the column
    // count, and if it is not zero, there is a result set.

    if (getColumnCount () > 0)
        hasResultSet = sal_True;


    return hasResultSet;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OPreparedStatement::executeUpdate(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    sal_Int32 numRows = -1;

    if(!isPrepared())
        prepareStatement();
    // Execute the statement.  If execute returns sal_False, a
    // row count exists.

    if (!execute())
        numRows = getUpdateCount ();
    else {

        // No update count was produced (a ResultSet was).  Raise
        // an exception

        throw SQLException(::rtl::OUString::createFromAscii("No row count was produced"),
            *this,
            ::rtl::OUString(),0,Any());
    }
    return numRows;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setString( sal_Int32 parameterIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();

    setChar(parameterIndex, DataType::CHAR, 0, x);
}
// -------------------------------------------------------------------------

Reference< XConnection > SAL_CALL OPreparedStatement::getConnection(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    return (Reference< XConnection >)m_pConnection;
}
// -------------------------------------------------------------------------

Reference< XResultSet > SAL_CALL OPreparedStatement::executeQuery(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    Reference< XResultSet > rs = NULL;

    if(!isPrepared())
        prepareStatement();

    if (execute())
        rs = getResultSet(sal_False);

    else {

        // No ResultSet was produced.  Raise an exception

        throw SQLException(::rtl::OUString::createFromAscii("No ResultSet was produced"),
            *this,
            ::rtl::OUString(),0,Any());
    }
    return rs;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Int32 value = 0;

    // If the parameter is sal_True, set the value to 1
    if (x) {
        value = 1;
    }

    // Set the parameter as if it were an integer
    setInt (parameterIndex, value);
}
// -------------------------------------------------------------------------
#define PREP_BIND_PARAM(_ty,_jt) \
    bindParameter(          m_aStatementHandle,                     \
                                parameterIndex,                         \
                                bindBuf,getLengthBuf(parameterIndex),   \
                                _jt,                                    \
                                sal_False,sal_False,&x,(Reference <XInterface>)*this,getOwnConnection()->getTextEncoding())



void SAL_CALL OPreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();
    // Allocate a buffer to be used in binding.  This will be
        // a 'permanent' buffer that the bridge will fill in with
        // the bound data in native format.
    if( !parameterIndex || parameterIndex > numParams)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    sal_Int8* bindBuf = allocBindBuf(parameterIndex, 4);
    PREP_BIND_PARAM(sal_Int8,SQL_TINYINT);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDate( sal_Int32 parameterIndex, const Date& aData ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();
    // Allocate a buffer to be used in binding.  This will be
    // a 'permanent' buffer that the bridge will fill in with
    // the bound data in native format.

    if( !parameterIndex || parameterIndex > numParams)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    sal_Int8* bindBuf = allocBindBuf (parameterIndex, 32);

    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    DATE_STRUCT x = OTools::DateToOdbcDate(aData);
    PREP_BIND_PARAM(DATE_STRUCT,DataType::DATE);
}
// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setTime( sal_Int32 parameterIndex, const Time& aVal ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();
    // Allocate a buffer to be used in binding.  This will be
    // a 'permanent' buffer that the bridge will fill in with
    // the bound data in native format.
    if( !parameterIndex || parameterIndex > numParams)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    sal_Int8* bindBuf = allocBindBuf (parameterIndex, 32);

    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    TIME_STRUCT x = OTools::TimeToOdbcTime(aVal);
    PREP_BIND_PARAM(TIME_STRUCT,DataType::TIME);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setTimestamp( sal_Int32 parameterIndex, const DateTime& aVal ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();
    // Allocate a buffer to be used in binding.  This will be
    // a 'permanent' buffer that the bridge will fill in with
    // the bound data in native format.
    if( !parameterIndex || parameterIndex > numParams)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    sal_Int8* bindBuf = allocBindBuf (parameterIndex, 32);

    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    TIMESTAMP_STRUCT x = OTools::DateTimeToTimestamp(aVal);
    PREP_BIND_PARAM(TIMESTAMP_STRUCT,DataType::TIMESTAMP);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();
    // Allocate a buffer to be used in binding.  This will be
    // a 'permanent' buffer that the bridge will fill in with
    // the bound data in native format.
    if( !parameterIndex || parameterIndex > numParams)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    sal_Int8* bindBuf = allocBindBuf (parameterIndex, 8);
    PREP_BIND_PARAM(double,DataType::DOUBLE);
}

// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();
    // Allocate a buffer to be used in binding.  This will be
    // a 'permanent' buffer that the bridge will fill in with
    // the bound data in native format.
    if( !parameterIndex || parameterIndex > numParams)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    sal_Int8* bindBuf = allocBindBuf (parameterIndex, 8);
    PREP_BIND_PARAM(float,DataType::FLOAT);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();
    // Allocate a buffer to be used in binding.  This will be
    // a 'permanent' buffer that the bridge will fill in with
    // the bound data in native format.
    if( !parameterIndex || parameterIndex > numParams)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    sal_Int8* bindBuf = allocBindBuf (parameterIndex, 4);
    PREP_BIND_PARAM(sal_Int32,DataType::INTEGER);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 aVal ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();
    // Allocate a buffer to be used in binding.  This will be
    // a 'permanent' buffer that the bridge will fill in with
    // the bound data in native format.

    if( !parameterIndex || parameterIndex > numParams)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    sal_Int8 *bindBuf = allocBindBuf (parameterIndex, 8);
    float x = (float)aVal;
    PREP_BIND_PARAM(float,DataType::BIGINT);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();
    // Get the buffer needed for the length
    if( !parameterIndex || parameterIndex > numParams)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    sal_Int8* lenBuf = getLengthBuf (parameterIndex);
    *(SDWORD*)lenBuf = SQL_NULL_DATA;


    UDWORD  prec = 0;
    if (sqlType == SQL_CHAR || sqlType == SQL_VARCHAR || sqlType == SQL_LONGVARCHAR)
        prec = 1;

    SQLSMALLINT fCType = 0;
    SQLSMALLINT fSqlType = 0;
    SQLUINTEGER nColumnSize = 0;
    SQLSMALLINT nDecimalDigits = 0;
    OTools::getBindTypes(sal_False,sal_False,sqlType,fCType,fSqlType,nColumnSize,nDecimalDigits);

    SQLRETURN nReturn = N3SQLBindParameter(m_aStatementHandle,
                            parameterIndex,
                            SQL_PARAM_INPUT,
                            fCType,
                            fSqlType,
                            nDecimalDigits,
                            nColumnSize,
                            NULL,
                            prec,
                            (SDWORD*)lenBuf
                            );
    OTools::ThrowException(nReturn,m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setClob( sal_Int32 parameterIndex, const Reference< XClob >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!isPrepared())
        prepareStatement();
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBlob( sal_Int32 parameterIndex, const Reference< XBlob >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!isPrepared())
        prepareStatement();
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setArray( sal_Int32 parameterIndex, const Reference< XArray >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!isPrepared())
        prepareStatement();
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setRef( sal_Int32 parameterIndex, const Reference< XRef >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if(!isPrepared())
        prepareStatement();
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();
    // For each known SQL Type, call the appropriate
        // set routine

    switch (sqlType)
    {
        case DataType::CHAR:
            setString (parameterIndex, *(::rtl::OUString*) x.getValue());
            break;

        case DataType::VARCHAR:
            setChar (parameterIndex, sqlType, 0, *(::rtl::OUString*) x.getValue());
            break;

        case DataType::LONGVARCHAR:
            setChar (parameterIndex, sqlType, 0, *(::rtl::OUString*) x.getValue());
            break;

        case DataType::BIT:
            setBoolean (parameterIndex,*(sal_Bool*) x.getValue());
            break;

        case DataType::TINYINT:
            setByte (parameterIndex, *(sal_Int8*)x.getValue());
            break;

        case DataType::SMALLINT:
            setShort (parameterIndex, *(short*)x.getValue());
            break;

        case DataType::INTEGER:
            setInt (parameterIndex,*(sal_Int32*)x.getValue ());
            break;

        case DataType::BIGINT:
            //  setLong (parameterIndex,((sal_Int32) x).longValue ());
            break;

        case DataType::REAL:
            setFloat (parameterIndex, *(float*)x.getValue ());
            break;

        case DataType::FLOAT:
            setFloat (parameterIndex, *(float*)x.getValue ());
            break;

        case DataType::DOUBLE:
            setDouble (parameterIndex,*(double*)x.getValue ());
            break;

        case DataType::BINARY:
            setBytes (parameterIndex, *(Sequence<sal_Int8>*)x.getValue());
            break;

        case DataType::VARBINARY:
        case DataType::LONGVARBINARY:
            {
                Sequence<sal_Int8> y;
                x >>= y;

                if ( y.getLength() > 2000 )
                {
                    //  setBinaryStream (parameterIndex, y, y.getLength());
                }
                else
                {
                    setBinary (parameterIndex, sqlType, y);
                }
            }

            break;

        case DataType::DATE:
            setDate (parameterIndex,*(Date*) x.getValue());
            break;

        case DataType::TIME:
            setTime (parameterIndex, *(Time*)x.getValue());
            break;

        case DataType::TIMESTAMP:
            setTimestamp (parameterIndex,*(DateTime*)x.getValue());
            break;

        default:
            {
                ::rtl::OUString aVal = ::rtl::OUString::createFromAscii("Unknown SQL Type for PreparedStatement.setObject (SQL Type=");
                aVal += ::rtl::OUString::valueOf(sqlType);
                throw SQLException( aVal,*this,::rtl::OUString(),0,Any());
            }

        }
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectNull( sal_Int32 parameterIndex, sal_Int32 sqlType, const ::rtl::OUString& typeName ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    setNull(parameterIndex,sqlType);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();
    //  setObject (parameterIndex, x, sqlType, 0);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();
    // Allocate a buffer to be used in binding.  This will be
    // a 'permanent' buffer that the bridge will fill in with
    // the bound data in native format.
    if( !parameterIndex || parameterIndex > numParams)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    sal_Int8* bindBuf = allocBindBuf (parameterIndex, 4);
    PREP_BIND_PARAM(sal_Int16,DataType::SMALLINT);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();
    if ( x.getLength() > 2000 )
    {
        //  setBinaryStream (parameterIndex, new java.io.ByteArrayInputStream(x), x.length);
    }
    else
    {
        setBinary (parameterIndex, DataType::BINARY, x);
    }
}
// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();
    setStream (parameterIndex, x, length, DataType::LONGVARCHAR,OBoundParam::ASCII);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!isPrepared())
        prepareStatement();
    setStream (parameterIndex, x, length, DataType::LONGVARBINARY,OBoundParam::BINARY);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::clearParameters(  ) throw(SQLException, RuntimeException)
{
    if(!isPrepared())
        prepareStatement();
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    N3SQLFreeStmt (m_aStatementHandle, SQL_RESET_PARAMS);
}
// -------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::clearBatch(  ) throw(SQLException, RuntimeException)
{
    //  clearParameters(  );
    //  m_aBatchList.erase();
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

//====================================================================
// methods
//====================================================================

//--------------------------------------------------------------------
// initBoundParam
// Initialize the bound parameter objects
//--------------------------------------------------------------------

void OPreparedStatement::initBoundParam () throw(SQLException)
{
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    // Get the number of parameters
    numParams = 0;
    N3SQLNumParams (m_aStatementHandle,(short*)&numParams);

    // There are parameter markers, allocate the bound
    // parameter objects

    if (numParams > 0)
    {
        // Allocate an array of bound parameter objects

        boundParams = new OBoundParam[numParams];

        // Allocate and initialize each bound parameter

        for (sal_Int32 i = 0; i < numParams; i++)
        {
            boundParams[i] = OBoundParam();
            boundParams[i].initialize ();
        }
    }
}
// -------------------------------------------------------------------------

//--------------------------------------------------------------------
// allocBindBuf
// Allocate storage for the permanent data buffer for the bound
// parameter.
//--------------------------------------------------------------------

sal_Int8* OPreparedStatement::allocBindBuf( sal_Int32 index,sal_Int32 bufLen)
{
    sal_Int8* b = NULL;

    // Sanity check the parameter number

    if ((index >= 1) &&
        (index <= numParams))
    {
        b = boundParams[index - 1].allocBindDataBuffer(bufLen);
    }

    return b;
}
// -------------------------------------------------------------------------

//--------------------------------------------------------------------
// getDataBuf
// Gets the data buffer for the given parameter index
//--------------------------------------------------------------------

sal_Int8* OPreparedStatement::getDataBuf (sal_Int32 index)
{
    sal_Int8* b = NULL;

    // Sanity check the parameter number

    if ((index >= 1) &&
        (index <= numParams))
    {
        b = boundParams[index - 1].getBindDataBuffer ();
    }

    return b;
}
// -------------------------------------------------------------------------

//--------------------------------------------------------------------
// getLengthBuf
// Gets the length buffer for the given parameter index
//--------------------------------------------------------------------

sal_Int8* OPreparedStatement::getLengthBuf (sal_Int32 index)
{
    sal_Int8* b = NULL;

    // Sanity check the parameter number

    if ((index >= 1) &&
        (index <= numParams))
    {
        b = boundParams[index - 1].getBindLengthBuffer ();
    }

    return b;
}
// -------------------------------------------------------------------------

//--------------------------------------------------------------------
// getParamLength
// Returns the length of the given parameter number.  When each
// parameter was bound, a 4-sal_Int8 buffer was given to hold the
// length (stored in native format).  Get the buffer, convert the
// buffer from native format, and return it.  If the length is -1,
// the column is considered to be NULL.
//--------------------------------------------------------------------

sal_Int32 OPreparedStatement::getParamLength (  sal_Int32 index)
{
    sal_Int32 paramLen = SQL_NULL_DATA;

    // Sanity check the parameter number

    if ((index >= 1) &&
        (index <= numParams)) {

        // Now get the length of the parameter from the
        // bound param array.  -1 is returned if it is NULL.
        long n = 0;
        memcpy (&n, boundParams[index -1].getBindLengthBuffer (), sizeof (n));
        paramLen = n;
    }
    return paramLen;
}
// -------------------------------------------------------------------------

//--------------------------------------------------------------------
// putParamData
// Puts parameter data from a previously bound input stream.  The
// input stream was bound using SQL_LEN_DATA_AT_EXEC.
//--------------------------------------------------------------------

void OPreparedStatement::putParamData (sal_Int32 index) throw(SQLException)
{
    // We'll transfer up to maxLen at a time
    sal_Int32   maxLen = MAX_PUT_DATA_LENGTH;
    sal_Int32   bufLen;
    sal_Int32   realLen;
    //  sal_Int8*   buf = new sal_Int8[maxLen];
    Sequence< sal_Int8 > buf(maxLen);
    sal_Bool    endOfStream = sal_False;

    // Sanity check the parameter index
    if ((index < 1) ||
        (index > numParams))
    {
        return;
    }

    // Get the information about the input stream

    Reference< XInputStream> inputStream =  boundParams[index - 1].getInputStream ();
    sal_Int32 inputStreamLen = boundParams[index - 1].getInputStreamLen ();
    sal_Int32 inputStreamType = boundParams[index - 1].getStreamType ();

    // Loop while more data from the input stream

    while (!endOfStream)
    {

        // Read some data from the input stream

        try {
            bufLen = inputStream->readBytes(buf,maxLen);
        }
        catch (IOException& ex) {

            // If an I/O exception was generated, turn
            // it into a SQLException

            throw SQLException(ex.Message,*this,::rtl::OUString(),0,Any());
        }

        // -1 as the number of bytes read indicates that
        // there is no more data in the input stream

        if (bufLen == -1)
        {

            // Sanity check to ensure that all the data we said we
            // had was read.  If not, raise an exception

            if (inputStreamLen != 0) {
                throw SQLException (::rtl::OUString::createFromAscii("End of InputStream reached before satisfying length specified when InputStream was set"),
            *this,
                    ::rtl::OUString(),0,Any());
            }
            endOfStream = sal_True;
            break;
        }

        // If we got more bytes than necessary, truncate
        // the buffer by re-setting the buffer length.  Also,
        // indicate that we don't need to read any more.

        if (bufLen > inputStreamLen)
        {
            bufLen = inputStreamLen;
            endOfStream = sal_True;
        }

        realLen = bufLen;

        // For UNICODE streams, strip off the high sal_Int8 and set the
        // number of actual bytes present.  It is assumed that
        // there are 2 bytes present for every UNICODE character - if
        // not, then that's not our problem

        if (inputStreamType == OBoundParam::UNICODE)
        {
            realLen = bufLen / 2;

            for (sal_Int32 ii = 0; ii < realLen; ii++)
                buf[ii] = buf[(ii * 2) + 1];
        }

        // Put the data
        OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");

        N3SQLPutData (m_aStatementHandle, buf.getArray(), realLen);

        // Decrement the number of bytes still needed

        inputStreamLen -= bufLen;


        // If there is no more data to be read, exit loop

        if (inputStreamLen == 0)
            endOfStream = sal_True;
    }
}
// -------------------------------------------------------------------------
//--------------------------------------------------------------------
// getPrecision
// Given a SQL type, return the maximum precision for the column.
// Returns -1 if not known
//--------------------------------------------------------------------

sal_Int32 OPreparedStatement::getPrecision ( sal_Int32 sqlType)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    sal_Int32 prec = -1;
    OTypeInfo aInfo;
    aInfo.nType = sqlType;
    if (m_aTypeInfo.size())
    {
        ::std::vector<OTypeInfo>::const_iterator aIter = ::std::find(m_aTypeInfo.begin(),m_aTypeInfo.end(),aInfo);
        if(aIter != m_aTypeInfo.end())
            prec = (*aIter).nPrecision;
    }
    return prec;
}

//--------------------------------------------------------------------
// setStream
// Sets an input stream as a parameter, using the given SQL type
//--------------------------------------------------------------------

void OPreparedStatement::setStream (
    sal_Int32 ParameterIndex,
    const Reference< XInputStream>& x,
    sal_Int32 length,
    sal_Int32 SQLtype,
    sal_Int32 streamType)
    throw(SQLException)
{
    if( !ParameterIndex || ParameterIndex > numParams)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());
    // Get the buffer needed for the length

    sal_Int8* lenBuf = getLengthBuf(ParameterIndex);

    // Allocate a new buffer for the parameter data.  This buffer
    // will be returned by SQLParamData (it is set to the parameter
    // number, a 4-sal_Int8 integer)

    sal_Int8* dataBuf = allocBindBuf (ParameterIndex, 4);

    // Bind the parameter with SQL_LEN_DATA_AT_EXEC
    SWORD   Ctype = SQL_C_CHAR;
    SDWORD  atExec = SQL_LEN_DATA_AT_EXEC (length);
    memcpy (dataBuf, &ParameterIndex, sizeof(ParameterIndex));
    memcpy (lenBuf, &atExec, sizeof (atExec));

    if ((SQLtype == SQL_BINARY) || (SQLtype == SQL_VARBINARY) || (SQLtype == SQL_LONGVARBINARY))
        Ctype = SQL_C_BINARY;


    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    N3SQLBindParameter(m_aStatementHandle, ParameterIndex,SQL_PARAM_INPUT,Ctype,
                                SQLtype, length,0, dataBuf, sizeof(ParameterIndex),(SDWORD*)lenBuf);

    // Save the input stream

    boundParams[ParameterIndex - 1].setInputStream (x, length);

    // Set the stream type

    boundParams[ParameterIndex - 1].setStreamType (streamType);
}
// -------------------------------------------------------------------------

//--------------------------------------------------------------------
// setChar
// Binds the given string to the given SQL type
//--------------------------------------------------------------------
void OPreparedStatement::setChar(sal_Int32 parameterIndex,
                                sal_Int32 SQLtype,
                                sal_Int32 scale,
                                const ::rtl::OUString& x)
                                throw(SQLException)
{
    //  ::rtl::OString x1(::rtl::OUStringToOString(x,getConnection()->getTextEncoding()));

    if( !parameterIndex || parameterIndex > numParams)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    sal_Int8* bindBuf = allocBindBuf (parameterIndex,x.getLength());

    // Get the precision for this SQL type.  If the precision
    // is out of bounds, set it to our default

//  sal_Int32 precision = getPrecision (SQLtype);
//
//  if ((precision < 0) || (precision > 2000))
//      precision = 2000;
//
    //  bindParameter(m_aStatementHandle,parameterIndex,bindBuf);
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    PREP_BIND_PARAM(char*,SQLtype);

//  // Make a copy of the data
//  SDWORD  lBuf = x.getLength();
//  memcpy (bindBuf, x.getStr(), lBuf);
//  bindBuf[lBuf] = '\0';
//  precision = lBuf;
//
//  SQLINTEGER nNTS = SQL_NTS;
//
//  SQLRETURN nRet = N3SQLBindParameter(m_aStatementHandle, parameterIndex,SQL_PARAM_INPUT,
//                       SQLtype,SQL_C_CHAR, precision, scale, bindBuf,lBuf, &nNTS);
//
}
// -------------------------------------------------------------------------

//--------------------------------------------------------------------
// setBinary
// Binds the given sal_Int8 array to the given SQL type
//--------------------------------------------------------------------

void OPreparedStatement::setBinary (sal_Int32 parameterIndex,sal_Int32 SQLtype,
                                    const Sequence< sal_Int8 >& x) throw(SQLException)
{
    // Allocate a buffer to be used in binding.  This will be
    // a 'permanent' buffer that the bridge will fill in with
    // the bound data in native format.
    if( !parameterIndex || parameterIndex > numParams)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    sal_Int8* bindBuf = allocBindBuf (parameterIndex,x.getLength());

    // Get the buffer needed for the length

    //  sal_Int8* lenBuf = getLengthBuf (parameterIndex);
    bindParameter< Sequence< sal_Int8 > >(          m_aStatementHandle,
                                parameterIndex,
                                bindBuf,getLengthBuf(parameterIndex),
                                SQLtype,
                                sal_False,sal_False,&x,(Reference <XInterface>)*this,getOwnConnection()->getTextEncoding());


    //  N3SQLBindInParameterBinary (m_aStatementHandle, parameterIndex,
                            //  SQLtype, x.getConstArray(), bindBuf, lenBuf, buffers);

}
// -------------------------------------------------------------------------

void OPreparedStatement::FreeParams()
{
    delete [] boundParams;
    boundParams = NULL;
}
// -------------------------------------------------------------------------
void OPreparedStatement::setFastPropertyValue_NoBroadcast(sal_Int32 nHandle,const Any& rValue) throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            if(!isPrepared())
                setResultSetConcurrency(connectivity::getINT32(rValue));
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            if(!isPrepared())
                setResultSetType(connectivity::getINT32(rValue));
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            if(!isPrepared())
                setFetchDirection(connectivity::getINT32(rValue));
            break;
        case PROPERTY_ID_USEBOOKMARKS:
            if(!isPrepared())
                setUsingBookmarks(connectivity::getBOOL(rValue));
            break;
        default:
            OStatement_Base::setFastPropertyValue_NoBroadcast(nHandle,rValue);
    }
}
// -----------------------------------------------------------------------------
void OPreparedStatement::prepareStatement()
{
    m_bPrepared = sal_True;
    OSL_ENSURE(m_aStatementHandle,"StatementHandle is null!");
    ::rtl::OString aSql(::rtl::OUStringToOString(m_sSqlStatement,getOwnConnection()->getTextEncoding()));
    SQLRETURN nReturn = N3SQLPrepare(m_aStatementHandle,(SDB_ODBC_CHAR *) aSql.getStr(),aSql.getLength());
    OTools::ThrowException(nReturn,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    initBoundParam();
}
// -----------------------------------------------------------------------------


