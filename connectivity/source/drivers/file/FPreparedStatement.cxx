/*************************************************************************
 *
 *  $RCSfile: FPreparedStatement.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: oj $ $Date: 2001-04-06 14:03:08 $
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

#include <stdio.h>

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _CONNECTIVITY_FILE_OPREPAREDSTATEMENT_HXX_
#include "file/FPreparedStatement.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _CONNECTIVITY_FILE_ORESULTSETMETADATA_HXX_
#include "file/FResultSetMetaData.hxx"
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE file
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CONNECTIVITY_DATECONVERSION_HXX_
#include "connectivity/DateConversion.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _DBHELPER_DBCONVERSION_HXX_
#include "connectivity/dbconversion.hxx"
#endif


using namespace connectivity;
using namespace ::dbtools;
using namespace connectivity::file;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::util;

IMPLEMENT_SERVICE_INFO(OPreparedStatement,"com.sun.star.sdbc.driver.file.PreparedStatement","com.sun.star.sdbc.PreparedStatement");

OPreparedStatement::OPreparedStatement( OConnection* _pConnection,const ::std::vector<OTypeInfo>& _TypeInfo)
    : OStatement_BASE2( _pConnection )
    ,m_pResultSet(NULL)
    ,m_pTable(NULL)
{
}
// -------------------------------------------------------------------------
void OPreparedStatement::disposing()
{
    OStatement_BASE2::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    m_xMetaData = NULL;
    if(m_aRow.isValid())
    {
        m_aRow->clear();
        m_aRow = NULL;
    }

    if(m_pTable)
    {
        m_pTable->release();
        m_pTable = NULL;
    }
}
// -------------------------------------------------------------------------
void OPreparedStatement::construct(const ::rtl::OUString& sql)  throw(SQLException, RuntimeException)
{
    ::rtl::OUString aErr;
    m_pParseTree = m_aParser.parseTree(aErr,sql);
    if(m_pParseTree)
    {
        m_aSQLIterator.setParseTree(m_pParseTree);
        m_aSQLIterator.traverseAll();
        const OSQLTables& xTabs = m_aSQLIterator.getTables();
        if(xTabs.begin() == xTabs.end())
            throw SQLException(::rtl::OUString::createFromAscii("Driver does not support this function!"),*this,::rtl::OUString::createFromAscii("IM001"),0,Any());

        // at this moment we support only one table per select statement
        Reference< ::com::sun::star::lang::XUnoTunnel> xTunnel(xTabs.begin()->second,UNO_QUERY);
        if(xTunnel.is())
        {
            if(m_pTable)
                m_pTable->release();
            m_pTable = (OFileTable*)xTunnel->getSomething(OFileTable::getUnoTunnelImplementationId());
            if(m_pTable)
                m_pTable->acquire();
        }

        m_aRow = new OValueVector();
        m_aRow->push_back(sal_Int32(0));
    }
    else
        throw SQLException(aErr,*this,::rtl::OUString(),0,Any());
}
// -------------------------------------------------------------------------

Any SAL_CALL OPreparedStatement::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OStatement_BASE2::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = ::cppu::queryInterface(  rType,
                                        static_cast< XPreparedStatement*>(this),
                                        static_cast< XParameters*>(this),
                                        static_cast< XResultSetMetaDataSupplier*>(this));
    return aRet;
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL OPreparedStatement::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
        ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< XPreparedStatement > *)0 ),
                                        ::getCppuType( (const ::com::sun::star::uno::Reference< XParameters > *)0 ),
                                        ::getCppuType( (const ::com::sun::star::uno::Reference< XResultSetMetaDataSupplier > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OStatement_BASE2::getTypes());
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OPreparedStatement::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_aSQLIterator.getSelectColumns(),m_aSQLIterator.getTables().begin()->first,m_pTable);
    return m_xMetaData;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::close(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    clearMyResultSet();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OPreparedStatement::execute(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XResultSet > rs = initResultSet();

    return m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT || m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT_COUNT;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OPreparedStatement::executeUpdate(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    Reference< XResultSet > rs = initResultSet();
    return m_pResultSet ? m_pResultSet->getRowCountResult() : sal_Int32(0);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setString( sal_Int32 parameterIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if( parameterIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());
    if(parameterIndex >= (sal_Int32)m_aRow->size())
        m_aRow->push_back(x);
    else
        (*m_aRow)[parameterIndex] = x;
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

    Reference< XResultSet > rs = initResultSet();
    return rs;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBoolean( sal_Int32 parameterIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if( parameterIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    if(parameterIndex >= (sal_Int32)m_aRow->size())
        m_aRow->push_back(x);
    else
        (*m_aRow)[parameterIndex] = x;
}
// -------------------------------------------------------------------------
void SAL_CALL OPreparedStatement::setByte( sal_Int32 parameterIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if( parameterIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    if(parameterIndex >= (sal_Int32)m_aRow->size())
        m_aRow->push_back((sal_Int32)x);
    else
        (*m_aRow)[parameterIndex] = (sal_Int32)x;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDate( sal_Int32 parameterIndex, const Date& aData ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if( parameterIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    if(parameterIndex >= (sal_Int32)m_aRow->size())
        m_aRow->push_back(DBTypeConversion::toDouble(aData));
    else
        (*m_aRow)[parameterIndex] = DBTypeConversion::toDouble(aData);
}
// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setTime( sal_Int32 parameterIndex, const Time& aVal ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if( parameterIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    if(parameterIndex >= (sal_Int32)m_aRow->size())
        m_aRow->push_back(DBTypeConversion::toDouble(aVal));
    else
        (*m_aRow)[parameterIndex] = DBTypeConversion::toDouble(aVal);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setTimestamp( sal_Int32 parameterIndex, const DateTime& aVal ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if( parameterIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    if(parameterIndex >= (sal_Int32)m_aRow->size())
        m_aRow->push_back(DBTypeConversion::toDouble(aVal));
    else
        (*m_aRow)[parameterIndex] = DBTypeConversion::toDouble(aVal);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setDouble( sal_Int32 parameterIndex, double x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if( parameterIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    if(parameterIndex >= (sal_Int32)m_aRow->size())
        m_aRow->push_back(x);
    else
        (*m_aRow)[parameterIndex] = x;
}

// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setFloat( sal_Int32 parameterIndex, float x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if( parameterIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    if(parameterIndex >= (sal_Int32)m_aRow->size())
        m_aRow->push_back((double)x);
    else
        (*m_aRow)[parameterIndex] = (double)x;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setInt( sal_Int32 parameterIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if( parameterIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    if(parameterIndex >= (sal_Int32)m_aRow->size())
        m_aRow->push_back(x);
    else
        (*m_aRow)[parameterIndex] = x;
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setLong( sal_Int32 parameterIndex, sal_Int64 aVal ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setNull( sal_Int32 parameterIndex, sal_Int32 sqlType ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    // Get the buffer needed for the length
    if( parameterIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    if(parameterIndex >= (sal_Int32)m_aRow->size())
        m_aRow->push_back(ORowSetValue());
    else
        (*m_aRow)[parameterIndex].setNull();
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setClob( sal_Int32 parameterIndex, const Reference< XClob >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBlob( sal_Int32 parameterIndex, const Reference< XBlob >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setArray( sal_Int32 parameterIndex, const Reference< XArray >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setRef( sal_Int32 parameterIndex, const Reference< XRef >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObjectWithInfo( sal_Int32 parameterIndex, const Any& x, sal_Int32 sqlType, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if( parameterIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());
    // For each known SQL Type, call the appropriate
        // set routine

    switch (sqlType)
    {
        case DataType::CHAR:
        case DataType::VARCHAR:
        case DataType::LONGVARCHAR:
            (*m_aRow)[parameterIndex] = *(::rtl::OUString*) x.getValue();
            break;
        case DataType::BIT:
            (*m_aRow)[parameterIndex] = *(sal_Bool*) x.getValue();
            break;

        case DataType::TINYINT:
        case DataType::SMALLINT:
        case DataType::INTEGER:
            (*m_aRow)[parameterIndex] = *(sal_Int32*)x.getValue();
            break;

        case DataType::BIGINT:
        case DataType::REAL:
        case DataType::FLOAT:
        case DataType::DOUBLE:
            (*m_aRow)[parameterIndex] = *(double*)x.getValue();
            break;

        case DataType::BINARY:
            break;

        case DataType::VARBINARY:
        case DataType::LONGVARBINARY:
            break;

        case DataType::DATE:
            (*m_aRow)[parameterIndex] = DBTypeConversion::toDouble(*(Date*) x.getValue());
            break;

        case DataType::TIME:
            (*m_aRow)[parameterIndex] = DBTypeConversion::toDouble(*(Time*)x.getValue());
            break;

        case DataType::TIMESTAMP:
            (*m_aRow)[parameterIndex] = DBTypeConversion::toDouble(*(DateTime*)x.getValue());
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
    if( parameterIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());

    if(parameterIndex >= (sal_Int32)m_aRow->size())
        m_aRow->push_back(ORowSetValue());
    else
        (*m_aRow)[parameterIndex].setNull();
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setObject( sal_Int32 parameterIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());
    //  setObject (parameterIndex, x, sqlType, 0);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setShort( sal_Int32 parameterIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    if( parameterIndex < 1)
        throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());
    setInt(parameterIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBytes( sal_Int32 parameterIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());
}
// -------------------------------------------------------------------------


void SAL_CALL OPreparedStatement::setCharacterStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::setBinaryStream( sal_Int32 parameterIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    throw SQLException(STAT_INVALID_INDEX,*this,::rtl::OUString::createFromAscii("07009"),0,Any());
}
// -------------------------------------------------------------------------

void SAL_CALL OPreparedStatement::clearParameters(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (OStatement_BASE::rBHelper.bDisposed)
        throw DisposedException();
    m_aRow->clear();
}
// -------------------------------------------------------------------------
OResultSet* OPreparedStatement::createResultSet()
{
    return new OResultSet(this,m_aSQLIterator);
}
// -----------------------------------------------------------------------------
Reference<XResultSet> OPreparedStatement::initResultSet()
{
    ::comphelper::disposeComponent(m_xResultSet.get());
    m_pResultSet = createResultSet();
    Reference<XResultSet> xRs(m_pResultSet);
    m_xResultSet = xRs;
    if(m_aSQLIterator.getStatementType() == SQL_STATEMENT_SELECT || m_aSQLIterator.getStatementType() == SQL_STATEMENT_UPDATE)
        m_pResultSet->describeParameter();
    m_pResultSet->setParameterRow(m_aRow);
    m_pResultSet->OpenImpl();
    return xRs;
}
// -----------------------------------------------------------------------------

