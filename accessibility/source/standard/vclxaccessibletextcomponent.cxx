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
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/convert.hxx>
#include <helper/characterattributeshelper.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp2.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/settings.hxx>

#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


//  class VCLXAccessibleTextComponent


VCLXAccessibleTextComponent::VCLXAccessibleTextComponent( VCLXWindow* pVCLXWindow )
    :VCLXAccessibleComponent( pVCLXWindow )
{
    if ( GetWindow() )
        m_sText = OutputDevice::GetNonMnemonicString( GetWindow()->GetText() );
}


VCLXAccessibleTextComponent::~VCLXAccessibleTextComponent()
{
}


void VCLXAccessibleTextComponent::SetText( const OUString& sText )
{
    Any aOldValue, aNewValue;
    if ( implInitTextChangedEvent( m_sText, sText, aOldValue, aNewValue ) )
    {
        m_sText = sText;
        NotifyAccessibleEvent( AccessibleEventId::TEXT_CHANGED, aOldValue, aNewValue );
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
    if ( GetWindow() )
        aText = OutputDevice::GetNonMnemonicString( GetWindow()->GetText() );

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


// XInterface


IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleTextComponent, VCLXAccessibleComponent, VCLXAccessibleTextComponent_BASE )


// XTypeProvider


IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleTextComponent, VCLXAccessibleComponent, VCLXAccessibleTextComponent_BASE )


// XAccessibleText


sal_Int32 VCLXAccessibleTextComponent::getCaretPosition()
{
    OExternalLockGuard aGuard( this );

    return -1;
}


sal_Bool VCLXAccessibleTextComponent::setCaretPosition( sal_Int32 nIndex )
{
    OExternalLockGuard aGuard( this );

    return setSelection( nIndex, nIndex );
}


sal_Unicode VCLXAccessibleTextComponent::getCharacter( sal_Int32 nIndex )
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getCharacter( nIndex );
}


Sequence< PropertyValue > VCLXAccessibleTextComponent::getCharacterAttributes( sal_Int32 nIndex, const Sequence< OUString >& aRequestedAttributes )
{
    OExternalLockGuard aGuard( this );

    Sequence< PropertyValue > aValues;
    OUString sText( implGetText() );

    if ( !implIsValidIndex( nIndex, sText.getLength() ) )
        throw IndexOutOfBoundsException();

    if ( GetWindow() )
    {
        vcl::Font aFont = GetWindow()->GetControlFont();

        sal_Int32 nBackColor = GetWindow()->GetControlBackground().GetColor();
        sal_Int32 nColor = GetWindow()->GetControlForeground().GetColor();

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

        aValues = CharacterAttributesHelper( aFont, nBackColor, nColor )
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

    return OCommonAccessibleText::getCharacterCount();
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

    return OCommonAccessibleText::getText();
}


OUString VCLXAccessibleTextComponent::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getTextRange( nStartIndex, nEndIndex );
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

    if ( GetWindow() )
    {
        Reference< datatransfer::clipboard::XClipboard > xClipboard = GetWindow()->GetClipboard();
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

    return bReturn;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
