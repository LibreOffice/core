/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: instancelocker.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2008-01-04 16:37:37 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_comphelper.hxx"

#ifndef _COM_SUN_STAR_UTIL_XCLOSEBROADCASTER_HPP_
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDESKTOP_HPP_
#include <com/sun/star/frame/XDesktop.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DOUBLEINITIALIZATIONEXCEPTION_HPP_
#include <com/sun/star/frame/DoubleInitializationException.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_DOUBLEINITIALIZATIONEXCEPTION_HPP_
#include <com/sun/star/frame/DoubleInitializationException.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#include "instancelocker.hxx"

using namespace ::com::sun::star;


// ====================================================================
// OInstanceLocker
// ====================================================================

// --------------------------------------------------------
OInstanceLocker::OInstanceLocker( const uno::Reference< uno::XComponentContext >& xContext )
: m_xContext( xContext )
, m_pLockListener( NULL )
, m_pListenersContainer( NULL )
, m_bDisposed( sal_False )
, m_bInitialized( sal_False )
{
}

// --------------------------------------------------------
OInstanceLocker::~OInstanceLocker()
{
    if ( !m_bDisposed )
    {
        m_refCount++; // to call dispose
        try {
            dispose();
        }
        catch ( uno::RuntimeException& )
        {}
    }

    if ( m_pListenersContainer )
    {
        delete m_pListenersContainer;
        m_pListenersContainer = NULL;
    }
}

// XComponent
// --------------------------------------------------------
void SAL_CALL OInstanceLocker::dispose()
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        throw lang::DisposedException();

       lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >(this) );
    if ( m_pListenersContainer )
        m_pListenersContainer->disposeAndClear( aSource );

    if ( m_xLockListener.is() )
    {
        if ( m_pLockListener )
        {
            m_pLockListener->Dispose();
            m_pLockListener = NULL;
        }
        m_xLockListener = uno::Reference< uno::XInterface >();
    }

    m_bDisposed = sal_True;
}

// --------------------------------------------------------
void SAL_CALL OInstanceLocker::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pListenersContainer )
        m_pListenersContainer = new ::cppu::OInterfaceContainerHelper( m_aMutex );

    m_pListenersContainer->addInterface( xListener );
}

// --------------------------------------------------------
void SAL_CALL OInstanceLocker::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_pListenersContainer )
        m_pListenersContainer->removeInterface( xListener );
}

// XInitialization
// --------------------------------------------------------
void SAL_CALL OInstanceLocker::initialize( const uno::Sequence< uno::Any >& aArguments )
    throw (uno::Exception, uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bInitialized )
        throw frame::DoubleInitializationException();

    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_refCount )
        throw uno::RuntimeException(); // the object must be refcounted already!

    uno::Reference< uno::XInterface > xInstance;
    uno::Reference< embed::XActionsApproval > xApproval;
    sal_Int32 nModes = 0;

    try
    {
        sal_Int32 nLen = aArguments.getLength();
        if ( nLen < 2 || nLen > 3 )
            throw lang::IllegalArgumentException(
                            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Wrong count of parameters!" ) ),
                            uno::Reference< uno::XInterface >(),
                            0 );

        if ( !( aArguments[0] >>= xInstance ) || !xInstance.is() )
            throw lang::IllegalArgumentException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("Nonempty reference is expected as the first argument!" ) ),
                    uno::Reference< uno::XInterface >(),
                    0 );

        if ( !( aArguments[1] >>= nModes )
          || !( nModes & embed::Actions::PREVENT_CLOSE ) && !( nModes & embed::Actions::PREVENT_TERMINATION ) )
            throw lang::IllegalArgumentException(
                    ::rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM("The correct modes set is expected as the second argument!" ) ),
                    uno::Reference< uno::XInterface >(),
                    0 );

        if ( nLen == 3 && !( aArguments[2] >>= xApproval ) )
            throw lang::IllegalArgumentException(
                    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("If the third argument is provided, it must be XActionsApproval implementation!" ) ),
                    uno::Reference< uno::XInterface >(),
                    0 );

        m_pLockListener = new OLockListener( uno::Reference< lang::XComponent > ( static_cast< lang::XComponent* >( this ) ),
                                            xInstance,
                                            nModes,
                                            xApproval );
        m_xLockListener = uno::Reference< uno::XInterface >( static_cast< OWeakObject* >( m_pLockListener ) );
        m_pLockListener->Init();
    }
    catch( uno::Exception& )
    {
        dispose();
        throw;
    }

    m_bInitialized = sal_True;
}


// XServiceInfo
// --------------------------------------------------------
::rtl::OUString SAL_CALL OInstanceLocker::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return impl_staticGetImplementationName();
}

// --------------------------------------------------------
::sal_Bool SAL_CALL OInstanceLocker::supportsService( const ::rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aSeq = impl_staticGetSupportedServiceNames();

    for ( sal_Int32 nInd = 0; nInd < aSeq.getLength(); nInd++ )
        if ( ServiceName.compareTo( aSeq[nInd] ) == 0 )
            return sal_True;

    return sal_False;
}

// --------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OInstanceLocker::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    return impl_staticGetSupportedServiceNames();
}

// Static methods
// --------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OInstanceLocker::impl_staticGetSupportedServiceNames()
{
    const rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.embed.InstanceLocker" ) );
    return uno::Sequence< rtl::OUString >( &aServiceName, 1 );
}

// --------------------------------------------------------
::rtl::OUString SAL_CALL OInstanceLocker::impl_staticGetImplementationName()
{
    return rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.comp.embed.InstanceLocker" ) );
}

// --------------------------------------------------------
uno::Reference< uno::XInterface > SAL_CALL OInstanceLocker::impl_staticCreateSelfInstance(
                                const uno::Reference< uno::XComponentContext >& rxContext )
{
    return static_cast< cppu::OWeakObject * >( new OInstanceLocker( rxContext ) );
}



// ====================================================================
// OLockListener
// ====================================================================

// --------------------------------------------------------
OLockListener::OLockListener( const uno::WeakReference< lang::XComponent >& xWrapper,
                    const uno::Reference< uno::XInterface >& xInstance,
                    sal_Int32 nMode,
                    const uno::Reference< embed::XActionsApproval > xApproval )
: m_xInstance( xInstance )
, m_xApproval( xApproval )
, m_xWrapper( xWrapper )
, m_bDisposed( sal_False )
, m_bInitialized( sal_False )
, m_nMode( nMode )
{
}

// --------------------------------------------------------
OLockListener::~OLockListener()
{
}

// --------------------------------------------------------
void OLockListener::Dispose()
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );

    if ( m_bDisposed )
        return;

    if ( m_nMode & embed::Actions::PREVENT_CLOSE )
    {
        try
        {
            uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( m_xInstance, uno::UNO_QUERY );
            if ( xCloseBroadcaster.is() )
                xCloseBroadcaster->removeCloseListener( static_cast< util::XCloseListener* >( this ) );

            uno::Reference< util::XCloseable > xCloseable( m_xInstance, uno::UNO_QUERY );
            if ( xCloseable.is() )
                xCloseable->close( sal_True );
        }
        catch( uno::Exception& )
        {}
    }

    if ( m_nMode & embed::Actions::PREVENT_TERMINATION )
    {
        try
        {
            uno::Reference< frame::XDesktop > xDesktop( m_xInstance, uno::UNO_QUERY_THROW );
            xDesktop->removeTerminateListener( static_cast< frame::XTerminateListener* >( this ) );
        }
        catch( uno::Exception& )
        {}
    }

    m_xInstance = uno::Reference< uno::XInterface >();
    m_bDisposed = sal_True;
}

// XEventListener
// --------------------------------------------------------
void SAL_CALL OLockListener::disposing( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );

    // object is disposed
    if ( aEvent.Source == m_xInstance )
    {
        // the object does not listen for anything any more
        m_nMode = 0;

        // dispose the wrapper;
        uno::Reference< lang::XComponent > xComponent( m_xWrapper.get(), uno::UNO_QUERY );
        aGuard.clear();
        if ( xComponent.is() )
        {
            try { xComponent->dispose(); }
            catch( uno::Exception& ){}
        }
    }
}


// XCloseListener
// --------------------------------------------------------
void SAL_CALL OLockListener::queryClosing( const lang::EventObject& aEvent, sal_Bool )
    throw (util::CloseVetoException, uno::RuntimeException)
{
    // GetsOwnership parameter is always ignored, the user of the service must close the object always
    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( !m_bDisposed && aEvent.Source == m_xInstance && ( m_nMode & embed::Actions::PREVENT_CLOSE ) )
    {
        try
        {
            uno::Reference< embed::XActionsApproval > xApprove = m_xApproval;

            // unlock the mutex here
            aGuard.clear();

            if ( xApprove.is() && xApprove->approveAction( embed::Actions::PREVENT_CLOSE ) )
                throw util::CloseVetoException();
        }
        catch( util::CloseVetoException& )
        {
            // rethrow this exception
            throw;
        }
        catch( uno::Exception& )
        {
            // no action should be done
        }
    }
}

// --------------------------------------------------------
void SAL_CALL OLockListener::notifyClosing( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );

    // object is closed, no reason to listen
    if ( aEvent.Source == m_xInstance )
    {
        uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( aEvent.Source, uno::UNO_QUERY );
        if ( xCloseBroadcaster.is() )
        {
            xCloseBroadcaster->removeCloseListener( static_cast< util::XCloseListener* >( this ) );
            m_nMode &= ~embed::Actions::PREVENT_CLOSE;
            if ( !m_nMode )
            {
                // dispose the wrapper;
                uno::Reference< lang::XComponent > xComponent( m_xWrapper.get(), uno::UNO_QUERY );
                aGuard.clear();
                if ( xComponent.is() )
                {
                    try { xComponent->dispose(); }
                    catch( uno::Exception& ){}
                }
            }
        }
    }
}


// XTerminateListener
// --------------------------------------------------------
void SAL_CALL OLockListener::queryTermination( const lang::EventObject& aEvent )
    throw (frame::TerminationVetoException, uno::RuntimeException)
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( aEvent.Source == m_xInstance && ( m_nMode & embed::Actions::PREVENT_TERMINATION ) )
    {
        try
        {
            uno::Reference< embed::XActionsApproval > xApprove = m_xApproval;

            // unlock the mutex here
            aGuard.clear();

            if ( xApprove.is() && xApprove->approveAction( embed::Actions::PREVENT_TERMINATION ) )
                throw frame::TerminationVetoException();
        }
        catch( frame::TerminationVetoException& )
        {
            // rethrow this exception
            throw;
        }
        catch( uno::Exception& )
        {
            // no action should be done
        }
    }
}

// --------------------------------------------------------
void SAL_CALL OLockListener::notifyTermination( const lang::EventObject& aEvent )
    throw (uno::RuntimeException)
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );

    // object is terminated, no reason to listen
    if ( aEvent.Source == m_xInstance )
    {
        uno::Reference< frame::XDesktop > xDesktop( aEvent.Source, uno::UNO_QUERY );
        if ( xDesktop.is() )
        {
            try
            {
                xDesktop->removeTerminateListener( static_cast< frame::XTerminateListener* >( this ) );
                m_nMode &= ~embed::Actions::PREVENT_TERMINATION;
                if ( !m_nMode )
                {
                    // dispose the wrapper;
                    uno::Reference< lang::XComponent > xComponent( m_xWrapper.get(), uno::UNO_QUERY );
                    aGuard.clear();
                    if ( xComponent.is() )
                    {
                        try { xComponent->dispose(); }
                        catch( uno::Exception& ){}
                    }
                }
            }
            catch( uno::Exception& )
            {}
        }
    }
}


// XInitialization
// --------------------------------------------------------
sal_Bool OLockListener::Init()
{
    ::osl::ResettableMutexGuard aGuard( m_aMutex );

    if ( m_bDisposed || m_bInitialized )
        return sal_False;

    try
    {
        if ( m_nMode & embed::Actions::PREVENT_CLOSE )
        {
            uno::Reference< util::XCloseBroadcaster > xCloseBroadcaster( m_xInstance, uno::UNO_QUERY_THROW );
            xCloseBroadcaster->addCloseListener( static_cast< util::XCloseListener* >( this ) );
        }

        if ( m_nMode & embed::Actions::PREVENT_TERMINATION )
        {
            uno::Reference< frame::XDesktop > xDesktop( m_xInstance, uno::UNO_QUERY_THROW );
            xDesktop->addTerminateListener( static_cast< frame::XTerminateListener* >( this ) );
        }
    }
    catch( uno::Exception& )
    {
        // dispose the wrapper;
        uno::Reference< lang::XComponent > xComponent( m_xWrapper.get(), uno::UNO_QUERY );
        aGuard.clear();
        if ( xComponent.is() )
        {
            try { xComponent->dispose(); }
            catch( uno::Exception& ){}
        }

        throw;
    }

    m_bInitialized = sal_True;

    return sal_True;
}

