/*************************************************************************
 *
 *  $RCSfile: X11_clipboard.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: vg $ $Date: 2003-04-15 17:09:31 $
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

#include <X11/Xatom.h>
#include <X11_clipboard.hxx>
#include <X11_transferable.hxx>

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_RENDERINGCAPABILITIES_HPP_
#include <com/sun/star/datatransfer/clipboard/RenderingCapabilities.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif

#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif

#ifndef _UNO_DISPATCHER_H_
#include <uno/dispatcher.h> // declaration of generic uno interface
#endif
#ifndef _UNO_MAPPING_HXX_
#include <uno/mapping.hxx> // mapping stuff
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_
#include <cppuhelper/factory.hxx>
#endif
#ifndef _RTL_TENCINFO_H
#include <rtl/tencinfo.h>
#endif

#if OSL_DEBUG_LEVEL > 1
#include <stdio.h>
#endif

using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::clipboard;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::awt;
using namespace cppu;
using namespace osl;
using namespace rtl;
using namespace x11;

X11Clipboard::X11Clipboard( SelectionManager& rManager, Atom aSelection ) :
        ::cppu::WeakComponentImplHelper4<
    ::com::sun::star::datatransfer::clipboard::XClipboardEx,
    ::com::sun::star::datatransfer::clipboard::XClipboardNotifier,
    ::com::sun::star::lang::XServiceInfo,
    ::com::sun::star::lang::XInitialization
    >( m_aMutex ),

        m_rSelectionManager( rManager ),
        m_xSelectionManager( & rManager ),
        m_aSelection( aSelection )
{
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "creating instance of X11Clipboard (this=%x)\n", this );
#endif

    if( m_aSelection != None )
    {
        m_rSelectionManager.registerHandler( m_aSelection, *this );
    }
    else
    {
        m_rSelectionManager.registerHandler( XA_PRIMARY, *this );
        m_rSelectionManager.registerHandler( m_rSelectionManager.getAtom( OUString::createFromAscii( "CLIPBOARD" ) ), *this );
    }
}

// ------------------------------------------------------------------------

X11Clipboard::~X11Clipboard()
{
    MutexGuard aGuard( *Mutex::getGlobalMutex() );

#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "shutting down instance of X11Clipboard (this=%x, Selecttion=\"%s\")\n", this, OUStringToOString( m_rSelectionManager.getString( m_aSelection ), RTL_TEXTENCODING_ISO_8859_1 ).getStr() );
#endif
    if( m_aSelection != None )
        m_rSelectionManager.deregisterHandler( m_aSelection );
    else
    {
        m_rSelectionManager.deregisterHandler( XA_PRIMARY );
        m_rSelectionManager.deregisterHandler( m_rSelectionManager.getAtom( OUString::createFromAscii( "CLIPBOARD" ) ) );
    }
}


// ------------------------------------------------------------------------

void X11Clipboard::fireChangedContentsEvent()
{
    ClearableMutexGuard aGuard( m_aMutex );
#if OSL_DEBUG_LEVEL > 1
    fprintf( stderr, "X11Clipboard::fireChangedContentsEvent for %s (%d listeners)\n",
             OUStringToOString( m_rSelectionManager.getString( m_aSelection ), RTL_TEXTENCODING_ISO_8859_1 ).getStr(), m_aListeners.size() );
#endif
    ::std::list< Reference< XClipboardListener > > listeners( m_aListeners );
    aGuard.clear();

    ClipboardEvent aEvent( static_cast<OWeakObject*>(this), m_aContents);
    while( listeners.begin() != listeners.end() )
    {
        if( listeners.front().is() )
            listeners.front()->changedContents(aEvent);
        listeners.pop_front();
    }
}

// ------------------------------------------------------------------------

void X11Clipboard::clearContents()
{
    MutexGuard aGuard(m_aMutex);

    if ( m_aOwner.is() )
    {
        m_aOwner->lostOwnership(static_cast < XClipboard * > (this), m_aContents);
        m_aOwner.clear();
    }

    // may be set even if no owner.
    m_aContents.clear();
}

// ------------------------------------------------------------------------

Reference< XTransferable > SAL_CALL X11Clipboard::getContents()
    throw(RuntimeException)
{
    MutexGuard aGuard(m_aMutex);

    if( ! m_aContents.is() )
        m_aContents = new X11Transferable( SelectionManager::get(), static_cast< OWeakObject* >(this), m_aSelection );
    return m_aContents;
}

// ------------------------------------------------------------------------

void SAL_CALL X11Clipboard::setContents(
    const Reference< XTransferable >& xTrans,
    const Reference< XClipboardOwner >& xClipboardOwner )
    throw(RuntimeException)
{
    // remember old values for callbacks before setting the new ones.
    ClearableMutexGuard aGuard(m_aMutex);

    Reference< XClipboardOwner > oldOwner( m_aOwner );
    m_aOwner = xClipboardOwner;

    Reference< XTransferable > oldContents( m_aContents );
    m_aContents = xTrans;

    aGuard.clear();

    // for now request ownership for both selections
    if( m_aSelection != None )
        m_rSelectionManager.requestOwnership( m_aSelection );
    else
    {
        m_rSelectionManager.requestOwnership( XA_PRIMARY );
        m_rSelectionManager.requestOwnership( m_rSelectionManager.getAtom( OUString::createFromAscii( "CLIPBOARD" ) ) );
    }

    // notify old owner on loss of ownership
    if( oldOwner.is() )
        oldOwner->lostOwnership(static_cast < XClipboard * > (this), oldContents);

    // notify all listeners on content changes
    fireChangedContentsEvent();
}

// ------------------------------------------------------------------------

OUString SAL_CALL X11Clipboard::getName()
    throw(RuntimeException)
{
    return m_rSelectionManager.getString( m_aSelection );
}

// ------------------------------------------------------------------------

sal_Int8 SAL_CALL X11Clipboard::getRenderingCapabilities()
    throw(RuntimeException)
{
    return RenderingCapabilities::Delayed;
}


// ------------------------------------------------------------------------
void SAL_CALL X11Clipboard::addClipboardListener( const Reference< XClipboardListener >& listener )
    throw(RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_aListeners.push_back( listener );
}

// ------------------------------------------------------------------------

void SAL_CALL X11Clipboard::removeClipboardListener( const Reference< XClipboardListener >& listener )
    throw(RuntimeException)
{
    MutexGuard aGuard( m_aMutex );
    m_aListeners.remove( listener );
}


// ------------------------------------------------------------------------

Reference< XTransferable > X11Clipboard::getTransferable()
{
    return getContents();
}

// ------------------------------------------------------------------------

void X11Clipboard::clearTransferable()
{
    clearContents();
}

// ------------------------------------------------------------------------

void X11Clipboard::fireContentsChanged()
{
    fireChangedContentsEvent();
}

// ------------------------------------------------------------------------

OUString SAL_CALL X11Clipboard::getImplementationName(  )
    throw(RuntimeException)
{
    return OUString::createFromAscii(X11_CLIPBOARD_IMPLEMENTATION_NAME);
}

// ------------------------------------------------------------------------

sal_Bool SAL_CALL X11Clipboard::supportsService( const OUString& ServiceName )
    throw(RuntimeException)
{
    Sequence < OUString > SupportedServicesNames = X11Clipboard_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// ------------------------------------------------------------------------

void SAL_CALL X11Clipboard::initialize( const Sequence< Any >& rArgs ) throw( ::com::sun::star::uno::Exception )
{
}

// ------------------------------------------------------------------------

Sequence< OUString > SAL_CALL X11Clipboard::getSupportedServiceNames(    )
    throw(RuntimeException)
{
    return X11Clipboard_getSupportedServiceNames();
}

