/*************************************************************************
 *
 *  $RCSfile: querycomposer.cxx,v $
 *
 *  $Revision: 1.25 $
 *
 *  last change: $Author: oj $ $Date: 2001-03-01 11:04:31 $
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

#ifndef _CONNECTIVITY_COMMONTOOLS_HXX_
#include "querycomposer.hxx"
#endif
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
#ifndef _COM_SUN_STAR_REGISTRY_XSIMPLEREGISTRY_HPP_
#include <com/sun/star/registry/XSimpleRegistry.hpp>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
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
#ifndef _CONNECTIVITY_SDBCX_COLLECTION_HXX_
#include "connectivity/sdbcx/VCollection.hxx"
#endif
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef COMPHELPER_INDEXACCESSWRAPPER_HXX
#include <comphelper/IndexAccessWrapper.hxx>
#endif


using namespace dbaccess;
using namespace connectivity;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::i18n;
using namespace ::com::sun::star::lang;
using namespace ::cppu;
using namespace ::osl;
using namespace ::utl;
//  using namespace ::com::sun::star::registry;

#define STR_SELECT      ::rtl::OUString::createFromAscii("SELECT ")
#define STR_FROM        ::rtl::OUString::createFromAscii(" FROM ")
#define STR_WHERE       ::rtl::OUString::createFromAscii(" WHERE ")
#define STR_ORDER_BY    ::rtl::OUString::createFromAscii(" ORDER BY ")
#define STR_AND         ::rtl::OUString::createFromAscii(" AND ")
#define STR_LIKE        ::rtl::OUString::createFromAscii(" LIKE ")
#define STR_EQUAL       ::rtl::OUString::createFromAscii(" = ")
#define L_BRACKET       ::rtl::OUString::createFromAscii("(")
#define R_BRACKET       ::rtl::OUString::createFromAscii(")")
#define COMMA           ::rtl::OUString::createFromAscii(",")

namespace dbaccess
{
    // -----------------------------------------------------------------------------
    typedef connectivity::sdbcx::OCollection OPrivateColumns_Base;
    class OPrivateColumns : public OPrivateColumns_Base
    {
        OSQLColumns m_aColumns;
    protected:
        virtual Reference< XNamed > createObject(const ::rtl::OUString& _rName);
        virtual void impl_refresh() throw(RuntimeException) {}
        virtual Reference< XPropertySet > createEmptyObject()
        {
            return NULL;
        }
    public:
        OPrivateColumns(const OSQLColumns& _rColumns,
                        sal_Bool _bCase,
                        ::cppu::OWeakObject& _rParent,
                        ::osl::Mutex& _rMutex,
                        const ::std::vector< ::rtl::OUString> &_rVector
                    ) : sdbcx::OCollection(_rParent,_bCase,_rMutex,_rVector)
                        ,m_aColumns(_rColumns)
        {
        }
        virtual void SAL_CALL disposing(void)
        {
            clear_NoDispose();
                // we're not owner of the objects we're holding, instead the object we got in our ctor is
                // So we're not allowed to dispose our elements.
            OPrivateColumns_Base::disposing();
        }
    };
    // -------------------------------------------------------------------------
    Reference< XNamed > OPrivateColumns::createObject(const ::rtl::OUString& _rName)
    {
        return Reference< XNamed >(*find(m_aColumns.begin(),m_aColumns.end(),_rName,isCaseSensitive()),UNO_QUERY);
    }
    typedef connectivity::sdbcx::OCollection OPrivateTables_BASE;

    //==========================================================================
    //= OPrivateTables
    //==========================================================================
    class OPrivateTables : public OPrivateTables_BASE
    {
        OSQLTables  m_aTables;
    protected:
        virtual Reference< XNamed > createObject(const ::rtl::OUString& _rName);
        virtual void impl_refresh() throw(RuntimeException) {}
        virtual Reference< XPropertySet > createEmptyObject()
        {
            return NULL;
        }
    public:
        OPrivateTables( const OSQLTables& _rTables,
                        sal_Bool _bCase,
                        ::cppu::OWeakObject& _rParent,
                        ::osl::Mutex& _rMutex,
                        const ::std::vector< ::rtl::OUString> &_rVector
                    ) : sdbcx::OCollection(_rParent,_bCase,_rMutex,_rVector)
                        ,m_aTables(_rTables)
        {
        }
        virtual void SAL_CALL disposing(void)
        {
            clear_NoDispose();
                // we're not owner of the objects we're holding, instead the object we got in our ctor is
                // So we're not allowed to dispose our elements.
            m_aTables.clear();
            OPrivateTables_BASE::disposing();
        }
    };
    // -------------------------------------------------------------------------
    Reference< XNamed > OPrivateTables::createObject(const ::rtl::OUString& _rName)
    {
        OSQLTables::iterator aIter = m_aTables.find(_rName);
        OSL_ENSURE(aIter != m_aTables.end(),"Table not found!");
        OSL_ENSURE(aIter->second.is(),"Table is null!");
        return Reference< XNamed >(m_aTables.find(_rName)->second,UNO_QUERY);
    }
    // -----------------------------------------------------------------------------
}

DBG_NAME(OQueryComposer)
// -------------------------------------------------------------------------
OQueryComposer::OQueryComposer(const Reference< XNameAccess>& _xTableSupplier,
                               const Reference< XConnection>& _xConnection,
                               const Reference< XMultiServiceFactory >& _xServiceFactory)
 : OSubComponent(m_aMutex,_xConnection)
 , m_xConnection(_xConnection)
 , m_pSqlParseNode(NULL)
 , m_aSqlIterator(_xTableSupplier,_xConnection->getMetaData(),NULL)
 , m_xTableSupplier(_xTableSupplier)
 , m_aSqlParser(_xServiceFactory)
 ,m_xServiceFactory(_xServiceFactory)
 ,m_pColumns(NULL)
 ,m_pTables(NULL)
 ,m_pParameters(NULL)
{
    DBG_CTOR(OQueryComposer,NULL);
    OSL_ENSHURE(_xServiceFactory.is()," ServiceFactory cant be null!");
    OSL_ENSHURE(_xConnection.is()," Connection cant be null!");
    OSL_ENSHURE(_xTableSupplier.is(),"TableSupplier cant be null!");

    Any aValue = ConfigManager::GetDirectConfigProperty(ConfigManager::LOCALE);
    m_aLocale.Language = ::comphelper::getString(aValue);
    m_xNumberFormatsSupplier = dbtools::getNumberFormats(m_xConnection,sal_True,m_xServiceFactory);
    Reference< XLocaleData> xLocaleData = Reference<XLocaleData>(m_xServiceFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.i18n.LocaleData")),UNO_QUERY);
    LocaleDataItem aData = xLocaleData->getLocaleItem(m_aLocale);
    m_sDecimalSep = aData.decimalSeparator;
    OSL_ENSHURE(m_sDecimalSep.getLength() == 1,"OQueryComposer::OQueryComposer decimal separator is not 1 length");
}
// -------------------------------------------------------------------------
OQueryComposer::~OQueryComposer()
{
    DBG_DTOR(OQueryComposer,NULL);
}
// -------------------------------------------------------------------------
// OComponentHelper
void SAL_CALL OQueryComposer::disposing(void)
{
    OSubComponent::disposing();

    MutexGuard aGuard(m_aMutex);

    delete m_pSqlParseNode;
    m_pSqlParseNode     = NULL;
    m_aSqlIterator.dispose();
    m_xTableSupplier    = NULL;
    m_xConnection       = NULL;
    m_xServiceFactory   = NULL;
    if(m_pColumns)
    {
        m_pColumns->disposing();
        delete m_pColumns;
        m_pColumns = NULL;
    }
    if(m_pParameters)
    {
        m_pParameters->disposing();
        delete m_pParameters;
        m_pParameters = NULL;
    }
    if(m_pTables)
    {
        m_pTables->disposing();
        delete m_pTables;
        m_pTables = NULL;
    }
}
// -------------------------------------------------------------------------
// ::com::sun::star::lang::XTypeProvider
Sequence< Type > SAL_CALL OQueryComposer::getTypes() throw (RuntimeException)
{
    return ::comphelper::concatSequences(OSubComponent::getTypes(),OQueryComposer_BASE::getTypes());
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL OQueryComposer::getImplementationId() throw (RuntimeException)
{
    static OImplementationId * pId = 0;
    if (! pId)
    {
        MutexGuard aGuard( Mutex::getGlobalMutex() );
        if (! pId)
        {
            static OImplementationId aId;
            pId = &aId;
        }
    }
    return pId->getImplementationId();
}
// -------------------------------------------------------------------------
// com::sun::star::lang::XUnoTunnel
sal_Int64 SAL_CALL OQueryComposer::getSomething( const Sequence< sal_Int8 >& rId ) throw(RuntimeException)
{
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return (sal_Int64)this;

    return 0;
}
// -------------------------------------------------------------------------
Any SAL_CALL OQueryComposer::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aRet = OSubComponent::queryInterface(rType);
    if(!aRet.hasValue())
        aRet = OQueryComposer_BASE::queryInterface(rType);
    return aRet;
}
// -------------------------------------------------------------------------
// XServiceInfo
//------------------------------------------------------------------------------
rtl::OUString OQueryComposer::getImplementationName(  ) throw(RuntimeException)
{
    return rtl::OUString::createFromAscii("com.sun.star.sdb.dbaccess.OQueryComposer");
}
//------------------------------------------------------------------------------
sal_Bool OQueryComposer::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OQueryComposer::getSupportedServiceNames(  ) throw (RuntimeException)
{
    Sequence< rtl::OUString > aSNS( 1 );
    aSNS[0] = SERVICE_SDB_SQLQUERYCOMPOSER;
    return aSNS;
}
// -------------------------------------------------------------------------
// XSQLQueryComposer
::rtl::OUString SAL_CALL OQueryComposer::getQuery(  ) throw(RuntimeException)
{
    if (OSubComponent::rBHelper.bDisposed)
        throw DisposedException();

    return m_aQuery;
}
// -------------------------------------------------------------------------
void SAL_CALL OQueryComposer::setQuery( const ::rtl::OUString& command ) throw(SQLException, RuntimeException)
{
    if (OSubComponent::rBHelper.bDisposed)
        throw DisposedException();

    ::osl::MutexGuard aGuard( m_aMutex );

    m_aQuery = command;
    delete m_pSqlParseNode;
    ::rtl::OUString aErrorMsg;
    m_pSqlParseNode = m_aSqlParser.parseTree(aErrorMsg,m_aQuery);
    if(!m_pSqlParseNode)
    {
        SQLException aError2(aErrorMsg,*this,::rtl::OUString::createFromAscii("HY000"),1000,Any());
        SQLException aError1(command,*this,::rtl::OUString::createFromAscii("HY000"),1000,makeAny(aError2));
        throw SQLException(m_aSqlParser.getContext().getErrorMessage(OParseContext::ERROR_GENERAL),*this,::rtl::OUString::createFromAscii("HY000"),1000,makeAny(aError1));
    }

    m_aSqlIterator.setParseTree(m_pSqlParseNode);
    m_aSqlIterator.traverseAll();
    if( m_aSqlIterator.getStatementType() != SQL_STATEMENT_SELECT && m_aSqlIterator.getStatementType() != SQL_STATEMENT_SELECT_COUNT)
    {
        SQLException aError1(command,*this,::rtl::OUString::createFromAscii("HY000"),1000,Any());
        throw SQLException(::rtl::OUString::createFromAscii("No select statement!"),*this,::rtl::OUString::createFromAscii("HY000"),1000,makeAny(aError1));
    }

    m_aWorkSql = STR_SELECT;
    m_pSqlParseNode->getChild(1)->parseNodeToStr(m_aWorkSql,m_xConnection->getMetaData());
    m_pSqlParseNode->getChild(2)->parseNodeToStr(m_aWorkSql,m_xConnection->getMetaData());
    m_aWorkSql += STR_FROM;
    m_pSqlParseNode->getChild(3)->getChild(0)->getChild(1)->parseNodeToStr(m_aWorkSql,m_xConnection->getMetaData());

    m_aFilter = m_aOrgFilter = m_aOrgOrder = m_aOrder = ::rtl::OUString();

    const OSQLParseNode* pWhereNode = m_aSqlIterator.getWhereTree();
    if(pWhereNode)
        pWhereNode->getChild(1)->parseNodeToStr(m_aOrgFilter,m_xConnection->getMetaData());

    const OSQLParseNode* pOrderNode = m_aSqlIterator.getOrderTree();
    if(pOrderNode) // parse without "ORDER BY"
        pOrderNode->getChild(2)->parseNodeToStr(m_aOrgOrder,m_xConnection->getMetaData());

    // first clear the tables and columns
    if(m_pTables)
    {
        m_pTables->disposing();
        delete m_pTables;
        m_pTables = NULL;
    }
    if(m_pColumns)
    {
        m_pColumns->disposing();
        delete m_pColumns;
        m_pColumns = NULL;
    }
    if(m_pParameters)
    {
        m_pParameters->disposing();
        delete m_pParameters;
        m_pParameters = NULL;
    }
    // now set the columns
    const ::vos::ORef< OSQLColumns>& aCols = m_aSqlIterator.getSelectColumns();
    ::std::vector< ::rtl::OUString> aNames;
    for(OSQLColumns::const_iterator aIter = aCols->begin(); aIter != aCols->end();++aIter)
        aNames.push_back(getString((*aIter)->getPropertyValue(PROPERTY_NAME)));
    m_pColumns = new OPrivateColumns(*aCols,m_xConnection->getMetaData()->storesMixedCaseQuotedIdentifiers(),*this,m_aMutex,aNames);

    getTables();
    getParameters();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OQueryComposer::getComposedQuery(  ) throw(RuntimeException)
{
    if (OSubComponent::rBHelper.bDisposed)
        throw DisposedException();

    MutexGuard aGuard(m_aMutex);

    if (m_pSqlParseNode)
    {
        ::rtl::OUString aResult;
        m_pSqlParseNode->parseNodeToStr(aResult,m_xConnection->getMetaData());
        return aResult;
    }
    else
        return getQuery();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OQueryComposer::getFilter(  ) throw(RuntimeException)
{
    if (OSubComponent::rBHelper.bDisposed)
        throw DisposedException();
    MutexGuard aGuard(m_aMutex);
    return m_aFilter;
}
// -------------------------------------------------------------------------
Sequence< Sequence< PropertyValue > > SAL_CALL OQueryComposer::getStructuredFilter(  ) throw(RuntimeException)
{
    if (OSubComponent::rBHelper.bDisposed)
        throw DisposedException();

    MutexGuard aGuard(m_aMutex);

    Sequence< Sequence< PropertyValue > > aFilterSeq;
    if (m_aFilter.getLength() != 0)
    {
        ::rtl::OUString aSql(m_aWorkSql);
        // build a temporary parse node
        OSQLParseNode* pTempNode = m_pSqlParseNode;

        aSql += STR_WHERE;
        aSql += m_aFilter;
        //  THIS->resetIterator(aSql);
        ::rtl::OUString aErrorMsg;
        m_pSqlParseNode = m_aSqlParser.parseTree(aErrorMsg,aSql);
        m_aSqlIterator.setParseTree(m_pSqlParseNode);

        if (m_pSqlParseNode)
        {
            // normalize the filter
            OSQLParseNode* pWhereNode = m_pSqlParseNode->getChild(3)->getChild(1);
            OSQLParseNode* pCondition = pWhereNode->getChild(1);
            OSQLParseNode::negateSearchCondition(pCondition);
            pCondition = pWhereNode->getChild(1);
            OSQLParseNode::disjunctiveNormalForm(pCondition);
            pCondition = pWhereNode->getChild(1);
            OSQLParseNode::absorptions(pCondition);
            pCondition = pWhereNode->getChild(1);
            if (pCondition)
            {
                ::std::vector< ::std::vector < PropertyValue > > aFilters;
                Reference< ::com::sun::star::util::XNumberFormatter >  xFormatter(m_xServiceFactory
                                ->createInstance(rtl::OUString::createFromAscii("com.sun.star.util.NumberFormatter")), UNO_QUERY);
                xFormatter->attachNumberFormatsSupplier(m_xNumberFormatsSupplier);

                if (setORCriteria(pCondition, aFilters, xFormatter))
                {
                    aFilterSeq.realloc(aFilters.size());
                    Sequence<PropertyValue>* pFilters = aFilterSeq.getArray();
                    for (::std::vector< ::std::vector < PropertyValue > >::const_iterator i = aFilters.begin();
                         i != aFilters.end();
                         i++)
                    {
                        const ::std::vector < PropertyValue >& rProperties = *i;
                        pFilters->realloc(rProperties.size());
                        PropertyValue* pFilter = pFilters->getArray();
                        for (::std::vector < PropertyValue >::const_iterator j = rProperties.begin();
                             j != rProperties.end();
                             ++j)
                        {
                            *pFilter = *j;
                            pFilter++;
                        }
                        pFilters++;
                    }
                }
            }
            delete m_pSqlParseNode;
        }

        // restore
        m_pSqlParseNode = pTempNode;
        m_aSqlIterator.setParseTree(m_pSqlParseNode);
    }
    return aFilterSeq;
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OQueryComposer::getOrder(  ) throw(RuntimeException)
{
    if (OSubComponent::rBHelper.bDisposed)
        throw DisposedException();

    return m_aOrder;
}
// -----------------------------------------------------------------------------
void SAL_CALL OQueryComposer::appendFilterByColumn( const Reference< XPropertySet >& column ) throw(SQLException, RuntimeException)
{
    if (OSubComponent::rBHelper.bDisposed)
        throw DisposedException();

    if(!column.is() || !column->getPropertySetInfo()->hasPropertyByName(PROPERTY_VALUE))
        throw SQLException(::rtl::OUString::createFromAscii("Column doesn't support the property 'Value'!"),*this,::rtl::OUString::createFromAscii("HY000"),1000,Any());

    sal_Int32 nType = 0;
    column->getPropertyValue(PROPERTY_TYPE) >>= nType;
    sal_Int32 nSearchable = dbtools::getSearchColumnFlag(m_xConnection,nType);
    if(nSearchable == ColumnSearch::NONE)
        throw SQLException(::rtl::OUString::createFromAscii("Column not searchable!"),*this,::rtl::OUString::createFromAscii("HY000"),1000,Any());

    ::osl::MutexGuard aGuard( m_aMutex );

    ::rtl::OUString aName;
    column->getPropertyValue(PROPERTY_NAME) >>= aName;

    Any aValue;
    column->getPropertyValue(PROPERTY_VALUE) >>= aValue;

    ::rtl::OUString aSql;
    static ::rtl::OUString aQuote   = m_xConnection->getMetaData()->getIdentifierQuoteString();
    if(m_pColumns->hasByName(aName))
    {
        Reference<XPropertySet> xColumn;
        m_pColumns->getByName(aName) >>= xColumn;
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME),"Property REALNAME not available!");
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_TABLENAME),"Property TABLENAME not available!");

        ::rtl::OUString sRealName,sTableName;
        xColumn->getPropertyValue(PROPERTY_REALNAME)    >>= sRealName;
        xColumn->getPropertyValue(PROPERTY_TABLENAME)   >>= sTableName;
        if(sTableName.indexOf(0,'.') != -1)
        {
            ::rtl::OUString aCatlog,aSchema,aTable;
            ::dbtools::qualifiedNameComponents(m_xConnection->getMetaData(),sTableName,aCatlog,aSchema,aTable);
            ::dbtools::composeTableName(m_xConnection->getMetaData(),aCatlog,aSchema,aTable,sTableName,sal_True);
        }
        else
            sTableName = ::dbtools::quoteName(aQuote,sTableName);

        aSql =  sTableName;
        aSql += ::rtl::OUString::createFromAscii(".");
        aSql += ::dbtools::quoteName(aQuote,sRealName);
    }
    else
        aSql = getTableAlias(column) + ::dbtools::quoteName(aQuote,aName);

    //  ::rtl::OUString aSql = getTableAlias(column) + aName;

    switch(nType)
    {
        case DataType::VARCHAR:
        case DataType::CHAR:
        case DataType::LONGVARCHAR:
            aSql += STR_LIKE;
            aSql += ::rtl::OUString::createFromAscii("\'");
            aSql += toString(aValue).getStr();
            aSql += ::rtl::OUString::createFromAscii("\'");
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
                        aSql += ::rtl::OUString::createFromAscii("\'");
                    }
                    else
                        aSql += STR_EQUAL;
                    aSql += ::rtl::OUString::createFromAscii("0x");
                    const sal_Int8* pBegin  = aSeq.getConstArray();
                    const sal_Int8* pEnd    = pBegin + aSeq.getLength();
                    for(;pBegin != pEnd;++pBegin)
                    {
                        aSql += ::rtl::OUString::valueOf((sal_Int32)*pBegin,16).getStr();
                    }
                    if(nSearchable == ColumnSearch::NONE)
                        aSql += ::rtl::OUString::createFromAscii("\'");
                }
                else
                    throw SQLException(::rtl::OUString::createFromAscii("Column value isn't from type Sequence<sal_Int8>!"),*this,::rtl::OUString::createFromAscii("HY000"),1000,Any());
            }
            break;
        default:
            {
                ::rtl::OUString aValueStr(toString(aValue));
                aSql += STR_EQUAL;
                aSql += ::rtl::OUString::createFromAscii(" ");
                aSql += aValueStr.getStr();
            }
    }

    // filter anhaengen
    // select ohne where und order by aufbauen
    ::rtl::OUString aSql2(m_aWorkSql);

    if ((m_aFilter.getLength() != 0) && (aSql.getLength() != 0))
    {
        ::rtl::OUString sTemp(L_BRACKET);
        sTemp += m_aFilter;
        sTemp += R_BRACKET;
        sTemp += STR_AND;
        m_aFilter = sTemp;
    }
    m_aFilter += aSql;

    // add the filter and the sort order
    aSql2 += getComposedFilter();
    aSql2 += getComposedSort();

    resetIterator(aSql2);
}
// -------------------------------------------------------------------------
void SAL_CALL OQueryComposer::appendOrderByColumn( const Reference< XPropertySet >& column, sal_Bool ascending ) throw(SQLException, RuntimeException)
{
    if (OSubComponent::rBHelper.bDisposed)
        throw DisposedException();

    if(!column.is() || !column->getPropertySetInfo()->hasPropertyByName(PROPERTY_VALUE))
        throw SQLException(::rtl::OUString::createFromAscii("Column doesn't support the property 'Value'!"),*this,::rtl::OUString::createFromAscii("HY000"),1000,Any());

    ::osl::MutexGuard aGuard( m_aMutex );

    ::rtl::OUString aName,aAppendOrder;
    column->getPropertyValue(PROPERTY_NAME)         >>= aName;

    if(!m_xConnection->getMetaData()->supportsOrderByUnrelated() && !m_pColumns->hasByName(aName))
        throw SQLException(::rtl::OUString::createFromAscii("Column not in select clause!"),*this,::rtl::OUString::createFromAscii("HY000"),1000,Any());

    // filter anhaengen
    // select ohne where und order by aufbauen
    ::rtl::OUString aSql(m_aWorkSql);
    static ::rtl::OUString aQuote   = m_xConnection->getMetaData()->getIdentifierQuoteString();
    if(m_pColumns->hasByName(aName))
    {
        Reference<XPropertySet> xColumn;
        m_pColumns->getByName(aName) >>= xColumn;
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_REALNAME),"Property REALNAME not available!");
        OSL_ENSURE(xColumn->getPropertySetInfo()->hasPropertyByName(PROPERTY_TABLENAME),"Property TABLENAME not available!");

        ::rtl::OUString sRealName,sTableName;
        xColumn->getPropertyValue(PROPERTY_REALNAME)    >>= sRealName;
        xColumn->getPropertyValue(PROPERTY_TABLENAME)   >>= sTableName;
        if(sTableName.indexOf(0,'.') != -1)
        {
            ::rtl::OUString aCatlog,aSchema,aTable;
            ::dbtools::qualifiedNameComponents(m_xConnection->getMetaData(),sTableName,aCatlog,aSchema,aTable);
            ::dbtools::composeTableName(m_xConnection->getMetaData(),aCatlog,aSchema,aTable,sTableName,sal_True);
        }
        else
            sTableName = ::dbtools::quoteName(aQuote,sTableName);

        aAppendOrder =  sTableName;
        aAppendOrder += ::rtl::OUString::createFromAscii(".");
        aAppendOrder += ::dbtools::quoteName(aQuote,sRealName);
    }
    else
        aAppendOrder = getTableAlias(column) + ::dbtools::quoteName(aQuote,aName);

    if ((m_aOrder.getLength() != 0) && aAppendOrder.getLength())
        m_aOrder += COMMA;
    m_aOrder += aAppendOrder;
    if(!ascending && aAppendOrder.getLength())
        m_aOrder += ::rtl::OUString::createFromAscii(" DESC ");

    // add the filter and the sort order
    aSql += getComposedFilter();
    aSql += getComposedSort();

    resetIterator(aSql);
}
// -------------------------------------------------------------------------
void SAL_CALL OQueryComposer::setFilter( const ::rtl::OUString& filter ) throw(SQLException, RuntimeException)
{
    if (OSubComponent::rBHelper.bDisposed)
        throw DisposedException();

    ::osl::MutexGuard aGuard( m_aMutex );
    m_aFilter = filter;
    ::rtl::OUString aSql(m_aWorkSql);

    // add the filter and the sort order
    aSql += getComposedFilter();
    aSql += getComposedSort();

    resetIterator(aSql);

}
// -------------------------------------------------------------------------
void SAL_CALL OQueryComposer::setOrder( const ::rtl::OUString& order ) throw(SQLException, RuntimeException)
{
    if (OSubComponent::rBHelper.bDisposed)
        throw DisposedException();

    ::osl::MutexGuard aGuard( m_aMutex );

    m_aOrder = order;

    ::rtl::OUString aSql(m_aWorkSql);

    // add the filter and the sort order
    aSql += getComposedFilter();
    aSql += getGroupBy();
    aSql += getComposedSort();

    resetIterator(aSql);
}
// -------------------------------------------------------------------------
// XTablesSupplier
Reference< XNameAccess > SAL_CALL OQueryComposer::getTables(  ) throw(RuntimeException)
{
    if (OSubComponent::rBHelper.bDisposed)
        throw DisposedException();

    ::osl::MutexGuard aGuard( m_aMutex );
    if(!m_pTables)
    {
        const OSQLTables& aTables = m_aSqlIterator.getTables();
        ::std::vector< ::rtl::OUString> aNames;
        for(OSQLTables::const_iterator aIter = aTables.begin(); aIter != aTables.end();++aIter)
            aNames.push_back(aIter->first);

        m_pTables = new OPrivateTables(aTables,m_xConnection->getMetaData()->storesMixedCaseQuotedIdentifiers(),*this,m_aMutex,aNames);
    }

    return m_pTables;
}
// -------------------------------------------------------------------------
// XColumnsSupplier
Reference< XNameAccess > SAL_CALL OQueryComposer::getColumns(  ) throw(RuntimeException)
{
    if (OSubComponent::rBHelper.bDisposed)
        throw DisposedException();

    ::osl::MutexGuard aGuard( m_aMutex );
    return m_pColumns;
}
// -------------------------------------------------------------------------
sal_Bool OQueryComposer::setORCriteria(OSQLParseNode* pCondition,
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
sal_Bool OQueryComposer::setANDCriteria(OSQLParseNode * pCondition,
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


            //  pCondition->parseNodeToStr(aValue,m_xConnection->getMetaData(), xFormatter, m_aLocale,m_sDecimalSep.toChar());
            pCondition->parseNodeToStr(aValue,m_xConnection->getMetaData(),NULL);
            //  pCondition->getChild(0)->parseNodeToStr(aColumnName,m_xConnection->getMetaData(), xFormatter, m_aLocale,m_sDecimalSep.toChar());
            pCondition->getChild(0)->parseNodeToStr(aColumnName,m_xConnection->getMetaData(), NULL);

            // don't display the column name
            aValue = aValue.copy(aColumnName.getLength());
            aValue = aValue.trim();

            aItem.Name = getColumnName(pCondition->getChild(0));
            aItem.Value <<= aValue;
            aItem.Handle = 0; // just to know that this is not one the known ones
            if (SQL_ISRULE(pCondition,like_predicate))
            {
                if (pCondition->count() == 5)
                    aItem.Handle = SQL_PRED_NOTLIKE;
                else
                    aItem.Handle = SQL_PRED_LIKE;
            }
            else if (SQL_ISRULE(pCondition,test_for_null))
            {
                if (SQL_ISTOKEN(pCondition->getChild(2),NOT) )
                    aItem.Handle = SQL_PRED_ISNOTNULL;
                else
                    aItem.Handle = SQL_PRED_ISNULL;
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
sal_Int32 getPredicateType(OSQLParseNode * _pPredicate)
{
    sal_Int32 nPredicate = SQL_PRED_EQUAL;
    switch (_pPredicate->getNodeType())
    {
        case SQL_NODE_EQUAL:
            nPredicate = SQL_PRED_EQUAL;
            break;
        case SQL_NODE_LESS:
            nPredicate = SQL_PRED_LESS;
            break;
        case SQL_NODE_LESSEQ:
            nPredicate = SQL_PRED_LESSOREQUAL;
            break;
        case SQL_NODE_GREAT:
            nPredicate = SQL_PRED_GREATER;
            break;
        case SQL_NODE_GREATEQ:
            nPredicate = SQL_PRED_GREATEROREQUAL;
            break;
        default:
            OSL_ENSHURE(0,"Wrong NodeType!");
    }
    return nPredicate;
}
//------------------------------------------------------------------------------
sal_Bool OQueryComposer::setComparsionPredicate(OSQLParseNode * pCondition,
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
                pCondition->getChild(i)->parseNodeToPredicateStr(aValue,m_xConnection->getMetaData(), xFormatter, m_aLocale,m_sDecimalSep.toChar());
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
                    aItem.Handle = SQL_PRED_EQUAL;
                    break;
                case SQL_NODE_LESS:
                    // take the opposite as we change the order
                    i--;
                    aValue = ::rtl::OUString::createFromAscii(">=");
                    aItem.Handle = SQL_PRED_GREATEROREQUAL;
                    break;
                case SQL_NODE_LESSEQ:
                    // take the opposite as we change the order
                    i--;
                    aValue = ::rtl::OUString::createFromAscii(">");
                    aItem.Handle = SQL_PRED_GREATER;
                    break;
                case SQL_NODE_GREAT:
                    // take the opposite as we change the order
                    i--;
                    aValue = ::rtl::OUString::createFromAscii("<=");
                    aItem.Handle = SQL_PRED_LESSOREQUAL;
                    break;
                case SQL_NODE_GREATEQ:
                    // take the opposite as we change the order
                    i--;
                    aValue = ::rtl::OUString::createFromAscii("<");
                    aItem.Handle = SQL_PRED_LESS;
                    break;
            }

            // go backward
            for (; i >= 0; i--)
                pCondition->getChild(i)->parseNodeToPredicateStr(aValue,m_xConnection->getMetaData(), xFormatter, m_aLocale,m_sDecimalSep.toChar());
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

        pCondition->parseNodeToPredicateStr(aValue,m_xConnection->getMetaData(), xFormatter, m_aLocale,m_sDecimalSep.toChar());
        pCondition->getChild(0)->parseNodeToPredicateStr(aColumnName,m_xConnection->getMetaData(), xFormatter, m_aLocale,m_sDecimalSep.toChar());

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
             pCondition->getChild(i)->parseNodeToPredicateStr(aName,m_xConnection->getMetaData(), xFormatter, m_aLocale,m_sDecimalSep.toChar());

        // Kriterium
        aItem.Handle = pCondition->getChild(1)->getNodeType();
        aValue       = pCondition->getChild(1)->getTokenValue();
        for(i=0;i< pRhs->count();i++)
            pCondition->getChild(i)->parseNodeToPredicateStr(aValue,m_xConnection->getMetaData(), xFormatter, m_aLocale,m_sDecimalSep.toChar());

        aItem.Name = aName;
        aItem.Value <<= aValue;
        rFilter.push_back(aItem);
    }
    return sal_True;
}
// functions for analysing SQL
//--------------------------------------------------------------------------------------------------
::rtl::OUString OQueryComposer::getColumnName(::connectivity::OSQLParseNode* pColumnRef) const
{
    ::rtl::OUString aTableRange, aColumnName;
    m_aSqlIterator.getColumnRange(pColumnRef, aColumnName, aTableRange );
    return aColumnName;
}
//------------------------------------------------------------------------------
void OQueryComposer::resetIterator(const ::rtl::OUString& aSql)
{
    ::rtl::OUString aErrorMsg;
    delete m_pSqlParseNode;

    m_pSqlParseNode = m_aSqlParser.parseTree(aErrorMsg, aSql);
    m_aSqlIterator.setParseTree(m_pSqlParseNode);
}
//-----------------------------------------------------------------------------
::rtl::OUString OQueryComposer::getComposedFilter() const
{
    ::rtl::OUString aResult;
    // compose the query use brackets if necessary
    if ((m_aOrgFilter.getLength() != 0) && (m_aFilter.getLength() != 0))
        (((((((aResult = STR_WHERE) += L_BRACKET) += m_aOrgFilter) += R_BRACKET) += STR_AND) += L_BRACKET) += m_aFilter) += R_BRACKET;
    else if (m_aOrgFilter.getLength() != 0)
        (aResult = STR_WHERE) += m_aOrgFilter;
    else if (m_aFilter.getLength() != 0)
        (aResult = STR_WHERE) += m_aFilter;
    return aResult;
}

//-----------------------------------------------------------------------------
::rtl::OUString OQueryComposer::getComposedSort() const
{
    ::rtl::OUString aResult;
    // set the order part
    if ((m_aOrgOrder.getLength() != 0) && (m_aOrder.getLength() != 0))
        (((aResult = STR_ORDER_BY) += m_aOrgOrder) += COMMA) += m_aOrder;
    else if (m_aOrgOrder.getLength() != 0)
        (aResult = STR_ORDER_BY) += m_aOrgOrder;
    else if (m_aOrder.getLength() != 0)
        (aResult = STR_ORDER_BY) += m_aOrder;
    return aResult;
}
// -------------------------------------------------------------------------
::rtl::OUString OQueryComposer::getGroupBy() const
{
    ::rtl::OUString aResult;
    const OSQLParseNode* pGroupBy = m_aSqlIterator.getGroupByTree();
    if(pGroupBy)
        pGroupBy->parseNodeToStr(aResult,m_xConnection->getMetaData());

    const OSQLParseNode* pHaving = m_aSqlIterator.getHavingTree();
    if(pHaving)
        pHaving->parseNodeToStr(aResult,m_xConnection->getMetaData());
    return aResult;
}
// -----------------------------------------------------------------------------
::rtl::OUString OQueryComposer::getTableAlias(const Reference< XPropertySet >& column) const
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
            ::dbtools::composeTableName(m_xConnection->getMetaData(),aCatalog,aSchema,aTable,aComposedName,sal_False);

            // first check if this is the table we want to or has it a tablealias

            if(!m_pTables->hasByName(aComposedName))
            {
                ::comphelper::UStringMixEqual aComp(static_cast< ::comphelper::UStringMixLess*>(&m_aSqlIterator.getTables().key_comp())->isCaseSensitive());
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
            ::dbtools::composeTableName(m_xConnection->getMetaData(),aCatalog,aSchema,aTable,sReturn,sal_True);
            sReturn += ::rtl::OUString::createFromAscii(".");
        }
    }
    return sReturn;
}
// -----------------------------------------------------------------------------
Reference< XIndexAccess > SAL_CALL OQueryComposer::getParameters(  ) throw(RuntimeException)
{
    // now set the Parameters
    if(!m_pParameters)
    {
        const ::vos::ORef< OSQLColumns>& aCols = m_aSqlIterator.getParameters();
        ::std::vector< ::rtl::OUString> aNames;
        for(OSQLColumns::const_iterator aIter = aCols->begin(); aIter != aCols->end();++aIter)
            aNames.push_back(getString((*aIter)->getPropertyValue(PROPERTY_NAME)));
        m_pParameters = new OPrivateColumns(*aCols,m_xConnection->getMetaData()->storesMixedCaseQuotedIdentifiers(),*this,m_aMutex,aNames);
    }

    return m_pParameters;
}
// -----------------------------------------------------------------------------
