/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
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
#include <com/sun/star/embed/UnreachableStateException.hpp>
#include <com/sun/star/embed/WrongStateException.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/EmbedMapUnits.hpp>
#include <com/sun/star/embed/EntryInitModes.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>

#include <comphelper/multicontainer2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <osl/diagnose.h>
#include <dummyobject.hxx>


using namespace ::com::sun::star;


void ODummyEmbeddedObject::CheckInit_WrongState()
{
    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( u"The object has no persistence!"_ustr,
                                        static_cast< ::cppu::OWeakObject* >(this) );
}

void ODummyEmbeddedObject::CheckInit_Runtime()
{
    if ( m_bDisposed )
        throw lang::DisposedException();

    if ( m_nObjectState == -1 )
        throw uno::RuntimeException( u"The object has no persistence!"_ustr,
                                     static_cast< ::cppu::OWeakObject* >(this) );
}
void ODummyEmbeddedObject::PostEvent_Impl( const OUString& aEventName )
{
    if ( !m_pInterfaceContainer )
        return;

    comphelper::OInterfaceContainerHelper2* pIC = m_pInterfaceContainer->getContainer(
                                        cppu::UnoType<document::XEventListener>::get());
    if( !pIC )
        return;

    document::EventObject aEvent;
    aEvent.EventName = aEventName;
    aEvent.Source.set( static_cast< ::cppu::OWeakObject* >( this ) );
    // For now all the events are sent as object events
    // aEvent.Source = ( xSource.is() ? xSource
    //                     : uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >( this ) ) );
    comphelper::OInterfaceIteratorHelper2 aIt( *pIC );
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


ODummyEmbeddedObject::~ODummyEmbeddedObject()
{
}


void ODummyEmbeddedObject::changeState( sal_Int32 nNewState )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    if ( nNewState == embed::EmbedStates::LOADED )
        return;

    throw embed::UnreachableStateException();
}


cpo::uno::Sequence< sal_Int32 > ODummyEmbeddedObject::getReachableStates()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    return { embed::EmbedStates::LOADED };
}


sal_Int32 ODummyEmbeddedObject::getCurrentState()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    return m_nObjectState;
}


void ODummyEmbeddedObject::doVerb( sal_Int32 )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    // no supported verbs
}


cpo::uno::Sequence< embed::VerbDescriptor > ODummyEmbeddedObject::getSupportedVerbs()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    return cpo::uno::Sequence< embed::VerbDescriptor >();
}


void ODummyEmbeddedObject::setClientSite(
                const uno::Reference< embed::XEmbeddedClient >& xClient )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    m_xClientSite = xClient;
}


uno::Reference< embed::XEmbeddedClient > ODummyEmbeddedObject::getClientSite()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    return m_xClientSite;
}


void ODummyEmbeddedObject::update()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();
}


void ODummyEmbeddedObject::setUpdateMode( sal_Int32 )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();
}


sal_Int64 ODummyEmbeddedObject::getStatus( sal_Int64 )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    return 0;
}


void ODummyEmbeddedObject::setContainerName( const OUString& )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_Runtime();
}


void ODummyEmbeddedObject::setVisualAreaSize( sal_Int64 nAspect, const awt::Size& aSize )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( u"Illegal call!"_ustr,
                                    static_cast< ::cppu::OWeakObject* >(this) );

    m_nCachedAspect = nAspect;
    m_aCachedSize = aSize;
    m_bHasCachedSize = true;
}


awt::Size ODummyEmbeddedObject::getVisualAreaSize( sal_Int64 nAspect )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( u"Illegal call!"_ustr,
                                    static_cast< ::cppu::OWeakObject* >(this) );

    if ( !m_bHasCachedSize || m_nCachedAspect != nAspect )
        throw embed::NoVisualAreaSizeException(
                u"No size available!"_ustr,
                static_cast< ::cppu::OWeakObject* >(this) );

    return m_aCachedSize;
}


sal_Int32 ODummyEmbeddedObject::getMapUnit( sal_Int64 nAspect )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_Runtime();

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( u"Illegal call!"_ustr,
                                    static_cast< ::cppu::OWeakObject* >(this) );

    return embed::EmbedMapUnits::ONE_100TH_MM;
}


embed::VisualRepresentation ODummyEmbeddedObject::getPreferredVisualRepresentation( sal_Int64 )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    // no representation can be retrieved
    throw embed::WrongStateException( u"Illegal call!"_ustr,
                                static_cast< ::cppu::OWeakObject* >(this) );
}


void ODummyEmbeddedObject::setPersistentEntry(
                    const uno::Reference< embed::XStorage >& xStorage,
                    const OUString& sEntName,
                    sal_Int32 nEntryConnectionMode,
                    const cpo::uno::Sequence< beans::PropertyValue >& /* lArguments */,
                    const cpo::uno::Sequence< beans::PropertyValue >& /* lObjArgs */ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !xStorage.is() )
        throw lang::IllegalArgumentException( u"No parent storage is provided!"_ustr,
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            1 );

    if ( sEntName.isEmpty() )
        throw lang::IllegalArgumentException( u"Empty element name is provided!"_ustr,
                                            static_cast< ::cppu::OWeakObject* >(this),
                                            2 );

    if ( ( m_nObjectState != -1 || nEntryConnectionMode == embed::EntryInitModes::NO_INIT )
      && ( m_nObjectState == -1 || nEntryConnectionMode != embed::EntryInitModes::NO_INIT ) )
    {
        throw embed::WrongStateException(
                    u"Can't change persistent representation of activated object!"_ustr,
                    static_cast< ::cppu::OWeakObject* >(this) );
    }

    if ( m_bWaitSaveCompleted )
    {
        if ( nEntryConnectionMode != embed::EntryInitModes::NO_INIT )
            throw embed::WrongStateException(
                        u"The object waits for saveCompleted() call!"_ustr,
                        static_cast< ::cppu::OWeakObject* >(this) );

        saveCompleted( m_xParentStorage != xStorage || m_aEntryName != sEntName );

    }

    if ( nEntryConnectionMode != embed::EntryInitModes::DEFAULT_INIT
        && nEntryConnectionMode != embed::EntryInitModes::NO_INIT )
        throw lang::IllegalArgumentException( u"Wrong connection mode is provided!"_ustr,
                                static_cast< ::cppu::OWeakObject* >(this),
                                3 );

    if ( !xStorage->hasByName( sEntName ) )
        throw lang::IllegalArgumentException( u"Wrong entry is provided!"_ustr,
                            static_cast< ::cppu::OWeakObject* >(this),
                            2 );

    m_xParentStorage = xStorage;
    m_aEntryName = sEntName;
    m_nObjectState = embed::EmbedStates::LOADED;
}


void ODummyEmbeddedObject::storeToEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const OUString& sEntName,
                            const cpo::uno::Sequence< beans::PropertyValue >& /* lArguments */,
                            const cpo::uno::Sequence< beans::PropertyValue >& /* lObjArgs */ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    u"The object waits for saveCompleted() call!"_ustr,
                    static_cast< ::cppu::OWeakObject* >(this) );

    m_xParentStorage->copyElementTo( m_aEntryName, xStorage, sEntName );
}


void ODummyEmbeddedObject::storeAsEntry( const uno::Reference< embed::XStorage >& xStorage,
                            const OUString& sEntName,
                            const cpo::uno::Sequence< beans::PropertyValue >& /* lArguments */,
                            const cpo::uno::Sequence< beans::PropertyValue >& /* lObjArgs */ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    u"The object waits for saveCompleted() call!"_ustr,
                    static_cast< ::cppu::OWeakObject* >(this) );

    PostEvent_Impl( u"OnSaveAs"_ustr );

    m_xParentStorage->copyElementTo( m_aEntryName, xStorage, sEntName );

    m_bWaitSaveCompleted = true;
    m_xNewParentStorage = xStorage;
    m_aNewEntryName = sEntName;
}


void ODummyEmbeddedObject::saveCompleted( bool bUseNew )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    // it is allowed to call saveCompleted( false ) for nonstored objects
    if ( !m_bWaitSaveCompleted && !bUseNew )
        return;

    OSL_ENSURE( m_bWaitSaveCompleted, "Unexpected saveCompleted() call!" );
    if ( !m_bWaitSaveCompleted )
        throw io::IOException(); // TODO: illegal call

    OSL_ENSURE( m_xNewParentStorage.is() , "Internal object information is broken!" );
    if ( !m_xNewParentStorage.is() )
        throw uno::RuntimeException(); // TODO: broken internal information

    if ( bUseNew )
    {
        m_xParentStorage = m_xNewParentStorage;
        m_aEntryName = m_aNewEntryName;

        PostEvent_Impl( u"OnSaveAsDone"_ustr );
    }

    m_xNewParentStorage.clear();
    m_aNewEntryName.clear();
    m_bWaitSaveCompleted = false;
}


bool ODummyEmbeddedObject::hasEntry()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    u"The object waits for saveCompleted() call!"_ustr,
                    static_cast< ::cppu::OWeakObject* >(this) );

    if ( !m_aEntryName.isEmpty() )
        return true;

    return false;
}


OUString ODummyEmbeddedObject::getEntryName()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    u"The object waits for saveCompleted() call!"_ustr,
                    static_cast< ::cppu::OWeakObject* >(this) );

    return m_aEntryName;
}


void ODummyEmbeddedObject::storeOwn()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    u"The object waits for saveCompleted() call!"_ustr,
                    static_cast< ::cppu::OWeakObject* >(this) );

    // the object can not be activated or changed
}


bool ODummyEmbeddedObject::isReadonly()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    u"The object waits for saveCompleted() call!"_ustr,
                    static_cast< ::cppu::OWeakObject* >(this) );

    // this object can not be changed
    return true;
}


void ODummyEmbeddedObject::reload(
                const cpo::uno::Sequence< beans::PropertyValue >& /* lArguments */,
                const cpo::uno::Sequence< beans::PropertyValue >& /* lObjArgs */ )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_WrongState();

    if ( m_bWaitSaveCompleted )
        throw embed::WrongStateException(
                    u"The object waits for saveCompleted() call!"_ustr,
                    static_cast< ::cppu::OWeakObject* >(this) );

    // nothing to reload
}


cpo::uno::Sequence< sal_Int8 > ODummyEmbeddedObject::getClassID()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_Runtime();

    // currently the class ID is empty
    // TODO/LATER: should a special class ID be used in this case?
    return cpo::uno::Sequence< sal_Int8 >();
}


OUString ODummyEmbeddedObject::getClassName()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    return OUString();
}


void ODummyEmbeddedObject::setClassInfo(
                const cpo::uno::Sequence< sal_Int8 >& /*aClassID*/, const OUString& /*aClassName*/ )
{
    throw lang::NoSupportException();
}


uno::Reference< util::XCloseable > ODummyEmbeddedObject::getComponent()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    CheckInit_Runtime();

    return uno::Reference< util::XCloseable >();
}


void ODummyEmbeddedObject::addStateChangeListener( const uno::Reference< embed::XStateChangeListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        return;

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer.reset(new comphelper::OMultiTypeInterfaceContainerHelper2( m_aMutex ));

    m_pInterfaceContainer->addInterface( cppu::UnoType<embed::XStateChangeListener>::get(),
                                                        xListener );
}


void ODummyEmbeddedObject::removeStateChangeListener(
                    const uno::Reference< embed::XStateChangeListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( cppu::UnoType<embed::XStateChangeListener>::get(),
                                                xListener );
}


void ODummyEmbeddedObject::close( bool bDeliverOwnership )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    uno::Reference< uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >( this ) );
    lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );

    if ( m_pInterfaceContainer )
    {
        comphelper::OInterfaceContainerHelper2* pContainer =
            m_pInterfaceContainer->getContainer( cppu::UnoType<util::XCloseListener>::get());
        if ( pContainer != nullptr )
        {
            comphelper::OInterfaceIteratorHelper2 pIterator(*pContainer);
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
            comphelper::OInterfaceIteratorHelper2 pCloseIterator(*pContainer);
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


void ODummyEmbeddedObject::addCloseListener( const uno::Reference< util::XCloseListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        return;

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer.reset(new comphelper::OMultiTypeInterfaceContainerHelper2( m_aMutex ));

    m_pInterfaceContainer->addInterface( cppu::UnoType<util::XCloseListener>::get(), xListener );
}


void ODummyEmbeddedObject::removeCloseListener( const uno::Reference< util::XCloseListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( cppu::UnoType<util::XCloseListener>::get(),
                                                xListener );
}


void ODummyEmbeddedObject::addEventListener( const uno::Reference< document::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        return;

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer.reset(new comphelper::OMultiTypeInterfaceContainerHelper2( m_aMutex ));

    m_pInterfaceContainer->addInterface( cppu::UnoType<document::XEventListener>::get(), xListener );
}


void ODummyEmbeddedObject::removeEventListener( const uno::Reference< document::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( cppu::UnoType<document::XEventListener>::get(),
                                                xListener );
}

OUString ODummyEmbeddedObject::getImplementationName()
{
    return u"com.sun.star.comp.embed.ODummyEmbeddedObject"_ustr;
}

bool ODummyEmbeddedObject::supportsService(const OUString& ServiceName)
{
    return cppu::supportsService(this, ServiceName);
}

cpo::uno::Sequence<OUString> ODummyEmbeddedObject::getSupportedServiceNames()
{
    return { u"com.sun.star.comp.embed.ODummyEmbeddedObject"_ustr };
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
