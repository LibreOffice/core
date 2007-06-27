/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessiblebutton.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:37:06 $
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

// includes --------------------------------------------------------------

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEBUTTON_HXX
#include <accessibility/standard/vclxaccessiblebutton.hxx>
#endif

#ifndef ACCESSIBILITY_HELPER_TKARESMGR_HXX
#include <accessibility/helper/accresmgr.hxx>
#endif
#ifndef ACCESSIBILITY_HELPER_ACCESSIBLESTRINGS_HRC_
#include <accessibility/helper/accessiblestrings.hrc>
#endif

#ifndef _UTL_ACCESSIBLESTATESETHELPER_HXX
#include <unotools/accessiblestatesethelper.hxx>
#endif
#ifndef COMPHELPER_ACCESSIBLE_KEYBINDING_HELPER_HXX
#include <comphelper/accessiblekeybindinghelper.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_KEYMODIFIER_HPP_
#include <com/sun/star/awt/KeyModifier.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX
#include <comphelper/sequence.hxx>
#endif

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


// -----------------------------------------------------------------------------
// VCLXAccessibleButton
// -----------------------------------------------------------------------------

VCLXAccessibleButton::VCLXAccessibleButton( VCLXWindow* pVCLWindow )
    :VCLXAccessibleTextComponent( pVCLWindow )
{
}

// -----------------------------------------------------------------------------

VCLXAccessibleButton::~VCLXAccessibleButton()
{
}

// -----------------------------------------------------------------------------

void VCLXAccessibleButton::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_PUSHBUTTON_TOGGLE:
        {
            Any aOldValue;
            Any aNewValue;

            PushButton* pButton = (PushButton*) GetWindow();
            if ( pButton && pButton->GetState() == STATE_CHECK )
                aNewValue <<= AccessibleStateType::CHECKED;
            else
                aOldValue <<= AccessibleStateType::CHECKED;

            NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
        break;
        default:
            VCLXAccessibleTextComponent::ProcessWindowEvent( rVclWindowEvent );
   }
}

// -----------------------------------------------------------------------------

void VCLXAccessibleButton::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleTextComponent::FillAccessibleStateSet( rStateSet );

    PushButton* pButton = (PushButton*) GetWindow();
    if ( pButton )
    {
        rStateSet.AddState( AccessibleStateType::FOCUSABLE );

        if ( pButton->GetState() == STATE_CHECK )
            rStateSet.AddState( AccessibleStateType::CHECKED );

        if ( pButton->IsPressed() )
            rStateSet.AddState( AccessibleStateType::PRESSED );
    }
}

// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleButton, VCLXAccessibleTextComponent, VCLXAccessibleButton_BASE )

// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleButton, VCLXAccessibleTextComponent, VCLXAccessibleButton_BASE )

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleButton::getImplementationName() throw (RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.toolkit.AccessibleButton" );
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > VCLXAccessibleButton::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii( "com.sun.star.awt.AccessibleButton" );
    return aNames;
}

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleButton::getAccessibleName(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    ::rtl::OUString aName( VCLXAccessibleTextComponent::getAccessibleName() );
    sal_Int32 nLength = aName.getLength();

    if ( nLength >= 3 && aName.matchAsciiL( RTL_CONSTASCII_STRINGPARAM("..."), nLength - 3 ) )
    {
        if ( nLength == 3 )
        {
            // it's a browse button
            aName = ::rtl::OUString( TK_RES_STRING( RID_STR_ACC_NAME_BROWSEBUTTON ) );
        }
        else
        {
            // remove the three trailing dots
            aName = aName.copy( 0, nLength - 3 );
        }
    }
    else if ( nLength >= 3 && aName.matchAsciiL( RTL_CONSTASCII_STRINGPARAM("<< "), 0 ) )
    {
        // remove the leading symbols
        aName = aName.copy( 3, nLength - 3 );
    }
    else if ( nLength >= 3 && aName.matchAsciiL( RTL_CONSTASCII_STRINGPARAM(" >>"), nLength - 3 ) )
    {
        // remove the trailing symbols
        aName = aName.copy( 0, nLength - 3 );
    }

    return aName;
}

// -----------------------------------------------------------------------------
// XAccessibleAction
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleButton::getAccessibleActionCount( ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return 1;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleButton::doAccessibleAction ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    PushButton* pButton = (PushButton*) GetWindow();
    if ( pButton )
        pButton->Click();

    return sal_True;
}

// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleButton::getAccessibleActionDescription ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return ::rtl::OUString( TK_RES_STRING( RID_STR_ACC_ACTION_CLICK ) );
}

// -----------------------------------------------------------------------------

Reference< XAccessibleKeyBinding > VCLXAccessibleButton::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    OAccessibleKeyBindingHelper* pKeyBindingHelper = new OAccessibleKeyBindingHelper();
    Reference< XAccessibleKeyBinding > xKeyBinding = pKeyBindingHelper;

    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        KeyEvent aKeyEvent = pWindow->GetActivationKey();
        KeyCode aKeyCode = aKeyEvent.GetKeyCode();
        if ( aKeyCode.GetCode() != 0 )
        {
            awt::KeyStroke aKeyStroke;
            aKeyStroke.Modifiers = 0;
            if ( aKeyCode.IsShift() )
                aKeyStroke.Modifiers |= awt::KeyModifier::SHIFT;
            if ( aKeyCode.IsMod1() )
                aKeyStroke.Modifiers |= awt::KeyModifier::MOD1;
            if ( aKeyCode.IsMod2() )
                aKeyStroke.Modifiers |= awt::KeyModifier::MOD2;
            aKeyStroke.KeyCode = aKeyCode.GetCode();
            aKeyStroke.KeyChar = aKeyEvent.GetCharCode();
            aKeyStroke.KeyFunc = static_cast< sal_Int16 >( aKeyCode.GetFunction() );
            pKeyBindingHelper->AddKeyBinding( aKeyStroke );
        }
    }

    return xKeyBinding;
}

// -----------------------------------------------------------------------------
// XAccessibleValue
// -----------------------------------------------------------------------------

Any VCLXAccessibleButton::getCurrentValue(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Any aValue;

    PushButton* pButton = (PushButton*) GetWindow();
    if ( pButton )
        aValue <<= (sal_Int32) pButton->IsPressed();

    return aValue;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleButton::setCurrentValue( const Any& aNumber ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Bool bReturn = sal_False;

    PushButton* pButton = (PushButton*) GetWindow();
    if ( pButton )
    {
        sal_Int32 nValue = 0;
        OSL_VERIFY( aNumber >>= nValue );

        if ( nValue < 0 )
            nValue = 0;
        else if ( nValue > 1 )
            nValue = 1;

        pButton->SetPressed( (BOOL) nValue );
        bReturn = sal_True;
    }

    return bReturn;
}

// -----------------------------------------------------------------------------

Any VCLXAccessibleButton::getMaximumValue(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    aValue <<= (sal_Int32) 1;

    return aValue;
}

// -----------------------------------------------------------------------------

Any VCLXAccessibleButton::getMinimumValue(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    aValue <<= (sal_Int32) 0;

    return aValue;
}

// -----------------------------------------------------------------------------
