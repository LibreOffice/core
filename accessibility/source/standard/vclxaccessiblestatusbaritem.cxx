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

#include <accessibility/standard/vclxaccessiblestatusbaritem.hxx>
#include <toolkit/helper/externallock.hxx>
#include <toolkit/helper/convert.hxx>
#include <accessibility/helper/characterattributeshelper.hxx>

#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XFlushableClipboard.hpp>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp2.hxx>
#include <vcl/status.hxx>
#include <vcl/controllayout.hxx>

#include <memory>


using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;
using namespace ::comphelper;


// -----------------------------------------------------------------------------
// class VCLXAccessibleStatusBarItem
// -----------------------------------------------------------------------------

VCLXAccessibleStatusBarItem::VCLXAccessibleStatusBarItem( StatusBar* pStatusBar, sal_uInt16 nItemId )
    :AccessibleTextHelper_BASE( new VCLExternalSolarLock() )
    ,m_pStatusBar( pStatusBar )
    ,m_nItemId( nItemId )
{
    m_pExternalLock = static_cast< VCLExternalSolarLock* >( getExternalLock() );

    m_sItemName = GetItemName();
    m_sItemText = GetItemText();
    m_bShowing  = IsShowing();
}

// -----------------------------------------------------------------------------

VCLXAccessibleStatusBarItem::~VCLXAccessibleStatusBarItem()
{
    delete m_pExternalLock;
    m_pExternalLock = NULL;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleStatusBarItem::IsShowing()
{
    sal_Bool bShowing = sal_False;

    if ( m_pStatusBar )
        bShowing = m_pStatusBar->IsItemVisible( m_nItemId );

    return bShowing;
}

// -----------------------------------------------------------------------------

void VCLXAccessibleStatusBarItem::SetShowing( sal_Bool bShowing )
{
    if ( m_bShowing != bShowing )
    {
        Any aOldValue, aNewValue;
        if ( m_bShowing )
            aOldValue <<= AccessibleStateType::SHOWING;
        else
            aNewValue <<= AccessibleStateType::SHOWING;
        m_bShowing = bShowing;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    }
}

// -----------------------------------------------------------------------------

void VCLXAccessibleStatusBarItem::SetItemName( const OUString& sItemName )
{
    if ( !m_sItemName.equals( sItemName ) )
    {
        Any aOldValue, aNewValue;
        aOldValue <<= m_sItemName;
        aNewValue <<= sItemName;
        m_sItemName = sItemName;
        NotifyAccessibleEvent( AccessibleEventId::NAME_CHANGED, aOldValue, aNewValue );
    }
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleStatusBarItem::GetItemName()
{
    OUString sName;
    if ( m_pStatusBar )
        sName = m_pStatusBar->GetAccessibleName( m_nItemId );

    return sName;
}

// -----------------------------------------------------------------------------

void VCLXAccessibleStatusBarItem::SetItemText( const OUString& sItemText )
{
    Any aOldValue, aNewValue;
    if ( implInitTextChangedEvent( m_sItemText, sItemText, aOldValue, aNewValue ) )
    {
        m_sItemText = sItemText;
        NotifyAccessibleEvent( AccessibleEventId::TEXT_CHANGED, aOldValue, aNewValue );
    }
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleStatusBarItem::GetItemText()
{
    OUString sText;
    ::vcl::ControlLayoutData aLayoutData;
    if ( m_pStatusBar )
    {
        Rectangle aItemRect = m_pStatusBar->GetItemRect( m_nItemId );
        m_pStatusBar->RecordLayoutData( &aLayoutData, aItemRect );
        sText = aLayoutData.m_aDisplayText;
    }

    return sText;
}

// -----------------------------------------------------------------------------

void VCLXAccessibleStatusBarItem::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    rStateSet.AddState( AccessibleStateType::ENABLED );
    rStateSet.AddState( AccessibleStateType::SENSITIVE );

    rStateSet.AddState( AccessibleStateType::VISIBLE );

    if ( IsShowing() )
        rStateSet.AddState( AccessibleStateType::SHOWING );
}

// -----------------------------------------------------------------------------
// OCommonAccessibleComponent
// -----------------------------------------------------------------------------

awt::Rectangle VCLXAccessibleStatusBarItem::implGetBounds() throw (RuntimeException)
{
    awt::Rectangle aBounds( 0, 0, 0, 0 );

    if ( m_pStatusBar )
        aBounds = AWTRectangle( m_pStatusBar->GetItemRect( m_nItemId ) );

    return aBounds;
}

// -----------------------------------------------------------------------------
// OCommonAccessibleText
// -----------------------------------------------------------------------------

OUString VCLXAccessibleStatusBarItem::implGetText()
{
    return GetItemText();
}

// -----------------------------------------------------------------------------

lang::Locale VCLXAccessibleStatusBarItem::implGetLocale()
{
    return Application::GetSettings().GetLocale();
}

// -----------------------------------------------------------------------------

void VCLXAccessibleStatusBarItem::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
{
    nStartIndex = 0;
    nEndIndex = 0;
}

// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleStatusBarItem, AccessibleTextHelper_BASE, VCLXAccessibleStatusBarItem_BASE )

// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleStatusBarItem, AccessibleTextHelper_BASE, VCLXAccessibleStatusBarItem_BASE )

// -----------------------------------------------------------------------------
// XComponent
// -----------------------------------------------------------------------------

void VCLXAccessibleStatusBarItem::disposing()
{
    AccessibleTextHelper_BASE::disposing();

    m_pStatusBar = NULL;
    m_sItemName = OUString();
    m_sItemText = OUString();
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

OUString VCLXAccessibleStatusBarItem::getImplementationName() throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleStatusBarItem" );
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleStatusBarItem::supportsService( const OUString& rServiceName ) throw (RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}

// -----------------------------------------------------------------------------

Sequence< OUString > VCLXAccessibleStatusBarItem::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< OUString > aNames(1);
    aNames[0] = "com.sun.star.awt.AccessibleStatusBarItem";
    return aNames;
}

// -----------------------------------------------------------------------------
// XAccessible
// -----------------------------------------------------------------------------

Reference< XAccessibleContext > VCLXAccessibleStatusBarItem::getAccessibleContext(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return this;
}

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleStatusBarItem::getAccessibleChildCount() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return 0;
}

// -----------------------------------------------------------------------------

Reference< XAccessible > VCLXAccessibleStatusBarItem::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    return Reference< XAccessible >();
}

// -----------------------------------------------------------------------------

Reference< XAccessible > VCLXAccessibleStatusBarItem::getAccessibleParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Reference< XAccessible > xParent;
    if ( m_pStatusBar )
        xParent = m_pStatusBar->GetAccessible();

    return xParent;
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleStatusBarItem::getAccessibleIndexInParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nIndexInParent = -1;
    if ( m_pStatusBar )
        nIndexInParent = m_pStatusBar->GetItemPos( m_nItemId );

    return nIndexInParent;
}

// -----------------------------------------------------------------------------

sal_Int16 VCLXAccessibleStatusBarItem::getAccessibleRole(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::LABEL;
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleStatusBarItem::getAccessibleDescription(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    OUString sDescription;
    if ( m_pStatusBar )
        sDescription = m_pStatusBar->GetHelpText( m_nItemId );

    return sDescription;
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleStatusBarItem::getAccessibleName(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return GetItemName();
}

// -----------------------------------------------------------------------------

Reference< XAccessibleRelationSet > VCLXAccessibleStatusBarItem::getAccessibleRelationSet(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
    return xSet;
}

// -----------------------------------------------------------------------------

Reference< XAccessibleStateSet > VCLXAccessibleStatusBarItem::getAccessibleStateSet(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
    Reference< XAccessibleStateSet > xSet = pStateSetHelper;

    if ( !rBHelper.bDisposed && !rBHelper.bInDispose )
    {
        FillAccessibleStateSet( *pStateSetHelper );
    }
    else
    {
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );
    }

    return xSet;
}

// -----------------------------------------------------------------------------

Locale VCLXAccessibleStatusBarItem::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetLocale();
}

// -----------------------------------------------------------------------------
// XAccessibleComponent
// -----------------------------------------------------------------------------

Reference< XAccessible > VCLXAccessibleStatusBarItem::getAccessibleAtPoint( const awt::Point& ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Reference< XAccessible >();
}

// -----------------------------------------------------------------------------

void VCLXAccessibleStatusBarItem::grabFocus(  ) throw (RuntimeException)
{
    // no focus for status bar items
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleStatusBarItem::getForeground(   ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    Reference< XAccessible > xParent = getAccessibleParent();
    if ( xParent.is() )
    {
        Reference< XAccessibleComponent > xParentComp( xParent->getAccessibleContext(), UNO_QUERY );
        if ( xParentComp.is() )
            nColor = xParentComp->getForeground();
    }

    return nColor;
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleStatusBarItem::getBackground(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nColor = 0;
    Reference< XAccessible > xParent = getAccessibleParent();
    if ( xParent.is() )
    {
        Reference< XAccessibleComponent > xParentComp( xParent->getAccessibleContext(), UNO_QUERY );
        if ( xParentComp.is() )
            nColor = xParentComp->getBackground();
    }

    return nColor;
}

// -----------------------------------------------------------------------------
// XAccessibleExtendedComponent
// -----------------------------------------------------------------------------

Reference< awt::XFont > VCLXAccessibleStatusBarItem::getFont(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Reference< awt::XFont > xFont;
    Reference< XAccessible > xParent = getAccessibleParent();
    if ( xParent.is() )
    {
        Reference< XAccessibleExtendedComponent > xParentComp( xParent->getAccessibleContext(), UNO_QUERY );
        if ( xParentComp.is() )
            xFont = xParentComp->getFont();
    }

    return xFont;
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleStatusBarItem::getTitledBorderText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return GetItemText();
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleStatusBarItem::getToolTipText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OUString();
}

// -----------------------------------------------------------------------------
// XAccessibleText
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleStatusBarItem::getCaretPosition() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return -1;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleStatusBarItem::setCaretPosition( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidRange( nIndex, nIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    return sal_False;
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > VCLXAccessibleStatusBarItem::getCharacterAttributes( sal_Int32 nIndex, const Sequence< OUString >& aRequestedAttributes ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Sequence< PropertyValue > aValues;
    OUString sText( implGetText() );

    if ( !implIsValidIndex( nIndex, sText.getLength() ) )
        throw IndexOutOfBoundsException();

    if ( m_pStatusBar )
    {
        Font aFont = m_pStatusBar->GetFont();
        sal_Int32 nBackColor = getBackground();
        sal_Int32 nColor = getForeground();
        ::std::auto_ptr< CharacterAttributesHelper > pHelper( new CharacterAttributesHelper( aFont, nBackColor, nColor ) );
        aValues = pHelper->GetCharacterAttributes( aRequestedAttributes );
    }

    return aValues;
}

// -----------------------------------------------------------------------------

awt::Rectangle VCLXAccessibleStatusBarItem::getCharacterBounds( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidIndex( nIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    awt::Rectangle aBounds( 0, 0, 0, 0 );
    if ( m_pStatusBar )
    {
        ::vcl::ControlLayoutData aLayoutData;
        Rectangle aItemRect = m_pStatusBar->GetItemRect( m_nItemId );
        m_pStatusBar->RecordLayoutData( &aLayoutData, aItemRect );
        Rectangle aCharRect = aLayoutData.GetCharacterBounds( nIndex );
        aCharRect.Move( -aItemRect.Left(), -aItemRect.Top() );
        aBounds = AWTRectangle( aCharRect );
    }

    return aBounds;
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleStatusBarItem::getIndexAtPoint( const awt::Point& aPoint ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nIndex = -1;
    if ( m_pStatusBar )
    {
        ::vcl::ControlLayoutData aLayoutData;
        Rectangle aItemRect = m_pStatusBar->GetItemRect( m_nItemId );
        m_pStatusBar->RecordLayoutData( &aLayoutData, aItemRect );
        Point aPnt( VCLPoint( aPoint ) );
        aPnt += aItemRect.TopLeft();
        nIndex = aLayoutData.GetIndexForPoint( aPnt );
    }

    return nIndex;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleStatusBarItem::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    return sal_False;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleStatusBarItem::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Bool bReturn = sal_False;

    if ( m_pStatusBar )
    {
        Reference< datatransfer::clipboard::XClipboard > xClipboard = m_pStatusBar->GetClipboard();
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
