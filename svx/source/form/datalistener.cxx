/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_svx.hxx"

#include "datalistener.hxx"
#include "datanavi.hxx"

using namespace ::com::sun::star::container;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::dom::events;

//............................................................................
namespace svxform
{
//............................................................................

    DataListener::DataListener( DataNavigatorWindow* pNaviWin ) :

        m_pNaviWin( pNaviWin )

    {
        DBG_ASSERT( m_pNaviWin, "DataListener::Ctor(): no navigator win" );
    }

    DataListener::~DataListener()
    {
    }

    // XContainerListener
    void SAL_CALL DataListener::elementInserted( const ContainerEvent& /*Event*/ ) throw (RuntimeException)
    {
        m_pNaviWin->NotifyChanges();
    }

    void SAL_CALL DataListener::elementRemoved( const ContainerEvent& /*Event*/ ) throw (RuntimeException)
    {
        m_pNaviWin->NotifyChanges();
    }

    void SAL_CALL DataListener::elementReplaced( const ContainerEvent& /*Event*/ ) throw (RuntimeException)
    {
        m_pNaviWin->NotifyChanges();
    }

    // XFrameActionListener
    void SAL_CALL DataListener::frameAction( const FrameActionEvent& rActionEvt ) throw (RuntimeException)
    {
        if ( FrameAction_COMPONENT_ATTACHED == rActionEvt.Action ||
             FrameAction_COMPONENT_REATTACHED == rActionEvt.Action )
        {
            m_pNaviWin->NotifyChanges( FrameAction_COMPONENT_REATTACHED == rActionEvt.Action );
        }
    }

    // xml::dom::events::XEventListener
    void SAL_CALL DataListener::handleEvent( const Reference< XEvent >& /*evt*/ ) throw (RuntimeException)
    {
        m_pNaviWin->NotifyChanges();
    }

    // lang::XEventListener
    void SAL_CALL DataListener::disposing( const EventObject& /*Source*/ ) throw (RuntimeException)
    {
        DBG_ERRORFILE( "disposing" );
    }

//............................................................................
}   // namespace svxform
//............................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
