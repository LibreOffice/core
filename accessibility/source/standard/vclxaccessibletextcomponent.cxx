/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: vclxaccessibletextcomponent.cxx,v $
 * $Revision: 1.3 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_accessibility.hxx"
#include <accessibility/standard/vclxaccessibletextcomponent.hxx>
#include <toolkit/helper/macros.hxx>
#include <toolkit/helper/convert.hxx>
#include <accessibility/helper/characterattributeshelper.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <vcl/window.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp2.hxx>
#include <vcl/ctrl.hxx>

#include <memory>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::accessibility;
using namespace ::comphelper;


//  ----------------------------------------------------
//  class VCLXAccessibleTextComponent
//  ----------------------------------------------------

VCLXAccessibleTextComponent::VCLXAccessibleTextComponent( VCLXWindow* pVCLXWindow )
    :VCLXAccessibleComponent( pVCLXWindow )
{
    if ( GetWindow() )
        m_sText = OutputDevice::GetNonMnemonicString( GetWindow()->GetText() );
}

// -----------------------------------------------------------------------------

VCLXAccessibleTextComponent::~VCLXAccessibleTextComponent()
{
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTextComponent::SetText( const ::rtl::OUString& sText )
{
    Any aOldValue, aNewValue;
    if ( implInitTextChangedEvent( m_sText, sText, aOldValue, aNewValue ) )
    {
        m_sText = sText;
        NotifyAccessibleEvent( AccessibleEventId::TEXT_CHANGED, aOldValue, aNewValue );
    }
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTextComponent::ProcessWindowEvent( const VclWindowEvent& rVclWindowEvent )
{
    switch ( rVclWindowEvent.GetId() )
    {
        case VCLEVENT_WINDOW_FRAMETITLECHANGED:
        {
            VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
            SetText( implGetText() );
        }
        break;
        default:
            VCLXAccessibleComponent::ProcessWindowEvent( rVclWindowEvent );
   }
}

// -----------------------------------------------------------------------------
// OCommonAccessibleText
// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleTextComponent::implGetText()
{
    ::rtl::OUString aText;
    if ( GetWindow() )
        aText = OutputDevice::GetNonMnemonicString( GetWindow()->GetText() );

    return aText;
}

// -----------------------------------------------------------------------------

lang::Locale VCLXAccessibleTextComponent::implGetLocale()
{
    return Application::GetSettings().GetLocale();
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTextComponent::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
{
    nStartIndex = 0;
    nEndIndex = 0;
}

// -----------------------------------------------------------------------------
// XComponent
// -----------------------------------------------------------------------------

void VCLXAccessibleTextComponent::disposing()
{
    VCLXAccessibleComponent::disposing();

    m_sText = ::rtl::OUString();
}

// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleTextComponent, VCLXAccessibleComponent, VCLXAccessibleTextComponent_BASE )

// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleTextComponent, VCLXAccessibleComponent, VCLXAccessibleTextComponent_BASE )

// -----------------------------------------------------------------------------
// XAccessibleText
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTextComponent::getCaretPosition() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return -1;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleTextComponent::setCaretPosition( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return setSelection( nIndex, nIndex );
}

// -----------------------------------------------------------------------------

sal_Unicode VCLXAccessibleTextComponent::getCharacter( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getCharacter( nIndex );
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > VCLXAccessibleTextComponent::getCharacterAttributes( sal_Int32 nIndex, const Sequence< ::rtl::OUString >& aRequestedAttributes ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Sequence< PropertyValue > aValues;
    ::rtl::OUString sText( implGetText() );

    if ( !implIsValidIndex( nIndex, sText.getLength() ) )
        throw IndexOutOfBoundsException();

    if ( GetWindow() )
    {
        Font aFont = GetWindow()->GetControlFont();
        sal_Int32 nBackColor = GetWindow()->GetControlBackground().GetColor();
        sal_Int32 nColor = GetWindow()->GetControlForeground().GetColor();
        ::std::auto_ptr< CharacterAttributesHelper > pHelper( new CharacterAttributesHelper( aFont, nBackColor, nColor ) );
        aValues = pHelper->GetCharacterAttributes( aRequestedAttributes );
    }

    return aValues;
}

// -----------------------------------------------------------------------------

awt::Rectangle VCLXAccessibleTextComponent::getCharacterBounds( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidIndex( nIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    awt::Rectangle aRect;
    Control* pControl = static_cast< Control* >( GetWindow() );
    if ( pControl )
        aRect = AWTRectangle( pControl->GetCharacterBounds( nIndex ) );

    return aRect;
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTextComponent::getCharacterCount() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getCharacterCount();
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTextComponent::getIndexAtPoint( const awt::Point& aPoint ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nIndex = -1;
    Control* pControl = static_cast< Control* >( GetWindow() );
    if ( pControl )
        nIndex = pControl->GetIndexForPoint( VCLPoint( aPoint ) );

    return nIndex;
}

// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleTextComponent::getSelectedText() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getSelectedText();
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTextComponent::getSelectionStart() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getSelectionStart();
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTextComponent::getSelectionEnd() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getSelectionEnd();
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleTextComponent::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    return sal_False;
}

// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleTextComponent::getText() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getText();
}

// -----------------------------------------------------------------------------

::rtl::OUString VCLXAccessibleTextComponent::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getTextRange( nStartIndex, nEndIndex );
}

// -----------------------------------------------------------------------------

::com::sun::star::accessibility::TextSegment VCLXAccessibleTextComponent::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getTextAtIndex( nIndex, aTextType );
}

// -----------------------------------------------------------------------------

::com::sun::star::accessibility::TextSegment VCLXAccessibleTextComponent::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getTextBeforeIndex( nIndex, aTextType );
}

// -----------------------------------------------------------------------------

::com::sun::star::accessibility::TextSegment VCLXAccessibleTextComponent::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OCommonAccessibleText::getTextBehindIndex( nIndex, aTextType );
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleTextComponent::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Bool bReturn = sal_False;

    if ( GetWindow() )
    {
        Reference< datatransfer::clipboard::XClipboard > xClipboard = GetWindow()->GetClipboard();
        if ( xClipboard.is() )
        {
            ::rtl::OUString sText( getTextRange( nStartIndex, nEndIndex ) );

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

    return bReturn;
}

// -----------------------------------------------------------------------------
