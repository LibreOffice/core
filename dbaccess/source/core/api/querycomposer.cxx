/*************************************************************************
 *
 *  $RCSfile: querycomposer.cxx,v $
 *
 *  $Revision: 1.59 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:03:25 $
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
#ifndef INCLUDED_SVTOOLS_SYSLOCALE_HXX
#include <svtools/syslocale.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef DBA_HELPERCOLLECTIONS_HXX
#include "HelperCollections.hxx"
#endif
#ifndef DBACCESS_CORE_API_SINGLESELECTQUERYCOMPOSER_HXX
#include "SingleSelectQueryComposer.hxx"
#endif
#include <algorithm>

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


namespace
{
    struct OrderCreator : public ::std::unary_function< ::rtl::OUString, void>
    {
        mutable ::rtl::OUString m_sOrder;

        OrderCreator(){}

        void operator() (const ::rtl::OUString& lhs) const
        {
            append(lhs);
        }
        void append(const ::rtl::OUString& lhs) const
        {
            if ( lhs.getLength() )
            {
                if ( m_sOrder.getLength() )
                    m_sOrder += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" , "));
                m_sOrder += lhs;
            }
        }
    };
}

DBG_NAME(OQueryComposer)
// -------------------------------------------------------------------------
OQueryComposer::OQueryComposer(const Reference< XNameAccess>& _xTableSupplier,
                               const Reference< XConnection>& _xConnection,
                               const Reference< XMultiServiceFactory >& _xServiceFactory)
 : OSubComponent(m_aMutex,_xConnection)
{
    DBG_CTOR(OQueryComposer,NULL);
    OSL_ENSURE(_xConnection.is()," Connection cant be null!");

    try
    {
        Reference<XMultiServiceFactory> xFac(_xConnection,UNO_QUERY);
        if ( xFac.is() )
        {
            m_xAnalyzer.set( xFac->createInstance( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER ),UNO_QUERY);
            m_xAnalyzerHelper.set( xFac->createInstance( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER ),UNO_QUERY);
        }
        else
        {
            m_xAnalyzer = new OSingleSelectQueryComposer(_xTableSupplier,_xConnection, _xServiceFactory);
            m_xAnalyzerHelper = new OSingleSelectQueryComposer(_xTableSupplier,_xConnection, _xServiceFactory);
        }

        m_xComposer.set( m_xAnalyzer,UNO_QUERY);
        m_xComposerHelper.set( m_xAnalyzerHelper,UNO_QUERY);
    }
    catch(Exception)
    {
    }
    OSL_ENSURE(m_xAnalyzer.is(),"Composer copuld be created!");
}
// -------------------------------------------------------------------------
OQueryComposer::~OQueryComposer()
{
    DBG_DTOR(OQueryComposer,NULL);
}
// -----------------------------------------------------------------------------
void SAL_CALL OQueryComposer::disposing()
{
    m_xComposer = NULL;
    m_xComposerHelper = NULL;
    ::comphelper::disposeComponent(m_xAnalyzerHelper);
    ::comphelper::disposeComponent(m_xAnalyzer);
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
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.dbaccess.OQueryComposer"));
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
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    Reference<XPropertySet> xProp(m_xAnalyzer,UNO_QUERY);
    ::rtl::OUString sQuery;
    xProp->getPropertyValue(PROPERTY_ORIGINAL) >>= sQuery;
    return sQuery;
}
// -------------------------------------------------------------------------
void SAL_CALL OQueryComposer::setQuery( const ::rtl::OUString& command ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    m_aFilters.clear();
    m_xAnalyzer->setQuery(command);
    m_sOrgFilter = m_xAnalyzer->getFilter();
    m_sOrgOrder = m_xAnalyzer->getOrder();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OQueryComposer::getComposedQuery(  ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    MutexGuard aGuard(m_aMutex);

    return m_xAnalyzer->getQuery();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OQueryComposer::getFilter(  ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    FilterCreator aFilterCreator;
    aFilterCreator = ::std::for_each(m_aFilters.begin(),m_aFilters.end(),aFilterCreator);
    return aFilterCreator.m_sFilter; // m_xAnalyzer->getFilter();
}
// -------------------------------------------------------------------------
Sequence< Sequence< PropertyValue > > SAL_CALL OQueryComposer::getStructuredFilter(  ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    MutexGuard aGuard(m_aMutex);
    return m_xAnalyzer->getStructuredFilter();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OQueryComposer::getOrder(  ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    OrderCreator aOrderCreator;
    aOrderCreator = ::std::for_each(m_aOrders.begin(),m_aOrders.end(),aOrderCreator);
    return aOrderCreator.m_sOrder;
}
// -----------------------------------------------------------------------------
void SAL_CALL OQueryComposer::appendFilterByColumn( const Reference< XPropertySet >& column ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_xAnalyzerHelper->setQuery(getQuery());
    m_xComposerHelper->setFilter(::rtl::OUString());
    m_xComposerHelper->appendFilterByColumn(column,sal_True);

    FilterCreator aFilterCreator;
    aFilterCreator.append(getFilter());
    aFilterCreator.append(m_xAnalyzerHelper->getFilter());

    setFilter(aFilterCreator.m_sFilter);
}
// -------------------------------------------------------------------------
void SAL_CALL OQueryComposer::appendOrderByColumn( const Reference< XPropertySet >& column, sal_Bool ascending ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_xAnalyzerHelper->setQuery(getQuery());
    m_xComposerHelper->setOrder(::rtl::OUString());
    m_xComposerHelper->appendOrderByColumn(column,ascending);

    OrderCreator aOrderCreator;
    aOrderCreator.append(getOrder());
    aOrderCreator.append(m_xAnalyzerHelper->getOrder());

    setOrder(aOrderCreator.m_sOrder);
}
// -------------------------------------------------------------------------
void SAL_CALL OQueryComposer::setFilter( const ::rtl::OUString& filter ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    FilterCreator aFilterCreator;
    aFilterCreator.append(m_sOrgFilter);
    aFilterCreator.append(filter);

    m_aFilters.clear();
    if ( filter.getLength() )
        m_aFilters.push_back(filter);

    m_xComposer->setFilter(aFilterCreator.m_sFilter);
}
// -------------------------------------------------------------------------
void SAL_CALL OQueryComposer::setOrder( const ::rtl::OUString& order ) throw(SQLException, RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );

    OrderCreator aOrderCreator;
    aOrderCreator.append(m_sOrgOrder);
    aOrderCreator.append(order);

    m_aOrders.clear();
    if ( order.getLength() )
        m_aOrders.push_back(order);

    m_xComposer->setOrder(aOrderCreator.m_sOrder);
}
// -------------------------------------------------------------------------
// XTablesSupplier
Reference< XNameAccess > SAL_CALL OQueryComposer::getTables(  ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    return Reference<XTablesSupplier>(m_xAnalyzer,UNO_QUERY)->getTables();
}
// -------------------------------------------------------------------------
// XColumnsSupplier
Reference< XNameAccess > SAL_CALL OQueryComposer::getColumns(  ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    return Reference<XColumnsSupplier>(m_xAnalyzer,UNO_QUERY)->getColumns();
}
// -------------------------------------------------------------------------
Reference< XIndexAccess > SAL_CALL OQueryComposer::getParameters(  ) throw(RuntimeException)
{
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    return Reference<XParametersSupplier>(m_xAnalyzer,UNO_QUERY)->getParameters();
}
// -----------------------------------------------------------------------------
void SAL_CALL OQueryComposer::acquire() throw()
{
    OSubComponent::acquire();
}
// -----------------------------------------------------------------------------
void SAL_CALL OQueryComposer::release() throw()
{
    OSubComponent::release();
}
// -----------------------------------------------------------------------------
