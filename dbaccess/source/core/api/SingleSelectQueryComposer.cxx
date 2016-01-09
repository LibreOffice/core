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

#include <string.h>
#include "composertools.hxx"
#include "core_resource.hrc"
#include "core_resource.hxx"
#include "dbastrings.hrc"
#include "HelperCollections.hxx"
#include "SingleSelectQueryComposer.hxx"
#include "sqlbison.hxx"
#include "sdbcoretools.hxx"

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/i18n/LocaleData.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/script/Converter.hpp>
#include <com/sun/star/sdb/BooleanComparisonMode.hpp>
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#include <com/sun/star/sdb/XQueriesSupplier.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/uno/XAggregation.hpp>
#include <com/sun/star/util/NumberFormatter.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <connectivity/predicateinput.hxx>
#include <unotools/syslocale.hxx>
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <osl/diagnose.h>
#include <unotools/configmgr.hxx>
#include <unotools/sharedunocomponent.hxx>

#include <memory>

using namespace ::dbaccess;
using namespace ::dbtools;
using namespace ::comphelper;
using namespace ::connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::script;
using namespace ::com::sun::star::util;
using namespace ::cppu;
using namespace ::osl;
using namespace ::utl;

namespace dbaccess {
namespace BooleanComparisonMode = ::com::sun::star::sdb::BooleanComparisonMode;
}

#define STR_SELECT      "SELECT "
#define STR_FROM        " FROM "
#define STR_WHERE       " WHERE "
#define STR_GROUP_BY    " GROUP BY "
#define STR_HAVING      " HAVING "
#define STR_ORDER_BY    " ORDER BY "
#define STR_AND         " AND "
#define STR_OR          " OR "
#define STR_LIKE        OUString(" LIKE ")
#define L_BRACKET       "("
#define R_BRACKET       ")"
#define COMMA           ","

namespace
{
    /** parses the given statement, using the given parser, returns a parse node representing
        the statement

        If the statement cannot be parsed, an error is thrown.
    */
    const OSQLParseNode* parseStatement_throwError( OSQLParser& _rParser, const OUString& _rStatement, const Reference< XInterface >& _rxContext )
    {
        OUString aErrorMsg;
        const OSQLParseNode* pNewSqlParseNode = _rParser.parseTree( aErrorMsg, _rStatement );
        if ( !pNewSqlParseNode )
        {
            OUString sSQLStateGeneralError( getStandardSQLState( SQL_GENERAL_ERROR ) );
            SQLException aError2( aErrorMsg, _rxContext, sSQLStateGeneralError, 1000, Any() );
            SQLException aError1( _rStatement, _rxContext, sSQLStateGeneralError, 1000, makeAny( aError2 ) );
            throw SQLException(_rParser.getContext().getErrorMessage(OParseContext::ERROR_GENERAL),_rxContext,sSQLStateGeneralError,1000,makeAny(aError1));
        }
        return pNewSqlParseNode;
    }

    /** checks whether the given parse node describes a valid single select statement, throws
        an error if not
    */
    void checkForSingleSelect_throwError( const OSQLParseNode* pStatementNode, OSQLParseTreeIterator& _rIterator,
        const Reference< XInterface >& _rxContext, const OUString& _rOriginatingCommand )
    {
        const OSQLParseNode* pOldNode = _rIterator.getParseTree();

        // determine the statement type
        _rIterator.setParseTree( pStatementNode );
        _rIterator.traverseAll();
        bool bIsSingleSelect = ( _rIterator.getStatementType() == SQL_STATEMENT_SELECT );

        // throw the error, if necessary
        if ( !bIsSingleSelect || SQL_ISRULE( pStatementNode, union_statement ) ) // #i4229# OJ
        {
            // restore the old node before throwing the exception
            _rIterator.setParseTree( pOldNode );
            // and now really ...
            SQLException aError1( _rOriginatingCommand, _rxContext, getStandardSQLState( SQL_GENERAL_ERROR ), 1000, Any() );
            throw SQLException( DBACORE_RESSTRING( RID_STR_ONLY_QUERY ), _rxContext,
                getStandardSQLState( SQL_GENERAL_ERROR ), 1000, makeAny( aError1 ) );
        }

        delete pOldNode;
    }

    /** combines parseStatement_throwError and checkForSingleSelect_throwError
    */
    void parseAndCheck_throwError( OSQLParser& _rParser, const OUString& _rStatement,
        OSQLParseTreeIterator& _rIterator, const Reference< XInterface >& _rxContext )
    {
        const OSQLParseNode* pNode = parseStatement_throwError( _rParser, _rStatement, _rxContext );
        checkForSingleSelect_throwError( pNode, _rIterator, _rxContext, _rStatement );
    }

    /** transforms a parse node describing a complete statement into a pure select
        statement, without any filter/order/groupby/having clauses
    */
    OUString getPureSelectStatement( const OSQLParseNode* _pRootNode, Reference< XConnection > _rxConnection )
    {
        OUString sSQL = STR_SELECT;
        _pRootNode->getChild(1)->parseNodeToStr( sSQL, _rxConnection );
        _pRootNode->getChild(2)->parseNodeToStr( sSQL, _rxConnection );
        sSQL += STR_FROM;
        _pRootNode->getChild(3)->getChild(0)->getChild(1)->parseNodeToStr( sSQL, _rxConnection );
        return sSQL;
    }

    /** resets an SQL iterator, including deletion of the parse tree, and disposal if desired
    */
    void resetIterator( OSQLParseTreeIterator& _rIterator, bool _bDispose )
    {
        const OSQLParseNode* pSqlParseNode = _rIterator.getParseTree();
        _rIterator.setParseTree(nullptr);
        delete pSqlParseNode;
        if ( _bDispose )
            _rIterator.dispose();
    }
    void lcl_addFilterCriteria_throw(sal_Int32 i_nFilterOperator,const OUString& i_sValue,OUStringBuffer& o_sRet)
    {
        switch( i_nFilterOperator )
        {
            case SQLFilterOperator::EQUAL:
                o_sRet.append(" = " + i_sValue);
                break;
            case SQLFilterOperator::NOT_EQUAL:
                o_sRet.append(" <> " + i_sValue);
                break;
            case SQLFilterOperator::LESS:
                o_sRet.append(" < " + i_sValue);
                break;
            case SQLFilterOperator::GREATER:
                o_sRet.append(" > " + i_sValue);
                break;
            case SQLFilterOperator::LESS_EQUAL:
                o_sRet.append(" <= " + i_sValue);
                break;
            case SQLFilterOperator::GREATER_EQUAL:
                o_sRet.append(" >= " + i_sValue);
                break;
            case SQLFilterOperator::LIKE:
                o_sRet.append(" LIKE " + i_sValue);
                break;
            case SQLFilterOperator::NOT_LIKE:
                o_sRet.append(" NOT LIKE " + i_sValue);
                break;
            case SQLFilterOperator::SQLNULL:
                o_sRet.append(" IS NULL");
                break;
            case SQLFilterOperator::NOT_SQLNULL:
                o_sRet.append(" IS NOT NULL");
                break;
            default:
                throw SQLException();
        }
    }

}


OSingleSelectQueryComposer::OSingleSelectQueryComposer(const Reference< XNameAccess>& _rxTables,
                               const Reference< XConnection>& _xConnection,
                               const Reference<XComponentContext>& _rContext )
    :OSubComponent(m_aMutex,_xConnection)
    ,OPropertyContainer(m_aBHelper)
    ,m_aSqlParser( _rContext, &m_aParseContext )
    ,m_aSqlIterator( _xConnection, _rxTables, m_aSqlParser, nullptr )
    ,m_aAdditiveIterator( _xConnection, _rxTables, m_aSqlParser, nullptr )
    ,m_aElementaryParts( (size_t)SQLPartCount )
    ,m_xConnection(_xConnection)
    ,m_xMetaData(_xConnection->getMetaData())
    ,m_xConnectionTables( _rxTables )
    ,m_aContext( _rContext )
    ,m_pTables(nullptr)
    ,m_nBoolCompareMode( BooleanComparisonMode::EQUAL_INTEGER )
    ,m_nCommandType(CommandType::COMMAND)
{
    if ( !m_aContext.is() || !m_xConnection.is() || !m_xConnectionTables.is() )
        throw IllegalArgumentException();

    registerProperty(PROPERTY_ORIGINAL,PROPERTY_ID_ORIGINAL,PropertyAttribute::BOUND|PropertyAttribute::READONLY,&m_sOrignal,cppu::UnoType<decltype(m_sOrignal)>::get());

    m_aCurrentColumns.resize(4);

    m_aLocale = m_aParseContext.getPreferredLocale();
    m_xNumberFormatsSupplier = dbtools::getNumberFormats( m_xConnection, true, m_aContext );
    Reference< XLocaleData4 > xLocaleData( LocaleData::create(m_aContext) );
    LocaleDataItem aData = xLocaleData->getLocaleItem(m_aLocale);
    m_sDecimalSep = aData.decimalSeparator;
    OSL_ENSURE(m_sDecimalSep.getLength() == 1,"OSingleSelectQueryComposer::OSingleSelectQueryComposer decimal separator is not 1 length");
    try
    {
        Any aValue;
        Reference<XInterface> xDs = dbaccess::getDataSource(_xConnection);
        if ( dbtools::getDataSourceSetting(xDs,static_cast <OUString> (PROPERTY_BOOLEANCOMPARISONMODE),aValue) )
        {
            OSL_VERIFY( aValue >>= m_nBoolCompareMode );
        }
        Reference< XQueriesSupplier >  xQueriesAccess(m_xConnection, UNO_QUERY);
        if (xQueriesAccess.is())
            m_xConnectionQueries = xQueriesAccess->getQueries();
    }
    catch(Exception&)
    {
    }
}

OSingleSelectQueryComposer::~OSingleSelectQueryComposer()
{
    ::std::vector<OPrivateColumns*>::iterator aColIter = m_aColumnsCollection.begin();
    ::std::vector<OPrivateColumns*>::iterator aEnd = m_aColumnsCollection.end();
    for(;aColIter != aEnd;++aColIter)
        delete *aColIter;

    ::std::vector<OPrivateTables*>::iterator aTabIter = m_aTablesCollection.begin();
    ::std::vector<OPrivateTables*>::iterator aTabEnd = m_aTablesCollection.end();
    for(;aTabIter != aTabEnd;++aTabIter)
        delete *aTabIter;
}

// OComponentHelper
void SAL_CALL OSingleSelectQueryComposer::disposing()
{
    OSubComponent::disposing();

    MutexGuard aGuard(m_aMutex);

    resetIterator( m_aSqlIterator, true );
    resetIterator( m_aAdditiveIterator, true );

    m_xConnectionTables = nullptr;
    m_xConnection       = nullptr;

    clearCurrentCollections();
}

IMPLEMENT_FORWARD_XINTERFACE3(OSingleSelectQueryComposer,OSubComponent,OSingleSelectQueryComposer_BASE,OPropertyContainer)
IMPLEMENT_SERVICE_INFO1(OSingleSelectQueryComposer,"org.openoffice.comp.dba.OSingleSelectQueryComposer",SERVICE_NAME_SINGLESELECTQUERYCOMPOSER)

css::uno::Sequence<sal_Int8> OSingleSelectQueryComposer::getImplementationId()
    throw (css::uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

IMPLEMENT_GETTYPES3(OSingleSelectQueryComposer,OSubComponent,OSingleSelectQueryComposer_BASE,OPropertyContainer)
IMPLEMENT_PROPERTYCONTAINER_DEFAULTS(OSingleSelectQueryComposer)

// XSingleSelectQueryAnalyzer
OUString SAL_CALL OSingleSelectQueryComposer::getQuery(  ) throw(RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    TGetParseNode F_tmp(&OSQLParseTreeIterator::getParseTree);
    return getStatementPart(F_tmp,m_aSqlIterator);
}

void SAL_CALL OSingleSelectQueryComposer::setQuery( const OUString& command ) throw(SQLException, RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    m_nCommandType = CommandType::COMMAND;
    // first clear the tables and columns
    clearCurrentCollections();
    // now set the new one
    setQuery_Impl(command);
    m_sOrignal = command;

    // reset the additive iterator to the same statement
    parseAndCheck_throwError( m_aSqlParser, m_sOrignal, m_aAdditiveIterator, *this );

    // we have no "elementary" parts anymore (means filter/groupby/having/order clauses)
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        m_aElementaryParts[ eLoopParts ].clear();
}

void SAL_CALL OSingleSelectQueryComposer::setCommand( const OUString& Command,sal_Int32 _nCommandType ) throw(SQLException, RuntimeException, std::exception)
{
    OUStringBuffer sSQL;
    switch(_nCommandType)
    {
        case CommandType::COMMAND:
            setElementaryQuery(Command);
            return;
        case CommandType::TABLE:
            if ( m_xConnectionTables->hasByName(Command) )
            {
                sSQL.append("SELECT * FROM ");
                Reference< XPropertySet > xTable;
                try
                {
                    m_xConnectionTables->getByName( Command ) >>= xTable;
                }
                catch(const WrappedTargetException& e)
                {
                    SQLException e2;
                    if ( e.TargetException >>= e2 )
                        throw e2;
                }
                catch(Exception&)
                {
                    DBG_UNHANDLED_EXCEPTION();
                }

                sSQL.append(dbtools::composeTableNameForSelect(m_xConnection,xTable));
            }
            else
            {
                OUString sMessage( DBACORE_RESSTRING( RID_STR_TABLE_DOES_NOT_EXIST ) );
                throwGenericSQLException(sMessage.replaceAll( "$table$", Command ),*this);
            }
            break;
        case CommandType::QUERY:
            if ( m_xConnectionQueries->hasByName(Command) )
            {

                Reference<XPropertySet> xQuery(m_xConnectionQueries->getByName(Command),UNO_QUERY);
                OUString sCommand;
                xQuery->getPropertyValue(PROPERTY_COMMAND) >>= sCommand;
                sSQL.append(sCommand);
            }
            else
            {
                OUString sMessage( DBACORE_RESSTRING( RID_STR_QUERY_DOES_NOT_EXIST ) );
                throwGenericSQLException(sMessage.replaceAll( "$table$", Command ),*this);
            }

            break;
        default:
            break;
    }
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    m_nCommandType = _nCommandType;
    m_sCommand = Command;
    // first clear the tables and columns
    clearCurrentCollections();
    // now set the new one
    OUString sCommand = sSQL.makeStringAndClear();
    setElementaryQuery(sCommand);
    m_sOrignal = sCommand;
}

void OSingleSelectQueryComposer::setQuery_Impl( const OUString& command )
{
    // parse this
    parseAndCheck_throwError( m_aSqlParser, command, m_aSqlIterator, *this );

    // strip it from all clauses, to have the pure SELECT statement
    m_aPureSelectSQL = getPureSelectStatement( m_aSqlIterator.getParseTree(), m_xConnection );

    // update tables
    getTables();
}

Sequence< Sequence< PropertyValue > > SAL_CALL OSingleSelectQueryComposer::getStructuredHavingClause(  ) throw (RuntimeException, std::exception)
{
    TGetParseNode F_tmp(&OSQLParseTreeIterator::getSimpleHavingTree);
    return getStructuredCondition(F_tmp);
}

Sequence< Sequence< PropertyValue > > SAL_CALL OSingleSelectQueryComposer::getStructuredFilter(  ) throw(RuntimeException, std::exception)
{
    TGetParseNode F_tmp(&OSQLParseTreeIterator::getSimpleWhereTree);
    return getStructuredCondition(F_tmp);
}

void SAL_CALL OSingleSelectQueryComposer::appendHavingClauseByColumn( const Reference< XPropertySet >& column, sal_Bool andCriteria,sal_Int32 filterOperator ) throw (SQLException, WrappedTargetException, RuntimeException, std::exception)
{
    ::std::mem_fun1_t<bool,OSingleSelectQueryComposer,const OUString&> F_tmp(&OSingleSelectQueryComposer::implSetHavingClause);
    setConditionByColumn(column,andCriteria,F_tmp,filterOperator);
}

void SAL_CALL OSingleSelectQueryComposer::appendFilterByColumn( const Reference< XPropertySet >& column, sal_Bool andCriteria,sal_Int32 filterOperator ) throw(SQLException, WrappedTargetException, RuntimeException, std::exception)
{
    ::std::mem_fun1_t<bool,OSingleSelectQueryComposer,const OUString&> F_tmp(&OSingleSelectQueryComposer::implSetFilter);
    setConditionByColumn(column,andCriteria,F_tmp,filterOperator);
}

OUString OSingleSelectQueryComposer::impl_getColumnRealName_throw(const Reference< XPropertySet >& column, bool bGroupBy)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    getColumns();
    if ( !column.is()
        || !m_aCurrentColumns[SelectColumns]
        || !column->getPropertySetInfo()->hasPropertyByName(PROPERTY_NAME)
        )
        {
            OUString sError(DBACORE_RESSTRING(RID_STR_COLUMN_UNKNOWN_PROP));
            SQLException aErr(sError.replaceAll("%value", PROPERTY_NAME),*this,SQLSTATE_GENERAL,1000,Any() );
            throw SQLException(DBACORE_RESSTRING(RID_STR_COLUMN_NOT_VALID),*this,SQLSTATE_GENERAL,1000,makeAny(aErr) );
        }

    OUString aName, aNewName;
    column->getPropertyValue(PROPERTY_NAME)         >>= aName;

    if ( bGroupBy &&
         !m_xMetaData->supportsGroupByUnrelated() &&
         m_aCurrentColumns[SelectColumns] &&
         !m_aCurrentColumns[SelectColumns]->hasByName(aName) )
    {
        OUString sError(DBACORE_RESSTRING(RID_STR_COLUMN_MUST_VISIBLE));
        throw SQLException(sError.replaceAll("%name", aName),*this,SQLSTATE_GENERAL,1000,Any() );
    }

    OUString aQuote  = m_xMetaData->getIdentifierQuoteString();
    if ( m_aCurrentColumns[SelectColumns]->hasByName(aName) )
    {
        Reference<XPropertySet> xColumn;
        m_aCurrentColumns[SelectColumns]->getByName(aName) >>= xColumn;
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME),"Property REALNAME not available!");
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_TABLENAME),"Property TABLENAME not available!");
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName("Function"),"Property FUNCTION not available!");

        OUString sRealName, sTableName;
        xColumn->getPropertyValue(PROPERTY_REALNAME)    >>= sRealName;
        xColumn->getPropertyValue(PROPERTY_TABLENAME)   >>= sTableName;
        bool bFunction = false;
        xColumn->getPropertyValue("Function") >>= bFunction;
        if ( sRealName == aName )
        {
            if ( bFunction )
                aNewName = aName;
            else
            {
                if(sTableName.indexOf('.') != -1)
                {
                    OUString aCatlog,aSchema,aTable;
                    ::dbtools::qualifiedNameComponents(m_xMetaData,sTableName,aCatlog,aSchema,aTable,::dbtools::eInDataManipulation);
                    sTableName = ::dbtools::composeTableName( m_xMetaData, aCatlog, aSchema, aTable, true, ::dbtools::eInDataManipulation );
                }
                else if (!sTableName.isEmpty())
                    sTableName = ::dbtools::quoteName(aQuote,sTableName);

                if(sTableName.isEmpty())
                    aNewName =  ::dbtools::quoteName(aQuote,sRealName);
                else
                    aNewName =  sTableName + "." + ::dbtools::quoteName(aQuote,sRealName);
            }
        }
        else
            aNewName = ::dbtools::quoteName(aQuote,aName);
    }
    else
        aNewName = getTableAlias(column) + ::dbtools::quoteName(aQuote,aName);
    return aNewName;
}

OUString OSingleSelectQueryComposer::impl_getColumnName_throw(const Reference< XPropertySet >& column, bool bOrderBy)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    getColumns();
    if ( !column.is()
        || !m_aCurrentColumns[SelectColumns]
        || !column->getPropertySetInfo()->hasPropertyByName(PROPERTY_NAME)
        )
        {
            OUString sError(DBACORE_RESSTRING(RID_STR_COLUMN_UNKNOWN_PROP));
            SQLException aErr(sError.replaceAll("%value", PROPERTY_NAME),*this,SQLSTATE_GENERAL,1000,Any() );
            throw SQLException(DBACORE_RESSTRING(RID_STR_COLUMN_NOT_VALID),*this,SQLSTATE_GENERAL,1000,makeAny(aErr) );
        }

    OUString aName;
    column->getPropertyValue(PROPERTY_NAME)         >>= aName;

    const OUString aQuote  = m_xMetaData->getIdentifierQuoteString();

    if ( m_aCurrentColumns[SelectColumns] &&
         m_aCurrentColumns[SelectColumns]->hasByName(aName) )
    {
        // It is a column from the SELECT list, use it as such.
        return ::dbtools::quoteName(aQuote,aName);
    }

    // Nope, it is an unrelated column.
    // Is that supported?
    if ( bOrderBy &&
         !m_xMetaData->supportsOrderByUnrelated() )
    {
        OUString sError(DBACORE_RESSTRING(RID_STR_COLUMN_MUST_VISIBLE));
        throw SQLException(sError.replaceAll("%name", aName),*this,SQLSTATE_GENERAL,1000,Any() );
    }

    // We need to refer to it by its "real" name, that is by schemaName.tableName.columnNameInTable
    return impl_getColumnRealName_throw(column, false);
}

void SAL_CALL OSingleSelectQueryComposer::appendOrderByColumn( const Reference< XPropertySet >& column, sal_Bool ascending ) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OUString sColumnName( impl_getColumnName_throw(column, true) );
    OUString sOrder = getOrder();
    if ( !(sOrder.isEmpty() || sColumnName.isEmpty()) )
        sOrder += COMMA;
    sOrder += sColumnName;
    if ( !(ascending || sColumnName.isEmpty()) )
        sOrder += " DESC ";

    setOrder(sOrder);
}

void SAL_CALL OSingleSelectQueryComposer::appendGroupByColumn( const Reference< XPropertySet >& column) throw(SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    OUString sColumnName( impl_getColumnRealName_throw(column, true) );
    OrderCreator aComposer;
    aComposer.append( getGroup() );
    aComposer.append( sColumnName );
    setGroup( aComposer.getComposedAndClear() );
}

OUString OSingleSelectQueryComposer::composeStatementFromParts( const ::std::vector< OUString >& _rParts )
{
    OSL_ENSURE( _rParts.size() == (size_t)SQLPartCount, "OSingleSelectQueryComposer::composeStatementFromParts: invalid parts array!" );

    OUStringBuffer aSql( m_aPureSelectSQL );
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        if ( !_rParts[ eLoopParts ].isEmpty() )
        {
            aSql.append( getKeyword( eLoopParts ) );
            aSql.append( _rParts[ eLoopParts ] );
        }

    return aSql.makeStringAndClear();
}

OUString SAL_CALL OSingleSelectQueryComposer::getElementaryQuery() throw (css::uno::RuntimeException, std::exception)
{
    return composeStatementFromParts( m_aElementaryParts );
}

void SAL_CALL OSingleSelectQueryComposer::setElementaryQuery( const OUString& _rElementary ) throw (css::sdbc::SQLException, css::uno::RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    // remember the 4 current "additive" clauses
    ::std::vector< OUString > aAdditiveClauses( SQLPartCount );
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        aAdditiveClauses[ eLoopParts ] = getSQLPart( eLoopParts, m_aAdditiveIterator, false );

    // clear the tables and columns
    clearCurrentCollections();
    // set and parse the new query
    setQuery_Impl( _rElementary );

    // get the 4 elementary parts of the statement
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        m_aElementaryParts[ eLoopParts ] = getSQLPart( eLoopParts, m_aSqlIterator, false );

    // reset the AdditiveIterator: m_aPureSelectSQL may have changed
    try
    {
        parseAndCheck_throwError( m_aSqlParser, composeStatementFromParts( aAdditiveClauses ), m_aAdditiveIterator, *this );
    }
    catch( const Exception& e )
    {
        (void)e;
        SAL_WARN("dbaccess", "OSingleSelectQueryComposer::setElementaryQuery: there should be no error anymore for the additive statement!" );
        DBG_UNHANDLED_EXCEPTION();
        // every part of the additive statement should have passed other tests already, and should not
        // be able to cause any errors ... me thinks
    }
}

namespace
{
    OUString getComposedClause( const OUString& _rElementaryClause, const OUString& _rAdditionalClause,
        TokenComposer& _rComposer, const OUString& _rKeyword )
    {
        _rComposer.clear();
        _rComposer.append( _rElementaryClause );
        _rComposer.append( _rAdditionalClause );
        OUString sComposed = _rComposer.getComposedAndClear();
        if ( !sComposed.isEmpty() )
            sComposed = _rKeyword + sComposed;
        return sComposed;
    }
}

void OSingleSelectQueryComposer::setSingleAdditiveClause( SQLPart _ePart, const OUString& _rClause )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    // if nothing is changed, do nothing
    if ( getSQLPart( _ePart, m_aAdditiveIterator, false ) == _rClause )
        return;

    // collect the 4 single parts as they're currently set
    ::std::vector< OUString > aClauses;
    aClauses.reserve( (size_t)SQLPartCount );
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        aClauses.push_back( getSQLPart( eLoopParts, m_aSqlIterator, true ) );

    // overwrite the one part in question here
    std::unique_ptr< TokenComposer > pComposer;
    if ( ( _ePart == Where ) || ( _ePart == Having ) )
        pComposer.reset( new FilterCreator );
    else
        pComposer.reset( new OrderCreator );
    aClauses[ _ePart ] = getComposedClause( m_aElementaryParts[ _ePart ], _rClause,
        *pComposer, getKeyword( _ePart ) );

    // construct the complete statement
    OUStringBuffer aSql(m_aPureSelectSQL);
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        aSql.append(aClauses[ eLoopParts ]);

    // set the query
    setQuery_Impl(aSql.makeStringAndClear());

    // clear column collections which (might) have changed
    clearColumns( ParameterColumns );
    if ( _ePart == Order )
        clearColumns( OrderColumns );
    else if ( _ePart == Group )
        clearColumns( GroupByColumns );

    // also, since the "additive filter" change, we need to rebuild our "additive" statement
    aSql = m_aPureSelectSQL;
    // again, first get all the old additive parts
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        aClauses[ eLoopParts ] = getSQLPart( eLoopParts, m_aAdditiveIterator, true );
    // then overwrite the one in question
    aClauses[ _ePart ] = getComposedClause( OUString(), _rClause, *pComposer, getKeyword( _ePart ) );
    // and parse it, so that m_aAdditiveIterator is up to date
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        aSql.append(aClauses[ eLoopParts ]);
    try
    {
        parseAndCheck_throwError( m_aSqlParser, aSql.makeStringAndClear(), m_aAdditiveIterator, *this );
    }
    catch( const Exception& e )
    {
        (void)e;
        SAL_WARN("dbaccess", "OSingleSelectQueryComposer::setSingleAdditiveClause: there should be no error anymore for the additive statement!" );
        // every part of the additive statement should have passed other tests already, and should not
        // be able to cause any errors ... me thinks
    }
}

void SAL_CALL OSingleSelectQueryComposer::setFilter( const OUString& filter ) throw(SQLException, RuntimeException, std::exception)
{
    setSingleAdditiveClause( Where, filter );
}

void SAL_CALL OSingleSelectQueryComposer::setOrder( const OUString& order ) throw(SQLException, RuntimeException, std::exception)
{
    setSingleAdditiveClause( Order, order );
}

void SAL_CALL OSingleSelectQueryComposer::setGroup( const OUString& group ) throw (SQLException, RuntimeException, std::exception)
{
    setSingleAdditiveClause( Group, group );
}

void SAL_CALL OSingleSelectQueryComposer::setHavingClause( const OUString& filter ) throw(SQLException, RuntimeException, std::exception)
{
    setSingleAdditiveClause( Having, filter );
}

// XTablesSupplier
Reference< XNameAccess > SAL_CALL OSingleSelectQueryComposer::getTables(  ) throw(RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_pTables )
    {
        const OSQLTables& aTables = m_aSqlIterator.getTables();
        ::std::vector< OUString> aNames;
        OSQLTables::const_iterator aEnd = aTables.end();
        for(OSQLTables::const_iterator aIter = aTables.begin(); aIter != aEnd;++aIter)
            aNames.push_back(aIter->first);

        m_pTables = new OPrivateTables(aTables,m_xMetaData->supportsMixedCaseQuotedIdentifiers(),*this,m_aMutex,aNames);
    }

    return m_pTables;
}

// XColumnsSupplier
Reference< XNameAccess > SAL_CALL OSingleSelectQueryComposer::getColumns(  ) throw(RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !!m_aCurrentColumns[SelectColumns] )
        return m_aCurrentColumns[SelectColumns];

    ::std::vector< OUString> aNames;
    ::rtl::Reference< OSQLColumns> aSelectColumns;
    bool bCase = true;
    Reference< XNameAccess> xQueryColumns;
    if ( m_nCommandType == CommandType::QUERY )
    {
        Reference<XColumnsSupplier> xSup(m_xConnectionQueries->getByName(m_sCommand),UNO_QUERY);
        if(xSup.is())
            xQueryColumns = xSup->getColumns();
    }

    do {

    try
    {
        SharedUNOComponent< XStatement, DisposableComponent > xStatement;
        SharedUNOComponent< XPreparedStatement, DisposableComponent > xPreparedStatement;

        bCase = m_xMetaData->supportsMixedCaseQuotedIdentifiers();
        aSelectColumns = m_aSqlIterator.getSelectColumns();

        OUStringBuffer aSQL( m_aPureSelectSQL + STR_WHERE " ( 0 = 1 )");

        // preserve the original WHERE clause
        // #i102234#
        OUString sOriginalWhereClause = getSQLPart( Where, m_aSqlIterator, false );
        if ( !sOriginalWhereClause.isEmpty() )
        {
            aSQL.append( " AND ( " + sOriginalWhereClause + " ) " );
        }

        OUString sGroupBy = getSQLPart( Group, m_aSqlIterator, true );
        if ( !sGroupBy.isEmpty() )
            aSQL.append( sGroupBy );

        OUString sSQL( aSQL.makeStringAndClear() );
        // normalize the statement so that it doesn't contain any application-level features anymore
        OUString sError;
        const std::unique_ptr< OSQLParseNode > pStatementTree( m_aSqlParser.parseTree( sError, sSQL ) );
        OSL_ENSURE( pStatementTree.get(), "OSingleSelectQueryComposer::getColumns: could not parse the column retrieval statement!" );
        if ( pStatementTree.get() )
            if ( !pStatementTree->parseNodeToExecutableStatement( sSQL, m_xConnection, m_aSqlParser, nullptr ) )
                break;

        Reference< XResultSetMetaData > xResultSetMeta;
        Reference< XResultSetMetaDataSupplier > xResMetaDataSup;
        try
        {
            xPreparedStatement.set( m_xConnection->prepareStatement( sSQL ), UNO_QUERY_THROW );
            xResMetaDataSup.set( xPreparedStatement, UNO_QUERY_THROW );
            xResultSetMeta.set( xResMetaDataSup->getMetaData(), UNO_QUERY_THROW );
        }
        catch( const Exception& ) { }

        try
        {
            if ( !xResultSetMeta.is() )
            {
                xStatement.reset( Reference< XStatement >( m_xConnection->createStatement(), UNO_QUERY_THROW ) );
                Reference< XPropertySet > xStatementProps( xStatement, UNO_QUERY_THROW );
                try { xStatementProps->setPropertyValue( PROPERTY_ESCAPE_PROCESSING, makeAny( sal_False ) ); }
                catch ( const Exception& ) { DBG_UNHANDLED_EXCEPTION(); }
                xResMetaDataSup.set( xStatement->executeQuery( sSQL ), UNO_QUERY_THROW );
                xResultSetMeta.set( xResMetaDataSup->getMetaData(), UNO_QUERY_THROW );
            }
        }
        catch( const Exception& )
        {
            //@see issue http://qa.openoffice.org/issues/show_bug.cgi?id=110111
            // access returns a different order of column names when executing select * from
            // and asking the columns from the metadata.
            Reference< XParameters > xParameters( xPreparedStatement, UNO_QUERY_THROW );
            Reference< XIndexAccess > xPara = getParameters();
            for(sal_Int32 i = 1;i <= xPara->getCount();++i)
                xParameters->setNull(i,DataType::VARCHAR);
            xResMetaDataSup.set(xPreparedStatement->executeQuery(), UNO_QUERY_THROW );
            xResultSetMeta.set( xResMetaDataSup->getMetaData(), UNO_QUERY_THROW );
        }

        if ( aSelectColumns->get().empty() )
        {
            // This is a valid case. If we can syntactically parse the query, but not semantically
            // (e.g. because it is based on a table we do not know), then there will be no SelectColumns
            aSelectColumns = ::connectivity::parse::OParseColumn::createColumnsForResultSet( xResultSetMeta, m_xMetaData ,xQueryColumns);
            break;
        }

        const ::comphelper::UStringMixEqual aCaseCompare( bCase );
        typedef ::std::set< size_t > SizeTSet;
        SizeTSet aUsedSelectColumns;
        ::connectivity::parse::OParseColumn::StringMap aColumnNames;

        sal_Int32 nCount = xResultSetMeta->getColumnCount();
        OSL_ENSURE( (size_t) nCount == aSelectColumns->get().size(), "OSingleSelectQueryComposer::getColumns: inconsistent column counts, this might result in wrong columns!" );
        for(sal_Int32 i=1;i<=nCount;++i)
        {
            OUString sColumnName = xResultSetMeta->getColumnName(i);
            OUString sColumnLabel;
            if ( xQueryColumns.is() && xQueryColumns->hasByName(sColumnName) )
            {
                Reference<XPropertySet> xQueryColumn(xQueryColumns->getByName(sColumnName),UNO_QUERY_THROW);
                xQueryColumn->getPropertyValue(PROPERTY_LABEL) >>= sColumnLabel;
            }
            else
                sColumnLabel = xResultSetMeta->getColumnLabel(i);
            bool bFound = false;
            OSQLColumns::Vector::const_iterator aFind = ::connectivity::find(aSelectColumns->get().begin(),aSelectColumns->get().end(),sColumnLabel,aCaseCompare);
            size_t nFoundSelectColumnPos = aFind - aSelectColumns->get().begin();
            if ( aFind != aSelectColumns->get().end() )
            {
                if ( aUsedSelectColumns.find( nFoundSelectColumnPos ) != aUsedSelectColumns.end() )
                {   // we found a column name which exists twice
                    // so we start after the first found
                    do
                    {
                        aFind = ::connectivity::findRealName(++aFind,aSelectColumns->get().end(),sColumnName,aCaseCompare);
                        nFoundSelectColumnPos = aFind - aSelectColumns->get().begin();
                    }
                    while   (   ( aUsedSelectColumns.find( nFoundSelectColumnPos ) != aUsedSelectColumns.end() )
                                &&  ( aFind != aSelectColumns->get().end() )
                            );
                }
                if ( aFind != aSelectColumns->get().end() )
                {
                    (*aFind)->getPropertyValue(PROPERTY_NAME) >>= sColumnName;
                    aUsedSelectColumns.insert( nFoundSelectColumnPos );
                    aNames.push_back(sColumnName);
                    bFound = true;
                }
            }

            if ( bFound )
                continue;

            OSQLColumns::Vector::const_iterator aRealFind = ::connectivity::findRealName(
                aSelectColumns->get().begin(), aSelectColumns->get().end(), sColumnName, aCaseCompare );

            if ( i > static_cast< sal_Int32>( aSelectColumns->get().size() ) )
            {
                aSelectColumns->get().push_back(
                    ::connectivity::parse::OParseColumn::createColumnForResultSet( xResultSetMeta, m_xMetaData, i ,aColumnNames)
                );
                OSL_ENSURE( aSelectColumns->get().size() == (size_t)i, "OSingleSelectQueryComposer::getColumns: inconsistency!" );
            }
            else if ( aRealFind == aSelectColumns->get().end() )
            {
                // we can now only look if we found it under the realname property
                // here we have to make the assumption that the position is correct
                OSQLColumns::Vector::iterator aFind2 = aSelectColumns->get().begin() + i-1;
                Reference<XPropertySet> xProp(*aFind2,UNO_QUERY);
                if ( !xProp.is() || !xProp->getPropertySetInfo()->hasPropertyByName( PROPERTY_REALNAME ) )
                    continue;

                ::connectivity::parse::OParseColumn* pColumn = new ::connectivity::parse::OParseColumn(xProp,bCase);
                pColumn->setFunction(::comphelper::getBOOL(xProp->getPropertyValue("Function")));
                pColumn->setAggregateFunction(::comphelper::getBOOL(xProp->getPropertyValue("AggregateFunction")));

                OUString sRealName;
                xProp->getPropertyValue(PROPERTY_REALNAME) >>= sRealName;
                ::std::vector< OUString>::iterator aFindName;
                if ( sColumnName.isEmpty() )
                    xProp->getPropertyValue(PROPERTY_NAME) >>= sColumnName;

                aFindName = ::std::find_if(aNames.begin(),aNames.end(),::std::bind2nd(aCaseCompare,sColumnName));
                sal_Int32 j = 0;
                while ( aFindName != aNames.end() )
                {
                    sColumnName += OUString::number(++j);
                    aFindName = ::std::find_if(aNames.begin(),aNames.end(),::std::bind2nd(aCaseCompare,sColumnName));
                }

                pColumn->setName(sColumnName);
                pColumn->setRealName(sRealName);
                pColumn->setTableName(::comphelper::getString(xProp->getPropertyValue(PROPERTY_TABLENAME)));

                (aSelectColumns->get())[i-1] = pColumn;
            }
            else
                continue;

            aUsedSelectColumns.insert( (size_t)(i - 1) );
            aNames.push_back( sColumnName );
        }
    }
    catch(const Exception&)
    {
    }

    } while ( false );

    bool bMissingSomeColumnLabels = !aNames.empty() && aNames.size() != aSelectColumns->get().size();
    SAL_WARN_IF(bMissingSomeColumnLabels, "dbaccess", "We have column labels for *some* columns but not all");
    //^^this happens in the evolution address book where we have real column names of e.g.
    //first_name, second_name and city. On parsing via
    //OSQLParseTreeIterator::appendColumns it creates some labels using those real names
    //but the evo address book gives them proper labels of First Name, Second Name and City
    //the munge means that here we have e.g. just "City" as a label because it matches

    //This is all a horrible mess
    if (bMissingSomeColumnLabels)
        aNames.clear();

    if ( aNames.empty() )
        m_aCurrentColumns[ SelectColumns ] = OPrivateColumns::createWithIntrinsicNames( aSelectColumns, bCase, *this, m_aMutex );
    else
        m_aCurrentColumns[ SelectColumns ] = new OPrivateColumns( aSelectColumns, bCase, *this, m_aMutex, aNames );

    return m_aCurrentColumns[SelectColumns];
}

bool OSingleSelectQueryComposer::setORCriteria(OSQLParseNode* pCondition, OSQLParseTreeIterator& _rIterator,
                                    ::std::vector< ::std::vector < PropertyValue > >& rFilters, const Reference< css::util::XNumberFormatter > & xFormatter) const
{
    // Round brackets around the expression
    if (pCondition->count() == 3 &&
        SQL_ISPUNCTUATION(pCondition->getChild(0),"(") &&
        SQL_ISPUNCTUATION(pCondition->getChild(2),")"))
    {
        return setORCriteria(pCondition->getChild(1), _rIterator, rFilters, xFormatter);
    }
    // OR logic expression
    // a searchcondition can only look like this: search_condition SQL_TOKEN_OR boolean_term
    else if (SQL_ISRULE(pCondition,search_condition))
    {
        bool bResult = true;
        for (int i = 0; bResult && i < 3; i+=2)
        {
            // Is the first element a OR logic expression again?
            // Then descend recursively ...
            if (SQL_ISRULE(pCondition->getChild(i),search_condition))
                bResult = setORCriteria(pCondition->getChild(i), _rIterator, rFilters, xFormatter);
            else
            {
                rFilters.push_back( ::std::vector < PropertyValue >());
                bResult = setANDCriteria(pCondition->getChild(i), _rIterator, rFilters[rFilters.size() - 1], xFormatter);
            }
        }
        return bResult;
    }
    else
    {
        rFilters.push_back(::std::vector < PropertyValue >());
        return setANDCriteria(pCondition, _rIterator, rFilters[rFilters.size() - 1], xFormatter);
    }
}

bool OSingleSelectQueryComposer::setANDCriteria( OSQLParseNode * pCondition,
    OSQLParseTreeIterator& _rIterator, ::std::vector < PropertyValue >& rFilter, const Reference< XNumberFormatter > & xFormatter) const
{
    // Round brackets
    if (SQL_ISRULE(pCondition,boolean_primary))
    {
        // this should not occur
        SAL_WARN("dbaccess","boolean_primary in And-Criteria");
        return false;
    }
    // The first element is an AND logical expression again
    else if ( SQL_ISRULE(pCondition,boolean_term) && pCondition->count() == 3 )
    {
        return setANDCriteria(pCondition->getChild(0), _rIterator, rFilter, xFormatter) &&
               setANDCriteria(pCondition->getChild(2), _rIterator, rFilter, xFormatter);
    }
    else if (SQL_ISRULE(pCondition, comparison_predicate))
    {
        return setComparsionPredicate(pCondition,_rIterator,rFilter,xFormatter);
    }
    else if (SQL_ISRULE(pCondition,like_predicate) ||
             SQL_ISRULE(pCondition,test_for_null) ||
             SQL_ISRULE(pCondition,in_predicate) ||
             SQL_ISRULE(pCondition,all_or_any_predicate) ||
             SQL_ISRULE(pCondition,between_predicate))
    {
        if (SQL_ISRULE(pCondition->getChild(0), column_ref))
        {
            PropertyValue aItem;
            OUString aValue;
            OUString aColumnName;

            pCondition->parseNodeToStr( aValue, m_xConnection );
            pCondition->getChild(0)->parseNodeToStr( aColumnName, m_xConnection );

            // don't display the column name
            aValue = aValue.copy(aColumnName.getLength());
            aValue = aValue.trim();

            aItem.Name = getColumnName(pCondition->getChild(0),_rIterator);
            aItem.Value <<= aValue;
            aItem.Handle = 0; // just to know that this is not one the known ones
            if ( SQL_ISRULE(pCondition,like_predicate) )
            {
                if ( SQL_ISTOKEN(pCondition->getChild(1)->getChild(0),NOT) )
                    aItem.Handle = SQLFilterOperator::NOT_LIKE;
                else
                    aItem.Handle = SQLFilterOperator::LIKE;
            }
            else if (SQL_ISRULE(pCondition,test_for_null))
            {
                if (SQL_ISTOKEN(pCondition->getChild(1)->getChild(1),NOT) )
                    aItem.Handle = SQLFilterOperator::NOT_SQLNULL;
                else
                    aItem.Handle = SQLFilterOperator::SQLNULL;
            }
            else if (SQL_ISRULE(pCondition,in_predicate))
            {
                SAL_WARN("dbaccess", "OSingleSelectQueryComposer::setANDCriteria: in_predicate not implemented!" );
            }
            else if (SQL_ISRULE(pCondition,all_or_any_predicate))
            {
                SAL_WARN("dbaccess", "OSingleSelectQueryComposer::setANDCriteria: all_or_any_predicate not implemented!" );
            }
            else if (SQL_ISRULE(pCondition,between_predicate))
            {
                SAL_WARN("dbaccess", "OSingleSelectQueryComposer::setANDCriteria: between_predicate not implemented!" );
            }

            rFilter.push_back(aItem);
        }
        else
            return false;
    }
    else if (SQL_ISRULE(pCondition,existence_test) ||
             SQL_ISRULE(pCondition,unique_test))
    {
        // this couldn't be handled here, too complex
        // as we need a field name
        return false;
    }
    else
        return false;

    return true;
}

sal_Int32 OSingleSelectQueryComposer::getPredicateType(OSQLParseNode * _pPredicate)
{
    sal_Int32 nPredicate = SQLFilterOperator::EQUAL;
    switch (_pPredicate->getNodeType())
    {
        case SQL_NODE_EQUAL:
            nPredicate = SQLFilterOperator::EQUAL;
            break;
        case SQL_NODE_NOTEQUAL:
            nPredicate = SQLFilterOperator::NOT_EQUAL;
            break;
        case SQL_NODE_LESS:
            nPredicate = SQLFilterOperator::LESS;
            break;
        case SQL_NODE_LESSEQ:
            nPredicate = SQLFilterOperator::LESS_EQUAL;
            break;
        case SQL_NODE_GREAT:
            nPredicate = SQLFilterOperator::GREATER;
            break;
        case SQL_NODE_GREATEQ:
            nPredicate = SQLFilterOperator::GREATER_EQUAL;
            break;
        default:
            SAL_WARN("dbaccess","Wrong NodeType!");
    }
    return nPredicate;
}

bool OSingleSelectQueryComposer::setComparsionPredicate(OSQLParseNode * pCondition, OSQLParseTreeIterator& _rIterator,
                                            ::std::vector < PropertyValue >& rFilter, const Reference< css::util::XNumberFormatter > & xFormatter) const
{
    OSL_ENSURE(SQL_ISRULE(pCondition, comparison_predicate),"setComparsionPredicate: pCondition ist kein ComparsionPredicate");
    if (SQL_ISRULE(pCondition->getChild(0), column_ref) ||
        SQL_ISRULE(pCondition->getChild(pCondition->count()-1), column_ref))
    {
        PropertyValue aItem;
        OUString aValue;
        sal_uInt32 nPos;
        if (SQL_ISRULE(pCondition->getChild(0), column_ref))
        {
            nPos = 0;
            sal_uInt32 i=1;

            aItem.Handle = getPredicateType(pCondition->getChild(i));
            // don't display the equal
            if (pCondition->getChild(i)->getNodeType() == SQL_NODE_EQUAL)
                i++;

            // go forward
            for (;i < pCondition->count();i++)
                pCondition->getChild(i)->parseNodeToPredicateStr(
                    aValue, m_xConnection, xFormatter, m_aLocale, static_cast<sal_Char>(m_sDecimalSep.toChar() ) );
        }
        else if (SQL_ISRULE(pCondition->getChild(pCondition->count()-1), column_ref))
        {
            nPos = pCondition->count()-1;

            sal_Int32 i = pCondition->count() - 2;
            switch (pCondition->getChild(i)->getNodeType())
            {
                case SQL_NODE_EQUAL:
                    // don't display the equal
                    i--;
                    aItem.Handle = SQLFilterOperator::EQUAL;
                    break;
                case SQL_NODE_NOTEQUAL:
                    i--;
                    aItem.Handle = SQLFilterOperator::NOT_EQUAL;
                    break;
                case SQL_NODE_LESS:
                    // take the opposite as we change the order
                    i--;
                    aValue = ">=";
                    aItem.Handle = SQLFilterOperator::GREATER_EQUAL;
                    break;
                case SQL_NODE_LESSEQ:
                    // take the opposite as we change the order
                    i--;
                    aValue = ">";
                    aItem.Handle = SQLFilterOperator::GREATER;
                    break;
                case SQL_NODE_GREAT:
                    // take the opposite as we change the order
                    i--;
                    aValue = "<=";
                    aItem.Handle = SQLFilterOperator::LESS_EQUAL;
                    break;
                case SQL_NODE_GREATEQ:
                    // take the opposite as we change the order
                    i--;
                    aValue = "<";
                    aItem.Handle = SQLFilterOperator::LESS;
                    break;
                default:
                    break;
            }

            // go backward
            for (; i >= 0; i--)
                pCondition->getChild(i)->parseNodeToPredicateStr(
                    aValue, m_xConnection, xFormatter, m_aLocale, static_cast<sal_Char>( m_sDecimalSep.toChar() ) );
        }
        else
            return false;

        aItem.Name = getColumnName(pCondition->getChild(nPos),_rIterator);
        aItem.Value <<= aValue;
        rFilter.push_back(aItem);
    }
    else if (SQL_ISRULE(pCondition->getChild(0), set_fct_spec ) ||
             SQL_ISRULE(pCondition->getChild(0), general_set_fct))
    {
        PropertyValue aItem;
        OUString aValue;
        OUString aColumnName;

        pCondition->getChild(2)->parseNodeToPredicateStr(aValue, m_xConnection, xFormatter, m_aLocale, static_cast<sal_Char>( m_sDecimalSep.toChar() ) );
        pCondition->getChild(0)->parseNodeToPredicateStr( aColumnName, m_xConnection, xFormatter, m_aLocale, static_cast<sal_Char>( m_sDecimalSep .toChar() ) );

        aItem.Name = getColumnName(pCondition->getChild(0),_rIterator);
        aItem.Value <<= aValue;
        aItem.Handle = getPredicateType(pCondition->getChild(1));
        rFilter.push_back(aItem);
    }
    else // Can only be an expression
    {
        PropertyValue aItem;
        OUString aName, aValue;

        OSQLParseNode *pLhs = pCondition->getChild(0);
        OSQLParseNode *pRhs = pCondition->getChild(2);

        // Field names
        sal_uInt16 i;
        for (i=0;i< pLhs->count();i++)
             pLhs->getChild(i)->parseNodeToPredicateStr( aName, m_xConnection, xFormatter, m_aLocale, static_cast<sal_Char>( m_sDecimalSep.toChar() ) );

        // Criterion
        aItem.Handle = getPredicateType(pCondition->getChild(1));
        aValue       = pCondition->getChild(1)->getTokenValue();
        for(i=0;i< pRhs->count();i++)
            pRhs->getChild(i)->parseNodeToPredicateStr(aValue, m_xConnection, xFormatter, m_aLocale, static_cast<sal_Char>( m_sDecimalSep.toChar() ) );

        aItem.Name = aName;
        aItem.Value <<= aValue;
        rFilter.push_back(aItem);
    }
    return true;
}

// Functions for analysing SQL
OUString OSingleSelectQueryComposer::getColumnName( ::connectivity::OSQLParseNode* pColumnRef, OSQLParseTreeIterator& _rIterator )
{
    OUString aTableRange, aColumnName;
    _rIterator.getColumnRange(pColumnRef,aColumnName,aTableRange);
    return aColumnName;
}

OUString SAL_CALL OSingleSelectQueryComposer::getFilter(  ) throw(RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    return getSQLPart(Where,m_aAdditiveIterator,false);
}

OUString SAL_CALL OSingleSelectQueryComposer::getOrder(  ) throw(RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    return getSQLPart(Order,m_aAdditiveIterator,false);
}

OUString SAL_CALL OSingleSelectQueryComposer::getGroup(  ) throw (RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    return getSQLPart(Group,m_aAdditiveIterator,false);
}

OUString OSingleSelectQueryComposer::getHavingClause() throw (RuntimeException, std::exception)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    return getSQLPart(Having,m_aAdditiveIterator,false);
}

OUString OSingleSelectQueryComposer::getTableAlias(const Reference< XPropertySet >& column) const
{
    OUString sReturn;
    if(m_pTables && m_pTables->getCount() > 1)
    {
        OUString aCatalog,aSchema,aTable,aComposedName,aColumnName;
        if(column->getPropertySetInfo()->hasPropertyByName(PROPERTY_CATALOGNAME))
            column->getPropertyValue(PROPERTY_CATALOGNAME)  >>= aCatalog;
        if(column->getPropertySetInfo()->hasPropertyByName(PROPERTY_SCHEMANAME))
            column->getPropertyValue(PROPERTY_SCHEMANAME)   >>= aSchema;
        if(column->getPropertySetInfo()->hasPropertyByName(PROPERTY_TABLENAME))
            column->getPropertyValue(PROPERTY_TABLENAME)    >>= aTable;
        column->getPropertyValue(PROPERTY_NAME)         >>= aColumnName;

        Sequence< OUString> aNames(m_pTables->getElementNames());
        const OUString* pBegin     = aNames.getConstArray();
        const OUString* const pEnd = pBegin + aNames.getLength();

        if(aTable.isEmpty())
        { // we haven't found a table name, now we must search every table for this column
            for(;pBegin != pEnd;++pBegin)
            {
                Reference<XColumnsSupplier> xColumnsSupp;
                m_pTables->getByName(*pBegin) >>= xColumnsSupp;

                if(xColumnsSupp.is() && xColumnsSupp->getColumns()->hasByName(aColumnName))
                {
                    aTable = *pBegin;
                    break;
                }
            }
        }
        else
        {
            aComposedName = ::dbtools::composeTableName( m_xMetaData, aCatalog, aSchema, aTable, false, ::dbtools::eInDataManipulation );

            // Is this the right case for the table name?
            // Else, look for it with different case, if applicable.

            if(!m_pTables->hasByName(aComposedName))
            {
                ::comphelper::UStringMixLess aTmp(m_aAdditiveIterator.getTables().key_comp());
                ::comphelper::UStringMixEqual aComp(static_cast< ::comphelper::UStringMixLess*>(&aTmp)->isCaseSensitive());
                for(;pBegin != pEnd;++pBegin)
                {
                    Reference<XPropertySet> xTableProp;
                    m_pTables->getByName(*pBegin) >>= xTableProp;
                    OSL_ENSURE(xTableProp.is(),"Table isn't a propertyset!");
                    if(xTableProp.is())
                    {
                        OUString aCatalog2,aSchema2,aTable2;
                        xTableProp->getPropertyValue(PROPERTY_CATALOGNAME)  >>= aCatalog2;
                        xTableProp->getPropertyValue(PROPERTY_SCHEMANAME)   >>= aSchema2;
                        xTableProp->getPropertyValue(PROPERTY_NAME)         >>= aTable2;
                        if(aComp(aCatalog,aCatalog2) && aComp(aSchema,aSchema2) && aComp(aTable,aTable2))
                        {
                            aCatalog    = aCatalog2;
                            aSchema     = aSchema2;
                            aTable      = aTable2;
                            break;
                        }
                    }
                }
            }
        }
        if(pBegin != pEnd)
        {
            sReturn = ::dbtools::composeTableName( m_xMetaData, aCatalog, aSchema, aTable, true, ::dbtools::eInDataManipulation ) + ".";
        }
    }
    return sReturn;
}

Reference< XIndexAccess > SAL_CALL OSingleSelectQueryComposer::getParameters(  ) throw(RuntimeException, std::exception)
{
    // now set the Parameters
    if ( !m_aCurrentColumns[ParameterColumns] )
    {
        ::rtl::Reference< OSQLColumns> aCols = m_aSqlIterator.getParameters();
        ::std::vector< OUString> aNames;
        OSQLColumns::Vector::const_iterator aEnd = aCols->get().end();
        for(OSQLColumns::Vector::const_iterator aIter = aCols->get().begin(); aIter != aEnd;++aIter)
            aNames.push_back(getString((*aIter)->getPropertyValue(PROPERTY_NAME)));
        m_aCurrentColumns[ParameterColumns] = new OPrivateColumns(aCols,m_xMetaData->supportsMixedCaseQuotedIdentifiers(),*this,m_aMutex,aNames,true);
    }

    return m_aCurrentColumns[ParameterColumns];
}

void OSingleSelectQueryComposer::clearColumns( const EColumnType _eType )
{
    OPrivateColumns* pColumns = m_aCurrentColumns[ _eType ];
    if ( pColumns != nullptr )
    {
        pColumns->disposing();
        m_aColumnsCollection.push_back( pColumns );
        m_aCurrentColumns[ _eType ] = nullptr;
    }
}

void OSingleSelectQueryComposer::clearCurrentCollections()
{
    ::std::vector<OPrivateColumns*>::iterator aIter = m_aCurrentColumns.begin();
    ::std::vector<OPrivateColumns*>::iterator aEnd = m_aCurrentColumns.end();
    for (;aIter != aEnd;++aIter)
    {
        if ( *aIter )
        {
            (*aIter)->disposing();
            m_aColumnsCollection.push_back(*aIter);
            *aIter = nullptr;
        }
    }

    if(m_pTables)
    {
        m_pTables->disposing();
        m_aTablesCollection.push_back(m_pTables);
        m_pTables = nullptr;
    }
}

Reference< XIndexAccess > OSingleSelectQueryComposer::setCurrentColumns( EColumnType _eType,
    const ::rtl::Reference< OSQLColumns >& _rCols )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    // now set the group columns
    if ( !m_aCurrentColumns[_eType] )
    {
        ::std::vector< OUString> aNames;
        OSQLColumns::Vector::const_iterator aEnd = _rCols->get().end();
        for(OSQLColumns::Vector::const_iterator aIter = _rCols->get().begin(); aIter != aEnd;++aIter)
            aNames.push_back(getString((*aIter)->getPropertyValue(PROPERTY_NAME)));
        m_aCurrentColumns[_eType] = new OPrivateColumns(_rCols,m_xMetaData->supportsMixedCaseQuotedIdentifiers(),*this,m_aMutex,aNames,true);
    }

    return m_aCurrentColumns[_eType];
}

Reference< XIndexAccess > SAL_CALL OSingleSelectQueryComposer::getGroupColumns(  ) throw(RuntimeException, std::exception)
{
    return setCurrentColumns( GroupByColumns, m_aAdditiveIterator.getGroupColumns() );
}

Reference< XIndexAccess > SAL_CALL OSingleSelectQueryComposer::getOrderColumns(  ) throw(RuntimeException, std::exception)
{
    return setCurrentColumns( OrderColumns, m_aAdditiveIterator.getOrderColumns() );
}

OUString SAL_CALL OSingleSelectQueryComposer::getQueryWithSubstitution(  ) throw (SQLException, RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    OUString sSqlStatement( getQuery() );

    const OSQLParseNode* pStatementNode = m_aSqlIterator.getParseTree();
    if ( pStatementNode )
    {
        SQLException aError;
        if ( !pStatementNode->parseNodeToExecutableStatement( sSqlStatement, m_xConnection, m_aSqlParser, &aError ) )
            throw SQLException( aError );
    }

    return sSqlStatement;
}

OUString OSingleSelectQueryComposer::getStatementPart( TGetParseNode& _aGetFunctor, OSQLParseTreeIterator& _rIterator )
{
    OUString sResult;

    const OSQLParseNode* pNode = _aGetFunctor( &_rIterator );
    if ( pNode )
        pNode->parseNodeToStr( sResult, m_xConnection );

    return sResult;
}

namespace
{
    OUString lcl_getDecomposedColumnName(const OUString& rComposedName, const OUString& rQuoteString)
    {
        const sal_Int32 nQuoteLength = rQuoteString.getLength();
        OUString sName = rComposedName.trim();
        OUString sColumnName;
        sal_Int32 nPos, nRPos = 0;

        for (;;)
        {
            nPos = sName.indexOf( rQuoteString, nRPos );
            if ( nPos >= 0 )
            {
                nRPos = sName.indexOf( rQuoteString, nPos + nQuoteLength );
                if ( nRPos > nPos )
                {
                    if ( nRPos + nQuoteLength < sName.getLength() )
                    {
                        nRPos += nQuoteLength; // -1 + 1 skip dot
                    }
                    else
                    {
                        sColumnName = sName.copy( nPos + nQuoteLength, nRPos - nPos - nQuoteLength );
                        break;
                    }
                }
                else
                    break;
            }
            else
                break;
        }
        return sColumnName.isEmpty() ? rComposedName : sColumnName;
    }

    OUString lcl_getCondition(const Sequence< Sequence< PropertyValue > >& filter,
        const OPredicateInputController& i_aPredicateInputController,
        const Reference< XNameAccess >& i_xSelectColumns,
        const OUString& rQuoteString)
    {
        OUStringBuffer sRet;
        const Sequence< PropertyValue >* pOrIter = filter.getConstArray();
        const Sequence< PropertyValue >* pOrEnd = pOrIter + filter.getLength();
        while ( pOrIter != pOrEnd )
        {
            if ( pOrIter->getLength() )
            {
                sRet.append(L_BRACKET);
                const PropertyValue* pAndIter = pOrIter->getConstArray();
                const PropertyValue* pAndEnd = pAndIter + pOrIter->getLength();
                while ( pAndIter != pAndEnd )
                {
                    sRet.append(pAndIter->Name);
                    OUString sValue;
                    pAndIter->Value >>= sValue;
                    const OUString sColumnName = lcl_getDecomposedColumnName( pAndIter->Name, rQuoteString );
                    if ( i_xSelectColumns.is() && i_xSelectColumns->hasByName(sColumnName) )
                    {
                        Reference<XPropertySet> xColumn(i_xSelectColumns->getByName(sColumnName),UNO_QUERY);
                        sValue = i_aPredicateInputController.getPredicateValueStr(sValue,xColumn);
                    }
                    else
                    {
                        sValue = i_aPredicateInputController.getPredicateValueStr(pAndIter->Name,sValue);
                    }
                    lcl_addFilterCriteria_throw(pAndIter->Handle,sValue,sRet);
                    ++pAndIter;
                    if ( pAndIter != pAndEnd )
                        sRet.append(STR_AND);
                }
                sRet.append(R_BRACKET);
            }
            ++pOrIter;
            if ( pOrIter != pOrEnd && !sRet.isEmpty() )
                sRet.append(STR_OR);
        }
        return sRet.makeStringAndClear();
    }
}

void SAL_CALL OSingleSelectQueryComposer::setStructuredFilter( const Sequence< Sequence< PropertyValue > >& filter ) throw (SQLException, css::lang::IllegalArgumentException, RuntimeException, std::exception)
{
    OPredicateInputController aPredicateInput(m_aContext, m_xConnection, &m_aParseContext);
    setFilter(lcl_getCondition(filter, aPredicateInput, getColumns(), m_xMetaData->getIdentifierQuoteString()));
}

void SAL_CALL OSingleSelectQueryComposer::setStructuredHavingClause( const Sequence< Sequence< PropertyValue > >& filter ) throw (SQLException, RuntimeException, std::exception)
{
    OPredicateInputController aPredicateInput(m_aContext, m_xConnection);
    setHavingClause(lcl_getCondition(filter, aPredicateInput, getColumns(), m_xMetaData->getIdentifierQuoteString()));
}

void OSingleSelectQueryComposer::setConditionByColumn( const Reference< XPropertySet >& column, bool andCriteria ,::std::mem_fun1_t<bool,OSingleSelectQueryComposer,const OUString& >& _aSetFunctor,sal_Int32 filterOperator)
{
    try
    {
        ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

        if ( !column.is()
             || !column->getPropertySetInfo()->hasPropertyByName(PROPERTY_VALUE)
             || !column->getPropertySetInfo()->hasPropertyByName(PROPERTY_NAME)
             || !column->getPropertySetInfo()->hasPropertyByName(PROPERTY_TYPE))
            throw SQLException(DBACORE_RESSTRING(RID_STR_COLUMN_NOT_VALID),*this,SQLSTATE_GENERAL,1000,Any() );

        sal_Int32 nType = 0;
        column->getPropertyValue(PROPERTY_TYPE) >>= nType;
        sal_Int32 nSearchable = dbtools::getSearchColumnFlag(m_xConnection,nType);
        if(nSearchable == ColumnSearch::NONE)
            throw SQLException(DBACORE_RESSTRING(RID_STR_COLUMN_NOT_SEARCHABLE),*this,SQLSTATE_GENERAL,1000,Any() );

        ::osl::MutexGuard aGuard( m_aMutex );

        OUString aName;
        column->getPropertyValue(PROPERTY_NAME) >>= aName;

        Any aValue;
        column->getPropertyValue(PROPERTY_VALUE) >>= aValue;

        OUStringBuffer aSQL;
        const OUString aQuote    = m_xMetaData->getIdentifierQuoteString();
        getColumns();

        // TODO: if this is called for HAVING, check that the column is a GROUP BY column
        //       or that it is an aggregate function

        if ( m_aCurrentColumns[SelectColumns] && m_aCurrentColumns[SelectColumns]->hasByName(aName) )
        {
            Reference<XPropertySet> xColumn;
            m_aCurrentColumns[SelectColumns]->getByName(aName) >>= xColumn;
            OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME),"Property REALNAME not available!");
            OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_TABLENAME),"Property TABLENAME not available!");
            OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName("AggregateFunction"),"Property AggregateFunction not available!");

            OUString sRealName,sTableName;
            xColumn->getPropertyValue(PROPERTY_REALNAME)    >>= sRealName;
            xColumn->getPropertyValue(PROPERTY_TABLENAME)   >>= sTableName;
            if(sTableName.indexOf('.') != -1)
            {
                OUString aCatlog,aSchema,aTable;
                ::dbtools::qualifiedNameComponents(m_xMetaData,sTableName,aCatlog,aSchema,aTable,::dbtools::eInDataManipulation);
                sTableName = ::dbtools::composeTableName( m_xMetaData, aCatlog, aSchema, aTable, true, ::dbtools::eInDataManipulation );
            }
            else
                sTableName = ::dbtools::quoteName(aQuote,sTableName);

            if ( !::comphelper::getBOOL(xColumn->getPropertyValue("Function")) )
            {
                aSQL =  sTableName + "." + ::dbtools::quoteName( aQuote, sRealName );
            }
            else
                aSQL = sRealName;
        }
        else
        {
            aSQL = getTableAlias( column ) + ::dbtools::quoteName( aQuote, aName );
        }

        if ( aValue.hasValue() )
        {
            if(  !m_xTypeConverter.is() )
                m_xTypeConverter.set( Converter::create(m_aContext) );
            OSL_ENSURE(m_xTypeConverter.is(),"NO typeconverter!");

            if ( nType != DataType::BOOLEAN && DataType::BIT != nType )
            {
                lcl_addFilterCriteria_throw(filterOperator,"",aSQL);
            }

            switch(nType)
            {
            case DataType::VARCHAR:
            case DataType::CHAR:
            case DataType::LONGVARCHAR:
                aSQL.append( DBTypeConversion::toSQLString( nType, aValue, true, m_xTypeConverter ) );
                break;
            case DataType::CLOB:
                {
                    Reference< XClob > xClob(aValue,UNO_QUERY);
                    if ( xClob.is() )
                    {
                        const ::sal_Int64 nLength = xClob->length();
                        if ( sal_Int64(nLength + aSQL.getLength() + STR_LIKE.getLength() ) < sal_Int64(SAL_MAX_INT32) )
                        {
                            aSQL.append("'" + xClob->getSubString(1,(sal_Int32)nLength) + "'");
                        }
                    }
                    else
                    {
                        aSQL.append( DBTypeConversion::toSQLString( nType, aValue, true, m_xTypeConverter ) );
                    }
                }
                break;
            case DataType::VARBINARY:
            case DataType::BINARY:
            case DataType::LONGVARBINARY:
                {
                    Sequence<sal_Int8> aSeq;
                    if(aValue >>= aSeq)
                    {
                        if(nSearchable == ColumnSearch::CHAR)
                        {
                            aSQL.append( "\'" );
                        }
                        aSQL.append( "0x" );
                        const sal_Int8* pBegin  = aSeq.getConstArray();
                        const sal_Int8* pEnd    = pBegin + aSeq.getLength();
                        for(;pBegin != pEnd;++pBegin)
                        {
                            aSQL.append( (sal_Int32)*pBegin, 16 );
                        }
                        if(nSearchable == ColumnSearch::CHAR)
                            aSQL.append( "\'" );
                    }
                    else
                        throw SQLException(DBACORE_RESSTRING(RID_STR_NOT_SEQUENCE_INT8),*this,SQLSTATE_GENERAL,1000,Any() );
                }
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                {
                    bool bValue = false;
                    m_xTypeConverter->convertToSimpleType(aValue, TypeClass_BOOLEAN) >>= bValue;

                    OUString sColumnExp = aSQL.makeStringAndClear();
                    getBooleanComparisonPredicate( sColumnExp, bValue, m_nBoolCompareMode, aSQL );
                }
                break;
            default:
                aSQL.append( DBTypeConversion::toSQLString( nType, aValue, true, m_xTypeConverter ) );
                break;
            }
        }
        else
        {
            sal_Int32 nFilterOp = filterOperator;
            if ( filterOperator != SQLFilterOperator::SQLNULL && filterOperator != SQLFilterOperator::NOT_SQLNULL )
                nFilterOp = SQLFilterOperator::SQLNULL;
            lcl_addFilterCriteria_throw(nFilterOp,"",aSQL);
        }

        // Attach filter
        // Construct SELECT without WHERE and ORDER BY
        OUString sFilter = getFilter();

        if ( !sFilter.isEmpty() && !aSQL.isEmpty() )
        {
            OUString sTemp(L_BRACKET + sFilter + R_BRACKET);
            sTemp += andCriteria ? OUString(STR_AND) : OUString(STR_OR);
            sFilter = sTemp;
        }
        sFilter += aSQL.makeStringAndClear();

        // add the filter and the sort order
        _aSetFunctor(this,sFilter);
    }
    catch (css::lang::WrappedTargetException & e)
    {
        if (e.TargetException.isExtractableTo(
                cppu::UnoType<css::sdbc::SQLException>::get()))
        {
            cppu::throwException(e.TargetException);
        }
        else
        {
            throw;
        }
    }
}

Sequence< Sequence< PropertyValue > > OSingleSelectQueryComposer::getStructuredCondition( TGetParseNode& _aGetFunctor )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    MutexGuard aGuard(m_aMutex);

    Sequence< Sequence< PropertyValue > > aFilterSeq;
    OUString sFilter = getStatementPart( _aGetFunctor, m_aAdditiveIterator );

    if ( !sFilter.isEmpty() )
    {
        OUString aSql(m_aPureSelectSQL + STR_WHERE + sFilter);
        // build a temporary parse node
        const OSQLParseNode* pTempNode = m_aAdditiveIterator.getParseTree();

        OUString aErrorMsg;
        std::unique_ptr<OSQLParseNode> pSqlParseNode( m_aSqlParser.parseTree(aErrorMsg,aSql));
        if ( pSqlParseNode.get() )
        {
            m_aAdditiveIterator.setParseTree(pSqlParseNode.get());
            // normalize the filter
            OSQLParseNode* pWhereNode = const_cast<OSQLParseNode*>(m_aAdditiveIterator.getWhereTree());

            OSQLParseNode* pCondition = pWhereNode->getChild(1);
        #if OSL_DEBUG_LEVEL > 0
            OUString sCondition;
            pCondition->parseNodeToStr( sCondition, m_xConnection );
        #endif
            OSQLParseNode::negateSearchCondition(pCondition);

            pCondition = pWhereNode->getChild(1);
        #if OSL_DEBUG_LEVEL > 0
            sCondition.clear();
            pCondition->parseNodeToStr( sCondition, m_xConnection );
        #endif
            OSQLParseNode::disjunctiveNormalForm(pCondition);

            pCondition = pWhereNode->getChild(1);
        #if OSL_DEBUG_LEVEL > 0
            sCondition.clear();
            pCondition->parseNodeToStr( sCondition, m_xConnection );
        #endif
            OSQLParseNode::absorptions(pCondition);

            pCondition = pWhereNode->getChild(1);
        #if OSL_DEBUG_LEVEL > 0
            sCondition.clear();
            pCondition->parseNodeToStr( sCondition, m_xConnection );
        #endif
            if ( pCondition )
            {
                ::std::vector< ::std::vector < PropertyValue > > aFilters;
                Reference< XNumberFormatter > xFormatter( NumberFormatter::create(m_aContext), UNO_QUERY_THROW );
                xFormatter->attachNumberFormatsSupplier( m_xNumberFormatsSupplier );

                if (setORCriteria(pCondition, m_aAdditiveIterator, aFilters, xFormatter))
                {
                    aFilterSeq.realloc(aFilters.size());
                    Sequence<PropertyValue>* pFilters = aFilterSeq.getArray();
                    ::std::vector< ::std::vector < PropertyValue > >::const_iterator aEnd = aFilters.end();
                    ::std::vector< ::std::vector < PropertyValue > >::const_iterator i = aFilters.begin();
                    for ( ; i != aEnd ; ++i)
                    {
                        const ::std::vector < PropertyValue >& rProperties = *i;
                        pFilters->realloc(rProperties.size());
                        PropertyValue* pFilter = pFilters->getArray();
                        ::std::vector < PropertyValue >::const_iterator j = rProperties.begin();
                        ::std::vector < PropertyValue >::const_iterator aEnd2 = rProperties.end();
                        for ( ; j != aEnd2 ; ++j)
                        {
                            *pFilter = *j;
                            ++pFilter;
                        }
                        ++pFilters;
                    }
                }
            }
            // restore
            m_aAdditiveIterator.setParseTree(pTempNode);
        }
    }
    return aFilterSeq;
}

OUString OSingleSelectQueryComposer::getKeyword( SQLPart _ePart )
{
    OUString sKeyword;
    switch(_ePart)
    {
        default:
            SAL_WARN("dbaccess", "OSingleSelectQueryComposer::getKeyWord: Invalid enum value!" );
            // no break, fallback to WHERE
        case Where:
            sKeyword = STR_WHERE;
            break;
        case Group:
            sKeyword = STR_GROUP_BY;
            break;
        case Having:
            sKeyword = STR_HAVING;
            break;
        case Order:
            sKeyword = STR_ORDER_BY;
            break;
    }
    return sKeyword;
}

OUString OSingleSelectQueryComposer::getSQLPart( SQLPart _ePart, OSQLParseTreeIterator& _rIterator, bool _bWithKeyword )
{
    TGetParseNode F_tmp(&OSQLParseTreeIterator::getSimpleWhereTree);
    OUString sKeyword( getKeyword( _ePart ) );
    switch(_ePart)
    {
        case Where:
            F_tmp = TGetParseNode(&OSQLParseTreeIterator::getSimpleWhereTree);
            break;
        case Group:
            F_tmp = TGetParseNode (&OSQLParseTreeIterator::getSimpleGroupByTree);
            break;
        case Having:
            F_tmp = TGetParseNode(&OSQLParseTreeIterator::getSimpleHavingTree);
            break;
        case Order:
            F_tmp = TGetParseNode(&OSQLParseTreeIterator::getSimpleOrderTree);
            break;
        default:
            SAL_WARN("dbaccess","Invalid enum value!");
    }

    OUString sRet = getStatementPart( F_tmp, _rIterator );
    if ( _bWithKeyword && !sRet.isEmpty() )
        sRet = sKeyword + sRet;
    return sRet;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
