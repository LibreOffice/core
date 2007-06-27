/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessiblecheckbox.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:37:19 $
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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLECHECKBOX_HXX
#include <accessibility/standard/vclxaccessiblecheckbox.hxx>
#endif

#ifndef _TOOLKIT_AWT_VCLXWINDOWS_HXX
#include <toolkit/awt/vclxwindows.hxx>
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
// VCLXAccessibleCheckBox
// -----------------------------------------------------------------------------

VCLXAccessibleCheckBox::VCLXAccessibleCheckBox( VCLXWindow* pVCLWindow )
    :VCLXAccessibleTextComponent( pVCLWindow )
{
    m_bChecked = IsChecked();
    m_bIndeterminate = IsIndeterminate();
}

// -----------------------------------------------------------------------------

VCLXAccessibleCheckBox::~VCLXAccessibleCheckBox()
{
}

// -----------------------------------------------------------------------------

bool VCLXAccessibleCheckBox::IsChecked()
{
    bool bChecked = false;

    VCLXCheckBox* pVCLXCheckBox = static_cast< VCLXCheckBox* >( GetVCLXWindow() );
    if ( pVCLXCheckBox && pVCLXCheckBox->getState() == (sal_Int16) 1 )
        bChecked = true;

    return bChecked;
}

// -----------------------------------------------------------------------------

bool VCLXAccessibleCheckBox::IsIndeterminate()
{
    bool bIndeterminate = false;

    VCLXCheckBox* pVCLXCheckBox = static_cast< VCLXCheckBox* >( GetVCLXWindow() );
    if ( pVCLXCheckBox && pVCLXCheckBox->getState() == (sal_Int16) 2 )
        bIndeterminate = true;

    return bIndeterminate;
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------

void VCLXAccessibleCheckBox::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_CHECKBOX_TOGGLE:
        {
            SetChecked( IsChecked() );
            SetIndeterminate( IsIndeterminate() );
        }
        break;
        default:
            VCLXAccessibleTextComponent::ProcessWindowEvent( rVclWindowEvent );
   }
}

// -----------------------------------------------------------------------------

void VCLXAccessibleCheckBox::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleTextComponent::FillAccessibleStateSet( rStateSet );

    rStateSet.AddState( AccessibleStateType::FOCUSABLE );

    if ( IsChecked() )
        rStateSet.AddState( AccessibleStateType::CHECKED );

    if ( IsIndeterminate() )
        rStateSet.AddState( AccessibleStateType::INDETERMINATE );
}

// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleCheckBox, VCLXAccessibleTextComponent, VCLXAccessibleCheckBox_BASE )

// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleCheckBox, VCLXAccessibleTextComponent, VCLXAccessibleCheckBox_BASE )

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleCheckBox::getImplementationName() throw (RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.toolkit.AccessibleCheckBox" );
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > VCLXAccessibleCheckBox::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii( "com.sun.star.awt.AccessibleCheckBox" );
    return aNames;
}

// -----------------------------------------------------------------------------
// XAccessibleAction
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleCheckBox::getAccessibleActionCount( ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return 1;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleCheckBox::doAccessibleAction ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    CheckBox* pCheckBox = (CheckBox*) GetWindow();
    VCLXCheckBox* pVCLXCheckBox = static_cast< VCLXCheckBox* >( GetVCLXWindow() );
    if ( pCheckBox && pVCLXCheckBox )
    {
        sal_Int32 nValueMin = (sal_Int32) 0;
        sal_Int32 nValueMax = (sal_Int32) 1;

        if ( pCheckBox->IsTriStateEnabled() )
            nValueMax = (sal_Int32) 2;

        sal_Int32 nValue = (sal_Int32) pVCLXCheckBox->getState();

        ++nValue;

        if ( nValue > nValueMax )
            nValue = nValueMin;

        pVCLXCheckBox->setState( (sal_Int16) nValue );
    }

    return sal_True;
}

// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleCheckBox::getAccessibleActionDescription ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return ::rtl::OUString( TK_RES_STRING( RID_STR_ACC_ACTION_CLICK ) );
}

// -----------------------------------------------------------------------------

Reference< XAccessibleKeyBinding > VCLXAccessibleCheckBox::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
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

Any VCLXAccessibleCheckBox::getCurrentValue(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Any aValue;

    VCLXCheckBox* pVCLXCheckBox = static_cast< VCLXCheckBox* >( GetVCLXWindow() );
    if ( pVCLXCheckBox )
        aValue <<= (sal_Int32) pVCLXCheckBox->getState();

    return aValue;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleCheckBox::setCurrentValue( const Any& aNumber ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Bool bReturn = sal_False;

    VCLXCheckBox* pVCLXCheckBox = static_cast< VCLXCheckBox* >( GetVCLXWindow() );
    if ( pVCLXCheckBox )
    {
        sal_Int32 nValue = 0, nValueMin = 0, nValueMax = 0;
        OSL_VERIFY( aNumber >>= nValue );
        OSL_VERIFY( getMinimumValue() >>= nValueMin );
        OSL_VERIFY( getMaximumValue() >>= nValueMax );

        if ( nValue < nValueMin )
            nValue = nValueMin;
        else if ( nValue > nValueMax )
            nValue = nValueMax;

        pVCLXCheckBox->setState( (sal_Int16) nValue );
        bReturn = sal_True;
    }

    return bReturn;
}

// -----------------------------------------------------------------------------

Any VCLXAccessibleCheckBox::getMaximumValue(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Any aValue;

    CheckBox* pCheckBox = (CheckBox*) GetWindow();
    if ( pCheckBox && pCheckBox->IsTriStateEnabled() )
        aValue <<= (sal_Int32) 2;
    else
        aValue <<= (sal_Int32) 1;

    return aValue;
}

// -----------------------------------------------------------------------------

Any VCLXAccessibleCheckBox::getMinimumValue(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    aValue <<= (sal_Int32) 0;

    return aValue;
}

// -----------------------------------------------------------------------------
