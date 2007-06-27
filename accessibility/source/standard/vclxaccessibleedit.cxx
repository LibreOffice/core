/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclxaccessibleedit.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 15:38:04 $
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

#ifndef ACCESSIBILITY_STANDARD_VCLXACCESSIBLEEDIT_HXX
#include <accessibility/standard/vclxaccessibleedit.hxx>
#endif

#ifndef _TOOLKIT_AWT_VCLXWINDOWS_HXX
#include <toolkit/awt/vclxwindows.hxx>
#endif
#ifndef _TOOLKIT_HELPER_CONVERT_HXX_
#include <toolkit/helper/convert.hxx>
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
#ifndef _UTL_ACCESSIBLERELATIONSETHELPER_HXX
#include <unotools/accessiblerelationsethelper.hxx>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLESTATETYPE_HPP_
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLEEVENTID_HPP_
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#endif
#ifndef _COM_SUN_STAR_ACCESSIBILITY_STANDARD_ACCESSIBLEROLE_HPP_
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_CLIPBOARD_XCLIPBOARD_HPP_
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX
#include <cppuhelper/typeprovider.hxx>
#endif
#ifndef _COMPHELPER_SEQUENCE_HXX
#include <comphelper/sequence.hxx>
#endif

#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _SV_WINDOW_HXX
#include <vcl/window.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SOT_EXCHANGE_HXX
#include <sot/exchange.hxx>
#endif
#ifndef _SOT_FORMATS_HXX
#include <sot/formats.hxx>
#endif

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
    m_nCaretPosition = getCaretPosition();
}

// -----------------------------------------------------------------------------

VCLXAccessibleEdit::~VCLXAccessibleEdit()
{
}

// -----------------------------------------------------------------------------

void VCLXAccessibleEdit::UpdateCaretPosition()
{
    sal_Int32 nCaretPosition = getCaretPosition();

    if ( m_nCaretPosition != nCaretPosition )
    {
        Any aOldValue, aNewValue;
        aOldValue <<= (sal_Int32) m_nCaretPosition;
        aNewValue <<= (sal_Int32) nCaretPosition;
        m_nCaretPosition = nCaretPosition;
        NotifyAccessibleEvent( AccessibleEventId::CARET_CHANGED, aOldValue, aNewValue );
    }
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
            Window* pWindow = GetWindow();
            if ( pWindow && pWindow->HasChildPathFocus() )
            {
                NotifyAccessibleEvent( AccessibleEventId::TEXT_SELECTION_CHANGED, Any(), Any() );
                UpdateCaretPosition();
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

::rtl::OUString VCLXAccessibleEdit::implGetText()
{
    ::rtl::OUString aText;

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
            aText = sTmp.Fill( (USHORT)aText.getLength(), cEchoChar );
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

::rtl::OUString VCLXAccessibleEdit::getImplementationName() throw (RuntimeException)
{
    return ::rtl::OUString::createFromAscii( "com.sun.star.comp.toolkit.AccessibleEdit" );
}

// -----------------------------------------------------------------------------

Sequence< ::rtl::OUString > VCLXAccessibleEdit::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< ::rtl::OUString > aNames(1);
    aNames[0] = ::rtl::OUString::createFromAscii( "com.sun.star.awt.AccessibleEdit" );
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

    return 0;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleEdit::doAccessibleAction ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return sal_False;
}

// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleEdit::getAccessibleActionDescription ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return ::rtl::OUString();
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
    OExternalLockGuard aGuard( this );

    awt::Selection aSelection;
    VCLXEdit* pVCLXEdit = static_cast< VCLXEdit* >( GetVCLXWindow() );
    if ( pVCLXEdit )
        aSelection = pVCLXEdit->getSelection();

    return aSelection.Max;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleEdit::setCaretPosition( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return setSelection( nIndex, nIndex );
}

// -----------------------------------------------------------------------------

sal_Unicode VCLXAccessibleEdit::getCharacter( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getCharacter( nIndex );
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > VCLXAccessibleEdit::getCharacterAttributes( sal_Int32 nIndex, const Sequence< ::rtl::OUString >& aRequestedAttributes ) throw (IndexOutOfBoundsException, RuntimeException)
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

::rtl::OUString VCLXAccessibleEdit::getSelectedText(  ) throw (RuntimeException)
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
    ::rtl::OUString sText( implGetText() );

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

::rtl::OUString VCLXAccessibleEdit::getText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return VCLXAccessibleTextComponent::getText();
}

// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleEdit::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
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
                    ::rtl::OUString sText;
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

    return replaceText( nStartIndex, nEndIndex, ::rtl::OUString() );
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleEdit::insertText( const ::rtl::OUString& sText, sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return replaceText( nIndex, nIndex, sText );
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleEdit::replaceText( sal_Int32 nStartIndex, sal_Int32 nEndIndex, const ::rtl::OUString& sReplacement ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Bool bReturn = sal_False;
    ::rtl::OUString sText( implGetText() );

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

sal_Bool VCLXAccessibleEdit::setText( const ::rtl::OUString& sText ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Bool bSuccess = sal_False;
    try
    {
        bSuccess = replaceText( 0, implGetText().getLength(), sText );
    }
    catch( const IndexOutOfBoundsException& )
    {
        OSL_ENSURE( sal_False, "VCLXAccessibleText::setText: caught an exception!" );
    }
    return bSuccess;
}

// -----------------------------------------------------------------------------
