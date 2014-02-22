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


#include <classes/rootactiontriggercontainer.hxx>
#include <classes/actiontriggercontainer.hxx>
#include <classes/actiontriggerpropertyset.hxx>
#include <classes/actiontriggerseparatorpropertyset.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <framework/actiontriggerhelper.hxx>
#include <osl/mutex.hxx>
#include <threadhelp/resetableguard.hxx>
#include <vcl/svapp.hxx>

using namespace cppu;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::container;
using namespace com::sun::star::beans;


namespace framework
{

static Sequence< sal_Int8 > impl_getStaticIdentifier()
{
    static const sal_uInt8 pGUID[16] = { 0x17, 0x0F, 0xA2, 0xC9, 0xCA, 0x50, 0x4A, 0xD3, 0xA6, 0x3B, 0x39, 0x99, 0xC5, 0x96, 0x43, 0x27 };
    static ::com::sun::star::uno::Sequence< sal_Int8 > seqID((const sal_Int8*)pGUID,16) ;
    return seqID ;
}

RootActionTriggerContainer::RootActionTriggerContainer( const Menu* pMenu, const OUString* pMenuIdentifier ) :
    PropertySetContainer()
    ,   m_bContainerCreated( sal_False )
    ,   m_bContainerChanged( sal_False )
    ,   m_bInContainerCreation( sal_False )
    ,   m_pMenu( pMenu )
    ,   m_pMenuIdentifier( pMenuIdentifier )
{
}

RootActionTriggerContainer::~RootActionTriggerContainer()
{
}


Any SAL_CALL RootActionTriggerContainer::queryInterface( const Type& aType )
throw ( RuntimeException )
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


Reference< XInterface > SAL_CALL RootActionTriggerContainer::createInstance( const OUString& aServiceSpecifier )
throw ( Exception,  RuntimeException )
{
    if ( aServiceSpecifier.equalsAscii( SERVICENAME_ACTIONTRIGGER ))
        return (OWeakObject *)( new ActionTriggerPropertySet());
    else if ( aServiceSpecifier.equalsAscii( SERVICENAME_ACTIONTRIGGERCONTAINER ))
        return (OWeakObject *)( new ActionTriggerContainer());
    else if ( aServiceSpecifier.equalsAscii( SERVICENAME_ACTIONTRIGGERSEPARATOR ))
        return (OWeakObject *)( new ActionTriggerSeparatorPropertySet());
    else
        throw com::sun::star::uno::RuntimeException("Unknown service specifier!", (OWeakObject *)this );
}

Reference< XInterface > SAL_CALL RootActionTriggerContainer::createInstanceWithArguments( const OUString& ServiceSpecifier, const Sequence< Any >& /*Arguments*/ )
throw ( Exception, RuntimeException )
{
    return createInstance( ServiceSpecifier );
}

Sequence< OUString > SAL_CALL RootActionTriggerContainer::getAvailableServiceNames()
throw ( RuntimeException )
{
    Sequence< OUString > aSeq( 3 );

    aSeq[0] = OUString( SERVICENAME_ACTIONTRIGGER );
    aSeq[1] = OUString( SERVICENAME_ACTIONTRIGGERCONTAINER );
    aSeq[2] = OUString( SERVICENAME_ACTIONTRIGGERSEPARATOR );

    return aSeq;
}


void SAL_CALL RootActionTriggerContainer::insertByIndex( sal_Int32 Index, const Any& Element )
throw ( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if ( !m_bContainerCreated )
        FillContainer();

    if ( !m_bInContainerCreation )
        m_bContainerChanged = sal_True;
    PropertySetContainer::insertByIndex( Index, Element );
}

void SAL_CALL RootActionTriggerContainer::removeByIndex( sal_Int32 Index )
throw ( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if ( !m_bContainerCreated )
        FillContainer();

    if ( !m_bInContainerCreation )
        m_bContainerChanged = sal_True;
    PropertySetContainer::removeByIndex( Index );
}


void SAL_CALL RootActionTriggerContainer::replaceByIndex( sal_Int32 Index, const Any& Element )
throw ( IllegalArgumentException, IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if ( !m_bContainerCreated )
        FillContainer();

    if ( !m_bInContainerCreation )
        m_bContainerChanged = sal_True;
    PropertySetContainer::replaceByIndex( Index, Element );
}


sal_Int32 SAL_CALL RootActionTriggerContainer::getCount()
throw ( RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

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
throw ( IndexOutOfBoundsException, WrappedTargetException, RuntimeException )
{
    ResetableGuard aGuard( m_aLock );

    if ( !m_bContainerCreated )
        FillContainer();

    return PropertySetContainer::getByIndex( Index );
}


Type SAL_CALL RootActionTriggerContainer::getElementType()
    throw (::com::sun::star::uno::RuntimeException)
{
    return ::getCppuType(( Reference< XPropertySet >*)0);
}

sal_Bool SAL_CALL RootActionTriggerContainer::hasElements()
throw (::com::sun::star::uno::RuntimeException)
{
    if ( m_pMenu )
    {
        SolarMutexGuard aSolarMutexGuard;
        return ( m_pMenu->GetItemCount() > 0 );
    }

    return sal_False;
}


OUString SAL_CALL RootActionTriggerContainer::getImplementationName()
throw ( RuntimeException )
{
    return OUString( IMPLEMENTATIONNAME_ROOTACTIONTRIGGERCONTAINER );
}

sal_Bool SAL_CALL RootActionTriggerContainer::supportsService( const OUString& ServiceName )
throw ( RuntimeException )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL RootActionTriggerContainer::getSupportedServiceNames()
throw ( RuntimeException )
{
    Sequence< OUString > seqServiceNames( 1 );

    seqServiceNames[0] = OUString( SERVICENAME_ACTIONTRIGGERCONTAINER );
    return seqServiceNames;
}


sal_Int64 SAL_CALL RootActionTriggerContainer::getSomething( const Sequence< sal_Int8 >& aIdentifier ) throw ( RuntimeException )
{
    if ( aIdentifier == impl_getStaticIdentifier() )
        return reinterpret_cast< sal_Int64 >( this );
    else
        return 0;
}


Sequence< Type > SAL_CALL RootActionTriggerContainer::getTypes() throw ( RuntimeException )
{
    
    
    
    static ::cppu::OTypeCollection* pTypeCollection = NULL ;

    if ( pTypeCollection == NULL )
    {
        
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        
        if ( pTypeCollection == NULL )
        {
            
            static ::cppu::OTypeCollection aTypeCollection(
                        ::getCppuType(( const Reference< XMultiServiceFactory   >*)NULL ) ,
                        ::getCppuType(( const Reference< XIndexContainer        >*)NULL ) ,
                        ::getCppuType(( const Reference< XServiceInfo           >*)NULL ) ,
                        ::getCppuType(( const Reference< XTypeProvider          >*)NULL ) ,
                        ::getCppuType(( const Reference< XUnoTunnel             >*)NULL ) ,
                        ::getCppuType(( const Reference< XNamed                 >*)NULL )) ;

            
            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes() ;
}

Sequence< sal_Int8 > SAL_CALL RootActionTriggerContainer::getImplementationId() throw ( RuntimeException )
{
    
    

    
    
    
    static ::cppu::OImplementationId* pID = NULL ;

    if ( pID == NULL )
    {
        
        osl::MutexGuard aGuard( osl::Mutex::getGlobalMutex() ) ;

        
        if ( pID == NULL )
        {
            
            static ::cppu::OImplementationId aID( false ) ;
            
            pID = &aID ;
        }
    }

    return pID->getImplementationId() ;
}


void RootActionTriggerContainer::FillContainer()
{
    m_bContainerCreated = sal_True;
    m_bInContainerCreation = sal_True;
    Reference<XIndexContainer> xXIndexContainer( (OWeakObject *)this, UNO_QUERY );
    ActionTriggerHelper::FillActionTriggerContainerFromMenu(
        xXIndexContainer,
        m_pMenu );
    m_bInContainerCreation = sal_False;
}
OUString RootActionTriggerContainer::getName() throw ( RuntimeException )
{
    OUString sRet;
    if( m_pMenuIdentifier )
        sRet = *m_pMenuIdentifier;
    return sRet;
}

void RootActionTriggerContainer::setName( const OUString& ) throw ( RuntimeException)
{
    throw RuntimeException();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
