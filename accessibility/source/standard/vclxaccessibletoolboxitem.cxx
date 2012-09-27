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

#include <accessibility/standard/vclxaccessibletoolboxitem.hxx>
#include <toolkit/helper/convert.hxx>
#include <accessibility/helper/accresmgr.hxx>
#include <accessibility/helper/accessiblestrings.hrc>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolbox.hxx>
#include <vcl/unohelp2.hxx>
#include <vcl/help.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/externallock.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>

#include <com/sun/star/accessibility/XAccessibleSelection.hpp>

// class VCLXAccessibleToolBoxItem ------------------------------------------

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;
using namespace ::comphelper;

// -----------------------------------------------------------------------------
// Ctor() and Dtor()
// -----------------------------------------------------------------------------
VCLXAccessibleToolBoxItem::VCLXAccessibleToolBoxItem( ToolBox* _pToolBox, sal_Int32 _nPos ) :

    AccessibleTextHelper_BASE( new VCLExternalSolarLock() ),

    m_pToolBox      ( _pToolBox ),
    m_nIndexInParent( _nPos ),
    m_nRole         ( AccessibleRole::PUSH_BUTTON ),
    m_nItemId       ( 0 ),
    m_bHasFocus     ( sal_False ),
    m_bIsChecked    ( sal_False ),
    m_bIndeterminate( false )

{
    m_pExternalLock = static_cast< VCLExternalSolarLock* >( getExternalLock( ) );

    OSL_ENSURE( m_pToolBox, "invalid toolbox" );
    m_nItemId = m_pToolBox->GetItemId( (sal_uInt16)m_nIndexInParent );
    m_sOldName = GetText( true );
    m_bIsChecked = m_pToolBox->IsItemChecked( m_nItemId );
    m_bIndeterminate = ( m_pToolBox->GetItemState( m_nItemId ) == STATE_DONTKNOW );
    ToolBoxItemType eType = m_pToolBox->GetItemType( (sal_uInt16)m_nIndexInParent );
    switch ( eType )
    {
        case TOOLBOXITEM_BUTTON :
        {
            ToolBoxItemBits nBits = m_pToolBox->GetItemBits( m_nItemId );
            if (( nBits & TIB_DROPDOWN ) == TIB_DROPDOWN)
                m_nRole = AccessibleRole::BUTTON_DROPDOWN;
            else if (( ( nBits & TIB_CHECKABLE ) == TIB_CHECKABLE ) ||
                ( ( nBits & TIB_AUTOCHECK ) == TIB_AUTOCHECK ) )
                m_nRole = AccessibleRole::TOGGLE_BUTTON;
            else if ( m_pToolBox->GetItemWindow( m_nItemId ) )
                m_nRole = AccessibleRole::PANEL;
            break;
        }

        case TOOLBOXITEM_SPACE :
            m_nRole = AccessibleRole::FILLER;
            break;

        case TOOLBOXITEM_SEPARATOR :
        case TOOLBOXITEM_BREAK :
            m_nRole = AccessibleRole::SEPARATOR;
            break;

        default:
        {
            SAL_WARN( "accessibility", "unsupported toolbox itemtype" );
        }
    }
}
// -----------------------------------------------------------------------------
VCLXAccessibleToolBoxItem::~VCLXAccessibleToolBoxItem()
{
    delete m_pExternalLock;
    m_pExternalLock = NULL;
}
// -----------------------------------------------------------------------------
OUString VCLXAccessibleToolBoxItem::GetText( bool _bAsName )
{
    OUString sRet;
    // no text for separators and spaces
    if ( m_pToolBox && m_nItemId > 0 && ( _bAsName || m_pToolBox->GetButtonType() != BUTTON_SYMBOL ) )
    {
        sRet = m_pToolBox->GetItemText( m_nItemId );
    }
    return sRet;
}
// -----------------------------------------------------------------------------
void VCLXAccessibleToolBoxItem::SetFocus( sal_Bool _bFocus )
{
    if ( m_bHasFocus != _bFocus )
    {
        Any aOldValue;
        Any aNewValue;
        if ( m_bHasFocus )
            aOldValue <<= AccessibleStateType::FOCUSED;
        else
            aNewValue <<= AccessibleStateType::FOCUSED;
        m_bHasFocus = _bFocus;
         NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    }
}
// -----------------------------------------------------------------------------
void VCLXAccessibleToolBoxItem::SetChecked( sal_Bool _bCheck )
{
    if ( m_bIsChecked != _bCheck )
    {
        Any aOldValue;
        Any aNewValue;
        if ( m_bIsChecked )
            aOldValue <<= AccessibleStateType::CHECKED;
        else
            aNewValue <<= AccessibleStateType::CHECKED;
        m_bIsChecked = _bCheck;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    }
}
// -----------------------------------------------------------------------------
void VCLXAccessibleToolBoxItem::SetIndeterminate( bool _bIndeterminate )
{
    if ( m_bIndeterminate != _bIndeterminate )
    {
        Any aOldValue, aNewValue;
        if ( m_bIndeterminate )
            aOldValue <<= AccessibleStateType::INDETERMINATE;
        else
            aNewValue <<= AccessibleStateType::INDETERMINATE;
        m_bIndeterminate = _bIndeterminate;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    }
}
// -----------------------------------------------------------------------------
void VCLXAccessibleToolBoxItem::NameChanged()
{
    OUString sNewName = implGetText();
    if ( sNewName != m_sOldName )
    {
        Any aOldValue, aNewValue;
        aOldValue <<= m_sOldName;
        // save new name as old name for next change
        m_sOldName = sNewName;
        aNewValue <<= m_sOldName;
        NotifyAccessibleEvent( AccessibleEventId::NAME_CHANGED, aOldValue, aNewValue );
    }
}
// -----------------------------------------------------------------------------
void VCLXAccessibleToolBoxItem::SetChild( const Reference< XAccessible >& _xChild )
{
    m_xChild = _xChild;
}
// -----------------------------------------------------------------------------
void VCLXAccessibleToolBoxItem::NotifyChildEvent( const Reference< XAccessible >& _xChild, bool _bShow )
{
    Any aOld = _bShow ? Any() : makeAny( _xChild );
    Any aNew = _bShow ? makeAny( _xChild ) : Any();
    NotifyAccessibleEvent( AccessibleEventId::CHILD, aOld, aNew );
}
// -----------------------------------------------------------------------------
void VCLXAccessibleToolBoxItem::ToggleEnableState()
{
    Any aOldValue[2], aNewValue[2];
    if ( m_pToolBox->IsItemEnabled( m_nItemId ) )
    {
        aNewValue[0] <<= AccessibleStateType::SENSITIVE;
        aNewValue[1] <<= AccessibleStateType::ENABLED;
    }
    else
    {
        aOldValue[0] <<= AccessibleStateType::ENABLED;
        aOldValue[1] <<= AccessibleStateType::SENSITIVE;
    }

    NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue[0], aNewValue[0] );
    NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue[1], aNewValue[1] );
}
// -----------------------------------------------------------------------------
awt::Rectangle SAL_CALL VCLXAccessibleToolBoxItem::implGetBounds(  ) throw (RuntimeException)
{
    awt::Rectangle aRect;
    if ( m_pToolBox )
        aRect = AWTRectangle( m_pToolBox->GetItemPosRect( (sal_uInt16)m_nIndexInParent ) );

    return aRect;
}
// -----------------------------------------------------------------------------
OUString VCLXAccessibleToolBoxItem::implGetText()
{
    return GetText (true);
}
// -----------------------------------------------------------------------------
Locale VCLXAccessibleToolBoxItem::implGetLocale()
{
    return Application::GetSettings().GetUILocale();
}
// -----------------------------------------------------------------------------
void VCLXAccessibleToolBoxItem::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
{
    nStartIndex = 0;
    nEndIndex = 0;
}
// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_REFCOUNT( VCLXAccessibleToolBoxItem, AccessibleTextHelper_BASE )
Any SAL_CALL VCLXAccessibleToolBoxItem::queryInterface( const Type& _rType ) throw (RuntimeException)
{
    // #i33611# - toolbox buttons without text don't support XAccessibleText
    if ( _rType == ::getCppuType( ( const Reference< XAccessibleText >* ) 0 )
        && ( !m_pToolBox || m_pToolBox->GetButtonType() == BUTTON_SYMBOL ) )
        return Any();

    ::com::sun::star::uno::Any aReturn = AccessibleTextHelper_BASE::queryInterface( _rType );
    if ( !aReturn.hasValue() )
        aReturn = VCLXAccessibleToolBoxItem_BASE::queryInterface( _rType );
    return aReturn;
}
// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------
IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleToolBoxItem, AccessibleTextHelper_BASE, VCLXAccessibleToolBoxItem_BASE )
// -----------------------------------------------------------------------------
// XComponent
// -----------------------------------------------------------------------------
void SAL_CALL VCLXAccessibleToolBoxItem::disposing()
{
    AccessibleTextHelper_BASE::disposing();
    m_pToolBox = NULL;
}
// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------
OUString VCLXAccessibleToolBoxItem::getImplementationName() throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleToolBoxItem" );
}
// -----------------------------------------------------------------------------
sal_Bool VCLXAccessibleToolBoxItem::supportsService( const OUString& rServiceName ) throw (RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}
// -----------------------------------------------------------------------------
Sequence< OUString > VCLXAccessibleToolBoxItem::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< OUString > aNames(4);
    aNames[0] = "com.sun.star.accessibility.AccessibleContext";
    aNames[1] = "com.sun.star.accessibility.AccessibleComponent";
    aNames[2] = "com.sun.star.accessibility.AccessibleExtendedComponent";
    aNames[3] = "com.sun.star.accessibility.AccessibleToolBoxItem";
    return aNames;
}
// -----------------------------------------------------------------------------
// XAccessible
// -----------------------------------------------------------------------------
Reference< XAccessibleContext > SAL_CALL VCLXAccessibleToolBoxItem::getAccessibleContext(  ) throw (RuntimeException)
{
    return this;
}
// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL VCLXAccessibleToolBoxItem::getAccessibleChildCount(  ) throw (RuntimeException)
{
    OContextEntryGuard aGuard( this );

    return m_xChild.is() ? 1 : 0;
}
// -----------------------------------------------------------------------------
Reference< XAccessible > SAL_CALL VCLXAccessibleToolBoxItem::getAccessibleChild( sal_Int32 i ) throw (RuntimeException,  com::sun::star::lang::IndexOutOfBoundsException)
{
    OContextEntryGuard aGuard( this );

    // no child -> so index is out of bounds
    if ( !m_xChild.is() || i != 0 )
        throw IndexOutOfBoundsException();

    return m_xChild;
}
// -----------------------------------------------------------------------------
Reference< XAccessible > SAL_CALL VCLXAccessibleToolBoxItem::getAccessibleParent(  ) throw (RuntimeException)
{
    OContextEntryGuard aGuard( this );

    return m_pToolBox->GetAccessible();
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL VCLXAccessibleToolBoxItem::getAccessibleIndexInParent(  ) throw (RuntimeException)
{
    OContextEntryGuard aGuard( this );

    return m_nIndexInParent;
}
// -----------------------------------------------------------------------------
sal_Int16 SAL_CALL VCLXAccessibleToolBoxItem::getAccessibleRole(  ) throw (RuntimeException)
{
    OContextEntryGuard aGuard( this );

    return m_nRole;
}
// -----------------------------------------------------------------------------
OUString SAL_CALL VCLXAccessibleToolBoxItem::getAccessibleDescription(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    OUString sDescription;
    if ( m_pToolBox )
        sDescription = m_pToolBox->GetHelpText( m_nItemId );

    return sDescription;
}
// -----------------------------------------------------------------------------
OUString SAL_CALL VCLXAccessibleToolBoxItem::getAccessibleName(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    // entry text == accessible name
    return GetText( true );
}
// -----------------------------------------------------------------------------
Reference< XAccessibleRelationSet > SAL_CALL VCLXAccessibleToolBoxItem::getAccessibleRelationSet(  ) throw (RuntimeException)
{
    OContextEntryGuard aGuard( this );

    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
    return xSet;
}
// -----------------------------------------------------------------------------
Reference< XAccessibleStateSet > SAL_CALL VCLXAccessibleToolBoxItem::getAccessibleStateSet(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
    Reference< XAccessibleStateSet > xStateSet = pStateSetHelper;

    if ( m_pToolBox && !rBHelper.bDisposed && !rBHelper.bInDispose )
    {
        pStateSetHelper->AddState( AccessibleStateType::FOCUSABLE );
        if ( m_bIsChecked )
            pStateSetHelper->AddState( AccessibleStateType::CHECKED );
        if ( m_bIndeterminate )
            pStateSetHelper->AddState( AccessibleStateType::INDETERMINATE );
        if ( m_pToolBox->IsItemEnabled( m_nItemId ) )
        {
            pStateSetHelper->AddState( AccessibleStateType::ENABLED );
            pStateSetHelper->AddState( AccessibleStateType::SENSITIVE );
        }
        if ( m_pToolBox->IsItemVisible( m_nItemId ) )
            pStateSetHelper->AddState( AccessibleStateType::VISIBLE );
        if ( m_pToolBox->IsItemReallyVisible( m_nItemId ) )
            pStateSetHelper->AddState( AccessibleStateType::SHOWING );
        if ( m_bHasFocus )
            pStateSetHelper->AddState( AccessibleStateType::FOCUSED );
    }
    else
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

    return xStateSet;
}
// -----------------------------------------------------------------------------
// XAccessibleText
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL VCLXAccessibleToolBoxItem::getCaretPosition() throw (RuntimeException)
{
    return -1;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL VCLXAccessibleToolBoxItem::setCaretPosition( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidRange( nIndex, nIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    return sal_False;
}
// -----------------------------------------------------------------------------
Sequence< PropertyValue > SAL_CALL VCLXAccessibleToolBoxItem::getCharacterAttributes( sal_Int32 nIndex, const Sequence< OUString >& ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    OUString sText( implGetText() );

    if ( !implIsValidIndex( nIndex, sText.getLength() ) )
        throw IndexOutOfBoundsException();

    return Sequence< PropertyValue >();
}
// -----------------------------------------------------------------------------
awt::Rectangle SAL_CALL VCLXAccessibleToolBoxItem::getCharacterBounds( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    OUString sText( implGetText() );

    if ( !implIsValidIndex( nIndex, sText.getLength() ) )
        throw IndexOutOfBoundsException();

    awt::Rectangle aBounds( 0, 0, 0, 0 );
    if ( m_pToolBox && m_pToolBox->GetButtonType() != BUTTON_SYMBOL ) // symbol buttons have no character bounds
    {
        Rectangle aCharRect = m_pToolBox->GetCharacterBounds( m_nItemId, nIndex );
        Rectangle aItemRect = m_pToolBox->GetItemRect( m_nItemId );
        aCharRect.Move( -aItemRect.Left(), -aItemRect.Top() );
        aBounds = AWTRectangle( aCharRect );
    }

    return aBounds;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL VCLXAccessibleToolBoxItem::getIndexAtPoint( const awt::Point& aPoint ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nIndex = -1;
    if ( m_pToolBox && m_pToolBox->GetButtonType() != BUTTON_SYMBOL ) // symbol buttons have no character bounds
    {
        sal_uInt16 nItemId = 0;
        Rectangle aItemRect = m_pToolBox->GetItemRect( m_nItemId );
        Point aPnt( VCLPoint( aPoint ) );
        aPnt += aItemRect.TopLeft();
        sal_Int32 nIdx = m_pToolBox->GetIndexForPoint( aPnt, nItemId );
        if ( nIdx != -1 && nItemId == m_nItemId )
            nIndex = nIdx;
    }

    return nIndex;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL VCLXAccessibleToolBoxItem::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    return sal_False;
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL VCLXAccessibleToolBoxItem::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    sal_Bool bReturn = sal_False;

    if ( m_pToolBox )
    {
        Reference< datatransfer::clipboard::XClipboard > xClipboard = m_pToolBox->GetClipboard();
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

    return bReturn;
}
// -----------------------------------------------------------------------------
// XAccessibleComponent
// -----------------------------------------------------------------------------
Reference< XAccessible > SAL_CALL VCLXAccessibleToolBoxItem::getAccessibleAtPoint( const awt::Point& ) throw (RuntimeException)
{
    return Reference< XAccessible >();
}
// -----------------------------------------------------------------------------
void SAL_CALL VCLXAccessibleToolBoxItem::grabFocus(  ) throw (RuntimeException)
{
    Reference< XAccessible > xParent(getAccessibleParent());

    if( xParent.is() )
    {
        Reference< XAccessibleSelection > rxAccessibleSelection(xParent->getAccessibleContext(), UNO_QUERY);

        if ( rxAccessibleSelection.is() )
        {
            rxAccessibleSelection -> selectAccessibleChild ( getAccessibleIndexInParent() );
        }
    }
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL VCLXAccessibleToolBoxItem::getForeground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    if ( m_pToolBox )
       nColor = m_pToolBox->GetControlForeground().GetColor();

    return nColor;
}
// -----------------------------------------------------------------------------
sal_Int32 SAL_CALL VCLXAccessibleToolBoxItem::getBackground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    if ( m_pToolBox )
       nColor = m_pToolBox->GetControlBackground().GetColor();

    return nColor;
}
// -----------------------------------------------------------------------------
// XAccessibleExtendedComponent
// -----------------------------------------------------------------------------
Reference< awt::XFont > SAL_CALL VCLXAccessibleToolBoxItem::getFont(    ) throw (RuntimeException)
{
    return uno::Reference< awt::XFont >();
}
// -----------------------------------------------------------------------------
awt::FontDescriptor SAL_CALL VCLXAccessibleToolBoxItem::getFontMetrics( const Reference< awt::XFont >& xFont ) throw (RuntimeException)
{
    return xFont->getFontDescriptor();
}
// -----------------------------------------------------------------------------
OUString SAL_CALL VCLXAccessibleToolBoxItem::getTitledBorderText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    OUString sRet;
    if ( m_pToolBox )
        sRet = m_pToolBox->GetItemText( m_nItemId );

    return sRet;
}
// -----------------------------------------------------------------------------
OUString SAL_CALL VCLXAccessibleToolBoxItem::getToolTipText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    OUString sRet;
    if ( m_pToolBox )
    {
        if ( Help::IsExtHelpEnabled() )
            sRet = m_pToolBox->GetHelpText( m_nItemId );
        else
            sRet = m_pToolBox->GetQuickHelpText( m_nItemId );
        if ( sRet.isEmpty() )
            // no help text set, so use item text
            sRet = m_pToolBox->GetItemText( m_nItemId );
    }
    return sRet;
}
// -----------------------------------------------------------------------------
// XAccessibleAction
// -----------------------------------------------------------------------------
sal_Int32 VCLXAccessibleToolBoxItem::getAccessibleActionCount( ) throw (RuntimeException)
{
    // only one action -> "Click"
    return 1;
}
// -----------------------------------------------------------------------------
sal_Bool VCLXAccessibleToolBoxItem::doAccessibleAction ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    if ( m_pToolBox )
        m_pToolBox->TriggerItem( m_nItemId );

    return sal_True;
}
// -----------------------------------------------------------------------------
OUString VCLXAccessibleToolBoxItem::getAccessibleActionDescription ( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return OUString( TK_RES_STRING( RID_STR_ACC_ACTION_CLICK ) );
}
// -----------------------------------------------------------------------------
Reference< XAccessibleKeyBinding > VCLXAccessibleToolBoxItem::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OContextEntryGuard aGuard( this );

    if ( nIndex < 0 || nIndex >= getAccessibleActionCount() )
        throw IndexOutOfBoundsException();

    return Reference< XAccessibleKeyBinding >();
}
// -----------------------------------------------------------------------------
// XAccessibleValue
// -----------------------------------------------------------------------------
Any VCLXAccessibleToolBoxItem::getCurrentValue(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Any aValue;
    if ( m_pToolBox )
        aValue <<= (sal_Int32)m_pToolBox->IsItemChecked( m_nItemId );

    return aValue;
}
// -----------------------------------------------------------------------------
sal_Bool VCLXAccessibleToolBoxItem::setCurrentValue( const Any& aNumber ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Bool bReturn = sal_False;

    if ( m_pToolBox )
    {
        sal_Int32 nValue = 0;
        OSL_VERIFY( aNumber >>= nValue );

        if ( nValue < 0 )
            nValue = 0;
        else if ( nValue > 1 )
            nValue = 1;

        m_pToolBox->CheckItem( m_nItemId, (sal_Bool) nValue );
        bReturn = sal_True;
    }

    return bReturn;
}
// -----------------------------------------------------------------------------
Any VCLXAccessibleToolBoxItem::getMaximumValue(  ) throw (RuntimeException)
{
    return makeAny((sal_Int32)1);
}
// -----------------------------------------------------------------------------
Any VCLXAccessibleToolBoxItem::getMinimumValue(  ) throw (RuntimeException)
{
    return makeAny((sal_Int32)0);
}
// -----------------------------------------------------------------------------


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
