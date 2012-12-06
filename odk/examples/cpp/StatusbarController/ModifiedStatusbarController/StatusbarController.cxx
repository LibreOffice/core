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

#include "StatusbarController.hxx"
#include "defines.hxx"

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/ui/ItemStyle.hpp>
#include <com/sun/star/ucb/AlreadyInitializedException.hpp>

#include <functional>
#include <algorithm>

using namespace framework::statusbar_controller_modified;

using namespace com::sun::star;
using namespace com::sun::star::ui;
using namespace com::sun::star::util;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;
using namespace com::sun::star::frame;
using namespace com::sun::star::beans;

using rtl::OUString;
using com::sun::star::ucb::AlreadyInitializedException;

namespace
{
    template< class MAP >
    struct lcl_StatusListenerRemove
            : public std::unary_function< typename MAP::value_type &, void >
    {
        private:
            Reference< XStatusListener > xListener;
            Reference< XURLTransformer > xURLTransformer;
        public:
            explicit lcl_StatusListenerRemove(
                const Reference< XStatusListener > &rxListener,
                const Reference< XURLTransformer > &rxTransformer )
                : xListener( rxListener )
                , xURLTransformer( rxTransformer ) {}

            void operator()( typename MAP::value_type &rPair ) const
            {
                if ( !rPair.second.is() )
                    return;
                try
                {
                    URL aURL;
                    aURL.Complete = rPair.first;
                    xURLTransformer->parseStrict( aURL );
                    rPair.second->removeStatusListener( xListener, aURL );
                }
                catch ( ... )
                {
                }

                rPair.second.clear();
            }
    };
}


StatusbarController::StatusbarController()
    : StatusbarController_Base( m_aMutex )
    , m_bInitialized( false )
    , m_bOwnerDraw( false )
{
    OSL_TRACE( "sbctlmodif::StatusbarController::StatusbarController" );
}

StatusbarController::~StatusbarController()
{
    OSL_TRACE( "sbctlmodif::StatusbarController::~StatusbarController" );
}

void SAL_CALL
StatusbarController::dispose()
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlmodif::StatusbarController::dispose" );
    osl::ClearableMutexGuard aGuard( m_aMutex );
    ThrowIfDisposed();

    UnbindListeners();

    m_xFrame.clear();
    m_xStatusbarWindow.clear();
    m_xStatusbarItem.clear();
    m_xURLTransformer.clear();
    m_aListenerMap.clear();
    aGuard.clear();

    StatusbarController_Base::dispose();
}

void SAL_CALL
StatusbarController::initialize(
    const Sequence< Any > &aArguments )
throw ( Exception, RuntimeException )
{
    OSL_TRACE( "sbctlmodif::StatusbarController::initialize" );
    bool bInitialized( true );

    osl::ResettableMutexGuard aGuard( m_aMutex );
    bInitialized = m_bInitialized;
    aGuard.clear();

    if ( bInitialized )
        throw AlreadyInitializedException();

    Reference< XFrame > xFrame;
    Reference< XWindow > xStatusbarWindow;
    Reference< XStatusbarItem > xStatusbarItem;
    OUString sCommand, sModuleId;
    bool bOwnerDraw( false );

    PropertyValue aPropVal;
    for ( const Any *pAny = aArguments.getConstArray(),
            *pEnd = pAny + aArguments.getLength();
            pAny != pEnd;
            pAny++ )
    {
        if ( *pAny >>= aPropVal )
        {
            if ( aPropVal.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "Frame" ) ) )
                aPropVal.Value >>= xFrame;
            else if ( aPropVal.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "CommandURL" ) ) )
                aPropVal.Value >>= sCommand;
            else if ( aPropVal.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ParentWindow" ) ) )
                aPropVal.Value >>= xStatusbarWindow;
            else if ( aPropVal.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "ModuleName" ) ) )
                aPropVal.Value >>= sModuleId;
            else if ( aPropVal.Name.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM( "StatusbarItem" ) ) )
                aPropVal.Value >>= xStatusbarItem;
        }
    }

    if ( ( xStatusbarItem->getStyle() & ItemStyle::OWNER_DRAW ) == ItemStyle::OWNER_DRAW )
        bOwnerDraw = true;

    aGuard.reset();
    m_bInitialized = true;
    m_xFrame = xFrame;
    m_xStatusbarItem = xStatusbarItem;
    m_xStatusbarWindow = xStatusbarWindow;
    m_sModuleIdentifier = sModuleId;
    m_sCommandURL = sCommand;
    m_bOwnerDraw = bOwnerDraw;
    if ( m_sCommandURL.getLength() )
        m_aListenerMap.insert( URLToDispatchMap::value_type( m_sCommandURL, Reference< XDispatch >() ) );
    aGuard.clear();
}

void StatusbarController::BindListeners()
{
    OSL_TRACE( "sbctlmodif::StatusbarController::BindListeners" );
    std::vector< Listener > aListeners;
    Reference< XStatusListener > xStatusListener;
    OUString sOwnCommandURL;

    osl::ClearableMutexGuard aGuard( m_aMutex );
    if ( !m_bInitialized )
        return;

    sOwnCommandURL = m_sCommandURL;
    xStatusListener.set( static_cast< cppu::OWeakObject *>( this ), UNO_QUERY );
    Reference< XDispatchProvider > xProvider( m_xFrame, UNO_QUERY );
    if ( xProvider.is() )
    {
        for ( URLToDispatchMap::iterator it = m_aListenerMap.begin();
                it != m_aListenerMap.end(); it++ )
        {
            Reference< XURLTransformer > xURLTransformer = GetURLTransformer();
            URL aURL;
            aURL.Complete = it->first;
            xURLTransformer->parseStrict( aURL );

            // remove
            if ( it->second.is() )
            {
                try
                {
                    it->second->removeStatusListener( xStatusListener, aURL );
                }
                catch ( ... ) {}

                it->second.clear();
            }

            // requery
            try
            {
                it->second = xProvider->queryDispatch( aURL, OUString(), 0 );
            }
            catch ( ... ) {}

            aListeners.push_back( Listener( aURL, it->second ) );
        }
    }
    aGuard.clear();

    if ( !xStatusListener.is() || aListeners.empty() )
        return;

    for ( std::vector< Listener >::const_iterator it = aListeners.begin();
            it != aListeners.end(); it++ )
    {
        try
        {
            if ( it->xDispatch.is() )
                it->xDispatch->addStatusListener( xStatusListener, it->aURL );
            else if ( it->aURL.Complete.equals( sOwnCommandURL ) )
            {
                // no dispatch for our own command,
                // send an update with the Feature disabled
                FeatureStateEvent aFeatureState;
                aFeatureState.FeatureURL = it->aURL;
                aFeatureState.IsEnabled = sal_False;
                aFeatureState.Source = xStatusListener;

                xStatusListener->statusChanged( aFeatureState );
            }
        }
        catch ( ... )
        {}
    }

}

void StatusbarController::UnbindListeners()
{
    OSL_TRACE( "sbctlmodif::StatusbarController::UnbindListeners" );

    Reference< XStatusListener > xStatusListener( static_cast< cppu::OWeakObject * >( this ), UNO_QUERY );
    std::for_each( m_aListenerMap.begin(),
                   m_aListenerMap.end(),
                   lcl_StatusListenerRemove< URLToDispatchMap >( xStatusListener, GetURLTransformer() ) );
}

bool StatusbarController::IsBound() const
{
    OSL_TRACE( "sbctlmodif::StatusbarController::IsBound" );
    osl::MutexGuard aGuard( m_aMutex );
    if ( !m_bInitialized )
        return false;

    const URLToDispatchMap::const_iterator it = m_aListenerMap.find( m_sCommandURL );
    return it != m_aListenerMap.end() && it->second.is();
}

Reference< XURLTransformer > StatusbarController::GetURLTransformer()
{
    osl::MutexGuard aGuard( m_aMutex );
    if ( !m_xURLTransformer.is() && m_xContext.is() )
    {
        try
        {
            m_xURLTransformer.set(
                m_xContext->getServiceManager()->createInstanceWithContext(
                    OUSTR_SERVICENAME_URLTRANSFORMER, m_xContext ), UNO_QUERY_THROW );
        }
        catch ( ... )
        {}
    }

    return m_xURLTransformer;
}

void
StatusbarController::ThrowIfDisposed( ) throw ( RuntimeException )
{
    if ( rBHelper.bDisposed || rBHelper.bInDispose )
        throw lang::DisposedException( );
}

void StatusbarController::Execute(
    const Sequence< PropertyValue > &aArgs )
{
    OSL_TRACE( "sbctlmodif::StatusbarController::Execute" );
    Reference< XDispatch > xDispatch;
    Reference< XURLTransformer > xURLParser;
    OUString sCommandURL;

    osl::ClearableMutexGuard aGuard( m_aMutex );
    ThrowIfDisposed();
    if ( m_bInitialized && m_sCommandURL.getLength() )
    {
        sCommandURL = m_sCommandURL;
        xURLParser = GetURLTransformer();
        const URLToDispatchMap::const_iterator it = m_aListenerMap.find( m_sCommandURL );
        if ( it != m_aListenerMap.end() )
            xDispatch = it->second;
    }
    aGuard.clear();

    if ( xDispatch.is() && xURLParser.is() )
    {
        try
        {
            URL aURL;
            aURL.Complete = sCommandURL;
            xURLParser->parseStrict( aURL );
            xDispatch->dispatch( aURL, aArgs );
        }
        catch ( ... )
        {}
    }
}

void StatusbarController::Execute(
    const OUString &rCommandURL,
    const Sequence< PropertyValue > &aArgs )
{
    OSL_TRACE( "sbctlmodif::StatusbarController::Execute" );
    Reference< XDispatch > xDispatch;
    URL aURL;
    aURL.Complete = rCommandURL;

    osl::ClearableMutexGuard aGuard( m_aMutex );
    ThrowIfDisposed();
    if ( m_bInitialized && rCommandURL.getLength() )
    {
        Reference< XURLTransformer > xURLParser = GetURLTransformer();
        xURLParser->parseStrict( aURL );

        const URLToDispatchMap::const_iterator it = m_aListenerMap.find( rCommandURL );
        if ( it != m_aListenerMap.end() )
            xDispatch = it->second;

        if ( !xDispatch.is() )
        {
            try
            {
                Reference< XDispatchProvider > xProvider( m_xFrame, UNO_QUERY_THROW );
                xDispatch = xProvider->queryDispatch( aURL, OUString(), 0 );
            }
            catch ( ... )
            {}
        }
    }
    aGuard.clear();

    if ( xDispatch.is() )
    {
        try
        {
            xDispatch->dispatch( aURL, aArgs );
        }
        catch ( ... )
        {}
    }
}

void StatusbarController::UpdateStatus()
{
    BindListeners();
}

void StatusbarController::UpdateStatus( const OUString &rCommand )
{
    OSL_TRACE( "sbctlmodif::StatusbarController::UpdateStatus" );
    Reference< XStatusListener > xStatusListener;
    Reference< XDispatch > xDispatch;
    URL aURL;
    aURL.Complete = rCommand;

    osl::ClearableMutexGuard aGuard( m_aMutex );
    Reference< XDispatchProvider > xProvider( m_xFrame, UNO_QUERY );
    Reference< XURLTransformer > xURLParser = GetURLTransformer();
    xStatusListener.set( static_cast< cppu::OWeakObject * >( this ), UNO_QUERY );
    if ( !xProvider.is() || !xURLParser.is() )
        return;
    try
    {
        xURLParser->parseStrict( aURL );
        xDispatch = xProvider->queryDispatch( aURL, OUString(), 0 );
    }
    catch ( ... )
    {}
    aGuard.clear();

    if ( !xStatusListener.is() || !xDispatch.is() )
        return;

    try
    {
        xDispatch->addStatusListener( xStatusListener, aURL );
        xDispatch->removeStatusListener( xStatusListener, aURL );
    }
    catch ( ... )
    {}
}

void SAL_CALL
StatusbarController::statusChanged(
    const FeatureStateEvent &aFeatureState )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlmodif::StatusbarController::statusChanged" );
    osl::ClearableMutexGuard aGuard( m_aMutex );

    if ( rBHelper.bInDispose || rBHelper.bDisposed )
        return;

    if ( aFeatureState.FeatureURL.Complete.equals( m_sCommandURL ) )
    {
        OUString aStrVal;
        if ( aFeatureState.State >>= aStrVal )
        {
            m_xStatusbarItem->setText( aStrVal );
        }
        else if ( !aFeatureState.State.hasValue() && !aFeatureState.IsEnabled )
            m_xStatusbarItem->setText( OUString() );
    }
}

void SAL_CALL
StatusbarController::disposing(
    const lang::EventObject &Source )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlmodif::StatusbarController::disposing" );
    osl::MutexGuard aGuard( m_aMutex );

    if ( rBHelper.bInDispose || rBHelper.bDisposed )
        return;

    Reference< XFrame > xFrame( Source.Source, UNO_QUERY );
    if ( xFrame.is() )
    {
        if ( xFrame == m_xFrame )
            m_xFrame.clear();
        return;
    }

    Reference< XDispatch > xDispatch( Source.Source, UNO_QUERY );
    if ( !xDispatch.is() )
        return;

    URLToDispatchMap::iterator pIter = m_aListenerMap.begin();
    while ( pIter != m_aListenerMap.end() )
    {
        // Compare references and release dispatch references if they are equal.
        if ( xDispatch == pIter->second )
            pIter->second.clear();
        pIter++;
    }
}

void SAL_CALL
StatusbarController::update()
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlmodif::StatusbarController::update" );
    osl::ClearableMutexGuard aGuard( m_aMutex );
    ThrowIfDisposed();
    aGuard.clear();

    BindListeners();
}

::sal_Bool SAL_CALL
StatusbarController::mouseButtonDown( const MouseEvent &/*aMouseEvent*/ )
throw ( RuntimeException )
{
    return sal_False;
}

::sal_Bool SAL_CALL
StatusbarController::mouseMove( const MouseEvent &/*aMouseEvent*/ )
throw ( RuntimeException )
{
    return sal_False;
}

::sal_Bool SAL_CALL
StatusbarController::mouseButtonUp( const MouseEvent &/*aMouseEvent*/ )
throw ( RuntimeException )
{
    return sal_False;
}

void SAL_CALL
StatusbarController::command(
    const Point &/*aPos*/,
    ::sal_Int32 /*nCommand*/,
    ::sal_Bool /*bMouseEvent*/,
    const Any &/*aData*/ )
throw ( RuntimeException )
{
}

void SAL_CALL
StatusbarController::paint(
    const Reference< XGraphics > &/*xGraphics*/,
    const Rectangle &/*rOutputRectangle*/,
    ::sal_Int32 /*nStyle*/ )
throw ( RuntimeException )
{
}

void SAL_CALL
StatusbarController::click( const ::com::sun::star::awt::Point &/*aPos*/ )
throw ( RuntimeException )
{
}

void SAL_CALL
StatusbarController::doubleClick( const ::com::sun::star::awt::Point &/*aPos*/ )
throw ( RuntimeException )
{
    OSL_TRACE( "sbctlmodif::StatusbarController::doubleClick" );
    osl::ClearableMutexGuard aGuard( m_aMutex );
    if ( rBHelper.bDisposed || rBHelper.bInDispose )
        return;
    aGuard.clear();

    Sequence< PropertyValue > aDummyArgs;
    Execute( aDummyArgs );
}
