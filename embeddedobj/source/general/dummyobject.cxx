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
#include <com/sun/star/embed/UnreachableStateException.hpp>
#include <com/sun/star/embed/XEmbeddedClient.hpp>
#include <com/sun/star/embed/XInplaceClient.hpp>
#include <com/sun/star/embed/XWindowSupplier.hpp>
#include <com/sun/star/embed/StateChangeInProgressException.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/EmbedMapUnits.hpp>
#include <com/sun/star/embed/EntryInitModes.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>

#include <dummyobject.hxx>


using namespace ::com::sun::star;


void ODummyEmbeddedObject::CheckInit_WrongState()
{
    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The object has no persistence!",
                                        static_cast< ::cppu::OWeakObject* >(this) );
}

void ODummyEmbeddedObject::CheckInit_Runtime()
{
    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( m_nObjectState == -1 )
        throw uno::RuntimeException( "The object has no persistence!",
                                     static_cast< ::cppu::OWeakObject* >(this) );
}
void ODummyEmbeddedObject::PostEvent_Impl( const OUString& aEventName )
{
    if ( m_pInterfaceContainer )
    {
        ::cppu::OInterfaceContainerHelper* pIC = m_pInterfaceContainer->getContainer(
                                            cppu::UnoType<document::XEventListener>::get());
        if( pIC )
        {
            document::EventObject aEvent;
            aEvent.EventName = aEventName;
            aEvent.Source.set( static_cast< ::cppu::OWeakObject* >( this ) );
            // For now all the events are sent as object events
            // aEvent.Source = ( xSource.is() ? xSource
            //                     : uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) ) );
            ::cppu::OInterfaceIteratorHelper aIt( *pIC );
            while( aIt.hasMoreElements() )
            {
                try
                {
                    static_cast<document::XEventListener *>(aIt.next())->notifyEvent( aEvent );
                }
                catch( const uno::RuntimeException& )
                {
                    aIt.remove();
                }

                // the listener could dispose the object.
                if ( m_bDisposed )
                    return;
            }
        }
    }
}


ODummyEmbeddedObject::~ODummyEmbeddedObject()
{
}


void SAL_CALL ODummyEmbeddedObject::changeState( sal_Int32 nNewState )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    if ( nNewState == embed::EmbedStates::LOADED )
        return;

    throw embed::UnreachableStateException();
}


uno::Sequence< sal_Int32 > SAL_CALL ODummyEmbeddedObject::getReachableStates()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    uno::Sequence< sal_Int32 > aResult( 1 );
    aResult[0] = embed::EmbedStates::LOADED;

    return aResult;
}


sal_Int32 SAL_CALL ODummyEmbeddedObject::getCurrentState()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    return m_nObjectState;
}


void SAL_CALL ODummyEmbeddedObject::doVerb( sal_Int32 )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    // no supported verbs
}


uno::Sequence< embed::VerbDescriptor > SAL_CALL ODummyEmbeddedObject::getSupportedVerbs()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    return uno::Sequence< embed::VerbDescriptor >();
}


void SAL_CALL ODummyEmbeddedObject::setClientSite(
                const uno::Reference< embed::XEmbeddedClient >& xClient )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    m_xClientSite = xClient;
}


uno::Reference< embed::XEmbeddedClient > SAL_CALL ODummyEmbeddedObject::getClientSite()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    return m_xClientSite;
}


void SAL_CALL ODummyEmbeddedObject::update()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();
}


void SAL_CALL ODummyEmbeddedObject::setUpdateMode( sal_Int32 )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();
}


sal_Int64 SAL_CALL ODummyEmbeddedObject::getStatus( sal_Int64 )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    return 0;
}


void SAL_CALL ODummyEmbeddedObject::setContainerName( const OUString& )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_Runtime();
}


void SAL_CALL ODummyEmbeddedObject::setVisualAreaSize( sal_Int64 nAspect, const awt::Size& aSize )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!\n" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( "Illegal call!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    m_nCachedAspect = nAspect;
    m_aCachedSize = aSize;
    m_bHasCachedSize = true;
}


awt::Size SAL_CALL ODummyEmbeddedObject::getVisualAreaSize( sal_Int64 nAspect )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!\n" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( "Illegal call!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    if ( !m_bHasCachedSize || m_nCachedAspect != nAspect )
        throw embed::NoVisualAreaSizeException(
                "No size available!",
                static_cast< ::cppu::OWeakObject* >(this) );

    return m_aCachedSize;
}


sal_Int32 SAL_CALL ODummyEmbeddedObject::getMapUnit( sal_Int64 nAspect )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_Runtime();

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!\n" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( "Illegal call!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    return embed::EmbedMapUnits::ONE_100TH_MM;
}


embed::VisualRepresentation SAL_CALL ODummyEmbeddedObject::getPreferredVisualRepresentation( sal_Int64 )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    // no representation can be retrieved
    throw embed::WrongStateException( "Illegal call!",
                                static_cast< ::cppu::OWeakObject* >(this) );
}


void SAL_CALL ODummyEmbeddedObject::setPersistentEntry(
                    const uno::Reference< embed::XStorage >& xStorage,
                    const OUString& sEntName,
                    sal_Int32 nEntryConnectionMode,
                    const uno::Sequence< beans::PropertyValue >& /* lArguments */,
                    const uno::Sequence< beans::PropertyValue >& /* lObjArgs */ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( "No parent storage is provided!",
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( "Empty element name is provided!",
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            2 );

    if ( ( m_nObjectState != -1 || nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
      && ( m_nObjectState == -1 || nEntryConnectionMode != embed::EntryInitModes::NO_INIT ) )
    {
        throw embed::WrongStateException(
                    "Can't change persistent representation of activated object!",
                    static_cast< ::cppu::OWeakObject* >(this) );
    }

    if ( m_bWaitSaveCompleted )
    {
        if ( nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
            saveCompleted( ( m_xParentStorage != xStorage || !m_aEntryName.equals( sEntName ) ) );
        else
            throw embed::WrongStateException(
                        "The object waits for saveCompleted() call!",
                        static_cast< ::cppu::OWeakObject* >(this) );
    }

    if ( nEntryConnectionMode == embed::EntryInitModes::DEFAULT_INIT
      || nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
    {
        if ( xStorage->hasByName( sEntName ) )

        {
            m_xParentStorage = xStorage;
            m_aEntryName = sEntName;
            m_nObjectState = embed::EmbedStates::LOADED;
        }
        else
            throw lang::IllegalArgumentException( "Wrong entry is provided!",
                                static_cast< ::cppu::OWeakObject* >(this),
                                2 );

    }
    else
        throw lang::IllegalArgumentException( "Wrong connection mode is provided!",
                                static_cast< ::cppu::OWeakObject* >(this),
                                3 );
}


void SAL_CALL ODummyEmbeddedObject::storeToEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& /* lArguments */,
                            const uno::Sequence< beans::PropertyValue >& /* lObjArgs */ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    "The object waits for saveCompleted() call!",
                    static_cast< ::cppu::OWeakObject* >(this) );

    m_xParentStorage->copyElementTo( m_aEntryName, xStorage, sEntName );
}


void SAL_CALL ODummyEmbeddedObject::storeAsEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& /* lArguments */,
                            const uno::Sequence< beans::PropertyValue >& /* lObjArgs */ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    "The object waits for saveCompleted() call!",
                    static_cast< ::cppu::OWeakObject* >(this) );

    PostEvent_Impl( "OnSaveAs" );

    m_xParentStorage->copyElementTo( m_aEntryName, xStorage, sEntName );

    m_bWaitSaveCompleted = true;
    m_xNewParentStorage = xStorage;
    m_aNewEntryName = sEntName;
}


void SAL_CALL ODummyEmbeddedObject::saveCompleted( sal_Bool bUseNew )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    // it is allowed to call saveCompleted( false ) for nonstored objects
    if ( !m_bWaitSaveCompleted && !bUseNew )
        return;

    OSL_ENSURE( m_bWaitSaveCompleted, "Unexpected saveCompleted() call!\n" );
    if ( !m_bWaitSaveCompleted )
        throw io::IOException(); // TODO: illegal call

    OSL_ENSURE( m_xNewParentStorage.is() , "Internal object information is broken!\n" );
    if ( !m_xNewParentStorage.is() )
        throw uno::RuntimeException(); // TODO: broken internal information

    if ( bUseNew )
    {
        m_xParentStorage = m_xNewParentStorage;
        m_aEntryName = m_aNewEntryName;

        PostEvent_Impl( "OnSaveAsDone" );
    }

    m_xNewParentStorage.clear();
    m_aNewEntryName.clear();
    m_bWaitSaveCompleted = false;
}


sal_Bool SAL_CALL ODummyEmbeddedObject::hasEntry()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    "The object waits for saveCompleted() call!",
                    static_cast< ::cppu::OWeakObject* >(this) );

    if ( !m_aEntryName.isEmpty() )
        return true;

    return false;
}


OUString SAL_CALL ODummyEmbeddedObject::getEntryName()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    "The object waits for saveCompleted() call!",
                    static_cast< ::cppu::OWeakObject* >(this) );

    return m_aEntryName;
}


void SAL_CALL ODummyEmbeddedObject::storeOwn()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    "The object waits for saveCompleted() call!",
                    static_cast< ::cppu::OWeakObject* >(this) );

    // the object can not be activated or changed
    return;
}


sal_Bool SAL_CALL ODummyEmbeddedObject::isReadonly()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    "The object waits for saveCompleted() call!",
                    static_cast< ::cppu::OWeakObject* >(this) );

    // this object can not be changed
    return true;
}


void SAL_CALL ODummyEmbeddedObject::reload(
                const uno::Sequence< beans::PropertyValue >& /* lArguments */,
                const uno::Sequence< beans::PropertyValue >& /* lObjArgs */ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    "The object waits for saveCompleted() call!",
                    static_cast< ::cppu::OWeakObject* >(this) );

    // nothing to reload
}


uno::Sequence< sal_Int8 > SAL_CALL ODummyEmbeddedObject::getClassID()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_Runtime();

    // currently the class ID is empty
    // TODO/LATER: should a special class ID be used in this case?
    return uno::Sequence< sal_Int8 >();
}


OUString SAL_CALL ODummyEmbeddedObject::getClassName()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return OUString();
}


void SAL_CALL ODummyEmbeddedObject::setClassInfo(
                const uno::Sequence< sal_Int8 >& /*aClassID*/, const OUString& /*aClassName*/ )
{
    throw lang::NoSupportException();
}


uno::Reference< util::XCloseable > SAL_CALL ODummyEmbeddedObject::getComponent()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_Runtime();

    return uno::Reference< util::XCloseable >();
}


void SAL_CALL ODummyEmbeddedObject::addStateChangeListener( const uno::Reference< embed::XStateChangeListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        return;

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer.reset(new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex ));

    m_pInterfaceContainer->addInterface( cppu::UnoType<embed::XStateChangeListener>::get(),
                                                        xListener );
}


void SAL_CALL ODummyEmbeddedObject::removeStateChangeListener(
                    const uno::Reference< embed::XStateChangeListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( cppu::UnoType<embed::XStateChangeListener>::get(),
                                                xListener );
}


void SAL_CALL ODummyEmbeddedObject::close( sal_Bool bDeliverOwnership )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    uno::Reference< uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >( this ) );
    lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );

    if ( m_pInterfaceContainer )
    {
        ::cppu::OInterfaceContainerHelper* pContainer =
            m_pInterfaceContainer->getContainer( cppu::UnoType<util::XCloseListener>::get());
        if ( pContainer != nullptr )
        {
            ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
            while (pIterator.hasMoreElements())
            {
                try
                {
                    static_cast<util::XCloseListener*>(pIterator.next())->queryClosing( aSource, bDeliverOwnership );
                }
                catch( const uno::RuntimeException& )
                {
                    pIterator.remove();
                }
            }
        }

        pContainer = m_pInterfaceContainer->getContainer(
                                    cppu::UnoType<util::XCloseListener>::get());
        if ( pContainer != nullptr )
        {
            ::cppu::OInterfaceIteratorHelper pCloseIterator(*pContainer);
            while (pCloseIterator.hasMoreElements())
            {
                try
                {
                    static_cast<util::XCloseListener*>(pCloseIterator.next())->notifyClosing( aSource );
                }
                catch( const uno::RuntimeException& )
                {
                    pCloseIterator.remove();
                }
            }
        }

        m_pInterfaceContainer->disposeAndClear( aSource );
    }

    m_bDisposed = true; // the object is disposed now for outside
}


void SAL_CALL ODummyEmbeddedObject::addCloseListener( const uno::Reference< util::XCloseListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        return;

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer.reset(new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex ));

    m_pInterfaceContainer->addInterface( cppu::UnoType<util::XCloseListener>::get(), xListener );
}


void SAL_CALL ODummyEmbeddedObject::removeCloseListener( const uno::Reference< util::XCloseListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( cppu::UnoType<util::XCloseListener>::get(),
                                                xListener );
}


void SAL_CALL ODummyEmbeddedObject::addEventListener( const uno::Reference< document::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        return;

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer.reset(new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex ));

    m_pInterfaceContainer->addInterface( cppu::UnoType<document::XEventListener>::get(), xListener );
}


void SAL_CALL ODummyEmbeddedObject::removeEventListener( const uno::Reference< document::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( cppu::UnoType<document::XEventListener>::get(),
                                                xListener );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
