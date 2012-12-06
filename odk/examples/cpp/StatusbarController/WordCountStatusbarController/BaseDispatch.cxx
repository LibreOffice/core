/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include "BaseDispatch.hxx"
#include "defines.hxx"

#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <map>
#include <algorithm>

using namespace com::sun::star;
using namespace framework::statusbar_controller_wordcount;

using com::sun::star::beans::PropertyValue;
using com::sun::star::frame::FeatureStateEvent;
using com::sun::star::frame::XController;
using com::sun::star::frame::XDispatch;
using com::sun::star::frame::XFrame;
using com::sun::star::frame::XStatusListener;
using com::sun::star::lang::DisposedException;
using com::sun::star::uno::Reference;
using com::sun::star::uno::RuntimeException;
using com::sun::star::uno::Sequence;
using com::sun::star::uno::UNO_QUERY;
using com::sun::star::uno::UNO_QUERY_THROW;
using com::sun::star::uno::XComponentContext;
using com::sun::star::uno::XInterface;
using com::sun::star::util::URL;

using rtl::OUString;

namespace
{
    template< class Map >
    struct lcl_DisposeAndClearAndDeleteMapElement :
        public ::std::unary_function< typename Map::value_type, void >
    {

            lcl_DisposeAndClearAndDeleteMapElement( const Reference< uno::XInterface > &xEventSource ) :
                m_aEvent( xEventSource ) { }

            void operator( ) ( typename Map::value_type &rElement )
            {
                if ( rElement.second )
                {
                    rElement.second->disposeAndClear( m_aEvent );
                    delete rElement.second;
                }
            }
        private:
            lang::EventObject m_aEvent;
    };

    template< class Map >
    void
    lcl_DisposeAndClearAndDeleteAllMapElements(
        Map &rMap,
        const Reference< uno::XInterface > &xEventSource )
    {
        ::std::for_each( rMap.begin( ), rMap.end( ),
                         lcl_DisposeAndClearAndDeleteMapElement< Map > ( xEventSource ) );
    }
}

BaseDispatch::BaseDispatch(
    const Reference< XComponentContext > &xContext,
    const Reference<XFrame> &xFrame,
    const OUString &rModuleIdentifier )
    : BaseDispatch_Base( m_aMutex )
    , m_xContext( xContext )
    , m_xFrame( xFrame )
    , m_sModuleIdentifier( rModuleIdentifier )
{
    OSL_TRACE( "sbctlwc::BaseDispatch::BaseDispatch" );
}

BaseDispatch::~BaseDispatch()
{
    OSL_TRACE( "sbctlwc::BaseDispatch::~BaseDispatch" );
}

void SAL_CALL
BaseDispatch::addStatusListener(
    const Reference< XStatusListener > &xControl,
    const URL &aURL )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlwc::BaseDispatch::addStatusListener" );
    osl::MutexGuard aGuad( m_aMutex );

    tListenerMap::iterator aIt( m_aListeners.find( aURL.Complete ) );
    if ( aIt == m_aListeners.end( ) )
    {
        aIt = m_aListeners.insert(
                  m_aListeners.begin( ),
                  tListenerMap::value_type( aURL.Complete, new ::cppu::OInterfaceContainerHelper( m_aMutex ) ) );
    }

    OSL_ASSERT( aIt != m_aListeners.end( ) );

    aIt->second->addInterface( xControl );
    FireStatusEvent( aURL, xControl );
}

void SAL_CALL
BaseDispatch::removeStatusListener(
    const Reference< XStatusListener > &xControl,
    const URL &aURL )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlwc::BaseDispatch::removeStatusListener" );
    osl::MutexGuard aGuad( m_aMutex );

    tListenerMap::iterator aIt( m_aListeners.find( aURL.Complete ) );
    if ( aIt != m_aListeners.end( ) )
        ( *aIt ).second->removeInterface( xControl );
}

void SAL_CALL
BaseDispatch::disposing( )
{
    OSL_TRACE( "sbctlwc::BaseDispatch::disposing" );
    osl::MutexGuard aGuard( m_aMutex );

    lcl_DisposeAndClearAndDeleteAllMapElements( m_aListeners, static_cast < cppu::OWeakObject * > ( this ) );
    m_aListeners.clear( );
}

void
BaseDispatch::ThrowIfDisposed( ) throw ( RuntimeException )
{
    if ( rBHelper.bInDispose || rBHelper.bInDispose )
        throw DisposedException( );
}


void
BaseDispatch::FireStatusEvent()
{
    URL aURL;
    if ( m_xURLTransformer.is() )
    {
        aURL.Complete = GetCommand();
        m_xURLTransformer->parseStrict( aURL );
    }

    FireStatusEvent( aURL );
}

void
BaseDispatch::FireStatusEvent( const URL &rURL, const Reference< XStatusListener > &xControl )
{
    FeatureStateEvent aEvent = GetState( rURL );
    if ( xControl.is() )
        xControl->statusChanged( aEvent );
    else
    {
        tListenerMap::iterator aIt( m_aListeners.find( rURL.Complete ) );
        if ( aIt != m_aListeners.end() )
        {
            if ( aIt->second )
            {
                ::cppu::OInterfaceIteratorHelper aIntfIt( *( ( *aIt ).second ) );

                while ( aIntfIt.hasMoreElements() )
                {
                    Reference< XStatusListener > xListener( aIntfIt.next(), UNO_QUERY );
                    try
                    {
                        if ( xListener.is() )
                            xListener->statusChanged( aEvent );
                    }
                    catch ( ... )
                    {
                    }
                }
            }
        }
    }
}

void SAL_CALL BaseDispatch::dispatch(
    const URL &aURL, const Sequence< PropertyValue > &lArguments )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlwc::BaseDispatch::dispatch" );
    osl::MutexGuard aGuard( m_aMutex );
    ThrowIfDisposed();

    ExecuteCommand( aURL, lArguments );
}


void SAL_CALL BaseDispatch::modified(
    const ::com::sun::star::lang::EventObject &aEvent )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlwc::BaseDispatch::modified" );
    FireStatusEvent();
}


void SAL_CALL BaseDispatch::selectionChanged(
    const ::com::sun::star::lang::EventObject &aEvent )
throw ( ::com::sun::star::uno::RuntimeException )
{
    OSL_TRACE( "sbctlwc::BaseDispatch::selectionChanged" );
    FireStatusEvent();
}


void SAL_CALL BaseDispatch::disposing(
    const ::com::sun::star::lang::EventObject &aEvent )
throw ( ::com::sun::star::uno::RuntimeException )
{
    OSL_TRACE( "sbctlwc::BaseDispatch::disposing(aEvent)" );
}
