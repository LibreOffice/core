/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessibletabbarbase.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:33:22 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"

#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABBARBASE_HXX
#include "accessibility/extended/accessibletabbarbase.hxx"
#endif
#ifndef ACCESSIBILITY_EXT_ACCESSIBLETABBARPAGELIST
#include "accessibility/extended/accessibletabbarpagelist.hxx"
#endif
#ifndef _TOOLKIT_HELPER_EXTERNALLOCK_HXX_
#include <toolkit/helper/externallock.hxx>
#endif
#ifndef _TABBAR_HXX
#include <svtools/tabbar.hxx>
#endif

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

