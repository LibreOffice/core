/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: SingleSelectQueryComposer.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:37:34 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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
 ************************************************************************/

#include "SingleSelectQueryComposer.hxx"

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_COLUMNSEARCH_HPP_
#include <com/sun/star/sdbc/ColumnSearch.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_DATATYPE_HPP_
#include <com/sun/star/sdbc/DataType.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLFILTEROPERATOR_HPP_
#include <com/sun/star/sdb/SQLFilterOperator.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATASUPPLIER_HPP_
#include <com/sun/star/sdbc/XResultSetMetaDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XRESULTSETMETADATA_HPP_
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_UNO_XAGGREGATION_HPP_
#include <com/sun/star/uno/XAggregation.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_PROCESSFACTORY_HXX_
#include <comphelper/processfactory.hxx>
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _DBA_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef _DBA_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _UTL_CONFIGMGR_HXX_
#include <unotools/configmgr.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_I18N_XLOCALEDATA_HPP_
#include <com/sun/star/i18n/XLocaleData.hpp>
#endif
#ifndef DBA_HELPERCOLLECTIONS_HXX
#include "HelperCollections.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef DBACCESS_SOURCE_CORE_INC_COMPOSERTOOLS_HXX
#include "composertools.hxx"
#endif
#include <memory>

using namespace dbaccess;
using namespace dbtools;
using namespace comphelper;
using namespace connectivity;
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

#define STR_SELECT      ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("SELECT "))
#define STR_FROM        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" FROM "))
#define STR_WHERE       ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" WHERE "))
#define STR_GROUP_BY    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" GROUP BY "))
#define STR_HAVING      ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" HAVING "))
#define STR_ORDER_BY    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" ORDER BY "))
#define STR_AND         ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" AND "))
#define STR_OR          ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" OR "))
#define STR_LIKE        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" LIKE "))
#define STR_EQUAL       ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" = "))
#define L_BRACKET       ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("("))
#define R_BRACKET       ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(")"))
#define COMMA           ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(","))

// -------------------------------------------------------------------------
namespace
{
    // .....................................................................
    /** parses the given statement, using the given parser, returns a parse node representing
        the statement

        If the statement cannot be parsed, an error is thrown.
    */
    const OSQLParseNode* parseStatement_throwError( OSQLParser& _rParser, const ::rtl::OUString& _rStatement, const Reference< XInterface >& _rxContext )
    {
        ::rtl::OUString aErrorMsg;
        const OSQLParseNode* pNewSqlParseNode = _rParser.parseTree( aErrorMsg, _rStatement );
        if ( !pNewSqlParseNode )
        {
            SQLException aError2(aErrorMsg,_rxContext,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY000")),1000,Any());
            SQLException aError1(_rStatement,_rxContext,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY000")),1000,makeAny(aError2));
            throw SQLException(_rParser.getContext().getErrorMessage(OParseContext::ERROR_GENERAL),_rxContext,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY000")),1000,makeAny(aError1));
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
        bool bIsSingleSelect = ( _rIterator.getStatementType() == SQL_STATEMENT_SELECT )
                            || ( _rIterator.getStatementType() == SQL_STATEMENT_SELECT_COUNT );

        // throw the error, if necessary
        if ( !bIsSingleSelect || SQL_ISRULE( pStatementNode, union_statement ) ) // #i4229# OJ
        {
            // restore the old node before throwing the exception
            _rIterator.setParseTree( pOldNode );
            // and now really ...
            SQLException aError1( _rOriginatingCommand, _rxContext, ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HY000" ) ), 1000, Any() );
            throw SQLException( DBACORE_RESSTRING( RID_STR_ONLY_QUERY ), _rxContext,
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HY000" ) ), 1000, makeAny( aError1 ) );
        }

        delete pOldNode;
    }

    // .....................................................................
    /** combines parseStatement_throwError and checkForSingleSelect_throwError
    */
    void parseAndCheck_throwError( OSQLParser& _rParser, const ::rtl::OUString& _rStatement,
        OSQLParseTreeIterator& _rIterator, const Reference< XInterface >& _rxContext )
    {
        const OSQLParseNode* pNode = parseStatement_throwError( _rParser, _rStatement, _rxContext );
        checkForSingleSelect_throwError( pNode, _rIterator, _rxContext, _rStatement );
    }

    // .....................................................................
    /** transforms a parse node describing a complete statement into a pure select
        statement, without any filter/order/groupby/having clauses
    */
    ::rtl::OUString getPureSelectStatement( const OSQLParseNode* _pRootNode, Reference< XDatabaseMetaData > _rxMedaData )
    {
        ::rtl::OUString sSQL = STR_SELECT;
        _pRootNode->getChild(1)->parseNodeToStr( sSQL, _rxMedaData );
        _pRootNode->getChild(2)->parseNodeToStr( sSQL, _rxMedaData );
        sSQL += STR_FROM;
        _pRootNode->getChild(3)->getChild(0)->getChild(1)->parseNodeToStr( sSQL, _rxMedaData );
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
}

DBG_NAME(OSingleSelectQueryComposer)
// -------------------------------------------------------------------------
OSingleSelectQueryComposer::OSingleSelectQueryComposer(const Reference< XNameAccess>& _xTableSupplier,
                               const Reference< XConnection>& _xConnection,
                               const Reference< XMultiServiceFactory >& _xServiceFactory)
    :OSubComponent(m_aMutex,_xConnection)
    ,OPropertyContainer(m_aBHelper)
    ,m_aSqlParser(_xServiceFactory)
    ,m_aSqlIterator(_xTableSupplier,_xConnection->getMetaData(),NULL)
    ,m_aAdditiveIterator(_xTableSupplier,_xConnection->getMetaData(),NULL)
    ,m_aElementaryParts( (size_t)SQLPartCount )
    ,m_xConnection(_xConnection)
    ,m_xMetaData(_xConnection->getMetaData())
    ,m_xTableSupplier(_xTableSupplier)
    ,m_xServiceFactory(_xServiceFactory)
    ,m_pTables(NULL)
    ,m_nBoolCompareMode(BOOL_COMPARISON_DEFAULT)
{
    DBG_CTOR(OSingleSelectQueryComposer,NULL);
    OSL_ENSURE(_xServiceFactory.is()," ServiceFactory cant be null!");
    OSL_ENSURE(_xConnection.is()," Connection cant be null!");
    OSL_ENSURE(_xTableSupplier.is(),"TableSupplier cant be null!");

    registerProperty(PROPERTY_ORIGINAL,PROPERTY_ID_ORIGINAL,PropertyAttribute::BOUND|PropertyAttribute::READONLY,&m_sOrignal,::getCppuType(&m_sOrignal));

    m_aCurrentColumns.resize(4);


    m_aLocale = SvtSysLocale().GetLocaleData().getLocale();
    m_xNumberFormatsSupplier = dbtools::getNumberFormats(m_xConnection,sal_True,m_xServiceFactory);
    Reference< XLocaleData> xLocaleData = Reference<XLocaleData>(m_xServiceFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.i18n.LocaleData"))),UNO_QUERY);
    LocaleDataItem aData = xLocaleData->getLocaleItem(m_aLocale);
    m_sDecimalSep = aData.decimalSeparator;
    OSL_ENSURE(m_sDecimalSep.getLength() == 1,"OSingleSelectQueryComposer::OSingleSelectQueryComposer decimal separator is not 1 length");
    try
    {
        Reference< XChild> xChild(_xConnection, UNO_QUERY);
        if(xChild.is())
        {
            Reference< XPropertySet> xProp(xChild->getParent(),UNO_QUERY);
            if ( xProp.is() )
            {
                Sequence< PropertyValue > aInfo;
                xProp->getPropertyValue(PROPERTY_INFO) >>= aInfo;
                const PropertyValue* pBegin = aInfo.getConstArray();
                const PropertyValue* pEnd = pBegin + aInfo.getLength();
                for (; pBegin != pEnd; ++pBegin)
                {
                    if ( pBegin->Name == static_cast <rtl::OUString> (PROPERTY_BOOLEANCOMPARISONMODE) )
                    {
                        pBegin->Value >>= m_nBoolCompareMode;
                    }
                }
            }
        }
    }
    catch(Exception&)
    {
    }
}
// -------------------------------------------------------------------------
OSingleSelectQueryComposer::~OSingleSelectQueryComposer()
{
    DBG_DTOR(OSingleSelectQueryComposer,NULL);
    ::std::vector<OPrivateColumns*>::iterator aColIter = m_aColumnsCollection.begin();
    ::std::vector<OPrivateColumns*>::iterator aEnd = m_aColumnsCollection.end();
    for(;aColIter != aEnd;++aColIter)
        delete *aColIter;

    ::std::vector<OPrivateTables*>::iterator aTabIter = m_aTablesCollection.begin();
    for(;aTabIter != m_aTablesCollection.end();++aTabIter)
        delete *aTabIter;
}
// -------------------------------------------------------------------------
// OComponentHelper
void SAL_CALL OSingleSelectQueryComposer::disposing(void)
{
    OSubComponent::disposing();

    MutexGuard aGuard(m_aMutex);

    resetIterator( m_aSqlIterator, true );
    resetIterator( m_aAdditiveIterator, true );

    m_xTableSupplier    = NULL;
    m_xConnection       = NULL;
    m_xServiceFactory   = NULL;

    clearCurrentCollections();
}
IMPLEMENT_FORWARD_XINTERFACE3(OSingleSelectQueryComposer,OSubComponent,OSingleSelectQueryComposer_BASE,OPropertyContainer)
IMPLEMENT_SERVICE_INFO1(OSingleSelectQueryComposer,"org.openoffice.comp.dba.OSingleSelectQueryComposer",SERVICE_NAME_SINGLESELECTQUERYCOMPOSER)
IMPLEMENT_TYPEPROVIDER3(OSingleSelectQueryComposer,OSubComponent,OSingleSelectQueryComposer_BASE,OPropertyContainer)
IMPLEMENT_PROPERTYCONTAINER_DEFAULTS(OSingleSelectQueryComposer)

// -------------------------------------------------------------------------
// com::sun::star::lang::XUnoTunnel
sal_Int64 SAL_CALL OSingleSelectQueryComposer::getSomething( const Sequence< sal_Int8 >& rId ) throw(RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    return sal_Int64(0);
}

// -------------------------------------------------------------------------
// XSingleSelectQueryAnalyzer
::rtl::OUString SAL_CALL OSingleSelectQueryComposer::getQuery(  ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    TGetParseNode F_tmp(&OSQLParseTreeIterator::getParseTree);
    return getStatementPart(F_tmp,m_aSqlIterator);
}

// -------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::setQuery( const ::rtl::OUString& command ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
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
// -----------------------------------------------------------------------------
void OSingleSelectQueryComposer::setQuery_Impl( const ::rtl::OUString& command )
{
    // parse this
    parseAndCheck_throwError( m_aSqlParser, command, m_aSqlIterator, *this );

    // strip it from all clauses, to have the pure SELECT statement
    m_aPureSelectSQL = getPureSelectStatement( m_aSqlIterator.getParseTree(), m_xMetaData );

    // update columns and tables
    getColumns();
    getTables();
}
// -----------------------------------------------------------------------------
Sequence< Sequence< PropertyValue > > SAL_CALL OSingleSelectQueryComposer::getStructuredHavingClause(  ) throw (RuntimeException)
{
    TGetParseNode F_tmp(&OSQLParseTreeIterator::getSimpleHavingTree);
    return getStructuredCondition(F_tmp);
}
// -------------------------------------------------------------------------
Sequence< Sequence< PropertyValue > > SAL_CALL OSingleSelectQueryComposer::getStructuredFilter(  ) throw(RuntimeException)
{
    TGetParseNode F_tmp(&OSQLParseTreeIterator::getSimpleWhereTree);
    return getStructuredCondition(F_tmp);
}
// -----------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::appendHavingClauseByColumn( const Reference< XPropertySet >& column, sal_Bool andCriteria ) throw (SQLException, RuntimeException)
{
    ::std::mem_fun1_t<bool,OSingleSelectQueryComposer,::rtl::OUString> F_tmp(&OSingleSelectQueryComposer::implSetHavingClause);
    setConditionByColumn(column,andCriteria,F_tmp);
}
// -----------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::appendFilterByColumn( const Reference< XPropertySet >& column, sal_Bool andCriteria ) throw(SQLException, RuntimeException)
{
    ::std::mem_fun1_t<bool,OSingleSelectQueryComposer,::rtl::OUString> F_tmp(&OSingleSelectQueryComposer::implSetFilter);
    setConditionByColumn(column,andCriteria,F_tmp);
}
// -------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::appendOrderByColumn( const Reference< XPropertySet >& column, sal_Bool ascending ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

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

    ::osl::MutexGuard aGuard( m_aMutex );

    ::rtl::OUString aName,aAppendOrder;
    column->getPropertyValue(PROPERTY_NAME)         >>= aName;

    if ( !m_xMetaData->supportsOrderByUnrelated() && m_aCurrentColumns[SelectColumns] && !m_aCurrentColumns[SelectColumns]->hasByName(aName))
    {
        String sError(DBACORE_RESSTRING(RID_STR_COLUMN_MUST_VISIBLE));
        sError.SearchAndReplaceAscii("%name", aName);
        throw SQLException(sError,*this,SQLSTATE_GENERAL,1000,Any() );
    }

    // filter anhaengen
    // select ohne where und order by aufbauen
    ::rtl::OUString aSql(m_aPureSelectSQL);
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
                aAppendOrder += aName;
            else
            {
                if(sTableName.indexOf('.',0) != -1)
                {
                    ::rtl::OUString aCatlog,aSchema,aTable;
                    ::dbtools::qualifiedNameComponents(m_xMetaData,sTableName,aCatlog,aSchema,aTable,::dbtools::eInDataManipulation);
                    ::dbtools::composeTableName(m_xMetaData,aCatlog,aSchema,aTable,sTableName,sal_True,::dbtools::eInDataManipulation);
                }
                else
                    sTableName = ::dbtools::quoteName(aQuote,sTableName);

                aAppendOrder =  sTableName;
                aAppendOrder += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("."));
                aAppendOrder += ::dbtools::quoteName(aQuote,sRealName);
            }
        }
        else
            aAppendOrder += ::dbtools::quoteName(aQuote,aName);
    }
    else
        aAppendOrder = getTableAlias(column) + ::dbtools::quoteName(aQuote,aName);

    ::rtl::OUString sOrder = getOrder();
    if ( (sOrder.getLength() != 0) && aAppendOrder.getLength() )
        sOrder += COMMA;
    sOrder += aAppendOrder;
    if ( !ascending && aAppendOrder.getLength() )
        sOrder += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" DESC "));

    setOrder(sOrder);
}

// -------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::appendGroupByColumn( const Reference< XPropertySet >& column) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    if  ( !column.is()
        || !m_aCurrentColumns[SelectColumns]
        || !column->getPropertySetInfo()->hasPropertyByName(PROPERTY_NAME)
        )
        {
            String sError(DBACORE_RESSTRING(RID_STR_COLUMN_UNKNOWN_PROP));
            sError.SearchAndReplaceAscii("%value", ::rtl::OUString(PROPERTY_NAME));
            SQLException aErr(sError,*this,SQLSTATE_GENERAL,1000,Any() );
            throw SQLException(DBACORE_RESSTRING(RID_STR_COLUMN_NOT_VALID),*this,SQLSTATE_GENERAL,1000,makeAny(aErr) );
        }

    ::osl::MutexGuard aGuard( m_aMutex );

    ::rtl::OUString aName,aAppendOrder;
    column->getPropertyValue(PROPERTY_NAME)         >>= aName;

    if ( !m_xMetaData->supportsGroupByUnrelated() && m_aCurrentColumns[SelectColumns] && !m_aCurrentColumns[SelectColumns]->hasByName(aName))
    {
        String sError(DBACORE_RESSTRING(RID_STR_COLUMN_MUST_VISIBLE));
        sError.SearchAndReplaceAscii("%name", aName);
        throw SQLException(sError,*this,SQLSTATE_GENERAL,1000,Any() );
    }

    // filter anhaengen
    // select ohne where und order by aufbauen
    ::rtl::OUString aSql(m_aPureSelectSQL);
    ::rtl::OUString aQuote  = m_xMetaData->getIdentifierQuoteString();
    if ( m_aCurrentColumns[SelectColumns]->hasByName(aName) )
    {
        Reference<XPropertySet> xColumn;
        m_aCurrentColumns[SelectColumns]->getByName(aName) >>= xColumn;
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME),"Property REALNAME not available!");
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_TABLENAME),"Property TABLENAME not available!");

        ::rtl::OUString sRealName,sTableName;
        xColumn->getPropertyValue(PROPERTY_REALNAME)    >>= sRealName;
        xColumn->getPropertyValue(PROPERTY_TABLENAME)   >>= sTableName;
        if(sTableName.indexOf('.',0) != -1)
        {
            ::rtl::OUString aCatlog,aSchema,aTable;
            ::dbtools::qualifiedNameComponents(m_xMetaData,sTableName,aCatlog,aSchema,aTable,::dbtools::eInDataManipulation);
            ::dbtools::composeTableName(m_xMetaData,aCatlog,aSchema,aTable,sTableName,sal_True,::dbtools::eInDataManipulation);
        }
        else
            sTableName = ::dbtools::quoteName(aQuote,sTableName);

        aAppendOrder =  sTableName;
        aAppendOrder += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("."));
        aAppendOrder += ::dbtools::quoteName(aQuote,sRealName);
    }
    else
        aAppendOrder = getTableAlias(column) + ::dbtools::quoteName(aQuote,aName);

    OrderCreator aComposer;
    aComposer.append( getGroup() );
    aComposer.append( aAppendOrder );
    setGroup( aComposer.getComposedAndClear() );
}
// -------------------------------------------------------------------------
::rtl::OUString OSingleSelectQueryComposer::composeStatementFromParts( const ::std::vector< ::rtl::OUString >& _rParts )
{
    DBG_ASSERT( _rParts.size() == (size_t)SQLPartCount, "OSingleSelectQueryComposer::composeStatementFromParts: invalid parts array!" );

    ::rtl::OUStringBuffer aSql( m_aPureSelectSQL );
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        if ( _rParts[ eLoopParts ].getLength() )
        {
            aSql.append( getKeyword( eLoopParts ) );
            aSql.append( _rParts[ eLoopParts ] );
        }

    return aSql.makeStringAndClear();
}

// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSingleSelectQueryComposer::getElementaryQuery() throw (::com::sun::star::uno::RuntimeException)
{
    return composeStatementFromParts( m_aElementaryParts );
}

// -------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::setElementaryQuery( const ::rtl::OUString& _rElementary ) throw (::com::sun::star::sdbc::SQLException, ::com::sun::star::uno::RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    // parse and verify the statement, building a temporary iterator
    OSQLParseTreeIterator aElementaryIterator( m_xTableSupplier, m_xMetaData, NULL );
    parseAndCheck_throwError( m_aSqlParser, _rElementary, aElementaryIterator, *this );

    // remember the 4 current "additive" clauses
    ::std::vector< ::rtl::OUString > aAdditiveClauses( SQLPartCount );
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        aAdditiveClauses[ eLoopParts ] = getSQLPart( eLoopParts, m_aAdditiveIterator, sal_False );

    // strip every filter/order/groupby/having clause, and set the pure select statement
    // as query
    setQuery( _rElementary );

    // reset the the AdditiveIterator: m_aPureSelectSQL may have changed, and even if not,
    // setQuery touched the m_aAdditiveIterator
    try
    {
        parseAndCheck_throwError( m_aSqlParser, composeStatementFromParts( aAdditiveClauses ), m_aAdditiveIterator, *this );
    }
    catch( const Exception& e )
    {
        (void)e;
        DBG_ERROR( "OSingleSelectQueryComposer::setElementaryQuery: there should be no error anymore for the additive statement!" );
        // every part of the additive statement should have passed other tests already, and should not
        // be able to cause any errors ... me thinks
    }

    // get the four parts of the statement
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        m_aElementaryParts[ eLoopParts ] = getSQLPart( eLoopParts, aElementaryIterator, sal_False );

    // temporary iterator not needed anymore
    resetIterator( aElementaryIterator, true );
}

// -------------------------------------------------------------------------
namespace
{
    ::rtl::OUString getComposedClause( const ::rtl::OUString _rElementaryClause, const ::rtl::OUString _rAdditionalClause,
        TokenComposer& _rComposer, const ::rtl::OUString _rKeyword )
    {
        _rComposer.clear();
        _rComposer.append( _rElementaryClause );
        _rComposer.append( _rAdditionalClause );
        ::rtl::OUString sComposed = _rComposer.getComposedAndClear();
        if ( sComposed.getLength() )
            sComposed = _rKeyword + sComposed;
        return sComposed;
    }
}

// -------------------------------------------------------------------------
void OSingleSelectQueryComposer::setSingleAdditiveClause( SQLPart _ePart, const ::rtl::OUString& _rClause )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    // collect the 4 single parts as they're currently set
    ::std::vector< ::rtl::OUString > aClauses;
    aClauses.reserve( (size_t)SQLPartCount );
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        aClauses.push_back( getSQLPart( eLoopParts, m_aSqlIterator, sal_True ) );

    // overwrite the one part in question here
    ::std::auto_ptr< TokenComposer > pComposer;
    if ( ( _ePart == Where ) || ( _ePart == Having ) )
        pComposer.reset( new FilterCreator );
    else
        pComposer.reset( new OrderCreator );
    aClauses[ _ePart ] = getComposedClause( m_aElementaryParts[ _ePart ], _rClause,
        *pComposer, getKeyword( _ePart ) );

    // construct the complete statement
    ::rtl::OUString aSql(m_aPureSelectSQL);
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        aSql += aClauses[ eLoopParts ];

    // set the query
    setQuery_Impl(aSql);
    // parameters may also have changed
    clearParametersCollection();

    // also, since the "additive filter" change, we need to rebuild our "additive" statement
    aSql = m_aPureSelectSQL;
    // again, first get all the old additive parts
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        aClauses[ eLoopParts ] = getSQLPart( eLoopParts, m_aAdditiveIterator, sal_True );
    // then overwrite the one in question
    aClauses[ _ePart ] = getComposedClause( ::rtl::OUString(), _rClause, *pComposer, getKeyword( _ePart ) );
    // and parse it, so that m_aAdditiveIterator is up to date
    for ( SQLPart eLoopParts = Where; eLoopParts != SQLPartCount; incSQLPart( eLoopParts ) )
        aSql += aClauses[ eLoopParts ];
    try
    {
        parseAndCheck_throwError( m_aSqlParser, aSql, m_aAdditiveIterator, *this );
    }
    catch( const Exception& e )
    {
        (void)e;
        DBG_ERROR( "OSingleSelectQueryComposer::setSingleAdditiveClause: there should be no error anymore for the additive statement!" );
        // every part of the additive statement should have passed other tests already, and should not
        // be able to cause any errors ... me thinks
    }
}

// -------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::setFilter( const ::rtl::OUString& filter ) throw(SQLException, RuntimeException)
{
    setSingleAdditiveClause( Where, filter );
}

// -------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::setOrder( const ::rtl::OUString& order ) throw(SQLException, RuntimeException)
{
    setSingleAdditiveClause( Order, order );
}
// -----------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::setGroup( const ::rtl::OUString& group ) throw (SQLException, RuntimeException)
{
    setSingleAdditiveClause( Group, group );
}
// -------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::setHavingClause( const ::rtl::OUString& filter ) throw(SQLException, RuntimeException)
{
    setSingleAdditiveClause( Having, filter );
}

// -------------------------------------------------------------------------
// XTablesSupplier
Reference< XNameAccess > SAL_CALL OSingleSelectQueryComposer::getTables(  ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_pTables )
    {
        const OSQLTables& aTables = m_aSqlIterator.getTables();
        ::std::vector< ::rtl::OUString> aNames;
        for(OSQLTables::const_iterator aIter = aTables.begin(); aIter != aTables.end();++aIter)
            aNames.push_back(aIter->first);

        m_pTables = new OPrivateTables(aTables,m_xMetaData->storesMixedCaseQuotedIdentifiers(),*this,m_aMutex,aNames);
    }

    return m_pTables;
}
// -------------------------------------------------------------------------
// XColumnsSupplier
Reference< XNameAccess > SAL_CALL OSingleSelectQueryComposer::getColumns(  ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( !m_aCurrentColumns[SelectColumns] )
    {
        ::std::vector< ::rtl::OUString> aNames;
        ::vos::ORef< OSQLColumns> aCols;
        sal_Bool bCase = sal_True;
        try
        {
            bCase = m_xMetaData->storesMixedCaseQuotedIdentifiers();
            aCols = m_aSqlIterator.getSelectColumns();
            // now set the columns we have to look if the order of the columns is correct
            Reference<XStatement> xStmt = m_xConnection->createStatement();
            if ( xStmt.is() )
            {
                ::rtl::OUString sSql = m_aPureSelectSQL;
                sSql += STR_WHERE;
                sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" 0 = 1 "));
                ::rtl::OUString sGroupBy = getSQLPart(Group,m_aSqlIterator,sal_True);
                if ( sGroupBy.getLength() )
                    sSql += sGroupBy;

                Reference<XResultSetMetaDataSupplier> xResMetaDataSup;
                xResMetaDataSup = Reference<XResultSetMetaDataSupplier>(xStmt->executeQuery(sSql),UNO_QUERY);
                Reference<XResultSetMetaData> xMeta = xResMetaDataSup->getMetaData();

                sal_Int32 nCount = xMeta.is() ? xMeta->getColumnCount() : sal_Int32(0);
                ::comphelper::UStringMixEqual stringComp(m_xMetaData->storesMixedCaseQuotedIdentifiers());
                ::comphelper::TStringMixEqualFunctor stringFunc(m_xMetaData->storesMixedCaseQuotedIdentifiers());
                ::std::map<OSQLColumns::const_iterator,int> aColumnMap;

                for(sal_Int32 i=1;i<=nCount;++i)
                {
                    ::rtl::OUString sName = xMeta->getColumnName(i);
                    sal_Bool bFound = sal_False;
                    OSQLColumns::const_iterator aFind = ::connectivity::find(aCols->begin(),aCols->end(),sName,stringComp);
                    if(aFind != aCols->end())
                        //aNames.end() == ::std::find_if(aNames.begin(),aNames.end(),::std::bind2nd(stringFunc,sName)))
                    {
                        if(aColumnMap.find(aFind) != aColumnMap.end())
                        {   // we found a column name which exists twice
                            // so we start after the first found
                            do
                            {
                                aFind = ::connectivity::findRealName(++aFind,aCols->end(),sName,stringComp);
                            }
                            while(aColumnMap.find(aFind) != aColumnMap.end() && aFind != aCols->end());
                        }
                        if(aFind != aCols->end())
                        {
                            (*aFind)->getPropertyValue(PROPERTY_NAME) >>= sName;
                            aColumnMap.insert(::std::map<OSQLColumns::const_iterator,int>::value_type(aFind,0));
                            aNames.push_back(sName);
                            bFound = sal_True;
                        }
                    }
                    if ( !bFound )
                    { // we can now only look if we found it under the realname propertery
                        OSQLColumns::const_iterator aRealFind = ::connectivity::findRealName(aCols->begin(),aCols->end(),sName,stringComp);

                        if ( i <= static_cast< sal_Int32>(aCols->size()) )
                        {
                            if(aRealFind == aCols->end())
                            { // here we have to make the assumption that the position is correct
                                OSQLColumns::iterator aFind2 = aCols->begin() + i-1;
                                Reference<XPropertySet> xProp(*aFind2,UNO_QUERY);
                                if(xProp.is() && xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME))
                                {
                                    ::connectivity::parse::OParseColumn* pColumn = new ::connectivity::parse::OParseColumn(xProp,m_xMetaData->storesMixedCaseQuotedIdentifiers());
                                    pColumn->setFunction(::comphelper::getBOOL(xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Function")))));
                                    pColumn->setAggregateFunction(::comphelper::getBOOL(xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AggregateFunction")))));

                                    ::rtl::OUString sRealName;
                                    xProp->getPropertyValue(PROPERTY_REALNAME) >>= sRealName;
                                    ::std::vector< ::rtl::OUString>::iterator aFindName;
                                    if ( !sName.getLength() )
                                        xProp->getPropertyValue(PROPERTY_NAME) >>= sName;


                                    aFindName = ::std::find_if(aNames.begin(),aNames.end(),::std::bind2nd(stringFunc,sName));
                                    sal_Int32 j = 0;
                                    while ( aFindName != aNames.end() )
                                    {
                                        sName += ::rtl::OUString::valueOf(++j);
                                        aFindName = ::std::find_if(aNames.begin(),aNames.end(),::std::bind2nd(stringFunc,sName));
                                    }

                                    pColumn->setName(sName);
                                    pColumn->setRealName(sRealName);
                                    pColumn->setTableName(::comphelper::getString(xProp->getPropertyValue(PROPERTY_TABLENAME)));

                                    (*aCols)[i-1] = pColumn;
                                    aColumnMap.insert(::std::map<OSQLColumns::const_iterator,int>::value_type((*aCols).begin() + i-1,0));
                                }
                            }
                            aNames.push_back(sName);
                        }
                    }
                }
                ::comphelper::disposeComponent(xStmt);
            }
            else
            {
                for(OSQLColumns::const_iterator aIter = aCols->begin(); aIter != aCols->end();++aIter)
                    aNames.push_back(getString((*aIter)->getPropertyValue(PROPERTY_NAME)));
            }
        }
        catch(Exception&)
        {
        }
        m_aCurrentColumns[SelectColumns] = new OPrivateColumns(aCols,bCase,*this,m_aMutex,aNames);
    }
    return m_aCurrentColumns[SelectColumns];
}
// -------------------------------------------------------------------------
sal_Bool OSingleSelectQueryComposer::setORCriteria(OSQLParseNode* pCondition, OSQLParseTreeIterator& _rIterator,
                                    ::std::vector< ::std::vector < PropertyValue > >& rFilters, const Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter) const
{
    // Runde Klammern um den Ausdruck
    if (pCondition->count() == 3 &&
        SQL_ISPUNCTUATION(pCondition->getChild(0),"(") &&
        SQL_ISPUNCTUATION(pCondition->getChild(2),")"))
    {
        return setORCriteria(pCondition->getChild(1), _rIterator, rFilters, xFormatter);
    }
    // oder Verknuepfung
    // a searchcondition can only look like this: search_condition SQL_TOKEN_OR boolean_term
    else if (SQL_ISRULE(pCondition,search_condition))
    {
        sal_Bool bResult = sal_True;
        for (int i = 0; bResult && i < 3; i+=2)
        {
            // Ist das erste Element wieder eine OR-Verknuepfung?
            // Dann rekursiv absteigen ...
            //if (!i && SQL_ISRULE(pCondition->getChild(i),search_condition))
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

//--------------------------------------------------------------------------------------------------
sal_Bool OSingleSelectQueryComposer::setANDCriteria( OSQLParseNode * pCondition,
    OSQLParseTreeIterator& _rIterator, ::std::vector < PropertyValue >& rFilter, const Reference< XNumberFormatter > & xFormatter) const
{
    // Runde Klammern
    if (SQL_ISRULE(pCondition,boolean_primary))
    {
        // this should not occur
        DBG_ERROR("Primary condition in And-Criteria");
        return sal_False;
    }
    // Das erste Element ist (wieder) eine AND-Verknuepfung
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


            //  pCondition->parseNodeToStr(aValue,m_xMetaData, xFormatter, m_aLocale,static_cast<sal_Char>(m_sDecimalSep.toChar()));
            pCondition->parseNodeToStr(aValue,m_xMetaData,NULL);
            //  pCondition->getChild(0)->parseNodeToStr(aColumnName,m_xMetaData, xFormatter, m_aLocale,static_cast<sal_Char>(m_sDecimalSep.toChar()));
            pCondition->getChild(0)->parseNodeToStr(aColumnName,m_xMetaData, NULL);

            // don't display the column name
            aValue = aValue.copy(aColumnName.getLength());
            aValue = aValue.trim();

            aItem.Name = getColumnName(pCondition->getChild(0),_rIterator);
            aItem.Value <<= aValue;
            aItem.Handle = 0; // just to know that this is not one the known ones
            if (SQL_ISRULE(pCondition,like_predicate))
            {
                if (pCondition->count() == 5)
                    aItem.Handle = SQLFilterOperator::NOT_LIKE;
                else
                    aItem.Handle = SQLFilterOperator::LIKE;
            }
            else if (SQL_ISRULE(pCondition,test_for_null))
            {
                if (SQL_ISTOKEN(pCondition->getChild(2),NOT) )
                    aItem.Handle = SQLFilterOperator::NOT_SQLNULL;
                else
                    aItem.Handle = SQLFilterOperator::SQLNULL;
            }
            else if (SQL_ISRULE(pCondition,in_predicate))
            {
            }
            else if (SQL_ISRULE(pCondition,all_or_any_predicate))
            {
            }
            else if (SQL_ISRULE(pCondition,between_predicate))
            {
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
// -----------------------------------------------------------------------------
sal_Int32 OSingleSelectQueryComposer::getPredicateType(OSQLParseNode * _pPredicate) const
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
            OSL_ENSURE(0,"Wrong NodeType!");
    }
    return nPredicate;
}
//------------------------------------------------------------------------------
sal_Bool OSingleSelectQueryComposer::setComparsionPredicate(OSQLParseNode * pCondition, OSQLParseTreeIterator& _rIterator,
                                            ::std::vector < PropertyValue >& rFilter, const Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter) const
{
    DBG_ASSERT(SQL_ISRULE(pCondition, comparison_predicate),"setComparsionPredicate: pCondition ist kein ComparsionPredicate");
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
                pCondition->getChild(i)->parseNodeToPredicateStr(aValue,m_xMetaData, xFormatter, m_aLocale,static_cast<sal_Char>(m_sDecimalSep.toChar()));
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
                pCondition->getChild(i)->parseNodeToPredicateStr(aValue,m_xMetaData, xFormatter, m_aLocale,static_cast<sal_Char>(m_sDecimalSep.toChar()));
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

        pCondition->parseNodeToPredicateStr(aValue,m_xMetaData, xFormatter, m_aLocale,static_cast<sal_Char>(m_sDecimalSep.toChar()));
        pCondition->getChild(0)->parseNodeToPredicateStr(aColumnName,m_xMetaData, xFormatter, m_aLocale,static_cast<sal_Char>(m_sDecimalSep.toChar()));

        // don't display the column name
        aValue = aValue.copy(aColumnName.getLength());
        aValue.trim();

        aItem.Name = UniString(getColumnName(pCondition->getChild(0),_rIterator));
        aItem.Value <<= aValue;
        aItem.Handle = pCondition->getNodeType();
        rFilter.push_back(aItem);
    }
    else // kann sich nur um einen Expr. Ausdruck handeln
    {
        PropertyValue aItem;
        ::rtl::OUString aName, aValue;

        OSQLParseNode *pLhs = pCondition->getChild(0);
        OSQLParseNode *pRhs = pCondition->getChild(2);

        // Feldnamen
        sal_uInt16 i;
        for (i=0;i< pLhs->count();i++)
             pCondition->getChild(i)->parseNodeToPredicateStr(aName,m_xMetaData, xFormatter, m_aLocale,static_cast<sal_Char>(m_sDecimalSep.toChar()));

        // Kriterium
        aItem.Handle = pCondition->getChild(1)->getNodeType();
        aValue       = pCondition->getChild(1)->getTokenValue();
        for(i=0;i< pRhs->count();i++)
            pCondition->getChild(i)->parseNodeToPredicateStr(aValue,m_xMetaData, xFormatter, m_aLocale,static_cast<sal_Char>(m_sDecimalSep.toChar()));

        aItem.Name = aName;
        aItem.Value <<= aValue;
        rFilter.push_back(aItem);
    }
    return sal_True;
}
// functions for analysing SQL
//--------------------------------------------------------------------------------------------------
::rtl::OUString OSingleSelectQueryComposer::getColumnName( ::connectivity::OSQLParseNode* pColumnRef, OSQLParseTreeIterator& _rIterator ) const
{
    ::rtl::OUString aTableRange, aColumnName;
    _rIterator.getColumnRange(pColumnRef,aColumnName,aTableRange);
    return aColumnName;
}
//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSingleSelectQueryComposer::getFilter(  ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    return getSQLPart(Where,m_aAdditiveIterator,sal_False);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSingleSelectQueryComposer::getOrder(  ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    return getSQLPart(Order,m_aAdditiveIterator,sal_False);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSingleSelectQueryComposer::getGroup(  ) throw (RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    return getSQLPart(Group,m_aAdditiveIterator,sal_False);
}
// -----------------------------------------------------------------------------
::rtl::OUString OSingleSelectQueryComposer::getHavingClause() throw (RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    return getSQLPart(Having,m_aAdditiveIterator,sal_False);
}
// -----------------------------------------------------------------------------
::rtl::OUString OSingleSelectQueryComposer::getTableAlias(const Reference< XPropertySet >& column) const
{
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

        if(!aTable.getLength())
        { // we don't found a table name, now we must search every table for this column
            for(;pBegin != pEnd;++pBegin)
            {
                Reference<XColumnsSupplier> xColumnsSupp;
                m_pTables->getByName(*pBegin) >>= xColumnsSupp;

                if(xColumnsSupp.is() && xColumnsSupp->getColumns()->hasByName(aColumnName))
                {
//                  Reference<XPropertySet> xTableProp(xColumnsSupp,UNO_QUERY);
//                  xTableProp->getPropertyValue(PROPERTY_CATALOGNAME)  >>= aCatalog;
//                  xTableProp->getPropertyValue(PROPERTY_SCHEMANAME)   >>= aSchema;
//                  xTableProp->getPropertyValue(PROPERTY_NAME)         >>= aTable;
                    aTable = *pBegin;
                    break;
                }
            }
        }
        else
        {
            ::dbtools::composeTableName(m_xMetaData,aCatalog,aSchema,aTable,aComposedName,sal_False,::dbtools::eInDataManipulation);

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
            ::dbtools::composeTableName(m_xMetaData,aCatalog,aSchema,aTable,sReturn,sal_True,::dbtools::eInDataManipulation);
            sReturn += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("."));
        }
    }
    return sReturn;
}
// -----------------------------------------------------------------------------
Reference< XIndexAccess > SAL_CALL OSingleSelectQueryComposer::getParameters(  ) throw(RuntimeException)
{
    // now set the Parameters
    if ( !m_aCurrentColumns[ParameterColumns] )
    {
        ::vos::ORef< OSQLColumns> aCols = m_aSqlIterator.getParameters();
        ::std::vector< ::rtl::OUString> aNames;
        for(OSQLColumns::const_iterator aIter = aCols->begin(); aIter != aCols->end();++aIter)
            aNames.push_back(getString((*aIter)->getPropertyValue(PROPERTY_NAME)));
        m_aCurrentColumns[ParameterColumns] = new OPrivateColumns(aCols,m_xMetaData->storesMixedCaseQuotedIdentifiers(),*this,m_aMutex,aNames,sal_True);
    }

    return m_aCurrentColumns[ParameterColumns];
}
// -----------------------------------------------------------------------------
void OSingleSelectQueryComposer::clearParametersCollection()
{
    if ( m_aCurrentColumns[ParameterColumns] )
    {
        m_aCurrentColumns[ParameterColumns]->disposing();
        m_aColumnsCollection.push_back(m_aCurrentColumns[ParameterColumns]);
        m_aCurrentColumns[ParameterColumns] = NULL;
    }
}
// -----------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------
Reference< XIndexAccess > OSingleSelectQueryComposer::setCurrentColumns( EColumnType _eType,
    const ::vos::ORef< OSQLColumns >& _rCols )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    // now set the group columns
    if ( !m_aCurrentColumns[_eType] )
    {
        ::std::vector< ::rtl::OUString> aNames;
        for(OSQLColumns::const_iterator aIter = _rCols->begin(); aIter != _rCols->end();++aIter)
            aNames.push_back(getString((*aIter)->getPropertyValue(PROPERTY_NAME)));
        m_aCurrentColumns[_eType] = new OPrivateColumns(_rCols,m_xMetaData->storesMixedCaseQuotedIdentifiers(),*this,m_aMutex,aNames,sal_True);
    }

    return m_aCurrentColumns[_eType];
}
// -----------------------------------------------------------------------------
Reference< XIndexAccess > SAL_CALL OSingleSelectQueryComposer::getGroupColumns(  ) throw(RuntimeException)
{
    return setCurrentColumns( GroupByColumns, m_aAdditiveIterator.getGroupColumns() );
}
// -------------------------------------------------------------------------
Reference< XIndexAccess > SAL_CALL OSingleSelectQueryComposer::getOrderColumns(  ) throw(RuntimeException)
{
    return setCurrentColumns( OrderColumns, m_aAdditiveIterator.getOrderColumns() );
}
// -----------------------------------------------------------------------------
::rtl::OUString OSingleSelectQueryComposer::getStatementPart( TGetParseNode& _aGetFunctor, OSQLParseTreeIterator& _rIterator )
{
    ::rtl::OUString sResult;

    const OSQLParseNode* pNode = _aGetFunctor( &_rIterator );
    if ( pNode )
        pNode->parseNodeToStr(sResult,m_xMetaData);

    return sResult;
}
// -----------------------------------------------------------------------------
namespace
{
    ::rtl::OUString lcl_getCondition(const Sequence< Sequence< PropertyValue > >& filter )
    {
        ::rtl::OUString sRet;
        const Sequence< PropertyValue >* pOrIter = filter.getConstArray();
        const Sequence< PropertyValue >* pOrEnd = pOrIter + filter.getLength();
        while ( pOrIter != pOrEnd )
        {
            if ( pOrIter->getLength() )
            {
                sRet += L_BRACKET;
                const PropertyValue* pAndIter = pOrIter->getConstArray();
                const PropertyValue* pAndEnd = pAndIter + pOrIter->getLength();
                while ( pAndIter != pAndEnd )
                {
                    sRet += pAndIter->Name;
                    ::rtl::OUString sValue;
                    pAndIter->Value >>= sValue;
                    switch( pAndIter->Handle )
                    {
                        case SQLFilterOperator::EQUAL:
                            sRet += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" = "));
                            sRet += sValue;
                            break;
                        case SQLFilterOperator::NOT_EQUAL:
                            sRet += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" <> "));
                            sRet += sValue;
                            break;
                        case SQLFilterOperator::LESS:
                            sRet += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" < "));
                            sRet += sValue;
                            break;
                        case SQLFilterOperator::GREATER:
                            sRet += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" > "));
                            sRet += sValue;
                            break;
                        case SQLFilterOperator::LESS_EQUAL:
                            sRet += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" <= "));
                            sRet += sValue;
                            break;
                        case SQLFilterOperator::GREATER_EQUAL:
                            sRet += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" >= "));
                            sRet += sValue;
                            break;
                        case SQLFilterOperator::LIKE:
                            sRet += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" LIKE "));
                            sRet += sValue;
                            break;
                        case SQLFilterOperator::NOT_LIKE:
                            sRet += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" NOT LIKE "));
                            sRet += sValue;
                            break;
                        case SQLFilterOperator::SQLNULL:
                            sRet += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" IS NULL")) ;
                            break;
                        case SQLFilterOperator::NOT_SQLNULL:
                            sRet += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" IS NOT NULL")) ;
                            break;
                        default:
                            throw IllegalArgumentException();
                    }
                    ++pAndIter;
                    if ( pAndIter != pAndEnd )
                        sRet += STR_AND;
                }
                sRet += R_BRACKET;
            }
            ++pOrIter;
            if ( pOrIter != pOrEnd && sRet.getLength() )
                sRet += STR_OR;
        }
        return sRet;
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::setStructuredFilter( const Sequence< Sequence< PropertyValue > >& filter ) throw (SQLException, ::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    setFilter(lcl_getCondition(filter));
}
// -----------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::setStructuredHavingClause( const Sequence< Sequence< PropertyValue > >& filter ) throw (SQLException, RuntimeException)
{
    setHavingClause(lcl_getCondition(filter));
}
// -----------------------------------------------------------------------------
void OSingleSelectQueryComposer::setConditionByColumn( const Reference< XPropertySet >& column, sal_Bool andCriteria ,::std::mem_fun1_t<bool,OSingleSelectQueryComposer,::rtl::OUString>& _aSetFunctor)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    if ( !column.is()
        || !column->getPropertySetInfo()->hasPropertyByName(PROPERTY_VALUE)
        || !column->getPropertySetInfo()->hasPropertyByName(PROPERTY_NAME) )
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

    ::rtl::OUString aSql;
    ::rtl::OUString aQuote  = m_xMetaData->getIdentifierQuoteString();

    if ( m_aCurrentColumns[SelectColumns] && m_aCurrentColumns[SelectColumns]->hasByName(aName) )
    {
        Reference<XPropertySet> xColumn;
        m_aCurrentColumns[SelectColumns]->getByName(aName) >>= xColumn;
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME),"Property REALNAME not available!");
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_TABLENAME),"Property TABLENAME not available!");
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AggregateFunction"))),"Property AggregateFunctionnot available!");

        ::rtl::OUString sRealName,sTableName;
        xColumn->getPropertyValue(PROPERTY_REALNAME)    >>= sRealName;
        xColumn->getPropertyValue(PROPERTY_TABLENAME)   >>= sTableName;
        if(sTableName.indexOf('.',0) != -1)
        {
            ::rtl::OUString aCatlog,aSchema,aTable;
            ::dbtools::qualifiedNameComponents(m_xMetaData,sTableName,aCatlog,aSchema,aTable,::dbtools::eInDataManipulation);
            ::dbtools::composeTableName(m_xMetaData,aCatlog,aSchema,aTable,sTableName,sal_True,::dbtools::eInDataManipulation);
        }
        else
            sTableName = ::dbtools::quoteName(aQuote,sTableName);

        if ( !::comphelper::getBOOL(xColumn->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Function")))) )
        {
            aSql =  sTableName;
            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("."));
            aSql += ::dbtools::quoteName(aQuote,sRealName);
        }
        else
            aSql += sRealName;

    }
    else
        aSql = getTableAlias(column) + ::dbtools::quoteName(aQuote,aName);

    if ( aValue.hasValue() )
    {
        if(!m_xTypeConverter.is())
            m_xTypeConverter = Reference< XTypeConverter >(m_xServiceFactory->createInstance(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.Converter"))),UNO_QUERY);
        OSL_ENSURE(m_xTypeConverter.is(),"NO typeconverter!");

        switch(nType)
        {
            case DataType::VARCHAR:
            case DataType::CHAR:
            case DataType::LONGVARCHAR:
                aSql += STR_LIKE;
                aSql += DBTypeConversion::toSQLString(nType,aValue,sal_True,m_xTypeConverter);
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
                            aSql += STR_LIKE;
                            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\'"));
                        }
                        else
                            aSql += STR_EQUAL;
                        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0x"));
                        const sal_Int8* pBegin  = aSeq.getConstArray();
                        const sal_Int8* pEnd    = pBegin + aSeq.getLength();
                        for(;pBegin != pEnd;++pBegin)
                        {
                            aSql += ::rtl::OUString::valueOf((sal_Int32)*pBegin,16).getStr();
                        }
                        if(nSearchable == ColumnSearch::NONE)
                            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\'"));
                    }
                    else
                        throw SQLException(DBACORE_RESSTRING(RID_STR_NOT_SEQUENCE_INT8),*this,SQLSTATE_GENERAL,1000,Any() );
                }
                break;
            case DataType::BIT:
            case DataType::BOOLEAN:
                {
                    ::rtl::OUString sTmpName = aSql;


                    sal_Bool bValue = sal_False;
                    m_xTypeConverter->convertToSimpleType(aValue, TypeClass_BOOLEAN) >>= bValue;
                    switch ( m_nBoolCompareMode )
                    {
                        case BOOL_COMPARISON_SQL:
                            aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" IS "));
                            if ( bValue )
                                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" TRUE "));
                            else
                                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" FALSE "));
                            break;
                        case BOOL_COMPARISON_MISC:
                            aSql += STR_EQUAL;
                            if ( bValue )
                                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" TRUE "));
                            else
                                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" FALSE "));
                            break;
                        case BOOL_COMPARISON_ACCESS:
                            if ( bValue )
                            {
                                aSql = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" NOT ( "));
                                aSql += sTmpName;
                                aSql += STR_EQUAL;
                                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"));
                                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" OR "));
                                aSql += sTmpName;
                                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" IS NULL )")) ;
                            }
                            else
                            {
                                aSql += STR_EQUAL;
                                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"));
                            }
                            break;
                        case BOOL_COMPARISON_DEFAULT: // fall through
                        default:
                            aSql += STR_EQUAL;
                            if ( bValue )
                                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("1"));
                            else
                                aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("0"));


                    }
                    break;
                }
            default:
                aSql += STR_EQUAL;
                aSql += DBTypeConversion::toSQLString(nType,aValue,sal_True,m_xTypeConverter);
        }
    }
    else
        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" IS NULL")) ;

    // filter anhaengen
    // select ohne where und order by aufbauen
    ::rtl::OUString sFilter = getFilter();

    if ( (sFilter.getLength() != 0) && (aSql.getLength() != 0) )
    {
        ::rtl::OUString sTemp(L_BRACKET);
        sTemp += sFilter;
        sTemp += R_BRACKET;
        sTemp += andCriteria ? STR_AND : STR_OR;
        sFilter = sTemp;
    }
    sFilter += aSql;

    // add the filter and the sort order
    _aSetFunctor(this,sFilter);
}
// -----------------------------------------------------------------------------
Sequence< Sequence< PropertyValue > > OSingleSelectQueryComposer::getStructuredCondition( TGetParseNode& _aGetFunctor )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    MutexGuard aGuard(m_aMutex);

    Sequence< Sequence< PropertyValue > > aFilterSeq;
    ::rtl::OUString sFilter = getStatementPart( _aGetFunctor, m_aAdditiveIterator );

    if ( sFilter.getLength() != 0 )
    {
        ::rtl::OUString aSql(m_aPureSelectSQL);
        // build a temporary parse node
        const OSQLParseNode* pTempNode = m_aAdditiveIterator.getParseTree();

        aSql += STR_WHERE;
        aSql += sFilter;

        ::rtl::OUString aErrorMsg;
        ::std::auto_ptr<OSQLParseNode> pSqlParseNode( m_aSqlParser.parseTree(aErrorMsg,aSql));
        if ( pSqlParseNode.get() )
        {
            m_aAdditiveIterator.setParseTree(pSqlParseNode.get());
            // normalize the filter
            OSQLParseNode* pWhereNode = const_cast<OSQLParseNode*>(m_aAdditiveIterator.getWhereTree());

            OSQLParseNode* pCondition = pWhereNode->getChild(1);
            OSQLParseNode::negateSearchCondition(pCondition);

            pCondition = pWhereNode->getChild(1);
            OSQLParseNode::disjunctiveNormalForm(pCondition);

            pCondition = pWhereNode->getChild(1);
            OSQLParseNode::absorptions(pCondition);

            pCondition = pWhereNode->getChild(1);
            if ( pCondition )
            {
                ::std::vector< ::std::vector < PropertyValue > > aFilters;
                Reference< ::com::sun::star::util::XNumberFormatter >  xFormatter(m_xServiceFactory
                                ->createInstance(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.util.NumberFormatter"))), UNO_QUERY);
                xFormatter->attachNumberFormatsSupplier(m_xNumberFormatsSupplier);

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
// -----------------------------------------------------------------------------
::rtl::OUString OSingleSelectQueryComposer::getKeyword( SQLPart _ePart ) const
{
    ::rtl::OUString sKeyword;
    switch(_ePart)
    {
        default:
            OSL_ENSURE( 0, "OSingleSelectQueryComposer::getKeyWord: Invalid enum value!" );
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

// -----------------------------------------------------------------------------
::rtl::OUString OSingleSelectQueryComposer::getSQLPart( SQLPart _ePart, OSQLParseTreeIterator& _rIterator, sal_Bool _bWithKeyword )
{
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
            OSL_ENSURE(0,"Invalid enum value!");
    }

    ::rtl::OUString sRet = getStatementPart( F_tmp, _rIterator );
    if ( _bWithKeyword && sRet.getLength() )
        sRet = sKeyword + sRet;
    return sRet;
}
// -----------------------------------------------------------------------------
