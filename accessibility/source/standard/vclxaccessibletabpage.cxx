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

#include <accessibility/standard/vclxaccessibletabpage.hxx>
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
#include <vcl/tabctrl.hxx>
#include <vcl/tabpage.hxx>

#include <memory>


using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star;
using namespace ::comphelper;


// -----------------------------------------------------------------------------
// class VCLXAccessibleTabPage
// -----------------------------------------------------------------------------

VCLXAccessibleTabPage::VCLXAccessibleTabPage( TabControl* pTabControl, sal_uInt16 nPageId )
    :AccessibleTextHelper_BASE( new VCLExternalSolarLock() )
    ,m_pTabControl( pTabControl )
    ,m_nPageId( nPageId )
{
    m_pExternalLock = static_cast< VCLExternalSolarLock* >( getExternalLock() );
    m_bFocused  = IsFocused();
    m_bSelected = IsSelected();
    m_sPageText = GetPageText();
}

// -----------------------------------------------------------------------------

VCLXAccessibleTabPage::~VCLXAccessibleTabPage()
{
    delete m_pExternalLock;
    m_pExternalLock = NULL;
}

// -----------------------------------------------------------------------------

bool VCLXAccessibleTabPage::IsFocused()
{
    bool bFocused = false;

    if ( m_pTabControl && m_pTabControl->HasFocus() && m_pTabControl->GetCurPageId() == m_nPageId )
        bFocused = true;

    return bFocused;
}

// -----------------------------------------------------------------------------

bool VCLXAccessibleTabPage::IsSelected()
{
    bool bSelected = false;

    if ( m_pTabControl && m_pTabControl->GetCurPageId() == m_nPageId )
        bSelected = true;

    return bSelected;
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTabPage::SetFocused( bool bFocused )
{
    if ( m_bFocused != bFocused )
    {
        Any aOldValue, aNewValue;
        if ( m_bFocused )
            aOldValue <<= AccessibleStateType::FOCUSED;
        else
            aNewValue <<= AccessibleStateType::FOCUSED;
        m_bFocused = bFocused;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    }
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTabPage::SetSelected( bool bSelected )
{
    if ( m_bSelected != bSelected )
    {
        Any aOldValue, aNewValue;
        if ( m_bSelected )
            aOldValue <<= AccessibleStateType::SELECTED;
        else
            aNewValue <<= AccessibleStateType::SELECTED;
        m_bSelected = bSelected;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    }
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTabPage::SetPageText( const OUString& sPageText )
{
    Any aOldValue, aNewValue;
    if ( OCommonAccessibleText::implInitTextChangedEvent( m_sPageText, sPageText, aOldValue, aNewValue ) )
    {
        Any aOldName, aNewName;
        aOldName <<= m_sPageText;
        aNewName <<= sPageText;
        m_sPageText = sPageText;
        NotifyAccessibleEvent( AccessibleEventId::NAME_CHANGED, aOldName, aNewName );
        NotifyAccessibleEvent( AccessibleEventId::TEXT_CHANGED, aOldValue, aNewValue );
    }
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleTabPage::GetPageText()
{
    OUString sText;
    if ( m_pTabControl )
        sText = OutputDevice::GetNonMnemonicString( m_pTabControl->GetPageText( m_nPageId ) );

    return sText;
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTabPage::Update( bool bNew )
{
    if ( m_pTabControl )
    {
        TabPage* pTabPage = m_pTabControl->GetTabPage( m_nPageId );
        if ( pTabPage )
        {
            Reference< XAccessible > xChild( pTabPage->GetAccessible( bNew ) );
            if ( xChild.is() )
            {
                Any aOldValue, aNewValue;
                if ( bNew )
                    aNewValue <<= xChild;
                else
                    aOldValue <<= xChild;
                NotifyAccessibleEvent( AccessibleEventId::CHILD, aOldValue, aNewValue );
            }
        }
    }
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTabPage::FillAccessibleStateSet( utl::AccessibleStateSetHelper& rStateSet )
{
    rStateSet.AddState( AccessibleStateType::ENABLED );
    rStateSet.AddState( AccessibleStateType::SENSITIVE );

    rStateSet.AddState( AccessibleStateType::FOCUSABLE );

    if ( IsFocused() )
        rStateSet.AddState( AccessibleStateType::FOCUSED );

    rStateSet.AddState( AccessibleStateType::VISIBLE );

    rStateSet.AddState( AccessibleStateType::SHOWING );

    rStateSet.AddState( AccessibleStateType::SELECTABLE );

    if ( IsSelected() )
        rStateSet.AddState( AccessibleStateType::SELECTED );
}

// -----------------------------------------------------------------------------
// OCommonAccessibleComponent
// -----------------------------------------------------------------------------

awt::Rectangle VCLXAccessibleTabPage::implGetBounds() throw (RuntimeException)
{
    awt::Rectangle aBounds( 0, 0, 0, 0 );

    if ( m_pTabControl )
        aBounds = AWTRectangle( m_pTabControl->GetTabBounds( m_nPageId ) );

    return aBounds;
}

// -----------------------------------------------------------------------------
// OCommonAccessibleText
// -----------------------------------------------------------------------------

OUString VCLXAccessibleTabPage::implGetText()
{
    return GetPageText();
}

// -----------------------------------------------------------------------------

lang::Locale VCLXAccessibleTabPage::implGetLocale()
{
    return Application::GetSettings().GetLocale();
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTabPage::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
{
    nStartIndex = 0;
    nEndIndex = 0;
}

// -----------------------------------------------------------------------------
// XInterface
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XINTERFACE2( VCLXAccessibleTabPage, AccessibleTextHelper_BASE, VCLXAccessibleTabPage_BASE )

// -----------------------------------------------------------------------------
// XTypeProvider
// -----------------------------------------------------------------------------

IMPLEMENT_FORWARD_XTYPEPROVIDER2( VCLXAccessibleTabPage, AccessibleTextHelper_BASE, VCLXAccessibleTabPage_BASE )

// -----------------------------------------------------------------------------
// XComponent
// -----------------------------------------------------------------------------

void VCLXAccessibleTabPage::disposing()
{
    AccessibleTextHelper_BASE::disposing();

    m_pTabControl = NULL;
    m_sPageText = OUString();
}

// -----------------------------------------------------------------------------
// XServiceInfo
// -----------------------------------------------------------------------------

OUString VCLXAccessibleTabPage::getImplementationName() throw (RuntimeException)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleTabPage" );
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleTabPage::supportsService( const OUString& rServiceName ) throw (RuntimeException)
{
    return cppu::supportsService(this, rServiceName);
}

// -----------------------------------------------------------------------------

Sequence< OUString > VCLXAccessibleTabPage::getSupportedServiceNames() throw (RuntimeException)
{
    Sequence< OUString > aNames(1);
    aNames[0] = "com.sun.star.awt.AccessibleTabPage";
    return aNames;
}

// -----------------------------------------------------------------------------
// XAccessible
// -----------------------------------------------------------------------------

Reference< XAccessibleContext > VCLXAccessibleTabPage::getAccessibleContext(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return this;
}

// -----------------------------------------------------------------------------
// XAccessibleContext
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTabPage::getAccessibleChildCount() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nCount = 0;
    if ( m_pTabControl )
    {
        TabPage* pTabPage = m_pTabControl->GetTabPage( m_nPageId );
        if ( pTabPage && pTabPage->IsVisible() )
            nCount = 1;
    }

    return nCount;
}

// -----------------------------------------------------------------------------

Reference< XAccessible > VCLXAccessibleTabPage::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( i < 0 || i >= getAccessibleChildCount() )
        throw IndexOutOfBoundsException();

    Reference< XAccessible > xChild;
    if ( m_pTabControl )
    {
        TabPage* pTabPage = m_pTabControl->GetTabPage( m_nPageId );
        if ( pTabPage && pTabPage->IsVisible() )
            xChild = pTabPage->GetAccessible();
    }

    return xChild;
}

// -----------------------------------------------------------------------------

Reference< XAccessible > VCLXAccessibleTabPage::getAccessibleParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Reference< XAccessible > xParent;
    if ( m_pTabControl )
        xParent = m_pTabControl->GetAccessible();

    return xParent;
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTabPage::getAccessibleIndexInParent(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nIndexInParent = -1;
    if ( m_pTabControl )
        nIndexInParent = m_pTabControl->GetPagePos( m_nPageId );

    return nIndexInParent;
}

// -----------------------------------------------------------------------------

sal_Int16 VCLXAccessibleTabPage::getAccessibleRole(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return AccessibleRole::PAGE_TAB;
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleTabPage::getAccessibleDescription(    ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    OUString sDescription;
    if ( m_pTabControl )
        sDescription = m_pTabControl->GetHelpText( m_nPageId );

    return sDescription;
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleTabPage::getAccessibleName(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return GetPageText();
}

// -----------------------------------------------------------------------------

Reference< XAccessibleRelationSet > VCLXAccessibleTabPage::getAccessibleRelationSet(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
    return xSet;
}

// -----------------------------------------------------------------------------

Reference< XAccessibleStateSet > VCLXAccessibleTabPage::getAccessibleStateSet(  ) throw (RuntimeException)
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

Locale VCLXAccessibleTabPage::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return Application::GetSettings().GetLocale();
}

// -----------------------------------------------------------------------------
// XAccessibleComponent
// -----------------------------------------------------------------------------

Reference< XAccessible > VCLXAccessibleTabPage::getAccessibleAtPoint( const awt::Point& rPoint ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Reference< XAccessible > xChild;
    for ( sal_uInt32 i = 0, nCount = getAccessibleChildCount(); i < nCount; ++i )
    {
        Reference< XAccessible > xAcc = getAccessibleChild( i );
        if ( xAcc.is() )
        {
            Reference< XAccessibleComponent > xComp( xAcc->getAccessibleContext(), UNO_QUERY );
            if ( xComp.is() )
            {
                Rectangle aRect = VCLRectangle( xComp->getBounds() );
                Point aPos = VCLPoint( rPoint );
                if ( aRect.IsInside( aPos ) )
                {
                    xChild = xAcc;
                    break;
                }
            }
        }
    }

    return xChild;
}

// -----------------------------------------------------------------------------

void VCLXAccessibleTabPage::grabFocus(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( m_pTabControl )
    {
        m_pTabControl->SelectTabPage( m_nPageId );
        m_pTabControl->GrabFocus();
    }
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTabPage::getForeground( ) throw (RuntimeException)
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

sal_Int32 VCLXAccessibleTabPage::getBackground(  ) throw (RuntimeException)
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

Reference< awt::XFont > VCLXAccessibleTabPage::getFont(  ) throw (RuntimeException)
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

OUString VCLXAccessibleTabPage::getTitledBorderText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OUString();
}

// -----------------------------------------------------------------------------

OUString VCLXAccessibleTabPage::getToolTipText(  ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return OUString();
}

// -----------------------------------------------------------------------------
// XAccessibleText
// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTabPage::getCaretPosition() throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    return -1;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleTabPage::setCaretPosition( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidRange( nIndex, nIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    return sal_False;
}

// -----------------------------------------------------------------------------

Sequence< PropertyValue > VCLXAccessibleTabPage::getCharacterAttributes( sal_Int32 nIndex, const Sequence< OUString >& aRequestedAttributes ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    Sequence< PropertyValue > aValues;
    OUString sText( implGetText() );

    if ( !implIsValidIndex( nIndex, sText.getLength() ) )
        throw IndexOutOfBoundsException();

    if ( m_pTabControl )
    {
        Font aFont = m_pTabControl->GetFont();
        sal_Int32 nBackColor = getBackground();
        sal_Int32 nColor = getForeground();
        ::std::auto_ptr< CharacterAttributesHelper > pHelper( new CharacterAttributesHelper( aFont, nBackColor, nColor ) );
        aValues = pHelper->GetCharacterAttributes( aRequestedAttributes );
    }

    return aValues;
}

// -----------------------------------------------------------------------------

awt::Rectangle VCLXAccessibleTabPage::getCharacterBounds( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidIndex( nIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    awt::Rectangle aBounds( 0, 0, 0, 0 );
    if ( m_pTabControl )
    {
        Rectangle aPageRect = m_pTabControl->GetTabBounds( m_nPageId );
        Rectangle aCharRect = m_pTabControl->GetCharacterBounds( m_nPageId, nIndex );
        aCharRect.Move( -aPageRect.Left(), -aPageRect.Top() );
        aBounds = AWTRectangle( aCharRect );
    }

    return aBounds;
}

// -----------------------------------------------------------------------------

sal_Int32 VCLXAccessibleTabPage::getIndexAtPoint( const awt::Point& aPoint ) throw (RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Int32 nIndex = -1;
    if ( m_pTabControl )
    {
        sal_uInt16 nPageId = 0;
        Rectangle aPageRect = m_pTabControl->GetTabBounds( m_nPageId );
        Point aPnt( VCLPoint( aPoint ) );
        aPnt += aPageRect.TopLeft();
        sal_Int32 nI = m_pTabControl->GetIndexForPoint( aPnt, nPageId );
        if ( nI != -1 && m_nPageId == nPageId )
            nIndex = nI;
    }

    return nIndex;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleTabPage::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    return sal_False;
}

// -----------------------------------------------------------------------------

sal_Bool VCLXAccessibleTabPage::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
{
    OExternalLockGuard aGuard( this );

    sal_Bool bReturn = sal_False;

    if ( m_pTabControl )
    {
        Reference< datatransfer::clipboard::XClipboard > xClipboard = m_pTabControl->GetClipboard();
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
