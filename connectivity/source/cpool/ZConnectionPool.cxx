/*************************************************************************
 *
 *  $RCSfile: ZConnectionPool.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: fs $ $Date: 2001-06-27 10:14:52 $
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

#include <stdio.h>
#ifndef _CONNECTIVITY_ZCONNECTIONPOOL_HXX_
#include "ZConnectionPool.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_ELEMENTEXISTEXCEPTION_HPP_
#include <com/sun/star/container/ElementExistException.hpp>
#endif

#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COMPHELPER_TYPES_HXX_
#include <comphelper/types.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef CONNECTIVITY_POOLEDCONNECTION_HXX
#include "ZPooledConnection.hxx"
#endif
#ifndef _CONNECTIVITY_CPOOL_ZDRIVERWRAPPER_HXX_
#include "ZDriverWrapper.hxx"
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::reflection;
using namespace ::osl;
using namespace connectivity;

#define CONNECTION_TIMEOUT  10
// the connection will be expiared when 10 minutes are gone
//==========================================================================
//= OPoolTimer
//==========================================================================
void SAL_CALL OPoolTimer::onShot()
{
    m_pPool->invalidatePooledConnections();
}
//==========================================================================
//= OConnectionPool
//==========================================================================
//--------------------------------------------------------------------------
OConnectionPool::OConnectionPool(const Reference< XMultiServiceFactory >&   _rxFactory)
    :m_xServiceFactory(_rxFactory)
{
    // bootstrap all objects supporting the .sdb.Driver service
    m_xManager = Reference< XDriverManager >(m_xServiceFactory->createInstance(::rtl::OUString::createFromAscii("com.sun.star.sdbc.DriverManager") ), UNO_QUERY);
    m_xDriverAccess = Reference< XDriverAccess >(m_xManager, UNO_QUERY);

    OSL_ENSURE(m_xDriverAccess.is(), "OConnectionPool::OConnectionPool: have no (or an invalid) driver manager!");

    m_xTimer = new OPoolTimer(this,::vos::TTimeValue(10,0));
    m_xTimer->start();

    m_xProxyFactory = Reference< XProxyFactory >(
        m_xServiceFactory->createInstance(
            ::rtl::OUString::createFromAscii("com.sun.star.reflection.ProxyFactory")),
        UNO_QUERY);
    OSL_ENSURE(m_xProxyFactory.is(), "OConnectionPool::OConnectionPool: could not create a proxy factory!");
}
// -----------------------------------------------------------------------------
OConnectionPool::~OConnectionPool()
{
    {
        MutexGuard aGuard(m_aMutex);
        if(m_xTimer->isTicking())
            m_xTimer->stop();
    }
}
//--------------------------------------------------------------------------
void SAL_CALL OConnectionPool::acquire() throw(RuntimeException)
{
    osl_incrementInterlockedCount(&m_refCount);
}
// -----------------------------------------------------------------------------
void SAL_CALL OConnectionPool::release() throw(RuntimeException)
{
    osl_decrementInterlockedCount(&m_refCount);
}

//--------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OConnectionPool::getConnection( const ::rtl::OUString& _rURL ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    Reference<XConnection> xConnection;
    pair<TConnectionMap::iterator, TConnectionMap::iterator> aThisURLConns = m_aPool.equal_range(_rURL);
    TConnectionMap::iterator aIter = aThisURLConns.first;

    if (aIter != aThisURLConns.second)
    {// we know the url so we have to check if we found one without properties
        do
        {
            if(!aIter->second.aProps.size())
                xConnection = getPooledConnection(aIter);
        }
        while ((++aIter != aThisURLConns.second) && !xConnection.is());
    }
    if(!xConnection.is())
        xConnection = createNewConnection(_rURL,Sequence< PropertyValue >());

    return xConnection;
}

//--------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OConnectionPool::getConnectionWithInfo( const ::rtl::OUString& _rURL, const Sequence< PropertyValue >& _rInfo ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    Reference<XConnection> xConnection;

    pair<TConnectionMap::iterator, TConnectionMap::iterator> aThisURLConns = m_aPool.equal_range(_rURL);
    TConnectionMap::iterator aIter = aThisURLConns.first;

    if  (aIter != m_aPool.end())
    {// we know the url so we have to check if we found one without properties
        PropertyMap aMap;
        createPropertyMap(_rInfo,aMap);

        do
        {
            if (checkSequences(aIter->second.aProps,aMap))
                xConnection = getPooledConnection(aIter);
        }
        while ((++aIter != aThisURLConns.second) && !xConnection.is());
    }
    if(!xConnection.is())
        xConnection = createNewConnection(_rURL,_rInfo);

    return xConnection;
}
//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnectionPool::getImplementationName(  ) throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);
    return getImplementationName_Static();
}

//--------------------------------------------------------------------------
sal_Bool SAL_CALL OConnectionPool::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());
    const ::rtl::OUString* pSupported = aSupported.getConstArray();
    const ::rtl::OUString* pEnd = pSupported + aSupported.getLength();
    for (;pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported)
        ;

    return pSupported != pEnd;
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OConnectionPool::getSupportedServiceNames(  ) throw(RuntimeException)
{
    return getSupportedServiceNames_Static();
}

//--------------------------------------------------------------------------
Reference< XInterface > SAL_CALL OConnectionPool::CreateInstance(const Reference< XMultiServiceFactory >& _rxFactory)
{
    return static_cast<XDriverManager*>(new OConnectionPool(_rxFactory));
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OConnectionPool::getImplementationName_Static(  ) throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.sdbc.OConnectionPool");
}

//--------------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OConnectionPool::getSupportedServiceNames_Static(  ) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(1);
    aSupported[0] = ::rtl::OUString::createFromAscii("com.sun.star.sdbc.ConnectionPool");
    return aSupported;
}
//--------------------------------------------------------------------------
Reference< XDriver > SAL_CALL OConnectionPool::getDriverByURL( const ::rtl::OUString& _rURL ) throw(RuntimeException)
{
    Reference< XDriver > xDriver;
    if (m_xDriverAccess.is())
    {
        xDriver = m_xDriverAccess->getDriverByURL(_rURL);
        if (xDriver.is())
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

                    xDriver = new ODriverWrapper(xDriverProxy, this);
                }
                else
                    OSL_ENSURE(sal_False, "OConnectionPool::getDriverByURL: could not instantiate a proxy factory!");
            }
        }
    }

    return xDriver;
}

//--------------------------------------------------------------------------
void SAL_CALL OConnectionPool::disposing( const ::com::sun::star::lang::EventObject& Source ) throw (RuntimeException)
{
    Reference<XConnection> xConnection(Source.Source,UNO_QUERY);
    if(xConnection.is())
    {
        MutexGuard aGuard(m_aMutex);
        TActiveConnectionMap::iterator aIter = m_aActiveConnections.find(xConnection);
        OSL_ENSURE(aIter != m_aActiveConnections.end(),"OConnectionPool::disposing: Conenction wasn't in pool");
        if(aIter != m_aActiveConnections.end())
        { // move the pooled connection back to the pool
            aIter->second.aPos->second.nALiveCount = CONNECTION_TIMEOUT;
            aIter->second.aPos->second.aConnections.push_back(aIter->second.xPooledConnection);
            m_aActiveConnections.erase(aIter);
        }
    }
}
// -----------------------------------------------------------------------------
sal_Bool OConnectionPool::checkSequences(const PropertyMap& _rLh,const PropertyMap& _rRh)
{
    if(_rLh.size() != _rRh.size())
        return sal_False;
    sal_Bool bRet = sal_True;
    PropertyMap::const_iterator aIter = _rLh.begin();
    for (; bRet && aIter != _rLh.end(); ++aIter)
    {
        PropertyMap::const_iterator aFind = _rRh.find(aIter->first);
        bRet = (aFind != _rRh.end()) ? ::comphelper::compare(aFind->second,aIter->second) : sal_False;
    }
    return bRet;
}
// -----------------------------------------------------------------------------
void SAL_CALL OConnectionPool::setLoginTimeout( sal_Int32 seconds ) throw(RuntimeException)
{
    m_xManager->setLoginTimeout(seconds);
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL OConnectionPool::getLoginTimeout(  ) throw(RuntimeException)
{
    return m_xManager->getLoginTimeout();
}
// -----------------------------------------------------------------------------
Reference< XConnection> OConnectionPool::createNewConnection(const ::rtl::OUString& _rURL,const Sequence< PropertyValue >& _rInfo)
{
    // create new pooled conenction
    Reference< XPooledConnection > xPooledConnection = new ::connectivity::OPooledConnection(m_xManager->getConnectionWithInfo(_rURL,_rInfo));
    // get the new connection from the pooled connection
    Reference<XConnection> xConnection = xPooledConnection->getConnection();
    if(xConnection.is())
    {
        // add our own as dispose listener to know when we should put the connection back to the pool
        Reference< XComponent >  xComponent(xConnection, UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener(this);

        // save some information to find the right pool later on
        PropertyMap aMap;
        createPropertyMap(_rInfo,aMap); // by ref to avoid copying
        TConnectionPool aPack;
        aPack.aProps        = aMap;
        aPack.nALiveCount   = CONNECTION_TIMEOUT;
        TActiveConnectionInfo aActiveInfo;
        aActiveInfo.aPos = m_aPool.insert(TConnectionMap::value_type(_rURL,aPack));
        aActiveInfo.xPooledConnection = xPooledConnection;
        m_aActiveConnections[xConnection] = aActiveInfo;

        if(m_xTimer->isExpired())
            m_xTimer->start();
    }

    return xConnection;
}
// -----------------------------------------------------------------------------
void OConnectionPool::createPropertyMap(const Sequence< PropertyValue >& _rInfo,PropertyMap& _rMap)
{
    const PropertyValue* pBegin   = _rInfo.getConstArray();
    const PropertyValue* pEnd     = pBegin + _rInfo.getLength();
    for(;pBegin != pEnd;++pBegin)
    {
        _rMap[pBegin->Name] = pBegin->Value;
    }
}
// -----------------------------------------------------------------------------
void OConnectionPool::invalidatePooledConnections()
{
    MutexGuard aGuard(m_aMutex);
    TConnectionMap::iterator aIter = m_aPool.begin();
    for (; aIter != m_aPool.end(); )
    {
        --(aIter->second.nALiveCount);
        if(!(aIter->second.nALiveCount)) // connections are invalid
        {
            TPooledConnections::iterator aLoop = aIter->second.aConnections.begin();
            for (; aLoop != aIter->second.aConnections.end();++aLoop )
                ::comphelper::disposeComponent(*aLoop);
            aIter->second.aConnections.clear();

            // look if the itertaor aIter is still present in the active connection map
            TActiveConnectionMap::iterator aActIter = m_aActiveConnections.begin();
            for (; aActIter != m_aActiveConnections.end(); ++aActIter)
            {
                if(aIter == aActIter->second.aPos)
                    break;
            }
            if(aActIter == m_aActiveConnections.end())
            {// he isn't so we can delete him
                TConnectionMap::iterator aDeleteIter = aIter;
                ++aIter;
                m_aPool.erase(aDeleteIter);
            }
            else
                ++aIter;
        }
        else
            ++aIter;
    }
    if(m_aPool.size())
        m_xTimer->start();
}
// -----------------------------------------------------------------------------
Reference< XConnection> OConnectionPool::getPooledConnection(TConnectionMap::iterator& _rIter)
{
    Reference<XConnection> xConnection;

    if(_rIter->second.aConnections.size())
    {
        Reference< XPooledConnection > xPooledConnection = _rIter->second.aConnections.back();
        _rIter->second.aConnections.pop_back();

        OSL_ENSURE(xPooledConnection.is(),"Can not be null here!");
        xConnection = xPooledConnection->getConnection();
        Reference< XComponent >  xComponent(xConnection, UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener(this);

        TActiveConnectionInfo aActiveInfo;
        aActiveInfo.aPos = _rIter;
        aActiveInfo.xPooledConnection = xPooledConnection;
        m_aActiveConnections[xConnection] = aActiveInfo;
    }
    return xConnection;
}
// -----------------------------------------------------------------------------
