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

#include <accessibility/standard/vclxaccessiblemenubar.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::comphelper;



// class VCLXAccessibleMenuBar


VCLXAccessibleMenuBar::VCLXAccessibleMenuBar( Menu* pMenu )
    :OAccessibleMenuComponent( pMenu )
{
    if ( pMenu )
    {
        m_pWindow = pMenu->GetWindow();

        if ( m_pWindow )
            m_pWindow->AddEventListener( LINK( this, VCLXAccessibleMenuBar, WindowEventListener ) );
    }
    else
    {
        m_pWindow = nullptr;
    }
}



VCLXAccessibleMenuBar::~VCLXAccessibleMenuBar()
{
    if ( m_pWindow )
        m_pWindow->RemoveEventListener( LINK( this, VCLXAccessibleMenuBar, WindowEventListener ) );
}



bool VCLXAccessibleMenuBar::IsFocused()
{
    bool bFocused = false;

    if ( m_pWindow && m_pWindow->HasFocus() && !IsChildHighlighted() )
        bFocused = true;

    return bFocused;
}



IMPL_LINK_TYPED( VCLXAccessibleMenuBar, WindowEventListener, VclWindowEvent&, rEvent, void )
{
    OSL_ENSURE( rEvent.GetWindow(), "VCLXAccessibleMenuBar::WindowEventListener: no window!" );
    if ( !rEvent.GetWindow()->IsAccessibilityEventsSuppressed() || ( rEvent.GetId() == VCLEVENT_OBJECT_DYING ) )
    {
        ProcessWindowEvent( rEvent );
    }
}



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
                m_pWindow = nullptr;
            }
        }
        break;
        default:
        {
        }
        break;
    }
}


// XComponent


void VCLXAccessibleMenuBar::disposing()
{
    OAccessibleMenuComponent::disposing();

    if ( m_pWindow )
    {
        m_pWindow->RemoveEventListener( LINK( this, VCLXAccessibleMenuBar, WindowEventListener ) );
        m_pWindow = nullptr;
    }
}


// XServiceInfo


OUString VCLXAccessibleMenuBar::getImplementationName() throw (RuntimeException, std::exception)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleMenuBar" );
}



Sequence< OUString > VCLXAccessibleMenuBar::getSupportedServiceNames() throw (RuntimeException, std::exception)
{
    Sequence< OUString > aNames { "com.sun.star.awt.AccessibleMenuBar" };
    return aNames;
}


// XAccessibleContext


sal_Int32 VCLXAccessibleMenuBar::getAccessibleIndexInParent(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nIndexInParent = -1;

    if ( m_pMenu )
    {
        vcl::Window* pWindow = m_pMenu->GetWindow();
        if ( pWindow )
        {
            vcl::Window* pParent = pWindow->GetAccessibleParentWindow();
            if ( pParent )
            {
                for ( sal_uInt16 n = pParent->GetAccessibleChildWindowCount(); n; )
                {
                    vcl::Window* pChild = pParent->GetAccessibleChildWindow( --n );
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



sal_Int16 VCLXAccessibleMenuBar::getAccessibleRole(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::MENU_BAR;
}


// XAccessibleExtendedComponent


sal_Int32 VCLXAccessibleMenuBar::getBackground(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetStyleSettings().GetMenuBarColor().GetColor();
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
