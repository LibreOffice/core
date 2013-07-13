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

#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <comphelper/property.hxx>
#include <comphelper/sequence.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/extract.hxx>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <comphelper/types.hxx>
#include <connectivity/dbexception.hxx>
#include <connectivity/dbtools.hxx>

#include <TSortIndex.hxx>
#include <rtl/string.hxx>
#include <vector>
#include <algorithm>
#include "MResultSet.hxx"
#include "MResultSetMetaData.hxx"
#include "FDatabaseMetaDataResultSet.hxx"

#include "resource/mork_res.hrc"
#include "resource/common_res.hrc"

#if OSL_DEBUG_LEVEL > 0
# define OUtoCStr( x ) ( OUStringToOString ( (x), RTL_TEXTENCODING_ASCII_US).getStr())
#else /* OSL_DEBUG_LEVEL */
# define OUtoCStr( x ) ("dummy")
#endif /* OSL_DEBUG_LEVEL */

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::mork;
using namespace ::cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;

//------------------------------------------------------------------------------
//  IMPLEMENT_SERVICE_INFO(OResultSet,"com.sun.star.sdbcx.OResultSet","com.sun.star.sdbc.ResultSet");
OUString SAL_CALL OResultSet::getImplementationName(  ) throw ( RuntimeException)    \
{
    return OUString("com.sun.star.sdbcx.mork.ResultSet");
}
// -------------------------------------------------------------------------
 Sequence< OUString > SAL_CALL OResultSet::getSupportedServiceNames(  ) throw( RuntimeException)
{
    ::com::sun::star::uno::Sequence< OUString > aSupported(2);
    aSupported[0] = OUString("com.sun.star.sdbc.ResultSet");
    aSupported[1] = OUString("com.sun.star.sdbcx.ResultSet");
    return aSupported;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::supportsService( const OUString& _rServiceName ) throw( RuntimeException)
{
    Sequence< OUString > aSupported(getSupportedServiceNames());
    const OUString* pSupported = aSupported.getConstArray();
    const OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

// -------------------------------------------------------------------------
OResultSet::OResultSet(OCommonStatement* pStmt, const ::boost::shared_ptr< connectivity::OSQLParseTreeIterator >& _pSQLIterator )
    : OResultSet_BASE(m_aMutex)
    ,OPropertySetHelper(OResultSet_BASE::rBHelper)
    ,m_pStatement(pStmt)
    ,m_xStatement(*pStmt)
    ,m_xMetaData(NULL)
    ,m_nRowPos(0)
    ,m_nOldRowPos(0)
    ,m_nFetchSize(0)
    ,m_nResultSetType(ResultSetType::SCROLL_INSENSITIVE)
    ,m_nFetchDirection(FetchDirection::FORWARD)
    ,m_nResultSetConcurrency(ResultSetConcurrency::UPDATABLE)
    ,m_pSQLIterator( _pSQLIterator )
    ,m_pParseTree( _pSQLIterator->getParseTree() )
    ,m_aQueryHelper(pStmt->getOwnConnection()->getColumnAlias())
    ,m_pTable(NULL)
    ,m_CurrentRowCount(0)
    ,m_nParamIndex(0)
    ,m_bIsAlwaysFalseQuery(sal_False)
    ,m_pKeySet(NULL)
    ,m_nNewRow(0)
    ,m_nUpdatedRow(0)
    ,m_RowStates(0)
    ,m_bIsReadOnly(-1)
{

    //m_aQuery.setMaxNrOfReturns(pStmt->getOwnConnection()->getMaxResultRecords());
}
// -------------------------------------------------------------------------
OResultSet::~OResultSet()
{
}

// -------------------------------------------------------------------------
void OResultSet::disposing(void)
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    m_xStatement.clear();
    m_xMetaData.clear();
    m_pParseTree    = NULL;
    m_xColumns = NULL;
    m_xParamColumns = NULL;
    m_pKeySet       = NULL;
    if(m_pTable)
    {
        m_pTable->release();
        m_pTable = NULL;
    }
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
void OResultSet::methodEntry()
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    if ( !m_pTable )
    {
        OSL_FAIL( "OResultSet::methodEntry: looks like we're disposed, but how is this possible?" );
        throw DisposedException( OUString(), *this );
    }
}

// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSet::findColumn( const OUString& columnName ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );

    // find the first column with the name columnName
    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    sal_Int32 i = 1;
    for(;i<=nLen;++i)
        if(xMeta->isCaseSensitive(i) ? columnName == xMeta->getColumnName(i) :
                columnName.equalsIgnoreAsciiCase(xMeta->getColumnName(i)))
            break;
    /* FIXME should throw in case of not found ? or at least return -1 */

    return i;
}
// -------------------------------------------------------------------------
Reference< XInputStream > SAL_CALL OResultSet::getBinaryStream( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XInputStream > SAL_CALL OResultSet::getCharacterStream( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    return NULL;
}

// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::getBoolean( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    m_bWasNull = sal_True;
    return sal_False;
}
// -------------------------------------------------------------------------

sal_Int8 SAL_CALL OResultSet::getByte( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    return 0;
}
// -------------------------------------------------------------------------

Sequence< sal_Int8 > SAL_CALL OResultSet::getBytes( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    return Sequence< sal_Int8 >();
}
// -------------------------------------------------------------------------

Date SAL_CALL OResultSet::getDate( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    return Date();
}
// -------------------------------------------------------------------------

double SAL_CALL OResultSet::getDouble( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    return 0.0;
}
// -------------------------------------------------------------------------

float SAL_CALL OResultSet::getFloat( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    return 0;
}
// -------------------------------------------------------------------------

sal_Int32 SAL_CALL OResultSet::getInt( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    return 0;
}

// -------------------------------------------------------------------------
sal_Int32 SAL_CALL OResultSet::getRow(  ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );

    OSL_TRACE("In/Out: OResultSet::getRow, return = %u", m_nRowPos );
    return m_nRowPos;
}
// -------------------------------------------------------------------------

sal_Int64 SAL_CALL OResultSet::getLong( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    return sal_Int64();
}
// -------------------------------------------------------------------------

Reference< XResultSetMetaData > SAL_CALL OResultSet::getMetaData(  ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );

    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(
        m_pSQLIterator->getSelectColumns(), m_pSQLIterator->getTables().begin()->first ,m_pTable,determineReadOnly());
    return m_xMetaData;
}
// -------------------------------------------------------------------------
Reference< XArray > SAL_CALL OResultSet::getArray( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    return NULL;
}

// -------------------------------------------------------------------------

Reference< XClob > SAL_CALL OResultSet::getClob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------
Reference< XBlob > SAL_CALL OResultSet::getBlob( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------

Reference< XRef > SAL_CALL OResultSet::getRef( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    return NULL;
}
// -------------------------------------------------------------------------

Any SAL_CALL OResultSet::getObject( sal_Int32 /*columnIndex*/, const Reference< ::com::sun::star::container::XNameAccess >& /*typeMap*/ ) throw(SQLException, RuntimeException)
{
    return Any();
}
// -------------------------------------------------------------------------

sal_Int16 SAL_CALL OResultSet::getShort( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    return 0;
}
// -------------------------------------------------------------------------

void OResultSet::checkIndex(sal_Int32 columnIndex ) throw(::com::sun::star::sdbc::SQLException)
{
    if(columnIndex <= 0 || columnIndex > (sal_Int32)m_xColumns->get().size())
        ::dbtools::throwInvalidIndexException(*this);
}
// -------------------------------------------------------------------------
sal_uInt32 OResultSet::currentRowCount()
{
    if ( m_bIsAlwaysFalseQuery )
        return 0;
    //return 0;//m_aQuery.getRealRowCount() - deletedCount();
    // new implementation
    return m_aQueryHelper.getResultCount();
}

// -------------------------------------------------------------------------

sal_Bool OResultSet::fetchCurrentRow( ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("fetchCurrentRow, m_nRowPos = %u", m_nRowPos );
    return fetchRow(getCurrentCardNumber());
}

// -------------------------------------------------------------------------
sal_Bool OResultSet::pushCard(sal_uInt32 /*cardNumber*/) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.mork", "=> OResultSet::pushCard()" );
    return sal_True;
/*
    if (cardNumber == 0)
        return sal_True;
    // Check whether we are storing the updated row
    if ( (m_aRow->get())[0].isNull() || (sal_Int32)(m_aRow->get())[0] != (sal_Int32)cardNumber )
        return sal_False;

    sal_Int32 nCount = m_aColumnNames.getLength();
    m_aQuery.setRowStates(cardNumber,m_RowStates);
    for( sal_Int32 i = 1; i <= nCount; i++ )
    {
        if ( (m_aRow->get())[i].isBound() )
        {
            //
            // Everything in the addressbook is a string!
            //
            if ( !m_aQuery.setRowValue( (m_aRow->get())[i], cardNumber, m_aColumnNames[i-1], DataType::VARCHAR ))
            {
                m_pStatement->getOwnConnection()->throwSQLException( m_aQuery.getError(), *this );
            }
        }
    }
    return sal_True;
*/
}
// -------------------------------------------------------------------------
sal_Bool OResultSet::fetchRow(sal_Int32 cardNumber,sal_Bool bForceReload) throw(SQLException, RuntimeException)
{
    SAL_INFO("connectivity.mork", "=> OResultSet::fetchRow()" );

    OSL_TRACE("fetchRow, cardNumber = %u", cardNumber );
    if (!bForceReload)
    {
        // Check whether we've already fetched the row...
        if ( !(m_aRow->get())[0].isNull() && (sal_Int32)(m_aRow->get())[0] == (sal_Int32)cardNumber )
            return sal_True;
        //Check whether the old row has been changed
        if (cardNumber == m_nUpdatedRow)
        {
            //write back the changes first
            if (!pushCard(cardNumber))  //error write back the changes
                throw SQLException();
        }
    }
//    else
//        m_aQuery.resyncRow(cardNumber);

    if ( validRow( cardNumber ) == sal_False )
        return sal_False;

    (m_aRow->get())[0] = (sal_Int32)cardNumber;
    sal_Int32 nCount = m_aColumnNames.getLength();
    //m_RowStates = m_aQuery.getRowStates(cardNumber);
    for( sal_Int32 i = 1; i <= nCount; i++ )
    {
        if ( (m_aRow->get())[i].isBound() )
        {
            //
            // Everything in the addressbook is a string!
            //
            if ( !m_aQueryHelper.getRowValue( (m_aRow->get())[i], cardNumber, m_aColumnNames[i-1], DataType::VARCHAR ))
            {
                m_pStatement->getOwnConnection()->throwSQLException( m_aQueryHelper.getError(), *this );
            }
        }
    }
    return sal_True;

}
// -------------------------------------------------------------------------

const ORowSetValue& OResultSet::getValue(sal_Int32 cardNumber, sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    if ( fetchRow( cardNumber ) == sal_False )
    {
        OSL_FAIL("fetchRow() returned False" );
        m_bWasNull = sal_True;
        return *ODatabaseMetaDataResultSet::getEmptyValue();
    }

    m_bWasNull = (m_aRow->get())[columnIndex].isNull();
    return (m_aRow->get())[columnIndex];

}
// -------------------------------------------------------------------------


OUString SAL_CALL OResultSet::getString( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );

    OSL_ENSURE(m_xColumns.is(), "Need the Columns!!");
    OSL_ENSURE(columnIndex <= (sal_Int32)m_xColumns->get().size(), "Trying to access invalid columns number");
    checkIndex( columnIndex );

    // If this query was sorted then we should have a valid KeySet, so use it
    return getValue(getCurrentCardNumber(), mapColumn( columnIndex ) );

}
// -------------------------------------------------------------------------

Time SAL_CALL OResultSet::getTime( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    return Time();
}
// -------------------------------------------------------------------------


DateTime SAL_CALL OResultSet::getTimestamp( sal_Int32 /*columnIndex*/ ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    return DateTime();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::isBeforeFirst(  ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );

    // here you have to implement your movements
    // return true means there is no data
    OSL_TRACE("In/Out: OResultSet::isBeforeFirst" );
    return( m_nRowPos < 1 );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isAfterLast(  ) throw(SQLException, RuntimeException)
{
    SAL_WARN("connectivity.mork", "OResultSet::isAfterLast() NOT IMPLEMENTED!");
    ResultSetEntryGuard aGuard( *this );

    OSL_TRACE("In/Out: OResultSet::isAfterLast" );
//    return sal_True;
    return m_nRowPos > currentRowCount() && m_aQueryHelper.queryComplete();
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isFirst(  ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );

    OSL_TRACE("In/Out: OResultSet::isFirst" );
    return m_nRowPos == 1;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::isLast(  ) throw(SQLException, RuntimeException)
{
    SAL_WARN("connectivity.mork", "OResultSet::isLast() NOT IMPLEMENTED!");
    ResultSetEntryGuard aGuard( *this );

    OSL_TRACE("In/Out: OResultSet::isLast" );
//    return sal_True;
    return m_nRowPos == currentRowCount() && m_aQueryHelper.queryComplete();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::beforeFirst(  ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );

    // move before the first row so that isBeforeFirst returns false
    OSL_TRACE("In/Out: OResultSet::beforeFirst" );
    if ( first() )
        previous();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::afterLast(  ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    OSL_TRACE("In/Out: OResultSet::afterLast" );

    if(last())
        next();
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::close() throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::close" );
    dispose();
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::first(  ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::first" );
    return seekRow( FIRST_POS );
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::last(  ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::last" );
    return seekRow( LAST_POS );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::absolute( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::absolute" );
    return seekRow( ABSOLUTE_POS, row );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::relative( sal_Int32 row ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::relative" );
    return seekRow( RELATIVE_POS, row );
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::previous(  ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    OSL_TRACE("In/Out: OResultSet::previous" );
    return seekRow( PRIOR_POS );
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OResultSet::getStatement(  ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );

    OSL_TRACE("In/Out: OResultSet::getStatement" );
    return m_xStatement;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::rowDeleted(  ) throw(SQLException, RuntimeException)
{
    SAL_WARN("connectivity.mork", "OResultSet::rowDeleted() NOT IMPLEMENTED!");
    ResultSetEntryGuard aGuard( *this );
    OSL_TRACE("In/Out: OResultSet::rowDeleted, m_RowStates=%u",m_RowStates );
    return sal_True;//return ((m_RowStates & RowStates_Deleted) == RowStates_Deleted) ;
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowInserted(  ) throw(SQLException, RuntimeException)
{
    SAL_WARN("connectivity.mork", "OResultSet::rowInserted() NOT IMPLEMENTED!");
    ResultSetEntryGuard aGuard( *this );
    OSL_TRACE("In/Out: OResultSet::rowInserted,m_RowStates=%u",m_RowStates );
    return sal_True;//return ((m_RowStates & RowStates_Inserted) == RowStates_Inserted);
}
// -------------------------------------------------------------------------
sal_Bool SAL_CALL OResultSet::rowUpdated(  ) throw(SQLException, RuntimeException)
{
    SAL_WARN("connectivity.mork", "OResultSet::rowUpdated() NOT IMPLEMENTED!");
    ResultSetEntryGuard aGuard( *this );
    OSL_TRACE("In/Out: OResultSet::rowUpdated,m_RowStates=%u",m_RowStates );
    return sal_True;// return ((m_RowStates & RowStates_Updated) == RowStates_Updated) ;
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::next(  ) throw(SQLException, RuntimeException)
{
    return seekRow( NEXT_POS );
}
// -------------------------------------------------------------------------

sal_Bool SAL_CALL OResultSet::wasNull(  ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );

    return m_bWasNull;
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::cancel(  ) throw(RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    OSL_TRACE("In/Out: OResultSet::cancel" );

}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::clearWarnings(  ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::clearWarnings" );
}
// -------------------------------------------------------------------------
Any SAL_CALL OResultSet::getWarnings(  ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::getWarnings" );
    return Any();
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::refreshRow(  ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("In/Out: OResultSet::refreshRow" );
    if (fetchRow(getCurrentCardNumber(),sal_True)) {
        //force fetch current row will cause we lose all change to the current row
        m_pStatement->getOwnConnection()->throwSQLException( STR_ERROR_REFRESH_ROW, *this );
    }
}
// -------------------------------------------------------------------------
IPropertyArrayHelper* OResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps(5);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
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
                            Any & /*rConvertedValue*/,
                            Any & /*rOldValue*/,
                            sal_Int32 nHandle,
                            const Any& /*rValue*/ )
                                throw (::com::sun::star::lang::IllegalArgumentException)
{
    OSL_FAIL( "OResultSet::convertFastPropertyValue: not implemented!" );
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw ::com::sun::star::lang::IllegalArgumentException();
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        default:
            ;
    }
    return sal_False;
}
// -------------------------------------------------------------------------
void OResultSet::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& /*rValue*/
                                                 )
                                                 throw (Exception)
{
    OSL_FAIL( "OResultSet::setFastPropertyValue_NoBroadcast: not implemented!" );
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw Exception();
        case PROPERTY_ID_FETCHDIRECTION:
            break;
        case PROPERTY_ID_FETCHSIZE:
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
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            rValue <<= (sal_Int32)m_nResultSetConcurrency;
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            rValue <<= m_nResultSetType;
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            rValue <<= m_nFetchDirection;
            break;
        case PROPERTY_ID_FETCHSIZE:
            rValue <<= m_nFetchSize;
            break;
        case PROPERTY_ID_ISBOOKMARKABLE:
            const_cast< OResultSet* >( this )->determineReadOnly();
            rValue <<= !m_bIsReadOnly;
         break;
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::acquire() throw()
{
    OResultSet_BASE::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::release() throw()
{
    OResultSet_BASE::release();
}
// -----------------------------------------------------------------------------
::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL OResultSet::getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException)
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}

// -------------------------------------------------------------------------
void OResultSet::parseParameter( const OSQLParseNode* pNode, OUString& rMatchString )
{
    OSL_ENSURE(pNode->count() > 0,"Error parsing parameter in Parse Tree");
    OSQLParseNode *pMark = pNode->getChild(0);

    // Initialize to empty string
    rMatchString = OUString("");

    OUString aParameterName;
    if (SQL_ISPUNCTUATION(pMark,"?")) {
        aParameterName = OUString("?");
    }
    else if (SQL_ISPUNCTUATION(pMark,":")) {
        aParameterName = pNode->getChild(1)->getTokenValue();
    }
    // XXX - Now we know name, what's value????
    m_nParamIndex ++;
    OSL_TRACE("Parameter name [%d]: %s", m_nParamIndex,OUtoCStr(aParameterName) );

    if ( m_aParameterRow.is() ) {
        OSL_ENSURE( m_nParamIndex < (sal_Int32)m_aParameterRow->get().size() + 1, "More parameters than values found" );
        rMatchString = (m_aParameterRow->get())[(sal_uInt16)m_nParamIndex];
#if OSL_DEBUG_LEVEL > 0
        OSL_TRACE("Prop Value       : %s", OUtoCStr( rMatchString ) );
#endif
    }
#if OSL_DEBUG_LEVEL > 0
    else {
        OSL_TRACE("Prop Value       : Invalid ParameterRow!" );
    }
#endif
}

void OResultSet::analyseWhereClause( const OSQLParseNode*                 parseTree,
                                     MQueryExpression                     &queryExpression)
{
    OUString         columnName;
    MQueryOp::cond_type     op( MQueryOp::Is );
    OUString         matchString;

    if ( parseTree == NULL )
        return;

    if ( m_pSQLIterator->getParseTree() != NULL ) {
        ::rtl::Reference<OSQLColumns> xColumns = m_pSQLIterator->getParameters();
        if(xColumns.is())
        {
            OUString aColName, aParameterValue;
            OSQLColumns::Vector::iterator aIter = xColumns->get().begin();
            sal_Int32 i = 1;
            for(;aIter != xColumns->get().end();++aIter)
            {
                (*aIter)->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= aColName;
                OSL_TRACE("Prop Column Name : %s", OUtoCStr( aColName ) );
                if ( m_aParameterRow.is() ) {
                    aParameterValue = (m_aParameterRow->get())[(sal_uInt16)i];
#if OSL_DEBUG_LEVEL > 0
                    OSL_TRACE("Prop Value       : %s", OUtoCStr( aParameterValue ) );
#endif
                }
#if OSL_DEBUG_LEVEL > 0
                else {
                    OSL_TRACE("Prop Value       : Invalid ParameterRow!" );
                }
#endif
                i++;
            }
        }

    }

    if ( SQL_ISRULE(parseTree,where_clause) )
    {
        OSL_TRACE("analyseSQL : Got WHERE clause");
        // Reset Parameter Counter
        resetParameters();
        analyseWhereClause( parseTree->getChild( 1 ), queryExpression );
    }
    else if ( parseTree->count() == 3 &&                         // Handle ()'s
        SQL_ISPUNCTUATION(parseTree->getChild(0),"(") &&
        SQL_ISPUNCTUATION(parseTree->getChild(2),")"))
    {

        OSL_TRACE("analyseSQL : Got Punctuation ()");
        MQueryExpression *subExpression = new MQueryExpression();
        analyseWhereClause( parseTree->getChild( 1 ), *subExpression );
        queryExpression.getExpressions().push_back( subExpression );
    }
    else if ((SQL_ISRULE(parseTree,search_condition) || (SQL_ISRULE(parseTree,boolean_term)))
             && parseTree->count() == 3)                   // Handle AND/OR
    {

        OSL_TRACE("analyseSQL : Got AND/OR clause");

        // TODO - Need to take care or AND, for now match is always OR
        analyseWhereClause( parseTree->getChild( 0 ), queryExpression );
        analyseWhereClause( parseTree->getChild( 2 ), queryExpression );

        if (SQL_ISTOKEN(parseTree->getChild(1),OR)) {         // OR-Operator
            queryExpression.setExpressionCondition( MQueryExpression::OR );
        }
        else if (SQL_ISTOKEN(parseTree->getChild(1),AND)) {  // AND-Operator
            queryExpression.setExpressionCondition( MQueryExpression::AND );
        }
        else {
            OSL_FAIL("analyseSQL: Error in Parse Tree");
        }
    }
    else if (SQL_ISRULE(parseTree,comparison_predicate))
    {
        OSL_ENSURE(parseTree->count() == 3, "Error parsing COMPARE predicate");
        if (!(SQL_ISRULE(parseTree->getChild(0),column_ref) ||
          parseTree->getChild(2)->getNodeType() == SQL_NODE_STRING ||
          parseTree->getChild(2)->getNodeType() == SQL_NODE_INTNUM ||
          parseTree->getChild(2)->getNodeType() == SQL_NODE_APPROXNUM ||
          SQL_ISTOKEN(parseTree->getChild(2),TRUE) ||
          SQL_ISTOKEN(parseTree->getChild(2),FALSE) ||
          SQL_ISRULE(parseTree->getChild(2),parameter) ||
          // odbc date
          (SQL_ISRULE(parseTree->getChild(2),set_fct_spec) && SQL_ISPUNCTUATION(parseTree->getChild(2)->getChild(0),"{"))))
        {
            m_pStatement->getOwnConnection()->throwSQLException( STR_QUERY_TOO_COMPLEX, *this );
        }

        OSQLParseNode *pPrec = parseTree->getChild(1);
        if (pPrec->getNodeType() == SQL_NODE_EQUAL)
            op = MQueryOp::Is;
        else if (pPrec->getNodeType() == SQL_NODE_NOTEQUAL)
            op = MQueryOp::IsNot;

        OUString sTableRange;
        if(SQL_ISRULE(parseTree->getChild(0),column_ref))
            m_pSQLIterator->getColumnRange(parseTree->getChild(0),columnName,sTableRange);
        else if(parseTree->getChild(0)->isToken())
            columnName = parseTree->getChild(0)->getTokenValue();

        if ( SQL_ISRULE(parseTree->getChild(2),parameter) ) {
            parseParameter( parseTree->getChild(2), matchString );
        }
        else {
            matchString = parseTree->getChild(2)->getTokenValue();
        }

        if ( columnName.compareToAscii("0") ==0 && op == MQueryOp::Is &&
             matchString.compareToAscii("1") == 0 ) {
            OSL_TRACE("Query always evaluates to FALSE");
            m_bIsAlwaysFalseQuery = sal_True;
        }
        queryExpression.getExpressions().push_back( new MQueryExpressionString( columnName, op, matchString ));
    }
    else if (SQL_ISRULE(parseTree,like_predicate))
    {
        OSL_ENSURE(parseTree->count() == 2, "Error parsing LIKE predicate");

        OSL_TRACE("analyseSQL : Got LIKE rule");

        if ( !(SQL_ISRULE(parseTree->getChild(0), column_ref)) )
        {
            m_pStatement->getOwnConnection()->throwSQLException( STR_QUERY_INVALID_LIKE_COLUMN, *this );
        }


        OSQLParseNode *pColumn;
        OSQLParseNode *pAtom;
        OSQLParseNode *pOptEscape;
        const OSQLParseNode* pPart2 = parseTree->getChild(1);
        pColumn     = parseTree->getChild(0);                        // Match Item
        pAtom       = pPart2->getChild(static_cast<sal_uInt32>(pPart2->count()-2));     // Match String
        pOptEscape  = pPart2->getChild(static_cast<sal_uInt32>(pPart2->count()-1));     // Opt Escape Rule
        (void)pOptEscape;
        const bool bNot = SQL_ISTOKEN(pPart2->getChild(0), NOT);

        if (!(pAtom->getNodeType() == SQL_NODE_STRING ||
              pAtom->getNodeType() == SQL_NODE_NAME ||
              SQL_ISRULE(pAtom,parameter) ||
              ( pAtom->getChild(0) && pAtom->getChild(0)->getNodeType() == SQL_NODE_NAME ) ||
              ( pAtom->getChild(0) && pAtom->getChild(0)->getNodeType() == SQL_NODE_STRING )
              ) )
        {
            OSL_TRACE("analyseSQL : pAtom->count() = %zu", pAtom->count() );

            m_pStatement->getOwnConnection()->throwSQLException( STR_QUERY_INVALID_LIKE_STRING, *this );
        }

        const sal_Unicode WILDCARD = '%';
        const sal_Unicode ALT_WILDCARD = '*';
        const sal_Unicode MATCHCHAR = '_';

        OUString sTableRange;
        if(SQL_ISRULE(pColumn,column_ref))
            m_pSQLIterator->getColumnRange(pColumn,columnName,sTableRange);

        OSL_TRACE("ColumnName = %s", OUtoCStr( columnName ) );

        if ( SQL_ISRULE(pAtom,parameter) ) {
            parseParameter( pAtom, matchString );
            // Replace all '*' with '%' : UI Usually does this but not with
            // Parameters for some reason.
            matchString = matchString.replace( ALT_WILDCARD, WILDCARD );
        }
        else
        {
            matchString = pAtom->getTokenValue();
        }

        // Determine where '%' character is...

        if ( matchString.equals( OUString::valueOf( WILDCARD ) ) )
        {
            // String containing only a '%' and nothing else
            op = MQueryOp::Exists;
            // Will be ignored for Exists case, but clear anyway.
            matchString = OUString("");
        }
        else if ( matchString.indexOf ( WILDCARD ) == -1 &&
             matchString.indexOf ( MATCHCHAR ) == -1 )
        {
            // Simple string , eg. "to match"
            if ( bNot )
                op = MQueryOp::DoesNotContain;
            else
                op = MQueryOp::Contains;
        }
        else if (  matchString.indexOf ( WILDCARD ) == 0
                   && matchString.lastIndexOf ( WILDCARD ) == matchString.getLength() -1
                   && matchString.indexOf ( WILDCARD, 1 ) == matchString.lastIndexOf ( WILDCARD )
                   && matchString.indexOf( MATCHCHAR ) == -1
                 )
        {
            // Relatively simple "%string%" - ie, contains...
            // Cut '%'  from front and rear
            matchString = matchString.replaceAt( 0, 1, OUString() );
            matchString = matchString.replaceAt( matchString.getLength() -1 , 1, OUString() );

            if (bNot)
                op = MQueryOp::DoesNotContain;
            else
                op = MQueryOp::Contains;
        }
        else if ( bNot )
        {
            // We currently can't handle a 'NOT LIKE' when there are '%' or
            // '_' dispersed throughout
            m_pStatement->getOwnConnection()->throwSQLException( STR_QUERY_NOT_LIKE_TOO_COMPLEX, *this );
        }
        else
        {
            if ( (matchString.indexOf ( WILDCARD ) == matchString.lastIndexOf ( WILDCARD ))
                  && matchString.indexOf( MATCHCHAR ) == -1
                )
            {
                // One occurrence of '%' - no '_' matches...
                if ( matchString.indexOf ( WILDCARD ) == 0 )
                {
                    op = MQueryOp::EndsWith;
                    matchString = matchString.replaceAt( 0, 1, OUString());
                }
                else if ( matchString.indexOf ( WILDCARD ) == matchString.getLength() -1 )
                {
                    op = MQueryOp::BeginsWith;
                    matchString = matchString.replaceAt( matchString.getLength() -1 , 1, OUString() );
                }
                else
                {
                    sal_Int32 pos = matchString.indexOf ( WILDCARD );
                    matchString = matchString.replaceAt( pos, 1,OUString(".*") );
                    op = MQueryOp::RegExp;
                }

            }
            else
            {
                // Most Complex, need to use an RE
                sal_Int32 pos = matchString.indexOf ( WILDCARD );
                while ( (pos = matchString.indexOf ( WILDCARD )) != -1 )
                {
                    matchString = matchString.replaceAt( pos, 1, OUString(".*") );
                }

                pos = matchString.indexOf ( MATCHCHAR );
                while ( (pos = matchString.indexOf( MATCHCHAR )) != -1 )
                {
                    matchString = matchString.replaceAt( pos, 1, OUString(".") );
                }

                op = MQueryOp::RegExp;
            }
        }

        queryExpression.getExpressions().push_back( new MQueryExpressionString( columnName, op, matchString ));
    }
    else if (SQL_ISRULE(parseTree,test_for_null))
    {
        OSL_ENSURE(parseTree->count() == 2,"Error in ParseTree");
        const OSQLParseNode* pPart2 = parseTree->getChild(1);
        OSL_ENSURE(SQL_ISTOKEN(pPart2->getChild(0),IS),"Error in ParseTree");

        if (!SQL_ISRULE(parseTree->getChild(0),column_ref))
        {
            m_pStatement->getOwnConnection()->throwSQLException( STR_QUERY_INVALID_IS_NULL_COLUMN, *this );
        }

        if (SQL_ISTOKEN(pPart2->getChild(1),NOT))
        {
            op = MQueryOp::Exists;
        }
        else
        {
            op = MQueryOp::DoesNotExist;
        }

        OUString sTableRange;
        m_pSQLIterator->getColumnRange(parseTree->getChild(0),columnName,sTableRange);

        queryExpression.getExpressions().push_back( new MQueryExpressionString( columnName, op ));
    }
    else
    {
        OSL_TRACE( "Unexpected statement!!!" );

        m_pStatement->getOwnConnection()->throwSQLException( STR_QUERY_TOO_COMPLEX, *this );
    }
}

// -----------------------------------------------------------------------------

void OResultSet::fillRowData()
    throw( ::com::sun::star::sdbc::SQLException )
{
    OSL_ENSURE( m_pStatement, "Require a statement" );

    MQueryExpression queryExpression;

    OConnection* xConnection = static_cast<OConnection*>(m_pStatement->getConnection().get());
    m_xColumns = m_pSQLIterator->getSelectColumns();

    OSL_ENSURE(m_xColumns.is(), "Need the Columns!!");

    OSQLColumns::Vector::const_iterator aIter = m_xColumns->get().begin();
    const OUString sProprtyName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);
    OUString sName;
    m_aAttributeStrings.clear();
    m_aAttributeStrings.reserve(m_xColumns->get().size());
    for (sal_Int32 i = 1; aIter != m_xColumns->get().end();++aIter, i++)
    {
        (*aIter)->getPropertyValue(sProprtyName) >>= sName;
#if OSL_DEBUG_LEVEL > 0
        OSL_TRACE("Query Columns : (%d) %s", i, OUtoCStr(sName) );
#endif
        m_aAttributeStrings.push_back( sName );
    }

    // Generate Match Conditions for Query
    const OSQLParseNode*  pParseTree = m_pSQLIterator->getWhereTree();

    m_bIsAlwaysFalseQuery = sal_False;
    if ( pParseTree != NULL )
    {
        // Extract required info

        OSL_TRACE("\tHave a Where Clause");

        analyseWhereClause( pParseTree, queryExpression );
    }
    // If the query is a 0=1 then set Row count to 0 and return
    if ( m_bIsAlwaysFalseQuery )
    {
        m_bIsReadOnly = 1;
        return;
    }

    m_aQueryHelper.setExpression( queryExpression );

    OUString aStr(  m_pTable->getName() );
    m_aQueryHelper.setAddressbook( aStr );

    sal_Int32 rv = m_aQueryHelper.executeQuery(xConnection);
    if ( rv == -1 ) {
        m_pStatement->getOwnConnection()->throwSQLException( STR_ERR_EXECUTING_QUERY, *this );
    }

    if (m_aQueryHelper.hadError())
    {
        m_pStatement->getOwnConnection()->throwSQLException( m_aQueryHelper.getError(), *this );
    }

    //determine whether the address book is readonly
    determineReadOnly();

#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE( "executeQuery returned %d", rv );

    OSL_TRACE( "\tOUT OResultSet::fillRowData()" );
#endif
}

// -----------------------------------------------------------------------------
static sal_Bool matchRow( OValueRow& row1, OValueRow& row2 )
{
    OValueVector::Vector::iterator row1Iter = row1->get().begin();
    OValueVector::Vector::iterator row2Iter = row2->get().begin();
    for ( ++row1Iter,++row2Iter; // the first column is the bookmark column
          row1Iter != row1->get().end(); ++row1Iter,++row2Iter)
    {
        if ( row1Iter->isBound())
        {
            // Compare values, if at anytime there's a mismatch return false
            if ( !( (*row1Iter) == (*row2Iter) ) )
                return sal_False;
        }
    }

    // If we get to here the rows match
    return sal_True;
}

sal_Int32 OResultSet::getRowForCardNumber(sal_Int32 nCardNum)
{
    OSL_TRACE("In/Out: OResultSet::getRowForCardNumber, nCardNum = %u", nCardNum );

    if ( m_pKeySet.is() )
    {
        sal_Int32  nPos;
        for(nPos=0;nPos < (sal_Int32)m_pKeySet->get().size();nPos++)
        {
            if (nCardNum == (m_pKeySet->get())[nPos])
            {
                OSL_TRACE("In/Out: OResultSet::getRowForCardNumber, return = %u", nPos+1 );
                return nPos+1;
            }
        }
    }

    m_pStatement->getOwnConnection()->throwSQLException( STR_INVALID_BOOKMARK, *this );

    return 0;
}

// -----------------------------------------------------------------------------
void SAL_CALL OResultSet::executeQuery() throw( ::com::sun::star::sdbc::SQLException,
                                                ::com::sun::star::uno::RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );

    OSL_ENSURE( m_pTable, "Need a Table object");
    if(!m_pTable)
    {
        const OSQLTables& xTabs = m_pSQLIterator->getTables();
        if ((xTabs.begin() == xTabs.end()) || !xTabs.begin()->second.is())
            m_pStatement->getOwnConnection()->throwSQLException( STR_QUERY_TOO_COMPLEX, *this );

        m_pTable = static_cast< OTable* > ((xTabs.begin()->second).get());

    }

    m_nRowPos = 0;

    fillRowData();

    OSL_ENSURE(m_xColumns.is(), "Need the Columns!!");

    switch( m_pSQLIterator->getStatementType() )
    {
        case SQL_STATEMENT_SELECT:
        {
            if(m_bIsAlwaysFalseQuery) {
                break;
            }
            else if(isCount())
            {
                m_pStatement->getOwnConnection()->throwSQLException( STR_NO_COUNT_SUPPORT, *this );
            }
            else
            {
                sal_Bool bDistinct = sal_False;
                OSQLParseNode *pDistinct = m_pParseTree->getChild(1);
                if (pDistinct && pDistinct->getTokenID() == SQL_TOKEN_DISTINCT)
                {
                    if(!IsSorted())
                    {
                        m_aOrderbyColumnNumber.push_back(m_aColMapping[1]);
                        m_aOrderbyAscending.push_back(SQL_DESC);
                    }
                    bDistinct = sal_True;
                }

                OSortIndex::TKeyTypeVector eKeyType(m_aOrderbyColumnNumber.size());
                ::std::vector<sal_Int32>::iterator aOrderByIter = m_aOrderbyColumnNumber.begin();
                for ( ::std::vector<sal_Int16>::size_type i = 0; aOrderByIter != m_aOrderbyColumnNumber.end(); ++aOrderByIter,++i)
                {
                    OSL_ENSURE((sal_Int32)m_aRow->get().size() > *aOrderByIter,"Invalid Index");
                    switch ((m_aRow->get().begin()+*aOrderByIter)->getTypeKind())
                    {
                    case DataType::CHAR:
                        case DataType::VARCHAR:
                            eKeyType[i] = SQL_ORDERBYKEY_STRING;
                            break;

                        case DataType::OTHER:
                        case DataType::TINYINT:
                        case DataType::SMALLINT:
                        case DataType::INTEGER:
                        case DataType::DECIMAL:
                        case DataType::NUMERIC:
                        case DataType::REAL:
                        case DataType::DOUBLE:
                        case DataType::DATE:
                        case DataType::TIME:
                        case DataType::TIMESTAMP:
                        case DataType::BIT:
                            eKeyType[i] = SQL_ORDERBYKEY_DOUBLE;
                            break;

                    // Other types aren't implemented (so they are always FALSE)
                        default:
                            eKeyType[i] = SQL_ORDERBYKEY_NONE;
                            OSL_FAIL("MResultSet::executeQuery: Order By Data Type not implemented");
                            break;
                    }
                }

                if (IsSorted())
                {
                    // Implement Sorting

                    // So that we can sort we need to wait until the executed
                    // query to the mozilla addressbooks has returned all
                    // values.

                    OSL_TRACE("Query is to be sorted");

                    OSL_ENSURE( m_aQueryHelper.queryComplete(), "Query not complete!!");

                    m_pSortIndex = new OSortIndex(eKeyType,m_aOrderbyAscending);

                    OSL_TRACE("OrderbyColumnNumber->size() = %zu",m_aOrderbyColumnNumber.size());
#if OSL_DEBUG_LEVEL > 0
                    for ( ::std::vector<sal_Int32>::size_type i = 0; i < m_aColMapping.size(); i++ )
                        OSL_TRACE("Mapped: %d -> %d", i, m_aColMapping[i] );
#endif
                    for ( sal_Int32 nRow = 1; nRow <= m_aQueryHelper.getResultCount(); nRow++ ) {

                        OKeyValue* pKeyValue = OKeyValue::createKeyValue((nRow));

                        ::std::vector<sal_Int32>::iterator aIter = m_aOrderbyColumnNumber.begin();
                        for (;aIter != m_aOrderbyColumnNumber.end(); ++aIter)
                        {
                            const ORowSetValue& value = getValue(nRow, *aIter);

                            OSL_TRACE( "Adding Value: (%d,%d) : %s", nRow, *aIter,OUtoCStr( value ));

                            pKeyValue->pushKey(new ORowSetValueDecorator(value));
                        }

                        m_pSortIndex->AddKeyValue( pKeyValue );
                    }

                    m_pKeySet = m_pSortIndex->CreateKeySet();
                    m_CurrentRowCount = static_cast<sal_Int32>(m_pKeySet->get().size());
#if OSL_DEBUG_LEVEL > 0
                    for( OKeySet::Vector::size_type i = 0; i < m_pKeySet->get().size(); i++ )
                        OSL_TRACE("Sorted: %d -> %d", i, (m_pKeySet->get())[i] );
#endif

                    m_pSortIndex = NULL;
                    beforeFirst(); // Go back to start
                }
                else  //we always need m_pKeySet now
                    m_pKeySet = new OKeySet();

                // Handle the DISTINCT case
                if ( bDistinct && m_pKeySet.is() )
                {
                    OValueRow aSearchRow = new OValueVector( m_aRow->get().size() );

                    for( OKeySet::Vector::size_type i = 0; i < m_pKeySet->get().size(); i++ )
                    {
                        fetchRow( (m_pKeySet->get())[i] );        // Fills m_aRow
                        if ( matchRow( m_aRow, aSearchRow ) )
                        {
                            (m_pKeySet->get())[i] = 0;   // Marker for later to be removed
                        }
                        else
                        {
                            // They don't match, so it's not a duplicate.
                            // Use the current Row as the next one to match against
                            *aSearchRow = *m_aRow;
                        }
                    }
                    // Now remove any keys marked with a 0
                    m_pKeySet->get().erase(::std::remove_if(m_pKeySet->get().begin(),m_pKeySet->get().end()
                                    ,::std::bind2nd(::std::equal_to<sal_Int32>(),0))
                                     ,m_pKeySet->get().end());

                }
            }
        }   break;

        case SQL_STATEMENT_UPDATE:
        case SQL_STATEMENT_DELETE:
        case SQL_STATEMENT_INSERT:
            break;
        default:
            m_pStatement->getOwnConnection()->throwSQLException( STR_STMT_TYPE_NOT_SUPPORTED, *this );
            break;
    }
}

// -----------------------------------------------------------------------------


void OResultSet::setBoundedColumns(const OValueRow& _rRow,
                                   const ::rtl::Reference<connectivity::OSQLColumns>& _rxColumns,
                                   const Reference<XIndexAccess>& _xNames,
                                   sal_Bool _bSetColumnMapping,
                                   const Reference<XDatabaseMetaData>& _xMetaData,
                                   ::std::vector<sal_Int32>& _rColMapping)
{
    ::comphelper::UStringMixEqual aCase(_xMetaData->supportsMixedCaseQuotedIdentifiers());

    Reference<XPropertySet> xTableColumn;
    OUString sTableColumnName, sSelectColumnRealName;

    const OUString sName     = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);
    const OUString sRealName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME);

    ::std::vector< OUString> aColumnNames;
    aColumnNames.reserve(_rxColumns->get().size());
    OValueVector::Vector::iterator aRowIter = _rRow->get().begin()+1;
    for (sal_Int32 i=0; // the first column is the bookmark column
         aRowIter != _rRow->get().end();
            ++i, ++aRowIter
        )
    {
        try
        {
            // get the table column and it's name
            _xNames->getByIndex(i) >>= xTableColumn;
            OSL_ENSURE(xTableColumn.is(), "OResultSet::setBoundedColumns: invalid table column!");
            if (xTableColumn.is())
                xTableColumn->getPropertyValue(sName) >>= sTableColumnName;
            else
                sTableColumnName = OUString();

            // look if we have such a select column
            // TODO: would like to have a O(log n) search here ...
            sal_Int32 nColumnPos = 0;
            for (   OSQLColumns::Vector::iterator aIter = _rxColumns->get().begin();
                    aIter != _rxColumns->get().end();
                    ++aIter,++nColumnPos
                )
            {
                if ( nColumnPos < (sal_Int32)aColumnNames.size() )
                    sSelectColumnRealName = aColumnNames[nColumnPos];
                else
                {
                    if((*aIter)->getPropertySetInfo()->hasPropertyByName(sRealName))
                        (*aIter)->getPropertyValue(sRealName) >>= sSelectColumnRealName;
                    else
                        (*aIter)->getPropertyValue(sName) >>= sSelectColumnRealName;
                    aColumnNames.push_back(sSelectColumnRealName);
                }

                if (aCase(sTableColumnName, sSelectColumnRealName))
                {
                    if(_bSetColumnMapping)
                    {
                        sal_Int32 nSelectColumnPos = static_cast<sal_Int32>(aIter - _rxColumns->get().begin() + 1);
                            // the getXXX methods are 1-based ...
                        sal_Int32 nTableColumnPos = i + 1;
                            // get first table column is the bookmark column
                            // ...
#if OSL_DEBUG_LEVEL > 0
                        OSL_TRACE("Set Col Mapping: %d -> %d", nSelectColumnPos, nTableColumnPos );
#endif
                        _rColMapping[nSelectColumnPos] = nTableColumnPos;
                    }

                    aRowIter->setBound(sal_True);
                    aRowIter->setTypeKind(DataType::VARCHAR);
                }
            }
        }
        catch (Exception&)
        {
            OSL_FAIL("OResultSet::setBoundedColumns: caught an Exception!");
        }
    }
}


// -----------------------------------------------------------------------------
sal_Bool OResultSet::isCount() const
{
    return (m_pParseTree &&
            m_pParseTree->count() > 2                                                       &&
            SQL_ISRULE(m_pParseTree->getChild(2),scalar_exp_commalist)                      &&
            SQL_ISRULE(m_pParseTree->getChild(2)->getChild(0),derived_column)               &&
            SQL_ISRULE(m_pParseTree->getChild(2)->getChild(0)->getChild(0),general_set_fct) &&
            m_pParseTree->getChild(2)->getChild(0)->getChild(0)->count() == 4
            );
}

// -----------------------------------------------------------------------------
//
// Check for valid row in m_aQuery
//
sal_Bool OResultSet::validRow( sal_uInt32 nRow)
{
    sal_Int32  nNumberOfRecords = m_aQueryHelper.getResultCount();

    while ( nRow > (sal_uInt32)nNumberOfRecords && !m_aQueryHelper.queryComplete() ) {
#if OSL_DEBUG_LEVEL > 0
            OSL_TRACE("validRow: waiting...");
#endif
            if (m_aQueryHelper.checkRowAvailable( nRow ) == sal_False)
            {
                OSL_TRACE("validRow(%u): return False", nRow);
                return sal_False;
            }

            if ( m_aQueryHelper.hadError() )
            {
                m_pStatement->getOwnConnection()->throwSQLException( m_aQueryHelper.getError(), *this );
            }

            nNumberOfRecords = m_aQueryHelper.getResultCount();
    }

    if (( nRow == 0 ) ||
        ( nRow > (sal_uInt32)nNumberOfRecords && m_aQueryHelper.queryComplete()) ){
        OSL_TRACE("validRow(%u): return False", nRow);
        return sal_False;
    }
#if OSL_DEBUG_LEVEL > 0
    OSL_TRACE("validRow(%u): return True", nRow);
#endif

    return sal_True;
}
sal_Bool OResultSet::fillKeySet(sal_Int32 nMaxCardNumber)
{
    impl_ensureKeySet();
    if (m_CurrentRowCount < nMaxCardNumber)
    {
        sal_Int32   nKeyValue;
        if ( (sal_Int32)m_pKeySet->get().capacity() < nMaxCardNumber )
            m_pKeySet->get().reserve(nMaxCardNumber + 20 );

        for (nKeyValue = m_CurrentRowCount+1; nKeyValue  <= nMaxCardNumber; nKeyValue ++)
            m_pKeySet->get().push_back( nKeyValue );
        m_CurrentRowCount = nMaxCardNumber;
    }
    return sal_True;
}

sal_Int32 OResultSet::deletedCount()
{
    impl_ensureKeySet();
    return m_CurrentRowCount - static_cast<sal_Int32>(m_pKeySet->get().size());

}
// -----------------------------------------------------------------------------
sal_Bool OResultSet::seekRow( eRowPosition pos, sal_Int32 nOffset )
{
    SAL_INFO("connectivity.mork", "=> OResultSet::seekRow()" );

    ResultSetEntryGuard aGuard( *this );
    if ( !m_pKeySet.is() )
        m_pStatement->getOwnConnection()->throwSQLException( STR_ILLEGAL_MOVEMENT, *this );

    sal_Int32  nNumberOfRecords = m_aQueryHelper.getResultCount();
    sal_Int32  nRetrivedRows = currentRowCount();
    sal_Int32  nCurPos = m_nRowPos;

    OSL_TRACE("seekRow: nCurPos = %d", nCurPos );
    switch( pos ) {
        case NEXT_POS:
            OSL_TRACE("seekRow: NEXT");
            nCurPos++;
            break;
        case PRIOR_POS:
            OSL_TRACE("seekRow: PRIOR");
            if ( nCurPos > 0 )
                nCurPos--;
            break;

        case FIRST_POS:
            OSL_TRACE("seekRow: FIRST");
            nCurPos = 1;
            break;

        case LAST_POS:
            OSL_TRACE("seekRow: LAST");
            nCurPos = nRetrivedRows;
            break;
        case ABSOLUTE_POS:
            OSL_TRACE("seekRow: ABSOLUTE : %d", nOffset);
            nCurPos = nOffset;
            break;
        case RELATIVE_POS:
            OSL_TRACE("seekRow: RELATIVE : %d", nOffset);
            nCurPos += sal_uInt32( nOffset );
            break;
    }

    if ( nCurPos <= 0 ) {
        m_nRowPos = 0;
        OSL_TRACE("seekRow: return False, m_nRowPos = %u", m_nRowPos );
        return sal_False;
    }
    sal_Int32 nCurCard = nCurPos;
    if ( nCurPos < (sal_Int32)m_pKeySet->get().size() ) //The requested row is exist in m_pKeySet, so we just use it
    {
        nCurCard = (m_pKeySet->get())[nCurPos-1];
    }
    else    //The requested row has not been retrived until now. We should get the right card for it.
        nCurCard = nCurPos + deletedCount();

    if ( nCurCard > nNumberOfRecords) {
        fillKeySet(nNumberOfRecords);
        m_nRowPos = static_cast<sal_uInt32>(m_pKeySet->get().size() + 1);
        OSL_TRACE("seekRow: return False, m_nRowPos = %u", m_nRowPos );
        return sal_False;
    }
    //Insert new retrived items for later use
    fillKeySet(nNumberOfRecords);
    m_nRowPos = (sal_uInt32)nCurPos;
    OSL_TRACE("seekRow: return True, m_nRowPos = %u", m_nRowPos );
    fetchCurrentRow();
    return sal_True;
}
// -----------------------------------------------------------------------------
void OResultSet::setColumnMapping(const ::std::vector<sal_Int32>& _aColumnMapping)
{
    m_aColMapping = _aColumnMapping;
#if OSL_DEBUG_LEVEL > 0
    for ( sal_uInt32 i = 0; i < m_aColMapping.size(); i++ )
        OSL_TRACE("Set Mapped: %d -> %d", i, m_aColMapping[i] );
#endif
}
// -----------------------------------------------------------------------------

::com::sun::star::uno::Any OResultSet::getBookmark(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    OSL_TRACE("getBookmark, m_nRowPos = %u", m_nRowPos );
    ResultSetEntryGuard aGuard( *this );
    if ( fetchCurrentRow() == sal_False ) {
        m_pStatement->getOwnConnection()->throwSQLException( STR_ERROR_GET_ROW, *this );
    }

    OSL_ENSURE((!m_aRow->isDeleted()),"getBookmark called for deleted row");
    return makeAny((sal_Int32)(m_aRow->get())[0]);
}
sal_Bool  OResultSet::moveToBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    OSL_TRACE("moveToBookmark, bookmark = %u", comphelper::getINT32(bookmark) );
    sal_Int32 nCardNum = comphelper::getINT32(bookmark);
    m_nRowPos = getRowForCardNumber(nCardNum);
    fetchCurrentRow();
    return sal_True;
}
sal_Bool  OResultSet::moveRelativeToBookmark( const ::com::sun::star::uno::Any& bookmark, sal_Int32 rows ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    OSL_TRACE("moveRelativeToBookmark, bookmark = %u rows= %u", comphelper::getINT32(bookmark),rows );
    sal_Int32 nCardNum = comphelper::getINT32(bookmark);
    m_nRowPos = getRowForCardNumber(nCardNum);
    return seekRow(RELATIVE_POS,rows );
}
sal_Int32 OResultSet::compareBookmarks( const ::com::sun::star::uno::Any& lhs, const ::com::sun::star::uno::Any& rhs ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    OSL_TRACE("compareBookmarks, m_nRowPos = %u", m_nRowPos );
        sal_Int32 nFirst=0;
        sal_Int32 nSecond=0;
        sal_Int32 nResult=0;

        if ( !( lhs >>= nFirst ) || !( rhs >>= nSecond ) ) {
            m_pStatement->getOwnConnection()->throwSQLException( STR_INVALID_BOOKMARK, *this );
        }

    if(nFirst < nSecond)
         nResult = -1;
    else if(nFirst > nSecond)
         nResult = 1;
    else
         nResult = 0;

    return  nResult;
}
sal_Bool OResultSet::hasOrderedBookmarks(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    OSL_TRACE("hasOrderedBookmarks, m_nRowPos = %u", m_nRowPos );
    return sal_True;
}
sal_Int32 OResultSet::hashBookmark( const ::com::sun::star::uno::Any& bookmark ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    OSL_TRACE("hashBookmark, m_nRowPos = %u", m_nRowPos );
    return  comphelper::getINT32(bookmark);
}

sal_Int32 OResultSet::getCurrentCardNumber()
{
    if ( ( m_nRowPos == 0 ) || !m_pKeySet.is() )
        return 0;
    if (m_pKeySet->get().size() < m_nRowPos)
        return 0;
    return (m_pKeySet->get())[m_nRowPos-1];
}
void OResultSet::checkPendingUpdate() throw(SQLException, RuntimeException)
{
    OSL_FAIL( "OResultSet::checkPendingUpdate() not implemented" );
/*
    OSL_TRACE("checkPendingUpdate, m_nRowPos = %u", m_nRowPos );
    const sal_Int32 nCurrentRow = getCurrentCardNumber();

    if ((m_nNewRow && nCurrentRow != m_nNewRow)
        || ( m_nUpdatedRow && m_nUpdatedRow != nCurrentRow))
    {
        const OUString sError( m_pStatement->getOwnConnection()->getResources().getResourceStringWithSubstitution(
                STR_COMMIT_ROW,
                "$position$", OUString::valueOf(nCurrentRow)
             ) );
        ::dbtools::throwGenericSQLException(sError,*this);
    }
*/

}
void OResultSet::updateValue(sal_Int32 columnIndex ,const ORowSetValue& x) throw(SQLException, RuntimeException)
{
    OSL_TRACE("updateValue, m_nRowPos = %u", m_nRowPos );
    ResultSetEntryGuard aGuard( *this );
    if ( fetchCurrentRow() == sal_False ) {
        m_pStatement->getOwnConnection()->throwSQLException( STR_ERROR_GET_ROW, *this );
    }

    checkPendingUpdate();

    checkIndex(columnIndex );
    columnIndex = mapColumn(columnIndex);

    (m_aRow->get())[columnIndex].setBound(sal_True);
    (m_aRow->get())[columnIndex] = x;
    m_nUpdatedRow = getCurrentCardNumber();
//    m_RowStates = m_RowStates | RowStates_Updated;
}
// -----------------------------------------------------------------------------

void SAL_CALL OResultSet::updateNull( sal_Int32 columnIndex ) throw(SQLException, RuntimeException)
{
    OSL_TRACE("updateNull, m_nRowPos = %u", m_nRowPos );
    ResultSetEntryGuard aGuard( *this );
    if ( fetchCurrentRow() == sal_False )
        m_pStatement->getOwnConnection()->throwSQLException( STR_ERROR_GET_ROW, *this );

    checkPendingUpdate();
    checkIndex(columnIndex );
    columnIndex = mapColumn(columnIndex);

    (m_aRow->get())[columnIndex].setBound(sal_True);
    (m_aRow->get())[columnIndex].setNull();
    m_nUpdatedRow = getCurrentCardNumber();
//    m_RowStates = m_RowStates | RowStates_Updated;
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex, static_cast<bool>(x));
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateByte( sal_Int32 columnIndex, sal_Int8 x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateShort( sal_Int32 columnIndex, sal_Int16 x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateInt( sal_Int32 columnIndex, sal_Int32 x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateLong( sal_Int32 /*columnIndex*/, sal_Int64 /*x*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XRowUpdate::updateLong", *this );
}
// -----------------------------------------------------------------------
void SAL_CALL OResultSet::updateFloat( sal_Int32 columnIndex, float x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateDouble( sal_Int32 columnIndex, double x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateString( sal_Int32 columnIndex, const OUString& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateBytes( sal_Int32 columnIndex, const Sequence< sal_Int8 >& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateDate( sal_Int32 columnIndex, const ::com::sun::star::util::Date& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateTime( sal_Int32 columnIndex, const ::com::sun::star::util::Time& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateTimestamp( sal_Int32 columnIndex, const ::com::sun::star::util::DateTime& x ) throw(SQLException, RuntimeException)
{
    updateValue(columnIndex,x);
}
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateBinaryStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );

    if(!x.is())
        ::dbtools::throwFunctionSequenceException(*this);

    Sequence<sal_Int8> aSeq;
    x->readBytes(aSeq,length);
    updateValue(columnIndex,aSeq);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateCharacterStream( sal_Int32 columnIndex, const Reference< ::com::sun::star::io::XInputStream >& x, sal_Int32 length ) throw(SQLException, RuntimeException)
{
    updateBinaryStream(columnIndex,x,length);
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateObject( sal_Int32 columnIndex, const Any& x ) throw(SQLException, RuntimeException)
{
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
    {
        const OUString sError( m_pStatement->getOwnConnection()->getResources().getResourceStringWithSubstitution(
                STR_COLUMN_NOT_UPDATEABLE,
                "$position$", OUString::valueOf(columnIndex)
             ) );
        ::dbtools::throwGenericSQLException(sError,*this);
    } // if (!::dbtools::implUpdateObject(this, columnIndex, x))
 }
// -------------------------------------------------------------------------

void SAL_CALL OResultSet::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 /*scale*/ ) throw(SQLException, RuntimeException)
{
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
    {
        const OUString sError( m_pStatement->getOwnConnection()->getResources().getResourceStringWithSubstitution(
                STR_COLUMN_NOT_UPDATEABLE,
                "$position$", OUString::valueOf(columnIndex)
             ) );
        ::dbtools::throwGenericSQLException(sError,*this);
    }
}

// XResultSetUpdate
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::insertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    OSL_TRACE("insertRow in, m_nRowPos = %u", m_nRowPos );
//    m_RowStates = RowStates_Inserted;
    updateRow();
    m_nOldRowPos = 0;
    m_nNewRow = 0;
    //m_aQueryHelper.setRowStates(getCurrentCardNumber(),m_RowStates);
    OSL_TRACE("insertRow out, m_nRowPos = %u", m_nRowPos );
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::updateRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    OSL_FAIL( "OResultSet::updateRow(  ) not implemented" );
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::deleteRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    OSL_FAIL( "OResultSet::deleteRow(  ) not implemented" );
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::cancelRowUpdates(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    OSL_FAIL( "OResultSet::cancelRowUpdates(  ) not implemented" );
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::moveToInsertRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    OSL_FAIL( "OResultSet::moveToInsertRow(  ) not implemented" );
}
// -------------------------------------------------------------------------
void SAL_CALL OResultSet::moveToCurrentRow(  ) throw(::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ResultSetEntryGuard aGuard( *this );
    OSL_TRACE("moveToCurrentRow, m_nRowPos = %u", m_nRowPos );
    if (rowInserted())
    {
        m_nRowPos = m_nOldRowPos;
        fetchCurrentRow();
    }
}

sal_Bool OResultSet::determineReadOnly()
{
//    OSL_FAIL( "OResultSet::determineReadOnly(  ) not implemented" );

    if (m_bIsReadOnly == -1)
    {
        m_bIsReadOnly = sal_True;
//        OConnection* xConnection = static_cast<OConnection*>(m_pStatement->getConnection().get());
//        m_bIsReadOnly = !m_aQueryHelper.isWritable(xConnection) || m_bIsAlwaysFalseQuery;
    }

    return m_bIsReadOnly != 0;
}

void OResultSet::setTable(OTable* _rTable)
{
    OSL_TRACE("In : setTable");
    m_pTable = _rTable;
    m_pTable->acquire();
    m_xTableColumns = m_pTable->getColumns();
    if(m_xTableColumns.is())
        m_aColumnNames = m_xTableColumns->getElementNames();
    OSL_TRACE("Out : setTable");
}

void OResultSet::setOrderByColumns(const ::std::vector<sal_Int32>& _aColumnOrderBy)
{
    m_aOrderbyColumnNumber = _aColumnOrderBy;
}

void OResultSet::setOrderByAscending(const ::std::vector<TAscendingOrder>& _aOrderbyAsc)
{
    m_aOrderbyAscending = _aOrderbyAsc;
}
Sequence< sal_Int32 > SAL_CALL OResultSet::deleteRows( const Sequence< Any >& /*rows*/ ) throw(SQLException, RuntimeException)
{
    ::dbtools::throwFeatureNotImplementedException( "XDeleteRows::deleteRows", *this );
    return Sequence< sal_Int32 >();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
