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
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <framework/actiontriggerhelper.hxx>
#include <vcl/svapp.hxx>

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;

namespace framework
{

const css::uno::Sequence<sal_Int8>& RootActionTriggerContainer::getUnoTunnelId() noexcept
{
    static const sal_uInt8 pGUID[16] = { 0x17, 0x0F, 0xA2, 0xC9, 0xCA, 0x50, 0x4A, 0xD3, 0xA6, 0x3B, 0x39, 0x99, 0xC5, 0x96, 0x43, 0x27 };
    static css::uno::Sequence< sal_Int8 > seqID(reinterpret_cast<const sal_Int8*>(pGUID), 16);
    return seqID;
}

RootActionTriggerContainer::RootActionTriggerContainer(const css::uno::Reference<css::awt::XPopupMenu>& rMenu,
                                                       const OUString* pMenuIdentifier)
    : m_bContainerCreated(false)
    , m_xMenu(rMenu)
    , m_pMenuIdentifier(pMenuIdentifier)
{
}

RootActionTriggerContainer::~RootActionTriggerContainer()
{
}

// XInterface
Any SAL_CALL RootActionTriggerContainer::queryInterface( const Type& aType )
{
    Any a = ::cppu::queryInterface(
                aType ,
                static_cast< XMultiServiceFactory*   >(this),
                static_cast< XServiceInfo*           >(this),
                static_cast< XUnoTunnel*             >(this),
                static_cast< XTypeProvider*          >(this),
                static_cast< XNamed*                 >(this));

    if( a.hasValue() )
    {
        return a;
    }

    return PropertySetContainer::queryInterface( aType );
}

void SAL_CALL RootActionTriggerContainer::acquire() noexcept
{
    PropertySetContainer::acquire();
}

void SAL_CALL RootActionTriggerContainer::release() noexcept
{
    PropertySetContainer::release();
}

// XMultiServiceFactory
Reference< XInterface > SAL_CALL RootActionTriggerContainer::createInstance( const OUString& aServiceSpecifier )
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
{
    return createInstance( ServiceSpecifier );
}

Sequence< OUString > SAL_CALL RootActionTriggerContainer::getAvailableServiceNames()
{
    Sequence< OUString > aSeq{ SERVICENAME_ACTIONTRIGGER,
                               SERVICENAME_ACTIONTRIGGERCONTAINER,
                               SERVICENAME_ACTIONTRIGGERSEPARATOR };
    return aSeq;
}

// XIndexContainer
void SAL_CALL RootActionTriggerContainer::insertByIndex( sal_Int32 Index, const Any& Element )
{
    SolarMutexGuard g;

    if ( !m_bContainerCreated )
        FillContainer();

    PropertySetContainer::insertByIndex( Index, Element );
}

void SAL_CALL RootActionTriggerContainer::removeByIndex( sal_Int32 Index )
{
    SolarMutexGuard g;

    if ( !m_bContainerCreated )
        FillContainer();

    PropertySetContainer::removeByIndex( Index );
}

// XIndexReplace
void SAL_CALL RootActionTriggerContainer::replaceByIndex( sal_Int32 Index, const Any& Element )
{
    SolarMutexGuard g;

    if ( !m_bContainerCreated )
        FillContainer();

    PropertySetContainer::replaceByIndex( Index, Element );
}

// XIndexAccess
sal_Int32 SAL_CALL RootActionTriggerContainer::getCount()
{
    SolarMutexGuard g;

    if ( !m_bContainerCreated )
    {
        if ( m_xMenu )
        {
            SolarMutexGuard aSolarMutexGuard;
            return m_xMenu->getItemCount();
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
{
    SolarMutexGuard g;

    if ( !m_bContainerCreated )
        FillContainer();

    return PropertySetContainer::getByIndex( Index );
}

// XElementAccess
Type SAL_CALL RootActionTriggerContainer::getElementType()
{
    return cppu::UnoType<XPropertySet>::get();
}

sal_Bool SAL_CALL RootActionTriggerContainer::hasElements()
{
    if (m_xMenu)
    {
        SolarMutexGuard aSolarMutexGuard;
        return m_xMenu->getItemCount() > 0;
    }

    return false;
}

// XServiceInfo
OUString SAL_CALL RootActionTriggerContainer::getImplementationName()
{
    return IMPLEMENTATIONNAME_ROOTACTIONTRIGGERCONTAINER;
}

sal_Bool SAL_CALL RootActionTriggerContainer::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL RootActionTriggerContainer::getSupportedServiceNames()
{
    return { SERVICENAME_ACTIONTRIGGERCONTAINER };
}

// XUnoTunnel
sal_Int64 SAL_CALL RootActionTriggerContainer::getSomething( const Sequence< sal_Int8 >& aIdentifier )
{
    return comphelper::getSomethingImpl(aIdentifier, this);
}

// XTypeProvider
Sequence< Type > SAL_CALL RootActionTriggerContainer::getTypes()
{
    // Create a static typecollection ...
    static ::cppu::OTypeCollection ourTypeCollection(
                        cppu::UnoType<XMultiServiceFactory>::get(),
                        cppu::UnoType<XIndexContainer>::get(),
                        cppu::UnoType<XServiceInfo>::get(),
                        cppu::UnoType<XTypeProvider>::get(),
                        cppu::UnoType<XUnoTunnel>::get(),
                        cppu::UnoType<XNamed>::get());

    return ourTypeCollection.getTypes();
}

Sequence< sal_Int8 > SAL_CALL RootActionTriggerContainer::getImplementationId()
{
    return css::uno::Sequence<sal_Int8>();
}

// private implementation helper
void RootActionTriggerContainer::FillContainer()
{
    m_bContainerCreated = true;
    ActionTriggerHelper::FillActionTriggerContainerFromMenu(
        this, m_xMenu);
}
OUString RootActionTriggerContainer::getName()
{
    OUString sRet;
    if( m_pMenuIdentifier )
        sRet = *m_pMenuIdentifier;
    return sRet;
}

void RootActionTriggerContainer::setName( const OUString& )
{
    throw RuntimeException();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
