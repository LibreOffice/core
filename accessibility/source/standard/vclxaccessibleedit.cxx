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

#include <accessibility/standard/vclxaccessibleedit.hxx>

#include <toolkit/awt/vclxwindows.hxx>
#include <toolkit/helper/convert.hxx>
#include <accessibility/helper/accresmgr.hxx>
#include <accessibility/helper/accessiblestrings.hrc>

#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/string.hxx>
#include <vcl/svapp.hxx>
#include <vcl/window.hxx>
#include <vcl/edit.hxx>
#include <sot/exchange.hxx>
#include <sot/formats.hxx>

#include <algorithm>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;



// VCLXAccessibleEdit


VCLXAccessibleEdit::VCLXAccessibleEdit( VCLXWindow* pVCLWindow )
    :VCLXAccessibleTextComponent( pVCLWindow )
{
    m_nSelectionStart = getSelectionStart();
    m_nCaretPosition = getCaretPosition();
}



VCLXAccessibleEdit::~VCLXAccessibleEdit()
{
}



void VCLXAccessibleEdit::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_EDIT_MODIFY:
        {
            SetText( implGetText() );
        }
        break;
        case VCLEVENT_EDIT_CARETCHANGED:
        {
            sal_Int32 nOldCaretPosition = m_nCaretPosition;
            m_nCaretPosition = getCaretPosition();

            vcl::Window* pWindow = GetWindow();
            if (pWindow && pWindow->HasChildPathFocus())
            {
                if (m_nCaretPosition != nOldCaretPosition)
                {
                    Any aOldValue, aNewValue;
                    aOldValue <<= nOldCaretPosition;
                    aNewValue <<= m_nCaretPosition;
                    NotifyAccessibleEvent( AccessibleEventId::CARET_CHANGED, aOldValue, aNewValue );
                }
            }
        }
        break;
        case VCLEVENT_EDIT_SELECTIONCHANGED:
        {
            vcl::Window* pWindow = GetWindow();
            if (pWindow && pWindow->HasChildPathFocus())
            {
                NotifyAccessibleEvent( AccessibleEventId::TEXT_SELECTION_CHANGED, Any(), Any() );
            }
        }
        break;
        default:
            VCLXAccessibleTextComponent::ProcessWindowEvent( rVclWindowEvent );
   }
}



void VCLXAccessibleEdit::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    VCLXAccessibleTextComponent::FillAccessibleStateSet( rStateSet );

    VCLXEdit* pVCLXEdit = static_cast< VCLXEdit* >( GetVCLXWindow() );
    if ( pVCLXEdit )
    {
        rStateSet.AddState( AccessibleStateType::FOCUSABLE );
        rStateSet.AddState( AccessibleStateType::SINGLE_LINE );
        if ( pVCLXEdit->isEditable() )
            rStateSet.AddState( AccessibleStateType::EDITABLE );
    }
}


// OCommonAccessibleText


OUString VCLXAccessibleEdit::implGetText()
{
    OUString aText;

    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit )
    {
        aText = OutputDevice::GetNonMnemonicString( pEdit->GetText() );

        if ( getAccessibleRole() == AccessibleRole::PASSWORD_TEXT )
        {
            sal_Unicode cEchoChar = pEdit->GetEchoChar();
            if ( !cEchoChar )
                cEchoChar = '*';
            OUStringBuffer sTmp;
            aText = comphelper::string::padToLength(sTmp, aText.getLength(),
                cEchoChar).makeStringAndClear();
        }
    }

    return aText;
}



void VCLXAccessibleEdit::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
{
    awt::Selection aSelection;
    VCLXEdit* pVCLXEdit = static_cast< VCLXEdit* >( GetVCLXWindow() );
    if ( pVCLXEdit )
        aSelection = pVCLXEdit->getSelection();

    nStartIndex = aSelection.Min;
    nEndIndex = aSelection.Max;
}


// XInterface


IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleEdit, VCLXAccessibleTextComponent, VCLXAccessibleEdit_BASE )


// XTypeProvider


IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleEdit, VCLXAccessibleTextComponent, VCLXAccessibleEdit_BASE )


// XServiceInfo


OUString VCLXAccessibleEdit::getImplementationName() throw (RuntimeException, std::exception)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleEdit" );
}



Sequence< OUString > VCLXAccessibleEdit::getSupportedServiceNames() throw (RuntimeException, std::exception)
{
    Sequence< OUString > aNames { "com.sun.star.awt.AccessibleEdit" };
    return aNames;
}


// XAccessibleContext


sal_Int32 VCLXAccessibleEdit::getAccessibleChildCount() throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return 0;
}



Reference< XAccessible > VCLXAccessibleEdit::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    return Reference< XAccessible >();
}



sal_Int16 VCLXAccessibleEdit::getAccessibleRole(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    sal_Int16 nRole;
    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pEdit && ( ( pEdit->GetStyle() & WB_PASSWORD ) || pEdit->GetEchoChar() ) )
        nRole = AccessibleRole::PASSWORD_TEXT;
    else
        nRole = AccessibleRole::TEXT;

    return nRole;
}


// XAccessibleAction


sal_Int32 VCLXAccessibleEdit::getAccessibleActionCount( ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    // There is one action: activate
    return 1;
}



sal_Bool VCLXAccessibleEdit::doAccessibleAction ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    bool bDoAction = false;
    vcl::Window* pWindow = GetWindow();
    if ( pWindow )
    {
        pWindow->GrabFocus();
        bDoAction = true;
    }

    return bDoAction;
}



OUString VCLXAccessibleEdit::getAccessibleActionDescription ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return OUString( "activate" );
}



Reference< XAccessibleKeyBinding > VCLXAccessibleEdit::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return Reference< XAccessibleKeyBinding >();
}


// XAccessibleText


sal_Int32 VCLXAccessibleEdit::getCaretPosition(  ) throw (RuntimeException, std::exception)
{
    return getSelectionEnd();
}



sal_Bool VCLXAccessibleEdit::setCaretPosition( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    return setSelection( nIndex, nIndex );
}



sal_Unicode VCLXAccessibleEdit::getCharacter( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getCharacter( nIndex );
}



Sequence< PropertyValue > VCLXAccessibleEdit::getCharacterAttributes( sal_Int32 nIndex, const Sequence< OUString >& aRequestedAttributes ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getCharacterAttributes( nIndex, aRequestedAttributes );
}



awt::Rectangle VCLXAccessibleEdit::getCharacterBounds( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    awt::Rectangle aBounds( 0, 0, 0, 0 );
    sal_Int32 nLength = implGetText().getLength();

    if ( !implIsValidRange( nIndex, nIndex, nLength ) )
        throw IndexOutOfBoundsException();

    VclPtr< Control > pControl = GetAs< Control >();
    if ( pControl )
    {
        if ( nIndex == nLength )
        {
            // #108914# calculate virtual bounding rectangle
            for ( sal_Int32 i = 0; i < nLength; ++i )
            {
                Rectangle aRect = pControl->GetCharacterBounds( i );
                sal_Int32 nHeight = aRect.GetHeight();
                if ( aBounds.Height < nHeight )
                {
                    aBounds.Y = aRect.Top();
                    aBounds.Height = nHeight;
                }
                if ( i == nLength - 1 )
                {
                    aBounds.X = aRect.Right() + 1;
                    aBounds.Width = 1;
                }
            }
        }
        else
        {
            aBounds = AWTRectangle( pControl->GetCharacterBounds( nIndex ) );
        }
    }

    return aBounds;
}



sal_Int32 VCLXAccessibleEdit::getCharacterCount(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getCharacterCount();
}



sal_Int32 VCLXAccessibleEdit::getIndexAtPoint( const awt::Point& aPoint ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getIndexAtPoint( aPoint );
}



OUString VCLXAccessibleEdit::getSelectedText(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getSelectedText();
}



sal_Int32 VCLXAccessibleEdit::getSelectionStart(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getSelectionStart();
}



sal_Int32 VCLXAccessibleEdit::getSelectionEnd(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getSelectionEnd();
}



sal_Bool VCLXAccessibleEdit::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    bool bReturn = false;
    OUString sText( implGetText() );

    if ( !implIsValidRange( nStartIndex, nEndIndex, sText.getLength() ) )
        throw IndexOutOfBoundsException();

    VCLXEdit* pVCLXEdit = static_cast< VCLXEdit* >( GetVCLXWindow() );
    VclPtr< Edit > pEdit = GetAs< Edit >();
    if ( pVCLXEdit && pEdit && pEdit->IsEnabled() )
    {
        pVCLXEdit->setSelection( awt::Selection( nStartIndex, nEndIndex ) );
        bReturn = true;
    }

    return bReturn;
}



OUString VCLXAccessibleEdit::getText(  ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getText();
}



OUString VCLXAccessibleEdit::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getTextRange( nStartIndex, nEndIndex );
}



css::accessibility::TextSegment VCLXAccessibleEdit::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getTextAtIndex( nIndex, aTextType );
}



css::accessibility::TextSegment VCLXAccessibleEdit::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getTextBeforeIndex( nIndex, aTextType );
}



css::accessibility::TextSegment VCLXAccessibleEdit::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getTextBehindIndex( nIndex, aTextType );
}



sal_Bool VCLXAccessibleEdit::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::copyText( nStartIndex, nEndIndex );
}


// XAccessibleEditableText


sal_Bool VCLXAccessibleEdit::cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return copyText( nStartIndex, nEndIndex ) && deleteText( nStartIndex, nEndIndex );
}



sal_Bool VCLXAccessibleEdit::pasteText( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    bool bReturn = false;

    if ( GetWindow() )
    {
        Reference< datatransfer::clipboard::XClipboard > xClipboard = GetWindow()->GetClipboard();
        if ( xClipboard.is() )
        {
            Reference< datatransfer::XTransferable > xDataObj;
            {
                SolarMutexReleaser aReleaser;
                xDataObj = xClipboard->getContents();
            }
            if ( xDataObj.is() )
            {
                datatransfer::DataFlavor aFlavor;
                SotExchange::GetFormatDataFlavor( SotClipboardFormatId::STRING, aFlavor );
                if ( xDataObj->isDataFlavorSupported( aFlavor ) )
                {
                    Any aData = xDataObj->getTransferData( aFlavor );
                    OUString sText;
                    aData >>= sText;
                    bReturn = replaceText( nIndex, nIndex, sText );
                }
            }
        }
    }

    return bReturn;
}



sal_Bool VCLXAccessibleEdit::deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return replaceText( nStartIndex, nEndIndex, OUString() );
}



sal_Bool VCLXAccessibleEdit::insertText( const OUString& sText, sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    return replaceText( nIndex, nIndex, sText );
}



sal_Bool VCLXAccessibleEdit::replaceText( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const OUString& sReplacement ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    bool bReturn = false;
    OUString sText( implGetText() );

    if ( !implIsValidRange( nStartIndex, nEndIndex, sText.getLength() ) )
        throw IndexOutOfBoundsException();

    sal_Int32 nMinIndex = ::std::min( nStartIndex, nEndIndex );
    sal_Int32 nMaxIndex = ::std::max( nStartIndex, nEndIndex );

    VCLXEdit* pVCLXEdit = static_cast< VCLXEdit* >( GetVCLXWindow() );
    if ( pVCLXEdit && pVCLXEdit->isEditable() )
    {
        pVCLXEdit->setText( sText.replaceAt( nMinIndex, nMaxIndex - nMinIndex, sReplacement ) );
        sal_Int32 nIndex = nMinIndex + sReplacement.getLength();
        setSelection( nIndex, nIndex );
        bReturn = true;
    }

    return bReturn;
}



sal_Bool VCLXAccessibleEdit::setAttributes( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const Sequence<PropertyValue>& ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    return false;   // attributes cannot be set for an edit
}



sal_Bool VCLXAccessibleEdit::setText( const OUString& sText ) throw (RuntimeException, std::exception)
{
    OExternalLockGuard aGuard( this );

    bool bSuccess = false;
    try
    {
        bSuccess = replaceText( 0, implGetText().getLength(), sText );
    }
    catch( const IndexOutOfBoundsException& )
    {
        OSL_FAIL( "VCLXAccessibleText::setText: caught an exception!" );
    }
    return bSuccess;
}



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
