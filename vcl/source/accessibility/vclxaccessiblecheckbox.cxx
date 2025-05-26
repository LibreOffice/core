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

#include <accessibility/vclxaccessiblecheckbox.hxx>

#include <svdata.hxx>
#include <strings.hrc>

#include <comphelper/accessiblecontexthelper.hxx>
#include <comphelper/accessiblekeybindinghelper.hxx>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>

#include <vcl/event.hxx>
#include <vcl/vclevent.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


// VCLXAccessibleCheckBox


VCLXAccessibleCheckBox::VCLXAccessibleCheckBox(CheckBox* pCheckBox)
    :ImplInheritanceHelper(pCheckBox)
{
    m_bChecked = IsChecked();
    m_bIndeterminate = IsIndeterminate();
}


bool VCLXAccessibleCheckBox::IsChecked() const
{
    VclPtr<CheckBox> pCheckBox = GetAs<CheckBox>();
    return pCheckBox && pCheckBox->IsChecked();
}


bool VCLXAccessibleCheckBox::IsIndeterminate() const
{
    VclPtr<CheckBox> pCheckBox = GetAs<CheckBox>();
    return pCheckBox && pCheckBox->GetState() == TRISTATE_INDET;
}


void VCLXAccessibleCheckBox::SetChecked( bool bChecked )
{
    if ( m_bChecked != bChecked )
    {
        Any aOldValue, aNewValue;
        if ( m_bChecked )
            aOldValue <<= AccessibleStateType::CHECKED;
        else
            aNewValue <<= AccessibleStateType::CHECKED;
        m_bChecked = bChecked;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    }
}


void VCLXAccessibleCheckBox::SetIndeterminate( bool bIndeterminate )
{
    if ( m_bIndeterminate != bIndeterminate )
    {
        Any aOldValue, aNewValue;
        if ( m_bIndeterminate )
            aOldValue <<= AccessibleStateType::INDETERMINATE;
        else
            aNewValue <<= AccessibleStateType::INDETERMINATE;
        m_bIndeterminate = bIndeterminate;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    }
}


void VCLXAccessibleCheckBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::CheckboxToggle:
        {
            const sal_Int32 nOldValue = m_bIndeterminate ? 2 : (m_bChecked ? 1 : 0);

            SetChecked( IsChecked() );
            SetIndeterminate( IsIndeterminate() );

            const sal_Int32 nNewValue = m_bIndeterminate ? 2 : (m_bChecked ? 1 : 0);
            if (nOldValue != nNewValue)
                NotifyAccessibleEvent(AccessibleEventId::VALUE_CHANGED, Any(nOldValue), Any(nNewValue));
        }
        break;
        default:
            VCLXAccessibleTextComponent::ProcessWindowEvent( rVclWindowEvent );
   }
}


void VCLXAccessibleCheckBox::FillAccessibleStateSet( sal_Int64& rStateSet )
{
    VCLXAccessibleTextComponent::FillAccessibleStateSet( rStateSet );

    rStateSet |= AccessibleStateType::CHECKABLE;
    rStateSet |= AccessibleStateType::FOCUSABLE;

    if ( IsChecked() )
        rStateSet |= AccessibleStateType::CHECKED;

    if ( IsIndeterminate() )
        rStateSet |= AccessibleStateType::INDETERMINATE;
}


// XServiceInfo


OUString VCLXAccessibleCheckBox::getImplementationName()
{
    return u"com.sun.star.comp.toolkit.AccessibleCheckBox"_ustr;
}


Sequence< OUString > VCLXAccessibleCheckBox::getSupportedServiceNames()
{
    return { u"com.sun.star.awt.AccessibleCheckBox"_ustr };
}

// XAccessibleAction


sal_Int32 VCLXAccessibleCheckBox::getAccessibleActionCount( )
{
    OExternalLockGuard aGuard( this );

    return 1;
}


sal_Bool VCLXAccessibleCheckBox::doAccessibleAction ( sal_Int32 nIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nIndex != 0 )
        throw IndexOutOfBoundsException();

    VclPtr<CheckBox> pCheckBox = GetAs<CheckBox>();
    if (pCheckBox)
    {
        sal_Int32 nValueMax = sal_Int32(1);

        if ( pCheckBox->IsTriStateEnabled() )
            nValueMax = sal_Int32(2);

        sal_Int32 nValue = static_cast<sal_Int32>(pCheckBox->GetState());

        ++nValue;

        if ( nValue > nValueMax )
            nValue = 0;

        pCheckBox->SetState(static_cast<TriState>(nValue));
    }

    return true;
}


OUString VCLXAccessibleCheckBox::getAccessibleActionDescription ( sal_Int32 nIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nIndex != 0 )
        throw IndexOutOfBoundsException();

    if(IsChecked())
        return VclResId( RID_STR_ACC_ACTION_UNCHECK );
    else
        return VclResId( RID_STR_ACC_ACTION_CHECK );
}


Reference< XAccessibleKeyBinding > VCLXAccessibleCheckBox::getAccessibleActionKeyBinding( sal_Int32 nIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nIndex != 0 )
        throw IndexOutOfBoundsException();

    rtl::Reference<OAccessibleKeyBindingHelper> pKeyBindingHelper = new OAccessibleKeyBindingHelper();

    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
    {
        KeyEvent aKeyEvent = pWindow->GetActivationKey();
        vcl::KeyCode aKeyCode = aKeyEvent.GetKeyCode();
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

    return pKeyBindingHelper;
}


// XAccessibleValue


Any VCLXAccessibleCheckBox::getCurrentValue(  )
{
    OExternalLockGuard aGuard( this );

    Any aValue;

    VclPtr<CheckBox> pCheckBox = GetAs<CheckBox>();
    if (pCheckBox)
        aValue <<= static_cast<sal_Int32>(pCheckBox->GetState());

    return aValue;
}


sal_Bool VCLXAccessibleCheckBox::setCurrentValue( const Any& aNumber )
{
    OExternalLockGuard aGuard( this );

    bool bReturn = false;

    VclPtr<CheckBox> pCheckBox = GetAs<CheckBox>();
    if (pCheckBox)
    {
        sal_Int32 nValue = 0, nValueMin = 0, nValueMax = 0;
        OSL_VERIFY( aNumber >>= nValue );
        nValueMax=implGetMaximumValue();

        if ( nValue < nValueMin )
            nValue = nValueMin;
        else if ( nValue > nValueMax )
            nValue = nValueMax;

        pCheckBox->SetState(static_cast<TriState>(nValue));
        bReturn = true;
    }

    return bReturn;
}


Any VCLXAccessibleCheckBox::getMaximumValue(  )
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    aValue <<= implGetMaximumValue();

    return aValue;
}

sal_Int32 VCLXAccessibleCheckBox::implGetMaximumValue(  )
{
    VclPtr< CheckBox > pCheckBox = GetAs< CheckBox >();
    if ( pCheckBox && pCheckBox->IsTriStateEnabled() )
        return 2;

    return 1;
}

Any VCLXAccessibleCheckBox::getMinimumValue(  )
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    aValue <<= sal_Int32(0);

    return aValue;
}

Any VCLXAccessibleCheckBox::getMinimumIncrement( )
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    aValue <<= sal_Int32(1);

    return aValue;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
