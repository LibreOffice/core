/*************************************************************************
 *
 *  $RCSfile: SingleSelectQueryComposer.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:01:24 $
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
#ifndef _ISOLANG_HXX
#include <tools/isolang.hxx>
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
#include <memory>
#include <functional>

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

DBG_NAME(OSingleSelectQueryComposer)
// -------------------------------------------------------------------------
OSingleSelectQueryComposer::OSingleSelectQueryComposer(const Reference< XNameAccess>& _xTableSupplier,
                               const Reference< XConnection>& _xConnection,
                               const Reference< XMultiServiceFactory >& _xServiceFactory)
 : OSubComponent(m_aMutex,_xConnection)
 ,OPropertyContainer(m_aBHelper)
 , m_xConnection(_xConnection)
 , m_xMetaData(_xConnection->getMetaData())
 , m_aSqlIterator(_xTableSupplier,_xConnection->getMetaData(),NULL)
 , m_xTableSupplier(_xTableSupplier)
 , m_aSqlParser(_xServiceFactory)
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

    const OSQLParseNode* pSqlParseNode = m_aSqlIterator.getParseTree();
    m_aSqlIterator.setParseTree(NULL);
    delete pSqlParseNode;
    m_aSqlIterator.dispose();

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
    ::std::const_mem_fun_t<const OSQLParseNode*,OSQLParseTreeIterator> F_tmp(&OSQLParseTreeIterator::getParseTree);

    return getStatementPart(F_tmp);
}
// -------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::setQuery( const ::rtl::OUString& command ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    // first clear the tables and columns
    clearCurrentCollections();
    // now set the new one
    m_sOrignal = command;
    setQuery_Impl(m_sOrignal);
}
// -----------------------------------------------------------------------------
void OSingleSelectQueryComposer::setQuery_Impl( const ::rtl::OUString& command )
{
    const OSQLParseNode* pSqlParseNode = m_aSqlIterator.getParseTree();
    m_aSqlIterator.setParseTree(NULL);
    delete pSqlParseNode;

    ::rtl::OUString aErrorMsg;
    pSqlParseNode = m_aSqlParser.parseTree(aErrorMsg,command);
    if ( !pSqlParseNode )
    {
        SQLException aError2(aErrorMsg,*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY000")),1000,Any());
        SQLException aError1(command,*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY000")),1000,makeAny(aError2));
        throw SQLException(m_aSqlParser.getContext().getErrorMessage(OParseContext::ERROR_GENERAL),*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY000")),1000,makeAny(aError1));
    }

    m_aSqlIterator.setParseTree(pSqlParseNode);
    m_aSqlIterator.traverseAll();
    if ((   m_aSqlIterator.getStatementType() != SQL_STATEMENT_SELECT
        &&  m_aSqlIterator.getStatementType() != SQL_STATEMENT_SELECT_COUNT)
        ||  SQL_ISRULE(pSqlParseNode,union_statement) ) // #i4229# OJ
    {
        SQLException aError1(command,*this,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY000")),1000,Any());
        throw SQLException(DBACORE_RESSTRING(RID_STR_ONLY_QUERY),*this,
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("HY000")),1000,makeAny(aError1));
    }

    m_aWorkSql = STR_SELECT;
    pSqlParseNode->getChild(1)->parseNodeToStr(m_aWorkSql,m_xMetaData);
    pSqlParseNode->getChild(2)->parseNodeToStr(m_aWorkSql,m_xMetaData);
    m_aWorkSql += STR_FROM;
    pSqlParseNode->getChild(3)->getChild(0)->getChild(1)->parseNodeToStr(m_aWorkSql,m_xMetaData);

    getColumns();
    getTables();
}
// -----------------------------------------------------------------------------
Sequence< Sequence< PropertyValue > > SAL_CALL OSingleSelectQueryComposer::getStructuredHavingFilter(  ) throw (RuntimeException)
{
    ::std::const_mem_fun_t<const OSQLParseNode*,OSQLParseTreeIterator> F_tmp(&OSQLParseTreeIterator::getSimpleHavingTree);
    return getStructuredCondition(F_tmp);
}
// -------------------------------------------------------------------------
Sequence< Sequence< PropertyValue > > SAL_CALL OSingleSelectQueryComposer::getStructuredFilter(  ) throw(RuntimeException)
{
    ::std::const_mem_fun_t<const OSQLParseNode*,OSQLParseTreeIterator> F_tmp(&OSQLParseTreeIterator::getSimpleWhereTree);
    return getStructuredCondition(F_tmp);
}
// -----------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::appendHavingFilterByColumn( const Reference< XPropertySet >& column, sal_Bool andCriteria ) throw (SQLException, RuntimeException)
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
    ::rtl::OUString aSql(m_aWorkSql);
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
    ::rtl::OUString aSql(m_aWorkSql);
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

    ::rtl::OUString sGroup = getGroup();
    if ( (sGroup.getLength() != 0) && aAppendOrder.getLength() )
        sGroup += COMMA;
    sGroup += aAppendOrder;

    setGroup(sGroup);
}
// -------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::setFilter( const ::rtl::OUString& filter ) throw(SQLException, RuntimeException)
{
    // add the filter and the sort order
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    ::rtl::OUString aSql(m_aWorkSql);

    if ( filter.getLength() )
    {
        aSql += STR_WHERE;
        aSql += filter;
    }
    aSql += getSQLPart(Group);
    aSql += getSQLPart(Having);
    aSql += getSQLPart(Order);

    setQuery_Impl(aSql);
    clearParametersCollection();  // parameters may also have changed with the filter
}
// -------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::setOrder( const ::rtl::OUString& order ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    ::rtl::OUString aSql(m_aWorkSql);

    aSql += getSQLPart(Where);
    aSql += getSQLPart(Group);
    aSql += getSQLPart(Having);
    if ( order.getLength() )
    {
        aSql += STR_ORDER_BY;
        aSql += order;
    }

    setQuery_Impl(aSql);
}
// -----------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::setGroup( const ::rtl::OUString& group ) throw (SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    ::rtl::OUString aSql(m_aWorkSql);

    aSql += getSQLPart(Where);
    if ( group.getLength() )
    {
        aSql += STR_GROUP_BY;
        aSql += group;
    }
    aSql += getSQLPart(Having);
    aSql += getSQLPart(Order);

    setQuery_Impl(aSql);
}
// -------------------------------------------------------------------------
void SAL_CALL OSingleSelectQueryComposer::setHavingClause( const ::rtl::OUString& filter ) throw(SQLException, RuntimeException)
{
    // add the filter and the sort order
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );
    ::rtl::OUString aSql(m_aWorkSql);

    aSql += getSQLPart(Where);
    aSql += getSQLPart(Group);
    if ( filter.getLength() )
    {
        aSql += STR_HAVING;
        aSql += filter;
    }
    aSql += getSQLPart(Order);

    setQuery_Impl(aSql);
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
        try
        {
            // now set the columns we have to look if the order of the columns is correct
            Reference<XStatement> xStmt = m_xConnection->createStatement();

            ::std::vector< ::rtl::OUString> aNames;
            ::vos::ORef< OSQLColumns> aCols = m_aSqlIterator.getSelectColumns();
            if ( xStmt.is() )
            {
                ::rtl::OUString sSql = m_aWorkSql;
                sSql += STR_WHERE;
                sSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" 0 = 1 "));
                ::rtl::OUString sGroupBy = getSQLPart(Group);
                if ( sGroupBy.getLength() )
                    sSql += sGroupBy;

                Reference<XResultSetMetaDataSupplier> xResMetaDataSup;
                xResMetaDataSup = Reference<XResultSetMetaDataSupplier>(xStmt->executeQuery(sSql),UNO_QUERY);
                Reference<XResultSetMetaData> xMeta = xResMetaDataSup->getMetaData();

                sal_Int32 nCount = xMeta.is() ? xMeta->getColumnCount() : sal_Int32(0);
                ::comphelper::UStringMixEqual bCase(m_xMetaData->storesMixedCaseQuotedIdentifiers());
                ::comphelper::TStringMixEqualFunctor bCase2(m_xMetaData->storesMixedCaseQuotedIdentifiers());
                ::std::map<OSQLColumns::const_iterator,int> aColumnMap;

                for(sal_Int32 i=1;i<=nCount;++i)
                {
                    ::rtl::OUString sName = xMeta->getColumnName(i);
                    OSQLColumns::const_iterator aFind = ::connectivity::find(aCols->begin(),aCols->end(),sName,bCase);
                    if(aFind != aCols->end())
                        //aNames.end() == ::std::find_if(aNames.begin(),aNames.end(),::std::bind2nd(bCase2,sName)))
                    {
                        if(aColumnMap.find(aFind) != aColumnMap.end())
                        {   // we found a column name which exists twice
                            // so we start after the first found
                            do
                            {
                                aFind = ::connectivity::findRealName(++aFind,aCols->end(),sName,bCase);
                            }
                            while(aColumnMap.find(aFind) != aColumnMap.end() && aFind != aCols->end());

                            OSL_ENSURE(aFind != aCols->end(),"Invalid column!");
                        }
                        if(aFind != aCols->end())
                        {
                            (*aFind)->getPropertyValue(PROPERTY_NAME) >>= sName;
                            aColumnMap.insert(::std::map<OSQLColumns::const_iterator,int>::value_type(aFind,0));
                            aNames.push_back(sName);
                        }
                    }
                    else
                    { // we can now only look if we found it under the realname propertery
                        OSQLColumns::const_iterator aRealFind = ::connectivity::findRealName(aCols->begin(),aCols->end(),sName,bCase);

                        if ( i <= static_cast< sal_Int32>(aCols->size()) )
                        {
                            if(aRealFind == aCols->end())
                            { // here we have to make the assumption that the position is correct
                                OSQLColumns::iterator aFind2 = aCols->begin() + i-1;
                                Reference<XPropertySet> xProp(*aFind2,UNO_QUERY);
                                if(xProp.is() && xProp->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME))
                                {
                                    ::connectivity::parse::OParseColumn* pColumn = new ::connectivity::parse::OParseColumn(xProp,m_xMetaData->storesMixedCaseQuotedIdentifiers());
                                    if(sName.getLength())
                                        pColumn->setName(sName);
                                    else
                                        xProp->getPropertyValue(PROPERTY_NAME) >>= sName;
                                    pColumn->setRealName(::comphelper::getString(xProp->getPropertyValue(PROPERTY_REALNAME)));
                                    pColumn->setTableName(::comphelper::getString(xProp->getPropertyValue(PROPERTY_TABLENAME)));
                                    (*aCols)[i-1] = pColumn;
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

            m_aCurrentColumns[SelectColumns] = new OPrivateColumns(aCols,m_xMetaData->storesMixedCaseQuotedIdentifiers(),*this,m_aMutex,aNames);
        }
        catch(Exception&)
        {
        }
    return m_aCurrentColumns[SelectColumns];
}
// -------------------------------------------------------------------------
sal_Bool OSingleSelectQueryComposer::setORCriteria(OSQLParseNode* pCondition,
                                    ::std::vector< ::std::vector < PropertyValue > >& rFilters, const Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter) const
{
    // Runde Klammern um den Ausdruck
    if (pCondition->count() == 3 &&
        SQL_ISPUNCTUATION(pCondition->getChild(0),"(") &&
        SQL_ISPUNCTUATION(pCondition->getChild(2),")"))
    {
        return setORCriteria(pCondition->getChild(1), rFilters, xFormatter);
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
                bResult = setORCriteria(pCondition->getChild(i), rFilters, xFormatter);
            else
            {
                rFilters.push_back( ::std::vector < PropertyValue >());
                bResult = setANDCriteria(pCondition->getChild(i), rFilters[rFilters.size() - 1], xFormatter);
            }
        }
        return bResult;
    }
    else
    {
        rFilters.push_back(::std::vector < PropertyValue >());
        return setANDCriteria(pCondition, rFilters[rFilters.size() - 1], xFormatter);
    }
}

//--------------------------------------------------------------------------------------------------
sal_Bool OSingleSelectQueryComposer::setANDCriteria(OSQLParseNode * pCondition,
                                  ::std::vector < PropertyValue >& rFilter, const Reference< ::com::sun::star::util::XNumberFormatter > & xFormatter) const
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
        return setANDCriteria(pCondition->getChild(0), rFilter, xFormatter) &&
               setANDCriteria(pCondition->getChild(2), rFilter, xFormatter);
    }
    else if (SQL_ISRULE(pCondition, comparison_predicate))
    {
        return setComparsionPredicate(pCondition,rFilter,xFormatter);
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

            aItem.Name = getColumnName(pCondition->getChild(0));
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
sal_Bool OSingleSelectQueryComposer::setComparsionPredicate(OSQLParseNode * pCondition,
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

            sal_uInt32 i = pCondition->count() - 2;
            switch (pCondition->getChild(i)->getNodeType())
            {
                case SQL_NODE_EQUAL:
                    // don't display the equal
                    i--;
                    aItem.Handle = SQLFilterOperator::EQUAL;
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
            }

            // go backward
            for (; i >= 0; i--)
                pCondition->getChild(i)->parseNodeToPredicateStr(aValue,m_xMetaData, xFormatter, m_aLocale,static_cast<sal_Char>(m_sDecimalSep.toChar()));
        }
        else
            return sal_False;

        aItem.Name = getColumnName(pCondition->getChild(nPos));
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

        aItem.Name = UniString(getColumnName(pCondition->getChild(0)));
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
::rtl::OUString OSingleSelectQueryComposer::getColumnName(::connectivity::OSQLParseNode* pColumnRef) const
{
    ::rtl::OUString aTableRange, aColumnName;
    m_aSqlIterator.getColumnRange(pColumnRef, aColumnName, aTableRange );
    return aColumnName;
}
//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSingleSelectQueryComposer::getFilter(  ) throw(RuntimeException)
{
    return getSQLPart(Where,sal_False);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSingleSelectQueryComposer::getOrder(  ) throw(RuntimeException)
{
    return getSQLPart(Order,sal_False);
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OSingleSelectQueryComposer::getGroup(  ) throw (RuntimeException)
{
    return getSQLPart(Group,sal_False);
}
// -----------------------------------------------------------------------------
::rtl::OUString OSingleSelectQueryComposer::getHavingClause() throw (RuntimeException)
{
    return getSQLPart(Having,sal_False);
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
                ::comphelper::UStringMixLess aTmp(m_aSqlIterator.getTables().key_comp());
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
Reference< XIndexAccess > OSingleSelectQueryComposer::setColumns( EColumnType _eType
                                ,::std::const_mem_fun_t< ::vos::ORef< OSQLColumns>,::connectivity::OSQLParseTreeIterator>& _aGetFunctor )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    // now set the group columns
    if ( !m_aCurrentColumns[_eType] )
    {
        ::vos::ORef< OSQLColumns> aCols = _aGetFunctor(&m_aSqlIterator);
        ::std::vector< ::rtl::OUString> aNames;
        for(OSQLColumns::const_iterator aIter = aCols->begin(); aIter != aCols->end();++aIter)
            aNames.push_back(getString((*aIter)->getPropertyValue(PROPERTY_NAME)));
        m_aCurrentColumns[_eType] = new OPrivateColumns(aCols,m_xMetaData->storesMixedCaseQuotedIdentifiers(),*this,m_aMutex,aNames,sal_True);
    }

    return m_aCurrentColumns[_eType];
}
// -----------------------------------------------------------------------------
Reference< XIndexAccess > SAL_CALL OSingleSelectQueryComposer::getGroupColumns(  ) throw(RuntimeException)
{
    ::std::const_mem_fun_t< ::vos::ORef< OSQLColumns>,OSQLParseTreeIterator> F_tmp(&OSQLParseTreeIterator::getGroupColumns);

    return setColumns(GroupByColumns,F_tmp);
}
// -------------------------------------------------------------------------
Reference< XIndexAccess > SAL_CALL OSingleSelectQueryComposer::getOrderColumns(  ) throw(RuntimeException)
{
    ::std::const_mem_fun_t< ::vos::ORef< OSQLColumns>,OSQLParseTreeIterator> F_tmp(&OSQLParseTreeIterator::getOrderColumns);

    return setColumns(OrderColumns,F_tmp);
}
// -----------------------------------------------------------------------------
::rtl::OUString OSingleSelectQueryComposer::getStatementPart(::std::const_mem_fun_t< const ::connectivity::OSQLParseNode*,::connectivity::OSQLParseTreeIterator>& _aGetFunctor)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    ::rtl::OUString sResult;

    const OSQLParseNode* pNode = _aGetFunctor(&m_aSqlIterator);
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
            ++pOrIter;
            if ( pOrIter != pOrEnd )
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
void SAL_CALL OSingleSelectQueryComposer::setStructuredHavingFilter( const Sequence< Sequence< PropertyValue > >& filter ) throw (SQLException, RuntimeException)
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

        aSql =  sTableName;
        aSql += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("."));
        aSql += ::dbtools::quoteName(aQuote,sRealName);
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
Sequence< Sequence< PropertyValue > > OSingleSelectQueryComposer::getStructuredCondition( ::std::const_mem_fun_t< const ::connectivity::OSQLParseNode*,::connectivity::OSQLParseTreeIterator>& _aGetFunctor )
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    MutexGuard aGuard(m_aMutex);

    Sequence< Sequence< PropertyValue > > aFilterSeq;
    ::rtl::OUString sFilter = getStatementPart(_aGetFunctor);

    if ( sFilter.getLength() != 0 )
    {
        ::rtl::OUString aSql(m_aWorkSql);
        // build a temporary parse node
        const OSQLParseNode* pTempNode = m_aSqlIterator.getParseTree();

        aSql += STR_WHERE;
        aSql += sFilter;

        ::rtl::OUString aErrorMsg;
        ::std::auto_ptr<OSQLParseNode> pSqlParseNode( m_aSqlParser.parseTree(aErrorMsg,aSql));
        if ( pSqlParseNode.get() )
        {
            m_aSqlIterator.setParseTree(pSqlParseNode.get());
            // normalize the filter
            OSQLParseNode* pWhereNode = const_cast<OSQLParseNode*>(m_aSqlIterator.getWhereTree());

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

                if (setORCriteria(pCondition, aFilters, xFormatter))
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
            m_aSqlIterator.setParseTree(pTempNode);
        }
    }
    return aFilterSeq;
}
// -----------------------------------------------------------------------------
::rtl::OUString OSingleSelectQueryComposer::getSQLPart(SQLPart _ePart,sal_Bool _bWithKeyword)
{
    ::std::const_mem_fun_t<const OSQLParseNode*,OSQLParseTreeIterator> F_tmp(&OSQLParseTreeIterator::getSimpleWhereTree);
    ::rtl::OUString sKeyword;
    switch(_ePart)
    {
        case Where:
            sKeyword = STR_WHERE;
            F_tmp = ::std::const_mem_fun_t<const OSQLParseNode*,OSQLParseTreeIterator>(&OSQLParseTreeIterator::getSimpleWhereTree);
            break;
        case Group:
            sKeyword = STR_GROUP_BY;
            F_tmp = ::std::const_mem_fun_t<const OSQLParseNode*,OSQLParseTreeIterator> (&OSQLParseTreeIterator::getSimpleGroupByTree);
            break;
        case Having:
            sKeyword = STR_HAVING;
            F_tmp = ::std::const_mem_fun_t<const OSQLParseNode*,OSQLParseTreeIterator>(&OSQLParseTreeIterator::getSimpleHavingTree);
            break;
        case Order:
            sKeyword = STR_ORDER_BY;
            F_tmp = ::std::const_mem_fun_t<const OSQLParseNode*,OSQLParseTreeIterator>(&OSQLParseTreeIterator::getSimpleOrderTree);
            break;
        default:
            OSL_ENSURE(0,"Invalid enum value!");
    }

    ::rtl::OUString sRet = getStatementPart(F_tmp);
    if ( _bWithKeyword && sRet.getLength() )
        sRet = sKeyword + sRet;
    return sRet;
}
// -----------------------------------------------------------------------------
