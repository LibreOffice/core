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
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/reflection/ProxyFactory.hpp>
#include <com/sun/star/sdbc/DriverManager.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <osl/diagnose.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::reflection;
using namespace ::osl;
using namespace connectivity;


static OUString getConnectionPoolNodeName()
{
    return OUString(  "org.openoffice.Office.DataAccess/ConnectionPool" );
}

static OUString getEnablePoolingNodeName()
{
    return OUString(  "EnablePooling" );
}

static OUString getDriverNameNodeName()
{
    return OUString(  "DriverName" );
}

static OUString getDriverSettingsNodeName()
{
    return OUString(  "DriverSettings" );
}

static OUString getEnableNodeName()
{
    return OUString(  "Enable" );
}


OPoolCollection::OPoolCollection(const Reference< XComponentContext >& _rxContext)
    :m_xContext(_rxContext)
{
    // bootstrap all objects supporting the .sdb.Driver service
    m_xManager = DriverManager::create( m_xContext );

    m_xProxyFactory = ProxyFactory::create( m_xContext );

    Reference<XPropertySet> xProp(getConfigPoolRoot(),UNO_QUERY);
    if ( xProp.is() )
        xProp->addPropertyChangeListener(getEnablePoolingNodeName(),this);
    // attach as desktop listener to know when we have to release our pools
    osl_atomic_increment( &m_refCount );
    {

        m_xDesktop = css::frame::Desktop::create( m_xContext );
        m_xDesktop->addTerminateListener(this);

    }
    osl_atomic_decrement( &m_refCount );
}

OPoolCollection::~OPoolCollection()
{
    clearConnectionPools(false);
}

Reference< XConnection > SAL_CALL OPoolCollection::getConnection( const OUString& _rURL ) throw(SQLException, RuntimeException, std::exception)
{
    return getConnectionWithInfo(_rURL,Sequence< PropertyValue >());
}

Reference< XConnection > SAL_CALL OPoolCollection::getConnectionWithInfo( const OUString& _rURL, const Sequence< PropertyValue >& _rInfo ) throw(SQLException, RuntimeException, std::exception)
{
    MutexGuard aGuard(m_aMutex);
    Reference< XConnection > xConnection;
    Reference< XDriver > xDriver;
    Reference< XInterface > xDriverNode;
    OUString sImplName;
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

void SAL_CALL OPoolCollection::setLoginTimeout( sal_Int32 seconds ) throw(RuntimeException, std::exception)
{
    MutexGuard aGuard(m_aMutex);
    m_xManager->setLoginTimeout(seconds);
}

sal_Int32 SAL_CALL OPoolCollection::getLoginTimeout(  ) throw(RuntimeException, std::exception)
{
    MutexGuard aGuard(m_aMutex);
    return m_xManager->getLoginTimeout();
}

OUString SAL_CALL OPoolCollection::getImplementationName(  ) throw(RuntimeException, std::exception)
{
    return getImplementationName_Static();
}

sal_Bool SAL_CALL OPoolCollection::supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}


Sequence< OUString > SAL_CALL OPoolCollection::getSupportedServiceNames(  ) throw(RuntimeException, std::exception)
{
    return getSupportedServiceNames_Static();
}

//---------------------------------------OPoolCollection----------------------------------
Reference< XInterface > SAL_CALL OPoolCollection::CreateInstance(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return static_cast<XDriverManager*>(new OPoolCollection(comphelper::getComponentContext(_rxFactory)));
}


OUString SAL_CALL OPoolCollection::getImplementationName_Static(  ) throw(RuntimeException)
{
    return OUString("com.sun.star.sdbc.OConnectionPool");
}


Sequence< OUString > SAL_CALL OPoolCollection::getSupportedServiceNames_Static(  ) throw(RuntimeException)
{
    Sequence< OUString > aSupported { "com.sun.star.sdbc.ConnectionPool" };
    return aSupported;
}

Reference< XDriver > SAL_CALL OPoolCollection::getDriverByURL( const OUString& _rURL ) throw(RuntimeException, std::exception)
{
    // returns the original driver when no connection pooling is enabled else it returns the proxy
    MutexGuard aGuard(m_aMutex);

    Reference< XDriver > xDriver;
    Reference< XInterface > xDriverNode;
    OUString sImplName;
    if(isPoolingEnabledByUrl(_rURL,xDriver,sImplName,xDriverNode))
    {
        Reference< XDriver > xExistentProxy;
        // look if we already have a proxy for this driver
        for (   MapDriver2DriverRef::const_iterator aLookup = m_aDriverProxies.begin();
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
            Reference< XAggregation > xDriverProxy = m_xProxyFactory->createProxy(xDriver.get());
            OSL_ENSURE(xDriverProxy.is(), "OConnectionPool::getDriverByURL: invalid proxy returned by the proxy factory!");

            OConnectionPool* pConnectionPool = getConnectionPool(sImplName,xDriver,xDriverNode);
            xDriver = new ODriverWrapper(xDriverProxy, pConnectionPool);
        }
    }

    return xDriver;
}

bool OPoolCollection::isDriverPoolingEnabled(const OUString& _sDriverImplName,
                                                 Reference< XInterface >& _rxDriverNode)
{
    bool bEnabled = false;
    Reference<XInterface> xConnectionPoolRoot = getConfigPoolRoot();
    // then look for which of them settings are stored in the configuration
    Reference< XNameAccess > xDirectAccess(openNode(getDriverSettingsNodeName(),xConnectionPoolRoot),UNO_QUERY);

    if(xDirectAccess.is())
    {
        Sequence< OUString > aDriverKeys = xDirectAccess->getElementNames();
        const OUString* pDriverKeys = aDriverKeys.getConstArray();
        const OUString* pDriverKeysEnd = pDriverKeys + aDriverKeys.getLength();
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

bool OPoolCollection::isPoolingEnabled()
{
    // the config node where all pooling relevant info are stored under
    Reference<XInterface> xConnectionPoolRoot = getConfigPoolRoot();

    // the global "enabled" flag
    bool bEnabled = false;
    if(xConnectionPoolRoot.is())
        getNodeValue(getEnablePoolingNodeName(),xConnectionPoolRoot) >>= bEnabled;
    return bEnabled;
}

Reference<XInterface> const & OPoolCollection::getConfigPoolRoot()
{
    if(!m_xConfigNode.is())
        m_xConfigNode = createWithServiceFactory(getConnectionPoolNodeName());
    return m_xConfigNode;
}

bool OPoolCollection::isPoolingEnabledByUrl(const OUString& _sUrl,
                                                Reference< XDriver >& _rxDriver,
                                                OUString& _rsImplName,
                                                Reference< XInterface >& _rxDriverNode)
{
    bool bEnabled = false;
    _rxDriver = m_xManager->getDriverByURL(_sUrl);
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
    return bEnabled;
}

void OPoolCollection::clearConnectionPools(bool _bDispose)
{
    OConnectionPools::const_iterator aIter = m_aPools.begin();
    while(aIter != m_aPools.end())
    {
        aIter->second->clear(_bDispose);
        ++aIter;
    }
    m_aPools.clear();
}

OConnectionPool* OPoolCollection::getConnectionPool(const OUString& _sImplName,
                                                    const Reference< XDriver >& _xDriver,
                                                    const Reference< XInterface >& _xDriverNode)
{
    OConnectionPool *pRet = nullptr;
    OConnectionPools::const_iterator aFind = m_aPools.find(_sImplName);
    if (aFind != m_aPools.end())
        pRet = aFind->second.get();
    else if (_xDriver.is() && _xDriverNode.is())
    {
        Reference<XPropertySet> xProp(_xDriverNode,UNO_QUERY);
        if(xProp.is())
            xProp->addPropertyChangeListener(getEnableNodeName(),this);
        OConnectionPool* pConnectionPool = new OConnectionPool(_xDriver,_xDriverNode,m_xProxyFactory);
        aFind = m_aPools.insert(OConnectionPools::value_type(_sImplName,pConnectionPool)).first;
        pRet = aFind->second.get();
    }

    OSL_ENSURE(pRet, "Could not query DriverManager from ConnectionPool!");

    return pRet;
}

Reference< XInterface > OPoolCollection::createWithServiceFactory(const OUString& _rPath) const
{
    return createWithProvider(
        css::configuration::theDefaultProvider::get(m_xContext),
        _rPath);
}

Reference< XInterface > OPoolCollection::createWithProvider(const Reference< XMultiServiceFactory >& _rxConfProvider,
                            const OUString& _rPath)
{
    OSL_ASSERT(_rxConfProvider.is());
    Sequence< Any > args(1);
    args[0] = makeAny(
        NamedValue(
            OUString("nodepath"),
            makeAny(_rPath)));
    Reference< XInterface > xInterface(
        _rxConfProvider->createInstanceWithArguments(
            "com.sun.star.configuration.ConfigurationAccess",
            args));
    OSL_ENSURE(
        xInterface.is(),
        "::createWithProvider: could not create the node access!");
    return xInterface;
}

Reference<XInterface> OPoolCollection::openNode(const OUString& _rPath,const Reference<XInterface>& _xTreeNode) throw()
{
    Reference< XHierarchicalNameAccess > xHierarchyAccess(_xTreeNode, UNO_QUERY);
    Reference< XNameAccess > xDirectAccess(_xTreeNode, UNO_QUERY);
    Reference< XInterface > xNode;

    try
    {
        if (xDirectAccess.is() && xDirectAccess->hasByName(_rPath))
        {
            xNode.set(xDirectAccess->getByName(_rPath), css::uno::UNO_QUERY);
            SAL_WARN_IF(
                !xNode.is(), "connectivity.cpool",
                "OConfigurationNode::openNode: could not open the node!");
        }
        else if (xHierarchyAccess.is())
        {
            xNode.set(
                xHierarchyAccess->getByHierarchicalName(_rPath),
                css::uno::UNO_QUERY);
            SAL_WARN_IF(
                !xNode.is(), "connectivity.cpool",
                "OConfigurationNode::openNode: could not open the node!");
        }

    }
    catch(const NoSuchElementException&)
    {
        SAL_WARN("connectivity.cpool", "::openNode: there is no element named " <<
                 _rPath << "!");
    }
    catch(Exception&)
    {
        SAL_WARN("connectivity.cpool", "OConfigurationNode::openNode: caught an exception while retrieving the node!");
    }
    return xNode;
}

Any OPoolCollection::getNodeValue(const OUString& _rPath,const Reference<XInterface>& _xTreeNode) throw()
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
        SAL_WARN("connectivity.cpool", "::getNodeValue: caught a "
                 "NoSuchElementException while trying to open " <<
                 e.Message << "!" );
    }
    return aReturn;
}

void SAL_CALL OPoolCollection::queryTermination( const EventObject& /*Event*/ ) throw (css::frame::TerminationVetoException, RuntimeException, std::exception)
{
}

void SAL_CALL OPoolCollection::notifyTermination( const EventObject& /*Event*/ ) throw (RuntimeException, std::exception)
{
    clearDesktop();
}

void SAL_CALL OPoolCollection::disposing( const EventObject& Source ) throw (RuntimeException, std::exception)
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
            SAL_WARN("connectivity.cpool", "Exception caught");
        }
    }
}

void SAL_CALL OPoolCollection::propertyChange( const css::beans::PropertyChangeEvent& evt ) throw (RuntimeException, std::exception)
{
    MutexGuard aGuard(m_aMutex);
    if(evt.Source == m_xConfigNode)
    {
        bool bEnabled = true;
        evt.NewValue >>= bEnabled;
        if(!bEnabled )
        {
            m_aDriverProxies.clear();
            m_aDriverProxies = MapDriver2DriverRef();
            OConnectionPools::iterator aIter = m_aPools.begin();
            for(;aIter != m_aPools.end();++aIter)
            {
                aIter->second->clear(false);
            }
            m_aPools.clear();
        }
    }
    else if(evt.Source.is())
    {
        bool bEnabled = true;
        evt.NewValue >>= bEnabled;
        if(!bEnabled)
        {
            OUString sThisDriverName;
            getNodeValue(getDriverNameNodeName(),evt.Source) >>= sThisDriverName;
            // 1nd relase the driver
            // look if we already have a proxy for this driver
            MapDriver2DriverRef::iterator aLookup = m_aDriverProxies.begin();
            while(  aLookup != m_aDriverProxies.end())
            {
                MapDriver2DriverRef::iterator aFind = aLookup;
                Reference<XServiceInfo> xInfo(aLookup->first,UNO_QUERY);
                ++aLookup;
                if(xInfo.is() && xInfo->getImplementationName() == sThisDriverName)
                    m_aDriverProxies.erase(aFind);
            }

            // 2nd clear the connectionpool
            OConnectionPools::iterator aFind = m_aPools.find(sThisDriverName);
            if(aFind != m_aPools.end())
            {
                aFind->second->clear(false);
                m_aPools.erase(aFind);
            }
        }
    }
}

void OPoolCollection::clearDesktop()
{
    clearConnectionPools(true);
    if ( m_xDesktop.is() )
        m_xDesktop->removeTerminateListener(this);
m_xDesktop.clear();
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
