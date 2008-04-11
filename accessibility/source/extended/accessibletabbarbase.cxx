/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accessibletabbarbase.cxx,v $
 * $Revision: 1.3 $
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
#include "precompiled_accessibility.hxx"
#include "accessibility/extended/accessibletabbarbase.hxx"
#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABBARPAGELIST
#include "accessibility/extended/accessibletabbarpagelist.hxx"
#endif
#include <toolkit/helper/externallock.hxx>
#include <svtools/tabbar.hxx>

//.........................................................................
namespace accessibility
{
//.........................................................................

AccessibleTabBarBase::AccessibleTabBarBase( TabBar* pTabBar ) :
    AccessibleExtendedComponentHelper_BASE( new VCLExternalSolarLock() ),
    m_pTabBar( 0 )
{
    m_pExternalLock = static_cast< VCLExternalSolarLock* >( getExternalLock() );
    SetTabBarPointer( pTabBar );
}

AccessibleTabBarBase::~AccessibleTabBarBase()
{
    ClearTabBarPointer();
    DELETEZ( m_pExternalLock );
}

IMPL_LINK( AccessibleTabBarBase, WindowEventListener, VclSimpleEvent*, pEvent )
{
    VclWindowEvent* pWinEvent = dynamic_cast< VclWindowEvent* >( pEvent );
    DBG_ASSERT( pWinEvent, "AccessibleTabBarBase::WindowEventListener - unknown window event" );
    if( pWinEvent )
    {
        Window* pEventWindow = pWinEvent->GetWindow();
        DBG_ASSERT( pEventWindow, "AccessibleTabBarBase::WindowEventListener: no window!" );

        if( ( pWinEvent->GetId() == VCLEVENT_TABBAR_PAGEREMOVED ) &&
            ( (sal_uInt16)(sal_IntPtr) pWinEvent->GetData() == TAB_PAGE_NOTFOUND ) &&
            ( dynamic_cast< AccessibleTabBarPageList *> (this) != NULL ) )
        {
            return 0;
        }

        if ( !pEventWindow->IsAccessibilityEventsSuppressed() || (pWinEvent->GetId() == VCLEVENT_OBJECT_DYING) )
            ProcessWindowEvent( *pWinEvent );
    }
    return 0;
}

void AccessibleTabBarBase::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    if( rVclWindowEvent.GetId() == VCLEVENT_OBJECT_DYING )
        ClearTabBarPointer();
}

// XComponent

void AccessibleTabBarBase::disposing()
{
    AccessibleExtendedComponentHelper_BASE::disposing();
    ClearTabBarPointer();
}

// private

void AccessibleTabBarBase::SetTabBarPointer( TabBar* pTabBar )
{
    DBG_ASSERT( !m_pTabBar, "AccessibleTabBarBase::SetTabBarPointer - multiple call" );
    m_pTabBar = pTabBar;
    if( m_pTabBar )
        m_pTabBar->AddEventListener( LINK( this, AccessibleTabBarBase, WindowEventListener ) );
}

void AccessibleTabBarBase::ClearTabBarPointer()
{
    if( m_pTabBar )
    {
        m_pTabBar->RemoveEventListener( LINK( this, AccessibleTabBarBase, WindowEventListener ) );
        m_pTabBar = 0;
    }
}

//.........................................................................
}   // namespace accessibility
//.........................................................................

