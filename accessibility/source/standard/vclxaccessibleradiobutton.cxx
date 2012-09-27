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

#include <accessibility/standard/vclxaccessibleradiobutton.hxx>

#include <toolkit/awt/vclxwindows.hxx>
#include <accessibility/helper/accresmgr.hxx>
#include <accessibility/helper/accessiblestrings.hrc>

#include <unotools/accessiblerelationsethelper.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/accessiblekeybindinghelper.hxx>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/window.hxx>
#include <vcl/button.hxx>


using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


// -----------------------------------------------------------------------------
// VCLXAccessibleRadioButton
// -----------------------------------------------------------------------------

VCLXAccessibleRadioButton::VCLXAccessibleRadioButton( VCLXWindow* pVCLWindow )
    :VCLXAccessibleTextComponent( pVCLWindow )
{
}

// -----------------------------------------------------------------------------

VCLXAccessibleRadioButton::~VCLXAccessibleRadioButton()
{
}

// -----------------------------------------------------------------------------

void VCLXAccessibleRadioButton::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_RADIOBUTTON_TOGGLE:
        {
            Any aOldValue;
            Any aNewValue;

            VCLXRadioButton* pVCLXRadioButton = static_cast< VCLXRadioButton* >( GetVCLXWindow() );
            if ( pVCLXRadioButton && pVCLXRadioButton->getState() )
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

void VCLXAccessibleRadioButton::FillAccessibleRelationSet( utl::AccessibleRelationSetHelper& rRelationSet )
{
    VCLXAccessibleTextComponent::FillAccessibleRelationSet( rRelationSet );

    RadioButton* pRadioButton = dynamic_cast< RadioButton* >( GetWindow() );
    if ( pRadioButton )
    {
        ::std::vector< RadioButton* > aGroup(pRadioButton->GetRadioButtonGroup(true));
        if (!aGroup.empty())
        {
            sal_Int32 i = 0;
            Sequence< Reference< XInterface > > aSequence( static_cast< sal_Int32 >( aGroup.size() ) );
            ::std::vector< RadioButton* >::const_iterator aEndItr = aGroup.end();
            for ( ::std::vector< RadioButton* >::const_iterator aItr = aGroup.begin(); aItr < aEndItr; ++aItr )
            {
                aSequence[i++] = (*aItr)->GetAccessible();
            }
            rRelationSet.AddRelation( AccessibleRelation( AccessibleRelationType::MEMBER_OF, aSequence ) );
        }
    }
}

// -----------------------------------------------------------------------------

void VCLXAccessibleRadioButton::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleTextComponent::FillAccessibleStateSet( rStateSet );

    VCLXRadioButton* pVCLXRadioButton = static_cast< VCLXRadioButton* >( GetVCLXWindow() );
    if ( pVCLXRadioButton )
    {
        rStateSet.AddState( AccessibleStateType::FOCUSABLE );
        if ( pVCLXRadioButton->getState() )
            rStateSet.AddState( AccessibleStateType::CHECKED );
    }
}

// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleRadioButton, VCLXAccessibleTextComponent, VCLXAccessibleRadioButton_BASE )

// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleRadioButton, VCLXAccessibleTextComponent, VCLXAccessibleRadioButton_BASE )

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

OUString VCLXAccessibleRadioButton::getImplementationName() throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleRadioButton" );
}

// -----------------------------------------------------------------------------

Sequence< OUString > VCLXAccessibleRadioButton::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< OUString > aNames(1);
    aNames[0] = "com.sun.star.awt.AccessibleRadioButton";
    return aNames;
}

// -----------------------------------------------------------------------------
// XAccessibleAction
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleRadioButton::getAccessibleActionCount( ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return 1;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleRadioButton::doAccessibleAction ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    VCLXRadioButton* pVCLXRadioButton = static_cast< VCLXRadioButton* >( GetVCLXWindow() );
    if ( pVCLXRadioButton && !pVCLXRadioButton->getState() )
        pVCLXRadioButton->setState( sal_True );

    return sal_True;
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleRadioButton::getAccessibleActionDescription ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return OUString( TK_RES_STRING( RID_STR_ACC_ACTION_CLICK ) );
}

// -----------------------------------------------------------------------------

Reference< XAccessibleKeyBinding > VCLXAccessibleRadioButton::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
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

Any VCLXAccessibleRadioButton::getCurrentValue(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Any aValue;

    VCLXRadioButton* pVCLXRadioButton = static_cast< VCLXRadioButton* >( GetVCLXWindow() );
    if ( pVCLXRadioButton )
        aValue <<= (sal_Int32) pVCLXRadioButton->getState();

    return aValue;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleRadioButton::setCurrentValue( const Any& aNumber ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Bool bReturn = sal_False;

    VCLXRadioButton* pVCLXRadioButton = static_cast< VCLXRadioButton* >( GetVCLXWindow() );
    if ( pVCLXRadioButton )
    {
        sal_Int32 nValue = 0;
        OSL_VERIFY( aNumber >>= nValue );

        if ( nValue < 0 )
            nValue = 0;
        else if ( nValue > 1 )
            nValue = 1;

        pVCLXRadioButton->setState( (sal_Bool) nValue );
        bReturn = sal_True;
    }

    return bReturn;
}

// -----------------------------------------------------------------------------

Any VCLXAccessibleRadioButton::getMaximumValue(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    aValue <<= (sal_Int32) 1;

    return aValue;
}

// -----------------------------------------------------------------------------

Any VCLXAccessibleRadioButton::getMinimumValue(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    aValue <<= (sal_Int32) 0;

    return aValue;
}

// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
