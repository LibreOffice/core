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

#include <svtools/toolboxcontroller.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/frame/XLayoutManager.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <vcl/svapp.hxx>
#include <svtools/imgdef.hxx>
#include <svtools/miscopt.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/toolbox.hxx>
#include <comphelper/processfactory.hxx>

const int TOOLBARCONTROLLER_PROPHANDLE_SUPPORTSVISIBLE  = 1;
const char TOOLBARCONTROLLER_PROPNAME_SUPPORTSVISIBLE[] = "SupportsVisible";


using namespace ::cppu;
using namespace css::awt;
using namespace css::uno;
using namespace css::util;
using namespace css::beans;
using namespace css::lang;
using namespace css::frame;

namespace svt
{

ToolboxController::ToolboxController(
    const Reference< XComponentContext >& rxContext,
    const Reference< XFrame >& xFrame,
    const OUString& aCommandURL ) :
    OPropertyContainer( GetBroadcastHelper() )
    ,   m_bSupportVisible( false )
    ,   m_bInitialized( false )
    ,   m_bDisposed( false )
    ,   m_nToolBoxId( SAL_MAX_UINT16 )
    ,   m_xFrame( xFrame )
    ,   m_xContext( rxContext )
    ,   m_aCommandURL( aCommandURL )
    ,   m_aListenerContainer( m_aMutex )
{
    OSL_ASSERT( m_xContext.is() );
    registerProperty( TOOLBARCONTROLLER_PROPNAME_SUPPORTSVISIBLE,
        TOOLBARCONTROLLER_PROPHANDLE_SUPPORTSVISIBLE,
        css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY,
        &m_bSupportVisible, cppu::UnoType<decltype(m_bSupportVisible)>::get());

    try
    {
        m_xUrlTransformer = URLTransformer::create( rxContext );
    }
    catch(const Exception&)
    {
    }
}

ToolboxController::ToolboxController() :
    OPropertyContainer(GetBroadcastHelper())
    ,   m_bSupportVisible(false)
    ,   m_bInitialized( false )
    ,   m_bDisposed( false )
    ,   m_nToolBoxId( SAL_MAX_UINT16 )
    ,   m_aListenerContainer( m_aMutex )
{
    registerProperty( TOOLBARCONTROLLER_PROPNAME_SUPPORTSVISIBLE,
        TOOLBARCONTROLLER_PROPHANDLE_SUPPORTSVISIBLE,
        css::beans::PropertyAttribute::TRANSIENT | css::beans::PropertyAttribute::READONLY,
        &m_bSupportVisible, cppu::UnoType<decltype(m_bSupportVisible)>::get());
}

ToolboxController::~ToolboxController()
{
}

Reference< XFrame > ToolboxController::getFrameInterface() const
{
    SolarMutexGuard aSolarMutexGuard;
    return m_xFrame;
}

const Reference< XComponentContext > & ToolboxController::getContext() const
{
    SolarMutexGuard aSolarMutexGuard;
    return m_xContext;
}

Reference< XLayoutManager > ToolboxController::getLayoutManager() const
{
    Reference< XLayoutManager > xLayoutManager;
    Reference< XPropertySet > xPropSet;
    {
        SolarMutexGuard aSolarMutexGuard;
        xPropSet.set( m_xFrame, UNO_QUERY );
    }

    if ( xPropSet.is() )
    {
        try
        {
            xLayoutManager.set(xPropSet->getPropertyValue("LayoutManager"),UNO_QUERY);
        }
        catch ( Exception& )
        {
        }
    }

    return xLayoutManager;
}

// XInterface
Any SAL_CALL ToolboxController::queryInterface( const Type& rType )
{
    css::uno::Any a(ToolboxController_Base::queryInterface(rType));
    return a.hasValue() ? a : OPropertyContainer::queryInterface(rType);
}

void SAL_CALL ToolboxController::acquire() throw ()
{
    ToolboxController_Base::acquire();
}

void SAL_CALL ToolboxController::release() throw ()
{
    ToolboxController_Base::release();
}

css::uno::Sequence<css::uno::Type> ToolboxController::getTypes()
{
    return comphelper::concatSequences(ToolboxController_Base::getTypes(),
                getBaseTypes());
}

void SAL_CALL ToolboxController::initialize( const Sequence< Any >& aArguments )
{
    bool bInitialized( true );

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( m_bDisposed )
            throw DisposedException();

        bInitialized = m_bInitialized;
    }

    if ( bInitialized )
        return;

    SolarMutexGuard aSolarMutexGuard;
    m_bInitialized = true;
    m_bSupportVisible = false;
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
            {
                Reference<XMultiServiceFactory> xMSF(aPropValue.Value, UNO_QUERY);
                if (xMSF.is())
                    m_xContext = comphelper::getComponentContext(xMSF);
            }
            else if ( aPropValue.Name == "ParentWindow" )
                m_xParentWindow.set(aPropValue.Value,UNO_QUERY);
            else if ( aPropValue.Name == "ModuleIdentifier" )
                aPropValue.Value >>= m_sModuleName;
            else if ( aPropValue.Name == "Identifier" )
                aPropValue.Value >>= m_nToolBoxId;
        }
    }

    try
    {
        if ( !m_xUrlTransformer.is() && m_xContext.is() )
            m_xUrlTransformer = URLTransformer::create( m_xContext );
    }
    catch(const Exception&)
    {
    }

    if ( !m_aCommandURL.isEmpty() )
        m_aListenerMap.emplace( m_aCommandURL, Reference< XDispatch >() );
}

void SAL_CALL ToolboxController::update()
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
{
    Reference< XComponent > xThis( static_cast< OWeakObject* >(this), UNO_QUERY );

    {
        SolarMutexGuard aSolarMutexGuard;
        if ( m_bDisposed )
            throw DisposedException();
    }

    css::lang::EventObject aEvent( xThis );
    m_aListenerContainer.disposeAndClear( aEvent );

    SolarMutexGuard aSolarMutexGuard;
    Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
    for (auto const& listener : m_aListenerMap)
    {
        try
        {
            Reference< XDispatch > xDispatch( listener.second );

            css::util::URL aTargetURL;
            aTargetURL.Complete = listener.first;
            if ( m_xUrlTransformer.is() )
                m_xUrlTransformer->parseStrict( aTargetURL );

            if ( xDispatch.is() && xStatusListener.is() )
                xDispatch->removeStatusListener( xStatusListener, aTargetURL );
        }
        catch ( Exception& )
        {
        }

    }

    m_bDisposed = true;
}

void SAL_CALL ToolboxController::addEventListener( const Reference< XEventListener >& xListener )
{
    m_aListenerContainer.addInterface( cppu::UnoType<XEventListener>::get(), xListener );
}

void SAL_CALL ToolboxController::removeEventListener( const Reference< XEventListener >& aListener )
{
    m_aListenerContainer.removeInterface( cppu::UnoType<XEventListener>::get(), aListener );
}

// XEventListener
void SAL_CALL ToolboxController::disposing( const EventObject& Source )
{
    Reference< XInterface > xSource( Source.Source );

    SolarMutexGuard aSolarMutexGuard;

    if ( m_bDisposed )
        return;

    for (auto & listener : m_aListenerMap)
    {
        // Compare references and release dispatch references if they are equal.
        Reference< XInterface > xIfac(listener.second, UNO_QUERY);
        if ( xSource == xIfac )
            listener.second.clear();
    }

    Reference< XInterface > xIfac( m_xFrame, UNO_QUERY );
    if ( xIfac == xSource )
        m_xFrame.clear();
}

// XStatusListener
void SAL_CALL ToolboxController::statusChanged( const FeatureStateEvent& )
{
    // must be implemented by sub class
}

// XToolbarController
void SAL_CALL ToolboxController::execute( sal_Int16 KeyModifier )
{
    Reference< XDispatch >       xDispatch;
    OUString                     aCommandURL;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( m_bDisposed )
            throw DisposedException();

        if ( m_bInitialized &&
             m_xFrame.is() &&
             !m_aCommandURL.isEmpty() )
        {
            aCommandURL = m_aCommandURL;
            URLToDispatchMap::iterator pIter = m_aListenerMap.find( m_aCommandURL );
            if ( pIter != m_aListenerMap.end() )
                xDispatch = pIter->second;
        }
    }

    if ( !xDispatch.is() )
        return;

    try
    {
        css::util::URL aTargetURL;
        Sequence<PropertyValue>   aArgs( 1 );

        // Provide key modifier information to dispatch function
        aArgs[0].Name   = "KeyModifier";
        aArgs[0].Value  <<= KeyModifier;

        aTargetURL.Complete = aCommandURL;
        if ( m_xUrlTransformer.is() )
            m_xUrlTransformer->parseStrict( aTargetURL );
        xDispatch->dispatch( aTargetURL, aArgs );
    }
    catch ( DisposedException& )
    {
    }
}

void SAL_CALL ToolboxController::click()
{
}

void SAL_CALL ToolboxController::doubleClick()
{
}

Reference< XWindow > SAL_CALL ToolboxController::createPopupWindow()
{
    return Reference< XWindow >();
}

Reference< XWindow > SAL_CALL ToolboxController::createItemWindow( const Reference< XWindow >& )
{
    return Reference< XWindow >();
}

void ToolboxController::addStatusListener( const OUString& aCommandURL )
{
    Reference< XDispatch >       xDispatch;
    Reference< XStatusListener > xStatusListener;
    css::util::URL    aTargetURL;

    {
        SolarMutexGuard aSolarMutexGuard;
        URLToDispatchMap::iterator pIter = m_aListenerMap.find( aCommandURL );

        // Already in the list of status listener. Do nothing.
        if ( pIter != m_aListenerMap.end() )
            return;

        // Check if we are already initialized. Implementation starts adding itself as status listener when
        // initialize is called.
        if ( !m_bInitialized )
        {
            // Put into the unordered_map of status listener. Will be activated when initialized is called
            m_aListenerMap.emplace( aCommandURL, Reference< XDispatch >() );
            return;
        }
        else
        {
            // Add status listener directly as initialize has already been called.
            Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
            if ( m_xContext.is() && xDispatchProvider.is() )
            {
                aTargetURL.Complete = aCommandURL;
                if ( m_xUrlTransformer.is() )
                    m_xUrlTransformer->parseStrict( aTargetURL );
                xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );

                xStatusListener.set( static_cast< OWeakObject* >( this ), UNO_QUERY );
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
                    m_aListenerMap.emplace( aCommandURL, xDispatch );
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

void ToolboxController::removeStatusListener( const OUString& aCommandURL )
{
    SolarMutexGuard aSolarMutexGuard;

    URLToDispatchMap::iterator pIter = m_aListenerMap.find( aCommandURL );
    if ( pIter == m_aListenerMap.end() )
        return;

    Reference< XDispatch > xDispatch( pIter->second );
    Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
    m_aListenerMap.erase( pIter );

    try
    {
        css::util::URL aTargetURL;
        aTargetURL.Complete = aCommandURL;
        if ( m_xUrlTransformer.is() )
            m_xUrlTransformer->parseStrict( aTargetURL );

        if ( xDispatch.is() && xStatusListener.is() )
            xDispatch->removeStatusListener( xStatusListener, aTargetURL );
    }
    catch ( Exception& )
    {
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
        if ( m_xContext.is() && xDispatchProvider.is() )
        {
            xStatusListener.set( static_cast< OWeakObject* >( this ), UNO_QUERY );
            for (auto & listener : m_aListenerMap)
            {
                css::util::URL aTargetURL;
                aTargetURL.Complete = listener.first;
                if ( m_xUrlTransformer.is() )
                    m_xUrlTransformer->parseStrict( aTargetURL );

                Reference< XDispatch > xDispatch(listener.second);
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

                listener.second.clear();
                xDispatch.clear();

                // Query for dispatch object. Old dispatch will be released with this, too.
                try
                {
                    xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
                }
                catch ( Exception& )
                {
                }
                listener.second = xDispatch;

                Listener aListener( aTargetURL, xDispatch );
                aDispatchVector.push_back( aListener );
            }
        }
    }

    // Call without locked mutex as we are called back from dispatch implementation
    if ( !xStatusListener.is() )
        return;

    try
    {
        for (Listener & rListener : aDispatchVector)
        {
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
                    aFeatureStateEvent.IsEnabled = false;
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

void ToolboxController::unbindListener()
{
    SolarMutexGuard aSolarMutexGuard;

    if ( !m_bInitialized )
        return;

    // Collect all registered command URL's and store them temporary
    Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
    if ( !(m_xContext.is() && xDispatchProvider.is()) )
        return;

    Reference< XStatusListener > xStatusListener( static_cast< OWeakObject* >( this ), UNO_QUERY );
    for (auto & listener : m_aListenerMap)
    {
        css::util::URL aTargetURL;
        aTargetURL.Complete = listener.first;
        if ( m_xUrlTransformer.is() )
            m_xUrlTransformer->parseStrict( aTargetURL );

        Reference< XDispatch > xDispatch(listener.second);
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
        listener.second.clear();
    }
}

bool ToolboxController::isBound() const
{
    SolarMutexGuard aSolarMutexGuard;

    if ( !m_bInitialized )
        return false;

    URLToDispatchMap::const_iterator pIter = m_aListenerMap.find( m_aCommandURL );
    if ( pIter != m_aListenerMap.end() )
        return pIter->second.is();

    return false;
}

void ToolboxController::updateStatus()
{
    bindListener();
}

void ToolboxController::updateStatus( const OUString& aCommandURL )
{
    Reference< XDispatch > xDispatch;
    Reference< XStatusListener > xStatusListener;
    css::util::URL aTargetURL;

    {
        SolarMutexGuard aSolarMutexGuard;

        if ( !m_bInitialized )
            return;

        // Try to find a dispatch object for the requested command URL
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY );
        xStatusListener.set( static_cast< OWeakObject* >( this ), UNO_QUERY );
        if ( m_xContext.is() && xDispatchProvider.is() )
        {
            aTargetURL.Complete = aCommandURL;
            if ( m_xUrlTransformer.is() )
                m_xUrlTransformer->parseStrict( aTargetURL );
            xDispatch = xDispatchProvider->queryDispatch( aTargetURL, OUString(), 0 );
        }
    }

    if ( !(xDispatch.is() && xStatusListener.is()) )
        return;

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


void ToolboxController::dispatchCommand( const OUString& sCommandURL, const Sequence< PropertyValue >& rArgs, const OUString &sTarget )
{
    try
    {
        Reference< XDispatchProvider > xDispatchProvider( m_xFrame, UNO_QUERY_THROW );
        URL aURL;
        aURL.Complete = sCommandURL;
        getURLTransformer()->parseStrict( aURL );

        Reference< XDispatch > xDispatch( xDispatchProvider->queryDispatch( aURL, sTarget, 0 ), UNO_QUERY_THROW );

        std::unique_ptr<DispatchInfo> pDispatchInfo(new DispatchInfo( xDispatch, aURL, rArgs ));
        if ( Application::PostUserEvent( LINK(nullptr, ToolboxController, ExecuteHdl_Impl),
                                          pDispatchInfo.get() ) )
            pDispatchInfo.release();

    }
    catch( Exception& )
    {
    }
}


css::uno::Reference< css::beans::XPropertySetInfo >  SAL_CALL ToolboxController::getPropertySetInfo()
{
    Reference<XPropertySetInfo>  xInfo( createPropertySetInfo( getInfoHelper() ) );
    return xInfo;
}

::cppu::IPropertyArrayHelper& ToolboxController::getInfoHelper()
{
        return *getArrayHelper();
}


::cppu::IPropertyArrayHelper* ToolboxController::createArrayHelper( ) const
{
        css::uno::Sequence< Property > aProps;
        describeProperties(aProps);
        return new ::cppu::OPropertyArrayHelper(aProps);
}

sal_Bool SAL_CALL ToolboxController::convertFastPropertyValue( css::uno::Any&    aConvertedValue ,
                                             css::uno::Any&        aOldValue       ,
                                             sal_Int32                        nHandle         ,
                                             const css::uno::Any&  aValue          )
{
    switch (nHandle)
    {
        case TOOLBARCONTROLLER_PROPHANDLE_SUPPORTSVISIBLE:
        {
            bool aNewValue(false);
            aValue >>= aNewValue;
            if (aNewValue != m_bSupportVisible)
            {
                aConvertedValue <<= aNewValue;
                aOldValue <<= m_bSupportVisible;
                return true;
            }
            return false;
        }
    }
    return OPropertyContainer::convertFastPropertyValue(aConvertedValue, aOldValue, nHandle, aValue);
}

void SAL_CALL ToolboxController::setFastPropertyValue_NoBroadcast(
    sal_Int32                       nHandle,
    const css::uno::Any& aValue )
{
    OPropertyContainer::setFastPropertyValue_NoBroadcast(nHandle, aValue);
    if (TOOLBARCONTROLLER_PROPHANDLE_SUPPORTSVISIBLE == nHandle)
    {
        bool rValue(false);
        if (( aValue >>= rValue ) && m_bInitialized)
            m_bSupportVisible = rValue;
    }
}


IMPL_STATIC_LINK( ToolboxController, ExecuteHdl_Impl, void*, p, void )
{
    DispatchInfo* pDispatchInfo = static_cast<DispatchInfo*>(p);
    pDispatchInfo->mxDispatch->dispatch( pDispatchInfo->maURL, pDispatchInfo->maArgs );
    delete pDispatchInfo;
}

void ToolboxController::enable( bool bEnable )
{
    ToolBox* pToolBox = nullptr;
    sal_uInt16 nItemId = 0;
    if( getToolboxId( nItemId, &pToolBox ) )
    {
        pToolBox->EnableItem( nItemId, bEnable );
    }
}

bool ToolboxController::getToolboxId( sal_uInt16& rItemId, ToolBox** ppToolBox )
{
    if( (m_nToolBoxId != SAL_MAX_UINT16) && (ppToolBox == nullptr) )
        return m_nToolBoxId;

    ToolBox* pToolBox = static_cast< ToolBox* >( VCLUnoHelper::GetWindow( getParent() ).get() );

    if( (m_nToolBoxId == SAL_MAX_UINT16) && pToolBox )
    {
        const ToolBox::ImplToolItems::size_type nCount = pToolBox->GetItemCount();
        for ( ToolBox::ImplToolItems::size_type nPos = 0; nPos < nCount; ++nPos )
        {
            const sal_uInt16 nItemId = pToolBox->GetItemId( nPos );
            if ( pToolBox->GetItemCommand( nItemId ) == m_aCommandURL )
            {
                m_nToolBoxId = nItemId;
                break;
            }
        }
    }

    if( ppToolBox )
        *ppToolBox = pToolBox;

    rItemId = m_nToolBoxId;

    return (rItemId != SAL_MAX_UINT16) && (( ppToolBox == nullptr) || (*ppToolBox != nullptr) );
}
//end

} // svt

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
