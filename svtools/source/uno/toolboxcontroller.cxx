/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <svtools/toolboxcontroller.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <svtools/imgdef.hxx>
#include <svtools/miscopt.hxx>
#include <toolkit/unohlp.hxx>
#include <vcl/toolbox.hxx>
#include <comphelper/processfactory.hxx>

const int TOOLBARCONTROLLER_PROPHANDLE_SUPPORTSVISIBLE  = 1;
const char TOOLBARCONTROLLER_PROPNAME_SUPPORTSVISIBLE[] = "SupportsVisible";

using ::rtl::OUString;

using namespace ::cppu;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame;

namespace svt
{

struct DispatchInfo
{
    Reference< XDispatch > mxDispatch;
    const URL maURL;
    const Sequence< PropertyValue > maArgs;

    DispatchInfo( const Reference< XDispatch >& xDispatch, const URL& rURL, const Sequence< PropertyValue >& rArgs )
        : mxDispatch( xDispatch ), maURL( rURL ), maArgs( rArgs ) {}
};

struct ToolboxController_Impl
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >          m_xParentWindow;
    ::com::sun::star::uno::Reference< ::com::sun::star::util::XURLTransformer > m_xUrlTransformer;
    rtl::OUString m_sModuleName;
     sal_uInt16 m_nToolBoxId;

    DECL_STATIC_LINK( ToolboxController_Impl, ExecuteHdl_Impl, DispatchInfo* );

    ToolboxController_Impl()
        : m_nToolBoxId( SAL_MAX_UINT16 )
    {}
};

ToolboxController::ToolboxController(

    const Reference< XMultiServiceFactory >& rServiceManager,
    const Reference< XFrame >& xFrame,
    const ::rtl::OUString& aCommandURL ) :
    OPropertyContainer(GetBroadcastHelper())
    ,   OWeakObject()
    ,   m_bSupportVisible(sal_False)
    ,   m_bInitialized( sal_False )
    ,   m_bDisposed( sal_False )
    ,   m_xFrame(xFrame)
    ,   m_xServiceManager( rServiceManager )
    ,   m_aCommandURL( aCommandURL )
    ,   m_aListenerContainer( m_aMutex )
{
    registerProperty(rtl::OUString(TOOLBARCONTROLLER_PROPNAME_SUPPORTSVISIBLE), TOOLBARCONTROLLER_PROPHANDLE_SUPPORTSVISIBLE, com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY,
        &m_bSupportVisible, getCppuType(&m_bSupportVisible));

    m_pImpl = new ToolboxController_Impl;

    try
    {
        m_pImpl->m_xUrlTransformer.set(
            ::com::sun::star::util::URLTransformer::create(
                ::comphelper::getComponentContext(m_xServiceManager) ) );
    }
    catch(const Exception&)
    {
    }
}

ToolboxController::ToolboxController() :
    OPropertyContainer(GetBroadcastHelper())
    ,   OWeakObject()
    ,   m_bSupportVisible(sal_False)
    ,   m_bInitialized( sal_False )
    ,   m_bDisposed( sal_False )
    ,   m_aListenerContainer( m_aMutex )
{
    registerProperty(rtl::OUString(TOOLBARCONTROLLER_PROPNAME_SUPPORTSVISIBLE), TOOLBARCONTROLLER_PROPHANDLE_SUPPORTSVISIBLE, com::sun::star::beans::PropertyAttribute::TRANSIENT | com::sun::star::beans::PropertyAttribute::READONLY,
        &m_bSupportVisible, getCppuType(&m_bSupportVisible));

    m_pImpl = new ToolboxController_Impl;
}

ToolboxController::~ToolboxController()
{
    delete m_pImpl;
}

Reference< XFrame > ToolboxController::getFrameInterface() const
{
    SolarMutexGuard aSolarMutexGuard;
    return m_xFrame;
}

Reference< XMultiServiceFactory > ToolboxController::getServiceManager() const
{
    SolarMutexGuard aSolarMutexGuard;
    return m_xServiceManager;
}

Reference< XLayoutManager > ToolboxController::getLayoutManager() const
{
    Reference< XLayoutManager > xLayoutManager;
    Reference< XPropertySet > xPropSet;
    {
        SolarMutexGuard aSolarMutexGuard;
        xPropSet = Reference< XPropertySet >( m_xFrame, UNO_QUERY );
    }

    if ( xPropSet.is() )
    {
        try
        {
            xLayoutManager.set(xPropSet->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" ))),UNO_QUERY);
        }
        catch ( Exception& )
        {
        }
    }

    return xLayoutManager;
}

// XInterface
Any SAL_CALL ToolboxController::queryInterface( const Type& rType )
throw ( RuntimeException )
{
    Any a = ::cppu::queryInterface(
                rType ,
                static_cast< XToolbarController* >( this ),
                static_cast< XStatusListener* >( this ),
                static_cast< XEventListener* >( this ),
                static_cast< XInitialization* >( this ),
                static_cast< XComponent* >( this ),
                static_cast< XUpdatable* >( this ));
    if ( !a.hasValue())
    {
        a = ::cppu::queryInterface(rType
            ,static_cast<XPropertySet*>(this)
            ,static_cast<XMultiPropertySet*>(this)
            ,static_cast<XFastPropertySet*>(this));
        if (!a.hasValue())
            return OWeakObject::queryInterface( rType );
    }
    return a;
}

void SAL_CALL ToolboxController::acquire() throw ()
{
    OWeakObject::acquire();
}

void SAL_CALL ToolboxController::release() throw ()
{
    OWeakObject::release();
}

void SAL_CALL ToolboxController::initialize( const Sequence< Any >& aArguments )
throw ( Exception, RuntimeException )
{
    bool bInitialized( true );

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( m_bDisposed )
            throw DisposedException();

        bInitialized = m_bInitialized;
    }

    if ( !bInitialized )
    {
        SolarMutexGuard aSolarMutexGuard;
        m_bInitialized = sal_True;
        //shizhoubo add
        m_bSupportVisible = sal_False;
        PropertyValue aPropValue;
        for ( int i = 0; i < aArguments.getLength(); i++ )
        {
            if ( aArguments[i] >>= aPropValue )
            {
                if ( aPropValue.Name == "Frame" )
                    m_xFrame.set(aPropValue.Value,UNO_QUERY);
                else if ( aPropValue.Name == "CommandURL" )
                    aPropValue.Value >>= m_aCommandURL;
                else if ( aPropValue.Name == "ServiceManager" )
                    m_xServiceManager.set(aPropValue.Value,UNO_QUERY);
                else if ( aPropValue.Name == "ParentWindow" )
                    m_pImpl->m_xParentWindow.set(aPropValue.Value,UNO_QUERY);
                else if ( aPropValue.Name == "ModuleName" )
                    aPropValue.Value >>= m_pImpl->m_sModuleName;
            }
        }

        try
        {
            if ( !m_pImpl->m_xUrlTransformer.is() && m_xServiceManager.is() )
                m_pImpl->m_xUrlTransformer.set(
                    ::com::sun::star::util::URLTransformer::create(
                        ::comphelper::getComponentContext(m_xServiceManager) ) );
        }
        catch(const Exception&)
        {
        }

        if ( !m_aCommandURL.isEmpty() )
            m_aListenerMap.insert( URLToDispatchMap::value_type( m_aCommandURL, Reference< XDispatch >() ));
    }
}

void SAL_CALL ToolboxController::update()
throw ( RuntimeException )
{
    {
        SolarMutexGuard aSolarMutexGuard;
        if ( m_bDisposed )
            throw DisposedException();
    }

    // Bind all registered listeners to their dispatch objects
    bindListener();
}

// XComponent
void SAL_CALL ToolboxController::dispose()
throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    {
        SolarMutexGuard aSolarMutexGuard;
        if ( m_bDisposed )
            throw DisposedException();
    }

    com::sun::star::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    SolarMutexGuard aSolarMutexGuard;
    Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
    URLToDispatchMap::iterator pIter = m_aListenerMap.begin();
    while ( pIter != m_aListenerMap.end() )
    {
        try
        {
            Reference< XDispatch > xDispatch( pIter->second );

            com::sun::star::util::URL aTargetURL;
            aTargetURL.Complete = pIter->first;
            if ( m_pImpl->m_xUrlTransformer.is() )
                m_pImpl->m_xUrlTransformer->parseStrict( aTargetURL );

            if ( xDispatch.is() && xStatusListener.is() )
                xDispatch->removeStatusListener( xStatusListener, aTargetURL );
        }
        catch ( Exception& )
        {
        }

        ++pIter;
    }

    m_bDisposed = sal_True;
}

void SAL_CALL ToolboxController::addEventListener( const Reference< XEventListener >& xListener )
throw ( RuntimeException )
{
    m_aListenerContainer.addInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), xListener );
}

void SAL_CALL ToolboxController::removeEventListener( const Reference< XEventListener >& aListener )
throw ( RuntimeException )
{
    m_aListenerContainer.removeInterface( ::getCppuType( ( const Reference< XEventListener >* ) NULL ), aListener );
}

// XEventListener
void SAL_CALL ToolboxController::disposing( const EventObject& Source )
throw ( RuntimeException )
{
    Reference< XInterface > xSource( Source.Source );

    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        return;

    URLToDispatchMap::iterator pIter = m_aListenerMap.begin();
    while ( pIter != m_aListenerMap.end() )
    {
        // Compare references and release dispatch references if they are equal.
        Reference< XInterface > xIfac( pIter->second, UNO_QUERY );
        if ( xSource == xIfac )
            pIter->second.clear();
        ++pIter;
    }

    Reference< XInterface > xIfac( m_xFrame, UNO_QUERY );
    if ( xIfac == xSource )
        m_xFrame.clear();
}

// XStatusListener
void SAL_CALL ToolboxController::statusChanged( const FeatureStateEvent& )
throw ( RuntimeException )
{
    // must be implemented by sub class
}

// XToolbarController
void SAL_CALL ToolboxController::execute( sal_Int16 KeyModifier )
throw (::com::sun::star::uno::RuntimeException)
{
    Reference< XDispatch >       xDispatch;
    ::rtl::OUString                     aCommandURL;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( m_bDisposed )
            throw DisposedException();

        if ( m_bInitialized &&
             m_xFrame.is() &&
             m_xServiceManager.is() &&
             !m_aCommandURL.isEmpty() )
        {

            aCommandURL = m_aCommandURL;
            URLToDispatchMap::iterator pIter = m_aListenerMap.find( m_aCommandURL );
            if ( pIter != m_aListenerMap.end() )
                xDispatch = pIter->second;
        }
    }

    if ( xDispatch.is() )
    {
        try
        {
            com::sun::star::util::URL aTargetURL;
            Sequence<PropertyValue>   aArgs( 1 );

            // Provide key modifier information to dispatch function
            aArgs[0].Name   = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "KeyModifier" ));
            aArgs[0].Value  = makeAny( KeyModifier );

            aTargetURL.Complete = aCommandURL;
            if ( m_pImpl->m_xUrlTransformer.is() )
                m_pImpl->m_xUrlTransformer->parseStrict( aTargetURL );
            xDispatch->dispatch( aTargetURL, aArgs );
        }
        catch ( DisposedException& )
        {
        }
    }
}

void SAL_CALL ToolboxController::click()
throw (::com::sun::star::uno::RuntimeException)
{
}

void SAL_CALL ToolboxController::doubleClick()
throw (::com::sun::star::uno::RuntimeException)
{
}

Reference< XWindow > SAL_CALL ToolboxController::createPopupWindow()
throw (::com::sun::star::uno::RuntimeException)
{
    return Reference< XWindow >();
}

Reference< XWindow > SAL_CALL ToolboxController::createItemWindow( const Reference< XWindow >& )
throw (::com::sun::star::uno::RuntimeException)
{
    return Reference< XWindow >();
}

void ToolboxController::addStatusListener( const rtl::OUString& aCommandURL )
{
    Reference< XDispatch >       xDispatch;
    Reference< XStatusListener > xStatusListener;
    com::sun::star::util::URL    aTargetURL;

    {
        SolarMutexGuard aSolarMutexGuard;
        URLToDispatchMap::iterator pIter = m_aListenerMap.find( aCommandURL );

        // Already in the list of status listener. Do nothing.
        if ( pIter != m_aListenerMap.end() )
            return;

        // Check if we are already initialized. Implementation starts adding itself as status listener when
        // intialize is called.
        if ( !m_bInitialized )
        {
            // Put into the boost::unordered_map of status listener. Will be activated when initialized is called
            m_aListenerMap.insert( URLToDispatchMap::value_type( aCommandURL, Reference< XDispatch >() ));
            return;
        }
        else
        {
            // Add status listener directly as intialize has already been called.
            Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
            if ( m_xServiceManager.is() && xDispatchProvider.is() )
            {
                aTargetURL.Complete = aCommandURL;
                if ( m_pImpl->m_xUrlTransformer.is() )
                    m_pImpl->m_xUrlTransformer->parseStrict( aTargetURL );
                xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );

                xStatusListener = Reference< XStatusListener >( static_cast< OWeakObject* >( this ), UNO_QUERY );
                URLToDispatchMap::iterator aIter = m_aListenerMap.find( aCommandURL );
                if ( aIter != m_aListenerMap.end() )
                {
                    Reference< XDispatch > xOldDispatch( aIter->second );
                    aIter->second = xDispatch;

                    try
                    {
                        if ( xOldDispatch.is() )
                            xOldDispatch->removeStatusListener( xStatusListener, aTargetURL );
                    }
                    catch ( Exception& )
                    {
                    }
                }
                else
                    m_aListenerMap.insert( URLToDispatchMap::value_type( aCommandURL, xDispatch ));
            }
        }
    }

    // Call without locked mutex as we are called back from dispatch implementation
    try
    {
        if ( xDispatch.is() )
            xDispatch->addStatusListener( xStatusListener, aTargetURL );
    }
    catch ( Exception& )
    {
    }
}

void ToolboxController::removeStatusListener( const rtl::OUString& aCommandURL )
{
    SolarMutexGuard aSolarMutexGuard;

    URLToDispatchMap::iterator pIter = m_aListenerMap.find( aCommandURL );
    if ( pIter != m_aListenerMap.end() )
    {
        Reference< XDispatch > xDispatch( pIter->second );
        Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
        m_aListenerMap.erase( pIter );

        try
        {
            com::sun::star::util::URL aTargetURL;
            aTargetURL.Complete = aCommandURL;
            if ( m_pImpl->m_xUrlTransformer.is() )
                m_pImpl->m_xUrlTransformer->parseStrict( aTargetURL );

            if ( xDispatch.is() && xStatusListener.is() )
                xDispatch->removeStatusListener( xStatusListener, aTargetURL );
        }
        catch ( Exception& )
        {
        }
    }
}

void ToolboxController::bindListener()
{
    std::vector< Listener > aDispatchVector;
    Reference< XStatusListener > xStatusListener;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( !m_bInitialized )
            return;

        // Collect all registered command URL's and store them temporary
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        if ( m_xServiceManager.is() && xDispatchProvider.is() )
        {
            xStatusListener = Reference< XStatusListener >( static_cast< OWeakObject* >( this ), UNO_QUERY );
            URLToDispatchMap::iterator pIter = m_aListenerMap.begin();
            while ( pIter != m_aListenerMap.end() )
            {
                com::sun::star::util::URL aTargetURL;
                aTargetURL.Complete = pIter->first;
                if ( m_pImpl->m_xUrlTransformer.is() )
                    m_pImpl->m_xUrlTransformer->parseStrict( aTargetURL );

                Reference< XDispatch > xDispatch( pIter->second );
                if ( xDispatch.is() )
                {
                    // We already have a dispatch object => we have to requery.
                    // Release old dispatch object and remove it as listener
                    try
                    {
                        xDispatch->removeStatusListener( xStatusListener, aTargetURL );
                    }
                    catch ( Exception& )
                    {
                    }
                }

                pIter->second.clear();
                xDispatch.clear();

                // Query for dispatch object. Old dispatch will be released with this, too.
                try
                {
                    xDispatch = xDispatchProvider->queryDispatch( aTargetURL, ::rtl::OUString(), 0 );
                }
                catch ( Exception& )
                {
                }
                pIter->second = xDispatch;

                Listener aListener( aTargetURL, xDispatch );
                aDispatchVector.push_back( aListener );
                ++pIter;
            }
        }
    }

    // Call without locked mutex as we are called back from dispatch implementation
    if ( xStatusListener.is() )
    {
        try
        {
            for ( sal_uInt32 i = 0; i < aDispatchVector.size(); i++ )
            {
                Listener& rListener = aDispatchVector[i];
                if ( rListener.xDispatch.is() )
                    rListener.xDispatch->addStatusListener( xStatusListener, rListener.aURL );
                else if ( rListener.aURL.Complete == m_aCommandURL )
                {
                    try
                    {
                        // Send status changed for the main URL, if we cannot get a valid dispatch object.
                        // UI disables the button. Catch exception as we release our mutex, it is possible
                        // that someone else already disposed this instance!
                        FeatureStateEvent aFeatureStateEvent;
                        aFeatureStateEvent.IsEnabled = sal_False;
                        aFeatureStateEvent.FeatureURL = rListener.aURL;
                        aFeatureStateEvent.State = Any();
                        xStatusListener->statusChanged( aFeatureStateEvent );
                    }
                    catch ( Exception& )
                    {
                    }
                }
            }
        }
        catch ( Exception& )
        {
        }
    }
}

void ToolboxController::unbindListener()
{
    SolarMutexGuard aSolarMutexGuard;

    if ( !m_bInitialized )
        return;

    // Collect all registered command URL's and store them temporary
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    if ( m_xServiceManager.is() && xDispatchProvider.is() )
    {
        Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
        URLToDispatchMap::iterator pIter = m_aListenerMap.begin();
        while ( pIter != m_aListenerMap.end() )
        {
            com::sun::star::util::URL aTargetURL;
            aTargetURL.Complete = pIter->first;
            if ( m_pImpl->m_xUrlTransformer.is() )
                m_pImpl->m_xUrlTransformer->parseStrict( aTargetURL );

            Reference< XDispatch > xDispatch( pIter->second );
            if ( xDispatch.is() )
            {
                // We already have a dispatch object => we have to requery.
                // Release old dispatch object and remove it as listener
                try
                {
                    xDispatch->removeStatusListener( xStatusListener, aTargetURL );
                }
                catch ( Exception& )
                {
                }
            }
            pIter->second.clear();
            ++pIter;
        }
    }
}

sal_Bool ToolboxController::isBound() const
{
    SolarMutexGuard aSolarMutexGuard;

    if ( !m_bInitialized )
        return sal_False;

    URLToDispatchMap::const_iterator pIter = m_aListenerMap.find( m_aCommandURL );
    if ( pIter != m_aListenerMap.end() )
        return ( pIter->second.is() );

    return sal_False;
}

sal_Bool ToolboxController::hasBigImages() const
{
    return SvtMiscOptions().AreCurrentSymbolsLarge();
}

void ToolboxController::updateStatus()
{
    bindListener();
}

void ToolboxController::updateStatus( const rtl::OUString aCommandURL )
{
    Reference< XDispatch > xDispatch;
    Reference< XStatusListener > xStatusListener;
    com::sun::star::util::URL aTargetURL;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( !m_bInitialized )
            return;

        // Try to find a dispatch object for the requested command URL
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        xStatusListener = Reference< XStatusListener >( static_cast< OWeakObject* >( this ), UNO_QUERY );
        if ( m_xServiceManager.is() && xDispatchProvider.is() )
        {
            aTargetURL.Complete = aCommandURL;
            if ( m_pImpl->m_xUrlTransformer.is() )
                m_pImpl->m_xUrlTransformer->parseStrict( aTargetURL );
            xDispatch = xDispatchProvider->queryDispatch( aTargetURL, rtl::OUString(), 0 );
        }
    }

    if ( xDispatch.is() && xStatusListener.is() )
    {
        // Catch exception as we release our mutex, it is possible that someone else
        // has already disposed this instance!
        // Add/remove status listener to get a update status information from the
        // requested command.
        try
        {
            xDispatch->addStatusListener( xStatusListener, aTargetURL );
            xDispatch->removeStatusListener( xStatusListener, aTargetURL );
        }
        catch ( Exception& )
        {
        }
    }
}

Reference< XURLTransformer > ToolboxController::getURLTransformer() const
{
    return m_pImpl->m_xUrlTransformer;
}

Reference< ::com::sun::star::awt::XWindow > ToolboxController::getParent() const
{
    return m_pImpl->m_xParentWindow;
}

const rtl::OUString& ToolboxController::getModuleName() const
{
    return m_pImpl->m_sModuleName;
}

void ToolboxController::dispatchCommand( const OUString& sCommandURL, const Sequence< PropertyValue >& rArgs )
{
    try
    {
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY_THROW );
        URL aURL;
        aURL.Complete = sCommandURL;
        getURLTransformer()->parseStrict( aURL );

        Reference< XDispatch > xDispatch( xDispatchProvider->queryDispatch( aURL, OUString(), 0 ), UNO_QUERY_THROW );

        Application::PostUserEvent( STATIC_LINK(0, ToolboxController_Impl, ExecuteHdl_Impl), new DispatchInfo( xDispatch, aURL, rArgs ) );

    }
    catch( Exception& )
    {
    }
}

//
//-------------------------------------------------------------------------
// XPropertySet by shizhoubo
com::sun::star::uno::Reference< com::sun::star::beans::XPropertySetInfo >  SAL_CALL ToolboxController::getPropertySetInfo() throw(::com::sun::star::uno::RuntimeException)
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}
//-------------------------------------------------------------------------
::cppu::IPropertyArrayHelper& ToolboxController::getInfoHelper()
{
        return *const_cast<ToolboxController*>(this)->getArrayHelper();
}
//OPropertyArrayUsageHelper by shizhoubo
//------------------------------------------------------------------------------
::cppu::IPropertyArrayHelper* ToolboxController::createArrayHelper( ) const
{
        com::sun::star::uno::Sequence< Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
}
//shizhoubo for supportsvisiable
void ToolboxController::setSupportVisibleProperty(sal_Bool bValue)
{
    m_bSupportVisible = bValue;
}
//OPropertySetHelper by shizhoubo
sal_Bool SAL_CALL ToolboxController::convertFastPropertyValue( com::sun::star::uno::Any&    aConvertedValue ,
                                             com::sun::star::uno::Any&        aOldValue       ,
                                             sal_Int32                        nHandle         ,
                                             const com::sun::star::uno::Any&  aValue          ) throw( com::sun::star::lang::IllegalArgumentException )
{
    switch (nHandle)
    {
        case TOOLBARCONTROLLER_PROPHANDLE_SUPPORTSVISIBLE:
        {
            sal_Bool aNewValue(sal_False);
            aValue >>= aNewValue;
            if (aNewValue != m_bSupportVisible)
            {
                aConvertedValue <<= aNewValue;
                aOldValue <<= m_bSupportVisible;
                return sal_True;
            }
            return sal_False;
        }
    }
    return OPropertyContainer::convertFastPropertyValue(aConvertedValue, aOldValue, nHandle, aValue);
}

void SAL_CALL ToolboxController::setFastPropertyValue_NoBroadcast(
    sal_Int32                       nHandle,
    const com::sun::star::uno::Any& aValue )
throw( com::sun::star::uno::Exception)
{
    OPropertyContainer::setFastPropertyValue_NoBroadcast(nHandle, aValue);
    if (TOOLBARCONTROLLER_PROPHANDLE_SUPPORTSVISIBLE == nHandle)
    {
        sal_Bool rValue(sal_False);
        if (( aValue >>= rValue ) && m_bInitialized)
            this->setSupportVisibleProperty( rValue );
    }
}

//--------------------------------------------------------------------

IMPL_STATIC_LINK_NOINSTANCE( ToolboxController_Impl, ExecuteHdl_Impl, DispatchInfo*, pDispatchInfo )
{
    pDispatchInfo->mxDispatch->dispatch( pDispatchInfo->maURL, pDispatchInfo->maArgs );
    delete pDispatchInfo;
    return 0;
}

void ToolboxController::enable( bool bEnable )
{
    ToolBox* pToolBox = 0;
    sal_uInt16 nItemId = 0;
    if( getToolboxId( nItemId, &pToolBox ) )
    {
        pToolBox->EnableItem( nItemId, bEnable ? sal_True : sal_False );
    }
}

bool ToolboxController::getToolboxId( sal_uInt16& rItemId, ToolBox** ppToolBox )
{
    if( (m_pImpl->m_nToolBoxId != SAL_MAX_UINT16) && (ppToolBox == 0) )
        return m_pImpl->m_nToolBoxId;

    ToolBox* pToolBox = static_cast< ToolBox* >( VCLUnoHelper::GetWindow( getParent() ) );

    if( (m_pImpl->m_nToolBoxId == SAL_MAX_UINT16) && pToolBox )
    {
        const sal_uInt16 nCount = pToolBox->GetItemCount();
        for ( sal_uInt16 nPos = 0; nPos < nCount; ++nPos )
        {
            const sal_uInt16 nItemId = pToolBox->GetItemId( nPos );
            if ( pToolBox->GetItemCommand( nItemId ) == String( m_aCommandURL ) )
            {
                m_pImpl->m_nToolBoxId = nItemId;
                break;
            }
        }
    }

    if( ppToolBox )
        *ppToolBox = pToolBox;

    rItemId = m_pImpl->m_nToolBoxId;

    return (rItemId != SAL_MAX_UINT16) && (( ppToolBox == 0) || (*ppToolBox != 0) );
}
//end

} // svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
