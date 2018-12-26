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

#include <extended/accessibletabbarbase.hxx>
#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABBARPAGELIST
#include <extended/accessibletabbarpagelist.hxx>
#endif
#include <svtools/tabbar.hxx>
#include <vcl/vclevent.hxx>


namespace accessibility
{


AccessibleTabBarBase::AccessibleTabBarBase( TabBar* pTabBar ) :
    m_pTabBar( nullptr )
{
    SetTabBarPointer( pTabBar );
}

AccessibleTabBarBase::~AccessibleTabBarBase()
{
    ClearTabBarPointer();
}

IMPL_LINK( AccessibleTabBarBase, WindowEventListener, VclWindowEvent&, rEvent, void )
{
    vcl::Window* pEventWindow = rEvent.GetWindow();
    OSL_ENSURE( pEventWindow, "AccessibleTabBarBase::WindowEventListener: no window!" );

    if( ( rEvent.GetId() == VclEventId::TabbarPageRemoved ) &&
        ( static_cast<sal_uInt16>(reinterpret_cast<sal_IntPtr>(rEvent.GetData())) == TabBar::PAGE_NOT_FOUND ) &&
        (dynamic_cast<AccessibleTabBarPageList *>(this) == nullptr))
    {
        return;
    }

    if ( !pEventWindow->IsAccessibilityEventsSuppressed() || (rEvent.GetId() == VclEventId::ObjectDying) )
        ProcessWindowEvent( rEvent );
}

void AccessibleTabBarBase::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    if( rVclWindowEvent.GetId() == VclEventId::ObjectDying )
        ClearTabBarPointer();
}

// XComponent

void AccessibleTabBarBase::disposing()
{
    OAccessibleExtendedComponentHelper::disposing();
    ClearTabBarPointer();
}

// private

void AccessibleTabBarBase::SetTabBarPointer( TabBar* pTabBar )
{
    OSL_ENSURE( !m_pTabBar, "AccessibleTabBarBase::SetTabBarPointer - multiple call" );
    m_pTabBar = pTabBar;
    if( m_pTabBar )
        m_pTabBar->AddEventListener( LINK( this, AccessibleTabBarBase, WindowEventListener ) );
}

void AccessibleTabBarBase::ClearTabBarPointer()
{
    if( m_pTabBar )
    {
        m_pTabBar->RemoveEventListener( LINK( this, AccessibleTabBarBase, WindowEventListener ) );
        m_pTabBar = nullptr;
    }
}


}   // namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
