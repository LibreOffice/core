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

#include <cppuhelper/supportsservice.hxx>
#include <X11_selection.hxx>

using namespace x11;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::awt;
using namespace com::sun::star::datatransfer;
using namespace com::sun::star::datatransfer::dnd;

DropTarget::DropTarget() :
        ::cppu::WeakComponentImplHelper<
            XDropTarget,
            XInitialization,
            XServiceInfo
        >( m_aMutex ),
    m_bActive( false ),
    m_nDefaultActions( 0 ),
    m_aTargetWindow( None ),
    m_pSelectionManager( nullptr )
{
}

DropTarget::~DropTarget()
{
    if( m_pSelectionManager )
        m_pSelectionManager->deregisterDropTarget( m_aTargetWindow );
}

void DropTarget::initialize( const Sequence< Any >& arguments ) throw( css::uno::Exception, std::exception )
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

void DropTarget::addDropTargetListener( const Reference< XDropTargetListener >& xListener ) throw(std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_aListeners.push_back( xListener );
}

void DropTarget::removeDropTargetListener( const Reference< XDropTargetListener >& xListener ) throw(std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_aListeners.remove( xListener );
}

sal_Bool DropTarget::isActive() throw(std::exception)
{
    return m_bActive;
}

void DropTarget::setActive( sal_Bool active ) throw(std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_bActive = active;
}

sal_Int8 DropTarget::getDefaultActions() throw(std::exception)
{
    return m_nDefaultActions;
}

void DropTarget::setDefaultActions( sal_Int8 actions ) throw(std::exception)
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_nDefaultActions = actions;
}

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

// XServiceInfo
OUString DropTarget::getImplementationName() throw(std::exception)
{
    return OUString(XDND_DROPTARGET_IMPLEMENTATION_NAME);
}

sal_Bool DropTarget::supportsService( const OUString& ServiceName ) throw(std::exception)
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > DropTarget::getSupportedServiceNames() throw(std::exception)
{
    return Xdnd_dropTarget_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
