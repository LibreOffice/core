/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessiblemenucomponent.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-28 14:14:47 $
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

#ifndef ACCESSIBILITY_STANDARD_ACCESSIBLEMENUCOMPONENT_HXX
#include <accessibility/standard/accessiblemenucomponent.hxx>
#endif

#ifndef _TOOLKIT_AWT_VCLXFONT_HXX
#include <toolkit/awt/vclxfont.hxx>
#endif

#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
#endif

#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef _UTL_ACCESSIBLERELATIONSETHELPER_HXX
#include <unotools/accessiblerelationsethelper.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX
#include <comphelper/sequence.hxx>
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
#ifndef _VCL_UNOHELP2_HXX
#include <vcl/unohelp2.hxx>
#endif


using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;
using namespace ::comphelper;


// -----------------------------------------------------------------------------
// class OAccessibleMenuComponent
// -----------------------------------------------------------------------------

OAccessibleMenuComponent::OAccessibleMenuComponent( Menu* pMenu )
    :OAccessibleMenuBaseComponent( pMenu )
{
}

// -----------------------------------------------------------------------------

OAccessibleMenuComponent::~OAccessibleMenuComponent()
{
}

// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuComponent::IsEnabled()
{
    return sal_True;
}

// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuComponent::IsVisible()
{
    sal_Bool bVisible = sal_False;

    if ( m_pMenu )
        bVisible = m_pMenu->IsMenuVisible();

    return bVisible;
}

// -----------------------------------------------------------------------------

void OAccessibleMenuComponent::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    if ( IsEnabled() )
    {
        rStateSet.AddState( AccessibleStateType::ENABLED );
        rStateSet.AddState( AccessibleStateType::SENSITIVE );
    }

    rStateSet.AddState( AccessibleStateType::FOCUSABLE );

    if ( IsFocused() )
        rStateSet.AddState( AccessibleStateType::FOCUSED );

    if ( IsVisible() )
    {
        rStateSet.AddState( AccessibleStateType::VISIBLE );
        rStateSet.AddState( AccessibleStateType::SHOWING );
    }

    rStateSet.AddState( AccessibleStateType::OPAQUE );
}

// -----------------------------------------------------------------------------
// OCommonAccessibleComponent
// -----------------------------------------------------------------------------

awt::Rectangle OAccessibleMenuComponent::implGetBounds() throw (RuntimeException)
{
    awt::Rectangle aBounds( 0, 0, 0, 0 );

    if ( m_pMenu )
    {
        Window* pWindow = m_pMenu->GetWindow();
        if ( pWindow )
        {
            // get bounding rectangle of the window in screen coordinates
            Rectangle aRect = pWindow->GetWindowExtentsRelative( NULL );
            aBounds = AWTRectangle( aRect );

            // get position of the accessible parent in screen coordinates
            Reference< XAccessible > xParent = getAccessibleParent();
            if ( xParent.is() )
            {
                Reference< XAccessibleComponent > xParentComponent( xParent->getAccessibleContext(), UNO_QUERY );
                if ( xParentComponent.is() )
                {
                    awt::Point aParentScreenLoc = xParentComponent->getLocationOnScreen();

                    // calculate position of the window relative to the accessible parent
                    aBounds.X -= aParentScreenLoc.X;
                    aBounds.Y -= aParentScreenLoc.Y;
                }
            }
        }
    }

    return aBounds;
}

// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( OAccessibleMenuComponent, OAccessibleMenuBaseComponent, OAccessibleMenuComponent_BASE )

// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( OAccessibleMenuComponent, OAccessibleMenuBaseComponent, OAccessibleMenuComponent_BASE )

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

sal_Int32 OAccessibleMenuComponent::getAccessibleChildCount() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return GetChildCount();
}

// -----------------------------------------------------------------------------

Reference< XAccessible > OAccessibleMenuComponent::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || i >= GetChildCount() )
        throw IndexOutOfBoundsException();

    return GetChild( i );
}

// -----------------------------------------------------------------------------

Reference< XAccessible > OAccessibleMenuComponent::getAccessibleParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Reference< XAccessible > xParent;

    if ( m_pMenu )
    {
        Window* pWindow = m_pMenu->GetWindow();
        if ( pWindow )
        {
            Window* pParent = pWindow->GetAccessibleParentWindow();
            if ( pParent )
                xParent = pParent->GetAccessible();
        }
    }

    return xParent;
}

// -----------------------------------------------------------------------------

sal_Int16 OAccessibleMenuComponent::getAccessibleRole(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::UNKNOWN;
}

// -----------------------------------------------------------------------------

::rtl::OUString OAccessibleMenuComponent::getAccessibleDescription( ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    ::rtl::OUString sDescription;
    if ( m_pMenu )
    {
        Window* pWindow = m_pMenu->GetWindow();
        if ( pWindow )
            sDescription = pWindow->GetAccessibleDescription();
    }

    return sDescription;
}

// -----------------------------------------------------------------------------

::rtl::OUString OAccessibleMenuComponent::getAccessibleName(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return ::rtl::OUString();
}

// -----------------------------------------------------------------------------

Reference< XAccessibleRelationSet > OAccessibleMenuComponent::getAccessibleRelationSet(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
    return xSet;
}

// -----------------------------------------------------------------------------

Locale OAccessibleMenuComponent::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetLocale();
}

// -----------------------------------------------------------------------------
// XAccessibleComponent
// -----------------------------------------------------------------------------

Reference< XAccessible > OAccessibleMenuComponent::getAccessibleAtPoint( const awt::Point& rPoint ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return GetChildAt( rPoint );
}

// -----------------------------------------------------------------------------

awt::Point OAccessibleMenuComponent::getLocationOnScreen(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    awt::Point aPos;

    if ( m_pMenu )
    {
        Window* pWindow = m_pMenu->GetWindow();
        if ( pWindow )
        {
            Rectangle aRect = pWindow->GetWindowExtentsRelative( NULL );
            aPos = AWTPoint( aRect.TopLeft() );
        }
    }

    return aPos;
}

// -----------------------------------------------------------------------------

void OAccessibleMenuComponent::grabFocus(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( m_pMenu )
    {
        Window* pWindow = m_pMenu->GetWindow();
        if ( pWindow )
            pWindow->GrabFocus();
    }
}

// -----------------------------------------------------------------------------

sal_Int32 OAccessibleMenuComponent::getForeground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    sal_Int32 nColor = rStyleSettings.GetMenuTextColor().GetColor();

    return nColor;
}

// -----------------------------------------------------------------------------

sal_Int32 OAccessibleMenuComponent::getBackground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return 0;
}

// -----------------------------------------------------------------------------
// XAccessibleExtendedComponent
// -----------------------------------------------------------------------------

Reference< awt::XFont > OAccessibleMenuComponent::getFont(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Reference< awt::XFont > xFont;

    if ( m_pMenu )
    {
        Window* pWindow = m_pMenu->GetWindow();
        if ( pWindow )
        {
            Reference< awt::XDevice > xDev( pWindow->GetComponentInterface(), UNO_QUERY );
            if ( xDev.is() )
            {
                const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
                VCLXFont* pVCLXFont = new VCLXFont;
                pVCLXFont->Init( *xDev.get(), rStyleSettings.GetMenuFont() );
                xFont = pVCLXFont;
            }
        }
    }

    return xFont;
}

// -----------------------------------------------------------------------------

::rtl::OUString OAccessibleMenuComponent::getTitledBorderText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return ::rtl::OUString();
}

// -----------------------------------------------------------------------------

::rtl::OUString OAccessibleMenuComponent::getToolTipText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return ::rtl::OUString();
}

// -----------------------------------------------------------------------------
// XAccessibleSelection
// -----------------------------------------------------------------------------

void OAccessibleMenuComponent::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || nChildIndex >= GetChildCount() )
        throw IndexOutOfBoundsException();

    SelectChild( nChildIndex );
}

// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuComponent::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || nChildIndex >= GetChildCount() )
        throw IndexOutOfBoundsException();

    return IsChildSelected( nChildIndex );
}

// -----------------------------------------------------------------------------

void OAccessibleMenuComponent::clearAccessibleSelection(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    DeSelectAll();
}

// -----------------------------------------------------------------------------

void OAccessibleMenuComponent::selectAllAccessibleChildren(  ) throw (RuntimeException)
{
    // This method makes no sense in a menu, and so does nothing.
}

// -----------------------------------------------------------------------------

sal_Int32 OAccessibleMenuComponent::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nRet = 0;

    for ( sal_Int32 i = 0, nCount = GetChildCount(); i < nCount; i++ )
    {
        if ( IsChildSelected( i ) )
            ++nRet;
    }

    return nRet;
}

// -----------------------------------------------------------------------------

Reference< XAccessible > OAccessibleMenuComponent::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;

    for ( sal_Int32 i = 0, j = 0, nCount = GetChildCount(); i < nCount; i++ )
    {
        if ( IsChildSelected( i ) && ( j++ == nSelectedChildIndex ) )
        {
            xChild = GetChild( i );
            break;
        }
    }

    return xChild;
}

// -----------------------------------------------------------------------------

void OAccessibleMenuComponent::deselectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nChildIndex < 0 || nChildIndex >= GetChildCount() )
        throw IndexOutOfBoundsException();

    DeSelectAll();
}

// -----------------------------------------------------------------------------
