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

#include <standard/vclxaccessibletextcomponent.hxx>
#include <toolkit/helper/convert.hxx>
#include <helper/characterattributeshelper.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <comphelper/accessiblecontexthelper.hxx>
#include <vcl/window.hxx>
#include <vcl/mnemonic.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp2.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/settings.hxx>
#include <i18nlangtag/languagetag.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;




VCLXAccessibleTextComponent::VCLXAccessibleTextComponent( VCLXWindow* pVCLXWindow )
    :ImplInheritanceHelper( pVCLXWindow )
{
    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
        m_sText = removeMnemonicFromString( pWindow->GetText() );
}


void VCLXAccessibleTextComponent::SetText( const OUString& sText )
{
    Any aOldValue, aNewValue;
    bool bChanged = false;
    if (!PreferFullTextInTextChangedEvent())
    {
        bChanged = implInitTextChangedEvent(m_sText, sText, aOldValue, aNewValue);

    }
    else if (sText != m_sText)
    {
        // use the full old/new text as old/new value
        TextSegment aDeletedText(m_sText, 0, m_sText.getLength());
        aOldValue <<= aDeletedText;
        TextSegment aInsertedText(sText, 0, sText.getLength());
        aNewValue <<= aInsertedText;
        bChanged = true;
    }

    if (bChanged)
    {
        m_sText = sText;
        NotifyAccessibleEvent( AccessibleEventId::TEXT_CHANGED, aOldValue, aNewValue );
    }

    // check whether accessible name has also changed, since text is (often) used as name as well
    const OUString sName = getAccessibleName();
    if (sName != m_sOldName)
    {
        NotifyAccessibleEvent(AccessibleEventId::NAME_CHANGED, Any(m_sOldName), Any(sName));
        m_sOldName = sName;
    }
}


void VCLXAccessibleTextComponent::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VclEventId::WindowFrameTitleChanged:
        {
            VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
            SetText( implGetText() );
        }
        break;
        default:
            VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
   }
}


// OCommonAccessibleText


OUString VCLXAccessibleTextComponent::implGetText()
{
    OUString aText;
    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
        aText = removeMnemonicFromString( pWindow->GetText() );

    return aText;
}


lang::Locale VCLXAccessibleTextComponent::implGetLocale()
{
    return Application::GetSettings().GetLanguageTag().getLocale();
}


void VCLXAccessibleTextComponent::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
{
    nStartIndex = 0;
    nEndIndex = 0;
}


// XComponent


void VCLXAccessibleTextComponent::disposing()
{
    VCLXAccessibleComponent::disposing();

    m_sText.clear();
}


// XAccessibleText


sal_Int32 VCLXAccessibleTextComponent::getCaretPosition()
{
    return -1;
}


sal_Bool VCLXAccessibleTextComponent::setCaretPosition( sal_Int32 nIndex )
{
    return setSelection( nIndex, nIndex );
}


sal_Unicode VCLXAccessibleTextComponent::getCharacter( sal_Int32 nIndex )
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::implGetCharacter( implGetText(), nIndex );
}


Sequence< PropertyValue > VCLXAccessibleTextComponent::getCharacterAttributes( sal_Int32 nIndex, const Sequence< OUString >& aRequestedAttributes )
{
    OExternalLockGuard aGuard( this );

    Sequence< PropertyValue > aValues;
    OUString sText( implGetText() );

    if ( !implIsValidIndex( nIndex, sText.getLength() ) )
        throw IndexOutOfBoundsException();

    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
    {
        vcl::Font aFont = pWindow->GetControlFont();

        Color nBackColor = pWindow->GetControlBackground();
        Color nColor = pWindow->GetControlForeground();

        // MT: Code with default font was introduced with the IA2 CWS, but I am not convinced that this is the correct font...
        // Decide what to do when we have a concrete issue.
        /*
        Font aDefaultVCLFont;
        OutputDevice* pDev = Application::GetDefaultDevice();
        if ( pDev )
        {
            aDefaultVCLFont = pDev->GetSettings().GetStyleSettings().GetAppFont();
            if ( !aFont.GetName().Len() )
            {
                String aDefaultName = aDefaultVCLFont.GetName();
                aFont.SetName( aDefaultName );
            }
            if ( !aFont.GetHeight() )
            {
                aFont.SetHeight( aDefaultVCLFont.GetHeight() );
            }
            if ( aFont.GetWeight() == WEIGHT_DONTKNOW )
            {
                aFont.SetWeight( aDefaultVCLFont.GetWeight() );
            }

            //if nColor is -1, it may indicate that the default color black is using.
            if ( nColor == -1)
            {
                nColor = aDefaultVCLFont.GetColor().GetColor();
            }
        }
        */

        // MT: Adjustment stuff was introduced with the IA2 CWS, but adjustment is not a character attribute...
        // In case we reintroduce it, use adjustment as extra parameter for the CharacterAttributesHelper...
        /*
        WinBits aBits = GetWindow()->GetStyle();
        sal_Int16 nAdjust = -1;
        if ( aBits & WB_LEFT )
        {
            nAdjust = style::ParagraphAdjust_LEFT;
        }
        else if ( aBits & WB_RIGHT )
        {
            nAdjust = style::ParagraphAdjust_RIGHT;
        }
        else if ( aBits & WB_CENTER )
        {
            nAdjust = style::ParagraphAdjust_CENTER;
        }
        */

        aValues = CharacterAttributesHelper( aFont, sal_Int32(nBackColor), sal_Int32(nColor) )
            .GetCharacterAttributes( aRequestedAttributes );
    }

    return aValues;
}


awt::Rectangle VCLXAccessibleTextComponent::getCharacterBounds( sal_Int32 nIndex )
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidIndex( nIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    awt::Rectangle aRect;
    VclPtr< Control > pControl = GetAs< Control >();
    if ( pControl )
        aRect = AWTRectangle( pControl->GetCharacterBounds( nIndex ) );

    return aRect;
}


sal_Int32 VCLXAccessibleTextComponent::getCharacterCount()
{
    OExternalLockGuard aGuard( this );

    return implGetText().getLength();
}


sal_Int32 VCLXAccessibleTextComponent::getIndexAtPoint( const awt::Point& aPoint )
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nIndex = -1;
    VclPtr< Control > pControl = GetAs< Control >();
    if ( pControl )
        nIndex = pControl->GetIndexForPoint( VCLPoint( aPoint ) );

    return nIndex;
}


OUString VCLXAccessibleTextComponent::getSelectedText()
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getSelectedText();
}


sal_Int32 VCLXAccessibleTextComponent::getSelectionStart()
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getSelectionStart();
}


sal_Int32 VCLXAccessibleTextComponent::getSelectionEnd()
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getSelectionEnd();
}


sal_Bool VCLXAccessibleTextComponent::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    return false;
}


OUString VCLXAccessibleTextComponent::getText()
{
    OExternalLockGuard aGuard( this );

    return implGetText();
}


OUString VCLXAccessibleTextComponent::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::implGetTextRange( implGetText(), nStartIndex, nEndIndex );
}


css::accessibility::TextSegment VCLXAccessibleTextComponent::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType )
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getTextAtIndex( nIndex, aTextType );
}


css::accessibility::TextSegment VCLXAccessibleTextComponent::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType )
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getTextBeforeIndex( nIndex, aTextType );
}


css::accessibility::TextSegment VCLXAccessibleTextComponent::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType )
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getTextBehindIndex( nIndex, aTextType );
}


sal_Bool VCLXAccessibleTextComponent::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
{
    OExternalLockGuard aGuard( this );

    bool bReturn = false;

    VclPtr<vcl::Window> pWindow = GetWindow();
    if ( pWindow )
    {
        Reference< datatransfer::clipboard::XClipboard > xClipboard = pWindow->GetClipboard();
        if ( xClipboard.is() )
        {
            OUString sText( OCommonAccessibleText::implGetTextRange( implGetText(), nStartIndex, nEndIndex ) );

            rtl::Reference<vcl::unohelper::TextDataObject> pDataObj = new vcl::unohelper::TextDataObject( sText );

            SolarMutexReleaser aReleaser;
            xClipboard->setContents( pDataObj, nullptr );

            Reference< datatransfer::clipboard::XFlushableClipboard > xFlushableClipboard( xClipboard, uno::UNO_QUERY );
            if( xFlushableClipboard.is() )
                xFlushableClipboard->flushClipboard();

            bReturn = true;
        }
    }

    return bReturn;
}

sal_Bool VCLXAccessibleTextComponent::scrollSubstringTo( sal_Int32, sal_Int32, AccessibleScrollType )
{
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
