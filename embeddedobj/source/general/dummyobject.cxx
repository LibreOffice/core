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
#include <com/sun/star/embed/XEmbeddedClient.hpp>
#include <com/sun/star/embed/XInplaceClient.hpp>
#include <com/sun/star/embed/XWindowSupplier.hpp>
#include <com/sun/star/embed/StateChangeInProgressException.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/EmbedMapUnits.hpp>
#include <com/sun/star/embed/EntryInitModes.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <cppuhelper/interfacecontainer.h>

#include <dummyobject.hxx>


using namespace ::com::sun::star;

//----------------------------------------------
void ODummyEmbeddedObject::CheckInit()
{
    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object has no persistence!\n" )),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
}

//----------------------------------------------
void ODummyEmbeddedObject::PostEvent_Impl( const ::rtl::OUString& aEventName )
{
    if ( m_pInterfaceContainer )
    {
        ::cppu::OInterfaceContainerHelper* pIC = m_pInterfaceContainer->getContainer(
                                            ::getCppuType((const uno::Reference< document::XEventListener >*)0) );
        if( pIC )
        {
            document::EventObject aEvent;
            aEvent.EventName = aEventName;
            aEvent.Source = uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) );
            // For now all the events are sent as object events
            // aEvent.Source = ( xSource.is() ? xSource
            //                     : uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) ) );
            ::cppu::OInterfaceIteratorHelper aIt( *pIC );
            while( aIt.hasMoreElements() )
            {
                try
                {
                    ((document::XEventListener *)aIt.next())->notifyEvent( aEvent );
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

//----------------------------------------------
ODummyEmbeddedObject::~ODummyEmbeddedObject()
{
}

//----------------------------------------------
void SAL_CALL ODummyEmbeddedObject::changeState( sal_Int32 nNewState )
        throw ( embed::UnreachableStateException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    if ( nNewState == embed::EmbedStates::LOADED )
        return;

    throw embed::UnreachableStateException();
}

//----------------------------------------------
uno::Sequence< sal_Int32 > SAL_CALL ODummyEmbeddedObject::getReachableStates()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    uno::Sequence< sal_Int32 > aResult( 1 );
    aResult[0] = embed::EmbedStates::LOADED;

    return aResult;
}

//----------------------------------------------
sal_Int32 SAL_CALL ODummyEmbeddedObject::getCurrentState()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    return m_nObjectState;
}

//----------------------------------------------
void SAL_CALL ODummyEmbeddedObject::doVerb( sal_Int32 )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                embed::UnreachableStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    // no supported verbs
}

//----------------------------------------------
uno::Sequence< embed::VerbDescriptor > SAL_CALL ODummyEmbeddedObject::getSupportedVerbs()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    return uno::Sequence< embed::VerbDescriptor >();
}

//----------------------------------------------
void SAL_CALL ODummyEmbeddedObject::setClientSite(
                const uno::Reference< embed::XEmbeddedClient >& xClient )
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    m_xClientSite = xClient;
}

//----------------------------------------------
uno::Reference< embed::XEmbeddedClient > SAL_CALL ODummyEmbeddedObject::getClientSite()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    return m_xClientSite;
}

//----------------------------------------------
void SAL_CALL ODummyEmbeddedObject::update()
        throw ( embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();
}

//----------------------------------------------
void SAL_CALL ODummyEmbeddedObject::setUpdateMode( sal_Int32 )
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();
}

//----------------------------------------------
sal_Int64 SAL_CALL ODummyEmbeddedObject::getStatus( sal_Int64 )
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    return 0;
}

//----------------------------------------------
void SAL_CALL ODummyEmbeddedObject::setContainerName( const ::rtl::OUString& )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();
}

//----------------------------------------------
void SAL_CALL ODummyEmbeddedObject::setVisualAreaSize( sal_Int64 nAspect, const awt::Size& aSize )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!\n" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Illegal call!\n" )),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    m_nCachedAspect = nAspect;
    m_aCachedSize = aSize;
    m_bHasCachedSize = sal_True;
}

//----------------------------------------------
awt::Size SAL_CALL ODummyEmbeddedObject::getVisualAreaSize( sal_Int64 nAspect )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!\n" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Illegal call!\n" )),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    if ( !m_bHasCachedSize || m_nCachedAspect != nAspect )
        throw embed::NoVisualAreaSizeException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "No size available!\n" ) ),
                uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return m_aCachedSize;
}

//----------------------------------------------
sal_Int32 SAL_CALL ODummyEmbeddedObject::getMapUnit( sal_Int64 nAspect )
        throw ( uno::Exception,
                uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!\n" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Illegal call!\n" )),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return embed::EmbedMapUnits::ONE_100TH_MM;
}

//----------------------------------------------
embed::VisualRepresentation SAL_CALL ODummyEmbeddedObject::getPreferredVisualRepresentation( sal_Int64 )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    // no representation can be retrieved
    throw embed::WrongStateException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Illegal call!\n" )),
                                uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
}

//----------------------------------------------
void SAL_CALL ODummyEmbeddedObject::setPersistentEntry(
                    const uno::Reference< embed::XStorage >& xStorage,
                    const ::rtl::OUString& sEntName,
                    sal_Int32 nEntryConnectionMode,
                    const uno::Sequence< beans::PropertyValue >& /* lArguments */,
                    const uno::Sequence< beans::PropertyValue >& /* lObjArgs */ )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "No parent storage is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Empty element name is provided!\n" )),
                                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                            2 );

    if ( ( m_nObjectState != -1 || nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
      && ( m_nObjectState == -1 || nEntryConnectionMode != embed::EntryInitModes::NO_INIT ) )
    {
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Can't change persistant representation of activated object!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    if ( m_bWaitSaveCompleted )
    {
        if ( nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
            saveCompleted( ( m_xParentStorage != xStorage || !m_aEntryName.equals( sEntName ) ) );
        else
            throw embed::WrongStateException(
                        ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
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
            throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Wrong entry is provided!\n" )),
                                uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                2 );

    }
    else
        throw lang::IllegalArgumentException( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "Wrong connection mode is provided!\n" )),
                                uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ),
                                3 );
}

//------------------------------------------------------
void SAL_CALL ODummyEmbeddedObject::storeToEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const ::rtl::OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& /* lArguments */,
                            const uno::Sequence< beans::PropertyValue >& /* lObjArgs */ )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    m_xParentStorage->copyElementTo( m_aEntryName, xStorage, sEntName );
}

//------------------------------------------------------
void SAL_CALL ODummyEmbeddedObject::storeAsEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const ::rtl::OUString& sEntName,
                            const uno::Sequence< beans::PropertyValue >& /* lArguments */,
                            const uno::Sequence< beans::PropertyValue >& /* lObjArgs */ )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    PostEvent_Impl( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "OnSaveAs" )) );

    m_xParentStorage->copyElementTo( m_aEntryName, xStorage, sEntName );

    m_bWaitSaveCompleted = sal_True;
    m_xNewParentStorage = xStorage;
    m_aNewEntryName = sEntName;
}

//------------------------------------------------------
void SAL_CALL ODummyEmbeddedObject::saveCompleted( sal_Bool bUseNew )
        throw ( embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

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

        PostEvent_Impl( ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "OnSaveAsDone" )) );
    }

    m_xNewParentStorage = uno::Reference< embed::XStorage >();
    m_aNewEntryName = ::rtl::OUString();
    m_bWaitSaveCompleted = sal_False;
}

//------------------------------------------------------
sal_Bool SAL_CALL ODummyEmbeddedObject::hasEntry()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    if ( !m_aEntryName.isEmpty() )
        return sal_True;

    return sal_False;
}

//------------------------------------------------------
::rtl::OUString SAL_CALL ODummyEmbeddedObject::getEntryName()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return m_aEntryName;
}

//------------------------------------------------------
void SAL_CALL ODummyEmbeddedObject::storeOwn()
        throw ( embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    // the object can not be activated or changed
    return;
}

//------------------------------------------------------
sal_Bool SAL_CALL ODummyEmbeddedObject::isReadonly()
        throw ( embed::WrongStateException,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    // this object can not be changed
    return sal_True;
}

//------------------------------------------------------
void SAL_CALL ODummyEmbeddedObject::reload(
                const uno::Sequence< beans::PropertyValue >& /* lArguments */,
                const uno::Sequence< beans::PropertyValue >& /* lObjArgs */ )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                io::IOException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "The object waits for saveCompleted() call!\n" )),
                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    // nothing to reload
}

//------------------------------------------------------
uno::Sequence< sal_Int8 > SAL_CALL ODummyEmbeddedObject::getClassID()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    // currently the class ID is empty
    // TODO/LATER: should a special class ID be used in this case?
    return uno::Sequence< sal_Int8 >();
}

//------------------------------------------------------
::rtl::OUString SAL_CALL ODummyEmbeddedObject::getClassName()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return ::rtl::OUString();
}

//------------------------------------------------------
void SAL_CALL ODummyEmbeddedObject::setClassInfo(
                const uno::Sequence< sal_Int8 >& /*aClassID*/, const ::rtl::OUString& /*aClassName*/ )
        throw ( lang::NoSupportException,
                uno::RuntimeException )
{
    throw lang::NoSupportException();
}

//------------------------------------------------------
uno::Reference< util::XCloseable > SAL_CALL ODummyEmbeddedObject::getComponent()
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit();

    return uno::Reference< util::XCloseable >();
}

//----------------------------------------------
void SAL_CALL ODummyEmbeddedObject::addStateChangeListener( const uno::Reference< embed::XStateChangeListener >& xListener )
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        return;

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex );

    m_pInterfaceContainer->addInterface( ::getCppuType( (const uno::Reference< embed::XStateChangeListener >*)0 ),
                                                        xListener );
}

//----------------------------------------------
void SAL_CALL ODummyEmbeddedObject::removeStateChangeListener(
                    const uno::Reference< embed::XStateChangeListener >& xListener )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( (const uno::Reference< embed::XStateChangeListener >*)0 ),
                                                xListener );
}

//----------------------------------------------
void SAL_CALL ODummyEmbeddedObject::close( sal_Bool bDeliverOwnership )
    throw ( util::CloseVetoException,
            uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    uno::Reference< uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >( this ) );
    lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );

    if ( m_pInterfaceContainer )
    {
        ::cppu::OInterfaceContainerHelper* pContainer =
            m_pInterfaceContainer->getContainer( ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
        if ( pContainer != NULL )
        {
            ::cppu::OInterfaceIteratorHelper pIterator(*pContainer);
            while (pIterator.hasMoreElements())
            {
                try
                {
                    ((util::XCloseListener*)pIterator.next())->queryClosing( aSource, bDeliverOwnership );
                }
                catch( const uno::RuntimeException& )
                {
                    pIterator.remove();
                }
            }
        }

        pContainer = m_pInterfaceContainer->getContainer(
                                    ::getCppuType( ( const uno::Reference< util::XCloseListener >*) NULL ) );
        if ( pContainer != NULL )
        {
            ::cppu::OInterfaceIteratorHelper pCloseIterator(*pContainer);
            while (pCloseIterator.hasMoreElements())
            {
                try
                {
                    ((util::XCloseListener*)pCloseIterator.next())->notifyClosing( aSource );
                }
                catch( const uno::RuntimeException& )
                {
                    pCloseIterator.remove();
                }
            }
        }

        m_pInterfaceContainer->disposeAndClear( aSource );
    }

    m_bDisposed = sal_True; // the object is disposed now for outside
}

//----------------------------------------------
void SAL_CALL ODummyEmbeddedObject::addCloseListener( const uno::Reference< util::XCloseListener >& xListener )
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        return;

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex );

    m_pInterfaceContainer->addInterface( ::getCppuType( (const uno::Reference< util::XCloseListener >*)0 ), xListener );
}

//----------------------------------------------
void SAL_CALL ODummyEmbeddedObject::removeCloseListener( const uno::Reference< util::XCloseListener >& xListener )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( (const uno::Reference< util::XCloseListener >*)0 ),
                                                xListener );
}

//------------------------------------------------------
void SAL_CALL ODummyEmbeddedObject::addEventListener( const uno::Reference< document::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        return;

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex );

    m_pInterfaceContainer->addInterface( ::getCppuType( (const uno::Reference< document::XEventListener >*)0 ), xListener );
}

//------------------------------------------------------
void SAL_CALL ODummyEmbeddedObject::removeEventListener( const uno::Reference< document::XEventListener >& xListener )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( (const uno::Reference< document::XEventListener >*)0 ),
                                                xListener );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
