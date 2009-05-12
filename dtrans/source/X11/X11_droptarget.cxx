/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: X11_droptarget.cxx,v $
 * $Revision: 1.14 $
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
#include "precompiled_dtrans.hxx"

#include <X11_selection.hxx>

using namespace x11;
using namespace rtl;
using namespace com::sun::star::lang;
using namespace com::sun::star::awt;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::dnd;

DropTarget::DropTarget() :
        ::cppu::WeakComponentImplHelper3<
            XDropTarget,
            XInitialization,
            XServiceInfo
        >( m_aMutex ),
    m_bActive( false ),
    m_nDefaultActions( 0 ),
    m_aTargetWindow( None ),
    m_pSelectionManager( NULL )
{
}

DropTarget::~DropTarget()
{
    if( m_pSelectionManager )
        m_pSelectionManager->deregisterDropTarget( m_aTargetWindow );
}

// --------------------------------------------------------------------------

void DropTarget::initialize( const Sequence< Any >& arguments ) throw( ::com::sun::star::uno::Exception )
{
    if( arguments.getLength() > 1 )
    {
        OUString aDisplayName;
        Reference< XDisplayConnection > xConn;
        arguments.getConstArray()[0] >>= xConn;
        if( xConn.is() )
        {
            Any aIdentifier;
            aIdentifier >>= aDisplayName;
        }

        m_pSelectionManager = &SelectionManager::get( aDisplayName );
        m_xSelectionManager = static_cast< XDragSource* >(m_pSelectionManager);
        m_pSelectionManager->initialize( arguments );

        if( m_pSelectionManager->getDisplay() ) // #136582# sanity check
        {
            sal_Size aWindow = None;
            arguments.getConstArray()[1] >>= aWindow;
            m_pSelectionManager->registerDropTarget( aWindow, this );
            m_aTargetWindow = aWindow;
            m_bActive = true;
        }
    }
}

// --------------------------------------------------------------------------

void DropTarget::addDropTargetListener( const Reference< XDropTargetListener >& xListener ) throw()
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_aListeners.push_back( xListener );
}

// --------------------------------------------------------------------------

void DropTarget::removeDropTargetListener( const Reference< XDropTargetListener >& xListener ) throw()
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_aListeners.remove( xListener );
}

// --------------------------------------------------------------------------

sal_Bool DropTarget::isActive() throw()
{
    return m_bActive;
}

// --------------------------------------------------------------------------

void DropTarget::setActive( sal_Bool active ) throw()
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_bActive = active;
}

// --------------------------------------------------------------------------

sal_Int8 DropTarget::getDefaultActions() throw()
{
    return m_nDefaultActions;
}

// --------------------------------------------------------------------------

void DropTarget::setDefaultActions( sal_Int8 actions ) throw()
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_nDefaultActions = actions;
}

// --------------------------------------------------------------------------

void DropTarget::drop( const DropTargetDropEvent& dtde ) throw()
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    ::std::list< Reference< XDropTargetListener > >::iterator it1, it2;
    it1 = m_aListeners.begin();
    while( it1 != m_aListeners.end() )
    {
        it2 = it1;
        it1++;
        (*it2)->drop( dtde );
    }
}

// --------------------------------------------------------------------------

void DropTarget::dragEnter( const DropTargetDragEnterEvent& dtde ) throw()
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    ::std::list< Reference< XDropTargetListener > >::iterator it1, it2;
    it1 = m_aListeners.begin();
    while( it1 != m_aListeners.end() )
    {
        it2 = it1;
        it1++;
        (*it2)->dragEnter( dtde );
    }
}

// --------------------------------------------------------------------------

void DropTarget::dragExit( const DropTargetEvent& dte ) throw()
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    ::std::list< Reference< XDropTargetListener > >::iterator it1, it2;
    it1 = m_aListeners.begin();
    while( it1 != m_aListeners.end() )
    {
        it2 = it1;
        it1++;
        (*it2)->dragExit( dte );
    }
}

// --------------------------------------------------------------------------

void DropTarget::dragOver( const DropTargetDragEvent& dtde ) throw()
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    ::std::list< Reference< XDropTargetListener > >::iterator it1, it2;
    it1 = m_aListeners.begin();
    while( it1 != m_aListeners.end() )
    {
        it2 = it1;
        it1++;
        (*it2)->dragOver( dtde );
    }
}

// --------------------------------------------------------------------------

/*
 *  XServiceInfo
 */

// ------------------------------------------------------------------------

OUString DropTarget::getImplementationName() throw()
{
    return OUString::createFromAscii(XDND_DROPTARGET_IMPLEMENTATION_NAME);
}

// ------------------------------------------------------------------------

sal_Bool DropTarget::supportsService( const OUString& ServiceName ) throw()
{
    Sequence < OUString > SupportedServicesNames = Xdnd_dropTarget_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > DropTarget::getSupportedServiceNames() throw()
{
    return Xdnd_dropTarget_getSupportedServiceNames();
}


