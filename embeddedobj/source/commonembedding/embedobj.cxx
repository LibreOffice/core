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

#include <rtl/logfile.hxx>

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

//----------------------------------------------
sal_Int32 OCommonEmbeddedObject::ConvertVerbToState_Impl( sal_Int32 nVerb )
{
    for ( sal_Int32 nInd = 0; nInd < m_aVerbTable.getLength(); nInd++ )
        if ( m_aVerbTable[nInd][0] == nVerb )
            return m_aVerbTable[nInd][1];

    throw lang::IllegalArgumentException(); // TODO: unexpected verb provided
}

//----------------------------------------------
void OCommonEmbeddedObject::Deactivate()
{
    uno::Reference< util::XModifiable > xModif( m_pDocHolder->GetComponent(), uno::UNO_QUERY );

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
                ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The client could not store the object!" )),
                uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) ),
                uno::makeAny( e ) );
        }
    }

    m_pDocHolder->CloseFrame();

    xClientSite->visibilityChanged( sal_False );
}

//----------------------------------------------
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

            // should be locked after the method is finished successfully
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
                    // even if the listener complains ignore it for now
                   }

                if ( m_bDisposed )
                    return;
            }

            rGuard.reset();
        }
    }
}

//----------------------------------------------
void OCommonEmbeddedObject::SwitchStateTo_Impl( sal_Int32 nNextState )
{
    // TODO: may be needs interaction handler to detect wherether the object state
    //         can be changed even after errors

    if ( m_nObjectState == embed::EmbedStates::LOADED )
    {
        if ( nNextState == embed::EmbedStates::RUNNING )
        {
            // after the object reaches the running state the cloned size is not necessary any more
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
                    // in case embedded object is in loaded state the contents must
                    // be stored in the related storage and the storage
                    // must be created already
                    if ( !m_xObjectStorage.is() )
                        throw io::IOException(); //TODO: access denied

                    m_pDocHolder->SetComponent( LoadDocumentFromStorage_Impl(), m_bReadOnly );
                }
                else
                {
                    // objects without persistence will be initialized internally
                    uno::Sequence < uno::Any > aArgs(1);
                    aArgs[0] <<= uno::Reference < embed::XEmbeddedObject >( this );
                    uno::Reference< util::XCloseable > xDocument(
                            m_xFactory->createInstanceWithArguments( GetDocumentServiceName(), aArgs ), uno::UNO_QUERY );

                    uno::Reference < container::XChild > xChild( xDocument, uno::UNO_QUERY );
                    if ( xChild.is() )
                        xChild->setParent( m_xParent );

                    m_pDocHolder->SetComponent( xDocument, m_bReadOnly );
                }
            }

            if ( !m_pDocHolder->GetComponent().is() )
                throw embed::UnreachableStateException(); //TODO: can't open document

            m_nObjectState = nNextState;
        }
        else
        {
            OSL_FAIL( "Unacceptable state switch!\n" );
            throw uno::RuntimeException(); // TODO
        }
    }
    else if ( m_nObjectState == embed::EmbedStates::RUNNING )
    {
        if ( nNextState == embed::EmbedStates::LOADED )
        {
            m_nClonedMapUnit = m_pDocHolder->GetMapUnit( embed::Aspects::MSOLE_CONTENT );
            m_bHasClonedSize = m_pDocHolder->GetExtent( embed::Aspects::MSOLE_CONTENT, &m_aClonedSize );

            // actually frame should not exist at this point
            m_pDocHolder->CloseDocument( sal_False, sal_False );

            m_nObjectState = nNextState;
        }
        else
        {
            if ( nNextState == embed::EmbedStates::INPLACE_ACTIVE )
            {
                if ( !m_xClientSite.is() )
                    throw embed::WrongStateException(
                        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "client site not set, yet" ) ),
                        *this
                );

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
                    sal_Bool bOk = m_pDocHolder->ShowInplace( xClientWindowPeer, aRectangleToShow, xContainerDP );
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
                m_pDocHolder->Show();

                m_xClientSite->visibilityChanged( sal_True );
                m_nObjectState = nNextState;
            }
            else
            {
                OSL_FAIL( "Unacceptable state switch!\n" );
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
                // TODO:
                uno::Reference< ::com::sun::star::frame::XLayoutManager > xContainerLM =
                            xInplaceClient->getLayoutManager();
                if ( xContainerLM.is() )
                {
                    // dispatch provider may not be provided
                    uno::Reference< frame::XDispatchProvider > xContainerDP = xInplaceClient->getInplaceDispatchProvider();

                    // get the container module name
                    ::rtl::OUString aModuleName;
                    try
                    {
                        uno::Reference< embed::XComponentSupplier > xCompSupl( m_xClientSite, uno::UNO_QUERY_THROW );
                        uno::Reference< uno::XInterface > xContDoc( xCompSupl->getComponent(), uno::UNO_QUERY_THROW );

                        uno::Reference< frame::XModuleManager2 > xManager( frame::ModuleManager::create(comphelper::getComponentContext(m_xFactory)) );

                        aModuleName = xManager->identify( xContDoc );
                    }
                    catch( const uno::Exception& )
                    {}

                    // if currently another object is UIactive it will be deactivated; usually this will activate the LM of
                    // the container. Locking the LM will prevent flicker.
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
                        throw embed::WrongStateException(); //TODO: can't activate UI
                    }
                }
                else
                    throw embed::WrongStateException(); //TODO: can't activate UI
            }
        }
        else
        {
            OSL_FAIL( "Unacceptable state switch!\n" );
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
            OSL_FAIL( "Unacceptable state switch!\n" );
            throw uno::RuntimeException(); // TODO
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
                throw embed::WrongStateException(); //TODO: can't activate UI
        }
    }
    else
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object is in unacceptable state!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
}

//----------------------------------------------
uno::Sequence< sal_Int32 > OCommonEmbeddedObject::GetIntermediateStatesSequence_Impl( sal_Int32 nNewState )
{
    sal_Int32 nCurInd = 0;
    for ( nCurInd = 0; nCurInd < m_aAcceptedStates.getLength(); nCurInd++ )
        if ( m_aAcceptedStates[nCurInd] == m_nObjectState )
            break;

    if ( nCurInd == m_aAcceptedStates.getLength() )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object is in unacceptable state!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    sal_Int32 nDestInd = 0;
    for ( nDestInd = 0; nDestInd < m_aAcceptedStates.getLength(); nDestInd++ )
        if ( m_aAcceptedStates[nDestInd] == nNewState )
            break;

    if ( nDestInd == m_aAcceptedStates.getLength() )
        throw embed::UnreachableStateException(
            ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The state either not reachable, or the object allows the state only as an intermediate one!\n" )),
            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
            m_nObjectState,
            nNewState );

    return m_pIntermediateStatesSeqs[nCurInd][nDestInd];
}

//----------------------------------------------
void SAL_CALL OCommonEmbeddedObject::changeState( sal_Int32 nNewState )
        throw ( embed::UnreachableStateException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OCommonEmbeddedObject::changeState" );

    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ), uno::UNO_QUERY);
    {
        ::osl::ResettableMutexGuard aGuard( m_aMutex );
        if ( m_bDisposed )
            throw lang::DisposedException(); // TODO

        if ( m_nObjectState == -1 )
            throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

        sal_Int32 nOldState = m_nObjectState;

        if ( m_nTargetState != -1 )
        {
            // means that the object is currently trying to reach the target state
            throw embed::StateChangeInProgressException( ::rtl::OUString(),
                                                        uno::Reference< uno::XInterface >(),
                                                        m_nTargetState );
        }
        else
        {
            TargetStateControl_Impl aControl( m_nTargetState, nNewState );

            // in case the object is already in requested state
            if ( m_nObjectState == nNewState )
            {
                // if active object is activated again, bring it's window to top
                if ( m_nObjectState == embed::EmbedStates::ACTIVE )
                    m_pDocHolder->Show();

                return;
            }

            // retrieve sequence of states that should be passed to reach desired state
            uno::Sequence< sal_Int32 > aIntermediateStates = GetIntermediateStatesSequence_Impl( nNewState );

            // notify listeners that the object is going to change the state
            StateChangeNotification_Impl( sal_True, nOldState, nNewState,aGuard );

            try {
                for ( sal_Int32 nInd = 0; nInd < aIntermediateStates.getLength(); nInd++ )
                    SwitchStateTo_Impl( aIntermediateStates[nInd] );

                SwitchStateTo_Impl( nNewState );
            }
            catch( const uno::Exception& )
            {
                if ( nOldState != m_nObjectState )
                    // notify listeners that the object has changed the state
                    StateChangeNotification_Impl( sal_False, nOldState, m_nObjectState, aGuard );

                throw;
            }
        }

        // notify listeners that the object has changed the state
        StateChangeNotification_Impl( sal_False, nOldState, nNewState, aGuard );

        // let the object window be shown
        if ( nNewState == embed::EmbedStates::UI_ACTIVE || nNewState == embed::EmbedStates::INPLACE_ACTIVE )
            PostEvent_Impl( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OnVisAreaChanged" ) ) );
    }
}

//----------------------------------------------
uno::Sequence< sal_Int32 > SAL_CALL OCommonEmbeddedObject::getReachableStates()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return m_aAcceptedStates;
}

//----------------------------------------------
sal_Int32 SAL_CALL OCommonEmbeddedObject::getCurrentState()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return m_nObjectState;
}

//----------------------------------------------
void SAL_CALL OCommonEmbeddedObject::doVerb( sal_Int32 nVerbID )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                embed::UnreachableStateException,
                uno::Exception,
                uno::RuntimeException )
{
    RTL_LOGFILE_CONTEXT( aLog, "embeddedobj (mv76033) OCommonEmbeddedObject::doVerb" );

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

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

//----------------------------------------------
uno::Sequence< embed::VerbDescriptor > SAL_CALL OCommonEmbeddedObject::getSupportedVerbs()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return m_aObjectVerbs;
}

//----------------------------------------------
void SAL_CALL OCommonEmbeddedObject::setClientSite(
                const uno::Reference< embed::XEmbeddedClient >& xClient )
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_xClientSite != xClient)
    {
        if ( m_nObjectState != embed::EmbedStates::LOADED && m_nObjectState != embed::EmbedStates::RUNNING )
            throw embed::WrongStateException(
                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The client site can not be set currently!\n" )),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

        m_xClientSite = xClient;
    }
}

//----------------------------------------------
uno::Reference< embed::XEmbeddedClient > SAL_CALL OCommonEmbeddedObject::getClientSite()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return m_xClientSite;
}

//----------------------------------------------
void SAL_CALL OCommonEmbeddedObject::update()
        throw ( embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    PostEvent_Impl( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "OnVisAreaChanged" ) ) );
}

//----------------------------------------------
void SAL_CALL OCommonEmbeddedObject::setUpdateMode( sal_Int32 nMode )
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    OSL_ENSURE( nMode == embed::EmbedUpdateModes::ALWAYS_UPDATE
                    || nMode == embed::EmbedUpdateModes::EXPLICIT_UPDATE,
                "Unknown update mode!\n" );
    m_nUpdateMode = nMode;
}

//----------------------------------------------
sal_Int64 SAL_CALL OCommonEmbeddedObject::getStatus( sal_Int64 )
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return m_nMiscStatus;
}

//----------------------------------------------
void SAL_CALL OCommonEmbeddedObject::setContainerName( const ::rtl::OUString& sName )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

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

// XDefaultSizeTransmitter
void SAL_CALL OCommonEmbeddedObject::setDefaultSize( const ::com::sun::star::awt::Size& rSize_100TH_MM ) throw (::com::sun::star::uno::RuntimeException)
{
    //#i103460# charts do not necessaryly have an own size within ODF files, in this case they need to use the size settings from the surrounding frame, which is made available with this method
    m_aDefaultSizeForChart_In_100TH_MM = rSize_100TH_MM;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
