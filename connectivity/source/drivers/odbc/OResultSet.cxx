/*************************************************************************
 *
 *  $RCSfile: OResultSet.cxx,v $
 *
 *  $Revision: 1.38 $
 *
 *  last change: $Author: oj $ $Date: 2001-08-06 10:58:36 $
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
#ifndef _CONNECTIVITY_ODBC_ORESULTSET_HXX_
#include "odbc/OResultSet.hxx"
#endif
#ifndef _CONNECTIVITY_OTOOLS_HXX_
#include "odbc/OTools.hxx"
#endif
#ifndef _CONNECTIVITY_ODBC_ORESULTSETMETADATA_HXX_
#include "odbc/OResultSetMetaData.hxx"
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBCX_COMPAREBOOKMARK_HPP_
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif

#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include "connectivity/dbtools.hxx"
#endif
#ifndef _DBHELPER_DBEXCEPTION_HXX_
#include "connectivity/dbexception.hxx"
#endif

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::odbc;
using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

//------------------------------------------------------------------------------
//  IMPLEMENT_SERVICE_INFO(OResultSet,"com.sun.star.sdbcx.OResultSet","com.sun.star.sdbc.ResultSet");
::rtl::OUString SAL_CALL OResultSet::getImplementationName(  ) throw ( RuntimeException)    \
{
    return ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.odbc.ResultSet");
}
// -------------------------------------------------------------------------
 Sequence< ::rtl::OUString > SAL_CALL OResultSet::getSupportedServiceNames(  ) throw( RuntimeException)
{
     Sequence< ::rtl::OUString > aSupported(2);
    aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.ResultSet");
    aSupported[1] = ::rtl::OUString::createFromAscii("com.sun.star.sdbcx.ResultSet");
    return aSupported;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::supportsService( const ::rtl::OUString& _rServiceName ) throw( RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

// -------------------------------------------------------------------------
OResultSet::OResultSet(SQLHANDLE _pStatementHandle ,OStatement_Base* pStmt) :   OResultSet_BASE(m_aMutex)
                        ,OPropertySetHelper(OResultSet_BASE::rBHelper)
                        ,m_aStatement((OWeakObject*)pStmt)
                        ,m_aStatementHandle(_pStatementHandle)
                        ,m_aConnectionHandle(pStmt->getConnectionHandle())
                        ,m_nRowPos(0)
                        ,m_bLastRecord(sal_False)
                        ,m_bEOF(sal_False)
                        ,m_bFreeHandle(sal_False)
                        ,m_xMetaData(NULL)
                        ,m_bInserting(sal_False)
                        ,m_nLastColumnPos(0)
                        ,m_nTextEncoding(pStmt->getOwnConnection()->getTextEncoding())
                        ,m_pStatement(pStmt)
{
    osl_incrementInterlockedCount( &m_refCount );
    m_pRowStatusArray = new SQLUSMALLINT[1]; // the default value
    N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_ROW_STATUS_PTR,m_pRowStatusArray,SQL_IS_POINTER);

    sal_Int32 nValueLen = 0;
    OTools::GetInfo(m_pStatement->getOwnConnection(),m_aConnectionHandle,SQL_GETDATA_EXTENSIONS,nValueLen,*(Reference< XInterface >*)this);

    SQLINTEGER nCurType = 0;
    N3SQLGetStmtAttr(m_aStatementHandle,SQL_ATTR_CURSOR_TYPE,&nCurType,SQL_IS_UINTEGER,0);

    m_bFetchData = !((SQL_GD_ANY_ORDER & nValueLen) == SQL_GD_ANY_ORDER && nCurType != SQL_CURSOR_FORWARD_ONLY);

    osl_decrementInterlockedCount( &m_refCount );
}
// -------------------------------------------------------------------------
OResultSet::~OResultSet()
{
    delete m_pRowStatusArray;
}
// -----------------------------------------------------------------------------
void OResultSet::construct()
{
    osl_incrementInterlockedCount( &m_refCount );
    allocBuffer();
    osl_decrementInterlockedCount( &m_refCount );
}
// -------------------------------------------------------------------------
void OResultSet::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);
    if(m_aBindVector.size())
        releaseBuffer();
    if(m_bFreeHandle)
    {
        N3SQLFreeStmt(m_aStatementHandle,SQL_CLOSE);
        N3SQLFreeHandle(SQL_HANDLE_STMT,m_aStatementHandle);
        m_aStatementHandle = NULL;
    }

    m_aStatement    = NULL;
    m_xMetaData     = NULL;
}
// -----------------------------------------------------------------------------
sal_Int32 OResultSet::mapColumn (sal_Int32  column)
{
    sal_Int32   map = column;

    if (m_aColMapping.size())
    {
        // Validate column number
        OSL_ENSURE(column>0,"OResultSet::mapColumn column <= 0");
        map = m_aColMapping[column];
    }

    return map;
}
// -------------------------------------------------------------------------
void OResultSet::allocBuffer()
{
    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();

    m_aBindVector.reserve(nLen+1);
    m_aBindVector.push_back(NULL); // the first is reserved for the bookmark
    m_aRow.resize(nLen+1);

    for(sal_Int32 i = 1;i<=nLen;++i)
    {
        m_aRow[i].setTypeKind( xMeta->getColumnType(i) );
        switch (m_aRow[i].getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
                m_aBindVector.push_back((sal_Int64)new ::rtl::OString());
                break;
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::BIGINT:
                m_aBindVector.push_back((sal_Int64)new ::rtl::OString());
                break;
            case DataType::REAL:
            case DataType::DOUBLE:
                m_aBindVector.push_back((sal_Int64)new double(0.0));
                break;
            case DataType::LONGVARCHAR:
                m_aBindVector.push_back((sal_Int64)new char[2]);  // dient nur zum auffinden
                break;
            case DataType::LONGVARBINARY:
                m_aBindVector.push_back((sal_Int64)new char[2]);  // dient nur zum auffinden
                break;
            case DataType::DATE:
                m_aBindVector.push_back((sal_Int64)new DATE_STRUCT);
                break;
            case DataType::TIME:
                m_aBindVector.push_back((sal_Int64)new TIME_STRUCT);
                break;
            case DataType::TIMESTAMP:
                m_aBindVector.push_back((sal_Int64)new TIMESTAMP_STRUCT);
                break;
            case DataType::BIT:
                m_aBindVector.push_back((sal_Int64)new sal_Int8(0));
                break;
            case DataType::TINYINT:
            case DataType::SMALLINT:
                m_aBindVector.push_back((sal_Int64)new sal_Int16(0));
                break;
            case DataType::INTEGER:
                m_aBindVector.push_back((sal_Int64)new sal_Int32(0));
                break;
            case DataType::FLOAT:
                m_aBindVector.push_back((sal_Int64)new float(0));
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
                m_aBindVector.push_back((sal_Int64)new sal_Int8[xMeta->getPrecision(i)]);
                break;
            default:
                OSL_ENSURE(0,"Unknown type");
                m_aBindVector.push_back(NULL);
        }
    }
    m_aLengthVector.resize(nLen + 1);
}
// -------------------------------------------------------------------------
void OResultSet::releaseBuffer()
{
    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    TVoidVector::iterator pValue = m_aBindVector.begin() + 1;
    for(sal_Int32 i = 1; i<=nLen; ++i, ++pValue)
    {
        OSL_ENSURE(pValue != m_aBindVector.end(),"Iterator is equal end!");
        switch (xMeta->getColumnType(i))
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
                delete static_cast< ::rtl::OString* >((void*)*pValue);
                break;
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::BIGINT:
                delete static_cast< ::rtl::OString* >((void*)*pValue);
                break;
            case DataType::REAL:
            case DataType::DOUBLE:
                delete static_cast< double* >((void*)*pValue);
                break;
            case DataType::LONGVARCHAR:
                delete static_cast< char* >((void*)*pValue);
                break;
            case DataType::LONGVARBINARY:
                delete static_cast< char* >((void*)*pValue);
                break;
            case DataType::DATE:
                delete static_cast< DATE_STRUCT* >((void*)*pValue);
                break;
            case DataType::TIME:
                delete static_cast< TIME_STRUCT* >((void*)*pValue);
                break;
            case DataType::TIMESTAMP:
                delete static_cast< TIMESTAMP_STRUCT* >((void*)*pValue);
                break;
            case DataType::BIT:
                delete static_cast< sal_Int8* >((void*)*pValue);
                break;
            case DataType::TINYINT:
            case DataType::SMALLINT:
                delete static_cast< sal_Int16* >((void*)*pValue);
                break;
            case DataType::INTEGER:
                delete static_cast< sal_Int32* >((void*)*pValue);
                break;
            case DataType::FLOAT:
                delete static_cast< float* >((void*)*pValue);
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
                delete static_cast< sal_Int8* >((void*)*pValue);
                break;
        }
    }
    m_aLengthVector.clear();
}
// -------------------------------------------------------------------------
Any SAL_CALL OResultSet::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OResultSet_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
 Sequence<  Type > SAL_CALL OResultSet::getTypes(  ) throw( RuntimeException)
{
    OTypeCollection aTypes( ::getCppuType( (const  Reference< ::com::sun::star::beans::XMultiPropertySet > *)0 ),
                                                ::getCppuType( (const  Reference< ::com::sun::star::beans::XFastPropertySet > *)0 ),
                                                ::getCppuType( (const  Reference< ::com::sun::star::beans::XPropertySet > *)0 ));

    return ::comphelper::concatSequences(aTypes.getTypes(),OResultSet_BASE::getTypes());
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::findColumn( const ::rtl::OUString& columnName ) throw(SQLException, RuntimeException)
{

    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    ::osl::MutexGuard aGuard( m_aMutex );

    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    sal_Int32 i = 1;
    for(;i<=nLen;++i)
        if(xMeta->isCaseSensitive(i) ? columnName == xMeta->getColumnName(i) :
#if SUPD > 631
                columnName.equalsIgnoreAsciiCase(xMeta->getColumnName(i)))
#else
                columnName.equalsIgnoreCase(xMeta->getColumnName(i)))
#endif
            break;
    return i;
}
// -------------------------------------------------------------------------
Reference< XInputStream > SAL_CALL OResultSet::getBinaryStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    columnIndex = mapColumn(columnIndex);
    // TODO use getBytes instead of
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XInputStream > SAL_CALL OResultSet::getCharacterStream( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    columnIndex = mapColumn(columnIndex);
    // TODO use getBytes instead of
    return NULL;
}
// -----------------------------------------------------------------------------
const ORowSetValue& OResultSet::getValue(sal_Int32 _nColumnIndex,SQLSMALLINT _nType,void* _pValue,SQLINTEGER _rSize)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    _nColumnIndex = mapColumn(_nColumnIndex);

    if(m_bFetchData)
    {
        if(_nColumnIndex > m_nLastColumnPos)
            fillRow(_nColumnIndex);
        return m_aRow[_nColumnIndex];
    }
    else
        OTools::getValue(m_pStatement->getOwnConnection(),m_aStatementHandle,_nColumnIndex,_nType,m_bWasNull,**this,_pValue,_rSize);

    return m_aEmptyValue;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::getBoolean( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    sal_Int8 nVal(0);
    const ORowSetValue& aValue = getValue(columnIndex,SQL_C_BIT,&nVal,sizeof nVal);
    return (&aValue == &m_aEmptyValue) ? (sal_Bool)nVal : (sal_Bool)aValue;
}
// -------------------------------------------------------------------------

sal_Int8 SAL_CALL OResultSet::getByte( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    sal_Int8 nRet(0);
    const ORowSetValue& aValue = getValue(columnIndex,SQL_C_CHAR,&nRet,sizeof nRet);
    return (&aValue == &m_aEmptyValue) ? nRet : (sal_Int8)aValue;
}
// -------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL OResultSet::getBytes( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{

    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );


    columnIndex = mapColumn(columnIndex);
    if(m_bFetchData)
    {
        if(columnIndex > m_nLastColumnPos)
            fillRow(columnIndex);
        Sequence< sal_Int8 > nRet;
        switch(m_aRow[columnIndex].getTypeKind())
        {
            case DataType::BINARY:
            case DataType::VARBINARY:
            case DataType::LONGVARBINARY:
                nRet = m_aRow[columnIndex];
                break;
            default:
            {
                ::rtl::OUString sRet;
                sRet = m_aRow[columnIndex].getString();
                nRet = Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>(sRet.getStr()),sizeof(sal_Unicode)*sRet.getLength());
            }
        }
        return nRet;
    }

    sal_Int32 nType = getMetaData()->getColumnType(columnIndex);
    switch(nType)
    {
        case DataType::VARCHAR:
        case DataType::LONGVARCHAR:
            {
                ::rtl::OUString aRet = OTools::getStringValue(m_pStatement->getOwnConnection(),m_aStatementHandle,columnIndex,(SWORD)nType,m_bWasNull,**this,m_nTextEncoding);
                return Sequence<sal_Int8>(reinterpret_cast<const sal_Int8*>(aRet.getStr()),sizeof(sal_Unicode)*aRet.getLength());
            }
            break;
    }
    return OTools::getBytesValue(m_pStatement->getOwnConnection(),m_aStatementHandle,columnIndex,(SWORD)nType,m_bWasNull,**this);
}
// -------------------------------------------------------------------------

Date SAL_CALL OResultSet::getDate( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    DATE_STRUCT aDate;
    aDate.day   = 0;
    aDate.month = 0;
    aDate.year  = 0;

    const ORowSetValue& aValue = getValue(columnIndex,SQL_C_DATE,&aDate,sizeof aDate);
    return (&aValue == &m_aEmptyValue)  ? Date(aDate.day,aDate.month,aDate.year) : (Date)aValue;
}
// -------------------------------------------------------------------------

double SAL_CALL OResultSet::getDouble( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    double nRet(0);
    const ORowSetValue& aValue = getValue(columnIndex,SQL_C_DOUBLE,&nRet,sizeof nRet);
    return (&aValue == &m_aEmptyValue) ? nRet : (double)aValue;
}
// -------------------------------------------------------------------------

float SAL_CALL OResultSet::getFloat( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    float nRet(0);
    const ORowSetValue& aValue = getValue(columnIndex,SQL_C_FLOAT,&nRet,sizeof nRet);
    return (&aValue == &m_aEmptyValue) ? nRet : (float)aValue;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::getInt( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    sal_Int32 nRet(0);
    const ORowSetValue& aValue = getValue(columnIndex,SQL_C_LONG,&nRet,sizeof nRet);
    return (&aValue == &m_aEmptyValue) ? nRet : (sal_Int32)aValue;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::getRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    sal_Int32 nValue = 0;
    OTools::ThrowException(m_pStatement->getOwnConnection(),N3SQLGetStmtAttr(m_aStatementHandle,SQL_ATTR_ROW_NUMBER,&nValue,SQL_IS_UINTEGER,0),m_aStatementHandle,SQL_HANDLE_STMT,*this);
    if(!nValue) // some driver dosen't support this
        m_nRowPos = nValue;
    return nValue;
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL OResultSet::getLong( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return sal_Int64();
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(m_pStatement->getOwnConnection(),m_aStatementHandle);
    return m_xMetaData;
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL OResultSet::getArray( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return NULL;
}

// -------------------------------------------------------------------------

Reference< XClob > SAL_CALL OResultSet::getClob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL OResultSet::getBlob( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL OResultSet::getRef( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------

Any SAL_CALL OResultSet::getObject( sal_Int32 columnIndex, const Reference< ::com::sun::star::container::XNameAccess >& typeMap ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    columnIndex = mapColumn(columnIndex);
    fillRow(columnIndex);
    return m_aRow[columnIndex].makeAny();
}
// -------------------------------------------------------------------------

sal_Int16 SAL_CALL OResultSet::getShort( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    sal_Int16 nRet(0);
    const ORowSetValue& aValue = getValue(columnIndex,SQL_C_SHORT,&nRet,sizeof nRet);
    return (&aValue == &m_aEmptyValue) ? nRet : (sal_Int16)aValue;
}
// -------------------------------------------------------------------------


::rtl::OUString SAL_CALL OResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    columnIndex = mapColumn(columnIndex);
    ::rtl::OUString nRet;
    if(m_bFetchData)
        nRet = getValue(columnIndex,0,NULL,0);
    else
        nRet = OTools::getStringValue(m_pStatement->getOwnConnection(),m_aStatementHandle,columnIndex,(SWORD)getMetaData()->getColumnType(columnIndex),m_bWasNull,**this,m_nTextEncoding);
    return nRet;
}
// -------------------------------------------------------------------------

Time SAL_CALL OResultSet::getTime( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    TIME_STRUCT aTime={0,0,0};
    const ORowSetValue& aValue = getValue(columnIndex,SQL_C_TIME,&aTime,sizeof aTime);
    return (&aValue == &m_aEmptyValue) ? Time(0,aTime.second,aTime.minute,aTime.hour) : (Time)aValue;
}
// -------------------------------------------------------------------------


DateTime SAL_CALL OResultSet::getTimestamp( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    TIMESTAMP_STRUCT aTime={0,0,0,0,0,0,0};
    const ORowSetValue& aValue = getValue(columnIndex,SQL_C_TIMESTAMP,&aTime,sizeof aTime);
    return (&aValue == &m_aEmptyValue)
            ?
            DateTime(aTime.fraction*1000,aTime.second,aTime.minute,aTime.hour,aTime.day,aTime.month,aTime.year)
            :
            (DateTime)aValue;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos == 0;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos != 0 && m_nCurrentFetchState == SQL_NO_DATA;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_nRowPos == 1;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_bEOF && m_nCurrentFetchState != SQL_NO_DATA;
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    if(first())
        previous();
    m_nCurrentFetchState = SQL_SUCCESS;
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_nLastColumnPos = 0;
    if(last())
        next();
    m_bEOF = sal_True;
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::close(  ) throw(SQLException, RuntimeException)
{
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        checkDisposed(OResultSet_BASE::rBHelper.bDisposed);

    }
    dispose();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::first(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_nLastColumnPos = 0;
    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_FIRST,0);
    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    sal_Bool bRet;
    if(bRet = (m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO))
        m_nRowPos = 1;
    return bRet;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::last(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_nLastColumnPos = 0;
    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_LAST,0);
    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    // here I know definitely that I stand on the last record
    return m_bLastRecord = (m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_nLastColumnPos = 0;
    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_ABSOLUTE,row);
    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    sal_Bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
        m_nRowPos = row;
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_nLastColumnPos = 0;
    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_RELATIVE,row);
    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    sal_Bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet)
        m_nRowPos += row;
    return bRet;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::previous(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_nLastColumnPos = 0;
    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_PRIOR,0);
    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    sal_Bool bRet = m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
    if(bRet || m_nCurrentFetchState == SQL_NO_DATA)
        --m_nRowPos;
    return bRet;
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OResultSet::getStatement(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_aStatement.get();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_pRowStatusArray[0] == SQL_ROW_DELETED;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_pRowStatusArray[0] == SQL_ROW_ADDED;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_pRowStatusArray[0] == SQL_ROW_UPDATED;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::next(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_nLastColumnPos = 0;
    //  m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_NEXT,0);
    m_nCurrentFetchState = N3SQLFetch(m_aStatementHandle);
    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    if(m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO)
        ++m_nRowPos;
    return m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return m_bFetchData ? m_aRow[m_nLastColumnPos].isNull() : m_bWasNull;
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::cancel(  ) throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    OTools::ThrowException(m_pStatement->getOwnConnection(),N3SQLCancel(m_aStatementHandle),m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::clearWarnings(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------
Any SAL_CALL OResultSet::getWarnings(  ) throw(SQLException, RuntimeException)
{
    return Any();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::insertRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    SQLRETURN nRet;
    if(getOdbcFunction(ODBC3SQLBulkOperations))
        nRet = N3SQLBulkOperations(m_aStatementHandle, SQL_ADD);
    else
    {
        if(isBeforeFirst())
            next(); // must be done
        nRet = N3SQLSetPos(m_aStatementHandle,1,SQL_ADD,SQL_LOCK_NO_CHANGE);
    }
    OTools::ThrowException(m_pStatement->getOwnConnection(),nRet,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    nRet = N3SQLFreeStmt(m_aStatementHandle,SQL_UNBIND);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    SQLRETURN nRet;
    nRet = N3SQLSetPos(m_aStatementHandle,1,SQL_UPDATE,SQL_LOCK_NO_CHANGE);
    if( nRet == SQL_NEED_DATA)
    {
        void * pData = NULL;
        nRet = N3SQLParamData(m_aStatementHandle,&pData);
        do
        {
            if (nRet != SQL_SUCCESS && nRet != SQL_SUCCESS_WITH_INFO && nRet != SQL_NEED_DATA)
                break;

            TVoidVector::const_iterator aFound = ::std::find(m_aBindVector.begin(),m_aBindVector.end(),(sal_Int64)pData);
            sal_Int32 nPos = m_aBindVector.size() - (m_aBindVector.end() - aFound);

            // TODO transfer long data
            // N3SQLPutData(m_aStatementHandle,,);
            nRet = N3SQLParamData(m_aStatementHandle,&pData);
        }
        while (nRet == SQL_NEED_DATA);

    }
    OTools::ThrowException(m_pStatement->getOwnConnection(),nRet,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    // now unbind all columns so we can fetch all columns again with SQLGetData
    nRet = N3SQLFreeStmt(m_aStatementHandle,SQL_UNBIND);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::deleteRow(  ) throw(SQLException, RuntimeException)
{
    SQLRETURN nRet = N3SQLSetPos(m_aStatementHandle,1,SQL_DELETE,SQL_LOCK_NO_CHANGE);
    OTools::ThrowException(m_pStatement->getOwnConnection(),nRet,m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::cancelRowUpdates(  ) throw(SQLException, RuntimeException)
{
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::moveToInsertRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_nLastColumnPos = 0;
    // first unbound all columns
    N3SQLFreeStmt(m_aStatementHandle,SQL_UNBIND);
    //  SQLRETURN nRet = N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_ROW_ARRAY_SIZE ,(SQLPOINTER)1,SQL_IS_INTEGER);
    m_bInserting = sal_True;
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::moveToCurrentRow(  ) throw(SQLException, RuntimeException)
{
    m_nLastColumnPos = 0;
}
// -------------------------------------------------------------------------
void OResultSet::updateValue(sal_Int32 columnIndex,SQLSMALLINT _nType,void* _pValue) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    columnIndex = mapColumn(columnIndex);
    void* pData = (void*)m_aBindVector[columnIndex];
    OTools::bindValue(  m_pStatement->getOwnConnection(),
                        m_aStatementHandle,
                        columnIndex,
                        _nType,
                        0,
                        0,
                        _pValue,
                        pData,
                        &m_aLengthVector[columnIndex],
                        **this,
                        m_nTextEncoding,
                        m_pStatement->getOwnConnection()->useOldDateFormat());
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::updateNull( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    columnIndex = mapColumn(columnIndex);
    OTools::bindValue(m_pStatement->getOwnConnection(),m_aStatementHandle,columnIndex,SQL_CHAR,0,0,(sal_Int8*)NULL,NULL,&m_aLengthVector[columnIndex],**this,m_nTextEncoding,m_pStatement->getOwnConnection()->useOldDateFormat());
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,SQL_BIT,&x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,SQL_CHAR,&x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,SQL_TINYINT,&x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,SQL_INTEGER,&x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateLong( sal_Int32 columnIndex, sal_Int64 x ) throw(SQLException, RuntimeException)
{
    throw SQLException();
}
// -----------------------------------------------------------------------
void SAL_CALL OResultSet::updateFloat( sal_Int32 columnIndex, float x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,SQL_REAL,&x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateDouble( sal_Int32 columnIndex, double x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,SQL_DOUBLE,&x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateString( sal_Int32 columnIndex, const ::rtl::OUString& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,SQL_VARCHAR,(void*)&x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateBytes( sal_Int32 columnIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,SQL_BINARY,(void*)&x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateDate( sal_Int32 columnIndex, const Date& x ) throw(SQLException, RuntimeException)
{
    DATE_STRUCT aVal = OTools::DateToOdbcDate(x);
    updateValue(columnIndex,SQL_DATE,&aVal);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateTime( sal_Int32 columnIndex, const Time& x ) throw(SQLException, RuntimeException)
{
    TIME_STRUCT aVal = OTools::TimeToOdbcTime(x);
    updateValue(columnIndex,SQL_TIME,&aVal);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateTimestamp( sal_Int32 columnIndex, const DateTime& x ) throw(SQLException, RuntimeException)
{
    TIMESTAMP_STRUCT aVal = OTools::DateTimeToTimestamp(x);
    updateValue(columnIndex,SQL_TIMESTAMP,&aVal);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateBinaryStream( sal_Int32 columnIndex, const Reference< XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    if(!x.is())
        ::dbtools::throwFunctionSequenceException(*this);

    Sequence<sal_Int8> aSeq;
    x->readSomeBytes(aSeq,length);
    updateBytes(columnIndex,aSeq);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateCharacterStream( sal_Int32 columnIndex, const Reference< XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    updateBinaryStream(columnIndex,x,length);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    //  SQLRETURN nRet = N3SQLSetPos(m_aStatementHandle,1,SQL_REFRESH,SQL_LOCK_NO_CHANGE);
    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_RELATIVE,0);
    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateObject( sal_Int32 columnIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
        throw SQLException();
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 scale ) throw(SQLException, RuntimeException)
{
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
        throw SQLException();
}
// -------------------------------------------------------------------------
// XRowLocate
Any SAL_CALL OResultSet::getBookmark(  ) throw( SQLException,  RuntimeException)
{
     ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    sal_uInt32 nValue = SQL_UB_OFF;
    N3SQLGetStmtAttr(m_aStatementHandle,SQL_ATTR_USE_BOOKMARKS,&nValue,SQL_IS_UINTEGER,NULL);
    if(nValue == SQL_UB_OFF)
        throw SQLException();


     return makeAny(OTools::getBytesValue(m_pStatement->getOwnConnection(),m_aStatementHandle,0,SQL_BINARY,m_bWasNull,**this));
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::moveToBookmark( const  Any& bookmark ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_nLastColumnPos = 0;
    Sequence<sal_Int8> aBookmark;
    bookmark >>= aBookmark;
    SQLRETURN nReturn = N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_FETCH_BOOKMARK_PTR,aBookmark.getArray(),SQL_IS_POINTER);

    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_BOOKMARK,0);
    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    return m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::moveRelativeToBookmark( const  Any& bookmark, sal_Int32 rows ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    m_nLastColumnPos = 0;
    Sequence<sal_Int8> aBookmark;
    bookmark >>= aBookmark;
    SQLRETURN nReturn = N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_FETCH_BOOKMARK_PTR,aBookmark.getArray(),SQL_IS_POINTER);

    m_nCurrentFetchState = N3SQLFetchScroll(m_aStatementHandle,SQL_FETCH_BOOKMARK,rows);
    OTools::ThrowException(m_pStatement->getOwnConnection(),m_nCurrentFetchState,m_aStatementHandle,SQL_HANDLE_STMT,*this);
    return m_nCurrentFetchState == SQL_SUCCESS || m_nCurrentFetchState == SQL_SUCCESS_WITH_INFO;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSet::compareBookmarks( const  Any& first, const  Any& second ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    return (first == second) ? CompareBookmark::EQUAL : CompareBookmark::NOT_EQUAL;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::hasOrderedBookmarks(  ) throw( SQLException,  RuntimeException)
{
    return sal_False;
}
// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSet::hashBookmark( const  Any& bookmark ) throw( SQLException,  RuntimeException)
{
    throw SQLException();
}
// -------------------------------------------------------------------------
// XDeleteRows
Sequence< sal_Int32 > SAL_CALL OResultSet::deleteRows( const  Sequence<  Any >& rows ) throw( SQLException,  RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);


    SQLRETURN nReturn;

    const Any* pBegin = rows.getConstArray();
    const Any* pEnd = pBegin + rows.getLength();

    typedef sal_Int8* sal_INT8;
    sal_Int8** pArray = new sal_INT8[rows.getLength()];
    for(sal_Int32 i=0;pBegin != pEnd;++i,++pBegin)
    {
        pArray[i] = ((Sequence<sal_Int8>*)pBegin->getValue())->getArray();
    }

    sal_Int32* pStatusArray = new sal_Int32[rows.getLength()];


    nReturn = N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_ROW_STATUS_PTR ,(SQLPOINTER)pStatusArray,SQL_IS_POINTER);
    nReturn = N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_ROW_ARRAY_SIZE ,(SQLPOINTER)rows.getLength(),SQL_IS_INTEGER);
    sal_Int32 nLen = rows.getLength();
    nReturn = N3SQLBindCol(m_aStatementHandle,0,SQL_C_VARBOOKMARK,pArray,rows.getLength(),&nLen);
    nReturn = N3SQLBulkOperations(m_aStatementHandle,SQL_DELETE_BY_BOOKMARK);

    delete [] pArray;

    Sequence< sal_Int32 > aRet(rows.getLength());
    for(sal_Int32 j=0;j<rows.getLength();++j)
    {
        aRet.getArray()[j] = pStatusArray[j] == SQL_ROW_SUCCESS;
    }
    delete pStatusArray;
    delete pArray;
    return aRet;
}
//------------------------------------------------------------------------------
sal_Int32 OResultSet::getResultSetConcurrency() const throw( SQLException,  RuntimeException)
{
    sal_uInt32 nValue;
    N3SQLGetStmtAttr(m_aStatementHandle,SQL_ATTR_CONCURRENCY,&nValue,SQL_IS_UINTEGER,0);
    return nValue;
}
//------------------------------------------------------------------------------
sal_Int32 OResultSet::getResultSetType() const throw( SQLException,  RuntimeException)
{
    sal_uInt32 nValue;
    N3SQLGetStmtAttr(m_aStatementHandle,SQL_ATTR_CURSOR_SENSITIVITY,&nValue,SQL_IS_UINTEGER,0);
    return nValue;
}
//------------------------------------------------------------------------------
sal_Int32 OResultSet::getFetchDirection() const throw( SQLException,  RuntimeException)
{
    sal_uInt32 nValue;
    N3SQLGetStmtAttr(m_aStatementHandle,SQL_ATTR_CURSOR_TYPE,&nValue,SQL_IS_UINTEGER,0);
    return nValue;
}
//------------------------------------------------------------------------------
sal_Int32 OResultSet::getFetchSize() const throw( SQLException,  RuntimeException)
{
    sal_uInt32 nValue;
    N3SQLGetStmtAttr(m_aStatementHandle,SQL_ATTR_ROW_ARRAY_SIZE,&nValue,SQL_IS_UINTEGER,0);
    return nValue;
}
//------------------------------------------------------------------------------
::rtl::OUString OResultSet::getCursorName() const throw( SQLException,  RuntimeException)
{
    SQLCHAR pName[258];
    SQLSMALLINT nRealLen = 0;
    N3SQLGetCursorName(m_aStatementHandle,(SQLCHAR*)pName,256,&nRealLen);
    return ::rtl::OUString::createFromAscii((const char*)pName);
}
// -------------------------------------------------------------------------
sal_Bool  OResultSet::isBookmarkable() const throw( SQLException,  RuntimeException)
{
    if(!m_aConnectionHandle)
        return sal_False;

    sal_uInt32 nValue;
    N3SQLGetStmtAttr(m_aStatementHandle,SQL_ATTR_CURSOR_TYPE,&nValue,SQL_IS_UINTEGER,0);

    sal_Int32 nAttr = 0;
    switch(nValue)
    {
    case SQL_CURSOR_FORWARD_ONLY:
        return sal_False;
        break;
    case SQL_CURSOR_STATIC:
        OTools::GetInfo(m_pStatement->getOwnConnection(),m_aConnectionHandle,SQL_STATIC_CURSOR_ATTRIBUTES1,nAttr,*(Reference< XInterface >*)this);
        break;
    case SQL_CURSOR_KEYSET_DRIVEN:
        OTools::GetInfo(m_pStatement->getOwnConnection(),m_aConnectionHandle,SQL_KEYSET_CURSOR_ATTRIBUTES1,nAttr,*(Reference< XInterface >*)this);
        break;
    case SQL_CURSOR_DYNAMIC:
        OTools::GetInfo(m_pStatement->getOwnConnection(),m_aConnectionHandle,SQL_DYNAMIC_CURSOR_ATTRIBUTES1,nAttr,*(Reference< XInterface >*)this);
        break;
    }
    sal_uInt32 nUseBookmark = SQL_UB_OFF;
    N3SQLGetStmtAttr(m_aStatementHandle,SQL_ATTR_USE_BOOKMARKS,&nUseBookmark,SQL_IS_UINTEGER,NULL);

    return (nUseBookmark != SQL_UB_OFF) && (nAttr & SQL_CA1_BOOKMARK) == SQL_CA1_BOOKMARK;
}
//------------------------------------------------------------------------------
void OResultSet::setFetchDirection(sal_Int32 _par0) throw( SQLException,  RuntimeException)
{
    N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_CURSOR_TYPE,(SQLPOINTER)_par0,SQL_IS_UINTEGER);
}
//------------------------------------------------------------------------------
void OResultSet::setFetchSize(sal_Int32 _par0) throw( SQLException,  RuntimeException)
{
    N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_ROW_ARRAY_SIZE,(SQLPOINTER)_par0,SQL_IS_UINTEGER);
    delete m_pRowStatusArray;
    m_pRowStatusArray = new SQLUSMALLINT[_par0];
    N3SQLSetStmtAttr(m_aStatementHandle,SQL_ATTR_ROW_STATUS_PTR,m_pRowStatusArray,SQL_IS_POINTER);
}
// -------------------------------------------------------------------------
IPropertyArrayHelper* OResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps(6);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    DECL_PROP1IMPL(CURSORNAME,          ::rtl::OUString) PropertyAttribute::READONLY);
    DECL_PROP0(FETCHDIRECTION,          sal_Int32);
    DECL_PROP0(FETCHSIZE,               sal_Int32);
    DECL_BOOL_PROP1IMPL(ISBOOKMARKABLE) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETCONCURRENCY,sal_Int32) PropertyAttribute::READONLY);
    DECL_PROP1IMPL(RESULTSETTYPE,       sal_Int32) PropertyAttribute::READONLY);

    return new OPropertyArrayHelper(aProps);
}
// -------------------------------------------------------------------------
IPropertyArrayHelper & OResultSet::getInfoHelper()
{
    return *const_cast<OResultSet*>(this)->getArrayHelper();
}
// -------------------------------------------------------------------------
sal_Bool OResultSet::convertFastPropertyValue(
                            Any & rConvertedValue,
                            Any & rOldValue,
                            sal_Int32 nHandle,
                            const Any& rValue )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
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
void OResultSet::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& rValue
                                                 )
                                                 throw (Exception)
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_CURSORNAME:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw Exception();
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            setFetchDirection(getINT32(rValue));
            break;
        case PROPERTY_ID_FETCHSIZE:
            setFetchSize(getINT32(rValue));
            break;
        default:
            ;
    }
}
// -------------------------------------------------------------------------
void OResultSet::getFastPropertyValue(
                                Any& rValue,
                                sal_Int32 nHandle
                                     ) const
{
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
            rValue = bool2any(isBookmarkable());
            break;
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
void OResultSet::fillRow(sal_Int32 _nToColumn)
{
    if((sal_Int32)m_aRow.size() <= _nToColumn)
    {
        m_aRow.resize(_nToColumn+1);
        m_aRow[_nToColumn].setBound(sal_True);
    }
    m_bFetchData = sal_False;

    sal_Int32           nColumn     = m_nLastColumnPos + 1;
    TDataRow::iterator pColumn      = m_aRow.begin() + nColumn;
    TDataRow::iterator pColumnEnd   = m_aRow.begin() + _nToColumn + 1;

    for (; pColumn < pColumnEnd; ++nColumn, ++pColumn)
    {
        switch (pColumn->getTypeKind())
        {
            case DataType::CHAR:
            case DataType::VARCHAR:
                *pColumn = getString(nColumn);
                break;
            case DataType::DECIMAL:
            case DataType::NUMERIC:
            case DataType::BIGINT:
                *pColumn = getString(nColumn);
                break;
            case DataType::REAL:
            case DataType::DOUBLE:
                *pColumn = getDouble(nColumn);
                break;
            case DataType::LONGVARCHAR:
                *pColumn = getString(nColumn);
                break;
            case DataType::LONGVARBINARY:
                *pColumn = getBytes(nColumn);
                break;
            case DataType::DATE:
                *pColumn = getDate(nColumn);
                break;
            case DataType::TIME:
                *pColumn = getTime(nColumn);
                break;
            case DataType::TIMESTAMP:
                *pColumn = getTimestamp(nColumn);
                break;
            case DataType::BIT:
                *pColumn = getBoolean(nColumn);
                break;
            case DataType::TINYINT:
                *pColumn = getByte(nColumn);
                break;
            case DataType::SMALLINT:
                *pColumn = getShort(nColumn);
                break;
            case DataType::INTEGER:
                *pColumn = getInt(nColumn);
                break;
            case DataType::FLOAT:
                *pColumn = getFloat(nColumn);
                break;
            case DataType::BINARY:
            case DataType::VARBINARY:
                *pColumn = getBytes(nColumn);
                break;
        }

        if(wasNull())
            pColumn->setNull();
    }
    m_nLastColumnPos = _nToColumn;
    m_bFetchData = sal_True;
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::acquire() throw(::com::sun::star::uno::RuntimeException)
{
    OResultSet_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::release() throw(::com::sun::star::uno::RuntimeException)
{
    OResultSet_BASE::release();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OResultSet::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}
// -----------------------------------------------------------------------------



