/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessiblemenubar.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:39:28 $
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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEMENUBAR_HXX
#include <accessibility/standard/vclxaccessiblemenubar.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_MENU_HXX
#include <vcl/menu.hxx>
#endif


using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::comphelper;


// -----------------------------------------------------------------------------
// class VCLXAccessibleMenuBar
// -----------------------------------------------------------------------------

VCLXAccessibleMenuBar::VCLXAccessibleMenuBar( Menu* pMenu )
    :OAccessibleMenuComponent( pMenu )
{
    if ( pMenu )
    {
        m_pWindow = pMenu->GetWindow();

        if ( m_pWindow )
            m_pWindow->AddEventListener( LINK( this, VCLXAccessibleMenuBar, WindowEventListener ) );
    }
}

// -----------------------------------------------------------------------------

VCLXAccessibleMenuBar::~VCLXAccessibleMenuBar()
{
    if ( m_pWindow )
        m_pWindow->RemoveEventListener( LINK( this, VCLXAccessibleMenuBar, WindowEventListener ) );
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleMenuBar::IsFocused()
{
    sal_Bool bFocused = sal_False;

    if ( m_pWindow && m_pWindow->HasFocus() && !IsChildHighlighted() )
        bFocused = sal_True;

    return bFocused;
}

// -----------------------------------------------------------------------------

IMPL_LINK( VCLXAccessibleMenuBar, WindowEventListener, VclSimpleEvent*, pEvent )
{
    DBG_ASSERT( pEvent && pEvent->ISA( VclWindowEvent ), "VCLXAccessibleMenuBar::WindowEventListener: unknown window event!" );
    if ( pEvent && pEvent->ISA( VclWindowEvent ) )
    {
        DBG_ASSERT( ((VclWindowEvent*)pEvent)->GetWindow(), "VCLXAccessibleMenuBar::WindowEventListener: no window!" );
        if ( !((VclWindowEvent*)pEvent)->GetWindow()->IsAccessibilityEventsSuppressed() || ( pEvent->GetId() == VCLEVENT_OBJECT_DYING ) )
        {
            ProcessWindowEvent( *(VclWindowEvent*)pEvent );
        }
    }
    return 0;
}

// -----------------------------------------------------------------------------

void VCLXAccessibleMenuBar::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_GETFOCUS:
        case VCLEVENT_WINDOW_LOSEFOCUS:
        {
            SetFocused( rVclWindowEvent.GetId() == VCLEVENT_WINDOW_GETFOCUS );
        }
        break;
        case VCLEVENT_OBJECT_DYING:
        {
            if ( m_pWindow )
            {
                m_pWindow->RemoveEventListener( LINK( this, VCLXAccessibleMenuBar, WindowEventListener ) );
                m_pWindow = NULL;
            }
        }
        break;
        default:
        {
        }
        break;
    }
}

// -----------------------------------------------------------------------------
// XComponent
// -----------------------------------------------------------------------------

void VCLXAccessibleMenuBar::disposing()
{
    OAccessibleMenuComponent::disposing();

    if ( m_pWindow )
    {
        m_pWindow->RemoveEventListener( LINK( this, VCLXAccessibleMenuBar, WindowEventListener ) );
        m_pWindow = NULL;
    }
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleMenuBar::getImplementationName() throw (RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.toolkit.AccessibleMenuBar" );
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > VCLXAccessibleMenuBar::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii( "com.sun.star.awt.AccessibleMenuBar" );
    return aNames;
}

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleMenuBar::getAccessibleIndexInParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nIndexInParent = -1;

    if ( m_pMenu )
    {
        Window* pWindow = m_pMenu->GetWindow();
        if ( pWindow )
        {
            Window* pParent = pWindow->GetAccessibleParentWindow();
            if ( pParent )
            {
                for ( USHORT n = pParent->GetAccessibleChildWindowCount(); n; )
                {
                    Window* pChild = pParent->GetAccessibleChildWindow( --n );
                    if ( pChild == pWindow )
                    {
                        nIndexInParent = n;
                        break;
                    }
                }
            }
        }
    }

    return nIndexInParent;
}

// -----------------------------------------------------------------------------

sal_Int16 VCLXAccessibleMenuBar::getAccessibleRole(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::MENU_BAR;
}

// -----------------------------------------------------------------------------
// XAccessibleExtendedComponent
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleMenuBar::getBackground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetStyleSettings().GetMenuBarColor().GetColor();
}

// -----------------------------------------------------------------------------
