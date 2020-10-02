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

#include "sortdynres.hxx"
#include <comphelper/interfacecontainer2.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/weak.hxx>
#include <com/sun/star/ucb/ContentResultSetCapability.hpp>
#include <com/sun/star/ucb/ListActionType.hpp>
#include <com/sun/star/ucb/ListenerAlreadySetException.hpp>
#include <com/sun/star/ucb/ServiceNotFoundException.hpp>
#include <com/sun/star/ucb/WelcomeDynamicResultSetStruct.hpp>
#include <com/sun/star/ucb/CachedDynamicResultSetStubFactory.hpp>
#include <com/sun/star/ucb/XSourceInitialization.hpp>

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace comphelper;


//  The mutex to synchronize access to containers.
static osl::Mutex& getContainerMutex()
{
    static osl::Mutex ourMutex;

    return ourMutex;
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

    mpDisposeEventListeners.reset();

    mxOne.clear();
    mxTwo.clear();
    mxOriginal.clear();
}

// XServiceInfo methods.

OUString SAL_CALL SortedDynamicResultSet::getImplementationName()
{
    return "com.sun.star.comp.ucb.SortedDynamicResultSet";
}

sal_Bool SAL_CALL SortedDynamicResultSet::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL SortedDynamicResultSet::getSupportedServiceNames()
{
    return { "com.sun.star.ucb.SortedDynamicResultSet" };
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
        mpDisposeEventListeners.reset(
                    new OInterfaceContainerHelper2( getContainerMutex() ) );

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
    for ( const ListAction& aAction : Changes.Changes )
    {
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

                        std::unique_ptr<ListAction> pWelcomeAction(new ListAction);
                        pWelcomeAction->ActionInfo <<= aWelcome;
                        pWelcomeAction->Position = 0;
                        pWelcomeAction->Count = 0;
                        pWelcomeAction->ListActionType = ListActionType::WELCOME;

                        maActions.Insert( std::move(pWelcomeAction) );
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

void SortedDynamicResultSet::impl_disposing()
{
    mxListener.clear();
    mxOriginal.clear();
}

// private methods

void SortedDynamicResultSet::SendNotify()
{
    sal_Int32 nCount = maActions.Count();

    if ( nCount && mxListener.is() )
    {
        Sequence< ListAction > aActionList( maActions.Count() );
        ListAction *pActionList = aActionList.getArray();

        for ( sal_Int32 i=0; i<nCount; i++ )
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
    return "com.sun.star.comp.ucb.SortedDynamicResultSetFactory";
}

sal_Bool SAL_CALL SortedDynamicResultSetFactory::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService( this, ServiceName );
}

css::uno::Sequence< OUString > SAL_CALL SortedDynamicResultSetFactory::getSupportedServiceNames()
{
    return { "com.sun.star.ucb.SortedDynamicResultSetFactory" };
}


extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
ucb_SortedDynamicResultSetFactory_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(static_cast<cppu::OWeakObject*>(new SortedDynamicResultSetFactory(context)));
}

// SortedDynamicResultSetFactory methods.

Reference< XDynamicResultSet > SAL_CALL
SortedDynamicResultSetFactory::createSortedDynamicResultSet(
                const Reference< XDynamicResultSet > & Source,
                const Sequence< NumberedSortingInfo > & Info,
                const Reference< XAnyCompareFactory > & CompareFactory )
{
    Reference< XDynamicResultSet > xRet = new SortedDynamicResultSet( Source, Info, CompareFactory, m_xContext );
    return xRet;
}

// EventList

void EventList::Clear()
{
    maData.clear();
}

void EventList::AddEvent( sal_IntPtr nType, sal_IntPtr nPos )
{
    std::unique_ptr<ListAction> pAction(new ListAction);
    pAction->Position = nPos;
    pAction->Count = 1;
    pAction->ListActionType = nType;

    Insert( std::move(pAction) );
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
SortedDynamicResultSetListener::disposing( const EventObject& /*Source*/ )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpOwner )
        mpOwner->impl_disposing();
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

void
SortedDynamicResultSetListener::impl_OwnerDies()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    mpOwner = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
