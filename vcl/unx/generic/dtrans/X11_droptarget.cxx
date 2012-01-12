/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

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
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::list< Reference< XDropTargetListener > > aListeners( m_aListeners );
    aGuard.clear();

    for( std::list< Reference< XDropTargetListener > >::iterator it = aListeners.begin(); it!= aListeners.end(); ++it )
    {
        (*it)->drop( dtde );
    }
}

// --------------------------------------------------------------------------

void DropTarget::dragEnter( const DropTargetDragEnterEvent& dtde ) throw()
{
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::list< Reference< XDropTargetListener > > aListeners( m_aListeners );
    aGuard.clear();

    for( std::list< Reference< XDropTargetListener > >::iterator it = aListeners.begin(); it!= aListeners.end(); ++it )
    {
        (*it)->dragEnter( dtde );
    }
}

// --------------------------------------------------------------------------

void DropTarget::dragExit( const DropTargetEvent& dte ) throw()
{
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::list< Reference< XDropTargetListener > > aListeners( m_aListeners );
    aGuard.clear();

    for( std::list< Reference< XDropTargetListener > >::iterator it = aListeners.begin(); it!= aListeners.end(); ++it )
    {
        (*it)->dragExit( dte );
    }
}

// --------------------------------------------------------------------------

void DropTarget::dragOver( const DropTargetDragEvent& dtde ) throw()
{
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::list< Reference< XDropTargetListener > > aListeners( m_aListeners );
    aGuard.clear();

    for( std::list< Reference< XDropTargetListener > >::iterator it = aListeners.begin(); it!= aListeners.end(); ++it )
    {
        (*it)->dragOver( dtde );
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


