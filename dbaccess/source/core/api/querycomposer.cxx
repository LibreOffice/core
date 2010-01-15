/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: querycomposer.cxx,v $
 * $Revision: 1.68 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

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
#ifndef _COM_SUN_STAR_LANG_SERVICENOTREGISTEREDEXCEPTION_HPP_
#include <com/sun/star/lang/ServiceNotRegisteredException.hpp>
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
#include <unotools/syslocale.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCHILD_HPP_
#include <com/sun/star/container/XChild.hpp>
#endif
#ifndef DBACCESS_CORE_API_QUERYCOMPOSER_HXX
#include "querycomposer.hxx"
#endif
#ifndef DBA_HELPERCOLLECTIONS_HXX
#include "HelperCollections.hxx"
#endif
#ifndef DBACCESS_SOURCE_CORE_INC_COMPOSERTOOLS_HXX
#include "composertools.hxx"
#endif
#include <algorithm>
#include <rtl/logfile.hxx>

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


DBG_NAME(OQueryComposer)
// -------------------------------------------------------------------------
OQueryComposer::OQueryComposer(const Reference< XConnection>& _xConnection)
 : OSubComponent(m_aMutex,_xConnection)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::OQueryComposer" );
    DBG_CTOR(OQueryComposer,NULL);
    OSL_ENSURE(_xConnection.is()," Connection cant be null!");

    Reference<XMultiServiceFactory> xFac( _xConnection, UNO_QUERY_THROW );
    m_xComposer.set( xFac->createInstance( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER ), UNO_QUERY_THROW );
    m_xComposerHelper.set( xFac->createInstance( SERVICE_NAME_SINGLESELECTQUERYCOMPOSER ), UNO_QUERY_THROW );
}
// -------------------------------------------------------------------------
OQueryComposer::~OQueryComposer()
{
    DBG_DTOR(OQueryComposer,NULL);
}
// -----------------------------------------------------------------------------
void SAL_CALL OQueryComposer::disposing()
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::disposing" );
    ::comphelper::disposeComponent(m_xComposerHelper);
    ::comphelper::disposeComponent(m_xComposer);
}
// -------------------------------------------------------------------------
// ::com::sun::star::lang::XTypeProvider
Sequence< Type > SAL_CALL OQueryComposer::getTypes() throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::getTypes" );
    return ::comphelper::concatSequences(OSubComponent::getTypes(),OQueryComposer_BASE::getTypes());
}
// -------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL OQueryComposer::getImplementationId() throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::getImplementationId" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::getSomething" );
    if (rId.getLength() == 16 && 0 == rtl_compareMemory(getImplementationId().getConstArray(),  rId.getConstArray(), 16 ) )
        return reinterpret_cast<sal_Int64>(this);

    return 0;
}
// -------------------------------------------------------------------------
Any SAL_CALL OQueryComposer::queryInterface( const Type & rType ) throw(RuntimeException)
{
    //RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::queryInterface" );
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
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::getImplementationName" );
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.sdb.dbaccess.OQueryComposer"));
}
//------------------------------------------------------------------------------
sal_Bool OQueryComposer::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::supportsService" );
    return ::comphelper::findValue(getSupportedServiceNames(), _rServiceName, sal_True).getLength() != 0;
}
//------------------------------------------------------------------------------
Sequence< ::rtl::OUString > OQueryComposer::getSupportedServiceNames(  ) throw (RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::getSupportedServiceNames" );
    Sequence< rtl::OUString > aSNS( 1 );
    aSNS[0] = SERVICE_SDB_SQLQUERYCOMPOSER;
    return aSNS;
}
// -------------------------------------------------------------------------
// XSQLQueryComposer
::rtl::OUString SAL_CALL OQueryComposer::getQuery(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::getQuery" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    Reference<XPropertySet> xProp(m_xComposer,UNO_QUERY);
    ::rtl::OUString sQuery;
    if ( xProp.is() )
        xProp->getPropertyValue(PROPERTY_ORIGINAL) >>= sQuery;
    return sQuery;
}
// -------------------------------------------------------------------------
void SAL_CALL OQueryComposer::setQuery( const ::rtl::OUString& command ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::setQuery" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    m_aFilters.clear();
    m_xComposer->setQuery(command);
    m_sOrgFilter = m_xComposer->getFilter();
    m_sOrgOrder = m_xComposer->getOrder();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OQueryComposer::getComposedQuery(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::getComposedQuery" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    MutexGuard aGuard(m_aMutex);

    return m_xComposer->getQuery();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OQueryComposer::getFilter(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::getFilter" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    MutexGuard aGuard(m_aMutex);
    FilterCreator aFilterCreator;
    aFilterCreator = ::std::for_each(m_aFilters.begin(),m_aFilters.end(),aFilterCreator);
    return aFilterCreator.getComposedAndClear();
}
// -------------------------------------------------------------------------
Sequence< Sequence< PropertyValue > > SAL_CALL OQueryComposer::getStructuredFilter(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::getStructuredFilter" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    MutexGuard aGuard(m_aMutex);
    return m_xComposer->getStructuredFilter();
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL OQueryComposer::getOrder(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::getOrder" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    OrderCreator aOrderCreator;
    aOrderCreator = ::std::for_each(m_aOrders.begin(),m_aOrders.end(),aOrderCreator);
    return aOrderCreator.getComposedAndClear();
}
// -----------------------------------------------------------------------------
void SAL_CALL OQueryComposer::appendFilterByColumn( const Reference< XPropertySet >& column ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::appendFilterByColumn" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_xComposerHelper->setQuery(getQuery());
    m_xComposerHelper->setFilter(::rtl::OUString());
    m_xComposerHelper->appendFilterByColumn(column,sal_True);

    FilterCreator aFilterCreator;
    aFilterCreator.append(getFilter());
    aFilterCreator.append(m_xComposerHelper->getFilter());

    setFilter( aFilterCreator.getComposedAndClear() );
}
// -------------------------------------------------------------------------
void SAL_CALL OQueryComposer::appendOrderByColumn( const Reference< XPropertySet >& column, sal_Bool ascending ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::appendOrderByColumn" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);
    ::osl::MutexGuard aGuard( m_aMutex );

    m_xComposerHelper->setQuery(getQuery());
    m_xComposerHelper->setOrder(::rtl::OUString());
    m_xComposerHelper->appendOrderByColumn(column,ascending);

    OrderCreator aOrderCreator;
    aOrderCreator.append(getOrder());
    aOrderCreator.append(m_xComposerHelper->getOrder());

    setOrder(aOrderCreator.getComposedAndClear());
}
// -------------------------------------------------------------------------
void SAL_CALL OQueryComposer::setFilter( const ::rtl::OUString& filter ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::setFilter" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    FilterCreator aFilterCreator;
    aFilterCreator.append(m_sOrgFilter);
    aFilterCreator.append(filter);

    m_aFilters.clear();
    if ( filter.getLength() )
        m_aFilters.push_back(filter);

    m_xComposer->setFilter( aFilterCreator.getComposedAndClear() );
}
// -------------------------------------------------------------------------
void SAL_CALL OQueryComposer::setOrder( const ::rtl::OUString& order ) throw(SQLException, RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::setOrder" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );

    OrderCreator aOrderCreator;
    aOrderCreator.append(m_sOrgOrder);
    aOrderCreator.append(order);

    m_aOrders.clear();
    if ( order.getLength() )
        m_aOrders.push_back(order);

    m_xComposer->setOrder(aOrderCreator.getComposedAndClear());
}
// -------------------------------------------------------------------------
// XTablesSupplier
Reference< XNameAccess > SAL_CALL OQueryComposer::getTables(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::getTables" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    return Reference<XTablesSupplier>(m_xComposer,UNO_QUERY)->getTables();
}
// -------------------------------------------------------------------------
// XColumnsSupplier
Reference< XNameAccess > SAL_CALL OQueryComposer::getColumns(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::getColumns" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    return Reference<XColumnsSupplier>(m_xComposer,UNO_QUERY)->getColumns();
}
// -------------------------------------------------------------------------
Reference< XIndexAccess > SAL_CALL OQueryComposer::getParameters(  ) throw(RuntimeException)
{
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "Ocke.Janssen@sun.com", "OQueryComposer::getParameters" );
    ::connectivity::checkDisposed(OSubComponent::rBHelper.bDisposed);

    ::osl::MutexGuard aGuard( m_aMutex );
    return Reference<XParametersSupplier>(m_xComposer,UNO_QUERY)->getParameters();
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





