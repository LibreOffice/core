/*************************************************************************
 *
 *  $RCSfile: dynamicresultsetwrapper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kso $ $Date: 2001-03-01 08:03:25 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <dynamicresultsetwrapper.hxx>

#ifndef _UCBHELPER_MACROS_HXX
#include <ucbhelper/macros.hxx>
#endif

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef  _COM_SUN_STAR_UCB_LISTACTIONTYPE_HPP_
#include <com/sun/star/ucb/ListActionType.hpp>
#endif

#ifndef  _COM_SUN_STAR_UCB_WELCOMEDYNAMICRESULTSETSTRUCT_HPP_
#include <com/sun/star/ucb/WelcomeDynamicResultSetStruct.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XCACHEDDYNAMICRESULTSETSTUBFACTORY_HPP_
#include <com/sun/star/ucb/XCachedDynamicResultSetStubFactory.hpp>
#endif

using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace rtl;

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// class DynamicResultSetWrapper
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

DynamicResultSetWrapper::DynamicResultSetWrapper(
                    Reference< XDynamicResultSet > xOrigin
                    , const Reference< XMultiServiceFactory > & xSMgr )

                : m_xSMgr( xSMgr )
                , m_xSource( xOrigin )
                , m_xSourceResultOne( NULL )
                , m_xSourceResultTwo( NULL )
            //  , m_xSourceResultCurrent( NULL )
            //  , m_bUseOne( NULL )
                , m_xMyResultOne( NULL )
                , m_xMyResultTwo( NULL )
                , m_xListener( NULL )
                , m_pDisposeEventListeners( NULL )
                , m_bDisposed( sal_False )
                , m_bInDispose( sal_False )
                , m_bStatic( sal_False )
                , m_bGotWelcome( sal_False )
{
    m_pMyListenerImpl = new DynamicResultSetWrapperListener( this );
    m_xMyListenerImpl = Reference< XDynamicResultSetListener >( m_pMyListenerImpl );
    //call impl_init() at the end of constructor of derived class
};

void SAL_CALL DynamicResultSetWrapper::impl_init()
{
    //call this at the end of constructor of derived class
    //

    Reference< XDynamicResultSet > xSource = NULL;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        xSource = m_xSource;
        m_xSource = NULL;
    }
    if( xSource.is() )
        setSource( xSource );
}

DynamicResultSetWrapper::~DynamicResultSetWrapper()
{
    //call impl_deinit() at start of destructor of derived class

    delete m_pDisposeEventListeners;
};

void SAL_CALL DynamicResultSetWrapper::impl_deinit()
{
    //call this at start of destructor of derived class
    //
    m_pMyListenerImpl->impl_OwnerDies();
}

void SAL_CALL DynamicResultSetWrapper
::impl_EnsureNotDisposed()
    throw( DisposedException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    if( m_bDisposed )
        throw DisposedException();
}

//virtual
void SAL_CALL DynamicResultSetWrapper
::impl_InitResultSetOne( const Reference< XResultSet >& xResultSet )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    OSL_ENSURE( !m_xSourceResultOne.is(), "Source ResultSet One is set already" );
    m_xSourceResultOne = xResultSet;
    m_xMyResultOne = xResultSet;
}

//virtual
void SAL_CALL DynamicResultSetWrapper
::impl_InitResultSetTwo( const Reference< XResultSet >& xResultSet )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    OSL_ENSURE( !m_xSourceResultTwo.is(), "Source ResultSet Two is set already" );
    m_xSourceResultTwo = xResultSet;
    m_xMyResultTwo = xResultSet;
}

//--------------------------------------------------------------------------
// XInterface methods.
//--------------------------------------------------------------------------
//list all interfaces inclusive baseclasses of interfaces
QUERYINTERFACE_IMPL_START( DynamicResultSetWrapper )
    SAL_STATIC_CAST( XComponent*, this ) //base of XDynamicResultSet
    , SAL_STATIC_CAST( XDynamicResultSet*, this )
    , SAL_STATIC_CAST( XSourceInitialization*, this )
QUERYINTERFACE_IMPL_END

//--------------------------------------------------------------------------
// XComponent methods.
//--------------------------------------------------------------------------
// virtual
void SAL_CALL DynamicResultSetWrapper
    ::dispose() throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    Reference< XComponent > xSourceComponent;
    {
        osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );
        if( m_bInDispose || m_bDisposed )
            return;
        m_bInDispose = sal_True;

        xSourceComponent = Reference< XComponent >(m_xSource, UNO_QUERY);

        if( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
        {
            EventObject aEvt;
            aEvt.Source = static_cast< XComponent * >( this );

            aGuard.clear();
            m_pDisposeEventListeners->disposeAndClear( aEvt );
        }
    }

    /* //@todo ?? ( only if java collection needs to long )
    if( xSourceComponent.is() )
        xSourceComponent->dispose();
    */

    osl::Guard< osl::Mutex > aGuard( m_aMutex );
    m_bDisposed = sal_True;
    m_bInDispose = sal_False;
}

//--------------------------------------------------------------------------
// virtual
void SAL_CALL DynamicResultSetWrapper
    ::addEventListener( const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_pDisposeEventListeners )
        m_pDisposeEventListeners =
                    new OInterfaceContainerHelper( m_aContainerMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}

//--------------------------------------------------------------------------
// virtual
void SAL_CALL DynamicResultSetWrapper
    ::removeEventListener( const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}

//--------------------------------------------------------------------------
// own methods
//--------------------------------------------------------------------------

//virtual
void SAL_CALL DynamicResultSetWrapper
    ::impl_disposing( const EventObject& rEventObject )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if( !m_xSource.is() )
        return;

    //release all references to the broadcaster:
    m_xSource.clear();
    m_xSourceResultOne.clear();//?? or only when not static??
    m_xSourceResultTwo.clear();//??
    //@todo m_xMyResultOne.clear(); ???
    //@todo m_xMyResultTwo.clear(); ???
}

//virtual
void SAL_CALL DynamicResultSetWrapper
    ::impl_notify( const ListEvent& Changes )
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();
    //@todo
    /*
    <p>The Listener is allowed to blockade this call, until he really want to go
    to the new version. The only situation, where the listener has to return the
    update call at once is, while he disposes his broadcaster or while he is
    removing himsef as listener (otherwise you deadlock)!!!
    */
    // handle the actions in the list

    ListEvent aNewEvent;
    aNewEvent.Source = static_cast< XDynamicResultSet * >( this );
    aNewEvent.Changes = Changes.Changes;

    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        for( long i=0; !m_bGotWelcome && i<Changes.Changes.getLength(); i++ )
        {
            ListAction& rAction = aNewEvent.Changes[i];
            switch( rAction.ListActionType )
            {
                case ListActionType::WELCOME:
                {
                    WelcomeDynamicResultSetStruct aWelcome;
                    if( rAction.ActionInfo >>= aWelcome )
                    {
                        impl_InitResultSetOne( aWelcome.Old );
                        impl_InitResultSetTwo( aWelcome.New );
                        m_bGotWelcome = sal_True;

                        aWelcome.Old = m_xMyResultOne;
                        aWelcome.New = m_xMyResultTwo;

                         rAction.ActionInfo <<= aWelcome;
                    }
                    else
                    {
                        OSL_ENSURE( sal_False, "ListActionType was WELCOME but ActionInfo didn't contain a WelcomeDynamicResultSetStruct" );
                        //throw RuntimeException();
                    }
                    break;
                }
            }
        }
        OSL_ENSURE( m_bGotWelcome, "first notification was without WELCOME" );
    }

    if( !m_xListener.is() )
        m_aListenerSet.wait();
    m_xListener->notify( aNewEvent );

    /*
    m_bUseOne = !m_bUseOne;
    if( m_bUseOne )
        m_xSourceResultCurrent = m_xSourceResultOne;
    else
        m_xSourceResultCurrent = m_xSourceResultTwo;
    */
}

//--------------------------------------------------------------------------
// XSourceInitialization
//--------------------------------------------------------------------------
//virtual
void SAL_CALL DynamicResultSetWrapper
    ::setSource( const Reference< XInterface > & Source )
    throw( AlreadyInitializedException, RuntimeException )
{
    impl_EnsureNotDisposed();
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_xSource.is() )
        {
            throw AlreadyInitializedException();
        }
    }

    Reference< XDynamicResultSet > xSourceDynamic( Source, UNO_QUERY );
    OSL_ENSURE( xSourceDynamic.is(),
        "the given source is not of required type XDynamicResultSet" );

    Reference< XDynamicResultSetListener > xListener = NULL;
    Reference< XDynamicResultSetListener > xMyListenerImpl = NULL;

    sal_Bool bStatic = sal_False;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        m_xSource = xSourceDynamic;
        xListener = m_xListener;
        bStatic = m_bStatic;
        xMyListenerImpl = m_xMyListenerImpl;
    }
    if( xListener.is() )
        xSourceDynamic->setListener( m_xMyListenerImpl );
    else if( bStatic )
    {
        Reference< XComponent > xSourceComponent( Source, UNO_QUERY );
        xSourceComponent->addEventListener( Reference< XEventListener > ::query( xMyListenerImpl ) );
    }
    m_aSourceSet.set();
}

//--------------------------------------------------------------------------
// XDynamicResultSet
//--------------------------------------------------------------------------
//virtual
Reference< XResultSet > SAL_CALL DynamicResultSetWrapper
    ::getStaticResultSet()
    throw( ListenerAlreadySetException, RuntimeException )
{
    impl_EnsureNotDisposed();

    Reference< XDynamicResultSet > xSource = NULL;
    Reference< XEventListener > xMyListenerImpl = NULL;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_xListener.is() )
            throw ListenerAlreadySetException();

        xSource = m_xSource;
        m_bStatic = sal_True;
        xMyListenerImpl = Reference< XEventListener > ::query( m_xMyListenerImpl );
    }

    if( xSource.is() )
    {
        Reference< XComponent > xSourceComponent( xSource, UNO_QUERY );
        xSourceComponent->addEventListener( xMyListenerImpl );
    }
    if( !xSource.is() )
        m_aSourceSet.wait();


    Reference< XResultSet > xResultSet = xSource->getStaticResultSet();
    impl_InitResultSetOne( xResultSet );
    return m_xMyResultOne;
}

//virtual
void SAL_CALL DynamicResultSetWrapper
    ::setListener( const Reference<
    XDynamicResultSetListener > & Listener )
    throw( ListenerAlreadySetException, RuntimeException )
{
    impl_EnsureNotDisposed();

    Reference< XDynamicResultSet > xSource = NULL;
    Reference< XDynamicResultSetListener > xMyListenerImpl = NULL;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        if( m_xListener.is() )
            throw ListenerAlreadySetException();
        if( m_bStatic )
            throw ListenerAlreadySetException();

        m_xListener = Listener;
        addEventListener( Reference< XEventListener >::query( Listener ) );

        xSource = m_xSource;
        xMyListenerImpl = m_xMyListenerImpl;
    }
    if ( xSource.is() )
        xSource->setListener( xMyListenerImpl );

    m_aListenerSet.set();
}

//virtual
void SAL_CALL DynamicResultSetWrapper
    ::connectToCache( const Reference< XDynamicResultSet > & xCache )
    throw( ListenerAlreadySetException, AlreadyInitializedException, ServiceNotFoundException, RuntimeException )
{
    impl_EnsureNotDisposed();

    if( m_xListener.is() )
        throw ListenerAlreadySetException();
    if( m_bStatic )
        throw ListenerAlreadySetException();

    Reference< XSourceInitialization > xTarget( xCache, UNO_QUERY );
    OSL_ENSURE( xTarget.is(), "The given Target dosn't have the required interface 'XSourceInitialization'" );
    if( xTarget.is() && m_xSMgr.is() )
    {
        //@todo m_aSourceSet.wait();?

        Reference< XCachedDynamicResultSetStubFactory > xStubFactory;
        try
        {
            xStubFactory = Reference< XCachedDynamicResultSetStubFactory >(
                m_xSMgr->createInstance(
                    OUString::createFromAscii(
                        "com.sun.star.ucb.CachedDynamicResultSetStubFactory" ) ),
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
    OSL_ENSURE( sal_False, "could not connect to cache" );
    throw ServiceNotFoundException();
}

//virtual
sal_Int16 SAL_CALL DynamicResultSetWrapper
    ::getCapabilities()
    throw( RuntimeException )
{
    impl_EnsureNotDisposed();

    m_aSourceSet.wait();
    Reference< XDynamicResultSet > xSource = NULL;
    {
        osl::Guard< osl::Mutex > aGuard( m_aMutex );
        xSource = m_xSource;
    }
    return xSource->getCapabilities();
}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------
// class DynamicResultSetWrapperListener
//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

DynamicResultSetWrapperListener::DynamicResultSetWrapperListener(
        DynamicResultSetWrapper* pOwner )
        : m_pOwner( pOwner )
{

}

DynamicResultSetWrapperListener::~DynamicResultSetWrapperListener()
{

}

//--------------------------------------------------------------------------
// XInterface methods.
//--------------------------------------------------------------------------
//list all interfaces inclusive baseclasses of interfaces
XINTERFACE_IMPL_2( DynamicResultSetWrapperListener
                   , XDynamicResultSetListener
                   , XEventListener //base of XDynamicResultSetListener
                   );

//--------------------------------------------------------------------------
// XDynamicResultSetListener methods:
//--------------------------------------------------------------------------
//virtual
void SAL_CALL DynamicResultSetWrapperListener
    ::disposing( const EventObject& rEventObject )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if( m_pOwner )
        m_pOwner->impl_disposing( rEventObject );
}

//virtual
void SAL_CALL DynamicResultSetWrapperListener
    ::notify( const ListEvent& Changes )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if( m_pOwner )
        m_pOwner->impl_notify( Changes );
}

//--------------------------------------------------------------------------
// own methods:
//--------------------------------------------------------------------------

void SAL_CALL DynamicResultSetWrapperListener
    ::impl_OwnerDies()
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    m_pOwner = NULL;
}

