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

#include <vector>
#include <sortdynres.hxx>
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/ucb/ContentResultSetCapability.hpp>
#include <com/sun/star/ucb/ListActionType.hpp>
#include <com/sun/star/ucb/ServiceNotFoundException.hpp>
#include <com/sun/star/ucb/WelcomeDynamicResultSetStruct.hpp>
#include <com/sun/star/ucb/CachedDynamicResultSetStubFactory.hpp>
#include <com/sun/star/ucb/XSourceInitialization.hpp>
#include <ucbhelper/getcomponentcontext.hxx>

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace comphelper;


//  The mutex to synchronize access to containers.
static osl::Mutex& getContainerMutex()
{
    static osl::Mutex* pMutex = nullptr;
    if( !pMutex )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if( !pMutex )
        {
            static osl::Mutex aMutex;
            pMutex = &aMutex;
        }
    }

    return *pMutex;
}


// SortedDynamicResultSet

SortedDynamicResultSet::SortedDynamicResultSet(
                        const Reference < XDynamicResultSet > &xOriginal,
                        const Sequence < NumberedSortingInfo > &aOptions,
                        const Reference < XAnyCompareFactory > &xCompFac,
                        const Reference < XComponentContext > &rxContext )
{
    mpDisposeEventListeners = nullptr;
    mxOwnListener           = new SortedDynamicResultSetListener( this );

    mxOriginal  = xOriginal;
    maOptions   = aOptions;
    mxCompFac   = xCompFac;
    m_xContext  = rxContext;

    mbGotWelcome    = false;
    mbUseOne        = true;
    mbStatic        = false;
}


SortedDynamicResultSet::~SortedDynamicResultSet()
{
    mxOwnListener->impl_OwnerDies();
    mxOwnListener.clear();

    delete mpDisposeEventListeners;

    mxOne.clear();
    mxTwo.clear();
    mxOriginal.clear();
}

// XServiceInfo methods.

OUString SAL_CALL SortedDynamicResultSet::getImplementationName()
{
    return OUString( "com.sun.star.comp.ucb.SortedDynamicResultSet" );
}

sal_Bool SAL_CALL SortedDynamicResultSet::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL SortedDynamicResultSet::getSupportedServiceNames()
{
    return { DYNAMIC_RESULTSET_SERVICE_NAME };
}

// XComponent methods.

void SAL_CALL SortedDynamicResultSet::dispose()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpDisposeEventListeners && mpDisposeEventListeners->getLength() )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XComponent * >( this );
        mpDisposeEventListeners->disposeAndClear( aEvt );
    }

    mxOne.clear();
    mxTwo.clear();
    mxOriginal.clear();

    mbUseOne = true;
}

void SAL_CALL SortedDynamicResultSet::addEventListener(
                            const Reference< XEventListener >& Listener )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpDisposeEventListeners )
        mpDisposeEventListeners =
                    new OInterfaceContainerHelper2( getContainerMutex() );

    mpDisposeEventListeners->addInterface( Listener );
}

void SAL_CALL SortedDynamicResultSet::removeEventListener(
                            const Reference< XEventListener >& Listener )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpDisposeEventListeners )
        mpDisposeEventListeners->removeInterface( Listener );
}


// XDynamicResultSet methods.

Reference< XResultSet > SAL_CALL
SortedDynamicResultSet::getStaticResultSet()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mxListener.is() )
        throw ListenerAlreadySetException();

    mbStatic = true;

    if ( mxOriginal.is() )
    {
        mxOne = new SortedResultSet( mxOriginal->getStaticResultSet() );
        mxOne->Initialize( maOptions, mxCompFac );
    }

    return mxOne.get();
}


void SAL_CALL
SortedDynamicResultSet::setListener( const Reference< XDynamicResultSetListener >& Listener )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mxListener.is() )
        throw ListenerAlreadySetException();

    addEventListener( Reference< XEventListener >::query( Listener ) );

    mxListener = Listener;

    if ( mxOriginal.is() )
        mxOriginal->setListener( mxOwnListener.get() );
}


void SAL_CALL
SortedDynamicResultSet::connectToCache( const Reference< XDynamicResultSet > & xCache )
{
    if( mxListener.is() )
        throw ListenerAlreadySetException();

    if( mbStatic )
        throw ListenerAlreadySetException();

    Reference< XSourceInitialization > xTarget( xCache, UNO_QUERY );
    if( xTarget.is() && m_xContext.is() )
    {
        Reference< XCachedDynamicResultSetStubFactory > xStubFactory;
        try
        {
            xStubFactory = CachedDynamicResultSetStubFactory::create( m_xContext );
        }
        catch ( Exception const & )
        {
        }

        if( xStubFactory.is() )
        {
            xStubFactory->connectToCache(
                  this, xCache, Sequence< NumberedSortingInfo > (), nullptr );
            return;
        }
    }
    throw ServiceNotFoundException();
}


sal_Int16 SAL_CALL SortedDynamicResultSet::getCapabilities()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    sal_Int16 nCaps = 0;

    if ( mxOriginal.is() )
        nCaps = mxOriginal->getCapabilities();

    nCaps |= ContentResultSetCapability::SORTED;

    return nCaps;
}


// XDynamicResultSetListener methods.


/** In the first notify-call the listener gets the two
 <type>XResultSet</type>s and has to hold them. The <type>XResultSet</type>s
 are implementations of the service <type>ContentResultSet</type>.

 <p>The notified new <type>XResultSet</type> will stay valid after returning
 notification. The old one will become invalid after returning notification.

 <p>While in notify-call the listener is allowed to read old and new version,
 except in the first call, where only the new Resultset is valid.

 <p>The Listener is allowed to blockade this call, until he really want to go
 to the new version. The only situation, where the listener has to return the
 update call at once is, while he disposes his broadcaster or while he is
 removing himself as listener (otherwise you deadlock)!!!
*/
void SortedDynamicResultSet::impl_notify( const ListEvent& Changes )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    bool bHasNew = false;
    bool bHasModified = false;

    SortedResultSet *pCurSet = nullptr;

    // exchange mxNew and mxOld and immediately afterwards copy the tables
    // from Old to New
    if ( mbGotWelcome )
    {
        if ( mbUseOne )
        {
            mbUseOne = false;
            mxTwo->CopyData( mxOne.get() );
            pCurSet = mxTwo.get();
        }
        else
        {
            mbUseOne = true;
            mxOne->CopyData( mxTwo.get() );
            pCurSet = mxOne.get();
        }
    }

    if (!pCurSet)
        return;

    Any  aRet;

    try {
        aRet = pCurSet->getPropertyValue("IsRowCountFinal");
    }
    catch (const UnknownPropertyException&) {}
    catch (const WrappedTargetException&) {}

    long nOldCount = pCurSet->GetCount();
    bool bWasFinal = false;

    aRet >>= bWasFinal;

    // handle the actions in the list
    for ( long i=0; i<Changes.Changes.getLength(); i++ )
    {
        const ListAction aAction = Changes.Changes[i];
        switch ( aAction.ListActionType )
        {
            case ListActionType::WELCOME:
                {
                    WelcomeDynamicResultSetStruct aWelcome;
                    if ( aAction.ActionInfo >>= aWelcome )
                    {
                        mxTwo = new SortedResultSet( aWelcome.Old );
                        mxOne = new SortedResultSet( aWelcome.New );
                        mxOne->Initialize( maOptions, mxCompFac );
                        mbGotWelcome = true;
                        mbUseOne = true;
                        pCurSet = mxOne.get();

                        aWelcome.Old = mxTwo.get();
                        aWelcome.New = mxOne.get();

                        ListAction *pWelcomeAction = new ListAction;
                        pWelcomeAction->ActionInfo <<= aWelcome;
                        pWelcomeAction->Position = 0;
                        pWelcomeAction->Count = 0;
                        pWelcomeAction->ListActionType = ListActionType::WELCOME;

                        maActions.Insert( pWelcomeAction );
                    }
                    else
                    {
                        // throw RuntimeException();
                    }
                    break;
                }
            case ListActionType::INSERTED:
                {
                    pCurSet->InsertNew( aAction.Position, aAction.Count );
                    bHasNew = true;
                    break;
                }
            case ListActionType::REMOVED:
                {
                    pCurSet->Remove( aAction.Position,
                                     aAction.Count,
                                     &maActions );
                    break;
                }
            case ListActionType::MOVED:
                {
                    long nOffset = 0;
                    if ( aAction.ActionInfo >>= nOffset )
                    {
                        pCurSet->Move( aAction.Position,
                                       aAction.Count,
                                       nOffset );
                    }
                    break;
                }
            case ListActionType::PROPERTIES_CHANGED:
                {
                    pCurSet->SetChanged( aAction.Position, aAction.Count );
                    bHasModified = true;
                    break;
                }
            default: break;
        }
    }

    if ( bHasModified )
        pCurSet->ResortModified( &maActions );

    if ( bHasNew )
        pCurSet->ResortNew( &maActions );

    // send the new actions with a notify to the listeners
    SendNotify();

    // check for propertyChangeEvents
    pCurSet->CheckProperties( nOldCount, bWasFinal );
}

// XEventListener

void SortedDynamicResultSet::impl_disposing( const EventObject& )
{
    mxListener.clear();
    mxOriginal.clear();
}

// private methods

void SortedDynamicResultSet::SendNotify()
{
    long nCount = maActions.Count();

    if ( nCount && mxListener.is() )
    {
        Sequence< ListAction > aActionList( maActions.Count() );
        ListAction *pActionList = aActionList.getArray();

        for ( long i=0; i<nCount; i++ )
        {
            pActionList[ i ] = *(maActions.GetAction( i ));
        }

        ListEvent aNewEvent;
        aNewEvent.Changes = aActionList;

        mxListener->notify( aNewEvent );
    }

    // clean up
    maActions.Clear();
}

// SortedDynamicResultSetFactory

SortedDynamicResultSetFactory::SortedDynamicResultSetFactory(
                        const Reference< XComponentContext > & rxContext )
{
    m_xContext = rxContext;
}


SortedDynamicResultSetFactory::~SortedDynamicResultSetFactory()
{
}


// XServiceInfo methods.

OUString SAL_CALL SortedDynamicResultSetFactory::getImplementationName()
{
    return getImplementationName_Static();
}

OUString SortedDynamicResultSetFactory::getImplementationName_Static()
{
    return OUString( "com.sun.star.comp.ucb.SortedDynamicResultSetFactory" );
}

sal_Bool SAL_CALL SortedDynamicResultSetFactory::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL SortedDynamicResultSetFactory::getSupportedServiceNames()
{
    return getSupportedServiceNames_Static();
}

/// @throws css::uno::Exception
static css::uno::Reference< css::uno::XInterface > SAL_CALL
SortedDynamicResultSetFactory_CreateInstance( const css::uno::Reference<
                                              css::lang::XMultiServiceFactory> & rSMgr )
{
    css::lang::XServiceInfo* pX = static_cast<css::lang::XServiceInfo*>(
        new SortedDynamicResultSetFactory( ucbhelper::getComponentContext(rSMgr) ));
    return css::uno::Reference< css::uno::XInterface >::query( pX );
}

css::uno::Sequence< OUString > SortedDynamicResultSetFactory::getSupportedServiceNames_Static()
{
    css::uno::Sequence<OUString> aSNS { DYNAMIC_RESULTSET_FACTORY_NAME };
    return aSNS;
}


// Service factory implementation.
css::uno::Reference< css::lang::XSingleServiceFactory >
SortedDynamicResultSetFactory::createServiceFactory( const css::uno::Reference< css::lang::XMultiServiceFactory >& rxServiceMgr )
{
    return css::uno::Reference< css::lang::XSingleServiceFactory >(
            cppu::createOneInstanceFactory(
                    rxServiceMgr,
                    SortedDynamicResultSetFactory::getImplementationName_Static(),
                    SortedDynamicResultSetFactory_CreateInstance,
                    SortedDynamicResultSetFactory::getSupportedServiceNames_Static() ) );
}

// SortedDynamicResultSetFactory methods.

Reference< XDynamicResultSet > SAL_CALL
SortedDynamicResultSetFactory::createSortedDynamicResultSet(
                const Reference< XDynamicResultSet > & Source,
                const Sequence< NumberedSortingInfo > & Info,
                const Reference< XAnyCompareFactory > & CompareFactory )
{
    Reference< XDynamicResultSet > xRet;
    xRet = new SortedDynamicResultSet( Source, Info, CompareFactory, m_xContext );
    return xRet;
}

// EventList

void EventList::Clear()
{
    for (ListAction* p : maData)
    {
        delete p;
    }

    maData.clear();
}

void EventList::AddEvent( sal_IntPtr nType, sal_IntPtr nPos )
{
    ListAction *pAction = new ListAction;
    pAction->Position = nPos;
    pAction->Count = 1;
    pAction->ListActionType = nType;

    Insert( pAction );
}

// SortedDynamicResultSetListener

SortedDynamicResultSetListener::SortedDynamicResultSetListener(
                                SortedDynamicResultSet *mOwner )
{
    mpOwner = mOwner;
}


SortedDynamicResultSetListener::~SortedDynamicResultSetListener()
{
}

// XEventListener ( base of XDynamicResultSetListener )

void SAL_CALL
SortedDynamicResultSetListener::disposing( const EventObject& Source )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpOwner )
        mpOwner->impl_disposing( Source );
}


// XDynamicResultSetListener

void SAL_CALL
SortedDynamicResultSetListener::notify( const ListEvent& Changes )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpOwner )
        mpOwner->impl_notify( Changes );
}

// own methods:

void SAL_CALL
SortedDynamicResultSetListener::impl_OwnerDies()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    mpOwner = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
