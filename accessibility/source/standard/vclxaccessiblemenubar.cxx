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

#include <accessibility/standard/vclxaccessiblemenubar.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/menu.hxx>


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
    OSL_ENSURE( pEvent && pEvent->ISA( VclWindowEvent ), "VCLXAccessibleMenuBar::WindowEventListener: unknown window event!" );
    if ( pEvent && pEvent->ISA( VclWindowEvent ) )
    {
        OSL_ENSURE( ((VclWindowEvent*)pEvent)->GetWindow(), "VCLXAccessibleMenuBar::WindowEventListener: no window!" );
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

OUString VCLXAccessibleMenuBar::getImplementationName() throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleMenuBar" );
}

// -----------------------------------------------------------------------------

Sequence< OUString > VCLXAccessibleMenuBar::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< OUString > aNames(1);
    aNames[0] = "com.sun.star.awt.AccessibleMenuBar";
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
                for ( sal_uInt16 n = pParent->GetAccessibleChildWindowCount(); n; )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
