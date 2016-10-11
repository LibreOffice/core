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

#include <accessibility/standard/vclxaccessiblelistitem.hxx>
#include <toolkit/helper/convert.hxx>
#include <accessibility/helper/listboxhelper.hxx>
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
#include <vcl/controllayout.hxx>
#include <vcl/unohelp2.hxx>
#include <vcl/settings.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/accessibleeventnotifier.hxx>

namespace
{
    void checkIndex_Impl( sal_Int32 _nIndex, const OUString& _sText ) throw (css::lang::IndexOutOfBoundsException)
    {
        if ( _nIndex < 0 || _nIndex > _sText.getLength() )
            throw css::lang::IndexOutOfBoundsException();
    }
}

// class VCLXAccessibleListItem ------------------------------------------

using namespace ::com::sun::star::accessibility;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star;


// Ctor() and Dtor()

VCLXAccessibleListItem::VCLXAccessibleListItem(sal_Int32 _nIndexInParent, const rtl::Reference< VCLXAccessibleList >& _xParent)
    : VCLXAccessibleListItem_BASE(m_aMutex)
    , m_nIndexInParent(_nIndexInParent)
    , m_bSelected(false)
    , m_bVisible(false)
    , m_nClientId(0)
    , m_xParent(_xParent)
{
    if (m_xParent.is())
    {
        m_xParentContext = m_xParent->getAccessibleContext();
        ::accessibility::IComboListBoxHelper* pListBoxHelper = m_xParent->getListBoxHelper();
        if (pListBoxHelper)
            m_sEntryText = pListBoxHelper->GetEntry((sal_uInt16)_nIndexInParent);
    }
}

VCLXAccessibleListItem::~VCLXAccessibleListItem()
{
}

void VCLXAccessibleListItem::SetSelected( bool _bSelected )
{
    if ( m_bSelected != _bSelected )
    {
        Any aOldValue;
        Any aNewValue;
        if ( m_bSelected )
            aOldValue <<= AccessibleStateType::SELECTED;
        else
            aNewValue <<= AccessibleStateType::SELECTED;
        m_bSelected = _bSelected;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    }
}

void VCLXAccessibleListItem::SetVisible( bool _bVisible )
{
    if ( m_bVisible != _bVisible )
    {
        Any aOldValue, aNewValue;
        m_bVisible = _bVisible;
        (_bVisible ? aNewValue : aOldValue ) <<= AccessibleStateType::VISIBLE;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        (_bVisible ? aNewValue : aOldValue ) <<= AccessibleStateType::SHOWING;
        NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
    }
}

void VCLXAccessibleListItem::NotifyAccessibleEvent( sal_Int16 _nEventId,
                                                    const css::uno::Any& _aOldValue,
                                                    const css::uno::Any& _aNewValue )
{
    AccessibleEventObject aEvt;
    aEvt.Source = *this;
    aEvt.EventId = _nEventId;
    aEvt.OldValue = _aOldValue;
    aEvt.NewValue = _aNewValue;

    if (m_nClientId)
        comphelper::AccessibleEventNotifier::addEvent( m_nClientId, aEvt );
}

// OCommonAccessibleText

OUString VCLXAccessibleListItem::implGetText()
{
    return m_sEntryText;
}

Locale VCLXAccessibleListItem::implGetLocale()
{
    return Application::GetSettings().GetLanguageTag().getLocale();
}

void VCLXAccessibleListItem::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
{
    nStartIndex = 0;
    nEndIndex = 0;
}

// XInterface

Any SAL_CALL VCLXAccessibleListItem::queryInterface( Type const & rType ) throw (RuntimeException, std::exception)
{
    return VCLXAccessibleListItem_BASE::queryInterface( rType );
}

void SAL_CALL VCLXAccessibleListItem::acquire() throw ()
{
    VCLXAccessibleListItem_BASE::acquire();
}

void SAL_CALL VCLXAccessibleListItem::release() throw ()
{
    VCLXAccessibleListItem_BASE::release();
}

// XTypeProvider

Sequence< Type > SAL_CALL VCLXAccessibleListItem::getTypes(  ) throw (RuntimeException, std::exception)
{
    return VCLXAccessibleListItem_BASE::getTypes();
}

Sequence< sal_Int8 > VCLXAccessibleListItem::getImplementationId() throw (RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

// XComponent

void SAL_CALL VCLXAccessibleListItem::disposing()
{
    comphelper::AccessibleEventNotifier::TClientId nId( 0 );
    Reference< XInterface > xEventSource;
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        VCLXAccessibleListItem_BASE::disposing();
        m_sEntryText.clear();
        m_xParent           = nullptr;
        m_xParentContext    = nullptr;

        nId = m_nClientId;
        m_nClientId =  0;
        if ( nId )
            xEventSource = *this;
    }

    // Send a disposing to all listeners.
    if ( nId )
            comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( nId, *this );
}

// XServiceInfo

OUString VCLXAccessibleListItem::getImplementationName() throw (RuntimeException, std::exception)
{
    return OUString( "com.sun.star.comp.toolkit.AccessibleListItem" );
}

sal_Bool VCLXAccessibleListItem::supportsService( const OUString& rServiceName ) throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, rServiceName);
}

Sequence< OUString > VCLXAccessibleListItem::getSupportedServiceNames() throw (RuntimeException, std::exception)
{
    Sequence< OUString > aNames(3);
    aNames[0] = "com.sun.star.accessibility.AccessibleContext";
    aNames[1] = "com.sun.star.accessibility.AccessibleComponent";
    aNames[2] = "com.sun.star.accessibility.AccessibleListItem";
    return aNames;
}

// XAccessible

Reference< XAccessibleContext > SAL_CALL VCLXAccessibleListItem::getAccessibleContext(  ) throw (RuntimeException, std::exception)
{
    return this;
}

// XAccessibleContext

sal_Int32 SAL_CALL VCLXAccessibleListItem::getAccessibleChildCount(  ) throw (RuntimeException, std::exception)
{
    return 0;
}

Reference< XAccessible > SAL_CALL VCLXAccessibleListItem::getAccessibleChild( sal_Int32 ) throw (RuntimeException, std::exception)
{
    return Reference< XAccessible >();
}

Reference< XAccessible > SAL_CALL VCLXAccessibleListItem::getAccessibleParent(  ) throw (RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    return m_xParent.get();
}

sal_Int32 SAL_CALL VCLXAccessibleListItem::getAccessibleIndexInParent(  ) throw (RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return m_nIndexInParent;
}

sal_Int16 SAL_CALL VCLXAccessibleListItem::getAccessibleRole(  ) throw (RuntimeException, std::exception)
{
    return AccessibleRole::LIST_ITEM;
    //  return AccessibleRole::LABEL;
}

OUString SAL_CALL VCLXAccessibleListItem::getAccessibleDescription(  ) throw (RuntimeException, std::exception)
{
    // no description for every item
    return OUString();
}

OUString SAL_CALL VCLXAccessibleListItem::getAccessibleName(  ) throw (RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    // entry text == accessible name
    return implGetText();
}

Reference< XAccessibleRelationSet > SAL_CALL VCLXAccessibleListItem::getAccessibleRelationSet(  ) throw (RuntimeException, std::exception)
{
    utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
    Reference< XAccessibleRelationSet > xSet = pRelationSetHelper;
    return xSet;
}

Reference< XAccessibleStateSet > SAL_CALL VCLXAccessibleListItem::getAccessibleStateSet(  ) throw (RuntimeException, std::exception)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
    Reference< XAccessibleStateSet > xStateSet = pStateSetHelper;

    if ( !rBHelper.bDisposed && !rBHelper.bInDispose )
    {
        pStateSetHelper->AddState( AccessibleStateType::TRANSIENT );

        ::accessibility::IComboListBoxHelper* pListBoxHelper = m_xParent.is() ? m_xParent->getListBoxHelper() : nullptr;
        if (pListBoxHelper && pListBoxHelper->IsEnabled())
        {
            pStateSetHelper->AddState( AccessibleStateType::SELECTABLE );
            pStateSetHelper->AddState( AccessibleStateType::ENABLED );
            pStateSetHelper->AddState( AccessibleStateType::SENSITIVE );
        }

        if ( m_bSelected )
            pStateSetHelper->AddState( AccessibleStateType::SELECTED );
        if ( m_bVisible )
        {
            pStateSetHelper->AddState( AccessibleStateType::VISIBLE );
            pStateSetHelper->AddState( AccessibleStateType::SHOWING );
        }
    }
    else
        pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

    return xStateSet;
}

Locale SAL_CALL VCLXAccessibleListItem::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    return implGetLocale();
}

// XAccessibleComponent

sal_Bool SAL_CALL VCLXAccessibleListItem::containsPoint( const awt::Point& _aPoint ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    bool bInside = false;
    ::accessibility::IComboListBoxHelper* pListBoxHelper = m_xParent.is() ? m_xParent->getListBoxHelper() : nullptr;
    if (pListBoxHelper)
    {
        Rectangle aRect(pListBoxHelper->GetBoundingRectangle((sal_uInt16)m_nIndexInParent));
        aRect.Move(-aRect.TopLeft().X(),-aRect.TopLeft().Y());
        bInside = aRect.IsInside( VCLPoint( _aPoint ) );
    }
    return bInside;
}

Reference< XAccessible > SAL_CALL VCLXAccessibleListItem::getAccessibleAtPoint( const awt::Point& ) throw (RuntimeException, std::exception)
{
    return Reference< XAccessible >();
}

awt::Rectangle SAL_CALL VCLXAccessibleListItem::getBounds(  ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    awt::Rectangle aRect;
    ::accessibility::IComboListBoxHelper* pListBoxHelper = m_xParent.is() ? m_xParent->getListBoxHelper() : nullptr;
    if (pListBoxHelper)
        aRect = AWTRectangle(pListBoxHelper->GetBoundingRectangle((sal_uInt16)m_nIndexInParent));

    return aRect;
}

awt::Point SAL_CALL VCLXAccessibleListItem::getLocation(  ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    Point aPoint(0,0);
    ::accessibility::IComboListBoxHelper* pListBoxHelper = m_xParent.is() ? m_xParent->getListBoxHelper() : nullptr;
    if (pListBoxHelper)
    {
        Rectangle aRect = pListBoxHelper->GetBoundingRectangle( (sal_uInt16)m_nIndexInParent );
        aPoint = aRect.TopLeft();
    }
    return AWTPoint( aPoint );
}

awt::Point SAL_CALL VCLXAccessibleListItem::getLocationOnScreen(  ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    Point aPoint(0,0);
    ::accessibility::IComboListBoxHelper* pListBoxHelper = m_xParent.is() ? m_xParent->getListBoxHelper() : nullptr;
    if (pListBoxHelper)
    {
        Rectangle aRect = pListBoxHelper->GetBoundingRectangle((sal_uInt16)m_nIndexInParent);
        aPoint = aRect.TopLeft();
        aPoint += pListBoxHelper->GetWindowExtentsRelative().TopLeft();
    }
    return AWTPoint( aPoint );
}

awt::Size SAL_CALL VCLXAccessibleListItem::getSize(  ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    Size aSize;
    ::accessibility::IComboListBoxHelper* pListBoxHelper = m_xParent.is() ? m_xParent->getListBoxHelper() : nullptr;
    if (pListBoxHelper)
        aSize = pListBoxHelper->GetBoundingRectangle( (sal_uInt16)m_nIndexInParent ).GetSize();

    return AWTSize( aSize );
}

void SAL_CALL VCLXAccessibleListItem::grabFocus(  ) throw (RuntimeException, std::exception)
{
    // no focus for each item
}

// XAccessibleText

sal_Int32 SAL_CALL VCLXAccessibleListItem::getCaretPosition() throw (RuntimeException, std::exception)
{
    return -1;
}

sal_Bool SAL_CALL VCLXAccessibleListItem::setCaretPosition( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !implIsValidRange( nIndex, nIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    return false;
}

sal_Unicode SAL_CALL VCLXAccessibleListItem::getCharacter( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    return OCommonAccessibleText::getCharacter( nIndex );
}

Sequence< PropertyValue > SAL_CALL VCLXAccessibleListItem::getCharacterAttributes( sal_Int32 nIndex, const Sequence< OUString >& ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString sText( implGetText() );
    if ( !implIsValidIndex( nIndex, sText.getLength() ) )
        throw IndexOutOfBoundsException();

    return Sequence< PropertyValue >();
}

awt::Rectangle SAL_CALL VCLXAccessibleListItem::getCharacterBounds( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    OUString sText( implGetText() );
    if ( !implIsValidIndex( nIndex, sText.getLength() ) )
        throw IndexOutOfBoundsException();

    awt::Rectangle aBounds( 0, 0, 0, 0 );
    ::accessibility::IComboListBoxHelper* pListBoxHelper = m_xParent.is() ? m_xParent->getListBoxHelper() : nullptr;
    if (pListBoxHelper)
    {
        Rectangle aCharRect = pListBoxHelper->GetEntryCharacterBounds( m_nIndexInParent, nIndex );
        Rectangle aItemRect = pListBoxHelper->GetBoundingRectangle( (sal_uInt16)m_nIndexInParent );
        aCharRect.Move( -aItemRect.Left(), -aItemRect.Top() );
        aBounds = AWTRectangle( aCharRect );
    }

    return aBounds;
}

sal_Int32 SAL_CALL VCLXAccessibleListItem::getCharacterCount() throw (RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    return OCommonAccessibleText::getCharacterCount();
}

sal_Int32 SAL_CALL VCLXAccessibleListItem::getIndexAtPoint( const awt::Point& aPoint ) throw (RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    sal_Int32 nIndex = -1;
    ::accessibility::IComboListBoxHelper* pListBoxHelper = m_xParent.is() ? m_xParent->getListBoxHelper() : nullptr;
    if (pListBoxHelper)
    {
        sal_Int32 nPos = LISTBOX_ENTRY_NOTFOUND;
        Rectangle aItemRect = pListBoxHelper->GetBoundingRectangle( (sal_uInt16)m_nIndexInParent );
        Point aPnt( VCLPoint( aPoint ) );
        aPnt += aItemRect.TopLeft();
        sal_Int32 nI = pListBoxHelper->GetIndexForPoint( aPnt, nPos );
        if ( nI != -1 && m_nIndexInParent == nPos )
            nIndex = nI;
    }
    return nIndex;
}

OUString SAL_CALL VCLXAccessibleListItem::getSelectedText() throw (RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    return OCommonAccessibleText::getSelectedText();
}

sal_Int32 SAL_CALL VCLXAccessibleListItem::getSelectionStart() throw (RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    return OCommonAccessibleText::getSelectionStart();
}

sal_Int32 SAL_CALL VCLXAccessibleListItem::getSelectionEnd() throw (RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    return OCommonAccessibleText::getSelectionEnd();
}

sal_Bool SAL_CALL VCLXAccessibleListItem::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
        throw IndexOutOfBoundsException();

    return false;
}

OUString SAL_CALL VCLXAccessibleListItem::getText() throw (RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    return OCommonAccessibleText::getText();
}

OUString SAL_CALL VCLXAccessibleListItem::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    return OCommonAccessibleText::getTextRange( nStartIndex, nEndIndex );
}

css::accessibility::TextSegment SAL_CALL VCLXAccessibleListItem::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    return OCommonAccessibleText::getTextAtIndex( nIndex, aTextType );
}

css::accessibility::TextSegment SAL_CALL VCLXAccessibleListItem::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    return OCommonAccessibleText::getTextBeforeIndex( nIndex, aTextType );
}

css::accessibility::TextSegment SAL_CALL VCLXAccessibleListItem::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (css::lang::IndexOutOfBoundsException, css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    return OCommonAccessibleText::getTextBehindIndex( nIndex, aTextType );
}

sal_Bool SAL_CALL VCLXAccessibleListItem::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
{
    SolarMutexGuard aSolarGuard;
    ::osl::MutexGuard aGuard( m_aMutex );

    checkIndex_Impl( nStartIndex, m_sEntryText );
    checkIndex_Impl( nEndIndex, m_sEntryText );

    bool bRet = false;
    ::accessibility::IComboListBoxHelper* pListBoxHelper = m_xParent.is() ? m_xParent->getListBoxHelper() : nullptr;
    if (pListBoxHelper)
    {
        Reference< datatransfer::clipboard::XClipboard > xClipboard = pListBoxHelper->GetClipboard();
        if ( xClipboard.is() )
        {
            OUString sText( getTextRange( nStartIndex, nEndIndex ) );
            vcl::unohelper::TextDataObject* pDataObj = new vcl::unohelper::TextDataObject( sText );

            SolarMutexReleaser aReleaser;
            xClipboard->setContents( pDataObj, nullptr );
            Reference< datatransfer::clipboard::XFlushableClipboard > xFlushableClipboard( xClipboard, uno::UNO_QUERY );
            if( xFlushableClipboard.is() )
                xFlushableClipboard->flushClipboard();

            bRet = true;
        }
    }

    return bRet;
}

// XAccessibleEventBroadcaster

void SAL_CALL VCLXAccessibleListItem::addAccessibleEventListener( const Reference< XAccessibleEventListener >& xListener ) throw (RuntimeException, std::exception)
{
    if (xListener.is())
    {
        if (!m_nClientId)
            m_nClientId = comphelper::AccessibleEventNotifier::registerClient( );
        comphelper::AccessibleEventNotifier::addEventListener( m_nClientId, xListener );
    }
}

void SAL_CALL VCLXAccessibleListItem::removeAccessibleEventListener( const Reference< XAccessibleEventListener >& xListener ) throw (RuntimeException, std::exception)
{
    if ( xListener.is() && m_nClientId )
    {
        sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( m_nClientId, xListener );
        if ( !nListenerCount )
        {
            // no listeners anymore
            // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
            // and at least to us not firing any events anymore, in case somebody calls
            // NotifyAccessibleEvent, again
            if ( m_nClientId )
            {
                comphelper::AccessibleEventNotifier::TClientId nId( m_nClientId );
                m_nClientId = 0;
                comphelper::AccessibleEventNotifier::revokeClient( nId );
            }
        }
    }
}


// AF (Oct. 29 2002): Return black as constant foreground color.  This is an
// initial implementation and has to be substituted by code that determines
// the color that is actually used.
sal_Int32 SAL_CALL VCLXAccessibleListItem::getForeground()
    throw (css::uno::RuntimeException, std::exception)
{
    return COL_BLACK;
}

// AF (Oct. 29 2002): Return white as constant background color.  This is an
// initial implementation and has to be substituted by code that determines
// the color that is actually used.
sal_Int32 SAL_CALL VCLXAccessibleListItem::getBackground()
    throw (css::uno::RuntimeException, std::exception)
{
    return COL_WHITE;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
