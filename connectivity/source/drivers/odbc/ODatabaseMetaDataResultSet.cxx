/*************************************************************************
 *
 *  $RCSfile: ODatabaseMetaDataResultSet.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: fs $ $Date: 2001-03-02 16:19:05 $
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


#ifndef _CONNECTIVITY_ADO_ADATABASEMETADATARESULTSET_HXX_
#include "odbc/ODatabaseMetaDataResultSet.hxx"
#endif
#define CONNECTIVITY_PROPERTY_NAME_SPACE odbc
#ifndef _CONNECTIVITY_PROPERTYIDS_HXX_
#include "propertyids.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_KEYRULE_HPP_
#include <com/sun/star/sdbc/KeyRule.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_PROCEDURERESULT_HPP_
#include <com/sun/star/sdbc/ProcedureResult.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_INDEXTYPE_HPP_
#include <com/sun/star/sdbc/IndexType.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETCONCURRENCY_HPP_
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_RESULTSETTYPE_HPP_
#include <com/sun/star/sdbc/ResultSetType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_FETCHDIRECTION_HPP_
#include <com/sun/star/sdbc/FetchDirection.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CONNECTIVITY_ODBC_ORESULTSETMETADATA_HXX_
#include "odbc/OResultSetMetaData.hxx"
#endif
#ifndef _CONNECTIVITY_OTOOLS_HXX_
#include "odbc/OTools.hxx"
#endif

using namespace connectivity::odbc;
using namespace cppu;
//------------------------------------------------------------------------------
using namespace ::com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::util;

// -------------------------------------------------------------------------
ODatabaseMetaDataResultSet::ODatabaseMetaDataResultSet(SQLHANDLE _pStatementHandle,rtl_TextEncoding _nTextEncoding) :   ODatabaseMetaDataResultSet_BASE(m_aMutex)
                        ,OPropertySetHelper(ODatabaseMetaDataResultSet_BASE::rBHelper)
                        ,m_aStatement(NULL)
                        ,m_xMetaData(NULL)
                        ,m_aStatementHandle(_pStatementHandle)
                        ,m_bEOF(sal_False)
                        ,m_nTextEncoding(_nTextEncoding)
{
    osl_incrementInterlockedCount( &m_refCount );
    m_pRowStatusArray = new SQLUSMALLINT[1]; // the default value
    osl_decrementInterlockedCount( &m_refCount );
    //  allocBuffer();
}

// -------------------------------------------------------------------------
ODatabaseMetaDataResultSet::~ODatabaseMetaDataResultSet()
{
    delete m_pRowStatusArray;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    if(m_bFreeHandle)
    {
        N3SQLFreeStmt(m_aStatementHandle,SQL_CLOSE);
        N3SQLFreeHandle(SQL_HANDLE_STMT,m_aStatementHandle);
        m_aStatementHandle = NULL;
    }

    m_aStatement    = NULL;
    m_xMetaData     = NULL;
}
// -------------------------------------------------------------------------
Any SAL_CALL ODatabaseMetaDataResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = ODatabaseMetaDataResultSet_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL ODatabaseMetaDataResultSet::getTypes(  ) throw(::com::sun::star::uno::RuntimeException)
{
    ::cppu::OTypeCollection aTypes( ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XFastPropertySet > *)0 ),
                                    ::getCppuType( (const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),ODatabaseMetaDataResultSet_BASE::getTypes());
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::findColumn( const ::rtl::OUString& columnName ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed )
        throw DisposedException();

    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    sal_Int32 i = 1;
    for(;i<=nLen;++i)
        if(xMeta->isCaseSensitive(i) ? columnName == xMeta->getColumnName(i) : columnName.equalsIgnoreCase(xMeta->getColumnName(i)))
            break;
    return i;
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    return NULL;
}
// -------------------------------------------------------------------------
Reference< ::com::sun::star::io::XInputStream > SAL_CALL ODatabaseMetaDataResultSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    return NULL;
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    return getValue(m_aStatementHandle,columnIndex,SQL_C_BIT,m_bWasNull,**this,sal_Int8(0));
}
// -------------------------------------------------------------------------

sal_Int8 SAL_CALL ODatabaseMetaDataResultSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    sal_Int8  nVal = getValue(m_aStatementHandle,columnIndex,SQL_C_CHAR,m_bWasNull,**this,sal_Int8(0));

    if(m_aValueRange.size() && (m_aValueRangeIter = m_aValueRange.find(columnIndex)) != m_aValueRange.end())
        return (*m_aValueRangeIter).second[(sal_Int32)nVal];
    return nVal;
}
// -------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL ODatabaseMetaDataResultSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    sal_Int32 nType = getMetaData()->getColumnType(columnIndex);
    switch(nType)
    {
        case DataType::VARCHAR:
        case DataType::LONGVARCHAR:
            {
                ::rtl::OUString aRet = OTools::getStringValue(m_aStatementHandle,columnIndex,getMetaData()->getColumnType(columnIndex),m_bWasNull,**this,m_nTextEncoding);
                return Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>(aRet.getStr()),sizeof(sal_Unicode)*aRet.getLength());
            }
            break;
    }
    return OTools::getBytesValue(m_aStatementHandle,columnIndex,nType,m_bWasNull,**this);
}
// -------------------------------------------------------------------------

::com::sun::star::util::Date SAL_CALL ODatabaseMetaDataResultSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    DATE_STRUCT aDate;
    aDate.day = 0;
    aDate.month = 0;
    aDate.year = 0;
    aDate = getValue(m_aStatementHandle,columnIndex,SQL_C_DATE,m_bWasNull,**this,aDate);
    return Date(aDate.day,aDate.month,aDate.year);
}
// -------------------------------------------------------------------------

double SAL_CALL ODatabaseMetaDataResultSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    return getValue(m_aStatementHandle,columnIndex,SQL_C_DOUBLE,m_bWasNull,**this,double(0.0));
}
// -------------------------------------------------------------------------

float SAL_CALL ODatabaseMetaDataResultSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    return getValue(m_aStatementHandle,columnIndex,SQL_C_FLOAT,m_bWasNull,**this,float(0));
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    sal_Int32 nVal = getValue(m_aStatementHandle,columnIndex,SQL_C_LONG,m_bWasNull,**this,sal_Int32(0));

    if(m_aValueRange.size() && (m_aValueRangeIter = m_aValueRange.find(columnIndex)) != m_aValueRange.end())
        return (*m_aValueRangeIter).second[(sal_Int32)nVal];
    return nVal;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL ODatabaseMetaDataResultSet::getRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return 0;
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL ODatabaseMetaDataResultSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
    columnIndex = mapColumn(columnIndex);
    return sal_Int64(0);
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL ODatabaseMetaDataResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_aStatementHandle);
    return m_xMetaData;
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL ODatabaseMetaDataResultSet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    return NULL;
}

// -------------------------------------------------------------------------

Reference< XClob > SAL_CALL ODatabaseMetaDataResultSet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL ODatabaseMetaDataResultSet::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    return NULL;
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL ODatabaseMetaDataResultSet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    return NULL;
}
// -------------------------------------------------------------------------

Any SAL_CALL ODatabaseMetaDataResultSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    return Any();
}
// -------------------------------------------------------------------------

sal_Int16 SAL_CALL ODatabaseMetaDataResultSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    sal_Int16 nVal = getValue(m_aStatementHandle,columnIndex,SQL_C_SHORT,m_bWasNull,**this,sal_Int16(0));

    if(m_aValueRange.size() && (m_aValueRangeIter = m_aValueRange.find(columnIndex)) != m_aValueRange.end())
        return (*m_aValueRangeIter).second[(sal_Int32)nVal];
    return nVal;
}
// -------------------------------------------------------------------------

::rtl::OUString SAL_CALL ODatabaseMetaDataResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    ::rtl::OUString aVal = OTools::getStringValue(m_aStatementHandle,columnIndex,getMetaData()->getColumnType(columnIndex),m_bWasNull,**this,m_nTextEncoding);

    return aVal;
}

// -------------------------------------------------------------------------


::com::sun::star::util::Time SAL_CALL ODatabaseMetaDataResultSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    TIME_STRUCT aTime={0,0,0};
    aTime = getValue(m_aStatementHandle,columnIndex,SQL_C_TIME,m_bWasNull,**this,aTime);
    return Time(0,aTime.second,aTime.minute,aTime.hour);
}
// -------------------------------------------------------------------------


::com::sun::star::util::DateTime SAL_CALL ODatabaseMetaDataResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    columnIndex = mapColumn(columnIndex);
    TIMESTAMP_STRUCT aTime={0,0,0,0,0,0,0};
    aTime = getValue(m_aStatementHandle,columnIndex,SQL_C_TIMESTAMP,m_bWasNull,**this,aTime);
    return DateTime(aTime.fraction*1000,aTime.second,aTime.minute,aTime.hour,aTime.day,aTime.month,aTime.year);
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return m_nCurrentFetchState == SQL_NO_DATA;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return m_nRowPos == 1;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return m_bEOF && m_nCurrentFetchState != SQL_NO_DATA;
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(first())
        previous();
    m_nCurrentFetchState = SQL_SUCCESS;
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    if(last())
        next();
    m_bEOF = sal_True;
}
// -------------------------------------------------------------------------

void SAL_CALL ODatabaseMetaDataResultSet::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
            throw DisposedException();
    }
    dispose();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::first(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_FIRST,0);
    OTools::ThrowException(m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    sal_Bool bRet;
    if(bRet = (m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO))
        m_nRowPos = 1;
    return bRet;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::last(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed )
        throw DisposedException();

    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_LAST,0);
    OTools::ThrowException(m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    // here I know definitely that I stand on the last record
    return m_bLastRecord = (m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_ABSOLUTE,row);
    OTools::ThrowException(m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    sal_Bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
        m_nRowPos = row;
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_RELATIVE,row);
    OTools::ThrowException(m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    sal_Bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
        m_nRowPos += row;
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::previous(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_PRIOR,0);
    OTools::ThrowException(m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    sal_Bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
        --m_nRowPos;
    return bRet;
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL ODatabaseMetaDataResultSet::getStatement(  ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return m_pRowStatusArray[0] == SQL_ROW_DELETED;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{   ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return m_pRowStatusArray[0] == SQL_ROW_ADDED;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL ODatabaseMetaDataResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return m_pRowStatusArray[0] == SQL_ROW_UPDATED;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return m_nRowPos == 0;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::next(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    //  m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_NEXT,0);
    m_nCurrentFetchState = N3SQLFetch(m_aStatementHandle);
    OTools::ThrowException(m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    return m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL ODatabaseMetaDataResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    return m_bWasNull;
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();
}
// -------------------------------------------------------------------------

void SAL_CALL ODatabaseMetaDataResultSet::cancel(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if (ODatabaseMetaDataResultSet_BASE::rBHelper.bDisposed)
        throw DisposedException();

    OTools::ThrowException(N3SQLCancel(m_aStatementHandle),m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------
void SAL_CALL ODatabaseMetaDataResultSet::clearWarnings(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
Any SAL_CALL ODatabaseMetaDataResultSet::getWarnings(  ) throw(SQLException, RuntimeException)
{
    return Any();
}
//------------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::getResultSetConcurrency() const throw(SQLException, RuntimeException)
{
    return ResultSetConcurrency::READ_ONLY;
}
//------------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::getResultSetType() const throw(SQLException, RuntimeException)
{
    return ResultSetType::FORWARD_ONLY;
}
//------------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::getFetchDirection() const throw(SQLException, RuntimeException)
{
    return FetchDirection::FORWARD;
}
//------------------------------------------------------------------------------
sal_Int32 ODatabaseMetaDataResultSet::getFetchSize() const throw(SQLException, RuntimeException)
{
    sal_Int32 nValue=1;
    return nValue;
}
//------------------------------------------------------------------------------
::rtl::OUString ODatabaseMetaDataResultSet::getCursorName() const throw(SQLException, RuntimeException)
{
    return ::rtl::OUString();
}

//------------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setFetchDirection(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
}
//------------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setFetchSize(sal_Int32 _par0) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ODatabaseMetaDataResultSet::createArrayHelper( ) const
{

    Sequence< com::sun::star::beans::Property > aProps(5);
    com::sun::star::beans::Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP0(CURSORNAME,          ::rtl::OUString);
    DECL_PROP0(FETCHDIRECTION,      sal_Int32);
    DECL_PROP0(FETCHSIZE,           sal_Int32);
    DECL_PROP0(RESULTSETCONCURRENCY,sal_Int32);
    DECL_PROP0(RESULTSETTYPE,       sal_Int32);

    return new ::cppu::OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
::cppu::IPropertyArrayHelper & ODatabaseMetaDataResultSet::getInfoHelper()
{
    return *const_cast<ODatabaseMetaDataResultSet*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool ODatabaseMetaDataResultSet::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw ::com::sun::star::lang::IllegalArgumentException();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchDirection());
        case PROPERTY_ID_FETCHSIZE:
            return ::comphelper::tryPropertyValue(rConvertedValue, rOldValue, rValue, getFetchSize());
        default:
            ;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& rValue
                                                 )
                                                 throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
            throw Exception();
            break;
        default:
            OSL_ENSHURE(0,"setFastPropertyValue_NoBroadcast: Illegal handle value!");
    }
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::getFastPropertyValue(
                                Any& rValue,
                                sal_Int32 nHandle
                                     ) const
{
    switch(nHandle)
    {
        case PROPERTY_ID_CURSORNAME:
            rValue <<= getCursorName();
            break;
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            rValue <<= getResultSetConcurrency();
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            rValue <<= getResultSetType();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            rValue <<= getFetchDirection();
            break;
        case PROPERTY_ID_FETCHSIZE:
            rValue <<= getFetchSize();
            break;
    }
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openTypeInfo() throw(SQLException, RuntimeException)
{

    ::std::map<sal_Int32,sal_Int32> aMap;
    aMap[SQL_BIT]               = DataType::BIT;
    aMap[SQL_TINYINT]           = DataType::TINYINT;
    aMap[SQL_SMALLINT]          = DataType::SMALLINT;
    aMap[SQL_INTEGER]           = DataType::INTEGER;
    aMap[SQL_FLOAT]             = DataType::FLOAT;
    aMap[SQL_REAL]              = DataType::REAL;
    aMap[SQL_DOUBLE]            = DataType::DOUBLE;
    aMap[SQL_BIGINT]            = DataType::BIGINT;

    aMap[SQL_CHAR]              = DataType::CHAR;
    aMap[SQL_WCHAR]             = DataType::CHAR;
    aMap[SQL_VARCHAR]           = DataType::VARCHAR;
    aMap[SQL_WVARCHAR]          = DataType::VARCHAR;
    aMap[SQL_LONGVARCHAR]       = DataType::LONGVARCHAR;
    aMap[SQL_WLONGVARCHAR]      = DataType::LONGVARCHAR;

    aMap[SQL_TYPE_DATE]         = DataType::DATE;
    aMap[SQL_DATE]              = DataType::DATE;
    aMap[SQL_TYPE_TIME]         = DataType::TIME;
    aMap[SQL_TIME]              = DataType::TIME;
    aMap[SQL_TYPE_TIMESTAMP]    = DataType::TIMESTAMP;
    aMap[SQL_TIMESTAMP]         = DataType::TIMESTAMP;

    aMap[SQL_DECIMAL]           = DataType::DECIMAL;
    aMap[SQL_NUMERIC]           = DataType::NUMERIC;

    aMap[SQL_BINARY]            = DataType::BINARY;
    aMap[SQL_VARBINARY]         = DataType::VARBINARY;
    aMap[SQL_LONGVARBINARY]     = DataType::LONGVARBINARY;


    m_aValueRange[2] = aMap;

    OTools::ThrowException(N3SQLGetTypeInfo(m_aStatementHandle, SQL_ALL_TYPES),m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
//-----------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openTables(const Any& catalog, const ::rtl::OUString& schemaPattern,
                            const ::rtl::OUString& tableNamePattern,
                            const Sequence< ::rtl::OUString >& types )  throw(SQLException, RuntimeException)
{
    m_bFreeHandle = sal_True;
    ::rtl::OString aPKQ,aPKO,aPKN,aCOL;
    const ::rtl::OUString *pSchemaPat = NULL;

    if(schemaPattern.toChar() != '%')
        pSchemaPat = &schemaPattern;
    else
        pSchemaPat = NULL;

    aPKQ = ::rtl::OUStringToOString(connectivity::getString(catalog),m_nTextEncoding);
    aPKO = ::rtl::OUStringToOString(schemaPattern,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && aPKQ.getLength() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && pSchemaPat->getLength() ? aPKO.getStr() : NULL,
                *pPKN = aPKN = ::rtl::OUStringToOString(tableNamePattern,m_nTextEncoding).getStr();


    const char  *pCOL = NULL;
    const ::rtl::OUString* pBegin = types.getConstArray();
    const ::rtl::OUString* pEnd = pBegin + types.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        aCOL += ::rtl::OUStringToOString(*pBegin,m_nTextEncoding);
        aCOL += ",";
    }
    if(aCOL.getLength())
        pCOL = aCOL.getStr();

    SQLRETURN nRetcode = N3SQLTables(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && aPKQ.getLength()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS,
                            (SDB_ODBC_CHAR *) pCOL, pCOL ? SQL_NTS : 0);
    OTools::ThrowException(nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);

}
//-----------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openTablesTypes( ) throw(SQLException, RuntimeException)
{
    m_bFreeHandle = sal_True;
    SQLRETURN nRetcode = N3SQLTables(m_aStatementHandle,
                            0,0,
                            0,0,
                            0,0,
                            (SDB_ODBC_CHAR *) SQL_ALL_TABLE_TYPES,SQL_NTS);
    OTools::ThrowException(nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);

    m_aColMapping.clear();
    m_aColMapping.push_back(-1);
    m_aColMapping.push_back(4);
    m_xMetaData = new OResultSetMetaData(m_aStatementHandle,m_aColMapping);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openCatalogs() throw(SQLException, RuntimeException)
{
    m_bFreeHandle = sal_True;
    SQLRETURN nRetcode = N3SQLTables(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) SQL_ALL_CATALOGS,SQL_NTS,
                            0,0,
                            0,0,
                            (SDB_ODBC_CHAR *) "",SQL_NTS);

    OTools::ThrowException(nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);

    m_aColMapping.clear();
    m_aColMapping.push_back(-1);
    m_aColMapping.push_back(1);
    m_xMetaData = new OResultSetMetaData(m_aStatementHandle,m_aColMapping);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openSchemas() throw(SQLException, RuntimeException)
{
    m_bFreeHandle = sal_True;
    SQLRETURN nRetcode = N3SQLTables(m_aStatementHandle,
                            0,0,
                            (SDB_ODBC_CHAR *) SQL_ALL_SCHEMAS,SQL_NTS,
                            0,0,
                            (SDB_ODBC_CHAR *) "",SQL_NTS);
    OTools::ThrowException(nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);

    m_aColMapping.clear();
    m_aColMapping.push_back(-1);
    m_aColMapping.push_back(2);
    m_xMetaData = new OResultSetMetaData(m_aStatementHandle,m_aColMapping);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openColumnPrivileges(  const Any& catalog, const ::rtl::OUString& schema,
                                        const ::rtl::OUString& table,   const ::rtl::OUString& columnNamePattern )
                                        throw(SQLException, RuntimeException)
{
    const ::rtl::OUString *pSchemaPat = NULL;

    if(schema.toChar() != '%')
        pSchemaPat = &schema;
    else
        pSchemaPat = NULL;

    m_bFreeHandle = sal_True;
    ::rtl::OString aPKQ,aPKO,aPKN,aCOL;

    aPKQ = ::rtl::OUStringToOString(connectivity::getString(catalog),m_nTextEncoding);
    aPKO = ::rtl::OUStringToOString(schema,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && aPKQ.getLength() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && pSchemaPat->getLength() ? aPKO.getStr() : NULL,
                *pPKN = aPKN = ::rtl::OUStringToOString(table,m_nTextEncoding).getStr(),
                *pCOL = aCOL = ::rtl::OUStringToOString(columnNamePattern,m_nTextEncoding).getStr();


    SQLRETURN nRetcode = N3SQLColumnPrivileges(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && aPKQ.getLength()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0 ,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS,
                            (SDB_ODBC_CHAR *) pCOL, SQL_NTS);
    OTools::ThrowException(nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);


}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openColumns(   const Any& catalog,             const ::rtl::OUString& schemaPattern,
                                const ::rtl::OUString& tableNamePattern,    const ::rtl::OUString& columnNamePattern )
                                throw(SQLException, RuntimeException)
{
    const ::rtl::OUString *pSchemaPat = NULL;

    if(schemaPattern.toChar() != '%')
        pSchemaPat = &schemaPattern;
    else
        pSchemaPat = NULL;

    m_bFreeHandle = sal_True;
    ::rtl::OString aPKQ,aPKO,aPKN,aCOL;
    aPKQ = ::rtl::OUStringToOString(connectivity::getString(catalog),m_nTextEncoding);
    aPKO = ::rtl::OUStringToOString(schemaPattern,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && aPKQ.getLength() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && pSchemaPat->getLength() && pSchemaPat->getLength() ? aPKO.getStr() : NULL,
                *pPKN = aPKN = ::rtl::OUStringToOString(tableNamePattern,m_nTextEncoding).getStr(),
                *pCOL = aCOL = ::rtl::OUStringToOString(columnNamePattern,m_nTextEncoding).getStr();


    SQLRETURN nRetcode = N3SQLColumns(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && aPKQ.getLength()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS,
                            (SDB_ODBC_CHAR *) pCOL, SQL_NTS);

    OTools::ThrowException(nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    ::std::map<sal_Int32,sal_Int32> aMap;
    aMap[SQL_BIT]               = DataType::BIT;
    aMap[SQL_TINYINT]           = DataType::TINYINT;
    aMap[SQL_SMALLINT]          = DataType::SMALLINT;
    aMap[SQL_INTEGER]           = DataType::INTEGER;
    aMap[SQL_FLOAT]             = DataType::FLOAT;
    aMap[SQL_REAL]              = DataType::REAL;
    aMap[SQL_DOUBLE]            = DataType::DOUBLE;
    aMap[SQL_BIGINT]            = DataType::BIGINT;

    aMap[SQL_CHAR]              = DataType::CHAR;
    aMap[SQL_WCHAR]             = DataType::CHAR;
    aMap[SQL_VARCHAR]           = DataType::VARCHAR;
    aMap[SQL_WVARCHAR]          = DataType::VARCHAR;
    aMap[SQL_LONGVARCHAR]       = DataType::LONGVARCHAR;
    aMap[SQL_WLONGVARCHAR]      = DataType::LONGVARCHAR;

    aMap[SQL_TYPE_DATE]         = DataType::DATE;
    aMap[SQL_DATE]              = DataType::DATE;
    aMap[SQL_TYPE_TIME]         = DataType::TIME;
    aMap[SQL_TIME]              = DataType::TIME;
    aMap[SQL_TYPE_TIMESTAMP]    = DataType::TIMESTAMP;
    aMap[SQL_TIMESTAMP]         = DataType::TIMESTAMP;

    aMap[SQL_DECIMAL]           = DataType::DECIMAL;
    aMap[SQL_NUMERIC]           = DataType::NUMERIC;

    aMap[SQL_BINARY]            = DataType::BINARY;
    aMap[SQL_VARBINARY]         = DataType::VARBINARY;
    aMap[SQL_LONGVARBINARY]     = DataType::LONGVARBINARY;

    m_aValueRange[5] = aMap;
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openProcedureColumns(  const Any& catalog,     const ::rtl::OUString& schemaPattern,
                                const ::rtl::OUString& procedureNamePattern,const ::rtl::OUString& columnNamePattern )
                                throw(SQLException, RuntimeException)
{
    const ::rtl::OUString *pSchemaPat = NULL;

    if(schemaPattern.toChar() != '%')
        pSchemaPat = &schemaPattern;
    else
        pSchemaPat = NULL;

    m_bFreeHandle = sal_True;
    ::rtl::OString aPKQ,aPKO,aPKN,aCOL;
    aPKQ = ::rtl::OUStringToOString(connectivity::getString(catalog),m_nTextEncoding);
    aPKO = ::rtl::OUStringToOString(schemaPattern,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && aPKQ.getLength() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && pSchemaPat->getLength() ? aPKO.getStr() : NULL,
                *pPKN = aPKN = ::rtl::OUStringToOString(procedureNamePattern,m_nTextEncoding).getStr(),
                *pCOL = aCOL = ::rtl::OUStringToOString(columnNamePattern,m_nTextEncoding).getStr();


    SQLRETURN nRetcode = N3SQLProcedureColumns(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && aPKQ.getLength()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0 ,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS,
                            (SDB_ODBC_CHAR *) pCOL, SQL_NTS);

    OTools::ThrowException(nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openProcedures(const Any& catalog, const ::rtl::OUString& schemaPattern,
                                const ::rtl::OUString& procedureNamePattern)
                                throw(SQLException, RuntimeException)
{
    const ::rtl::OUString *pSchemaPat = NULL;

    if(schemaPattern.toChar() != '%')
        pSchemaPat = &schemaPattern;
    else
        pSchemaPat = NULL;

    m_bFreeHandle = sal_True;
    ::rtl::OString aPKQ,aPKO,aPKN,aCOL;

    aPKQ = ::rtl::OUStringToOString(connectivity::getString(catalog),m_nTextEncoding);
    aPKO = ::rtl::OUStringToOString(schemaPattern,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && aPKQ.getLength() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && pSchemaPat->getLength() ? aPKO.getStr() : NULL,
                *pPKN = aPKN = ::rtl::OUStringToOString(procedureNamePattern,m_nTextEncoding).getStr();


    SQLRETURN nRetcode = N3SQLProcedures(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && aPKQ.getLength()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0 ,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS);
    OTools::ThrowException(nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openSpecialColumns(sal_Bool _bRowVer,const Any& catalog, const ::rtl::OUString& schema,
                                    const ::rtl::OUString& table,sal_Int32 scope,   sal_Bool nullable )
                                    throw(SQLException, RuntimeException)
{
    const ::rtl::OUString *pSchemaPat = NULL;

    if(schema.toChar() != '%')
        pSchemaPat = &schema;
    else
        pSchemaPat = NULL;

    m_bFreeHandle = sal_True;
    ::rtl::OString aPKQ,aPKO,aPKN,aCOL;
    aPKQ = ::rtl::OUStringToOString(connectivity::getString(catalog),m_nTextEncoding);
    aPKO = ::rtl::OUStringToOString(schema,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && aPKQ.getLength() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && pSchemaPat->getLength() ? aPKO.getStr() : NULL,
                *pPKN = aPKN = ::rtl::OUStringToOString(table,m_nTextEncoding).getStr();


    SQLRETURN nRetcode = N3SQLSpecialColumns(m_aStatementHandle,_bRowVer ? SQL_ROWVER : SQL_BEST_ROWID,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && aPKQ.getLength()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0 ,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS,
                            scope,
                            nullable ? SQL_NULLABLE : SQL_NO_NULLS);
    OTools::ThrowException(nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openVersionColumns(const Any& catalog, const ::rtl::OUString& schema,
                                    const ::rtl::OUString& table)  throw(SQLException, RuntimeException)
{
    openSpecialColumns(sal_True,catalog,schema,table,SQL_SCOPE_TRANSACTION,sal_False);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openBestRowIdentifier( const Any& catalog, const ::rtl::OUString& schema,
                                        const ::rtl::OUString& table,sal_Int32 scope,sal_Bool nullable ) throw(SQLException, RuntimeException)
{
    openSpecialColumns(sal_False,catalog,schema,table,scope,nullable);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openForeignKeys( const Any& catalog, const ::rtl::OUString* schema,
                                  const ::rtl::OUString* table,
                                  const Any& catalog2, const ::rtl::OUString* schema2,
                                  const ::rtl::OUString* table2) throw(SQLException, RuntimeException)
{
    m_bFreeHandle = sal_True;

    ::rtl::OString aPKQ,aPKO,aPKN, aFKQ, aFKO, aFKN;
    aPKQ = ::rtl::OUStringToOString(connectivity::getString(catalog),m_nTextEncoding);
    aFKQ = ::rtl::OUStringToOString(connectivity::getString(catalog2),m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && aPKQ.getLength() ? aPKQ.getStr()  : NULL,
                *pPKO = schema && schema->getLength() ? ::rtl::OUStringToOString(*schema,m_nTextEncoding).getStr() : NULL,
                *pPKN = table   ? (aPKN = ::rtl::OUStringToOString(*table,m_nTextEncoding)).getStr(): NULL,
                *pFKQ = catalog2.hasValue() && aFKQ.getLength() ? aFKQ.getStr() : NULL,
                *pFKO = schema2 && schema2->getLength() ? (aFKO = ::rtl::OUStringToOString(*schema2,m_nTextEncoding)).getStr() : NULL,
                *pFKN = table2  ? (aFKN = ::rtl::OUStringToOString(*table2,m_nTextEncoding)).getStr() : NULL;


    SQLRETURN nRetcode = N3SQLForeignKeys(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && aPKQ.getLength()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKN, pPKN ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pFKQ, (catalog2.hasValue() && aFKQ.getLength()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pFKO, pFKO ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pFKN, SQL_NTS
                            );
    OTools::ThrowException(nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openImportedKeys(const Any& catalog, const ::rtl::OUString& schema,
                                  const ::rtl::OUString& table) throw(SQLException, RuntimeException)
{

    openForeignKeys(Any(),NULL,NULL,catalog,!schema.compareToAscii("%") ? &schema : NULL,&table);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openExportedKeys(const Any& catalog, const ::rtl::OUString& schema,
                                  const ::rtl::OUString& table) throw(SQLException, RuntimeException)
{
    openForeignKeys(catalog,!schema.compareToAscii("%") ? &schema : NULL,&table,Any(),NULL,NULL);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openPrimaryKeys(const Any& catalog, const ::rtl::OUString& schema,
                                  const ::rtl::OUString& table) throw(SQLException, RuntimeException)
{
    const ::rtl::OUString *pSchemaPat = NULL;

    if(schema.toChar() != '%')
        pSchemaPat = &schema;
    else
        pSchemaPat = NULL;

    m_bFreeHandle = sal_True;
    ::rtl::OString aPKQ,aPKO,aPKN,aCOL;

    aPKQ = ::rtl::OUStringToOString(connectivity::getString(catalog),m_nTextEncoding);
    aPKO = ::rtl::OUStringToOString(schema,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && aPKQ.getLength() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && pSchemaPat->getLength() ? aPKO.getStr() : NULL,
                *pPKN = (aPKN = ::rtl::OUStringToOString(table,m_nTextEncoding)).getStr();


    SQLRETURN nRetcode = N3SQLPrimaryKeys(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && aPKQ.getLength()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0 ,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS);
    OTools::ThrowException(nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openTablePrivileges(const Any& catalog, const ::rtl::OUString& schemaPattern,
                                  const ::rtl::OUString& tableNamePattern) throw(SQLException, RuntimeException)
{
    const ::rtl::OUString *pSchemaPat = NULL;

    if(schemaPattern.toChar() != '%')
        pSchemaPat = &schemaPattern;
    else
        pSchemaPat = NULL;

    m_bFreeHandle = sal_True;
    ::rtl::OString aPKQ,aPKO,aPKN;

    aPKQ = ::rtl::OUStringToOString(connectivity::getString(catalog),m_nTextEncoding);
    aPKO = ::rtl::OUStringToOString(schemaPattern,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && aPKQ.getLength() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && pSchemaPat->getLength() ? aPKO.getStr() : NULL,
                *pPKN = (aPKN = ::rtl::OUStringToOString(tableNamePattern,m_nTextEncoding)).getStr();


    SQLRETURN nRetcode = N3SQLTablePrivileges(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && aPKQ.getLength()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0 ,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS);
    OTools::ThrowException(nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------
void ODatabaseMetaDataResultSet::openIndexInfo( const Any& catalog, const ::rtl::OUString& schema,
                                const ::rtl::OUString& table,sal_Bool unique,sal_Bool approximate )
                                throw(SQLException, RuntimeException)
{
    const ::rtl::OUString *pSchemaPat = NULL;

    if(schema.toChar() != '%')
        pSchemaPat = &schema;
    else
        pSchemaPat = NULL;

    m_bFreeHandle = sal_True;
    ::rtl::OString aPKQ,aPKO,aPKN;

    aPKQ = ::rtl::OUStringToOString(connectivity::getString(catalog),m_nTextEncoding);
    aPKO = ::rtl::OUStringToOString(schema,m_nTextEncoding);

    const char  *pPKQ = catalog.hasValue() && aPKQ.getLength() ? aPKQ.getStr()  : NULL,
                *pPKO = pSchemaPat && pSchemaPat->getLength() ? aPKO.getStr() : NULL,
                *pPKN = (aPKN = ::rtl::OUStringToOString(table,m_nTextEncoding)).getStr();


    SQLRETURN nRetcode = N3SQLStatistics(m_aStatementHandle,
                            (SDB_ODBC_CHAR *) pPKQ, (catalog.hasValue() && aPKQ.getLength()) ? SQL_NTS : 0,
                            (SDB_ODBC_CHAR *) pPKO, pPKO ? SQL_NTS : 0 ,
                            (SDB_ODBC_CHAR *) pPKN, SQL_NTS,
                            unique,
                            approximate);
    OTools::ThrowException(nRetcode,m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------



