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


#include "composertools.hxx"
#include "core_resource.hrc"
#include "core_resource.hxx"
#include "dbastrings.hrc"
#include "HelperCollections.hxx"
#include "SingleSelectQueryComposer.hxx"
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
#include <cppuhelper/typeprovider.hxx>
#include <connectivity/predicateinput.hxx>
#include <rtl/logfile.hxx>
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

#define STR_SELECT      ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT "))
#define STR_FROM        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" FROM "))
#define STR_WHERE       ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" WHERE "))
#define STR_GROUP_BY    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" GROUP BY "))
#define STR_HAVING      ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" HAVING "))
#define STR_ORDER_BY    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ORDER BY "))
#define STR_AND         ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" AND "))
#define STR_OR          ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" OR "))
#define STR_LIKE        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" LIKE "))
#define L_BRACKET       ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("("))
#define R_BRACKET       ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"))
#define COMMA           ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","))

namespace
{
    /** parses the given statement, using the given parser, returns a parse node representing
        the statement

        If the statement cannot be parsed, an error is thrown.
    */
    const OSQLParseNode* parseStatement_throwError( OSQLParser& _rParser, const ::rtl::OUString& _rStatement, const Reference< XInterface >& _rxContext )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "frank.schoenheit@sun.com", "SingleSelectQueryComposer.cxx::parseStatement_throwError" );
        ::rtl::OUString aErrorMsg;
        const OSQLParseNode* pNewSqlParseNode = _rParser.parseTree( aErrorMsg, _rStatement );
        if ( !pNewSqlParseNode )
        {
            ::rtl::OUString sSQLStateGeneralError( getStandardSQLState( SQL_GENERAL_ERROR ) );
            SQLException aError2( aErrorMsg, _rxContext, sSQLStateGeneralError, 1000, Any() );
            SQLException aError1( _rStatement, _rxContext, sSQLStateGeneralError, 1000, makeAny( aError2 ) );
            throw SQLException(_rParser.getContext().getErrorMessage(OParseContext::ERROR_GENERAL),_rxContext,sSQLStateGeneralError,1000,makeAny(aError1));
        }
        return pNewSqlParseNode;
    }

    // .....................................................................
    /** checks whether the given parse node describes a valid single select statement, throws
        an error if not
    */
    void checkForSingleSelect_throwError( const OSQLParseNode* pStatementNode, OSQLParseTreeIterator& _rIterator,
        const Reference< XInterface >& _rxContext, const ::rtl::OUString& _rOriginatingCommand )
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

    // .....................................................................
    /** combines parseStatement_throwError and checkForSingleSelect_throwError
    */
    void parseAndCheck_throwError( OSQLParser& _rParser, const ::rtl::OUString& _rStatement,
        OSQLParseTreeIterator& _rIterator, const Reference< XInterface >& _rxContext )
    {
        RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "frank.schoenheit@sun.com", "SingleSelectQueryComposer.cxx::parseAndCheck_throwError" );
        const OSQLParseNode* pNode = parseStatement_throwError( _rParser, _rStatement, _rxContext );
        checkForSingleSelect_throwError( pNode, _rIterator, _rxContext, _rStatement );
    }

    // .....................................................................
    /** transforms a parse node describing a complete statement into a pure select
        statement, without any filter/order/groupby/having clauses
    */
    ::rtl::OUString getPureSelectStatement( const OSQLParseNode* _pRootNode, Reference< XConnection > _rxConnection )
    {
        ::rtl::OUString sSQL = STR_SELECT;
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
        _rIterator.setParseTree(NULL);
        delete pSqlParseNode;
        if ( _bDispose )
            _rIterator.dispose();
    }
    void lcl_addFilterCriteria_throw(sal_Int32 i_nFilterOperator,const ::rtl::OUString& i_sValue,::rtl::OUStringBuffer& o_sRet)
    {
        switch( i_nFilterOperator )
        {
            case SQLFilterOperator::EQUAL:
                o_sRet.append(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" = ")));
                o_sRet.append(i_sValue);
                break;
            case SQLFilterOperator::NOT_EQUAL:
                o_sRet.append(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" <> ")));
                o_sRet.append(i_sValue);
                break;
            case SQLFilterOperator::LESS:
                o_sRet.append(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" < ")));
                o_sRet.append(i_sValue);
                break;
            case SQLFilterOperator::GREATER:
                o_sRet.append(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" > ")));
                o_sRet.append(i_sValue);
                break;
            case SQLFilterOperator::LESS_EQUAL:
                o_sRet.append(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" <= ")));
                o_sRet.append(i_sValue);
                break;
            case SQLFilterOperator::GREATER_EQUAL:
                o_sRet.append(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" >= ")));
                o_sRet.append(i_sValue);
                break;
            case SQLFilterOperator::LIKE:
                o_sRet.append(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" LIKE ")));
                o_sRet.append(i_sValue);
                break;
            case SQLFilterOperator::NOT_LIKE:
                o_sRet.append(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" NOT LIKE ")));
                o_sRet.append(i_sValue);
                break;
            case SQLFilterOperator::SQLNULL:
                o_sRet.append(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" IS NULL")) );
                break;
            case SQLFilterOperator::NOT_SQLNULL:
                o_sRet.append(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" IS NOT NULL")) );
                break;
            default:
                throw SQLException();
        }
    }

}

DBG_NAME(OSingleSelectQueryComposer)

OSingleSelectQueryComposer::OSingleSelectQueryComposer(const Reference< XNameAccess>& _rxTables,
                               const Reference< XConnection>& _xConnection,
                               const ::comphelper::ComponentContext& _rContext )
    :OSubComponent(m_aMutex,_xConnection)
    ,OPropertyContainer(m_aBHelper)
    ,m_aSqlParser( _rContext.getLegacyServiceFactory() )
    ,m_aSqlIterator( _xConnection, _rxTables, m_aSqlParser, NULL )
    ,m_aAdditiveIterator( _xConnection, _rxTables, m_aSqlParser, NULL )
    ,m_aElementaryParts( (size_t)SQLPartCount )
    ,m_xConnection(_xConnection)
    ,m_xMetaData(_xConnection->getMetaData())
    ,m_xConnectionTables( _rxTables )
    ,m_aContext( _rContext )
    ,m_pTables(NULL)
    ,m_nBoolCompareMode( BooleanComparisonMode::EQUAL_INTEGER )
    ,m_nCommandType(CommandType::COMMAND)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::OSingleSelectQueryComposer" );
    DBG_CTOR(OSingleSelectQueryComposer,NULL);

    if ( !m_aContext.is() || !m_xConnection.is() || !m_xConnectionTables.is() )
        throw IllegalArgumentException();

    registerProperty(PROPERTY_ORIGINAL,PROPERTY_ID_ORIGINAL,PropertyAttribute::BOUND|PropertyAttribute::READONLY,&m_sOrignal,::getCppuType(&m_sOrignal));

    m_aCurrentColumns.resize(4);

    m_aLocale = SvtSysLocale().GetLocaleData().getLocale();
    m_xNumberFormatsSupplier = dbtools::getNumberFormats( m_xConnection, sal_True, m_aContext.getLegacyServiceFactory() );
    Reference< XLocaleData4 > xLocaleData( LocaleData::create(m_aContext.getUNOContext()) );
    LocaleDataItem aData = xLocaleData->getLocaleItem(m_aLocale);
    m_sDecimalSep = aData.decimalSeparator;
    OSL_ENSURE(m_sDecimalSep.getLength() == 1,"OSingleSelectQueryComposer::OSingleSelectQueryComposer decimal separator is not 1 length");
    try
    {
        Any aValue;
        Reference<XInterface> xDs = dbaccess::getDataSource(_xConnection);
        if ( dbtools::getDataSourceSetting(xDs,static_cast <rtl::OUString> (PROPERTY_BOOLEANCOMPARISONMODE),aValue) )
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
    DBG_DTOR(OSingleSelectQueryComposer,NULL);
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
void SAL_CALL OSingleSelectQueryComposer::disposing(void)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::disposing" );
    OSubComponent::disposing();

    MutexGuard aGuard(m_aMutex);

    resetIterator( m_aSqlIterator, true );
    resetIterator( m_aAdditiveIterator, true );

    m_xConnectionTables = NULL;
    m_xConnection       = NULL;

    clearCurrentCollections();
}
IMPLEMENT_FORWARD_XINTERFACE3(OSingleSelectQueryComposer,OSubComponent,OSingleSelectQueryComposer_BASE,OPropertyContainer)
IMPLEMENT_SERVICE_INFO1(OSingleSelectQueryComposer,"org.openoffice.comp.dba.OSingleSelectQueryComposer",SERVICE_NAME_SINGLESELECTQUERYCOMPOSER.ascii)
IMPLEMENT_TYPEPROVIDER3(OSingleSelectQueryComposer,OSubComponent,OSingleSelectQueryComposer_BASE,OPropertyContainer)
IMPLEMENT_PROPERTYCONTAINER_DEFAULTS(OSingleSelectQueryComposer)

// com::sun::star::lang::XUnoTunnel
sal_Int64 SAL_CALL OSingleSelectQueryComposer::getSomething( const Sequence< sal_Int8 >& rId ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getSomething" );
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    return sal_Int64(0);
}

// XSingleSelectQueryAnalyzer
::rtl::OUString SAL_CALL OSingleSelectQueryComposer::getQuery(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getQuery" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    TGetParseNode F_tmp(&OSQLParseTreeIterator::getParseTree);
    return getStatementPart(F_tmp,m_aSqlIterator);
}

void SAL_CALL OSingleSelectQueryComposer::setQuery( const ::rtl::OUString& command ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "frank.schoenheit@sun.com", "OSingleSelectQueryComposer::setQuery" );
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
        m_aElementaryParts[ eLoopParts ] = ::rtl::OUString();
}

void SAL_CALL OSingleSelectQueryComposer::setCommand( const ::rtl::OUString& Command,sal_Int32 _nCommandType ) throw(SQLException, RuntimeException)
{
    ::rtl::OUStringBuffer sSQL;
    switch(_nCommandType)
    {
        case CommandType::COMMAND:
            setElementaryQuery(Command);
            return;
        case CommandType::TABLE:
            if ( m_xConnectionTables->hasByName(Command) )
            {
                sSQL.appendAscii("SELECT * FROM ");
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
                String sMessage( DBACORE_RESSTRING( RID_STR_TABLE_DOES_NOT_EXIST ) );
                sMessage.SearchAndReplaceAscii( "$table$", Command );
                throwGenericSQLException(sMessage,*this);
            }
            break;
        case CommandType::QUERY:
            if ( m_xConnectionQueries->hasByName(Command) )
            {

                Reference<XPropertySet> xQuery(m_xConnectionQueries->getByName(Command),UNO_QUERY);
                ::rtl::OUString sCommand;
                xQuery->getPropertyValue(PROPERTY_COMMAND) >>= sCommand;
                sSQL.append(sCommand);
            }
            else
            {
                String sMessage( DBACORE_RESSTRING( RID_STR_QUERY_DOES_NOT_EXIST ) );
                sMessage.SearchAndReplaceAscii( "$table$", Command );
                throwGenericSQLException(sMessage,*this);
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
    ::rtl::OUString sCommand = sSQL.makeStringAndClear();
    setElementaryQuery(sCommand);
    m_sOrignal = sCommand;
}

void OSingleSelectQueryComposer::setQuery_Impl( const ::rtl::OUString& command )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "frank.schoenheit@sun.com", "OSingleSelectQueryComposer::setQuery_Impl" );
    // parse this
    parseAndCheck_throwError( m_aSqlParser, command, m_aSqlIterator, *this );

    // strip it from all clauses, to have the pure SELECT statement
    m_aPureSelectSQL = getPureSelectStatement( m_aSqlIterator.getParseTree(), m_xConnection );

    // update tables
    getTables();
}

Sequence< Sequence< PropertyValue > > SAL_CALL OSingleSelectQueryComposer::getStructuredHavingClause(  ) throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getStructuredHavingClause" );
    TGetParseNode F_tmp(&OSQLParseTreeIterator::getSimpleHavingTree);
    return getStructuredCondition(F_tmp);
}

Sequence< Sequence< PropertyValue > > SAL_CALL OSingleSelectQueryComposer::getStructuredFilter(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getStructuredFilter" );
    TGetParseNode F_tmp(&OSQLParseTreeIterator::getSimpleWhereTree);
    return getStructuredCondition(F_tmp);
}

void SAL_CALL OSingleSelectQueryComposer::appendHavingClauseByColumn( const Reference< XPropertySet >& column, sal_Bool andCriteria,sal_Int32 filterOperator ) throw (SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::appendHavingClauseByColumn" );
    ::std::mem_fun1_t<bool,OSingleSelectQueryComposer,::rtl::OUString> F_tmp(&OSingleSelectQueryComposer::implSetHavingClause);
    setConditionByColumn(column,andCriteria,F_tmp,filterOperator);
}

void SAL_CALL OSingleSelectQueryComposer::appendFilterByColumn( const Reference< XPropertySet >& column, sal_Bool andCriteria,sal_Int32 filterOperator ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::appendFilterByColumn" );
    ::std::mem_fun1_t<bool,OSingleSelectQueryComposer,::rtl::OUString> F_tmp(&OSingleSelectQueryComposer::implSetFilter);
    setConditionByColumn(column,andCriteria,F_tmp,filterOperator);
}

::rtl::OUString OSingleSelectQueryComposer::impl_getColumnName_throw(const Reference< XPropertySet >& column)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    getColumns();
    if ( !column.is()
        || !m_aCurrentColumns[SelectColumns]
        || !column->getPropertySetInfo()->hasPropertyByName(PROPERTY_NAME)
        )
        {
            String sError(DBACORE_RESSTRING(RID_STR_COLUMN_UNKNOWN_PROP));
            sError.SearchAndReplaceAscii("%value", ::rtl::OUString(PROPERTY_NAME));
            SQLException aErr(sError,*this,SQLSTATE_GENERAL,1000,Any() );
            throw SQLException(DBACORE_RESSTRING(RID_STR_COLUMN_NOT_VALID),*this,SQLSTATE_GENERAL,1000,makeAny(aErr) );
        }

    ::rtl::OUString aName,aNewName;
    column->getPropertyValue(PROPERTY_NAME)         >>= aName;

    if ( !m_xMetaData->supportsOrderByUnrelated() && m_aCurrentColumns[SelectColumns] && !m_aCurrentColumns[SelectColumns]->hasByName(aName))
    {
        String sError(DBACORE_RESSTRING(RID_STR_COLUMN_MUST_VISIBLE));
        sError.SearchAndReplaceAscii("%name", aName);
        throw SQLException(sError,*this,SQLSTATE_GENERAL,1000,Any() );
    }

    // Attach filter
    // Construct SELECT without WHERE and ORDER BY
    ::rtl::OUString aQuote  = m_xMetaData->getIdentifierQuoteString();
    if ( m_aCurrentColumns[SelectColumns]->hasByName(aName) )
    {
        Reference<XPropertySet> xColumn;
        m_aCurrentColumns[SelectColumns]->getByName(aName) >>= xColumn;
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME),"Property REALNAME not available!");
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_TABLENAME),"Property TABLENAME not available!");
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Function"))),"Property FUNCTION not available!");

        ::rtl::OUString sRealName,sTableName;
        xColumn->getPropertyValue(PROPERTY_REALNAME)    >>= sRealName;
        xColumn->getPropertyValue(PROPERTY_TABLENAME)   >>= sTableName;
        sal_Bool bFunction = sal_False;
        xColumn->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Function"))) >>= bFunction;
        if ( sRealName == aName )
        {
            if ( bFunction )
                aNewName = aName;
            else
            {
                if(sTableName.indexOf('.',0) != -1)
                {
                    ::rtl::OUString aCatlog,aSchema,aTable;
                    ::dbtools::qualifiedNameComponents(m_xMetaData,sTableName,aCatlog,aSchema,aTable,::dbtools::eInDataManipulation);
                    sTableName = ::dbtools::composeTableName( m_xMetaData, aCatlog, aSchema, aTable, sal_True, ::dbtools::eInDataManipulation );
                }
                else
                    sTableName = ::dbtools::quoteName(aQuote,sTableName);

                aNewName =  sTableName;
                aNewName += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("."));
                aNewName += ::dbtools::quoteName(aQuote,sRealName);
            }
        }
        else
            aNewName = ::dbtools::quoteName(aQuote,aName);
    }
    else
        aNewName = getTableAlias(column) + ::dbtools::quoteName(aQuote,aName);
    return aNewName;
}

void SAL_CALL OSingleSelectQueryComposer::appendOrderByColumn( const Reference< XPropertySet >& column, sal_Bool ascending ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::appendOrderByColumn" );
    ::osl::MutexGuard aGuard( m_aMutex );
    ::rtl::OUString sColumnName( impl_getColumnName_throw(column) );
    ::rtl::OUString sOrder = getOrder();
    if ( !(sOrder.isEmpty() || sColumnName.isEmpty()) )
        sOrder += COMMA;
    sOrder += sColumnName;
    if ( !(ascending || sColumnName.isEmpty()) )
        sOrder += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" DESC "));

    setOrder(sOrder);
}

void SAL_CALL OSingleSelectQueryComposer::appendGroupByColumn( const Reference< XPropertySet >& column) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::appendGroupByColumn" );
    ::osl::MutexGuard aGuard( m_aMutex );
    ::rtl::OUString sColumnName( impl_getColumnName_throw(column) );
    OrderCreator aComposer;
    aComposer.append( getGroup() );
    aComposer.append( sColumnName );
    setGroup( aComposer.getComposedAndClear() );
}

::rtl::OUString OSingleSelectQueryComposer::composeStatementFromParts( const ::std::vector< ::rtl::OUString >& _rParts )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::composeStatementFromParts" );
    OSL_ENSURE( _rParts.size() == (size_t)SQLPartCount, "OSingleSelectQueryComposer::composeStatementFromParts: invalid parts array!" );

    ::rtl::OUStringBuffer aSql( m_aPureSelectSQL );
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        if ( !_rParts[ eLoopParts ].isEmpty() )
        {
            aSql.append( getKeyword( eLoopParts ) );
            aSql.append( _rParts[ eLoopParts ] );
        }

    return aSql.makeStringAndClear();
}

::rtl::OUString SAL_CALL OSingleSelectQueryComposer::getElementaryQuery() throw (::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getElementaryQuery" );
    return composeStatementFromParts( m_aElementaryParts );
}

void SAL_CALL OSingleSelectQueryComposer::setElementaryQuery( const ::rtl::OUString& _rElementary ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "frank.schoenheit@sun.com", "OSingleSelectQueryComposer::setElementaryQuery" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    // remember the 4 current "additive" clauses
    ::std::vector< ::rtl::OUString > aAdditiveClauses( SQLPartCount );
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        aAdditiveClauses[ eLoopParts ] = getSQLPart( eLoopParts, m_aAdditiveIterator, sal_False );

    // clear the tables and columns
    clearCurrentCollections();
    // set and parse the new query
    setQuery_Impl( _rElementary );

    // get the 4 elementary parts of the statement
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        m_aElementaryParts[ eLoopParts ] = getSQLPart( eLoopParts, m_aSqlIterator, sal_False );

    // reset the the AdditiveIterator: m_aPureSelectSQL may have changed
    try
    {
        parseAndCheck_throwError( m_aSqlParser, composeStatementFromParts( aAdditiveClauses ), m_aAdditiveIterator, *this );
    }
    catch( const Exception& e )
    {
        (void)e;
        OSL_FAIL( "OSingleSelectQueryComposer::setElementaryQuery: there should be no error anymore for the additive statement!" );
        DBG_UNHANDLED_EXCEPTION();
        // every part of the additive statement should have passed other tests already, and should not
        // be able to cause any errors ... me thinks
    }
}

namespace
{
    ::rtl::OUString getComposedClause( const ::rtl::OUString _rElementaryClause, const ::rtl::OUString _rAdditionalClause,
        TokenComposer& _rComposer, const ::rtl::OUString _rKeyword )
    {
        _rComposer.clear();
        _rComposer.append( _rElementaryClause );
        _rComposer.append( _rAdditionalClause );
        ::rtl::OUString sComposed = _rComposer.getComposedAndClear();
        if ( !sComposed.isEmpty() )
            sComposed = _rKeyword + sComposed;
        return sComposed;
    }
}

void OSingleSelectQueryComposer::setSingleAdditiveClause( SQLPart _ePart, const ::rtl::OUString& _rClause )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::setSingleAdditiveClause" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    // if nothing is changed, do nothing
    if ( getSQLPart( _ePart, m_aAdditiveIterator, sal_False ) == _rClause )
        return;

    // collect the 4 single parts as they're currently set
    ::std::vector< ::rtl::OUString > aClauses;
    aClauses.reserve( (size_t)SQLPartCount );
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        aClauses.push_back( getSQLPart( eLoopParts, m_aSqlIterator, sal_True ) );

    // overwrite the one part in question here
    SAL_WNODEPRECATED_DECLARATIONS_PUSH
    ::std::auto_ptr< TokenComposer > pComposer;
    SAL_WNODEPRECATED_DECLARATIONS_POP
    if ( ( _ePart == Where ) || ( _ePart == Having ) )
        pComposer.reset( new FilterCreator );
    else
        pComposer.reset( new OrderCreator );
    aClauses[ _ePart ] = getComposedClause( m_aElementaryParts[ _ePart ], _rClause,
        *pComposer, getKeyword( _ePart ) );

    // construct the complete statement
    ::rtl::OUStringBuffer aSql(m_aPureSelectSQL);
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        aSql.append(aClauses[ eLoopParts ]);

    // set the query
    setQuery_Impl(aSql.makeStringAndClear());

    // clear column collections which (might) have changed
    clearColumns( ParameterColumns );
    if ( _ePart == Order )
        clearColumns( OrderColumns );
    if ( _ePart == Group )
        clearColumns( GroupByColumns );

    // also, since the "additive filter" change, we need to rebuild our "additive" statement
    aSql = m_aPureSelectSQL;
    // again, first get all the old additive parts
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        aClauses[ eLoopParts ] = getSQLPart( eLoopParts, m_aAdditiveIterator, sal_True );
    // then overwrite the one in question
    aClauses[ _ePart ] = getComposedClause( ::rtl::OUString(), _rClause, *pComposer, getKeyword( _ePart ) );
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
        OSL_FAIL( "OSingleSelectQueryComposer::setSingleAdditiveClause: there should be no error anymore for the additive statement!" );
        // every part of the additive statement should have passed other tests already, and should not
        // be able to cause any errors ... me thinks
    }
}

void SAL_CALL OSingleSelectQueryComposer::setFilter( const ::rtl::OUString& filter ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::setFilter" );
    setSingleAdditiveClause( Where, filter );
}

void SAL_CALL OSingleSelectQueryComposer::setOrder( const ::rtl::OUString& order ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::setOrder" );
    setSingleAdditiveClause( Order, order );
}

void SAL_CALL OSingleSelectQueryComposer::setGroup( const ::rtl::OUString& group ) throw (SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::setGroup" );
    setSingleAdditiveClause( Group, group );
}

void SAL_CALL OSingleSelectQueryComposer::setHavingClause( const ::rtl::OUString& filter ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::setHavingClause" );
    setSingleAdditiveClause( Having, filter );
}

// XTablesSupplier
Reference< XNameAccess > SAL_CALL OSingleSelectQueryComposer::getTables(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getTables" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_pTables )
    {
        const OSQLTables& aTables = m_aSqlIterator.getTables();
        ::std::vector< ::rtl::OUString> aNames;
        OSQLTables::const_iterator aEnd = aTables.end();
        for(OSQLTables::const_iterator aIter = aTables.begin(); aIter != aEnd;++aIter)
            aNames.push_back(aIter->first);

        m_pTables = new OPrivateTables(aTables,m_xMetaData->supportsMixedCaseQuotedIdentifiers(),*this,m_aMutex,aNames);
    }

    return m_pTables;
}

// XColumnsSupplier
Reference< XNameAccess > SAL_CALL OSingleSelectQueryComposer::getColumns(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getColumns" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !!m_aCurrentColumns[SelectColumns] )
        return m_aCurrentColumns[SelectColumns];

    ::std::vector< ::rtl::OUString> aNames;
    ::rtl::Reference< OSQLColumns> aSelectColumns;
    sal_Bool bCase = sal_True;
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

        ::rtl::OUStringBuffer aSQL;
        aSQL.append( m_aPureSelectSQL );
        aSQL.append( STR_WHERE );

        // preserve the original WHERE clause
        // #i102234#
        ::rtl::OUString sOriginalWhereClause = getSQLPart( Where, m_aSqlIterator, sal_False );
        if ( !sOriginalWhereClause.isEmpty() )
        {
            aSQL.appendAscii( " ( 0 = 1 ) AND ( " );
            aSQL.append( sOriginalWhereClause );
            aSQL.appendAscii( " ) " );
        }
        else
        {
            aSQL.appendAscii( " ( 0 = 1 ) " );
        }

        ::rtl::OUString sGroupBy = getSQLPart( Group, m_aSqlIterator, sal_True );
        if ( !sGroupBy.isEmpty() )
            aSQL.append( sGroupBy );

        ::rtl::OUString sSQL( aSQL.makeStringAndClear() );
        // normalize the statement so that it doesn't contain any application-level features anymore
        ::rtl::OUString sError;
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        const ::std::auto_ptr< OSQLParseNode > pStatementTree( m_aSqlParser.parseTree( sError, sSQL, false ) );
        SAL_WNODEPRECATED_DECLARATIONS_POP
        OSL_ENSURE( pStatementTree.get(), "OSingleSelectQueryComposer::getColumns: could not parse the column retrieval statement!" );
        if ( pStatementTree.get() )
            if ( !pStatementTree->parseNodeToExecutableStatement( sSQL, m_xConnection, m_aSqlParser, NULL ) )
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
        const ::comphelper::TStringMixEqualFunctor aCaseCompareFunctor( bCase );
        typedef ::std::set< size_t > SizeTSet;
        SizeTSet aUsedSelectColumns;
        ::connectivity::parse::OParseColumn::StringMap aColumnNames;

        sal_Int32 nCount = xResultSetMeta->getColumnCount();
        OSL_ENSURE( (size_t) nCount == aSelectColumns->get().size(), "OSingleSelectQueryComposer::getColumns: inconsistent column counts, this might result in wrong columns!" );
        for(sal_Int32 i=1;i<=nCount;++i)
        {
            ::rtl::OUString sColumnName = xResultSetMeta->getColumnName(i);
            ::rtl::OUString sColumnLabel;
            if ( xQueryColumns.is() && xQueryColumns->hasByName(sColumnName) )
            {
                Reference<XPropertySet> xQueryColumn(xQueryColumns->getByName(sColumnName),UNO_QUERY_THROW);
                xQueryColumn->getPropertyValue(PROPERTY_LABEL) >>= sColumnLabel;
            }
            else
                sColumnLabel = xResultSetMeta->getColumnLabel(i);
            sal_Bool bFound = sal_False;
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
                    bFound = sal_True;
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
                // we can now only look if we found it under the realname propertery
                // here we have to make the assumption that the position is correct
                OSQLColumns::Vector::iterator aFind2 = aSelectColumns->get().begin() + i-1;
                Reference<XPropertySet> xProp(*aFind2,UNO_QUERY);
                if ( !xProp.is() || !xProp->getPropertySetInfo()->hasPropertyByName( PROPERTY_REALNAME ) )
                    continue;

                ::connectivity::parse::OParseColumn* pColumn = new ::connectivity::parse::OParseColumn(xProp,bCase);
                pColumn->setFunction(::comphelper::getBOOL(xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Function")))));
                pColumn->setAggregateFunction(::comphelper::getBOOL(xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AggregateFunction")))));

                ::rtl::OUString sRealName;
                xProp->getPropertyValue(PROPERTY_REALNAME) >>= sRealName;
                ::std::vector< ::rtl::OUString>::iterator aFindName;
                if ( sColumnName.isEmpty() )
                    xProp->getPropertyValue(PROPERTY_NAME) >>= sColumnName;

                aFindName = ::std::find_if(aNames.begin(),aNames.end(),::std::bind2nd(aCaseCompareFunctor,sColumnName));
                sal_Int32 j = 0;
                while ( aFindName != aNames.end() )
                {
                    sColumnName += ::rtl::OUString::valueOf(++j);
                    aFindName = ::std::find_if(aNames.begin(),aNames.end(),::std::bind2nd(aCaseCompareFunctor,sColumnName));
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

    if ( aNames.empty() )
        m_aCurrentColumns[ SelectColumns ] = OPrivateColumns::createWithIntrinsicNames( aSelectColumns, bCase, *this, m_aMutex );
    else
        m_aCurrentColumns[ SelectColumns ] = new OPrivateColumns( aSelectColumns, bCase, *this, m_aMutex, aNames );

    return m_aCurrentColumns[SelectColumns];
}

sal_Bool OSingleSelectQueryComposer::setORCriteria(OSQLParseNode* pCondition, OSQLParseTreeIterator& _rIterator,
                                    ::std::vector< ::std::vector < PropertyValue > >& rFilters, const Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::setORCriteria" );
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
        sal_Bool bResult = sal_True;
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

sal_Bool OSingleSelectQueryComposer::setANDCriteria( OSQLParseNode * pCondition,
    OSQLParseTreeIterator& _rIterator, ::std::vector < PropertyValue >& rFilter, const Reference< XNumberFormatter > & xFormatter) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::setANDCriteria" );
    // Round brackets
    if (SQL_ISRULE(pCondition,boolean_primary))
    {
        // this should not occur
        OSL_FAIL("boolean_primary in And-Criteria");
        return sal_False;
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
            ::rtl::OUString aValue;
            ::rtl::OUString aColumnName;


            pCondition->parseNodeToStr( aValue, m_xConnection, NULL );
            pCondition->getChild(0)->parseNodeToStr( aColumnName, m_xConnection, NULL );

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
                OSL_FAIL( "OSingleSelectQueryComposer::setANDCriteria: in_predicate not implemented!" );
            }
            else if (SQL_ISRULE(pCondition,all_or_any_predicate))
            {
                OSL_FAIL( "OSingleSelectQueryComposer::setANDCriteria: all_or_any_predicate not implemented!" );
            }
            else if (SQL_ISRULE(pCondition,between_predicate))
            {
                OSL_FAIL( "OSingleSelectQueryComposer::setANDCriteria: between_predicate not implemented!" );
            }

            rFilter.push_back(aItem);
        }
        else
            return sal_False;
    }
    else if (SQL_ISRULE(pCondition,existence_test) ||
             SQL_ISRULE(pCondition,unique_test))
    {
        // this couldn't be handled here, too complex
        // as we need a field name
        return sal_False;
    }
    else
        return sal_False;

    return sal_True;
}

sal_Int32 OSingleSelectQueryComposer::getPredicateType(OSQLParseNode * _pPredicate) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getPredicateType" );
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
            OSL_FAIL("Wrong NodeType!");
    }
    return nPredicate;
}

sal_Bool OSingleSelectQueryComposer::setComparsionPredicate(OSQLParseNode * pCondition, OSQLParseTreeIterator& _rIterator,
                                            ::std::vector < PropertyValue >& rFilter, const Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::setComparsionPredicate" );
    OSL_ENSURE(SQL_ISRULE(pCondition, comparison_predicate),"setComparsionPredicate: pCondition ist kein ComparsionPredicate");
    if (SQL_ISRULE(pCondition->getChild(0), column_ref) ||
        SQL_ISRULE(pCondition->getChild(pCondition->count()-1), column_ref))
    {
        PropertyValue aItem;
        ::rtl::OUString aValue;
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
                    aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">="));
                    aItem.Handle = SQLFilterOperator::GREATER_EQUAL;
                    break;
                case SQL_NODE_LESSEQ:
                    // take the opposite as we change the order
                    i--;
                    aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(">"));
                    aItem.Handle = SQLFilterOperator::GREATER;
                    break;
                case SQL_NODE_GREAT:
                    // take the opposite as we change the order
                    i--;
                    aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<="));
                    aItem.Handle = SQLFilterOperator::LESS_EQUAL;
                    break;
                case SQL_NODE_GREATEQ:
                    // take the opposite as we change the order
                    i--;
                    aValue = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("<"));
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
            return sal_False;

        aItem.Name = getColumnName(pCondition->getChild(nPos),_rIterator);
        aItem.Value <<= aValue;
        rFilter.push_back(aItem);
    }
    else if (SQL_ISRULE(pCondition->getChild(0), set_fct_spec ) ||
             SQL_ISRULE(pCondition->getChild(0), general_set_fct))
    {
        PropertyValue aItem;
        ::rtl::OUString aValue;
        ::rtl::OUString aColumnName;

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
        ::rtl::OUString aName, aValue;

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
    return sal_True;
}

// Functions for analysing SQL
::rtl::OUString OSingleSelectQueryComposer::getColumnName( ::connectivity::OSQLParseNode* pColumnRef, OSQLParseTreeIterator& _rIterator ) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getColumnName" );
    ::rtl::OUString aTableRange, aColumnName;
    _rIterator.getColumnRange(pColumnRef,aColumnName,aTableRange);
    return aColumnName;
}

::rtl::OUString SAL_CALL OSingleSelectQueryComposer::getFilter(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getFilter" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    return getSQLPart(Where,m_aAdditiveIterator,sal_False);
}

::rtl::OUString SAL_CALL OSingleSelectQueryComposer::getOrder(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getOrder" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    return getSQLPart(Order,m_aAdditiveIterator,sal_False);
}

::rtl::OUString SAL_CALL OSingleSelectQueryComposer::getGroup(  ) throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getGroup" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    return getSQLPart(Group,m_aAdditiveIterator,sal_False);
}

::rtl::OUString OSingleSelectQueryComposer::getHavingClause() throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getHavingClause" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    return getSQLPart(Having,m_aAdditiveIterator,sal_False);
}

::rtl::OUString OSingleSelectQueryComposer::getTableAlias(const Reference< XPropertySet >& column) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getTableAlias" );
    ::rtl::OUString sReturn;
    if(m_pTables && m_pTables->getCount() > 1)
    {
        ::rtl::OUString aCatalog,aSchema,aTable,aComposedName,aColumnName;
        column->getPropertyValue(PROPERTY_CATALOGNAME)  >>= aCatalog;
        column->getPropertyValue(PROPERTY_SCHEMANAME)   >>= aSchema;
        column->getPropertyValue(PROPERTY_TABLENAME)    >>= aTable;
        column->getPropertyValue(PROPERTY_NAME)         >>= aColumnName;

        Sequence< ::rtl::OUString> aNames(m_pTables->getElementNames());
        const ::rtl::OUString* pBegin   = aNames.getConstArray();
        const ::rtl::OUString* pEnd     = pBegin + aNames.getLength();

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
            aComposedName = ::dbtools::composeTableName( m_xMetaData, aCatalog, aSchema, aTable, sal_False, ::dbtools::eInDataManipulation );

            // first check if this is the table we want to or has it a tablealias

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
                        ::rtl::OUString aCatalog2,aSchema2,aTable2;
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
            sReturn = ::dbtools::composeTableName( m_xMetaData, aCatalog, aSchema, aTable, sal_True, ::dbtools::eInDataManipulation );
            sReturn += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("."));
        }
    }
    return sReturn;
}

Reference< XIndexAccess > SAL_CALL OSingleSelectQueryComposer::getParameters(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getParameters" );
    // now set the Parameters
    if ( !m_aCurrentColumns[ParameterColumns] )
    {
        ::rtl::Reference< OSQLColumns> aCols = m_aSqlIterator.getParameters();
        ::std::vector< ::rtl::OUString> aNames;
        OSQLColumns::Vector::const_iterator aEnd = aCols->get().end();
        for(OSQLColumns::Vector::const_iterator aIter = aCols->get().begin(); aIter != aEnd;++aIter)
            aNames.push_back(getString((*aIter)->getPropertyValue(PROPERTY_NAME)));
        m_aCurrentColumns[ParameterColumns] = new OPrivateColumns(aCols,m_xMetaData->supportsMixedCaseQuotedIdentifiers(),*this,m_aMutex,aNames,sal_True);
    }

    return m_aCurrentColumns[ParameterColumns];
}

void OSingleSelectQueryComposer::clearColumns( const EColumnType _eType )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::clearColumns" );
    OPrivateColumns* pColumns = m_aCurrentColumns[ _eType ];
    if ( pColumns != NULL )
    {
        pColumns->disposing();
        m_aColumnsCollection.push_back( pColumns );
        m_aCurrentColumns[ _eType ] = NULL;
    }
}

void OSingleSelectQueryComposer::clearCurrentCollections()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::clearCurrentCollections" );
    ::std::vector<OPrivateColumns*>::iterator aIter = m_aCurrentColumns.begin();
    ::std::vector<OPrivateColumns*>::iterator aEnd = m_aCurrentColumns.end();
    for (;aIter != aEnd;++aIter)
    {
        if ( *aIter )
        {
            (*aIter)->disposing();
            m_aColumnsCollection.push_back(*aIter);
            *aIter = NULL;
        }
    }

    if(m_pTables)
    {
        m_pTables->disposing();
        m_aTablesCollection.push_back(m_pTables);
        m_pTables = NULL;
    }
}

Reference< XIndexAccess > OSingleSelectQueryComposer::setCurrentColumns( EColumnType _eType,
    const ::rtl::Reference< OSQLColumns >& _rCols )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::setCurrentColumns" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    // now set the group columns
    if ( !m_aCurrentColumns[_eType] )
    {
        ::std::vector< ::rtl::OUString> aNames;
        OSQLColumns::Vector::const_iterator aEnd = _rCols->get().end();
        for(OSQLColumns::Vector::const_iterator aIter = _rCols->get().begin(); aIter != aEnd;++aIter)
            aNames.push_back(getString((*aIter)->getPropertyValue(PROPERTY_NAME)));
        m_aCurrentColumns[_eType] = new OPrivateColumns(_rCols,m_xMetaData->supportsMixedCaseQuotedIdentifiers(),*this,m_aMutex,aNames,sal_True);
    }

    return m_aCurrentColumns[_eType];
}

Reference< XIndexAccess > SAL_CALL OSingleSelectQueryComposer::getGroupColumns(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getGroupColumns" );
    return setCurrentColumns( GroupByColumns, m_aAdditiveIterator.getGroupColumns() );
}

Reference< XIndexAccess > SAL_CALL OSingleSelectQueryComposer::getOrderColumns(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getOrderColumns" );
    return setCurrentColumns( OrderColumns, m_aAdditiveIterator.getOrderColumns() );
}

::rtl::OUString SAL_CALL OSingleSelectQueryComposer::getQueryWithSubstitution(  ) throw (SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getQueryWithSubstitution" );
    ::osl::MutexGuard aGuard( m_aMutex );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::rtl::OUString sSqlStatement( getQuery() );

    const OSQLParseNode* pStatementNode = m_aSqlIterator.getParseTree();
    if ( pStatementNode )
    {
        SQLException aError;
        if ( !pStatementNode->parseNodeToExecutableStatement( sSqlStatement, m_xConnection, m_aSqlParser, &aError ) )
            throw SQLException( aError );
    }

    return sSqlStatement;
}

::rtl::OUString OSingleSelectQueryComposer::getStatementPart( TGetParseNode& _aGetFunctor, OSQLParseTreeIterator& _rIterator )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getStatementPart" );
    ::rtl::OUString sResult;

    const OSQLParseNode* pNode = _aGetFunctor( &_rIterator );
    if ( pNode )
        pNode->parseNodeToStr( sResult, m_xConnection );

    return sResult;
}

namespace
{
    ::rtl::OUString lcl_getCondition(const Sequence< Sequence< PropertyValue > >& filter,const OPredicateInputController& i_aPredicateInputController,const Reference< XNameAccess >& i_xSelectColumns)
    {
        ::rtl::OUStringBuffer sRet;
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
                    ::rtl::OUString sValue;
                    pAndIter->Value >>= sValue;
                    if ( i_xSelectColumns.is() && i_xSelectColumns->hasByName(pAndIter->Name) )
                    {
                        Reference<XPropertySet> xColumn(i_xSelectColumns->getByName(pAndIter->Name),UNO_QUERY);
                        sValue = i_aPredicateInputController.getPredicateValue(sValue,xColumn,sal_True);
                    }
                    else
                    {
                        sValue = i_aPredicateInputController.getPredicateValue(pAndIter->Name,sValue,sal_True);
                    }
                    lcl_addFilterCriteria_throw(pAndIter->Handle,sValue,sRet);
                    ++pAndIter;
                    if ( pAndIter != pAndEnd )
                        sRet.append(STR_AND);
                }
                sRet.append(R_BRACKET);
            }
            ++pOrIter;
            if ( pOrIter != pOrEnd && sRet.getLength() )
                sRet.append(STR_OR);
        }
        return sRet.makeStringAndClear();
    }
}

void SAL_CALL OSingleSelectQueryComposer::setStructuredFilter( const Sequence< Sequence< PropertyValue > >& filter ) throw (SQLException, ::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::setStructuredFilter" );
    OPredicateInputController aPredicateInput(m_aContext.getLegacyServiceFactory(),m_xConnection);
    setFilter(lcl_getCondition(filter,aPredicateInput,getColumns()));
}

void SAL_CALL OSingleSelectQueryComposer::setStructuredHavingClause( const Sequence< Sequence< PropertyValue > >& filter ) throw (SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::setStructuredHavingClause" );
    OPredicateInputController aPredicateInput(m_aContext.getLegacyServiceFactory(),m_xConnection);
    setHavingClause(lcl_getCondition(filter,aPredicateInput,getColumns()));
}

void OSingleSelectQueryComposer::setConditionByColumn( const Reference< XPropertySet >& column, sal_Bool andCriteria ,::std::mem_fun1_t<bool,OSingleSelectQueryComposer,::rtl::OUString>& _aSetFunctor,sal_Int32 filterOperator)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::setConditionByColumn" );
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

    ::rtl::OUString aName;
    column->getPropertyValue(PROPERTY_NAME) >>= aName;

    Any aValue;
    column->getPropertyValue(PROPERTY_VALUE) >>= aValue;

    ::rtl::OUStringBuffer aSQL;
    const ::rtl::OUString aQuote    = m_xMetaData->getIdentifierQuoteString();
    getColumns();

    if ( m_aCurrentColumns[SelectColumns] && m_aCurrentColumns[SelectColumns]->hasByName(aName) )
    {
        Reference<XPropertySet> xColumn;
        m_aCurrentColumns[SelectColumns]->getByName(aName) >>= xColumn;
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME),"Property REALNAME not available!");
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_TABLENAME),"Property TABLENAME not available!");
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AggregateFunction"))),"Property AggregateFunction not available!");

        ::rtl::OUString sRealName,sTableName;
        xColumn->getPropertyValue(PROPERTY_REALNAME)    >>= sRealName;
        xColumn->getPropertyValue(PROPERTY_TABLENAME)   >>= sTableName;
        if(sTableName.indexOf('.',0) != -1)
        {
            ::rtl::OUString aCatlog,aSchema,aTable;
            ::dbtools::qualifiedNameComponents(m_xMetaData,sTableName,aCatlog,aSchema,aTable,::dbtools::eInDataManipulation);
            sTableName = ::dbtools::composeTableName( m_xMetaData, aCatlog, aSchema, aTable, sal_True, ::dbtools::eInDataManipulation );
        }
        else
            sTableName = ::dbtools::quoteName(aQuote,sTableName);

        if ( !::comphelper::getBOOL(xColumn->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Function")))) )
        {
            aSQL =  sTableName;
            aSQL.appendAscii( "." );
            aSQL.append( ::dbtools::quoteName( aQuote, sRealName ) );
        }
        else
            aSQL = sRealName;

    }
    else
    {
        aSQL = getTableAlias( column );
        aSQL.append( ::dbtools::quoteName( aQuote, aName ) );
    }

    if ( aValue.hasValue() )
    {
        if(  !m_xTypeConverter.is() )
            m_xTypeConverter.set( Converter::create(m_aContext.getUNOContext()) );
        OSL_ENSURE(m_xTypeConverter.is(),"NO typeconverter!");

        if ( nType != DataType::BOOLEAN && DataType::BIT != nType )
        {
            ::rtl::OUString sEmpty;
            lcl_addFilterCriteria_throw(filterOperator,sEmpty,aSQL);
        }

        switch(nType)
        {
            case DataType::VARCHAR:
            case DataType::CHAR:
            case DataType::LONGVARCHAR:
                aSQL.append( DBTypeConversion::toSQLString( nType, aValue, sal_True, m_xTypeConverter ) );
                break;
            case DataType::CLOB:
                {
                    Reference< XClob > xClob(aValue,UNO_QUERY);
                    if ( xClob.is() )
                    {
                        const ::sal_Int64 nLength = xClob->length();
                        if ( sal_Int64(nLength + aSQL.getLength() + STR_LIKE.getLength() ) < sal_Int64(SAL_MAX_INT32) )
                        {
                            aSQL.appendAscii("'");
                            aSQL.append( xClob->getSubString(1,(sal_Int32)nLength) );
                            aSQL.appendAscii("'");
                        }
                    }
                    else
                    {
                        aSQL.append( DBTypeConversion::toSQLString( nType, aValue, sal_True, m_xTypeConverter ) );
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
                            aSQL.appendAscii( "\'" );
                        }
                        aSQL.appendAscii( "0x" );
                        const sal_Int8* pBegin  = aSeq.getConstArray();
                        const sal_Int8* pEnd    = pBegin + aSeq.getLength();
                        for(;pBegin != pEnd;++pBegin)
                        {
                            aSQL.append( (sal_Int32)*pBegin, 16 ).getStr();
                        }
                        if(nSearchable == ColumnSearch::CHAR)
                            aSQL.appendAscii( "\'" );
                    }
                    else
                        throw SQLException(DBACORE_RESSTRING(RID_STR_NOT_SEQUENCE_INT8),*this,SQLSTATE_GENERAL,1000,Any() );
                }
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                {
                    sal_Bool bValue = sal_False;
                    m_xTypeConverter->convertToSimpleType(aValue, TypeClass_BOOLEAN) >>= bValue;

                    ::rtl::OUString sColumnExp = aSQL.makeStringAndClear();
                    getBoleanComparisonPredicate( sColumnExp, bValue, m_nBoolCompareMode, aSQL );
                }
                break;
            default:
                aSQL.append( DBTypeConversion::toSQLString( nType, aValue, sal_True, m_xTypeConverter ) );
                break;
        }
    }
    else
    {
        sal_Int32 nFilterOp = filterOperator;
        if ( filterOperator != SQLFilterOperator::SQLNULL && filterOperator != SQLFilterOperator::NOT_SQLNULL )
            nFilterOp = SQLFilterOperator::SQLNULL;
        ::rtl::OUString sEmpty;
        lcl_addFilterCriteria_throw(nFilterOp,sEmpty,aSQL);
    }

    // Attach filter
    // Construct SELECT without WHERE and ORDER BY
    ::rtl::OUString sFilter = getFilter();

    if ( !sFilter.isEmpty() && aSQL.getLength() )
    {
        ::rtl::OUString sTemp(L_BRACKET);
        sTemp += sFilter;
        sTemp += R_BRACKET;
        sTemp += andCriteria ? STR_AND : STR_OR;
        sFilter = sTemp;
    }
    sFilter += aSQL.makeStringAndClear();

    // add the filter and the sort order
    _aSetFunctor(this,sFilter);
}

Sequence< Sequence< PropertyValue > > OSingleSelectQueryComposer::getStructuredCondition( TGetParseNode& _aGetFunctor )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getStructuredCondition" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    MutexGuard aGuard(m_aMutex);

    Sequence< Sequence< PropertyValue > > aFilterSeq;
    ::rtl::OUString sFilter = getStatementPart( _aGetFunctor, m_aAdditiveIterator );

    if ( !sFilter.isEmpty() )
    {
        ::rtl::OUString aSql(m_aPureSelectSQL);
        // build a temporary parse node
        const OSQLParseNode* pTempNode = m_aAdditiveIterator.getParseTree();

        aSql += STR_WHERE;
        aSql += sFilter;

        ::rtl::OUString aErrorMsg;
        SAL_WNODEPRECATED_DECLARATIONS_PUSH
        ::std::auto_ptr<OSQLParseNode> pSqlParseNode( m_aSqlParser.parseTree(aErrorMsg,aSql));
        SAL_WNODEPRECATED_DECLARATIONS_POP
        if ( pSqlParseNode.get() )
        {
            m_aAdditiveIterator.setParseTree(pSqlParseNode.get());
            // normalize the filter
            OSQLParseNode* pWhereNode = const_cast<OSQLParseNode*>(m_aAdditiveIterator.getWhereTree());

            OSQLParseNode* pCondition = pWhereNode->getChild(1);
        #if OSL_DEBUG_LEVEL > 0
            ::rtl::OUString sCondition;
            pCondition->parseNodeToStr( sCondition, m_xConnection );
        #endif
            OSQLParseNode::negateSearchCondition(pCondition);

            pCondition = pWhereNode->getChild(1);
        #if OSL_DEBUG_LEVEL > 0
            sCondition = ::rtl::OUString();
            pCondition->parseNodeToStr( sCondition, m_xConnection );
        #endif
            OSQLParseNode::disjunctiveNormalForm(pCondition);

            pCondition = pWhereNode->getChild(1);
        #if OSL_DEBUG_LEVEL > 0
            sCondition = ::rtl::OUString();
            pCondition->parseNodeToStr( sCondition, m_xConnection );
        #endif
            OSQLParseNode::absorptions(pCondition);

            pCondition = pWhereNode->getChild(1);
        #if OSL_DEBUG_LEVEL > 0
            sCondition = ::rtl::OUString();
            pCondition->parseNodeToStr( sCondition, m_xConnection );
        #endif
            if ( pCondition )
            {
                ::std::vector< ::std::vector < PropertyValue > > aFilters;
                Reference< XNumberFormatter > xFormatter( NumberFormatter::create(m_aContext.getUNOContext()), UNO_QUERY_THROW );
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

::rtl::OUString OSingleSelectQueryComposer::getKeyword( SQLPart _ePart ) const
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getKeyword" );
    ::rtl::OUString sKeyword;
    switch(_ePart)
    {
        default:
            OSL_FAIL( "OSingleSelectQueryComposer::getKeyWord: Invalid enum value!" );
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

::rtl::OUString OSingleSelectQueryComposer::getSQLPart( SQLPart _ePart, OSQLParseTreeIterator& _rIterator, sal_Bool _bWithKeyword )
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OSingleSelectQueryComposer::getSQLPart" );
    TGetParseNode F_tmp(&OSQLParseTreeIterator::getSimpleWhereTree);
    ::rtl::OUString sKeyword( getKeyword( _ePart ) );
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
            OSL_FAIL("Invalid enum value!");
    }

    ::rtl::OUString sRet = getStatementPart( F_tmp, _rIterator );
    if ( _bWithKeyword && !sRet.isEmpty() )
        sRet = sKeyword + sRet;
    return sRet;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
