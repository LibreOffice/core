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
#include "X11_selection.hxx"

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
    m_aTargetWindow( None )
{
}

DropTarget::~DropTarget()
{
    if( m_xSelectionManager.is() )
        m_xSelectionManager->deregisterDropTarget( m_aTargetWindow );
}

void DropTarget::initialize( const Sequence< Any >& arguments )
{
    if( arguments.getLength() <= 1 )
        return;

    OUString aDisplayName;
    Reference< XDisplayConnection > xConn;
    arguments.getConstArray()[0] >>= xConn;
    if( xConn.is() )
    {
        Any aIdentifier;
        aIdentifier >>= aDisplayName;
    }

    m_xSelectionManager = &SelectionManager::get( aDisplayName );
    m_xSelectionManager->initialize( arguments );

    if( m_xSelectionManager->getDisplay() ) // #136582# sanity check
    {
        sal_IntPtr aWindow = None;
        arguments.getConstArray()[1] >>= aWindow;
        m_xSelectionManager->registerDropTarget( aWindow, this );
        m_aTargetWindow = aWindow;
        m_bActive = true;
    }
}

void DropTarget::addDropTargetListener( const Reference< XDropTargetListener >& xListener )
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_aListeners.push_back( xListener );
}

void DropTarget::removeDropTargetListener( const Reference< XDropTargetListener >& xListener )
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_aListeners.erase( std::remove(m_aListeners.begin(), m_aListeners.end(), xListener), m_aListeners.end() );
}

sal_Bool DropTarget::isActive()
{
    return m_bActive;
}

void DropTarget::setActive( sal_Bool active )
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_bActive = active;
}

sal_Int8 DropTarget::getDefaultActions()
{
    return m_nDefaultActions;
}

void DropTarget::setDefaultActions( sal_Int8 actions )
{
    ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

    m_nDefaultActions = actions;
}

void DropTarget::drop( const DropTargetDropEvent& dtde ) noexcept
{
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::vector< Reference< XDropTargetListener > > aListeners( m_aListeners );
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->drop(dtde);
    }
}

void DropTarget::dragEnter( const DropTargetDragEnterEvent& dtde ) noexcept
{
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::vector< Reference< XDropTargetListener > > aListeners( m_aListeners );
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->dragEnter(dtde);
    }
}

void DropTarget::dragExit( const DropTargetEvent& dte ) noexcept
{
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::vector< Reference< XDropTargetListener > > aListeners( m_aListeners );
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->dragExit(dte);
    }
}

void DropTarget::dragOver( const DropTargetDragEvent& dtde ) noexcept
{
    osl::ClearableGuard< ::osl::Mutex > aGuard( m_aMutex );
    std::vector< Reference< XDropTargetListener > > aListeners( m_aListeners );
    aGuard.clear();

    for (auto const& listener : aListeners)
    {
        listener->dragOver(dtde);
    }
}

// XServiceInfo
OUString DropTarget::getImplementationName()
{
    return "com.sun.star.datatransfer.dnd.XdndDropTarget";
}

sal_Bool DropTarget::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > DropTarget::getSupportedServiceNames()
{
    return Xdnd_dropTarget_getSupportedServiceNames();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
