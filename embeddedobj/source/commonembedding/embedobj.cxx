/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#include <com/sun/star/embed/XEmbeddedClient.hpp>
#include <com/sun/star/embed/XInplaceClient.hpp>
#include <com/sun/star/embed/XWindowSupplier.hpp>
#include <com/sun/star/embed/StateChangeInProgressException.hpp>
#include <com/sun/star/embed/Aspects.hpp>

#include <com/sun/star/awt/XWindowPeer.hpp>
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

#include <vcl/svapp.hxx>

#include <targetstatecontrol.hxx>

#include "commonembobj.hxx"
#include "intercept.hxx"


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

    throw lang::IllegalArgumentException(); 
}


void OCommonEmbeddedObject::Deactivate()
{
    uno::Reference< util::XModifiable > xModif( m_pDocHolder->GetComponent(), uno::UNO_QUERY );

    
    uno::Reference< embed::XEmbeddedClient > xClientSite = m_xClientSite;
    if ( !xClientSite.is() )
        throw embed::WrongStateException(); 

    
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
                OUString( "The client could not store the object!" ),
                uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) ),
                uno::makeAny( e ) );
        }
    }

    m_pDocHolder->CloseFrame();

    xClientSite->visibilityChanged( sal_False );
}


void OCommonEmbeddedObject::StateChangeNotification_Impl( sal_Bool bBeforeChange, sal_Int32 nOldState, sal_Int32 nNewState ,::osl::ResettableMutexGuard& rGuard )
{
    if ( m_pInterfaceContainer )
    {
        ::cppu::OInterfaceContainerHelper* pContainer = m_pInterfaceContainer->getContainer(
                            ::getCppuType( ( const uno::Reference< embed::XStateChangeListener >*) NULL ) );
        if ( pContainer != NULL )
        {
            lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );
            ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);

            
            rGuard.clear();

            while (pIterator.hasMoreElements())
            {
                try
                {
                    if ( bBeforeChange )
                        ((embed::XStateChangeListener*)pIterator.next())->changingState( aSource, nOldState, nNewState );
                    else
                        ((embed::XStateChangeListener*)pIterator.next())->stateChanged( aSource, nOldState, nNewState );
                }
                catch( const uno::Exception& )
                {
                    
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
    
    

    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        if ( nNextState == embed::EmbedStates::RUNNING )
        {
            
            m_bHasClonedSize = sal_False;

            if ( m_bIsLink )
            {
                m_pDocHolder->SetComponent( LoadLink_Impl(), m_bReadOnly );
            }
            else
            {
                uno::Reference < embed::XEmbedPersist > xPersist( static_cast < embed::XClassifiedObject* > (this), uno::UNO_QUERY );
                if ( xPersist.is() )
                {
                    
                    
                    
                    if ( !m_xObjectStorage.is() )
                        throw io::IOException(); 

                    m_pDocHolder->SetComponent( LoadDocumentFromStorage_Impl(), m_bReadOnly );
                }
                else
                {
                    
                    uno::Sequence < uno::Any > aArgs(1);
                    aArgs[0] <<= uno::Reference < embed::XEmbeddedObject >( this );
                    uno::Reference< util::XCloseable > xDocument(
                            m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext( GetDocumentServiceName(), aArgs, m_xContext),
                            uno::UNO_QUERY );

                    uno::Reference < container::XChild > xChild( xDocument, uno::UNO_QUERY );
                    if ( xChild.is() )
                        xChild->setParent( m_xParent );

                    m_pDocHolder->SetComponent( xDocument, m_bReadOnly );
                }
            }

            if ( !m_pDocHolder->GetComponent().is() )
                throw embed::UnreachableStateException(); 

            m_nObjectState = nNextState;
        }
        else
        {
            SAL_WARN( "embeddedobj.common", "Unacceptable state switch!" );
            throw uno::RuntimeException(); 
        }
    }
    else if ( m_nObjectState == embed::EmbedStates::RUNNING )
    {
        if ( nNextState == embed::EmbedStates::LOADED )
        {
            m_nClonedMapUnit = m_pDocHolder->GetMapUnit( embed::Aspects::MSOLE_CONTENT );
            m_bHasClonedSize = m_pDocHolder->GetExtent( embed::Aspects::MSOLE_CONTENT, &m_aClonedSize );

            
            m_pDocHolder->CloseDocument( sal_False, sal_False );

            m_nObjectState = nNextState;
        }
        else
        {
            if ( nNextState == embed::EmbedStates::INPLACE_ACTIVE )
            {
                if ( !m_xClientSite.is() )
                    throw embed::WrongStateException(
                        OUString( "client site not set, yet" ),
                        *this
                );

                uno::Reference< embed::XInplaceClient > xInplaceClient( m_xClientSite, uno::UNO_QUERY );
                if ( xInplaceClient.is() && xInplaceClient->canInplaceActivate() )
                {
                    xInplaceClient->activatingInplace();

                    uno::Reference< embed::XWindowSupplier > xClientWindowSupplier( xInplaceClient, uno::UNO_QUERY );
                    if ( !xClientWindowSupplier.is() )
                        throw uno::RuntimeException(); 

                    m_xClientWindow = xClientWindowSupplier->getWindow();
                    m_aOwnRectangle = xInplaceClient->getPlacement();
                    m_aClipRectangle = xInplaceClient->getClipRectangle();
                    awt::Rectangle aRectangleToShow = GetRectangleInterception( m_aOwnRectangle, m_aClipRectangle );

                    
                    
                    uno::Reference< awt::XWindowPeer > xClientWindowPeer( m_xClientWindow, uno::UNO_QUERY );
                    if ( !xClientWindowPeer.is() )
                        throw uno::RuntimeException(); 

                    
                    uno::Reference< frame::XDispatchProvider > xContainerDP = xInplaceClient->getInplaceDispatchProvider();
                    sal_Bool bOk = m_pDocHolder->ShowInplace( xClientWindowPeer, aRectangleToShow, xContainerDP );
                    m_nObjectState = nNextState;
                    if ( !bOk )
                    {
                        SwitchStateTo_Impl( embed::EmbedStates::RUNNING );
                        throw embed::WrongStateException(); 
                    }
                }
                else
                    throw embed::WrongStateException(); 
            }
            else if ( nNextState == embed::EmbedStates::ACTIVE )
            {
                if ( !m_xClientSite.is() )
                    throw embed::WrongStateException(); 

                
                m_pDocHolder->Show();

                m_xClientSite->visibilityChanged( sal_True );
                m_nObjectState = nNextState;
            }
            else
            {
                SAL_WARN( "embeddedobj.common", "Unacceptable state switch!" );
                throw uno::RuntimeException(); 
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

            m_xClientSite->visibilityChanged( sal_True );

            xInplaceClient->deactivatedInplace();
            Deactivate();
            m_nObjectState = nNextState;
        }
        else if ( nNextState == embed::EmbedStates::UI_ACTIVE )
        {
            if ( !(m_nMiscStatus & embed::EmbedMisc::MS_EMBED_NOUIACTIVATE) )
            {
                uno::Reference< embed::XInplaceClient > xInplaceClient( m_xClientSite, uno::UNO_QUERY_THROW );
                
                uno::Reference< ::com::sun::star::frame::XLayoutManager > xContainerLM =
                            xInplaceClient->getLayoutManager();
                if ( xContainerLM.is() )
                {
                    
                    uno::Reference< frame::XDispatchProvider > xContainerDP = xInplaceClient->getInplaceDispatchProvider();

                    
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

                    
                    
                    xContainerLM->lock();
                    xInplaceClient->activatingUI();
                    sal_Bool bOk = m_pDocHolder->ShowUI( xContainerLM, xContainerDP, aModuleName );
                    xContainerLM->unlock();

                    if ( bOk )
                    {
                        m_nObjectState = nNextState;
                        m_pDocHolder->ResizeHatchWindow();
                    }
                    else
                    {
                        xInplaceClient->deactivatedUI();
                        throw embed::WrongStateException(); 
                    }
                }
                else
                    throw embed::WrongStateException(); 
            }
        }
        else
        {
            SAL_WARN( "embeddedobj.common", "Unacceptable state switch!" );
            throw uno::RuntimeException(); 
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
            throw uno::RuntimeException(); 
        }
    }
    else if ( m_nObjectState == embed::EmbedStates::UI_ACTIVE )
    {
        if ( nNextState == embed::EmbedStates::INPLACE_ACTIVE )
        {
            uno::Reference< embed::XInplaceClient > xInplaceClient( m_xClientSite, uno::UNO_QUERY_THROW );
            uno::Reference< ::com::sun::star::frame::XLayoutManager > xContainerLM =
                        xInplaceClient->getLayoutManager();

            sal_Bool bOk = sal_False;
            if ( xContainerLM.is() )
                   bOk = m_pDocHolder->HideUI( xContainerLM );

            if ( bOk )
            {
                m_nObjectState = nNextState;
                m_pDocHolder->ResizeHatchWindow();
                   xInplaceClient->deactivatedUI();
            }
            else
                throw embed::WrongStateException(); 
        }
    }
    else
        throw embed::WrongStateException( OUString( "The object is in unacceptable state!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
}


uno::Sequence< sal_Int32 > OCommonEmbeddedObject::GetIntermediateStatesSequence_Impl( sal_Int32 nNewState )
{
    sal_Int32 nCurInd = 0;
    for ( nCurInd = 0; nCurInd < m_aAcceptedStates.getLength(); nCurInd++ )
        if ( m_aAcceptedStates[nCurInd] == m_nObjectState )
            break;

    if ( nCurInd == m_aAcceptedStates.getLength() )
        throw embed::WrongStateException( OUString( "The object is in unacceptable state!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    sal_Int32 nDestInd = 0;
    for ( nDestInd = 0; nDestInd < m_aAcceptedStates.getLength(); nDestInd++ )
        if ( m_aAcceptedStates[nDestInd] == nNewState )
            break;

    if ( nDestInd == m_aAcceptedStates.getLength() )
        throw embed::UnreachableStateException(
            OUString( "The state either not reachable, or the object allows the state only as an intermediate one!\n" ),
            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
            m_nObjectState,
            nNewState );

    return m_pIntermediateStatesSeqs[nCurInd][nDestInd];
}


void SAL_CALL OCommonEmbeddedObject::changeState( sal_Int32 nNewState )
        throw ( embed::UnreachableStateException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    SAL_INFO( "embeddedobj.common", "embeddedobj (mv76033) OCommonEmbeddedObject::changeState" );

    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ), uno::UNO_QUERY);
    {
        ::osl::ResettableMutexGuard aGuard( m_aMutex );
        if ( m_bDisposed )
            throw lang::DisposedException(); 

        if ( m_nObjectState == -1 )
            throw embed::WrongStateException( OUString( "The object has no persistence!\n" ),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

        sal_Int32 nOldState = m_nObjectState;

        if ( m_nTargetState != -1 )
        {
            
            throw embed::StateChangeInProgressException( OUString(),
                                                        uno::Reference< uno::XInterface >(),
                                                        m_nTargetState );
        }
        else
        {
            TargetStateControl_Impl aControl( m_nTargetState, nNewState );

            
            if ( m_nObjectState == nNewState )
            {
                
                if ( m_nObjectState == embed::EmbedStates::ACTIVE )
                    m_pDocHolder->Show();

                return;
            }

            
            uno::Sequence< sal_Int32 > aIntermediateStates = GetIntermediateStatesSequence_Impl( nNewState );

            
            StateChangeNotification_Impl( sal_True, nOldState, nNewState,aGuard );

            try {
                for ( sal_Int32 nInd = 0; nInd < aIntermediateStates.getLength(); nInd++ )
                    SwitchStateTo_Impl( aIntermediateStates[nInd] );

                SwitchStateTo_Impl( nNewState );
            }
            catch( const uno::Exception& )
            {
                if ( nOldState != m_nObjectState )
                    
                    StateChangeNotification_Impl( sal_False, nOldState, m_nObjectState, aGuard );

                throw;
            }
        }

        
        StateChangeNotification_Impl( sal_False, nOldState, nNewState, aGuard );

        
        if ( nNewState == embed::EmbedStates::UI_ACTIVE || nNewState == embed::EmbedStates::INPLACE_ACTIVE )
            PostEvent_Impl( OUString( "OnVisAreaChanged" ) );
    }
}


uno::Sequence< sal_Int32 > SAL_CALL OCommonEmbeddedObject::getReachableStates()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    if ( m_bDisposed )
        throw lang::DisposedException(); 

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( OUString( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return m_aAcceptedStates;
}


sal_Int32 SAL_CALL OCommonEmbeddedObject::getCurrentState()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    if ( m_bDisposed )
        throw lang::DisposedException(); 

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( OUString( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return m_nObjectState;
}


void SAL_CALL OCommonEmbeddedObject::doVerb( sal_Int32 nVerbID )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                embed::UnreachableStateException,
                uno::Exception,
                uno::RuntimeException )
{
    SAL_INFO( "embeddedobj.common", "embeddedobj (mv76033) OCommonEmbeddedObject::doVerb" );

    SolarMutexGuard aSolarGuard;
        
        
        
        
        
        
        
        

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); 

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( OUString( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    
    sal_Int32 nNewState = -1;
    try
    {
        nNewState = ConvertVerbToState_Impl( nVerbID );
    }
    catch( const uno::Exception& )
    {}

    if ( nNewState == -1 )
    {
        
        
    }
    else
    {
        aGuard.clear();
        changeState( nNewState );
    }
}


uno::Sequence< embed::VerbDescriptor > SAL_CALL OCommonEmbeddedObject::getSupportedVerbs()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    if ( m_bDisposed )
        throw lang::DisposedException(); 

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( OUString( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return m_aObjectVerbs;
}


void SAL_CALL OCommonEmbeddedObject::setClientSite(
                const uno::Reference< embed::XEmbeddedClient >& xClient )
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); 

    if ( m_xClientSite != xClient)
    {
        if ( m_nObjectState != embed::EmbedStates::LOADED && m_nObjectState != embed::EmbedStates::RUNNING )
            throw embed::WrongStateException(
                                    OUString( "The client site can not be set currently!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

        m_xClientSite = xClient;
    }
}


uno::Reference< embed::XEmbeddedClient > SAL_CALL OCommonEmbeddedObject::getClientSite()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    if ( m_bDisposed )
        throw lang::DisposedException(); 

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( OUString( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return m_xClientSite;
}


void SAL_CALL OCommonEmbeddedObject::update()
        throw ( embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); 

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( OUString( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    PostEvent_Impl( OUString( "OnVisAreaChanged" ) );
}


void SAL_CALL OCommonEmbeddedObject::setUpdateMode( sal_Int32 nMode )
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); 

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( OUString( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    OSL_ENSURE( nMode == embed::EmbedUpdateModes::ALWAYS_UPDATE
                    || nMode == embed::EmbedUpdateModes::EXPLICIT_UPDATE,
                "Unknown update mode!\n" );
    m_nUpdateMode = nMode;
}


sal_Int64 SAL_CALL OCommonEmbeddedObject::getStatus( sal_Int64 )
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    if ( m_bDisposed )
        throw lang::DisposedException(); 

    return m_nMiscStatus;
}


void SAL_CALL OCommonEmbeddedObject::setContainerName( const OUString& sName )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); 

    m_aContainerName = sName;
}

com::sun::star::uno::Reference< com::sun::star::uno::XInterface > SAL_CALL OCommonEmbeddedObject::getParent() throw (::com::sun::star::uno::RuntimeException)
{
    return m_xParent;
}

void SAL_CALL OCommonEmbeddedObject::setParent( const com::sun::star::uno::Reference< com::sun::star::uno::XInterface >& xParent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException)
{
    m_xParent = xParent;
    if ( m_nObjectState != -1 && m_nObjectState != embed::EmbedStates::LOADED )
    {
        uno::Reference < container::XChild > xChild( m_pDocHolder->GetComponent(), uno::UNO_QUERY );
        if ( xChild.is() )
            xChild->setParent( xParent );
    }
}


void SAL_CALL OCommonEmbeddedObject::setDefaultSize( const ::com::sun::star::awt::Size& rSize_100TH_MM ) throw (::com::sun::star::uno::RuntimeException)
{
    
    m_aDefaultSizeForChart_In_100TH_MM = rSize_100TH_MM;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
