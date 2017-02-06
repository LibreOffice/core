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

#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#include <com/sun/star/embed/ObjectSaveVetoException.hpp>
#include <com/sun/star/embed/StorageWrappedTargetException.hpp>
#include <com/sun/star/embed/UnreachableStateException.hpp>
#include <com/sun/star/embed/XEmbeddedClient.hpp>
#include <com/sun/star/embed/XInplaceClient.hpp>
#include <com/sun/star/embed/XWindowSupplier.hpp>
#include <com/sun/star/embed/StateChangeInProgressException.hpp>
#include <com/sun/star/embed/Aspects.hpp>

#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/frame/ModuleManager.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <com/sun/star/embed/EmbedMisc.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <vcl/svapp.hxx>

#include <targetstatecontrol.hxx>

#include "commonembobj.hxx"
#include "intercept.hxx"
#include "embedobj.hxx"

using namespace ::com::sun::star;

awt::Rectangle GetRectangleInterception( const awt::Rectangle& aRect1, const awt::Rectangle& aRect2 )
{
    awt::Rectangle aResult;

    OSL_ENSURE( aRect1.Width >= 0 && aRect2.Width >= 0 && aRect1.Height >= 0 && aRect2.Height >= 0,
                "Offset must not be less then zero!" );

    aResult.X = aRect1.X > aRect2.X ? aRect1.X : aRect2.X;
    aResult.Y = aRect1.Y > aRect2.Y ? aRect1.Y : aRect2.Y;

    sal_Int32 nRight1 = aRect1.X + aRect1.Width;
    sal_Int32 nBottom1 = aRect1.Y + aRect1.Height;
    sal_Int32 nRight2 = aRect2.X + aRect2.Width;
    sal_Int32 nBottom2 = aRect2.Y + aRect2.Height;
    aResult.Width = ( nRight1 < nRight2 ? nRight1 : nRight2 ) - aResult.X;
    aResult.Height = ( nBottom1 < nBottom2 ? nBottom1 : nBottom2 ) - aResult.Y;

    return aResult;
}


sal_Int32 OCommonEmbeddedObject::ConvertVerbToState_Impl( sal_Int32 nVerb )
{
    for ( sal_Int32 nInd = 0; nInd < m_aVerbTable.getLength(); nInd++ )
        if ( m_aVerbTable[nInd][0] == nVerb )
            return m_aVerbTable[nInd][1];

    throw lang::IllegalArgumentException(); // TODO: unexpected verb provided
}


void OCommonEmbeddedObject::Deactivate()
{
    uno::Reference< util::XModifiable > xModif( m_xDocHolder->GetComponent(), uno::UNO_QUERY );

    // no need to lock for the initialization
    uno::Reference< embed::XEmbeddedClient > xClientSite = m_xClientSite;
    if ( !xClientSite.is() )
        throw embed::WrongStateException(); //TODO: client site is not set!

    // store document if it is modified
    if ( xModif.is() && xModif->isModified() )
    {
        try {
            xClientSite->saveObject();
        }
        catch( const embed::ObjectSaveVetoException& )
        {
        }
        catch( const uno::Exception& e )
        {
            throw embed::StorageWrappedTargetException(
                "The client could not store the object!",
                static_cast< ::cppu::OWeakObject* >( this ),
                uno::makeAny( e ) );
        }
    }

    m_xDocHolder->CloseFrame();

    xClientSite->visibilityChanged( false );
}


void OCommonEmbeddedObject::StateChangeNotification_Impl( bool bBeforeChange, sal_Int32 nOldState, sal_Int32 nNewState ,::osl::ResettableMutexGuard& rGuard )
{
    if ( m_pInterfaceContainer )
    {
        ::cppu::OInterfaceContainerHelper* pContainer = m_pInterfaceContainer->getContainer(
                            cppu::UnoType<embed::XStateChangeListener>::get());
        if ( pContainer != nullptr )
        {
            lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );
            ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);

            // should be locked after the method is finished successfully
            rGuard.clear();

            while (pIterator.hasMoreElements())
            {
                try
                {
                    if ( bBeforeChange )
                        static_cast<embed::XStateChangeListener*>(pIterator.next())->changingState( aSource, nOldState, nNewState );
                    else
                        static_cast<embed::XStateChangeListener*>(pIterator.next())->stateChanged( aSource, nOldState, nNewState );
                }
                catch( const uno::Exception& )
                {
                    // even if the listener complains ignore it for now
                   }

                if ( m_bDisposed )
                    return;
            }

            rGuard.reset();
        }
    }
}


void OCommonEmbeddedObject::SwitchStateTo_Impl( sal_Int32 nNextState )
{
    // TODO: may be needs interaction handler to detect whether the object state
    //         can be changed even after errors

    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        if ( nNextState == embed::EmbedStates::RUNNING )
        {
            // after the object reaches the running state the cloned size is not necessary any more
            m_bHasClonedSize = false;

            if ( m_bIsLink )
            {
                m_xDocHolder->SetComponent( LoadLink_Impl(), m_bReadOnly );
            }
            else
            {
                uno::Reference < embed::XEmbedPersist > xPersist( static_cast < embed::XClassifiedObject* > (this), uno::UNO_QUERY );
                if ( xPersist.is() )
                {
                    // in case embedded object is in loaded state the contents must
                    // be stored in the related storage and the storage
                    // must be created already
                    if ( !m_xObjectStorage.is() )
                        throw io::IOException(); //TODO: access denied

                    m_xDocHolder->SetComponent( LoadDocumentFromStorage_Impl(), m_bReadOnly );
                }
                else
                {
                    // objects without persistence will be initialized internally
                    uno::Sequence < uno::Any > aArgs(1);
                    aArgs[0] <<= uno::Reference < embed::XEmbeddedObject >( this );
                    uno::Reference< util::XCloseable > xDocument(
                            m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( GetDocumentServiceName(), aArgs, m_xContext),
                            uno::UNO_QUERY );

                    uno::Reference < container::XChild > xChild( xDocument, uno::UNO_QUERY );
                    if ( xChild.is() )
                        xChild->setParent( m_xParent );

                    m_xDocHolder->SetComponent( xDocument, m_bReadOnly );
                }
            }

            if ( !m_xDocHolder->GetComponent().is() )
                throw embed::UnreachableStateException(); //TODO: can't open document

            m_nObjectState = nNextState;
        }
        else
        {
            SAL_WARN( "embeddedobj.common", "Unacceptable state switch!" );
            throw uno::RuntimeException(); // TODO
        }
    }
    else if ( m_nObjectState == embed::EmbedStates::RUNNING )
    {
        if ( nNextState == embed::EmbedStates::LOADED )
        {
            m_nClonedMapUnit = m_xDocHolder->GetMapUnit( embed::Aspects::MSOLE_CONTENT );
            m_bHasClonedSize = m_xDocHolder->GetExtent( embed::Aspects::MSOLE_CONTENT, &m_aClonedSize );

            // actually frame should not exist at this point
            m_xDocHolder->CloseDocument( false, false );

            m_nObjectState = nNextState;
        }
        else
        {
            if ( nNextState == embed::EmbedStates::INPLACE_ACTIVE )
            {
                if ( !m_xClientSite.is() )
                    throw embed::WrongStateException( "client site not set, yet", *this );

                uno::Reference< embed::XInplaceClient > xInplaceClient( m_xClientSite, uno::UNO_QUERY );
                if ( xInplaceClient.is() && xInplaceClient->canInplaceActivate() )
                {
                    xInplaceClient->activatingInplace();

                    uno::Reference< embed::XWindowSupplier > xClientWindowSupplier( xInplaceClient, uno::UNO_QUERY );
                    if ( !xClientWindowSupplier.is() )
                        throw uno::RuntimeException(); // TODO: the inplace client implementation must support XWinSupp

                    m_xClientWindow = xClientWindowSupplier->getWindow();
                    m_aOwnRectangle = xInplaceClient->getPlacement();
                    m_aClipRectangle = xInplaceClient->getClipRectangle();
                    awt::Rectangle aRectangleToShow = GetRectangleInterception( m_aOwnRectangle, m_aClipRectangle );

                    // create own window based on the client window
                    // place and resize the window according to the rectangles
                    uno::Reference< awt::XWindowPeer > xClientWindowPeer( m_xClientWindow, uno::UNO_QUERY );
                    if ( !xClientWindowPeer.is() )
                        throw uno::RuntimeException(); // TODO: the container window must support the interface

                    // dispatch provider may not be provided
                    uno::Reference< frame::XDispatchProvider > xContainerDP = xInplaceClient->getInplaceDispatchProvider();
                    bool bOk = m_xDocHolder->ShowInplace( xClientWindowPeer, aRectangleToShow, xContainerDP );
                    m_nObjectState = nNextState;
                    if ( !bOk )
                    {
                        SwitchStateTo_Impl( embed::EmbedStates::RUNNING );
                        throw embed::WrongStateException(); //TODO: can't activate inplace
                    }
                }
                else
                    throw embed::WrongStateException(); //TODO: can't activate inplace
            }
            else if ( nNextState == embed::EmbedStates::ACTIVE )
            {
                if ( !m_xClientSite.is() )
                    throw embed::WrongStateException(); //TODO: client site is not set!

                // create frame and load document in the frame
                m_xDocHolder->Show();

                m_xClientSite->visibilityChanged( true );
                m_nObjectState = nNextState;
            }
            else
            {
                SAL_WARN( "embeddedobj.common", "Unacceptable state switch!" );
                throw uno::RuntimeException(); // TODO
            }
        }
    }
    else if ( m_nObjectState == embed::EmbedStates::INPLACE_ACTIVE )
    {
        if ( nNextState == embed::EmbedStates::RUNNING )
        {
            uno::Reference< embed::XInplaceClient > xInplaceClient( m_xClientSite, uno::UNO_QUERY );
            if ( !xInplaceClient.is() )
                throw uno::RuntimeException();

            m_xClientSite->visibilityChanged( true );

            xInplaceClient->deactivatedInplace();
            Deactivate();
            m_nObjectState = nNextState;
        }
        else if ( nNextState == embed::EmbedStates::UI_ACTIVE )
        {
            if ( !(m_nMiscStatus & embed::EmbedMisc::MS_EMBED_NOUIACTIVATE) )
            {
                uno::Reference< embed::XInplaceClient > xInplaceClient( m_xClientSite, uno::UNO_QUERY_THROW );
                // TODO:
                uno::Reference< css::frame::XLayoutManager > xContainerLM =
                            xInplaceClient->getLayoutManager();
                if ( xContainerLM.is() )
                {
                    // dispatch provider may not be provided
                    uno::Reference< frame::XDispatchProvider > xContainerDP = xInplaceClient->getInplaceDispatchProvider();

                    // get the container module name
                    OUString aModuleName;
                    try
                    {
                        uno::Reference< embed::XComponentSupplier > xCompSupl( m_xClientSite, uno::UNO_QUERY_THROW );
                        uno::Reference< uno::XInterface > xContDoc( xCompSupl->getComponent(), uno::UNO_QUERY_THROW );

                        uno::Reference< frame::XModuleManager2 > xManager( frame::ModuleManager::create( m_xContext ) );

                        aModuleName = xManager->identify( xContDoc );
                    }
                    catch( const uno::Exception& )
                    {}

                    // if currently another object is UIactive it will be deactivated; usually this will activate the LM of
                    // the container. Locking the LM will prevent flicker.
                    xContainerLM->lock();
                    xInplaceClient->activatingUI();
                    bool bOk = m_xDocHolder->ShowUI( xContainerLM, xContainerDP, aModuleName );
                    xContainerLM->unlock();

                    if ( bOk )
                    {
                        m_nObjectState = nNextState;
                        m_xDocHolder->ResizeHatchWindow();
                    }
                    else
                    {
                        xInplaceClient->deactivatedUI();
                        throw embed::WrongStateException(); //TODO: can't activate UI
                    }
                }
                else
                    throw embed::WrongStateException(); //TODO: can't activate UI
            }
        }
        else
        {
            SAL_WARN( "embeddedobj.common", "Unacceptable state switch!" );
            throw uno::RuntimeException(); // TODO
        }
    }
    else if ( m_nObjectState == embed::EmbedStates::ACTIVE )
    {
        if ( nNextState == embed::EmbedStates::RUNNING )
        {
            Deactivate();
            m_nObjectState = nNextState;
        }
        else
        {
            SAL_WARN( "embeddedobj.common", "Unacceptable state switch!" );
            throw uno::RuntimeException(); // TODO
        }
    }
    else if ( m_nObjectState == embed::EmbedStates::UI_ACTIVE )
    {
        if ( nNextState == embed::EmbedStates::INPLACE_ACTIVE )
        {
            uno::Reference< embed::XInplaceClient > xInplaceClient( m_xClientSite, uno::UNO_QUERY_THROW );
            uno::Reference< css::frame::XLayoutManager > xContainerLM =
                        xInplaceClient->getLayoutManager();

            bool bOk = false;
            if ( xContainerLM.is() )
                bOk = m_xDocHolder->HideUI( xContainerLM );

            if ( bOk )
            {
                m_nObjectState = nNextState;
                m_xDocHolder->ResizeHatchWindow();
                xInplaceClient->deactivatedUI();
            }
            else
                throw embed::WrongStateException(); //TODO: can't activate UI
        }
    }
    else
        throw embed::WrongStateException( "The object is in unacceptable state!",
                                          static_cast< ::cppu::OWeakObject* >(this) );
}


uno::Sequence< sal_Int32 > const & OCommonEmbeddedObject::GetIntermediateStatesSequence_Impl( sal_Int32 nNewState )
{
    sal_Int32 nCurInd = 0;
    for ( nCurInd = 0; nCurInd < m_aAcceptedStates.getLength(); nCurInd++ )
        if ( m_aAcceptedStates[nCurInd] == m_nObjectState )
            break;

    if ( nCurInd == m_aAcceptedStates.getLength() )
        throw embed::WrongStateException( "The object is in unacceptable state!",
                                          static_cast< ::cppu::OWeakObject* >(this) );

    sal_Int32 nDestInd = 0;
    for ( nDestInd = 0; nDestInd < m_aAcceptedStates.getLength(); nDestInd++ )
        if ( m_aAcceptedStates[nDestInd] == nNewState )
            break;

    if ( nDestInd == m_aAcceptedStates.getLength() )
        throw embed::UnreachableStateException(
            "The state either not reachable, or the object allows the state only as an intermediate one!",
            static_cast< ::cppu::OWeakObject* >(this),
            m_nObjectState,
            nNewState );

    return m_pIntermediateStatesSeqs[nCurInd][nDestInd];
}


void SAL_CALL OCommonEmbeddedObject::changeState( sal_Int32 nNewState )
{
    {
        ::osl::ResettableMutexGuard aGuard( m_aMutex );
        if ( m_bDisposed )
            throw lang::DisposedException(); // TODO

        if ( m_nObjectState == -1 )
            throw embed::WrongStateException( "The object has no persistence!",
                                              static_cast< ::cppu::OWeakObject* >(this) );

        sal_Int32 nOldState = m_nObjectState;

        if ( m_nTargetState != -1 )
        {
            // means that the object is currently trying to reach the target state
            throw embed::StateChangeInProgressException( OUString(),
                                                        uno::Reference< uno::XInterface >(),
                                                        m_nTargetState );
        }
        else
        {
            TargetStateControl_Impl aControl( m_nTargetState, nNewState );

            // in case the object is already in requested state
            if ( m_nObjectState == nNewState )
            {
                // if active object is activated again, bring its window to top
                if ( m_nObjectState == embed::EmbedStates::ACTIVE )
                    m_xDocHolder->Show();

                return;
            }

            // retrieve sequence of states that should be passed to reach desired state
            uno::Sequence< sal_Int32 > aIntermediateStates = GetIntermediateStatesSequence_Impl( nNewState );

            // notify listeners that the object is going to change the state
            StateChangeNotification_Impl( true, nOldState, nNewState,aGuard );

            try {
                for ( sal_Int32 nInd = 0; nInd < aIntermediateStates.getLength(); nInd++ )
                    SwitchStateTo_Impl( aIntermediateStates[nInd] );

                SwitchStateTo_Impl( nNewState );
            }
            catch( const uno::Exception& )
            {
                if ( nOldState != m_nObjectState )
                    // notify listeners that the object has changed the state
                    StateChangeNotification_Impl( false, nOldState, m_nObjectState, aGuard );

                throw;
            }
        }

        // notify listeners that the object has changed the state
        StateChangeNotification_Impl( false, nOldState, nNewState, aGuard );

        // let the object window be shown
        if ( nNewState == embed::EmbedStates::UI_ACTIVE || nNewState == embed::EmbedStates::INPLACE_ACTIVE )
            PostEvent_Impl( "OnVisAreaChanged" );
    }
}


uno::Sequence< sal_Int32 > SAL_CALL OCommonEmbeddedObject::getReachableStates()
{
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The object has no persistence!",
                                           static_cast< ::cppu::OWeakObject* >(this) );

    return m_aAcceptedStates;
}


sal_Int32 SAL_CALL OCommonEmbeddedObject::getCurrentState()
{
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The object has no persistence!",
                                          static_cast< ::cppu::OWeakObject* >(this) );

    return m_nObjectState;
}


void SAL_CALL OCommonEmbeddedObject::doVerb( sal_Int32 nVerbID )
{
    SolarMutexGuard aSolarGuard;
        //TODO: a gross hack to avoid deadlocks when this is called from the
        // outside and OCommonEmbeddedObject::changeState, with m_aMutex locked,
        // calls into framework code that tries to lock the solar mutex, while
        // another thread (through Window::ImplCallPaint, say) calls
        // OCommonEmbeddedObject::getComponent with the solar mutex locked and
        // then tries to lock m_aMutex (see fdo#56818); the alternative would be
        // to get locking done right in this class, but that looks like a
        // daunting task

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The object has no persistence!",
                                          static_cast< ::cppu::OWeakObject* >(this) );

    // for internal documents this call is just a duplicate of changeState
    sal_Int32 nNewState = -1;
    try
    {
        nNewState = ConvertVerbToState_Impl( nVerbID );
    }
    catch( const uno::Exception& )
    {}

    if ( nNewState == -1 )
    {
        // TODO/LATER: Save Copy as... verb ( -8 ) is implemented by container
        // TODO/LATER: check if the verb is a supported one and if it is produce related operation
    }
    else
    {
        aGuard.clear();
        changeState( nNewState );
    }
}


uno::Sequence< embed::VerbDescriptor > SAL_CALL OCommonEmbeddedObject::getSupportedVerbs()
{
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The object has no persistence!",
                                          static_cast< ::cppu::OWeakObject* >(this) );

    return m_aObjectVerbs;
}


void SAL_CALL OCommonEmbeddedObject::setClientSite(
                const uno::Reference< embed::XEmbeddedClient >& xClient )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_xClientSite != xClient)
    {
        if ( m_nObjectState != embed::EmbedStates::LOADED && m_nObjectState != embed::EmbedStates::RUNNING )
            throw embed::WrongStateException(
                                    "The client site can not be set currently!",
                                     static_cast< ::cppu::OWeakObject* >(this) );

        m_xClientSite = xClient;
    }
}


uno::Reference< embed::XEmbeddedClient > SAL_CALL OCommonEmbeddedObject::getClientSite()
{
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The object has no persistence!",
                                          static_cast< ::cppu::OWeakObject* >(this) );

    return m_xClientSite;
}


void SAL_CALL OCommonEmbeddedObject::update()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The object has no persistence!",
                                          static_cast< ::cppu::OWeakObject* >(this) );

    PostEvent_Impl( "OnVisAreaChanged" );
}


void SAL_CALL OCommonEmbeddedObject::setUpdateMode( sal_Int32 nMode )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The object has no persistence!",
                                          static_cast< ::cppu::OWeakObject* >(this) );

    OSL_ENSURE( nMode == embed::EmbedUpdateModes::ALWAYS_UPDATE
                    || nMode == embed::EmbedUpdateModes::EXPLICIT_UPDATE,
                "Unknown update mode!\n" );
    m_nUpdateMode = nMode;
}


sal_Int64 SAL_CALL OCommonEmbeddedObject::getStatus( sal_Int64 )
{
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return m_nMiscStatus;
}


void SAL_CALL OCommonEmbeddedObject::setContainerName( const OUString& sName )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    m_aContainerName = sName;
}

css::uno::Reference< css::uno::XInterface > SAL_CALL OCommonEmbeddedObject::getParent()
{
    return m_xParent;
}

void SAL_CALL OCommonEmbeddedObject::setParent( const css::uno::Reference< css::uno::XInterface >& xParent )
{
    m_xParent = xParent;
    if ( m_nObjectState != -1 && m_nObjectState != embed::EmbedStates::LOADED )
    {
        uno::Reference < container::XChild > xChild( m_xDocHolder->GetComponent(), uno::UNO_QUERY );
        if ( xChild.is() )
            xChild->setParent( xParent );
    }
}

// XDefaultSizeTransmitter
void SAL_CALL OCommonEmbeddedObject::setDefaultSize( const css::awt::Size& rSize_100TH_MM )
{
    //#i103460# charts do not necessaryly have an own size within ODF files, in this case they need to use the size settings from the surrounding frame, which is made available with this method
    m_aDefaultSizeForChart_In_100TH_MM = rSize_100TH_MM;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
