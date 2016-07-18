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

#include <classes/rootactiontriggercontainer.hxx>
#include <classes/actiontriggercontainer.hxx>
#include <classes/actiontriggerpropertyset.hxx>
#include <classes/actiontriggerseparatorpropertyset.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <framework/actiontriggerhelper.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;

namespace framework
{

static Sequence< sal_Int8 > const & impl_getStaticIdentifier()
{
    static const sal_uInt8 pGUID[16] = { 0x17, 0x0F, 0xA2, 0xC9, 0xCA, 0x50, 0x4A, 0xD3, 0xA6, 0x3B, 0x39, 0x99, 0xC5, 0x96, 0x43, 0x27 };
    static css::uno::Sequence< sal_Int8 > seqID(reinterpret_cast<const sal_Int8*>(pGUID), 16);
    return seqID;
}

RootActionTriggerContainer::RootActionTriggerContainer( const Menu* pMenu, const OUString* pMenuIdentifier ) :
    PropertySetContainer()
    ,   m_bContainerCreated( false )
    ,   m_bContainerChanged( false )
    ,   m_bInContainerCreation( false )
    ,   m_pMenu( pMenu )
    ,   m_pMenuIdentifier( pMenuIdentifier )
{
}

RootActionTriggerContainer::~RootActionTriggerContainer()
{
}

// XInterface
Any SAL_CALL RootActionTriggerContainer::queryInterface( const Type& aType )
throw ( RuntimeException, std::exception )
{
    Any a = ::cppu::queryInterface(
                aType ,
                (static_cast< XMultiServiceFactory*   >(this)),
                (static_cast< XServiceInfo*           >(this)),
                (static_cast< XUnoTunnel*             >(this)),
                (static_cast< XTypeProvider*          >(this)),
                (static_cast< XNamed*                 >(this)));

    if( a.hasValue() )
    {
        return a;
    }

    return PropertySetContainer::queryInterface( aType );
}

void SAL_CALL RootActionTriggerContainer::acquire() throw ()
{
    PropertySetContainer::acquire();
}

void SAL_CALL RootActionTriggerContainer::release() throw ()
{
    PropertySetContainer::release();
}

// XMultiServiceFactory
Reference< XInterface > SAL_CALL RootActionTriggerContainer::createInstance( const OUString& aServiceSpecifier )
throw ( Exception,  RuntimeException, std::exception )
{
    if ( aServiceSpecifier == SERVICENAME_ACTIONTRIGGER )
        return static_cast<OWeakObject *>( new ActionTriggerPropertySet());
    else if ( aServiceSpecifier == SERVICENAME_ACTIONTRIGGERCONTAINER )
        return static_cast<OWeakObject *>( new ActionTriggerContainer());
    else if ( aServiceSpecifier == SERVICENAME_ACTIONTRIGGERSEPARATOR )
        return static_cast<OWeakObject *>( new ActionTriggerSeparatorPropertySet());
    else
        throw css::uno::RuntimeException("Unknown service specifier!", static_cast<OWeakObject *>(this) );
}

Reference< XInterface > SAL_CALL RootActionTriggerContainer::createInstanceWithArguments( const OUString& ServiceSpecifier, const Sequence< Any >& /*Arguments*/ )
throw ( Exception, RuntimeException, std::exception )
{
    return createInstance( ServiceSpecifier );
}

Sequence< OUString > SAL_CALL RootActionTriggerContainer::getAvailableServiceNames()
throw ( RuntimeException, std::exception )
{
    Sequence< OUString > aSeq( 3 );

    aSeq[0] = SERVICENAME_ACTIONTRIGGER;
    aSeq[1] = SERVICENAME_ACTIONTRIGGERCONTAINER;
    aSeq[2] = SERVICENAME_ACTIONTRIGGERSEPARATOR;

    return aSeq;
}

// XIndexContainer
void SAL_CALL RootActionTriggerContainer::insertByIndex( sal_Int32 Index, const Any& Element )
throw ( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception )
{
    SolarMutexGuard g;

    if ( !m_bContainerCreated )
        FillContainer();

    if ( !m_bInContainerCreation )
        m_bContainerChanged = true;
    PropertySetContainer::insertByIndex( Index, Element );
}

void SAL_CALL RootActionTriggerContainer::removeByIndex( sal_Int32 Index )
throw ( IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception )
{
    SolarMutexGuard g;

    if ( !m_bContainerCreated )
        FillContainer();

    if ( !m_bInContainerCreation )
        m_bContainerChanged = true;
    PropertySetContainer::removeByIndex( Index );
}

// XIndexReplace
void SAL_CALL RootActionTriggerContainer::replaceByIndex( sal_Int32 Index, const Any& Element )
throw ( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception )
{
    SolarMutexGuard g;

    if ( !m_bContainerCreated )
        FillContainer();

    if ( !m_bInContainerCreation )
        m_bContainerChanged = true;
    PropertySetContainer::replaceByIndex( Index, Element );
}

// XIndexAccess
sal_Int32 SAL_CALL RootActionTriggerContainer::getCount()
throw ( RuntimeException, std::exception )
{
    SolarMutexGuard g;

    if ( !m_bContainerCreated )
    {
        if ( m_pMenu )
        {
            SolarMutexGuard aSolarMutexGuard;
            return m_pMenu->GetItemCount();
        }
        else
            return 0;
    }
    else
    {
        return PropertySetContainer::getCount();
    }
}

Any SAL_CALL RootActionTriggerContainer::getByIndex( sal_Int32 Index )
throw ( IndexOutOfBoundsException, WrappedTargetException, RuntimeException, std::exception )
{
    SolarMutexGuard g;

    if ( !m_bContainerCreated )
        FillContainer();

    return PropertySetContainer::getByIndex( Index );
}

// XElementAccess
Type SAL_CALL RootActionTriggerContainer::getElementType()
    throw (css::uno::RuntimeException, std::exception)
{
    return cppu::UnoType<XPropertySet>::get();
}

sal_Bool SAL_CALL RootActionTriggerContainer::hasElements()
throw (css::uno::RuntimeException, std::exception)
{
    if ( m_pMenu )
    {
        SolarMutexGuard aSolarMutexGuard;
        return ( m_pMenu->GetItemCount() > 0 );
    }

    return false;
}

// XServiceInfo
OUString SAL_CALL RootActionTriggerContainer::getImplementationName()
throw ( RuntimeException, std::exception )
{
    return OUString( IMPLEMENTATIONNAME_ROOTACTIONTRIGGERCONTAINER );
}

sal_Bool SAL_CALL RootActionTriggerContainer::supportsService( const OUString& ServiceName )
throw ( RuntimeException, std::exception )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL RootActionTriggerContainer::getSupportedServiceNames()
throw ( RuntimeException, std::exception )
{
    Sequence< OUString > seqServiceNames { SERVICENAME_ACTIONTRIGGERCONTAINER };
    return seqServiceNames;
}

// XUnoTunnel
sal_Int64 SAL_CALL RootActionTriggerContainer::getSomething( const Sequence< sal_Int8 >& aIdentifier ) throw ( RuntimeException, std::exception )
{
    if ( aIdentifier == impl_getStaticIdentifier() )
        return reinterpret_cast< sal_Int64 >( this );
    else
        return 0;
}

// XTypeProvider
Sequence< Type > SAL_CALL RootActionTriggerContainer::getTypes() throw ( RuntimeException, std::exception )
{
    // Optimize this method !
    // We initialize a static variable only one time. And we don't must use a mutex at every call!
    // For the first call; pTypeCollection is NULL - for the second call pTypeCollection is different from NULL!
    static ::cppu::OTypeCollection* pTypeCollection = nullptr;

    if ( pTypeCollection == nullptr )
    {
        // Ready for multithreading; get global mutex for first call of this method only! see before
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() );

        // Control these pointer again ... it can be, that another instance will be faster then these!
        if ( pTypeCollection == nullptr )
        {
            // Create a static typecollection ...
            static ::cppu::OTypeCollection aTypeCollection(
                        cppu::UnoType<XMultiServiceFactory>::get(),
                        cppu::UnoType<XIndexContainer>::get(),
                        cppu::UnoType<XServiceInfo>::get(),
                        cppu::UnoType<XTypeProvider>::get(),
                        cppu::UnoType<XUnoTunnel>::get(),
                        cppu::UnoType<XNamed>::get());

            // ... and set his address to static pointer!
            pTypeCollection = &aTypeCollection;
        }
    }

    return pTypeCollection->getTypes();
}

Sequence< sal_Int8 > SAL_CALL RootActionTriggerContainer::getImplementationId() throw ( RuntimeException, std::exception )
{
    return css::uno::Sequence<sal_Int8>();
}

// private implementation helper
void RootActionTriggerContainer::FillContainer()
{
    m_bContainerCreated = true;
    m_bInContainerCreation = true;
    Reference<XIndexContainer> xXIndexContainer( static_cast<OWeakObject *>(this), UNO_QUERY );
    ActionTriggerHelper::FillActionTriggerContainerFromMenu(
        xXIndexContainer,
        m_pMenu );
    m_bInContainerCreation = false;
}
OUString RootActionTriggerContainer::getName() throw ( RuntimeException, std::exception )
{
    OUString sRet;
    if( m_pMenuIdentifier )
        sRet = *m_pMenuIdentifier;
    return sRet;
}

void RootActionTriggerContainer::setName( const OUString& ) throw ( RuntimeException, std::exception)
{
    throw RuntimeException();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
