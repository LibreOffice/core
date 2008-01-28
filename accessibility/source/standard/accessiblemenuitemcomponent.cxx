/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: accessiblemenuitemcomponent.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: vg $ $Date: 2008-01-28 14:15:01 $
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

#ifndef ACCESSIBILITY_STANDARD_ACCESSIBLEMENUITEMCOMPONENT_HXX
#include <accessibility/standard/accessiblemenuitemcomponent.hxx>
#endif


#ifndef ACCESSIBILITY_HELPER_TKARESMGR_HXX
#include <accessibility/helper/accresmgr.hxx>
#endif
#ifndef ACCESSIBILITY_HELPER_ACCESSIBLESTRINGS_HRC_
#include <accessibility/helper/accessiblestrings.hrc>
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOWS_HXX
#include <toolkit/awt/vclxwindows.hxx>
#endif
#ifndef _TOOLKIT_HELPER_EXTERNALLOCK_HXX_
#include <toolkit/helper/externallock.hxx>
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
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XFLUSHABLECLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
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
#ifndef COMPHELPER_ACCESSIBLE_TEXT_HELPER_HXX
#include <comphelper/accessibletexthelper.hxx>
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
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;
using namespace ::comphelper;


// -----------------------------------------------------------------------------
// class OAccessibleMenuItemComponent
// -----------------------------------------------------------------------------

OAccessibleMenuItemComponent::OAccessibleMenuItemComponent( Menu* pParent, sal_uInt16 nItemPos, Menu* pMenu )
    :OAccessibleMenuBaseComponent( pMenu )
    ,m_pParent( pParent )
    ,m_nItemPos( nItemPos )
{
    m_sAccessibleName = GetAccessibleName();
    m_sItemText = GetItemText();
}

// -----------------------------------------------------------------------------

OAccessibleMenuItemComponent::~OAccessibleMenuItemComponent()
{
}

// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuItemComponent::IsEnabled()
{
    OExternalLockGuard aGuard( this );

    sal_Bool bEnabled = sal_False;
    if ( m_pParent )
        bEnabled = m_pParent->IsItemEnabled( m_pParent->GetItemId( m_nItemPos ) );

    return bEnabled;
}

// -----------------------------------------------------------------------------

sal_Bool OAccessibleMenuItemComponent::IsVisible()
{
    sal_Bool bVisible = sal_False;

    if ( m_pParent )
        bVisible = m_pParent->IsItemPosVisible( m_nItemPos );

    return bVisible;
}

// -----------------------------------------------------------------------------

void OAccessibleMenuItemComponent::Select()
{
    // open the parent menu
    Reference< XAccessible > xParent( getAccessibleParent() );
    if ( xParent.is() )
    {
        OAccessibleMenuBaseComponent* pComp = static_cast< OAccessibleMenuBaseComponent* >( xParent.get() );
        if ( pComp && pComp->getAccessibleRole() == AccessibleRole::MENU && !pComp->IsPopupMenuOpen() )
            pComp->Click();
    }

    // highlight the menu item
    if ( m_pParent )
        m_pParent->HighlightItem( m_nItemPos );
}

// -----------------------------------------------------------------------------

void OAccessibleMenuItemComponent::DeSelect()
{
    if ( m_pParent && IsSelected() )
        m_pParent->DeHighlight();
}

// -----------------------------------------------------------------------------

void OAccessibleMenuItemComponent::Click()
{
    // open the parent menu
    Reference< XAccessible > xParent( getAccessibleParent() );
    if ( xParent.is() )
    {
        OAccessibleMenuBaseComponent* pComp = static_cast< OAccessibleMenuBaseComponent* >( xParent.get() );
        if ( pComp && pComp->getAccessibleRole() == AccessibleRole::MENU && !pComp->IsPopupMenuOpen() )
            pComp->Click();
    }

    // click the menu item
    if ( m_pParent )
    {
        Window* pWindow = m_pParent->GetWindow();
        if ( pWindow )
        {
            // #102438# Menu items are not selectable
            // Popup menus are executed asynchronously, triggered by a timer.
            // As Menu::SelectItem only works, if the corresponding menu window is
            // already created, we have to set the menu delay to 0, so
            // that the popup menus are executed synchronously.
            AllSettings aSettings = pWindow->GetSettings();
            MouseSettings aMouseSettings = aSettings.GetMouseSettings();
            ULONG nDelay = aMouseSettings.GetMenuDelay();
            aMouseSettings.SetMenuDelay( 0 );
            aSettings.SetMouseSettings( aMouseSettings );
            pWindow->SetSettings( aSettings );

            m_pParent->SelectItem( m_pParent->GetItemId( m_nItemPos ) );

            // meanwhile the window pointer may be invalid
            pWindow = m_pParent->GetWindow();
            if ( pWindow )
            {
                // set the menu delay back to the old value
                aSettings = pWindow->GetSettings();
                aMouseSettings = aSettings.GetMouseSettings();
                aMouseSettings.SetMenuDelay( nDelay );
                aSettings.SetMouseSettings( aMouseSettings );
                pWindow->SetSettings( aSettings );
            }
        }
    }
}

// -----------------------------------------------------------------------------

void OAccessibleMenuItemComponent::SetItemPos( sal_uInt16 nItemPos )
{
    m_nItemPos = nItemPos;
}

// -----------------------------------------------------------------------------

void OAccessibleMenuItemComponent::SetAccessibleName( const ::rtl::OUString& sAccessibleName )
{
    if ( !m_sAccessibleName.equals( sAccessibleName ) )
    {
        Any aOldValue, aNewValue;
        aOldValue <<= m_sAccessibleName;
        aNewValue <<= sAccessibleName;
        m_sAccessibleName = sAccessibleName;
        NotifyAccessibleEvent( AccessibleEventId::NAME_CHANGED, aOldValue, aNewValue );
    }
}

// -----------------------------------------------------------------------------

::rtl::OUString OAccessibleMenuItemComponent::GetAccessibleName()
{
    ::rtl::OUString sName;
    if ( m_pParent )
    {
        sal_uInt16 nItemId = m_pParent->GetItemId( m_nItemPos );
        sName = m_pParent->GetAccessibleName( nItemId );
        if ( sName.getLength() == 0 )
            sName = m_pParent->GetItemText( nItemId );
        sName = OutputDevice::GetNonMnemonicString( sName );
    }

    return sName;
}

// -----------------------------------------------------------------------------

void OAccessibleMenuItemComponent::SetItemText( const ::rtl::OUString& sItemText )
{
    Any aOldValue, aNewValue;
    if ( OCommonAccessibleText::implInitTextChangedEvent( m_sItemText, sItemText, aOldValue, aNewValue ) )
    {
        m_sItemText = sItemText;
        NotifyAccessibleEvent( AccessibleEventId::TEXT_CHANGED, aOldValue, aNewValue );
    }
}

// -----------------------------------------------------------------------------

::rtl::OUString OAccessibleMenuItemComponent::GetItemText()
{
    ::rtl::OUString sText;
    if ( m_pParent )
        sText = OutputDevice::GetNonMnemonicString( m_pParent->GetItemText( m_pParent->GetItemId( m_nItemPos ) ) );

    return sText;
}

// -----------------------------------------------------------------------------

void OAccessibleMenuItemComponent::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    if ( IsEnabled() )
    {
        rStateSet.AddState( AccessibleStateType::ENABLED );
        rStateSet.AddState( AccessibleStateType::SENSITIVE );
    }

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

awt::Rectangle OAccessibleMenuItemComponent::implGetBounds() throw (RuntimeException)
{
    awt::Rectangle aBounds( 0, 0, 0, 0 );

    if ( m_pParent )
    {
        // get bounding rectangle of the item relative to the containing window
        aBounds = AWTRectangle( m_pParent->GetBoundingRectangle( m_nItemPos ) );

        // get position of containing window in screen coordinates
        Window* pWindow = m_pParent->GetWindow();
        if ( pWindow )
        {
            Rectangle aRect = pWindow->GetWindowExtentsRelative( NULL );
            awt::Point aWindowScreenLoc = AWTPoint( aRect.TopLeft() );

            // get position of accessible parent in screen coordinates
            Reference< XAccessible > xParent = getAccessibleParent();
            if ( xParent.is() )
            {
                Reference< XAccessibleComponent > xParentComponent( xParent->getAccessibleContext(), UNO_QUERY );
                if ( xParentComponent.is() )
                {
                    awt::Point aParentScreenLoc = xParentComponent->getLocationOnScreen();

                    // calculate bounding rectangle of the item relative to the accessible parent
                    aBounds.X += aWindowScreenLoc.X - aParentScreenLoc.X;
                    aBounds.Y += aWindowScreenLoc.Y - aParentScreenLoc.Y;
                }
            }
        }
    }

    return aBounds;
}

// -----------------------------------------------------------------------------
// XComponent
// -----------------------------------------------------------------------------

void SAL_CALL OAccessibleMenuItemComponent::disposing()
{
    OAccessibleMenuBaseComponent::disposing();

    m_pParent = NULL;
    m_sAccessibleName = ::rtl::OUString();
    m_sItemText = ::rtl::OUString();
}

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

sal_Int32 OAccessibleMenuItemComponent::getAccessibleChildCount() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return 0;
}

// -----------------------------------------------------------------------------

Reference< XAccessible > OAccessibleMenuItemComponent::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    return Reference< XAccessible >();
}

// -----------------------------------------------------------------------------

Reference< XAccessible > OAccessibleMenuItemComponent::getAccessibleParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return m_pParent->GetAccessible();
}

// -----------------------------------------------------------------------------

sal_Int32 OAccessibleMenuItemComponent::getAccessibleIndexInParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return m_nItemPos;
}

// -----------------------------------------------------------------------------

sal_Int16 OAccessibleMenuItemComponent::getAccessibleRole(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::UNKNOWN;
}

// -----------------------------------------------------------------------------

::rtl::OUString OAccessibleMenuItemComponent::getAccessibleDescription( ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    ::rtl::OUString sDescription;
    if ( m_pParent )
        sDescription = m_pParent->GetHelpText( m_pParent->GetItemId( m_nItemPos ) );

    return sDescription;
}

// -----------------------------------------------------------------------------

::rtl::OUString OAccessibleMenuItemComponent::getAccessibleName(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return m_sAccessibleName;
}

// -----------------------------------------------------------------------------

Reference< XAccessibleRelationSet > OAccessibleMenuItemComponent::getAccessibleRelationSet(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
    return xSet;
}

// -----------------------------------------------------------------------------

Locale OAccessibleMenuItemComponent::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetLocale();
}

// -----------------------------------------------------------------------------
// XAccessibleComponent
// -----------------------------------------------------------------------------

Reference< XAccessible > OAccessibleMenuItemComponent::getAccessibleAtPoint( const awt::Point& ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Reference< XAccessible >();
}

// -----------------------------------------------------------------------------

void OAccessibleMenuItemComponent::grabFocus(  ) throw (RuntimeException)
{
    // no focus for items
}

// -----------------------------------------------------------------------------

sal_Int32 OAccessibleMenuItemComponent::getForeground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    Reference< XAccessible > xParent = getAccessibleParent();
    if ( xParent.is() )
    {
        Reference< XAccessibleComponent > xParentComp( xParent->getAccessibleContext(), UNO_QUERY );
        if ( xParentComp.is() )
            nColor = xParentComp->getForeground();
    }

    return nColor;
}

// -----------------------------------------------------------------------------

sal_Int32 OAccessibleMenuItemComponent::getBackground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    Reference< XAccessible > xParent = getAccessibleParent();
    if ( xParent.is() )
    {
        Reference< XAccessibleComponent > xParentComp( xParent->getAccessibleContext(), UNO_QUERY );
        if ( xParentComp.is() )
            nColor = xParentComp->getBackground();
    }

    return nColor;
}

// -----------------------------------------------------------------------------
// XAccessibleExtendedComponent
// -----------------------------------------------------------------------------

Reference< awt::XFont > OAccessibleMenuItemComponent::getFont(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Reference< awt::XFont > xFont;
    Reference< XAccessible > xParent = getAccessibleParent();
    if ( xParent.is() )
    {
        Reference< XAccessibleExtendedComponent > xParentComp( xParent->getAccessibleContext(), UNO_QUERY );
        if ( xParentComp.is() )
            xFont = xParentComp->getFont();
    }

    return xFont;
}

// -----------------------------------------------------------------------------

::rtl::OUString OAccessibleMenuItemComponent::getTitledBorderText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return ::rtl::OUString();
}

// -----------------------------------------------------------------------------

::rtl::OUString OAccessibleMenuItemComponent::getToolTipText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    ::rtl::OUString sRet;
    if ( m_pParent )
        sRet = m_pParent->GetTipHelpText( m_pParent->GetItemId( m_nItemPos ) );

    return sRet;
}

// -----------------------------------------------------------------------------
