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

// includes --------------------------------------------------------------
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

        // IA2 CWS: If the button has a poppup menu,it should has the state EXPANDABLE
        if( pButton->GetType() == WINDOW_MENUBUTTON )
        {
            rStateSet.AddState( AccessibleStateType::EXPANDABLE );
        }
        if( pButton->GetStyle() & WB_DEFBUTTON )
        {
            rStateSet.AddState( AccessibleStateType::DEFAULT );
        }
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

    // IA2 CWS: Removed special handling for browse/more buttons.
    //          Comment was "the '...' or '<<' or '>>' should be kepted per the requirements from AT"
    // MT: We did introduce this special handling by intention.
    //     As the original text is still what you get via XAccessibleText,
    //     I think for the accessible name the stuff below is correct.

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
