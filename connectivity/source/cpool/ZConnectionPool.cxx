/*************************************************************************
 *
 *  $RCSfile: ZConnectionPool.cxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-19 16:38:16 $
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
#ifndef CONNECTIVITY_POOLCOLLECTION_HXX
#include "ZPoolCollection.hxx"
#endif
#ifndef _CONNECTIVITY_CONNECTIONWRAPPER_HXX_
#include "connectivity/ConnectionWrapper.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _CONNECTIVITY_CONNECTIONWRAPPER_HXX_
#include "connectivity/ConnectionWrapper.hxx"
#endif


using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::osl;
using namespace connectivity;

#include <algorithm>

//==========================================================================
//= OPoolTimer
//==========================================================================
void SAL_CALL OPoolTimer::onShot()
{
    m_pPool->invalidatePooledConnections();
}
namespace
{
    //--------------------------------------------------------------------
    static const ::rtl::OUString& getTimeoutNodeName()
    {
        static ::rtl::OUString s_sNodeName = ::rtl::OUString::createFromAscii("Timeout");
        return s_sNodeName;
    }

}
//==========================================================================
//= OConnectionPool
//==========================================================================
//--------------------------------------------------------------------------
OConnectionPool::OConnectionPool(const Reference< XDriver >& _xDriver,
                                 const Reference< XInterface >& _xDriverNode,
                                 const Reference< ::com::sun::star::reflection::XProxyFactory >& _rxProxyFactory)
    :m_xDriver(_xDriver)
    ,m_xDriverNode(_xDriverNode)
    ,m_xProxyFactory(_rxProxyFactory)
    ,m_nTimeOut(10)
    ,m_nALiveCount(10)
{
    OSL_ENSURE(m_xDriverNode.is(),"NO valid Driver node set!");
    Reference< XComponent >  xComponent(m_xDriverNode, UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener(this);

    Reference<XPropertySet> xProp(m_xDriverNode,UNO_QUERY);
    if(xProp.is())
        xProp->addPropertyChangeListener(getTimeoutNodeName(),this);

    OPoolCollection::getNodeValue(getTimeoutNodeName(),m_xDriverNode) >>= m_nALiveCount;
    calculateTimeOuts(m_nALiveCount);

    m_xInvalidator = new OPoolTimer(this,::vos::TTimeValue(m_nTimeOut,0));
    m_xInvalidator->start();
}
// -----------------------------------------------------------------------------
OConnectionPool::~OConnectionPool()
{
    clear(sal_False);
}
// -----------------------------------------------------------------------------
struct TRemoveEventListenerFunctor : ::std::unary_function<TPooledConnections::value_type,void>
                                    ,::std::unary_function<TActiveConnectionMap::value_type,void>
{
    OConnectionPool* m_pConnectionPool;
    sal_Bool m_bDispose;

    TRemoveEventListenerFunctor(OConnectionPool* _pConnectionPool,sal_Bool _bDispose = sal_False)
        : m_pConnectionPool(_pConnectionPool)
        ,m_bDispose(_bDispose)
    {
        OSL_ENSURE(m_pConnectionPool,"No connection pool!");
    }
    // -----------------------------------------------------------------------------
    void dispose(const Reference<XInterface>& _xComponent)
    {
        Reference< XComponent >  xComponent(_xComponent, UNO_QUERY);

        if ( xComponent.is() )
        {
            xComponent->removeEventListener(m_pConnectionPool);
            if ( m_bDispose )
                xComponent->dispose();
        }
    }
    // -----------------------------------------------------------------------------
    void operator()(const TPooledConnections::value_type& _aValue)
    {
        dispose(_aValue);
    }
    // -----------------------------------------------------------------------------
    void operator()(const TActiveConnectionMap::value_type& _aValue)
    {
        dispose(_aValue.first);
    }
};
// -----------------------------------------------------------------------------
struct TConnectionPoolFunctor : ::std::unary_function<TConnectionMap::value_type,void>
{
    OConnectionPool* m_pConnectionPool;

    TConnectionPoolFunctor(OConnectionPool* _pConnectionPool)
        : m_pConnectionPool(_pConnectionPool)
    {
        OSL_ENSURE(m_pConnectionPool,"No connection pool!");
    }
    void operator()(const TConnectionMap::value_type& _aValue)
    {
        ::std::for_each(_aValue.second.aConnections.begin(),_aValue.second.aConnections.end(),TRemoveEventListenerFunctor(m_pConnectionPool,sal_True));
    }
};
// -----------------------------------------------------------------------------
void OConnectionPool::clear(sal_Bool _bDispose)
{
    MutexGuard aGuard(m_aMutex);

    if(m_xInvalidator->isTicking())
        m_xInvalidator->stop();

    ::std::for_each(m_aPool.begin(),m_aPool.end(),TConnectionPoolFunctor(this));
    m_aPool.clear();

    ::std::for_each(m_aActiveConnections.begin(),m_aActiveConnections.end(),TRemoveEventListenerFunctor(this,_bDispose));
    m_aActiveConnections.clear();

    Reference< XComponent >  xComponent(m_xDriverNode, UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener(this);
    Reference< XPropertySet >  xProp(m_xDriverNode, UNO_QUERY);
    if (xProp.is())
        xProp->removePropertyChangeListener(getTimeoutNodeName(),this);

    m_xDriverNode   = NULL;
    m_xDriver       = NULL;
}
//--------------------------------------------------------------------------
Reference< XConnection > SAL_CALL OConnectionPool::getConnectionWithInfo( const ::rtl::OUString& _rURL, const Sequence< PropertyValue >& _rInfo ) throw(SQLException, RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    Reference<XConnection> xConnection;

    // create a unique id and look for it in our map
    Sequence< PropertyValue > aInfo(_rInfo);
    TConnectionMap::key_type nId;
    OConnectionWrapper::createUniqueId(_rURL,aInfo,nId.m_pBuffer);
    TConnectionMap::iterator aIter = m_aPool.find(nId);

    if ( m_aPool.end() != aIter )
        xConnection = getPooledConnection(aIter);

    if ( !xConnection.is() )
        xConnection = createNewConnection(_rURL,_rInfo);

    return xConnection;
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
            aIter->second.aPos->second.nALiveCount = m_nALiveCount;
            aIter->second.aPos->second.aConnections.push_back(aIter->second.xPooledConnection);
            m_aActiveConnections.erase(aIter);
        }
    }
    else
    {
        m_xDriverNode = NULL;
    }
}
// -----------------------------------------------------------------------------
Reference< XConnection> OConnectionPool::createNewConnection(const ::rtl::OUString& _rURL,const Sequence< PropertyValue >& _rInfo)
{
    // create new pooled conenction
    Reference< XPooledConnection > xPooledConnection = new ::connectivity::OPooledConnection(m_xDriver->connect(_rURL,_rInfo),m_xProxyFactory);
    // get the new connection from the pooled connection
    Reference<XConnection> xConnection = xPooledConnection->getConnection();
    if(xConnection.is())
    {
        // add our own as dispose listener to know when we should put the connection back to the pool
        Reference< XComponent >  xComponent(xConnection, UNO_QUERY);
        if (xComponent.is())
            xComponent->addEventListener(this);

        // save some information to find the right pool later on
        Sequence< PropertyValue > aInfo(_rInfo);
        TConnectionMap::key_type nId;
        OConnectionWrapper::createUniqueId(_rURL,aInfo,nId.m_pBuffer);
        TConnectionPool aPack;

        // insert the new connection and struct into the active connection map
        aPack.nALiveCount               = m_nALiveCount;
        TActiveConnectionInfo aActiveInfo;
        aActiveInfo.aPos                = m_aPool.insert(TConnectionMap::value_type(nId,aPack)).first;
        aActiveInfo.xPooledConnection   = xPooledConnection;
        m_aActiveConnections.insert(TActiveConnectionMap::value_type(xConnection,aActiveInfo));

        if(m_xInvalidator->isExpired())
            m_xInvalidator->start();
    }

    return xConnection;
}
// -----------------------------------------------------------------------------
void OConnectionPool::invalidatePooledConnections()
{
    MutexGuard aGuard(m_aMutex);
    TConnectionMap::iterator aIter = m_aPool.begin();
    for (; aIter != m_aPool.end(); )
    {
        if(!(--(aIter->second.nALiveCount))) // connections are invalid
        {
            ::std::for_each(aIter->second.aConnections.begin(),aIter->second.aConnections.end(),TRemoveEventListenerFunctor(this,sal_True));

            aIter->second.aConnections.clear();

            // look if the iterator aIter is still present in the active connection map
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
    if(!m_aPool.empty())
        m_xInvalidator->start();
}
// -----------------------------------------------------------------------------
Reference< XConnection> OConnectionPool::getPooledConnection(TConnectionMap::iterator& _rIter)
{
    Reference<XConnection> xConnection;

    if(!_rIter->second.aConnections.empty())
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
void SAL_CALL OConnectionPool::propertyChange( const PropertyChangeEvent& evt ) throw (::com::sun::star::uno::RuntimeException)
{
    if(getTimeoutNodeName() == evt.PropertyName)
    {
        evt.NewValue >>= m_nALiveCount;
        calculateTimeOuts(m_nALiveCount);
    }
}
// -----------------------------------------------------------------------------
void OConnectionPool::calculateTimeOuts(sal_Int32 _nTimeOut)
{
    sal_Int32 nTimeOutCorrection = 10;
    if(m_nALiveCount < 100)
        nTimeOutCorrection = 20;

    m_nTimeOut      = m_nALiveCount / nTimeOutCorrection;
    m_nALiveCount   = m_nALiveCount / m_nTimeOut;
}
// -----------------------------------------------------------------------------
