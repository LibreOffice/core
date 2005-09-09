/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: resultsethelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 16:40:58 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

/**************************************************************************
                                TODO
 **************************************************************************

 - This implementation is far away from completion. It has no interface
   for changes notifications etc.

 *************************************************************************/

#ifndef _COM_SUN_STAR_UCB_LISTACTIONTYPE_HPP_
#include <com/sun/star/ucb/ListActionType.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_WELCOMEDYNAMICRESULTSETSTRUCT_HPP_
#include <com/sun/star/ucb/WelcomeDynamicResultSetStruct.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XCACHEDDYNAMICRESULTSETSTUBFACTORY_HPP_
#include <com/sun/star/ucb/XCachedDynamicResultSetStubFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UCB_XSOURCEINITIALIZATION_HPP_
#include <com/sun/star/ucb/XSourceInitialization.hpp>
#endif
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif
#ifndef _UCBHELPER_RESULTSETHELPER_HXX
#include <ucbhelper/resultsethelper.hxx>
#endif

using namespace com::sun::star::beans;
using namespace com::sun::star::lang;
using namespace com::sun::star::sdbc;
using namespace com::sun::star::ucb;
using namespace com::sun::star::uno;
using namespace cppu;
using namespace rtl;
using namespace ucb;

//=========================================================================
//=========================================================================
//
// ResultSetImplHelper Implementation.
//
//=========================================================================
//=========================================================================

#define RESULTSETHELPER_INIT()      \
     m_xSMgr( rxSMgr ),             \
      m_pDisposeEventListeners( 0 ),  \
      m_bInitDone( sal_False ),       \
      m_bStatic( sal_False )

//=========================================================================
ResultSetImplHelper::ResultSetImplHelper(
                      const Reference< XMultiServiceFactory >& rxSMgr )
: RESULTSETHELPER_INIT()
{
}

//=========================================================================
ResultSetImplHelper::ResultSetImplHelper(
                      const Reference< XMultiServiceFactory >& rxSMgr,
                      const OpenCommandArgument2& rCommand )
: RESULTSETHELPER_INIT(),
  m_aCommand( rCommand )
{
}

//=========================================================================
// virtual
ResultSetImplHelper::~ResultSetImplHelper()
{
    delete m_pDisposeEventListeners;
}

//=========================================================================
//
// XInterface methods.
//
//=========================================================================

XINTERFACE_IMPL_4( ResultSetImplHelper,
                   XTypeProvider,
                   XServiceInfo,
                   XComponent, /* base of XDynamicResultSet */
                   XDynamicResultSet );

//=========================================================================
//
// XTypeProvider methods.
//
//=========================================================================

XTYPEPROVIDER_IMPL_3( ResultSetImplHelper,
                      XTypeProvider,
                         XServiceInfo,
                      XDynamicResultSet );

//=========================================================================
//
// XServiceInfo methods.
//
//=========================================================================

XSERVICEINFO_NOFACTORY_IMPL_1( ResultSetImplHelper,
                    OUString::createFromAscii( "ResultSetImplHelper" ),
                    OUString::createFromAscii( DYNAMICRESULTSET_SERVICE_NAME ) );

//=========================================================================
//
// XComponent methods.
//
//=========================================================================

// virtual
void SAL_CALL ResultSetImplHelper::dispose()
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_pDisposeEventListeners && m_pDisposeEventListeners->getLength() )
    {
        EventObject aEvt;
        aEvt.Source = static_cast< XComponent * >( this );
        m_pDisposeEventListeners->disposeAndClear( aEvt );
    }
}

//=========================================================================
// virtual
void SAL_CALL ResultSetImplHelper::addEventListener(
                            const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_pDisposeEventListeners )
        m_pDisposeEventListeners = new OInterfaceContainerHelper( m_aMutex );

    m_pDisposeEventListeners->addInterface( Listener );
}

//=========================================================================
// virtual
void SAL_CALL ResultSetImplHelper::removeEventListener(
                            const Reference< XEventListener >& Listener )
    throw( RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_pDisposeEventListeners )
        m_pDisposeEventListeners->removeInterface( Listener );
}

//=========================================================================
//
// XDynamicResultSet methods.
//
//=========================================================================

// virtual
Reference< XResultSet > SAL_CALL ResultSetImplHelper::getStaticResultSet()
    throw( ListenerAlreadySetException, RuntimeException )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( m_xListener.is() )
        throw ListenerAlreadySetException();

    init( sal_True );
    return m_xResultSet1;
}

//=========================================================================
// virtual
void SAL_CALL ResultSetImplHelper::setListener(
                    const Reference< XDynamicResultSetListener >& Listener )
    throw( ListenerAlreadySetException, RuntimeException )
{
    osl::ClearableGuard< osl::Mutex > aGuard( m_aMutex );

    if ( m_bStatic || m_xListener.is() )
        throw ListenerAlreadySetException();

    m_xListener = Listener;

    //////////////////////////////////////////////////////////////////////
    // Create "welcome event" and send it to listener.
    //////////////////////////////////////////////////////////////////////

    // Note: We only have the implementation for a static result set at the
    //       moment (src590). The dynamic result sets passed to the listener
    //       are a fake. This implementation will never call "notify" at the
    //       listener to propagate any changes!!!

    init( sal_False );

    Any aInfo;
    aInfo <<= WelcomeDynamicResultSetStruct( m_xResultSet1 /* "old" */,
                                             m_xResultSet2 /* "new" */ );

    Sequence< ListAction > aActions( 1 );
    aActions.getArray()[ 0 ] = ListAction( 0, // Position; not used
                                           0, // Count; not used
                                           ListActionType::WELCOME,
                                              aInfo );
    aGuard.clear();

    Listener->notify(
                ListEvent( static_cast< OWeakObject * >( this ), aActions ) );
}

//=========================================================================
// virtual
sal_Int16 SAL_CALL ResultSetImplHelper::getCapabilities()
    throw( RuntimeException )
{
    // ! ContentResultSetCapability::SORTED
    return 0;
}

//=========================================================================
// virtual
void SAL_CALL ResultSetImplHelper::connectToCache(
                        const Reference< XDynamicResultSet > & xCache )
    throw( ListenerAlreadySetException,
           AlreadyInitializedException,
           ServiceNotFoundException,
           RuntimeException )
{
    if ( m_xListener.is() )
        throw ListenerAlreadySetException();

    if ( m_bStatic )
        throw ListenerAlreadySetException();

    Reference< XSourceInitialization > xTarget( xCache, UNO_QUERY );
    if ( xTarget.is() )
    {
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

        if ( xStubFactory.is() )
        {
            xStubFactory->connectToCache(
                                  this, xCache, m_aCommand.SortingInfo, 0 );
            return;
        }
    }
    throw ServiceNotFoundException();
}

//=========================================================================
//
// Non-interface methods.
//
//=========================================================================

void ResultSetImplHelper::init( sal_Bool bStatic )
{
    osl::Guard< osl::Mutex > aGuard( m_aMutex );

    if ( !m_bInitDone )
    {
        if ( bStatic )
        {
            // virtual... derived class fills m_xResultSet1
            initStatic();

            VOS_ENSURE( m_xResultSet1.is(),
                        "ResultSetImplHelper::init - No 1st result set!" );
            m_bStatic = sal_True;
        }
        else
        {
            // virtual... derived class fills m_xResultSet1 and m_xResultSet2
            initDynamic();

            VOS_ENSURE( m_xResultSet1.is(),
                        "ResultSetImplHelper::init - No 1st result set!" );
            VOS_ENSURE( m_xResultSet2.is(),
                        "ResultSetImplHelper::init - No 2nd result set!" );
            m_bStatic = sal_False;
        }
        m_bInitDone = sal_True;
    }

}

/*

pure virtual

//=========================================================================
// virtual
void ResultSetImplHelper::initStatic()
{
}

//=========================================================================
// virtual
void ResultSetImplHelper::initDynamic()
{
}

*/

