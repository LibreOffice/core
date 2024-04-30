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

#include <standard/vclxaccessiblemenubar.hxx>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <comphelper/accessiblecontexthelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/menu.hxx>
#include <vcl/settings.hxx>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;
using namespace ::comphelper;




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


IMPL_LINK( VCLXAccessibleMenuBar, WindowEventListener, VclWindowEvent&, rEvent, void )
{
    assert( rEvent.GetWindow() );
    if ( !rEvent.GetWindow()->IsAccessibilityEventsSuppressed() || ( rEvent.GetId() == VclEventId::ObjectDying ) )
    {
        ProcessWindowEvent( rEvent );
    }
}


void VCLXAccessibleMenuBar::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::WindowGetFocus:
        case VclEventId::WindowLoseFocus:
        {
            SetFocused( rVclWindowEvent.GetId() == VclEventId::WindowGetFocus );
        }
        break;
        case VclEventId::ObjectDying:
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


OUString VCLXAccessibleMenuBar::getImplementationName()
{
    return u"com.sun.star.comp.toolkit.AccessibleMenuBar"_ustr;
}


Sequence< OUString > VCLXAccessibleMenuBar::getSupportedServiceNames()
{
    return { u"com.sun.star.awt.AccessibleMenuBar"_ustr };
}


// XAccessibleContext


sal_Int64 VCLXAccessibleMenuBar::getAccessibleIndexInParent(  )
{
    OExternalLockGuard aGuard( this );

    sal_Int64 nIndexInParent = -1;

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


sal_Int16 VCLXAccessibleMenuBar::getAccessibleRole(  )
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::MENU_BAR;
}


// XAccessibleExtendedComponent


sal_Int32 VCLXAccessibleMenuBar::getBackground(  )
{
    OExternalLockGuard aGuard( this );

    return sal_Int32(Application::GetSettings().GetStyleSettings().GetMenuBarColor());
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
