/*************************************************************************
 *
 *  $RCSfile: X11_droptarget.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: pl $ $Date: 2001-02-14 16:32:31 $
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
 *  WITHOUT WARRUNTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRUNTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <X11_selection.hxx>

using namespace x11;
using namespace rtl;
using namespace com::sun::star::lang;
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
    m_aTargetWindow( None )
{
}

DropTarget::~DropTarget()
{
    SelectionManager::get().deregisterDropTarget( m_aTargetWindow );
}

// --------------------------------------------------------------------------

void DropTarget::initialize( const Sequence< Any >& args )
{
    if( args.getLength() > 1 )
    {
        sal_Int32 aWindow = None;
        args.getConstArray()[1] >>= aWindow;
        SelectionManager::get().initialize( args );
        SelectionManager::get().registerDropTarget( aWindow, this );
        m_bActive = true;
    }
}

// --------------------------------------------------------------------------

void DropTarget::addDropTargetListener( const Reference< XDropTargetListener >& xListener )
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_aListeners.push_back( xListener );
}

// --------------------------------------------------------------------------

void DropTarget::removeDropTargetListener( const Reference< XDropTargetListener >& xListener )
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_aListeners.remove( xListener );
}

// --------------------------------------------------------------------------

sal_Bool DropTarget::isActive()
{
    return m_bActive;
}

// --------------------------------------------------------------------------

void DropTarget::setActive( sal_Bool active )
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_bActive = active;
}

// --------------------------------------------------------------------------

sal_Int8 DropTarget::getDefaultActions()
{
    return m_nDefaultActions;
}

// --------------------------------------------------------------------------

void DropTarget::setDefaultActions( sal_Int8 actions )
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_nDefaultActions = actions;
}

// --------------------------------------------------------------------------

void DropTarget::drop( const DropTargetDropEvent& dtde )
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

void DropTarget::dragEnter( const DropTargetDragEnterEvent& dtde )
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

void DropTarget::dragExit( const DropTargetEvent& dte )
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

void DropTarget::dragOver( const DropTargetDragEvent& dtde )
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

void DropTarget::dropActionChanged( const DropTargetDragEvent& dtde )
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    ::std::list< Reference< XDropTargetListener > >::iterator it1, it2;
    it1 = m_aListeners.begin();
    while( it1 != m_aListeners.end() )
    {
        it2 = it1;
        it1++;
        (*it2)->dropActionChanged( dtde );
    }
}

/*
 *  XServiceInfo
 */

// ------------------------------------------------------------------------

OUString DropTarget::getImplementationName(  )
{
    return OUString::createFromAscii(XDND_DROPTARGET_IMPLEMENTATION_NAME);
}

// ------------------------------------------------------------------------

sal_Bool DropTarget::supportsService( const OUString& ServiceName )
{
    Sequence < OUString > SupportedServicesNames = Xdnd_dropTarget_getSupportedServiceNames();

    for ( sal_Int32 n = SupportedServicesNames.getLength(); n--; )
        if (SupportedServicesNames[n].compareTo(ServiceName) == 0)
            return sal_True;

    return sal_False;
}

// ------------------------------------------------------------------------

Sequence< OUString > DropTarget::getSupportedServiceNames()
{
    return Xdnd_dropTarget_getSupportedServiceNames();
}


