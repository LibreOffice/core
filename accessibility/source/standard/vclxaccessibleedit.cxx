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


// -----------------------------------------------------------------------------
// VCLXAccessibleEdit
// -----------------------------------------------------------------------------

VCLXAccessibleEdit::VCLXAccessibleEdit( VCLXWindow* pVCLWindow )
    :VCLXAccessibleTextComponent( pVCLWindow )
{
    m_nSelectionStart = getSelectionStart();
    m_nCaretPosition = getCaretPosition();
}

// -----------------------------------------------------------------------------

VCLXAccessibleEdit::~VCLXAccessibleEdit()
{
}

// -----------------------------------------------------------------------------

void VCLXAccessibleEdit::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_EDIT_MODIFY:
        {
            SetText( implGetText() );
        }
        break;
        case VCLEVENT_EDIT_SELECTIONCHANGED:
        {
            sal_Int32 nOldCaretPosition = m_nCaretPosition;
            sal_Int32 nOldSelectionStart = m_nSelectionStart;

            m_nCaretPosition = getCaretPosition();
            m_nSelectionStart = getSelectionStart();

            Window* pWindow = GetWindow();
            if ( pWindow && pWindow->HasChildPathFocus() )
            {
                if ( m_nCaretPosition != nOldCaretPosition )
                {
                    Any aOldValue, aNewValue;
                    aOldValue <<= (sal_Int32) nOldCaretPosition;
                    aNewValue <<= (sal_Int32) m_nCaretPosition;
                    NotifyAccessibleEvent( AccessibleEventId::CARET_CHANGED, aOldValue, aNewValue );
                }

                // #i104470# VCL only has SELECTION_CHANGED, but UAA distinguishes between SELECTION_CHANGED and CARET_CHANGED
                sal_Bool bHasSelection = ( m_nSelectionStart != m_nCaretPosition );
                sal_Bool bHadSelection = ( nOldSelectionStart != nOldCaretPosition );
                if ( ( bHasSelection != bHadSelection ) || ( bHasSelection && ( ( m_nCaretPosition != nOldCaretPosition ) || ( m_nSelectionStart != nOldSelectionStart ) ) ) )
                {
                    NotifyAccessibleEvent( AccessibleEventId::TEXT_SELECTION_CHANGED, Any(), Any() );
                }

            }
        }
        break;
        default:
            VCLXAccessibleTextComponent::ProcessWindowEvent( rVclWindowEvent );
   }
}

// -----------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------
// OCommonAccessibleText
// -----------------------------------------------------------------------------

OUString VCLXAccessibleEdit::implGetText()
{
    OUString aText;

    Edit* pEdit = static_cast< Edit* >( GetWindow() );
    if ( pEdit )
    {
        aText = OutputDevice::GetNonMnemonicString( pEdit->GetText() );

        if ( getAccessibleRole() == AccessibleRole::PASSWORD_TEXT )
        {
            xub_Unicode cEchoChar = pEdit->GetEchoChar();
            if ( !cEchoChar )
                cEchoChar = '*';
            XubString sTmp;
            aText = sTmp.Fill( (sal_uInt16)aText.getLength(), cEchoChar );
        }
    }

    return aText;
}

// -----------------------------------------------------------------------------

void VCLXAccessibleEdit::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
{
    awt::Selection aSelection;
    VCLXEdit* pVCLXEdit = static_cast< VCLXEdit* >( GetVCLXWindow() );
    if ( pVCLXEdit )
        aSelection = pVCLXEdit->getSelection();

    nStartIndex = aSelection.Min;
    nEndIndex = aSelection.Max;
}

// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleEdit, VCLXAccessibleTextComponent, VCLXAccessibleEdit_BASE )

// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleEdit, VCLXAccessibleTextComponent, VCLXAccessibleEdit_BASE )

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

OUString VCLXAccessibleEdit::getImplementationName() throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleEdit" );
}

// -----------------------------------------------------------------------------

Sequence< OUString > VCLXAccessibleEdit::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< OUString > aNames(1);
    aNames[0] = "com.sun.star.awt.AccessibleEdit";
    return aNames;
}

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleEdit::getAccessibleChildCount() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return 0;
}

// -----------------------------------------------------------------------------

Reference< XAccessible > VCLXAccessibleEdit::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    return Reference< XAccessible >();
}

// -----------------------------------------------------------------------------

sal_Int16 VCLXAccessibleEdit::getAccessibleRole(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int16 nRole;
    Edit* pEdit = static_cast< Edit* >( GetWindow() );
    if ( pEdit && ( ( pEdit->GetStyle() & WB_PASSWORD ) || pEdit->GetEchoChar() ) )
        nRole = AccessibleRole::PASSWORD_TEXT;
    else
        nRole = AccessibleRole::TEXT;

    return nRole;
}

// -----------------------------------------------------------------------------
// XAccessibleAction
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleEdit::getAccessibleActionCount( ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    // There is one action: activate
    return 1;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleEdit::doAccessibleAction ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    sal_Bool bDoAction = sal_False;
    Window* pWindow = GetWindow();
    if ( pWindow )
    {
        pWindow->GrabFocus();
        bDoAction = sal_True;
    }

    return bDoAction;
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleEdit::getAccessibleActionDescription ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    static const OUString sAction( "activate" );
    return sAction;
}

// -----------------------------------------------------------------------------

Reference< XAccessibleKeyBinding > VCLXAccessibleEdit::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return Reference< XAccessibleKeyBinding >();
}

// -----------------------------------------------------------------------------
// XAccessibleText
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleEdit::getCaretPosition(  ) throw (RuntimeException)
{
    return getSelectionEnd();
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleEdit::setCaretPosition( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    return setSelection( nIndex, nIndex );
}

// -----------------------------------------------------------------------------

sal_Unicode VCLXAccessibleEdit::getCharacter( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getCharacter( nIndex );
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > VCLXAccessibleEdit::getCharacterAttributes( sal_Int32 nIndex, const Sequence< OUString >& aRequestedAttributes ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getCharacterAttributes( nIndex, aRequestedAttributes );
}

// -----------------------------------------------------------------------------

awt::Rectangle VCLXAccessibleEdit::getCharacterBounds( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    awt::Rectangle aBounds( 0, 0, 0, 0 );
    sal_Int32 nLength = implGetText().getLength();

    if ( !implIsValidRange( nIndex, nIndex, nLength ) )
        throw IndexOutOfBoundsException();

    Control* pControl = static_cast< Control* >( GetWindow() );
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

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleEdit::getCharacterCount(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getCharacterCount();
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleEdit::getIndexAtPoint( const awt::Point& aPoint ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getIndexAtPoint( aPoint );
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleEdit::getSelectedText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getSelectedText();
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleEdit::getSelectionStart(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getSelectionStart();
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleEdit::getSelectionEnd(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getSelectionEnd();
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleEdit::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Bool bReturn = sal_False;
    OUString sText( implGetText() );

    if ( !implIsValidRange( nStartIndex, nEndIndex, sText.getLength() ) )
        throw IndexOutOfBoundsException();

    VCLXEdit* pVCLXEdit = static_cast< VCLXEdit* >( GetVCLXWindow() );
    Edit* pEdit = static_cast< Edit* >( GetWindow() );
    if ( pVCLXEdit && pEdit && pEdit->IsEnabled() )
    {
        pVCLXEdit->setSelection( awt::Selection( nStartIndex, nEndIndex ) );
        bReturn = sal_True;
    }

    return bReturn;
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleEdit::getText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getText();
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleEdit::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getTextRange( nStartIndex, nEndIndex );
}

// -----------------------------------------------------------------------------

::com::sun::star::accessibility::TextSegment VCLXAccessibleEdit::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getTextAtIndex( nIndex, aTextType );
}

// -----------------------------------------------------------------------------

::com::sun::star::accessibility::TextSegment VCLXAccessibleEdit::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getTextBeforeIndex( nIndex, aTextType );
}

// -----------------------------------------------------------------------------

::com::sun::star::accessibility::TextSegment VCLXAccessibleEdit::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getTextBehindIndex( nIndex, aTextType );
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleEdit::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::copyText( nStartIndex, nEndIndex );
}

// -----------------------------------------------------------------------------
// XAccessibleEditableText
// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleEdit::cutText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return copyText( nStartIndex, nEndIndex ) && deleteText( nStartIndex, nEndIndex );
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleEdit::pasteText( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Bool bReturn = sal_False;

    if ( GetWindow() )
    {
        Reference< datatransfer::clipboard::XClipboard > xClipboard = GetWindow()->GetClipboard();
        if ( xClipboard.is() )
        {
            const sal_uInt32 nRef = Application::ReleaseSolarMutex();
            Reference< datatransfer::XTransferable > xDataObj = xClipboard->getContents();
            Application::AcquireSolarMutex( nRef );
            if ( xDataObj.is() )
            {
                datatransfer::DataFlavor aFlavor;
                SotExchange::GetFormatDataFlavor( SOT_FORMAT_STRING, aFlavor );
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

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleEdit::deleteText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return replaceText( nStartIndex, nEndIndex, OUString() );
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleEdit::insertText( const OUString& sText, sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return replaceText( nIndex, nIndex, sText );
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleEdit::replaceText( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const OUString& sReplacement ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Bool bReturn = sal_False;
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
        bReturn = sal_True;
    }

    return bReturn;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleEdit::setAttributes( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const Sequence<PropertyValue>& ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    return sal_False;   // attributes cannot be set for an edit
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleEdit::setText( const OUString& sText ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Bool bSuccess = sal_False;
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

// -----------------------------------------------------------------------------

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
