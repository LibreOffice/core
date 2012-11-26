/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"
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
    VclWindowEvent* pVclWindowEvent = dynamic_cast< VclWindowEvent* >(pEvent);
    DBG_ASSERT( pVclWindowEvent , "VCLXAccessibleMenuBar::WindowEventListener: unknown window event!" );

    if ( pVclWindowEvent )
    {
        DBG_ASSERT( pVclWindowEvent->GetWindow(), "VCLXAccessibleMenuBar::WindowEventListener: no window!" );

        if ( !pVclWindowEvent->GetWindow()->IsAccessibilityEventsSuppressed() || ( pEvent->GetId() == VCLEVENT_OBJECT_DYING ) )
        {
            ProcessWindowEvent( *pVclWindowEvent );
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
