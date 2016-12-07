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

#include <extended/listboxaccessible.hxx>
#include <svtools/treelistbox.hxx>
#include <vcl/svapp.hxx>

namespace accessibility
{
    // ListBoxAccessibleBase
    ListBoxAccessibleBase::ListBoxAccessibleBase( SvTreeListBox& _rWindow )
        :m_pWindow( &_rWindow )
    {
        m_pWindow->AddEventListener( LINK( this, ListBoxAccessibleBase, WindowEventListener ) );
    }

    ListBoxAccessibleBase::~ListBoxAccessibleBase( )
    {
        if ( m_pWindow )
        {
            // cannot call "dispose" here, as it is abstract, so the VTABLE of the derived class
            // is not intact anymore
            // so we call our "disposing" only
            disposing();
        }
    }

    IMPL_LINK( ListBoxAccessibleBase, WindowEventListener, VclWindowEvent&, rEvent, void )
    {
        OSL_ENSURE( rEvent.GetWindow() , "ListBoxAccessibleBase::WindowEventListener: no event window!" );
        OSL_ENSURE( rEvent.GetWindow() == m_pWindow, "ListBoxAccessibleBase::WindowEventListener: where did this come from?" );

        if ( isAlive() )
        {
            switch ( rEvent.GetId() )
            {
                case  VclEventId::ObjectDying :
                {
                    if ( m_pWindow )
                        m_pWindow->RemoveEventListener( LINK( this, ListBoxAccessibleBase, WindowEventListener ) );
                    m_pWindow = nullptr;
                    dispose();
                    break;
                }
                default: break;
            }
        }
    }

    void ListBoxAccessibleBase::disposing()
    {
        SolarMutexGuard g;
        if ( m_pWindow )
            m_pWindow->RemoveEventListener( LINK( this, ListBoxAccessibleBase, WindowEventListener ) );
        m_pWindow = nullptr;
    }

}   // namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
