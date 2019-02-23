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

#include <com/sun/star/sdbc/ResultSetType.hpp>
#include <com/sun/star/sdbc/FetchDirection.hpp>
#include <com/sun/star/sdbc/ResultSetConcurrency.hpp>
#include <com/sun/star/sdbcx/CompareBookmark.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <connectivity/dbtools.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <sal/log.hxx>

#include <vector>
#include <algorithm>
#include "MResultSet.hxx"
#include <sqlbison.hxx>
#include "MResultSetMetaData.hxx"
#include <FDatabaseMetaDataResultSet.hxx>

#include <strings.hrc>

using namespace ::comphelper;
using namespace connectivity;
using namespace connectivity::mork;
using namespace ::cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::sdbcx;
using namespace com::sun::star::container;
using namespace com::sun::star::io;
using namespace com::sun::star::util;


//  IMPLEMENT_SERVICE_INFO(OResultSet,"com.sun.star.sdbcx.OResultSet","com.sun.star.sdbc.ResultSet");
OUString SAL_CALL OResultSet::getImplementationName(  )
{
    return OUString("com.sun.star.sdbcx.mork.ResultSet");
}

 Sequence< OUString > SAL_CALL OResultSet::getSupportedServiceNames(  )
{
   return {"com.sun.star.sdbc.ResultSet","com.sun.star.sdbcx.ResultSet"};
}

sal_Bool SAL_CALL OResultSet::supportsService( const OUString& _rServiceName )
{
    return cppu::supportsService(this, _rServiceName);
}


OResultSet::OResultSet(OCommonStatement* pStmt, const std::shared_ptr< connectivity::OSQLParseTreeIterator >& _pSQLIterator )
    : OResultSet_BASE(m_aMutex)
    ,OPropertySetHelper(OResultSet_BASE::rBHelper)
    ,m_pStatement(pStmt)
    ,m_xStatement(*pStmt)
    ,m_nRowPos(0)
    ,m_bWasNull(false)
    ,m_nResultSetType(ResultSetType::SCROLL_INSENSITIVE)
    ,m_nFetchDirection(FetchDirection::FORWARD)
    ,m_pSQLIterator( _pSQLIterator )
    ,m_pParseTree( _pSQLIterator->getParseTree() )
    ,m_aQueryHelper(pStmt->getOwnConnection()->getColumnAlias())
    ,m_CurrentRowCount(0)
    ,m_nParamIndex(0)
    ,m_bIsAlwaysFalseQuery(false)
    ,m_bIsReadOnly(TRISTATE_INDET)
{
    //m_aQuery.setMaxNrOfReturns(pStmt->getOwnConnection()->getMaxResultRecords());
}

OResultSet::~OResultSet()
{
}


void OResultSet::disposing()
{
    OPropertySetHelper::disposing();

    ::osl::MutexGuard aGuard(m_aMutex);

    m_xStatement.clear();
    m_xMetaData.clear();
    m_pParseTree    = nullptr;
    m_xColumns = nullptr;
    m_pKeySet       = nullptr;
    m_xTable.clear();
}

Any SAL_CALL OResultSet::queryInterface( const Type & rType )
{
    Any aRet = OPropertySetHelper::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OResultSet_BASE::queryInterface(rType);
    return aRet;
}

 Sequence<  Type > SAL_CALL OResultSet::getTypes(  )
{
    OTypeCollection aTypes( cppu::UnoType<css::beans::XMultiPropertySet>::get(),
                                                cppu::UnoType<css::beans::XFastPropertySet>::get(),
                                                cppu::UnoType<css::beans::XPropertySet>::get());

    return ::comphelper::concatSequences(aTypes.getTypes(),OResultSet_BASE::getTypes());
}

void OResultSet::methodEntry()
{
    checkDisposed(OResultSet_BASE::rBHelper.bDisposed);
    if ( !m_xTable.is() )
    {
        OSL_FAIL( "OResultSet::methodEntry: looks like we're disposed, but how is this possible?" );
        throw DisposedException( OUString(), *this );
    }
}


sal_Int32 SAL_CALL OResultSet::findColumn( const OUString& columnName )
{
    ResultSetEntryGuard aGuard( *this );

    // find the first column with the name columnName
    Reference< XResultSetMetaData > xMeta = getMetaData();
    sal_Int32 nLen = xMeta->getColumnCount();
    sal_Int32 i = 1;
    for(;i<=nLen;++i)
    {
        if(xMeta->isCaseSensitive(i) ? columnName == xMeta->getColumnName(i) :
                columnName.equalsIgnoreAsciiCase(xMeta->getColumnName(i)))
            return i;
    }

    ::dbtools::throwInvalidColumnException( columnName, *this );
    assert(false);
    return 0; // Never reached
}

Reference< XInputStream > SAL_CALL OResultSet::getBinaryStream( sal_Int32 /*columnIndex*/ )
{
    return nullptr;
}

Reference< XInputStream > SAL_CALL OResultSet::getCharacterStream( sal_Int32 /*columnIndex*/ )
{
    return nullptr;
}


sal_Bool SAL_CALL OResultSet::getBoolean( sal_Int32 /*columnIndex*/ )
{
    ResultSetEntryGuard aGuard( *this );
    m_bWasNull = true;
    return false;
}


sal_Int8 SAL_CALL OResultSet::getByte( sal_Int32 /*columnIndex*/ )
{
    return 0;
}


Sequence< sal_Int8 > SAL_CALL OResultSet::getBytes( sal_Int32 /*columnIndex*/ )
{
    return Sequence< sal_Int8 >();
}


Date SAL_CALL OResultSet::getDate( sal_Int32 /*columnIndex*/ )
{
    return Date();
}


double SAL_CALL OResultSet::getDouble( sal_Int32 /*columnIndex*/ )
{
    return 0.0;
}


float SAL_CALL OResultSet::getFloat( sal_Int32 /*columnIndex*/ )
{
    return 0;
}


sal_Int32 SAL_CALL OResultSet::getInt( sal_Int32 /*columnIndex*/ )
{
    return 0;
}


sal_Int32 SAL_CALL OResultSet::getRow(  )
{
    ResultSetEntryGuard aGuard( *this );

    SAL_INFO("connectivity.mork", "return = " << m_nRowPos);
    return m_nRowPos;
}


sal_Int64 SAL_CALL OResultSet::getLong( sal_Int32 /*columnIndex*/ )
{
    return sal_Int64();
}


Reference< XResultSetMetaData > SAL_CALL OResultSet::getMetaData(  )
{
    ResultSetEntryGuard aGuard( *this );

    if(!m_xMetaData.is())
        m_xMetaData = new OResultSetMetaData(
        m_pSQLIterator->getSelectColumns(), m_pSQLIterator->getTables().begin()->first, m_xTable.get(), determineReadOnly());
    return m_xMetaData;
}

Reference< XArray > SAL_CALL OResultSet::getArray( sal_Int32 /*columnIndex*/ )
{
    return nullptr;
}


Reference< XClob > SAL_CALL OResultSet::getClob( sal_Int32 /*columnIndex*/ )
{
    return nullptr;
}

Reference< XBlob > SAL_CALL OResultSet::getBlob( sal_Int32 /*columnIndex*/ )
{
    return nullptr;
}


Reference< XRef > SAL_CALL OResultSet::getRef( sal_Int32 /*columnIndex*/ )
{
    return nullptr;
}


Any SAL_CALL OResultSet::getObject( sal_Int32 /*columnIndex*/, const Reference< css::container::XNameAccess >& /*typeMap*/ )
{
    return Any();
}


sal_Int16 SAL_CALL OResultSet::getShort( sal_Int32 /*columnIndex*/ )
{
    return 0;
}


void OResultSet::checkIndex(sal_Int32 columnIndex )
{
    if(columnIndex <= 0 || columnIndex > static_cast<sal_Int32>(m_xColumns->get().size()))
        ::dbtools::throwInvalidIndexException(*this);
}

sal_uInt32 OResultSet::currentRowCount()
{
    if ( m_bIsAlwaysFalseQuery )
        return 0;
    //return 0;//m_aQuery.getRealRowCount() - deletedCount();
    // new implementation
    return m_aQueryHelper.getResultCount();
}


bool OResultSet::fetchCurrentRow( )
{
    SAL_INFO("connectivity.mork", "m_nRowPos = " << m_nRowPos);
    return fetchRow(getCurrentCardNumber());
}


bool OResultSet::fetchRow(sal_Int32 cardNumber,bool bForceReload)
{
    SAL_INFO("connectivity.mork", "cardNumber = " << cardNumber);
    if (!bForceReload)
    {
        // Check whether we've already fetched the row...
        if ( !(m_aRow->get())[0].isNull() && static_cast<sal_Int32>((m_aRow->get())[0]) == cardNumber )
            return true;
    }
//    else
//        m_aQuery.resyncRow(cardNumber);

    if ( !validRow( cardNumber ) )
        return false;

    (m_aRow->get())[0] = cardNumber;
    sal_Int32 nCount = m_aColumnNames.getLength();
    //m_RowStates = m_aQuery.getRowStates(cardNumber);
    for( sal_Int32 i = 1; i <= nCount; i++ )
    {
        if ( (m_aRow->get())[i].isBound() )
        {

            // Everything in the addressbook is a string!

            if ( !m_aQueryHelper.getRowValue( (m_aRow->get())[i], cardNumber, m_aColumnNames[i-1], DataType::VARCHAR ))
            {
                m_pStatement->getOwnConnection()->throwSQLException( m_aQueryHelper.getError(), *this );
            }
        }
    }
    return true;

}


const ORowSetValue& OResultSet::getValue(sal_Int32 cardNumber, sal_Int32 columnIndex )
{
    if ( !fetchRow( cardNumber ) )
    {
        OSL_FAIL("fetchRow() returned False" );
        m_bWasNull = true;
        return *ODatabaseMetaDataResultSet::getEmptyValue();
    }

    m_bWasNull = (m_aRow->get())[columnIndex].isNull();
    return (m_aRow->get())[columnIndex];

}


OUString SAL_CALL OResultSet::getString( sal_Int32 columnIndex )
{
    ResultSetEntryGuard aGuard( *this );

    OSL_ENSURE(m_xColumns.is(), "Need the Columns!!");
    OSL_ENSURE(columnIndex <= static_cast<sal_Int32>(m_xColumns->get().size()), "Trying to access invalid columns number");
    checkIndex( columnIndex );

    // If this query was sorted then we should have a valid KeySet, so use it
    return getValue(getCurrentCardNumber(), mapColumn( columnIndex ) );

}


Time SAL_CALL OResultSet::getTime( sal_Int32 /*columnIndex*/ )
{
    ResultSetEntryGuard aGuard( *this );
    return Time();
}


DateTime SAL_CALL OResultSet::getTimestamp( sal_Int32 /*columnIndex*/ )
{
    ResultSetEntryGuard aGuard( *this );
    return DateTime();
}


sal_Bool SAL_CALL OResultSet::isBeforeFirst(  )
{
    ResultSetEntryGuard aGuard( *this );

    // here you have to implement your movements
    // return true means there is no data
    return( m_nRowPos < 1 );
}

sal_Bool SAL_CALL OResultSet::isAfterLast(  )
{
    SAL_WARN("connectivity.mork", "OResultSet::isAfterLast() NOT IMPLEMENTED!");
    ResultSetEntryGuard aGuard( *this );

    return m_nRowPos > currentRowCount();
}

sal_Bool SAL_CALL OResultSet::isFirst(  )
{
    ResultSetEntryGuard aGuard( *this );

    return m_nRowPos == 1;
}

sal_Bool SAL_CALL OResultSet::isLast(  )
{
    SAL_WARN("connectivity.mork", "OResultSet::isLast() NOT IMPLEMENTED!");
    ResultSetEntryGuard aGuard( *this );

//    return sal_True;
    return m_nRowPos == currentRowCount();
}

void SAL_CALL OResultSet::beforeFirst(  )
{
    ResultSetEntryGuard aGuard( *this );

    // move before the first row so that isBeforeFirst returns false
    if ( first() )
        previous();
}

void SAL_CALL OResultSet::afterLast(  )
{
    ResultSetEntryGuard aGuard( *this );

    if(last())
        next();
}


void SAL_CALL OResultSet::close()
{
    dispose();
}


sal_Bool SAL_CALL OResultSet::first(  )
{
    return seekRow( FIRST_POS );
}


sal_Bool SAL_CALL OResultSet::last(  )
{
    return seekRow( LAST_POS );
}

sal_Bool SAL_CALL OResultSet::absolute( sal_Int32 row )
{
    return seekRow( ABSOLUTE_POS, row );
}

sal_Bool SAL_CALL OResultSet::relative( sal_Int32 row )
{
    return seekRow( RELATIVE_POS, row );
}

sal_Bool SAL_CALL OResultSet::previous(  )
{
    ResultSetEntryGuard aGuard( *this );
    return seekRow( PRIOR_POS );
}

Reference< XInterface > SAL_CALL OResultSet::getStatement(  )
{
    ResultSetEntryGuard aGuard( *this );
    return m_xStatement;
}


sal_Bool SAL_CALL OResultSet::rowDeleted(  )
{
    SAL_WARN("connectivity.mork", "OResultSet::rowDeleted() NOT IMPLEMENTED!");
    ResultSetEntryGuard aGuard( *this );
    return true;//return ((m_RowStates & RowStates_Deleted) == RowStates_Deleted) ;
}

sal_Bool SAL_CALL OResultSet::rowInserted(  )
{
    SAL_WARN("connectivity.mork", "OResultSet::rowInserted() NOT IMPLEMENTED!");
    ResultSetEntryGuard aGuard( *this );
    return true;//return ((m_RowStates & RowStates_Inserted) == RowStates_Inserted);
}

sal_Bool SAL_CALL OResultSet::rowUpdated(  )
{
    SAL_WARN("connectivity.mork", "OResultSet::rowUpdated() NOT IMPLEMENTED!");
    ResultSetEntryGuard aGuard( *this );
    return true;// return ((m_RowStates & RowStates_Updated) == RowStates_Updated) ;
}


sal_Bool SAL_CALL OResultSet::next(  )
{
    return seekRow( NEXT_POS );
}


sal_Bool SAL_CALL OResultSet::wasNull(  )
{
    ResultSetEntryGuard aGuard( *this );

    return m_bWasNull;
}


void SAL_CALL OResultSet::cancel(  )
{
}

void SAL_CALL OResultSet::clearWarnings(  )
{
}

Any SAL_CALL OResultSet::getWarnings(  )
{
    return Any();
}

void SAL_CALL OResultSet::refreshRow(  )
{
    if (fetchRow(getCurrentCardNumber(),true)) {
        //force fetch current row will cause we lose all change to the current row
        m_pStatement->getOwnConnection()->throwSQLException( STR_ERROR_REFRESH_ROW, *this );
    }
}

IPropertyArrayHelper* OResultSet::createArrayHelper( ) const
{
    Sequence< Property > aProps(5);
    Property* pProperties = aProps.getArray();
    sal_Int32 nPos = 0;
    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHDIRECTION),
        PROPERTY_ID_FETCHDIRECTION, cppu::UnoType<sal_Int32>::get(), 0);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_FETCHSIZE),
        PROPERTY_ID_FETCHSIZE, cppu::UnoType<sal_Int32>::get(), 0);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_ISBOOKMARKABLE),
        PROPERTY_ID_ISBOOKMARKABLE, cppu::UnoType<bool>::get(), PropertyAttribute::READONLY);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETCONCURRENCY),
        PROPERTY_ID_RESULTSETCONCURRENCY, cppu::UnoType<sal_Int32>::get(), PropertyAttribute::READONLY);

    pProperties[nPos++] = css::beans::Property(::connectivity::OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_RESULTSETTYPE),
        PROPERTY_ID_RESULTSETTYPE, cppu::UnoType<sal_Int32>::get(), PropertyAttribute::READONLY);

    return new OPropertyArrayHelper(aProps);
}

IPropertyArrayHelper & OResultSet::getInfoHelper()
{
    return *getArrayHelper();
}

sal_Bool OResultSet::convertFastPropertyValue(
                            Any & /*rConvertedValue*/,
                            Any & /*rOldValue*/,
                            sal_Int32 nHandle,
                            const Any& /*rValue*/ )
{
    OSL_FAIL( "OResultSet::convertFastPropertyValue: not implemented!" );
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw css::lang::IllegalArgumentException();
        case PROPERTY_ID_FETCHDIRECTION:
        case PROPERTY_ID_FETCHSIZE:
        default:
            ;
    }
    return false;
}

void OResultSet::setFastPropertyValue_NoBroadcast(
                                sal_Int32 nHandle,
                                const Any& /*rValue*/
                                                 )
{
    OSL_FAIL( "OResultSet::setFastPropertyValue_NoBroadcast: not implemented!" );
    switch(nHandle)
    {
        case PROPERTY_ID_ISBOOKMARKABLE:
        case PROPERTY_ID_RESULTSETCONCURRENCY:
        case PROPERTY_ID_RESULTSETTYPE:
            throw Exception("cannot set prop " + OUString::number(nHandle), nullptr);
        case PROPERTY_ID_FETCHDIRECTION:
            break;
        case PROPERTY_ID_FETCHSIZE:
            break;
        default:
            ;
    }
}

void OResultSet::getFastPropertyValue(
                                Any& rValue,
                                sal_Int32 nHandle
                                     ) const
{
    switch(nHandle)
    {
        case PROPERTY_ID_RESULTSETCONCURRENCY:
            rValue <<= sal_Int32(ResultSetConcurrency::UPDATABLE);
            break;
        case PROPERTY_ID_RESULTSETTYPE:
            rValue <<= m_nResultSetType;
            break;
        case PROPERTY_ID_FETCHDIRECTION:
            rValue <<= m_nFetchDirection;
            break;
        case PROPERTY_ID_FETCHSIZE:
            rValue <<= sal_Int32(0);
            break;
        case PROPERTY_ID_ISBOOKMARKABLE:
            const_cast< OResultSet* >( this )->determineReadOnly();
            rValue <<= (m_bIsReadOnly == TRISTATE_FALSE);
         break;
    }
}

void SAL_CALL OResultSet::acquire() throw()
{
    OResultSet_BASE::acquire();
}

void SAL_CALL OResultSet::release() throw()
{
    OResultSet_BASE::release();
}

css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL OResultSet::getPropertySetInfo(  )
{
    return ::cppu::OPropertySetHelper::createPropertySetInfo(getInfoHelper());
}


void OResultSet::parseParameter( const OSQLParseNode* pNode, OUString& rMatchString )
{
    OSL_ENSURE(pNode->count() > 0,"Error parsing parameter in Parse Tree");
    OSQLParseNode *pMark = pNode->getChild(0);

    // Initialize to empty string
    rMatchString.clear();

    OUString aParameterName;
    if (SQL_ISPUNCTUATION(pMark,"?")) {
        aParameterName = "?";
    }
    else if (SQL_ISPUNCTUATION(pMark,":")) {
        aParameterName = pNode->getChild(1)->getTokenValue();
    }
    // XXX - Now we know name, what's value????
    m_nParamIndex ++;
    SAL_INFO(
        "connectivity.mork",
        "Parameter name [" << m_nParamIndex << "]: " << aParameterName);

    if ( m_aParameterRow.is() ) {
        OSL_ENSURE( m_nParamIndex < static_cast<sal_Int32>(m_aParameterRow->get().size()) + 1, "More parameters than values found" );
        rMatchString = (m_aParameterRow->get())[static_cast<sal_uInt16>(m_nParamIndex)];
        SAL_INFO("connectivity.mork", "Prop Value: " << rMatchString);
    }
    else {
        SAL_INFO("connectivity.mork", "Prop Value: Invalid ParameterRow!");
    }
}

#define WILDCARD "%"
#define ALT_WILDCARD "*"
static const sal_Unicode MATCHCHAR = '_';

void OResultSet::analyseWhereClause( const OSQLParseNode*                 parseTree,
                                     MQueryExpression                     &queryExpression)
{
    OUString         columnName;
    MQueryOp::cond_type     op( MQueryOp::Is );
    OUString         matchString;

    if ( parseTree == nullptr )
        return;

    if ( m_pSQLIterator->getParseTree() != nullptr ) {
        ::rtl::Reference<OSQLColumns> xColumns = m_pSQLIterator->getParameters();
        if(xColumns.is())
        {
            OUString aColName, aParameterValue;
            sal_Int32 i = 1;
            for (auto const& column : xColumns->get())
            {
                column->getPropertyValue(OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME)) >>= aColName;
                SAL_INFO("connectivity.mork", "Prop Column Name: " << aColName);
                if ( m_aParameterRow.is() ) {
                    aParameterValue = (m_aParameterRow->get())[static_cast<sal_uInt16>(i)];
                    SAL_INFO("connectivity.mork", "Prop Value: " << aParameterValue);
                }
                else {
                    SAL_INFO("connectivity.mork", "Prop Value: Invalid ParameterRow!");
                }
                i++;
            }
        }

    }

    if ( SQL_ISRULE(parseTree,where_clause) )
    {
        // Reset Parameter Counter
        resetParameters();
        analyseWhereClause( parseTree->getChild( 1 ), queryExpression );
    }
    else if ( parseTree->count() == 3 &&                         // Handle ()'s
        SQL_ISPUNCTUATION(parseTree->getChild(0),"(") &&
        SQL_ISPUNCTUATION(parseTree->getChild(2),")"))
    {
        MQueryExpression *subExpression = new MQueryExpression();
        analyseWhereClause( parseTree->getChild( 1 ), *subExpression );
        queryExpression.addExpression( subExpression );
    }
    else if ((SQL_ISRULE(parseTree,search_condition) || SQL_ISRULE(parseTree,boolean_term))
             && parseTree->count() == 3)                   // Handle AND/OR
    {
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
          parseTree->getChild(2)->getNodeType() == SQLNodeType::String ||
          parseTree->getChild(2)->getNodeType() == SQLNodeType::IntNum ||
          parseTree->getChild(2)->getNodeType() == SQLNodeType::ApproxNum ||
          SQL_ISTOKEN(parseTree->getChild(2),TRUE) ||
          SQL_ISTOKEN(parseTree->getChild(2),FALSE) ||
          SQL_ISRULE(parseTree->getChild(2),parameter) ||
          // odbc date
          (SQL_ISRULE(parseTree->getChild(2),set_fct_spec) && SQL_ISPUNCTUATION(parseTree->getChild(2)->getChild(0),"{"))))
        {
            m_pStatement->getOwnConnection()->throwSQLException( STR_QUERY_TOO_COMPLEX, *this );
        }

        OSQLParseNode *pPrec = parseTree->getChild(1);
        if (pPrec->getNodeType() == SQLNodeType::Equal)
            op = MQueryOp::Is;
        else if (pPrec->getNodeType() == SQLNodeType::NotEqual)
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

        if ( columnName == "0" && op == MQueryOp::Is && matchString == "1" ) {
            m_bIsAlwaysFalseQuery = true;
        }
        queryExpression.addExpression( new MQueryExpressionString( columnName, op, matchString ));
    }
    else if (SQL_ISRULE(parseTree,like_predicate))
    {
        OSL_ENSURE(parseTree->count() == 2, "Error parsing LIKE predicate");

        if ( !SQL_ISRULE(parseTree->getChild(0), column_ref) )
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

        if (!(pAtom->getNodeType() == SQLNodeType::String ||
              pAtom->getNodeType() == SQLNodeType::Name ||
              SQL_ISRULE(pAtom,parameter) ||
              ( pAtom->getChild(0) && pAtom->getChild(0)->getNodeType() == SQLNodeType::Name ) ||
              ( pAtom->getChild(0) && pAtom->getChild(0)->getNodeType() == SQLNodeType::String )
              ) )
        {
            m_pStatement->getOwnConnection()->throwSQLException( STR_QUERY_INVALID_LIKE_STRING, *this );
        }

        OUString sTableRange;
        if(SQL_ISRULE(pColumn,column_ref))
            m_pSQLIterator->getColumnRange(pColumn,columnName,sTableRange);

        SAL_INFO("connectivity.mork", "ColumnName = " << columnName);

        if ( SQL_ISRULE(pAtom,parameter) ) {
            parseParameter( pAtom, matchString );
            // Replace all '*' with '%' : UI Usually does this but not with
            // Parameters for some reason.
            matchString = matchString.replaceAll( ALT_WILDCARD, WILDCARD );
        }
        else
        {
            matchString = pAtom->getTokenValue();
        }

        // Determine where '%' character is...

        if ( matchString == WILDCARD )
        {
            // String containing only a '%' and nothing else
            op = MQueryOp::Exists;
            // Will be ignored for Exists case, but clear anyway.
            matchString.clear();
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
        else if (  matchString.startsWith( WILDCARD )
                   && matchString.endsWith( WILDCARD )
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
                if ( matchString.startsWith( WILDCARD ) )
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
                    matchString = matchString.replaceAt( pos, 1, ".*" );
                    op = MQueryOp::RegExp;
                }

            }
            else
            {
                // Most Complex, need to use an RE
                sal_Int32 pos;
                while ( (pos = matchString.indexOf ( WILDCARD )) != -1 )
                {
                    matchString = matchString.replaceAt( pos, 1, ".*" );
                }

                while ( (pos = matchString.indexOf( MATCHCHAR )) != -1 )
                {
                    matchString = matchString.replaceAt( pos, 1, "." );
                }

                op = MQueryOp::RegExp;
            }
        }

        queryExpression.addExpression( new MQueryExpressionString( columnName, op, matchString ));
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

        queryExpression.addExpression( new MQueryExpressionString( columnName, op ));
    }
    else
    {
        SAL_WARN("connectivity.mork",  "Unexpected statement!!!" );
        m_pStatement->getOwnConnection()->throwSQLException( STR_QUERY_TOO_COMPLEX, *this );
    }
}

void OResultSet::fillRowData()
{
    OSL_ENSURE( m_pStatement, "Require a statement" );

    MQueryExpression queryExpression;

    OConnection* pConnection = static_cast<OConnection*>(m_pStatement->getConnection().get());
    m_xColumns = m_pSQLIterator->getSelectColumns();

    OSL_ENSURE(m_xColumns.is(), "Need the Columns!!");

    const OUString sPropertyName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);
    OUString sName;
    sal_Int32 i = 1;
    for (const auto& rxColumn : m_xColumns->get())
    {
        rxColumn->getPropertyValue(sPropertyName) >>= sName;
        SAL_INFO(
            "connectivity.mork", "Query Columns : (" << i << ") " << sName);
        i++;
    }

    // Generate Match Conditions for Query
    const OSQLParseNode*  pParseTree = m_pSQLIterator->getWhereTree();

    m_bIsAlwaysFalseQuery = false;
    if ( pParseTree != nullptr )
    {
        // Extract required info

        analyseWhereClause( pParseTree, queryExpression );
    }
    // If the query is a 0=1 then set Row count to 0 and return
    if ( m_bIsAlwaysFalseQuery )
    {
        m_bIsReadOnly = TRISTATE_TRUE;
        return;
    }

    OUString aStr(  m_xTable->getName() );
    m_aQueryHelper.setAddressbook( aStr );

    sal_Int32 rv = m_aQueryHelper.executeQuery(pConnection, queryExpression);
    if ( rv == -1 ) {
        m_pStatement->getOwnConnection()->throwSQLException( STR_ERR_EXECUTING_QUERY, *this );
    }

    if (m_aQueryHelper.hadError())
    {
        m_pStatement->getOwnConnection()->throwSQLException( m_aQueryHelper.getError(), *this );
    }

    //determine whether the address book is readonly
    determineReadOnly();

    SAL_INFO("connectivity.mork", "executeQuery returned " << rv);
}


static bool matchRow( OValueRow const & row1, OValueRow const & row2 )
{
    // the first column is the bookmark column
    return std::equal(std::next(row1->get().begin()), row1->get().end(), std::next(row2->get().begin()),
        [](const ORowSetValue& a, const ORowSetValue& b) { return !a.isBound() || a == b; });
}

sal_Int32 OResultSet::getRowForCardNumber(sal_Int32 nCardNum)
{
    SAL_INFO("connectivity.mork", "nCardNum = " << nCardNum);

    if ( m_pKeySet.is() )
    {
        sal_Int32  nPos;
        for(nPos=0;nPos < static_cast<sal_Int32>(m_pKeySet->get().size());nPos++)
        {
            if (nCardNum == (m_pKeySet->get())[nPos])
            {
                SAL_INFO("connectivity.mork", "return = " << nPos+1);
                return nPos+1;
            }
        }
    }

    m_pStatement->getOwnConnection()->throwSQLException( STR_INVALID_BOOKMARK, *this );

    return 0;
}

void OResultSet::executeQuery()
{
    ResultSetEntryGuard aGuard( *this );

    OSL_ENSURE( m_xTable.is(), "Need a Table object");
    if(!m_xTable.is())
    {
        const OSQLTables& rTabs = m_pSQLIterator->getTables();
        if (rTabs.empty() || !rTabs.begin()->second.is())
            m_pStatement->getOwnConnection()->throwSQLException( STR_QUERY_TOO_COMPLEX, *this );

        m_xTable = static_cast< OTable* > (rTabs.begin()->second.get());
    }

    m_nRowPos = 0;

    fillRowData();

    OSL_ENSURE(m_xColumns.is(), "Need the Columns!!");

    switch( m_pSQLIterator->getStatementType() )
    {
        case OSQLStatementType::Select:
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
                bool bDistinct = false;
                OSQLParseNode *pDistinct = m_pParseTree->getChild(1);
                if (pDistinct && pDistinct->getTokenID() == SQL_TOKEN_DISTINCT)
                {
                    if(!IsSorted())
                    {
                        m_aOrderbyColumnNumber.push_back(m_aColMapping[1]);
                        m_aOrderbyAscending.push_back(TAscendingOrder::DESC);
                    }
                    bDistinct = true;
                }

                OSortIndex::TKeyTypeVector eKeyType(m_aOrderbyColumnNumber.size());
                std::vector<sal_Int16>::size_type index = 0;
                for (const auto& rColIndex : m_aOrderbyColumnNumber)
                {
                    OSL_ENSURE(static_cast<sal_Int32>(m_aRow->get().size()) > rColIndex,"Invalid Index");
                    switch ((m_aRow->get().begin()+rColIndex)->getTypeKind())
                    {
                    case DataType::CHAR:
                        case DataType::VARCHAR:
                            eKeyType[index] = OKeyType::String;
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
                            eKeyType[index] = OKeyType::Double;
                            break;

                    // Other types aren't implemented (so they are always FALSE)
                        default:
                            eKeyType[index] = OKeyType::NONE;
                            OSL_FAIL("MResultSet::executeQuery: Order By Data Type not implemented");
                            break;
                    }
                    ++index;
                }

                if (IsSorted())
                {
                    // Implement Sorting

                    // So that we can sort we need to wait until the executed
                    // query to the mozilla addressbooks has returned all
                    // values.

                    OSortIndex aSortIndex(eKeyType,m_aOrderbyAscending);

#if OSL_DEBUG_LEVEL > 0
                    for ( std::vector<sal_Int32>::size_type i = 0; i < m_aColMapping.size(); i++ )
                        SAL_INFO(
                            "connectivity.mork",
                            "Mapped: " << i << " -> " << m_aColMapping[i]);
#endif
                    for ( sal_Int32 nRow = 1; nRow <= m_aQueryHelper.getResultCount(); nRow++ ) {

                        std::unique_ptr<OKeyValue> pKeyValue = OKeyValue::createKeyValue(nRow);

                        for (const auto& rColIndex : m_aOrderbyColumnNumber)
                        {
                            const ORowSetValue& value = getValue(nRow, rColIndex);

                            SAL_INFO(
                                "connectivity.mork",
                                "Adding Value: (" << nRow << "," << rColIndex
                                    << ") : " << value.getString());

                            pKeyValue->pushKey(new ORowSetValueDecorator(value));
                        }

                        aSortIndex.AddKeyValue( std::move(pKeyValue) );
                    }

                    m_pKeySet = aSortIndex.CreateKeySet();
                    m_CurrentRowCount = static_cast<sal_Int32>(m_pKeySet->get().size());
#if OSL_DEBUG_LEVEL > 0
                    for( OKeySet::Vector::size_type i = 0; i < m_pKeySet->get().size(); i++ )
                        SAL_INFO(
                            "connectivity.mork",
                            "Sorted: " << i << " -> " << (m_pKeySet->get())[i]);
#endif

                    beforeFirst(); // Go back to start
                }
                else  //we always need m_pKeySet now
                    m_pKeySet = new OKeySet();

                // Handle the DISTINCT case
                if ( bDistinct && m_pKeySet.is() )
                {
                    OValueRow aSearchRow = new OValueVector( m_aRow->get().size() );

                    for(sal_Int32 & i : m_pKeySet->get())
                    {
                        fetchRow( i );        // Fills m_aRow
                        if ( matchRow( m_aRow, aSearchRow ) )
                        {
                            i = 0;   // Marker for later to be removed
                        }
                        else
                        {
                            // They don't match, so it's not a duplicate.
                            // Use the current Row as the next one to match against
                            *aSearchRow = *m_aRow;
                        }
                    }
                    // Now remove any keys marked with a 0
                    m_pKeySet->get().erase(std::remove_if(m_pKeySet->get().begin(),m_pKeySet->get().end()
                                    ,[](sal_Int32 n) { return n == 0; })
                                     ,m_pKeySet->get().end());

                }
            }
        }   break;

        case OSQLStatementType::Update:
        case OSQLStatementType::Delete:
        case OSQLStatementType::Insert:
            break;
        default:
            m_pStatement->getOwnConnection()->throwSQLException( STR_STMT_TYPE_NOT_SUPPORTED, *this );
            break;
    }
}

void OResultSet::setBoundedColumns(const OValueRow& _rRow,
                                   const ::rtl::Reference<connectivity::OSQLColumns>& _rxColumns,
                                   const Reference<XIndexAccess>& _xNames,
                                   bool _bSetColumnMapping,
                                   const Reference<XDatabaseMetaData>& _xMetaData,
                                   std::vector<sal_Int32>& _rColMapping)
{
    ::comphelper::UStringMixEqual aCase(_xMetaData->supportsMixedCaseQuotedIdentifiers());

    Reference<XPropertySet> xTableColumn;
    OUString sTableColumnName, sSelectColumnRealName;

    const OUString sName     = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_NAME);
    const OUString sRealName = OMetaConnection::getPropMap().getNameByIndex(PROPERTY_ID_REALNAME);

    std::vector< OUString> aColumnNames;
    aColumnNames.reserve(_rxColumns->get().size());
    OValueVector::Vector::iterator aRowIter = _rRow->get().begin()+1;
    for (sal_Int32 i=0; // the first column is the bookmark column
         aRowIter != _rRow->get().end();
            ++i, ++aRowIter
        )
    {
        try
        {
            // get the table column and its name
            _xNames->getByIndex(i) >>= xTableColumn;
            OSL_ENSURE(xTableColumn.is(), "OResultSet::setBoundedColumns: invalid table column!");
            if (xTableColumn.is())
                xTableColumn->getPropertyValue(sName) >>= sTableColumnName;
            else
                sTableColumnName.clear();

            // look if we have such a select column
            // TODO: would like to have a O(log n) search here ...
            sal_Int32 nColumnPos = 0;
            for (const auto& rxColumn : _rxColumns->get())
            {
                if ( nColumnPos < static_cast<sal_Int32>(aColumnNames.size()) )
                    sSelectColumnRealName = aColumnNames[nColumnPos];
                else
                {
                    if(rxColumn->getPropertySetInfo()->hasPropertyByName(sRealName))
                        rxColumn->getPropertyValue(sRealName) >>= sSelectColumnRealName;
                    else
                        rxColumn->getPropertyValue(sName) >>= sSelectColumnRealName;
                    aColumnNames.push_back(sSelectColumnRealName);
                }

                if (aCase(sTableColumnName, sSelectColumnRealName))
                {
                    if(_bSetColumnMapping)
                    {
                        sal_Int32 nSelectColumnPos = nColumnPos + 1;
                            // the getXXX methods are 1-based ...
                        sal_Int32 nTableColumnPos = i + 1;
                            // get first table column is the bookmark column

                        SAL_INFO(
                            "connectivity.mork",
                            "Set Col Mapping: " << nSelectColumnPos << " -> "
                                << nTableColumnPos);
                        _rColMapping[nSelectColumnPos] = nTableColumnPos;
                    }

                    aRowIter->setBound(true);
                    aRowIter->setTypeKind(DataType::VARCHAR);
                }

                ++nColumnPos;
            }
        }
        catch (Exception&)
        {
            OSL_FAIL("OResultSet::setBoundedColumns: caught an Exception!");
        }
    }
}


bool OResultSet::isCount() const
{
    return (m_pParseTree &&
            m_pParseTree->count() > 2                                                       &&
            SQL_ISRULE(m_pParseTree->getChild(2),scalar_exp_commalist)                      &&
            SQL_ISRULE(m_pParseTree->getChild(2)->getChild(0),derived_column)               &&
            SQL_ISRULE(m_pParseTree->getChild(2)->getChild(0)->getChild(0),general_set_fct) &&
            m_pParseTree->getChild(2)->getChild(0)->getChild(0)->count() == 4
            );
}


// Check for valid row in m_aQuery

bool OResultSet::validRow( sal_uInt32 nRow)
{
    sal_Int32  nNumberOfRecords = m_aQueryHelper.getResultCount();

    if (( nRow == 0 ) ||
        ( nRow > static_cast<sal_uInt32>(nNumberOfRecords)) ){
        SAL_INFO("connectivity.mork", "validRow(" << nRow << "): return False");
        return false;
    }
    SAL_INFO("connectivity.mork", "validRow(" << nRow << "): return True");

    return true;
}

void OResultSet::fillKeySet(sal_Int32 nMaxCardNumber)
{
    impl_ensureKeySet();
    if (m_CurrentRowCount < nMaxCardNumber)
    {
        sal_Int32   nKeyValue;
        if ( static_cast<sal_Int32>(m_pKeySet->get().capacity()) < nMaxCardNumber )
            m_pKeySet->get().reserve(nMaxCardNumber + 20 );

        for (nKeyValue = m_CurrentRowCount+1; nKeyValue  <= nMaxCardNumber; nKeyValue ++)
            m_pKeySet->get().push_back( nKeyValue );
        m_CurrentRowCount = nMaxCardNumber;
    }
}

sal_Int32 OResultSet::deletedCount()
{
    impl_ensureKeySet();
    return m_CurrentRowCount - static_cast<sal_Int32>(m_pKeySet->get().size());

}

bool OResultSet::seekRow( eRowPosition pos, sal_Int32 nOffset )
{
    ResultSetEntryGuard aGuard( *this );
    if ( !m_pKeySet.is() )
        m_pStatement->getOwnConnection()->throwSQLException( STR_ILLEGAL_MOVEMENT, *this );

    sal_Int32  nNumberOfRecords = m_aQueryHelper.getResultCount();
    sal_Int32  nRetrievedRows = currentRowCount();
    sal_Int32  nCurPos = m_nRowPos;

    SAL_INFO("connectivity.mork", "nCurPos = " << nCurPos);
    switch( pos ) {
        case NEXT_POS:
            nCurPos++;
            break;
        case PRIOR_POS:
            if ( nCurPos > 0 )
                nCurPos--;
            break;
        case FIRST_POS:
            nCurPos = 1;
            break;
        case LAST_POS:
            nCurPos = nRetrievedRows;
            break;
        case ABSOLUTE_POS:
            nCurPos = nOffset;
            break;
        case RELATIVE_POS:
            nCurPos += sal_uInt32( nOffset );
            break;
    }

    if ( nCurPos <= 0 ) {
        m_nRowPos = 0;
        SAL_INFO(
            "connectivity.mork", "return False, m_nRowPos = " << m_nRowPos);
        return false;
    }
    sal_Int32 nCurCard;
    if ( nCurPos < static_cast<sal_Int32>(m_pKeySet->get().size()) ) //The requested row is exist in m_pKeySet, so we just use it
    {
        nCurCard = (m_pKeySet->get())[nCurPos-1];
    }
    else    //The requested row has not been retrieved until now. We should get the right card for it.
        nCurCard = nCurPos + deletedCount();

    if ( nCurCard > nNumberOfRecords) {
        fillKeySet(nNumberOfRecords);
        m_nRowPos = static_cast<sal_uInt32>(m_pKeySet->get().size() + 1);
        SAL_INFO(
            "connectivity.mork", "return False, m_nRowPos = " << m_nRowPos);
        return false;
    }
    //Insert new retrieved items for later use
    fillKeySet(nNumberOfRecords);
    m_nRowPos = static_cast<sal_uInt32>(nCurPos);
    SAL_INFO("connectivity.mork", "return True, m_nRowPos = " << m_nRowPos);
    fetchCurrentRow();
    return true;
}

void OResultSet::setColumnMapping(const std::vector<sal_Int32>& _aColumnMapping)
{
    m_aColMapping = _aColumnMapping;
#if OSL_DEBUG_LEVEL > 0
    for ( size_t i = 0; i < m_aColMapping.size(); i++ )
        SAL_INFO(
            "connectivity.mork",
            "Set Mapped: " << i << " -> " << m_aColMapping[i]);
#endif
}


css::uno::Any OResultSet::getBookmark(  )
{
    SAL_INFO("connectivity.mork", "m_nRowPos = " << m_nRowPos);
    ResultSetEntryGuard aGuard( *this );
    if ( !fetchCurrentRow() ) {
        m_pStatement->getOwnConnection()->throwSQLException( STR_ERROR_GET_ROW, *this );
    }

    OSL_ENSURE((!m_aRow->isDeleted()),"getBookmark called for deleted row");
    return makeAny(static_cast<sal_Int32>((m_aRow->get())[0]));
}
sal_Bool  OResultSet::moveToBookmark( const css::uno::Any& bookmark )
{
    ResultSetEntryGuard aGuard( *this );
    SAL_INFO(
        "connectivity.mork", "bookmark = " << comphelper::getINT32(bookmark));
    sal_Int32 nCardNum = comphelper::getINT32(bookmark);
    m_nRowPos = getRowForCardNumber(nCardNum);
    fetchCurrentRow();
    return true;
}
sal_Bool  OResultSet::moveRelativeToBookmark( const css::uno::Any& bookmark, sal_Int32 rows )
{
    ResultSetEntryGuard aGuard( *this );
    SAL_INFO(
        "connectivity.mork",
        "bookmark = " << comphelper::getINT32(bookmark) << " rows= " << rows);
    sal_Int32 nCardNum = comphelper::getINT32(bookmark);
    m_nRowPos = getRowForCardNumber(nCardNum);
    return seekRow(RELATIVE_POS,rows );
}
sal_Int32 OResultSet::compareBookmarks( const css::uno::Any& lhs, const css::uno::Any& rhs )
{
    ResultSetEntryGuard aGuard( *this );
    SAL_INFO("connectivity.mork", "m_nRowPos = " << m_nRowPos);
    sal_Int32 nFirst=0;
    sal_Int32 nSecond=0;
    sal_Int32 nResult=0;

    if ( !( lhs >>= nFirst ) || !( rhs >>= nSecond ) ) {
        m_pStatement->getOwnConnection()->throwSQLException( STR_INVALID_BOOKMARK, *this );
    }

    if(nFirst < nSecond)
        nResult = CompareBookmark::LESS;
    else if(nFirst > nSecond)
        nResult = CompareBookmark::GREATER;
    else
        nResult = CompareBookmark::EQUAL;

    return  nResult;
}
sal_Bool OResultSet::hasOrderedBookmarks(  )
{
    ResultSetEntryGuard aGuard( *this );
    SAL_INFO("connectivity.mork", "m_nRowPos = " << m_nRowPos);
    return true;
}
sal_Int32 OResultSet::hashBookmark( const css::uno::Any& bookmark )
{
    ResultSetEntryGuard aGuard( *this );
    SAL_INFO("connectivity.mork", "m_nRowPos = " << m_nRowPos);
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
void OResultSet::checkPendingUpdate()
{
    OSL_FAIL( "OResultSet::checkPendingUpdate() not implemented" );
/*
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
void OResultSet::updateValue(sal_Int32 columnIndex ,const ORowSetValue& x)
{
    SAL_INFO("connectivity.mork", "m_nRowPos = " << m_nRowPos);
    ResultSetEntryGuard aGuard( *this );
    if ( !fetchCurrentRow() ) {
        m_pStatement->getOwnConnection()->throwSQLException( STR_ERROR_GET_ROW, *this );
    }

    checkPendingUpdate();

    checkIndex(columnIndex );
    columnIndex = mapColumn(columnIndex);

    (m_aRow->get())[columnIndex].setBound(true);
    (m_aRow->get())[columnIndex] = x;
}


void SAL_CALL OResultSet::updateNull( sal_Int32 columnIndex )
{
    SAL_INFO("connectivity.mork", "m_nRowPos = " << m_nRowPos);
    ResultSetEntryGuard aGuard( *this );
    if ( !fetchCurrentRow() )
        m_pStatement->getOwnConnection()->throwSQLException( STR_ERROR_GET_ROW, *this );

    checkPendingUpdate();
    checkIndex(columnIndex );
    columnIndex = mapColumn(columnIndex);

    (m_aRow->get())[columnIndex].setBound(true);
    (m_aRow->get())[columnIndex].setNull();
}


void SAL_CALL OResultSet::updateBoolean( sal_Int32 columnIndex, sal_Bool x )
{
    updateValue(columnIndex, static_cast<bool>(x));
}

void SAL_CALL OResultSet::updateByte( sal_Int32 columnIndex, sal_Int8 x )
{
    updateValue(columnIndex,x);
}


void SAL_CALL OResultSet::updateShort( sal_Int32 columnIndex, sal_Int16 x )
{
    updateValue(columnIndex,x);
}

void SAL_CALL OResultSet::updateInt( sal_Int32 columnIndex, sal_Int32 x )
{
    updateValue(columnIndex,x);
}

void SAL_CALL OResultSet::updateLong( sal_Int32 /*columnIndex*/, sal_Int64 /*x*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XRowUpdate::updateLong", *this );
}

void SAL_CALL OResultSet::updateFloat( sal_Int32 columnIndex, float x )
{
    updateValue(columnIndex,x);
}


void SAL_CALL OResultSet::updateDouble( sal_Int32 columnIndex, double x )
{
    updateValue(columnIndex,x);
}

void SAL_CALL OResultSet::updateString( sal_Int32 columnIndex, const OUString& x )
{
    updateValue(columnIndex,x);
}

void SAL_CALL OResultSet::updateBytes( sal_Int32 columnIndex, const Sequence< sal_Int8 >& x )
{
    updateValue(columnIndex,x);
}

void SAL_CALL OResultSet::updateDate( sal_Int32 columnIndex, const css::util::Date& x )
{
    updateValue(columnIndex,x);
}


void SAL_CALL OResultSet::updateTime( sal_Int32 columnIndex, const css::util::Time& x )
{
    updateValue(columnIndex,x);
}


void SAL_CALL OResultSet::updateTimestamp( sal_Int32 columnIndex, const css::util::DateTime& x )
{
    updateValue(columnIndex,x);
}


void SAL_CALL OResultSet::updateBinaryStream( sal_Int32 columnIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length )
{
    ResultSetEntryGuard aGuard( *this );

    if(!x.is())
        ::dbtools::throwFunctionSequenceException(*this);

    Sequence<sal_Int8> aSeq;
    x->readBytes(aSeq,length);
    updateValue(columnIndex,aSeq);
}

void SAL_CALL OResultSet::updateCharacterStream( sal_Int32 columnIndex, const Reference< css::io::XInputStream >& x, sal_Int32 length )
{
    updateBinaryStream(columnIndex,x,length);
}

void SAL_CALL OResultSet::updateObject( sal_Int32 columnIndex, const Any& x )
{
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
    {
        const OUString sError( m_pStatement->getOwnConnection()->getResources().getResourceStringWithSubstitution(
                STR_COLUMN_NOT_UPDATEABLE,
                "$position$", OUString::number(columnIndex)
             ) );
        ::dbtools::throwGenericSQLException(sError,*this);
    } // if (!::dbtools::implUpdateObject(this, columnIndex, x))
 }


void SAL_CALL OResultSet::updateNumericObject( sal_Int32 columnIndex, const Any& x, sal_Int32 /*scale*/ )
{
    if (!::dbtools::implUpdateObject(this, columnIndex, x))
    {
        const OUString sError( m_pStatement->getOwnConnection()->getResources().getResourceStringWithSubstitution(
                STR_COLUMN_NOT_UPDATEABLE,
                "$position$", OUString::number(columnIndex)
             ) );
        ::dbtools::throwGenericSQLException(sError,*this);
    }
}

// XResultSetUpdate

void SAL_CALL OResultSet::insertRow(  )
{
    ResultSetEntryGuard aGuard( *this );
    SAL_INFO("connectivity.mork", "in, m_nRowPos = " << m_nRowPos);
//    m_RowStates = RowStates_Inserted;
    updateRow();
    //m_aQueryHelper.setRowStates(getCurrentCardNumber(),m_RowStates);
    SAL_INFO("connectivity.mork", "out, m_nRowPos = " << m_nRowPos);
}

void SAL_CALL OResultSet::updateRow(  )
{
    OSL_FAIL( "OResultSet::updateRow(  ) not implemented" );
}

void SAL_CALL OResultSet::deleteRow(  )
{
    OSL_FAIL( "OResultSet::deleteRow(  ) not implemented" );
}

void SAL_CALL OResultSet::cancelRowUpdates(  )
{
    OSL_FAIL( "OResultSet::cancelRowUpdates(  ) not implemented" );
}

void SAL_CALL OResultSet::moveToInsertRow(  )
{
    OSL_FAIL( "OResultSet::moveToInsertRow(  ) not implemented" );
}

void SAL_CALL OResultSet::moveToCurrentRow(  )
{
    ResultSetEntryGuard aGuard( *this );
    SAL_INFO("connectivity.mork", "m_nRowPos = " << m_nRowPos);
    if (rowInserted())
    {
        m_nRowPos = 0;
        fetchCurrentRow();
    }
}

bool OResultSet::determineReadOnly()
{
//    OSL_FAIL( "OResultSet::determineReadOnly(  ) not implemented" );

    if (m_bIsReadOnly == TRISTATE_INDET)
    {
        m_bIsReadOnly = TRISTATE_TRUE;
//        OConnection* xConnection = static_cast<OConnection*>(m_pStatement->getConnection().get());
//        m_bIsReadOnly = !m_aQueryHelper.isWritable(xConnection) || m_bIsAlwaysFalseQuery;
    }

    return m_bIsReadOnly != TRISTATE_FALSE;
}

void OResultSet::setTable(OTable* _rTable)
{
    m_xTable = _rTable;
    m_xTableColumns = m_xTable->getColumns();
    if(m_xTableColumns.is())
        m_aColumnNames = m_xTableColumns->getElementNames();
}

void OResultSet::setOrderByColumns(const std::vector<sal_Int32>& _aColumnOrderBy)
{
    m_aOrderbyColumnNumber = _aColumnOrderBy;
}

void OResultSet::setOrderByAscending(const std::vector<TAscendingOrder>& _aOrderbyAsc)
{
    m_aOrderbyAscending = _aOrderbyAsc;
}
Sequence< sal_Int32 > SAL_CALL OResultSet::deleteRows( const Sequence< Any >& /*rows*/ )
{
    ::dbtools::throwFeatureNotImplementedSQLException( "XDeleteRows::deleteRows", *this );
    return Sequence< sal_Int32 >();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
