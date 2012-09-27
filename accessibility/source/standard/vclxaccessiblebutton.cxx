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

#include <accessibility/standard/vclxaccessiblebutton.hxx>
#include <accessibility/helper/accresmgr.hxx>
#include <accessibility/helper/accessiblestrings.hrc>

#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/accessiblekeybindinghelper.hxx>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>

#include <vcl/button.hxx>

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

OUString VCLXAccessibleButton::getImplementationName() throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleButton" );
}

// -----------------------------------------------------------------------------

Sequence< OUString > VCLXAccessibleButton::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< OUString > aNames(1);
    aNames[0] = "com.sun.star.awt.AccessibleButton";
    return aNames;
}

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

OUString VCLXAccessibleButton::getAccessibleName(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    OUString aName( VCLXAccessibleTextComponent::getAccessibleName() );
    sal_Int32 nLength = aName.getLength();

    if ( nLength >= 3 && aName.matchAsciiL( RTL_CONSTASCII_STRINGPARAM("..."), nLength - 3 ) )
    {
        if ( nLength == 3 )
        {
            // it's a browse button
            aName = OUString( TK_RES_STRING( RID_STR_ACC_NAME_BROWSEBUTTON ) );
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

OUString VCLXAccessibleButton::getAccessibleActionDescription ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return OUString( TK_RES_STRING( RID_STR_ACC_ACTION_CLICK ) );
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
            if ( aKeyCode.IsMod3() )
                aKeyStroke.Modifiers |= awt::KeyModifier::MOD3;
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

        pButton->SetPressed( (sal_Bool) nValue );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
