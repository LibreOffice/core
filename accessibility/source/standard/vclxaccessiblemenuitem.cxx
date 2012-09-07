/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <accessibility/standard/vclxaccessiblemenuitem.hxx>
#include <accessibility/helper/accresmgr.hxx>
#include <accessibility/helper/accessiblestrings.hrc>
#include <toolkit/helper/convert.hxx>
#include <accessibility/helper/characterattributeshelper.hxx>
#include <comphelper/accessiblekeybindinghelper.hxx>
#include <com/sun/star/awt/KeyModifier.hpp>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/menu.hxx>
#include <vcl/unohelp2.hxx>

#include <memory>


using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;
using namespace ::comphelper;


// -----------------------------------------------------------------------------
// class VCLXAccessibleMenuItem
// -----------------------------------------------------------------------------

VCLXAccessibleMenuItem::VCLXAccessibleMenuItem( Menu* pParent, sal_uInt16 nItemPos, Menu* pMenu )
    :OAccessibleMenuItemComponent( pParent, nItemPos, pMenu )
{
}

// -----------------------------------------------------------------------------

VCLXAccessibleMenuItem::~VCLXAccessibleMenuItem()
{
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleMenuItem::IsFocused()
{
    return IsHighlighted();
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleMenuItem::IsSelected()
{
    return IsHighlighted();
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleMenuItem::IsChecked()
{
    sal_Bool bChecked = sal_False;

    if ( m_pParent )
    {
        sal_uInt16 nItemId = m_pParent->GetItemId( m_nItemPos );
        if ( m_pParent->IsItemChecked( nItemId ) )
            bChecked = sal_True;
    }

    return bChecked;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleMenuItem::IsHighlighted()
{
    sal_Bool bHighlighted = sal_False;

    if ( m_pParent && m_pParent->IsHighlighted( m_nItemPos ) )
        bHighlighted = sal_True;

    return bHighlighted;
}

// -----------------------------------------------------------------------------

void VCLXAccessibleMenuItem::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    OAccessibleMenuItemComponent::FillAccessibleStateSet( rStateSet );

    rStateSet.AddState( AccessibleStateType::FOCUSABLE );

    if ( IsFocused() )
        rStateSet.AddState( AccessibleStateType::FOCUSED );

    rStateSet.AddState( AccessibleStateType::SELECTABLE );

    if ( IsSelected() )
        rStateSet.AddState( AccessibleStateType::SELECTED );

    if ( IsChecked() )
        rStateSet.AddState( AccessibleStateType::CHECKED );
}

// -----------------------------------------------------------------------------
// OCommonAccessibleText
// -----------------------------------------------------------------------------

OUString VCLXAccessibleMenuItem::implGetText()
{
    return m_sItemText;
}

// -----------------------------------------------------------------------------

Locale VCLXAccessibleMenuItem::implGetLocale()
{
    return Application::GetSettings().GetLocale();
}

// -----------------------------------------------------------------------------

void VCLXAccessibleMenuItem::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
{
    nStartIndex = 0;
    nEndIndex = 0;
}

// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleMenuItem, OAccessibleMenuItemComponent, VCLXAccessibleMenuItem_BASE )

// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleMenuItem, OAccessibleMenuItemComponent, VCLXAccessibleMenuItem_BASE )

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

OUString VCLXAccessibleMenuItem::getImplementationName() throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleMenuItem" );
}

// -----------------------------------------------------------------------------

Sequence< OUString > VCLXAccessibleMenuItem::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< OUString > aNames(1);
    aNames[0] = "com.sun.star.awt.AccessibleMenuItem";
    return aNames;
}

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

sal_Int16 VCLXAccessibleMenuItem::getAccessibleRole(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::MENU_ITEM;
}

// -----------------------------------------------------------------------------
// XAccessibleText
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleMenuItem::getCaretPosition() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return -1;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleMenuItem::setCaretPosition( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{

    OExternalLockGuard aGuard( this );

    if ( !implIsValidRange( nIndex, nIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    return sal_False;
}

// -----------------------------------------------------------------------------

sal_Unicode VCLXAccessibleMenuItem::getCharacter( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getCharacter( nIndex );
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > VCLXAccessibleMenuItem::getCharacterAttributes( sal_Int32 nIndex, const Sequence< OUString >& aRequestedAttributes ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Sequence< PropertyValue > aValues;
    OUString sText( implGetText() );

    if ( !implIsValidIndex( nIndex, sText.getLength() ) )
        throw IndexOutOfBoundsException();

    Font aFont = Application::GetSettings().GetStyleSettings().GetMenuFont();
    sal_Int32 nBackColor = getBackground();
    sal_Int32 nColor = getForeground();
    ::std::auto_ptr< CharacterAttributesHelper > pHelper( new CharacterAttributesHelper( aFont, nBackColor, nColor ) );
    aValues = pHelper->GetCharacterAttributes( aRequestedAttributes );

    return aValues;
}

// -----------------------------------------------------------------------------

awt::Rectangle VCLXAccessibleMenuItem::getCharacterBounds( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidIndex( nIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    awt::Rectangle aBounds( 0, 0, 0, 0 );
    if ( m_pParent )
    {
        sal_uInt16 nItemId = m_pParent->GetItemId( m_nItemPos );
        Rectangle aItemRect = m_pParent->GetBoundingRectangle( m_nItemPos );
        Rectangle aCharRect = m_pParent->GetCharacterBounds( nItemId, nIndex );
        aCharRect.Move( -aItemRect.Left(), -aItemRect.Top() );
        aBounds = AWTRectangle( aCharRect );
    }

    return aBounds;
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleMenuItem::getCharacterCount() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getCharacterCount();
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleMenuItem::getIndexAtPoint( const awt::Point& aPoint ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nIndex = -1;
    if ( m_pParent )
    {
        sal_uInt16 nItemId = 0;
        Rectangle aItemRect = m_pParent->GetBoundingRectangle( m_nItemPos );
        Point aPnt( VCLPoint( aPoint ) );
        aPnt += aItemRect.TopLeft();
        sal_Int32 nI = m_pParent->GetIndexForPoint( aPnt, nItemId );
        if ( nI != -1 && m_pParent->GetItemId( m_nItemPos ) == nItemId )
            nIndex = nI;
    }

    return nIndex;
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleMenuItem::getSelectedText() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getSelectedText();
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleMenuItem::getSelectionStart() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getSelectionStart();
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleMenuItem::getSelectionEnd() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getSelectionEnd();
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleMenuItem::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    return sal_False;
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleMenuItem::getText() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getText();
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleMenuItem::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getTextRange( nStartIndex, nEndIndex );
}

// -----------------------------------------------------------------------------

::com::sun::star::accessibility::TextSegment VCLXAccessibleMenuItem::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getTextAtIndex( nIndex, aTextType );
}

// -----------------------------------------------------------------------------

::com::sun::star::accessibility::TextSegment VCLXAccessibleMenuItem::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getTextBeforeIndex( nIndex, aTextType );
}

// -----------------------------------------------------------------------------

::com::sun::star::accessibility::TextSegment VCLXAccessibleMenuItem::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getTextBehindIndex( nIndex, aTextType );
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleMenuItem::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Bool bReturn = sal_False;

    if ( m_pParent )
    {
        Window* pWindow = m_pParent->GetWindow();
        if ( pWindow )
        {
            Reference< datatransfer::clipboard::XClipboard > xClipboard = pWindow->GetClipboard();
            if ( xClipboard.is() )
            {
                OUString sText( getTextRange( nStartIndex, nEndIndex ) );

                ::vcl::unohelper::TextDataObject* pDataObj = new ::vcl::unohelper::TextDataObject( sText );
                const sal_uInt32 nRef = Application::ReleaseSolarMutex();
                xClipboard->setContents( pDataObj, NULL );

                Reference< datatransfer::clipboard::XFlushableClipboard > xFlushableClipboard( xClipboard, uno::UNO_QUERY );
                if( xFlushableClipboard.is() )
                    xFlushableClipboard->flushClipboard();

                Application::AcquireSolarMutex( nRef );

                bReturn = sal_True;
            }
        }
    }

    return bReturn;
}

// -----------------------------------------------------------------------------
// XAccessibleAction
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleMenuItem::getAccessibleActionCount( ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return 1;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleMenuItem::doAccessibleAction ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    Click();

    return sal_True;
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleMenuItem::getAccessibleActionDescription ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return OUString( TK_RES_STRING( RID_STR_ACC_ACTION_CLICK ) );
}

// -----------------------------------------------------------------------------

Reference< XAccessibleKeyBinding > VCLXAccessibleMenuItem::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    OAccessibleKeyBindingHelper* pKeyBindingHelper = new OAccessibleKeyBindingHelper();
    Reference< XAccessibleKeyBinding > xKeyBinding = pKeyBindingHelper;

    if ( m_pParent )
    {
        // create auto mnemonics
        if ( Application::GetSettings().GetStyleSettings().GetAutoMnemonic() && !( m_pParent->GetMenuFlags() & MENU_FLAG_NOAUTOMNEMONICS ) )
            m_pParent->CreateAutoMnemonics();

        // activation key
        KeyEvent aKeyEvent = m_pParent->GetActivationKey( m_pParent->GetItemId( m_nItemPos ) );
        KeyCode aKeyCode = aKeyEvent.GetKeyCode();
        Sequence< awt::KeyStroke > aSeq1(1);
        aSeq1[0].Modifiers = 0;
        Reference< XAccessible > xParent( getAccessibleParent() );
        if ( xParent.is() )
        {
            Reference< XAccessibleContext > xParentContext( xParent->getAccessibleContext() );
            if ( xParentContext.is() && xParentContext->getAccessibleRole() == AccessibleRole::MENU_BAR )
                aSeq1[0].Modifiers |= awt::KeyModifier::MOD2;
        }
        aSeq1[0].KeyCode = aKeyCode.GetCode();
        aSeq1[0].KeyChar = aKeyEvent.GetCharCode();
        aSeq1[0].KeyFunc = static_cast< sal_Int16 >( aKeyCode.GetFunction() );
        pKeyBindingHelper->AddKeyBinding( aSeq1 );

        // complete menu activation key sequence
        Sequence< awt::KeyStroke > aSeq;
        if ( xParent.is() )
        {
            Reference< XAccessibleContext > xParentContext( xParent->getAccessibleContext() );
            if ( xParentContext.is() && xParentContext->getAccessibleRole() == AccessibleRole::MENU )
            {
                Reference< XAccessibleAction > xAction( xParentContext, UNO_QUERY );
                if ( xAction.is() && xAction->getAccessibleActionCount() > 0 )
                {
                    Reference< XAccessibleKeyBinding > xKeyB( xAction->getAccessibleActionKeyBinding( 0 ) );
                    if ( xKeyB.is() && xKeyB->getAccessibleKeyBindingCount() > 1 )
                        aSeq = xKeyB->getAccessibleKeyBinding( 1 );
                }
            }
        }
        Sequence< awt::KeyStroke > aSeq2 = ::comphelper::concatSequences( aSeq, aSeq1 );
        pKeyBindingHelper->AddKeyBinding( aSeq2 );

        // accelerator key
        KeyCode aAccelKeyCode = m_pParent->GetAccelKey( m_pParent->GetItemId( m_nItemPos ) );
        if ( aAccelKeyCode.GetCode() != 0 )
        {
            Sequence< awt::KeyStroke > aSeq3(1);
            aSeq3[0].Modifiers = 0;
            if ( aAccelKeyCode.IsShift() )
                aSeq3[0].Modifiers |= awt::KeyModifier::SHIFT;
            if ( aAccelKeyCode.IsMod1() )
                aSeq3[0].Modifiers |= awt::KeyModifier::MOD1;
            if ( aAccelKeyCode.IsMod2() )
                aSeq3[0].Modifiers |= awt::KeyModifier::MOD2;
            if ( aAccelKeyCode.IsMod3() )
                aSeq3[0].Modifiers |= awt::KeyModifier::MOD3;
            aSeq3[0].KeyCode = aAccelKeyCode.GetCode();
            aSeq3[0].KeyFunc = static_cast< sal_Int16 >( aAccelKeyCode.GetFunction() );
            pKeyBindingHelper->AddKeyBinding( aSeq3 );
        }
    }

    return xKeyBinding;
}

// -----------------------------------------------------------------------------
// XAccessibleValue
// -----------------------------------------------------------------------------

Any VCLXAccessibleMenuItem::getCurrentValue(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    if ( IsSelected() )
        aValue <<= (sal_Int32) 1;
    else
        aValue <<= (sal_Int32) 0;

    return aValue;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleMenuItem::setCurrentValue( const Any& aNumber ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Bool bReturn = sal_False;
    sal_Int32 nValue = 0;
    OSL_VERIFY( aNumber >>= nValue );

    if ( nValue <= 0 )
    {
        DeSelect();
        bReturn = sal_True;
    }
    else if ( nValue >= 1 )
    {
        Select();
        bReturn = sal_True;
    }

    return bReturn;
}

// -----------------------------------------------------------------------------

Any VCLXAccessibleMenuItem::getMaximumValue(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    aValue <<= (sal_Int32) 1;

    return aValue;
}

// -----------------------------------------------------------------------------

Any VCLXAccessibleMenuItem::getMinimumValue(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    aValue <<= (sal_Int32) 0;

    return aValue;
}

// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
