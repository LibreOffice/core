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

#include <standard/vclxaccessiblemenuitem.hxx>
#include <helper/accresmgr.hxx>
#include <helper/accessiblestrings.hrc>
#include <toolkit/helper/convert.hxx>
#include <helper/characterattributeshelper.hxx>
#include <comphelper/accessiblekeybindinghelper.hxx>
#include <com/sun/star/awt/KeyModifier.hpp>

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <unotools/accessiblestatesethelper.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/menu.hxx>
#include <vcl/unohelp2.hxx>
#include <vcl/settings.hxx>

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;
using namespace ::comphelper;


// class VCLXAccessibleMenuItem


VCLXAccessibleMenuItem::VCLXAccessibleMenuItem( Menu* pParent, sal_uInt16 nItemPos, Menu* pMenu )
    :OAccessibleMenuItemComponent( pParent, nItemPos, pMenu )
{
}


VCLXAccessibleMenuItem::~VCLXAccessibleMenuItem()
{
}


bool VCLXAccessibleMenuItem::IsFocused()
{
    return IsHighlighted();
}


bool VCLXAccessibleMenuItem::IsSelected()
{
    return IsHighlighted();
}


bool VCLXAccessibleMenuItem::IsChecked()
{
    bool bChecked = false;

    if ( m_pParent )
    {
        sal_uInt16 nItemId = m_pParent->GetItemId( m_nItemPos );
        if ( m_pParent->IsItemChecked( nItemId ) )
            bChecked = true;
    }

    return bChecked;
}


bool VCLXAccessibleMenuItem::IsHighlighted()
{
    bool bHighlighted = false;

    if ( m_pParent && m_pParent->IsHighlighted( m_nItemPos ) )
        bHighlighted = true;

    return bHighlighted;
}


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


// OCommonAccessibleText


OUString VCLXAccessibleMenuItem::implGetText()
{
    return m_sItemText;
}


Locale VCLXAccessibleMenuItem::implGetLocale()
{
    return Application::GetSettings().GetLanguageTag().getLocale();
}


void VCLXAccessibleMenuItem::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
{
    nStartIndex = 0;
    nEndIndex = 0;
}


// XInterface


IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleMenuItem, OAccessibleMenuItemComponent, VCLXAccessibleMenuItem_BASE )


// XTypeProvider


IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleMenuItem, OAccessibleMenuItemComponent, VCLXAccessibleMenuItem_BASE )


// XServiceInfo


OUString VCLXAccessibleMenuItem::getImplementationName()
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleMenuItem" );
}


Sequence< OUString > VCLXAccessibleMenuItem::getSupportedServiceNames()
{
    return { "com.sun.star.awt.AccessibleMenuItem" };
}


// XAccessibleContext


sal_Int16 VCLXAccessibleMenuItem::getAccessibleRole(  )
{
    OExternalLockGuard aGuard( this );
    // IA2 CWS. MT: We had the additional roles in UAA for ever, but never used them anywhere.
    // Looks reasonable, but need to verify in Orca and VoiceOver.
    sal_Int16 nRole = AccessibleRole::MENU_ITEM;
    if ( m_pParent )
    {
        sal_uInt16 nItemId = m_pParent->GetItemId( m_nItemPos );
        MenuItemBits nItemBits = m_pParent->GetItemBits(nItemId);
        if(  nItemBits & MenuItemBits::RADIOCHECK)
            nRole = AccessibleRole::RADIO_MENU_ITEM;
        else if( nItemBits & MenuItemBits::CHECKABLE)
            nRole = AccessibleRole::CHECK_MENU_ITEM;
    }
    return nRole;
}


// XAccessibleText


sal_Int32 VCLXAccessibleMenuItem::getCaretPosition()
{
    OExternalLockGuard aGuard( this );

    return -1;
}


sal_Bool VCLXAccessibleMenuItem::setCaretPosition( sal_Int32 nIndex )
{

    OExternalLockGuard aGuard( this );

    if ( !implIsValidRange( nIndex, nIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    return false;
}


sal_Unicode VCLXAccessibleMenuItem::getCharacter( sal_Int32 nIndex )
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getCharacter( nIndex );
}


Sequence< PropertyValue > VCLXAccessibleMenuItem::getCharacterAttributes( sal_Int32 nIndex, const Sequence< OUString >& aRequestedAttributes )
{
    OExternalLockGuard aGuard( this );

    OUString sText( implGetText() );

    if ( !implIsValidIndex( nIndex, sText.getLength() ) )
        throw IndexOutOfBoundsException();

    vcl::Font aFont = Application::GetSettings().GetStyleSettings().GetMenuFont();
    sal_Int32 nBackColor = getBackground();
    sal_Int32 nColor = getForeground();
    return CharacterAttributesHelper( aFont, nBackColor, nColor )
        .GetCharacterAttributes( aRequestedAttributes );
}


awt::Rectangle VCLXAccessibleMenuItem::getCharacterBounds( sal_Int32 nIndex )
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


sal_Int32 VCLXAccessibleMenuItem::getCharacterCount()
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getCharacterCount();
}


sal_Int32 VCLXAccessibleMenuItem::getIndexAtPoint( const awt::Point& aPoint )
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


OUString VCLXAccessibleMenuItem::getSelectedText()
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getSelectedText();
}


sal_Int32 VCLXAccessibleMenuItem::getSelectionStart()
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getSelectionStart();
}


sal_Int32 VCLXAccessibleMenuItem::getSelectionEnd()
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getSelectionEnd();
}


sal_Bool VCLXAccessibleMenuItem::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    return false;
}


OUString VCLXAccessibleMenuItem::getText()
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getText();
}


OUString VCLXAccessibleMenuItem::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getTextRange( nStartIndex, nEndIndex );
}


css::accessibility::TextSegment VCLXAccessibleMenuItem::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType )
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getTextAtIndex( nIndex, aTextType );
}


css::accessibility::TextSegment VCLXAccessibleMenuItem::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType )
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getTextBeforeIndex( nIndex, aTextType );
}


css::accessibility::TextSegment VCLXAccessibleMenuItem::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType )
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getTextBehindIndex( nIndex, aTextType );
}


sal_Bool VCLXAccessibleMenuItem::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
{
    OExternalLockGuard aGuard( this );

    bool bReturn = false;

    if ( m_pParent )
    {
        vcl::Window* pWindow = m_pParent->GetWindow();
        if ( pWindow )
        {
            Reference< datatransfer::clipboard::XClipboard > xClipboard = pWindow->GetClipboard();
            if ( xClipboard.is() )
            {
                OUString sText( getTextRange( nStartIndex, nEndIndex ) );

                vcl::unohelper::TextDataObject* pDataObj = new vcl::unohelper::TextDataObject( sText );

                SolarMutexReleaser aReleaser;
                xClipboard->setContents( pDataObj, nullptr );
                Reference< datatransfer::clipboard::XFlushableClipboard > xFlushableClipboard( xClipboard, uno::UNO_QUERY );
                if( xFlushableClipboard.is() )
                    xFlushableClipboard->flushClipboard();

                bReturn = true;
            }
        }
    }

    return bReturn;
}


// XAccessibleAction


sal_Int32 VCLXAccessibleMenuItem::getAccessibleActionCount( )
{
    OExternalLockGuard aGuard( this );

    return 1;
}


sal_Bool VCLXAccessibleMenuItem::doAccessibleAction ( sal_Int32 nIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    Click();

    return true;
}


OUString VCLXAccessibleMenuItem::getAccessibleActionDescription ( sal_Int32 nIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return TK_RES_STRING( RID_STR_ACC_ACTION_SELECT );
}


Reference< XAccessibleKeyBinding > VCLXAccessibleMenuItem::getAccessibleActionKeyBinding( sal_Int32 nIndex )
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    OAccessibleKeyBindingHelper* pKeyBindingHelper = new OAccessibleKeyBindingHelper();
    Reference< XAccessibleKeyBinding > xKeyBinding = pKeyBindingHelper;

    if ( m_pParent )
    {
        // create auto mnemonics
        if (!(m_pParent->GetMenuFlags() & MenuFlags::NoAutoMnemonics))
            m_pParent->CreateAutoMnemonics();

        // activation key
        KeyEvent aKeyEvent = m_pParent->GetActivationKey( m_pParent->GetItemId( m_nItemPos ) );
        vcl::KeyCode aKeyCode = aKeyEvent.GetKeyCode();
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
        vcl::KeyCode aAccelKeyCode = m_pParent->GetAccelKey( m_pParent->GetItemId( m_nItemPos ) );
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


// XAccessibleValue


Any VCLXAccessibleMenuItem::getCurrentValue(  )
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    if ( IsSelected() )
        aValue <<= (sal_Int32) 1;
    else
        aValue <<= (sal_Int32) 0;

    return aValue;
}


sal_Bool VCLXAccessibleMenuItem::setCurrentValue( const Any& aNumber )
{
    OExternalLockGuard aGuard( this );

    bool bReturn = false;
    sal_Int32 nValue = 0;
    OSL_VERIFY( aNumber >>= nValue );

    if ( nValue <= 0 )
    {
        DeSelect();
        bReturn = true;
    }
    else if ( nValue >= 1 )
    {
        Select();
        bReturn = true;
    }

    return bReturn;
}


Any VCLXAccessibleMenuItem::getMaximumValue(  )
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    aValue <<= (sal_Int32) 1;

    return aValue;
}


Any VCLXAccessibleMenuItem::getMinimumValue(  )
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    aValue <<= (sal_Int32) 0;

    return aValue;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
