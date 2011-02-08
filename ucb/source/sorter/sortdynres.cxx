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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_ucb.hxx"

#include <vector>
#include <sortdynres.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/ucb/ContentResultSetCapability.hpp>
#include <com/sun/star/ucb/ListActionType.hpp>
#include <com/sun/star/ucb/WelcomeDynamicResultSetStruct.hpp>
#include <com/sun/star/ucb/XCachedDynamicResultSetStubFactory.hpp>
#include <com/sun/star/ucb/XSourceInitialization.hpp>

//-----------------------------------------------------------------------------
using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace rtl;

//=========================================================================

//  The mutex to synchronize access to containers.
static osl::Mutex& getContainerMutex()
{
    static osl::Mutex* pMutex = NULL;
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

//=========================================================================
//
// SortedDynamicResultSet
//
//=========================================================================

SortedDynamicResultSet::SortedDynamicResultSet(
                        const Reference < XDynamicResultSet > &xOriginal,
                        const Sequence < NumberedSortingInfo > &aOptions,
                        const Reference < XAnyCompareFactory > &xCompFac,
                        const Reference < XMultiServiceFactory > &xSMgr )
{
    mpDisposeEventListeners = NULL;
    mpOwnListener           = new SortedDynamicResultSetListener( this );

    mxOwnListener = Reference< XDynamicResultSetListener >( mpOwnListener );

    mxOriginal  = xOriginal;
    maOptions   = aOptions;
    mxCompFac   = xCompFac;
    mxSMgr      = xSMgr;

    mpOne = NULL;
    mpTwo = NULL;

    mbGotWelcome    = sal_False;
    mbUseOne        = sal_True;
    mbStatic        = sal_False;
}

//--------------------------------------------------------------------------
SortedDynamicResultSet::~SortedDynamicResultSet()
{
    mpOwnListener->impl_OwnerDies();
    mxOwnListener.clear();

    delete mpDisposeEventListeners;

    mxOne.clear();
    mxTwo.clear();
    mxOriginal.clear();

    mpOne = NULL;
    mpTwo = NULL;
}

//--------------------------------------------------------------------------
// XInterface methods.
//--------------------------------------------------------------------------

XINTERFACE_IMPL_4( SortedDynamicResultSet,
                   XTypeProvider,
                   XServiceInfo,
                   XComponent,      /* base class of XDynamicResultSet */
                   XDynamicResultSet );

//--------------------------------------------------------------------------
// XTypeProvider methods.
//--------------------------------------------------------------------------

XTYPEPROVIDER_IMPL_3( SortedDynamicResultSet,
                      XTypeProvider,
                         XServiceInfo,
                      XDynamicResultSet );

//--------------------------------------------------------------------------
// XServiceInfo methods.
//--------------------------------------------------------------------------

XSERVICEINFO_NOFACTORY_IMPL_1( SortedDynamicResultSet,
                                   OUString(RTL_CONSTASCII_USTRINGPARAM(
                                   "com.sun.star.comp.ucb.SortedDynamicResultSet" )),
                                   OUString(RTL_CONSTASCII_USTRINGPARAM(
                                   DYNAMIC_RESULTSET_SERVICE_NAME )) );

//--------------------------------------------------------------------------
// XComponent methods.
//--------------------------------------------------------------------------
void SAL_CALL SortedDynamicResultSet::dispose()
    throw( RuntimeException )
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

    mpOne = NULL;
    mpTwo = NULL;
    mbUseOne = sal_True;
}

//--------------------------------------------------------------------------
void SAL_CALL SortedDynamicResultSet::addEventListener(
                            const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( !mpDisposeEventListeners )
        mpDisposeEventListeners =
                    new OInterfaceContainerHelper( getContainerMutex() );

    mpDisposeEventListeners->addInterface( Listener );
}

//--------------------------------------------------------------------------
void SAL_CALL SortedDynamicResultSet::removeEventListener(
                            const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpDisposeEventListeners )
        mpDisposeEventListeners->removeInterface( Listener );
}

//--------------------------------------------------------------------------
// XDynamicResultSet methods.
// ------------------------------------------------------------------------------
Reference< XResultSet > SAL_CALL
SortedDynamicResultSet::getStaticResultSet()
    throw( ListenerAlreadySetException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mxListener.is() )
        throw ListenerAlreadySetException();

    mbStatic = sal_True;

    if ( mxOriginal.is() )
    {
        mpOne = new SortedResultSet( mxOriginal->getStaticResultSet() );
        mxOne = mpOne;
        mpOne->Initialize( maOptions, mxCompFac );
    }

    return mxOne;
}

// ------------------------------------------------------------------------------
void SAL_CALL
SortedDynamicResultSet::setListener( const Reference< XDynamicResultSetListener >& Listener )
    throw( ListenerAlreadySetException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mxListener.is() )
        throw ListenerAlreadySetException();

    addEventListener( Reference< XEventListener >::query( Listener ) );

    mxListener = Listener;

    if ( mxOriginal.is() )
        mxOriginal->setListener( mxOwnListener );
}

// ------------------------------------------------------------------------------
void SAL_CALL
SortedDynamicResultSet::connectToCache(
        const Reference< XDynamicResultSet > & xCache )
        throw( ListenerAlreadySetException,
               AlreadyInitializedException,
               ServiceNotFoundException,
               RuntimeException )
{
    if( mxListener.is() )
        throw ListenerAlreadySetException();

    if( mbStatic )
        throw ListenerAlreadySetException();

    Reference< XSourceInitialization > xTarget( xCache, UNO_QUERY );
    if( xTarget.is() && mxSMgr.is() )
    {
        Reference< XCachedDynamicResultSetStubFactory > xStubFactory;
        try
        {
            xStubFactory = Reference< XCachedDynamicResultSetStubFactory >(
                mxSMgr->createInstance(
                    OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.ucb.CachedDynamicResultSetStubFactory" )) ),
                UNO_QUERY );
        }
        catch ( Exception const & )
        {
        }

        if( xStubFactory.is() )
        {
            xStubFactory->connectToCache(
                  this, xCache, Sequence< NumberedSortingInfo > (), NULL );
            return;
        }
    }
    throw ServiceNotFoundException();
}

// ------------------------------------------------------------------------------
sal_Int16 SAL_CALL
SortedDynamicResultSet::getCapabilities()
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    sal_Int16 nCaps = 0;

    if ( mxOriginal.is() )
        nCaps = mxOriginal->getCapabilities();

    nCaps |= ContentResultSetCapability::SORTED;

    return nCaps;
}

//--------------------------------------------------------------------------
// XDynamicResultSetListener methods.
// ------------------------------------------------------------------------------

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
 removing himsef as listener (otherwise you deadlock)!!!
*/
void SAL_CALL
SortedDynamicResultSet::impl_notify( const ListEvent& Changes )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    sal_Bool bHasNew = sal_False;
    sal_Bool bHasModified = sal_False;

    SortedResultSet *pCurSet = NULL;

    // exchange mxNew and mxOld and immediately afterwards copy the tables
    // from Old to New
    if ( mbGotWelcome )
    {
        if ( mbUseOne )
        {
            mbUseOne = sal_False;
            mpTwo->CopyData( mpOne );
            pCurSet = mpTwo;
        }
        else
        {
            mbUseOne = sal_True;
            mpOne->CopyData( mpTwo );
            pCurSet = mpOne;
        }
    }

    Any  aRet;

    try {
        aRet = pCurSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("IsRowCountFinal")) );
    }
    catch ( UnknownPropertyException ) {}
    catch ( WrappedTargetException ) {}

    long nOldCount = pCurSet->GetCount();
    sal_Bool bWasFinal = false;

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
                        mpTwo = new SortedResultSet( aWelcome.Old );
                        mxTwo = mpTwo;
                        mpOne = new SortedResultSet( aWelcome.New );
                        mxOne = mpOne;
                        mpOne->Initialize( maOptions, mxCompFac );
                        mbGotWelcome = sal_True;
                        mbUseOne = sal_True;
                        pCurSet = mpOne;

                        aWelcome.Old = mxTwo;
                        aWelcome.New = mxOne;

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
                    bHasNew = sal_True;
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
                    bHasModified = sal_True;
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

//-----------------------------------------------------------------
// XEventListener
//-----------------------------------------------------------------
void SAL_CALL
SortedDynamicResultSet::impl_disposing( const EventObject& )
    throw( RuntimeException )
{
    mxListener.clear();
    mxOriginal.clear();
}

// ------------------------------------------------------------------------------
// private methods
// ------------------------------------------------------------------------------
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

//=========================================================================
//
// SortedDynamicResultSetFactory
//
//=========================================================================
SortedDynamicResultSetFactory::SortedDynamicResultSetFactory(
                        const Reference< XMultiServiceFactory > & rSMgr )
{
    mxSMgr = rSMgr;
}

//--------------------------------------------------------------------------
SortedDynamicResultSetFactory::~SortedDynamicResultSetFactory()
{
}

//--------------------------------------------------------------------------
// XInterface methods.
//--------------------------------------------------------------------------

XINTERFACE_IMPL_3( SortedDynamicResultSetFactory,
                   XTypeProvider,
                   XServiceInfo,
                   XSortedDynamicResultSetFactory );

//--------------------------------------------------------------------------
// XTypeProvider methods.
//--------------------------------------------------------------------------

XTYPEPROVIDER_IMPL_3( SortedDynamicResultSetFactory,
                      XTypeProvider,
                         XServiceInfo,
                      XSortedDynamicResultSetFactory );

//--------------------------------------------------------------------------
// XServiceInfo methods.
//--------------------------------------------------------------------------

XSERVICEINFO_IMPL_1( SortedDynamicResultSetFactory,
                         OUString(RTL_CONSTASCII_USTRINGPARAM(
                        "com.sun.star.comp.ucb.SortedDynamicResultSetFactory" )),
                         OUString(RTL_CONSTASCII_USTRINGPARAM(
                         DYNAMIC_RESULTSET_FACTORY_NAME )) );

//--------------------------------------------------------------------------
// Service factory implementation.
//--------------------------------------------------------------------------

ONE_INSTANCE_SERVICE_FACTORY_IMPL( SortedDynamicResultSetFactory );

//--------------------------------------------------------------------------
// SortedDynamicResultSetFactory methods.
//--------------------------------------------------------------------------
Reference< XDynamicResultSet > SAL_CALL
SortedDynamicResultSetFactory::createSortedDynamicResultSet(
                const Reference< XDynamicResultSet > & Source,
                const Sequence< NumberedSortingInfo > & Info,
                const Reference< XAnyCompareFactory > & CompareFactory )
    throw( RuntimeException )
{
    Reference< XDynamicResultSet > xRet;
    xRet = new SortedDynamicResultSet( Source, Info, CompareFactory, mxSMgr );
    return xRet;
}

//=========================================================================
//
// EventList
//
//=========================================================================

void EventList::Clear()
{
    for ( std::deque< LISTACTION* >::size_type i = 0;
          i < maData.size(); ++i )
    {
        delete maData[i];
    }

    maData.clear();
}

//--------------------------------------------------------------------------
void EventList::AddEvent( long nType, long nPos, long nCount )
{
    ListAction *pAction = new ListAction;
    pAction->Position = nPos;
    pAction->Count = nCount;
    pAction->ListActionType = nType;

    Insert( pAction );
}

//=================================================================
//
// SortedDynamicResultSetListener
//
//=================================================================

SortedDynamicResultSetListener::SortedDynamicResultSetListener(
                                SortedDynamicResultSet *mOwner )
{
    mpOwner = mOwner;
}

//-----------------------------------------------------------------
SortedDynamicResultSetListener::~SortedDynamicResultSetListener()
{
}

//-----------------------------------------------------------------
// XInterface methods.
//-----------------------------------------------------------------

XINTERFACE_IMPL_2( SortedDynamicResultSetListener,
                   XEventListener,  /* base class of XDynamicResultSetListener */
                   XDynamicResultSetListener );

//-----------------------------------------------------------------
// XEventListener ( base of XDynamicResultSetListener )
//-----------------------------------------------------------------
void SAL_CALL
SortedDynamicResultSetListener::disposing( const EventObject& Source )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpOwner )
        mpOwner->impl_disposing( Source );
}

//-----------------------------------------------------------------
// XDynamicResultSetListener
//-----------------------------------------------------------------
void SAL_CALL
SortedDynamicResultSetListener::notify( const ListEvent& Changes )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( maMutex );

    if ( mpOwner )
        mpOwner->impl_notify( Changes );
}

//-----------------------------------------------------------------
// own methods:
//-----------------------------------------------------------------
void SAL_CALL
SortedDynamicResultSetListener::impl_OwnerDies()
{
    osl::Guard< osl::Mutex > aGuard( maMutex );
    mpOwner = NULL;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
