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

#include "ZPoolCollection.hxx"
#include "ZDriverWrapper.hxx"
#include "ZConnectionPool.hxx"
#include <com/sun/star/configuration/theDefaultProvider.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <comphelper/extract.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include "diagnose_ex.h"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::reflection;
using namespace ::osl;
using namespace connectivity;

//--------------------------------------------------------------------
static const ::rtl::OUString& getConnectionPoolNodeName()
{
    static ::rtl::OUString s_sNodeName(  "org.openoffice.Office.DataAccess/ConnectionPool" );
    return s_sNodeName;
}
//--------------------------------------------------------------------
static const ::rtl::OUString& getEnablePoolingNodeName()
{
    static ::rtl::OUString s_sNodeName(  "EnablePooling" );
    return s_sNodeName;
}
//--------------------------------------------------------------------
static const ::rtl::OUString& getDriverNameNodeName()
{
    static ::rtl::OUString s_sNodeName(  "DriverName" );
    return s_sNodeName;
}
// -----------------------------------------------------------------------------
static const ::rtl::OUString& getDriverSettingsNodeName()
{
    static ::rtl::OUString s_sNodeName(  "DriverSettings" );
    return s_sNodeName;
}
//--------------------------------------------------------------------------
static const ::rtl::OUString& getEnableNodeName()
{
    static ::rtl::OUString s_sNodeName(  "Enable" );
    return s_sNodeName;
}

//--------------------------------------------------------------------
OPoolCollection::OPoolCollection(const Reference< XMultiServiceFactory >&   _rxFactory)
    :m_xServiceFactory(_rxFactory)
{
    // bootstrap all objects supporting the .sdb.Driver service
    m_xManager = Reference< XDriverManager >(m_xServiceFactory->createInstance(::rtl::OUString("com.sun.star.sdbc.DriverManager") ), UNO_QUERY);
    m_xDriverAccess = Reference< XDriverAccess >(m_xManager, UNO_QUERY);
    OSL_ENSURE(m_xDriverAccess.is(), "have no (or an invalid) driver manager!");

    m_xProxyFactory = Reference< XProxyFactory >(
        m_xServiceFactory->createInstance(
            ::rtl::OUString("com.sun.star.reflection.ProxyFactory")),
        UNO_QUERY);
    OSL_ENSURE(m_xProxyFactory.is(), "OConnectionPool::OConnectionPool: could not create a proxy factory!");

    Reference<XPropertySet> xProp(getConfigPoolRoot(),UNO_QUERY);
    if ( xProp.is() )
        xProp->addPropertyChangeListener(getEnablePoolingNodeName(),this);
    // attach as desktop listener to know when we have to release our pools
    osl_atomic_increment( &m_refCount );
    {

        m_xDesktop = Reference< ::com::sun::star::frame::XDesktop>( m_xServiceFactory->createInstance(::rtl::OUString("com.sun.star.frame.Desktop") ), UNO_QUERY);
        if ( m_xDesktop.is() )
            m_xDesktop->addTerminateListener(this);

    }
    osl_atomic_decrement( &m_refCount );
}
// -----------------------------------------------------------------------------
OPoolCollection::~OPoolCollection()
{
    clearConnectionPools(sal_False);
}
// -----------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OPoolCollection::getConnection( const ::rtl::OUString& _rURL ) throw(SQLException, RuntimeException)
{
    return getConnectionWithInfo(_rURL,Sequence< PropertyValue >());
}
// -----------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OPoolCollection::getConnectionWithInfo( const ::rtl::OUString& _rURL, const Sequence< PropertyValue >& _rInfo ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    Reference< XConnection > xConnection;
    Reference< XDriver > xDriver;
    Reference< XInterface > xDriverNode;
    ::rtl::OUString sImplName;
    if(isPoolingEnabledByUrl(_rURL,xDriver,sImplName,xDriverNode) && xDriver.is())
    {
        OConnectionPool* pConnectionPool = getConnectionPool(sImplName,xDriver,xDriverNode);

        if(pConnectionPool)
            xConnection = pConnectionPool->getConnectionWithInfo(_rURL,_rInfo);
    }
    else if(xDriver.is())
        xConnection = xDriver->connect(_rURL,_rInfo);

    return xConnection;
}
// -----------------------------------------------------------------------------
void SAL_CALL OPoolCollection::setLoginTimeout( sal_Int32 seconds ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    m_xManager->setLoginTimeout(seconds);
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OPoolCollection::getLoginTimeout(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return m_xManager->getLoginTimeout();
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OPoolCollection::getImplementationName(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return getImplementationName_Static();
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OPoolCollection::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OPoolCollection::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//---------------------------------------OPoolCollection----------------------------------
Reference< XInterface > SAL_CALL OPoolCollection::CreateInstance(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return static_cast<XDriverManager*>(new OPoolCollection(_rxFactory));
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OPoolCollection::getImplementationName_Static(  ) throw(RuntimeException)
{
    return ::rtl::OUString("com.sun.star.sdbc.OConnectionPool");
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OPoolCollection::getSupportedServiceNames_Static(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(1);
    aSupported[0] = ::rtl::OUString("com.sun.star.sdbc.ConnectionPool");
    return aSupported;
}
// -----------------------------------------------------------------------------
Reference< XDriver > SAL_CALL OPoolCollection::getDriverByURL( const ::rtl::OUString& _rURL ) throw(RuntimeException)
{
    // returns the original driver when no connection pooling is enabled else it returns the proxy
    MutexGuard aGuard(m_aMutex);

    Reference< XDriver > xDriver;
    Reference< XInterface > xDriverNode;
    ::rtl::OUString sImplName;
    if(isPoolingEnabledByUrl(_rURL,xDriver,sImplName,xDriverNode))
    {
        Reference< XDriver > xExistentProxy;
        // look if we already have a proxy for this driver
        for (   ConstMapDriver2DriverRefIterator aLookup = m_aDriverProxies.begin();
                aLookup != m_aDriverProxies.end();
                ++aLookup
            )
        {
            // hold the proxy alive as long as we're in this loop round
            xExistentProxy = aLookup->second;

            if (xExistentProxy.is() && (aLookup->first.get() == xDriver.get()))
                // already created a proxy for this
                break;
        }
        if (xExistentProxy.is())
        {
            xDriver = xExistentProxy;
        }
        else
        {   // create a new proxy for the driver
            // this allows us to control the connections created by it
            if (m_xProxyFactory.is())
            {
                Reference< XAggregation > xDriverProxy = m_xProxyFactory->createProxy(xDriver.get());
                OSL_ENSURE(xDriverProxy.is(), "OConnectionPool::getDriverByURL: invalid proxy returned by the proxy factory!");

                OConnectionPool* pConnectionPool = getConnectionPool(sImplName,xDriver,xDriverNode);
                xDriver = new ODriverWrapper(xDriverProxy, pConnectionPool);
            }
            else
                OSL_FAIL("OConnectionPool::getDriverByURL: could not instantiate a proxy factory!");
        }
    }

    return xDriver;
}
// -----------------------------------------------------------------------------
sal_Bool OPoolCollection::isDriverPoolingEnabled(const ::rtl::OUString& _sDriverImplName,
                                                 Reference< XInterface >& _rxDriverNode)
{
    sal_Bool bEnabled = sal_False;
    Reference<XInterface> xConnectionPoolRoot = getConfigPoolRoot();
    // then look for which of them settings are stored in the configuration
    Reference< XNameAccess > xDirectAccess(openNode(getDriverSettingsNodeName(),xConnectionPoolRoot),UNO_QUERY);

    if(xDirectAccess.is())
    {
        Sequence< ::rtl::OUString > aDriverKeys = xDirectAccess->getElementNames();
        const ::rtl::OUString* pDriverKeys = aDriverKeys.getConstArray();
        const ::rtl::OUString* pDriverKeysEnd = pDriverKeys + aDriverKeys.getLength();
        for (;pDriverKeys != pDriverKeysEnd; ++pDriverKeys)
        {
            // the name of the driver in this round
            if(_sDriverImplName == *pDriverKeys)
            {
                _rxDriverNode = openNode(*pDriverKeys,xDirectAccess);
                if(_rxDriverNode.is())
                    getNodeValue(getEnableNodeName(),_rxDriverNode) >>= bEnabled;
                break;
            }
        }
    }
    return bEnabled;
}
// -----------------------------------------------------------------------------
sal_Bool OPoolCollection::isPoolingEnabled()
{
    // the config node where all pooling relevant info are stored under
    Reference<XInterface> xConnectionPoolRoot = getConfigPoolRoot();

    // the global "enabled" flag
    sal_Bool bEnabled = sal_False;
    if(xConnectionPoolRoot.is())
        getNodeValue(getEnablePoolingNodeName(),xConnectionPoolRoot) >>= bEnabled;
    return bEnabled;
}
// -----------------------------------------------------------------------------
Reference<XInterface> OPoolCollection::getConfigPoolRoot()
{
    if(!m_xConfigNode.is())
        m_xConfigNode = createWithServiceFactory(getConnectionPoolNodeName());
    return m_xConfigNode;
}
// -----------------------------------------------------------------------------
sal_Bool OPoolCollection::isPoolingEnabledByUrl(const ::rtl::OUString& _sUrl,
                                                Reference< XDriver >& _rxDriver,
                                                ::rtl::OUString& _rsImplName,
                                                Reference< XInterface >& _rxDriverNode)
{
    sal_Bool bEnabled = sal_False;
    if (m_xDriverAccess.is())
    {
        _rxDriver = m_xDriverAccess->getDriverByURL(_sUrl);
        if (_rxDriver.is() && isPoolingEnabled())
        {
            Reference< XServiceInfo > xSerivceInfo(_rxDriver,UNO_QUERY);
            OSL_ENSURE(xSerivceInfo.is(),"Each driver should have a XServiceInfo interface!");

            if(xSerivceInfo.is())
            {
                // look for the implementation name of the driver
                _rsImplName = xSerivceInfo->getImplementationName();
                bEnabled = isDriverPoolingEnabled(_rsImplName,_rxDriverNode);
            }
        }
    }
    return bEnabled;
}
// -----------------------------------------------------------------------------
void OPoolCollection::clearConnectionPools(sal_Bool _bDispose)
{
    OConnectionPools::const_iterator aIter = m_aPools.begin();
    while(aIter != m_aPools.end())
    {
        aIter->second->clear(_bDispose);
        aIter->second->release();
        ::rtl::OUString sKeyValue = aIter->first;
        ++aIter;
        m_aPools.erase(sKeyValue);
    }
}
// -----------------------------------------------------------------------------
OConnectionPool* OPoolCollection::getConnectionPool(const ::rtl::OUString& _sImplName,
                                                    const Reference< XDriver >& _xDriver,
                                                    const Reference< XInterface >& _xDriverNode)
{
    OConnectionPool *pRet = 0;
    OConnectionPools::const_iterator aFind = m_aPools.find(_sImplName);
    if (aFind != m_aPools.end())
        pRet = aFind->second;
    else if (_xDriver.is() && _xDriverNode.is())
    {
        Reference<XPropertySet> xProp(_xDriverNode,UNO_QUERY);
        if(xProp.is())
            xProp->addPropertyChangeListener(getEnableNodeName(),this);
        OConnectionPool* pConnectionPool = new OConnectionPool(_xDriver,_xDriverNode,m_xProxyFactory);
        pConnectionPool->acquire();
        aFind = m_aPools.insert(OConnectionPools::value_type(_sImplName,pConnectionPool)).first;
        pRet = aFind->second;
    }

    OSL_ENSURE(pRet, "Could not query DriverManager from ConnectionPool!");

    return pRet;
}
// -----------------------------------------------------------------------------
Reference< XInterface > OPoolCollection::createWithServiceFactory(const ::rtl::OUString& _rPath) const
{
    return createWithProvider(
        com::sun::star::configuration::theDefaultProvider::get(
            comphelper::getComponentContext(m_xServiceFactory)),
        _rPath);
}
//------------------------------------------------------------------------
Reference< XInterface > OPoolCollection::createWithProvider(const Reference< XMultiServiceFactory >& _rxConfProvider,
                            const ::rtl::OUString& _rPath) const
{
    OSL_ASSERT(_rxConfProvider.is());
    Sequence< Any > args(1);
    args[0] = makeAny(
        NamedValue(
            rtl::OUString("nodepath"),
            makeAny(_rPath)));
    Reference< XInterface > xInterface(
        _rxConfProvider->createInstanceWithArguments(
            rtl::OUString( "com.sun.star.configuration.ConfigurationAccess"),
            args));
    OSL_ENSURE(
        xInterface.is(),
        "::createWithProvider: could not create the node access!");
    return xInterface;
}
// -----------------------------------------------------------------------------
Reference<XInterface> OPoolCollection::openNode(const ::rtl::OUString& _rPath,const Reference<XInterface>& _xTreeNode) const throw()
{
    Reference< XHierarchicalNameAccess > xHierarchyAccess(_xTreeNode, UNO_QUERY);
    Reference< XNameAccess > xDirectAccess(_xTreeNode, UNO_QUERY);
    Reference< XInterface > xNode;

    try
    {
        if (xDirectAccess.is() && xDirectAccess->hasByName(_rPath))
        {
            if (!::cppu::extractInterface(xNode, xDirectAccess->getByName(_rPath)))
                OSL_FAIL("OConfigurationNode::openNode: could not open the node!");
        }
        else if (xHierarchyAccess.is())
        {
            if (!::cppu::extractInterface(xNode, xHierarchyAccess->getByHierarchicalName(_rPath)))
                OSL_FAIL("OConfigurationNode::openNode: could not open the node!");
        }

    }
    catch(const NoSuchElementException&)
    {
        OSL_FAIL((::rtl::OString("::openNode: there is no element named ")
                  +=  ::rtl::OString(_rPath.getStr(), _rPath.getLength(), RTL_TEXTENCODING_ASCII_US)
                  +=  ::rtl::OString("!")).getStr());
    }
    catch(Exception&)
    {
        OSL_FAIL("OConfigurationNode::openNode: caught an exception while retrieving the node!");
    }
    return xNode;
}
// -----------------------------------------------------------------------------
Any OPoolCollection::getNodeValue(const ::rtl::OUString& _rPath,const Reference<XInterface>& _xTreeNode) throw()
{
    Reference< XHierarchicalNameAccess > xHierarchyAccess(_xTreeNode, UNO_QUERY);
    Reference< XNameAccess > xDirectAccess(_xTreeNode, UNO_QUERY);
    Any aReturn;
    try
    {
        if (xDirectAccess.is() && xDirectAccess->hasByName(_rPath) )
        {
            aReturn = xDirectAccess->getByName(_rPath);
        }
        else if (xHierarchyAccess.is())
        {
            aReturn = xHierarchyAccess->getByHierarchicalName(_rPath);
        }
    }
    catch(NoSuchElementException& e)
    {
        OSL_UNUSED( e );    // make compiler happy
        OSL_FAIL((::rtl::OString("::getNodeValue: caught a NoSuchElementException while trying to open ")
                  +=  ::rtl::OString(e.Message.getStr(), e.Message.getLength(), RTL_TEXTENCODING_ASCII_US)
                  +=  ::rtl::OString("!")).getStr());
    }
    return aReturn;
}
// -----------------------------------------------------------------------------
void SAL_CALL OPoolCollection::queryTermination( const EventObject& /*Event*/ ) throw (::com::sun::star::frame::TerminationVetoException, RuntimeException)
{
}
// -----------------------------------------------------------------------------
void SAL_CALL OPoolCollection::notifyTermination( const EventObject& /*Event*/ ) throw (RuntimeException)
{
    clearDesktop();
}
// -----------------------------------------------------------------------------
void SAL_CALL OPoolCollection::disposing( const EventObject& Source ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if ( m_xDesktop == Source.Source )
    {
        clearDesktop();
    }
    else
    {
        try
        {
            Reference<XPropertySet> xProp(Source.Source,UNO_QUERY);
            if(Source.Source == m_xConfigNode)
            {
                if ( xProp.is() )
                    xProp->removePropertyChangeListener(getEnablePoolingNodeName(),this);
            m_xConfigNode.clear();
            }
            else if ( xProp.is() )
                xProp->removePropertyChangeListener(getEnableNodeName(),this);
        }
        catch(const Exception&)
        {
            OSL_FAIL("Exception caught");
        }
    }
}
// -----------------------------------------------------------------------------
void SAL_CALL OPoolCollection::propertyChange( const ::com::sun::star::beans::PropertyChangeEvent& evt ) throw (RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    if(evt.Source == m_xConfigNode)
    {
        sal_Bool bEnabled = sal_True;
        evt.NewValue >>= bEnabled;
        if(!bEnabled )
        {
            m_aDriverProxies.clear();
            m_aDriverProxies = MapDriver2DriverRef();
            OConnectionPools::iterator aIter = m_aPools.begin();
            for(;aIter != m_aPools.end();++aIter)
            {
                aIter->second->clear(sal_False);
                aIter->second->release();
            }
            m_aPools.clear();
            m_aPools         = OConnectionPools();
        }
    }
    else if(evt.Source.is())
    {
        sal_Bool bEnabled = sal_True;
        evt.NewValue >>= bEnabled;
        if(!bEnabled)
        {
            ::rtl::OUString sThisDriverName;
            getNodeValue(getDriverNameNodeName(),evt.Source) >>= sThisDriverName;
            // 1nd relase the driver
            // look if we already have a proxy for this driver
            MapDriver2DriverRefIterator aLookup = m_aDriverProxies.begin();
            while(  aLookup != m_aDriverProxies.end())
            {
                MapDriver2DriverRefIterator aFind = aLookup;
                Reference<XServiceInfo> xInfo(aLookup->first,UNO_QUERY);
                ++aLookup;
                if(xInfo.is() && xInfo->getImplementationName() == sThisDriverName)
                    m_aDriverProxies.erase(aFind);
            }

            // 2nd clear the connectionpool
            OConnectionPools::iterator aFind = m_aPools.find(sThisDriverName);
            if(aFind != m_aPools.end() && aFind->second)
            {
                aFind->second->clear(sal_False);
                aFind->second->release();
                m_aPools.erase(aFind);
            }
        }
    }
}
// -----------------------------------------------------------------------------
void OPoolCollection::clearDesktop()
{
    clearConnectionPools(sal_True);
    if ( m_xDesktop.is() )
        m_xDesktop->removeTerminateListener(this);
m_xDesktop.clear();
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
