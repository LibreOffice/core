/*************************************************************************
 *
 *  $RCSfile: WinClipboard.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: tra $ $Date: 2001-03-19 09:10:17 $
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

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#ifndef _WINCLIPBOARD_HXX_
#include "WinClipboard.hxx"
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_CLIPBOARDEVENT_HPP_
#include <com/sun/star/datatransfer/clipboard/ClipboardEvent.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif

#ifndef _WINCLIPBIMPL_HXX_
#include "WinClipbImpl.hxx"
#endif

//------------------------------------------------------------------------
// namespace directives
//------------------------------------------------------------------------

using namespace rtl;
using namespace osl;
using namespace std;
using namespace cppu;

using namespace com::sun::star::uno;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;

//------------------------------------------------------------------------
// defines
//------------------------------------------------------------------------

#define WINCLIPBOARD_IMPL_NAME  "com.sun.star.datatransfer.clipboard.ClipboardW32"

//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

namespace
{
    Sequence< OUString > SAL_CALL WinClipboard_getSupportedServiceNames()
    {
        Sequence< OUString > aRet(1);
        aRet[0] = OUString::createFromAscii("com.sun.star.datatransfer.clipboard.SystemClipboard");
        return aRet;
    }
}

//------------------------------------------------------------------------
// ctor
//------------------------------------------------------------------------

CWinClipboard::CWinClipboard( const Reference< XMultiServiceFactory >& rServiceManager, const OUString& aClipboardName ) :
    WeakComponentImplHelper5< XClipboardEx, XFlushableClipboard, XClipboardNotifier, XEventListener, XServiceInfo >( m_aCbListenerMutex ),
    m_SrvMgr( rServiceManager )
{
    m_pImpl.reset( new CWinClipbImpl( aClipboardName, this ) );
}

//========================================================================
// XClipboard
//========================================================================

//------------------------------------------------------------------------
// getContent
// to avoid unecessary traffic we check first if there is a clipboard
// content which was set via setContent, in this case we don't need
// to query the content from the clipboard, create a new wrapper object
// and so on, we simply return the orignial XTransferable instead of our
// DOTransferable
//------------------------------------------------------------------------

Reference< XTransferable > SAL_CALL CWinClipboard::getContents( ) throw( RuntimeException )
{
    MutexGuard aGuard( m_aMutex );

    if ( rBHelper.bDisposed )
        throw DisposedException( OUString::createFromAscii( "object is already disposed" ),
                                 static_cast< XClipboardEx* >( this ) );

    if ( NULL != m_pImpl.get( ) )
        return m_pImpl->getContents( );

    return Reference< XTransferable >( );
}

//------------------------------------------------------------------------
// setContent
//------------------------------------------------------------------------

void SAL_CALL CWinClipboard::setContents( const Reference< XTransferable >& xTransferable,
                                          const Reference< XClipboardOwner >& xClipboardOwner )
                                          throw( RuntimeException )
{
    MutexGuard aGuard( m_aMutex );

    if ( rBHelper.bDisposed )
        throw DisposedException( OUString::createFromAscii( "object is already disposed" ),
                                 static_cast< XClipboardEx* >( this ) );

    if ( NULL != m_pImpl.get( ) )
        m_pImpl->setContents( xTransferable, xClipboardOwner );
}

//------------------------------------------------------------------------
// getName
//------------------------------------------------------------------------

OUString SAL_CALL CWinClipboard::getName(  ) throw( RuntimeException )
{
    if ( rBHelper.bDisposed )
        throw DisposedException( OUString::createFromAscii( "object is already disposed" ),
                                 static_cast< XClipboardEx* >( this ) );

    if ( NULL != m_pImpl.get( ) )
        return m_pImpl->getName( );

    return OUString::createFromAscii( "" );
}

//========================================================================
// XFlushableClipboard
//========================================================================

void SAL_CALL CWinClipboard::flushClipboard( ) throw( RuntimeException )
{
    MutexGuard aGuard( m_aMutex );

    if ( rBHelper.bDisposed )
        throw DisposedException( OUString::createFromAscii( "object is already disposed" ),
                                 static_cast< XClipboardEx* >( this ) );

    if ( NULL != m_pImpl.get( ) )
        m_pImpl->flushClipboard( );
}

//========================================================================
// XClipboardEx
//========================================================================

sal_Int8 SAL_CALL CWinClipboard::getRenderingCapabilities(  ) throw( RuntimeException )
{
    if ( rBHelper.bDisposed )
        throw DisposedException( OUString::createFromAscii( "object is already disposed" ),
                                 static_cast< XClipboardEx* >( this ) );

    if ( NULL != m_pImpl.get( ) )
        return m_pImpl->getRenderingCapabilities( );

    return 0;
}

//========================================================================
// XClipboardNotifier
//========================================================================

//------------------------------------------------------------------------
// getName
//------------------------------------------------------------------------

void SAL_CALL CWinClipboard::addClipboardListener( const Reference< XClipboardListener >& listener )
    throw( RuntimeException )
{
    if ( rBHelper.bDisposed )
        throw DisposedException( OUString::createFromAscii( "object is already disposed" ),
                                 static_cast< XClipboardEx* >( this ) );

    // check input parameter
    if ( !listener.is( ) )
        throw IllegalArgumentException( OUString::createFromAscii( "empty reference" ),
                                        static_cast< XClipboardEx* >( this ),
                                        1 );

    // if the listener supports XComponent then we will register
    // as a listener so that we can remove the listener from our
    // list in case of disposing if not already done
    Reference< XComponent > xComponent( listener, UNO_QUERY );
    if ( xComponent.is( ) )
        xComponent->addEventListener( static_cast< XEventListener* >(this) );

    rBHelper.aLC.addInterface( getCppuType( &listener ), listener );
}

//------------------------------------------------------------------------
// getName
//------------------------------------------------------------------------

void SAL_CALL CWinClipboard::removeClipboardListener( const Reference< XClipboardListener >& listener )
    throw( RuntimeException )
{
    if ( rBHelper.bDisposed )
        throw DisposedException( OUString::createFromAscii( "object is already disposed" ),
                                 static_cast< XClipboardEx* >( this ) );

    // check input parameter
    if ( !listener.is( ) )
        throw IllegalArgumentException( OUString::createFromAscii( "empty reference" ),
                                        static_cast< XClipboardEx* >( this ),
                                        1 );

    // remove ourself as an eventlistener from this component if
    // it supports XComponent
    Reference< XComponent > xComponent( listener, UNO_QUERY );
    if ( xComponent.is( ) )
        xComponent->removeEventListener( static_cast< XEventListener* >(this) );

    rBHelper.aLC.removeInterface( getCppuType( &listener ), listener );
}

//------------------------------------------------------------------------
// getName
//------------------------------------------------------------------------

void SAL_CALL CWinClipboard::notifyAllClipboardListener( )
{
    if ( !rBHelper.bDisposed )
    {
        ClearableMutexGuard aGuard( rBHelper.rMutex );
        if ( !rBHelper.bDisposed )
        {
            aGuard.clear( );

            OInterfaceContainerHelper* pICHelper = rBHelper.aLC.getContainer(
                getCppuType( ( Reference< XClipboardListener > * ) 0 ) );

            if ( pICHelper )
            {
                OInterfaceIteratorHelper iter( *pICHelper );
                Reference< XTransferable > rXTransf = m_pImpl->getContents( );
                ClipboardEvent aClipbEvent( static_cast< XClipboard* >( this ), rXTransf );

                while( iter.hasMoreElements( ) )
                {
                    try
                    {
                        Reference< XClipboardListener > xCBListener( iter.next( ), UNO_QUERY );
                        if ( xCBListener.is( ) )
                            xCBListener->changedContents( aClipbEvent );
                    }
                    catch( RuntimeException& )
                    {
                        OSL_ENSURE( false, "RuntimeException caught" );
                    }
                    catch( ... )
                    {
                        OSL_ENSURE( false, "Exception during event dispatching" );
                    }
                } // while
            } // end if
        } // end if
    } // end if
}

//------------------------------------------------
// overwritten base class method which will be
// called by the base class dispose method
//------------------------------------------------

void SAL_CALL CWinClipboard::disposing()
{
    // do my own stuff
    m_pImpl->dispose( );

    // force destruction of the impl class
    m_pImpl.reset( NULL );
}

// -------------------------------------------------
// XEventListener
// -------------------------------------------------

void SAL_CALL CWinClipboard::disposing( const EventObject& aEvent ) throw(RuntimeException)
{
    Reference< XClipboardListener > xClipboardListener( aEvent.Source, UNO_QUERY );

    if ( xClipboardListener.is( ) )
        removeClipboardListener( xClipboardListener );
}

// -------------------------------------------------
// XServiceInfo
// -------------------------------------------------

OUString SAL_CALL CWinClipboard::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString::createFromAscii( WINCLIPBOARD_IMPL_NAME );
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

sal_Bool SAL_CALL CWinClipboard::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    Sequence < OUString > SupportedServicesNames = WinClipboard_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// -------------------------------------------------
//  XServiceInfo
// -------------------------------------------------

Sequence< OUString > SAL_CALL CWinClipboard::getSupportedServiceNames(   )
    throw(RuntimeException)
{
    return WinClipboard_getSupportedServiceNames();
}