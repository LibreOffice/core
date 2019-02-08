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

#include <extended/accessibleiconchoicectrlentry.hxx>
#include <svtools/ivctrl.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <vcl/svapp.hxx>
#include <vcl/controllayout.hxx>
#include <vcl/settings.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/convert.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <svtools/stringtransfer.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <i18nlangtag/languagetag.hxx>

#define ACCESSIBLE_ACTION_COUNT     1

namespace
{
    /// @throws css::lang::IndexOutOfBoundsException
    void checkActionIndex_Impl( sal_Int32 _nIndex )
    {
        if ( _nIndex < 0 || _nIndex >= ACCESSIBLE_ACTION_COUNT )
            // only three actions
            throw css::lang::IndexOutOfBoundsException();
    }
}


namespace accessibility
{

    // class AccessibleIconChoiceCtrlEntry

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;


    // Ctor() and Dtor()

    AccessibleIconChoiceCtrlEntry::AccessibleIconChoiceCtrlEntry( SvtIconChoiceCtrl& _rIconCtrl,
                                                                  sal_Int32 _nPos,
                                                                  const Reference< XAccessible >& _xParent ) :

        AccessibleIconChoiceCtrlEntry_BASE  ( m_aMutex ),

        m_pIconCtrl     ( &_rIconCtrl ),
        m_nIndex        ( _nPos ),
        m_nClientId     ( 0 ),
        m_xParent       ( _xParent )

    {
        osl_atomic_increment( &m_refCount );
        {
            Reference< XComponent > xComp( m_xParent, UNO_QUERY );
            if ( xComp.is() )
                xComp->addEventListener( this );
        }
        osl_atomic_decrement( &m_refCount );
    }

    void AccessibleIconChoiceCtrlEntry::disposing( const css::lang::EventObject& _rSource )
    {
        if ( _rSource.Source == m_xParent )
        {
            dispose();
            OSL_ENSURE( !m_xParent.is() && ( m_pIconCtrl == nullptr ), "" );
        }
    }

    AccessibleIconChoiceCtrlEntry::~AccessibleIconChoiceCtrlEntry()
    {
        if ( IsAlive_Impl() )
        {
            // increment ref count to prevent double call of Dtor
            osl_atomic_increment( &m_refCount );
            dispose();
        }
    }

    tools::Rectangle AccessibleIconChoiceCtrlEntry::GetBoundingBox_Impl() const
    {
        tools::Rectangle aRect;
        SvxIconChoiceCtrlEntry* pEntry = m_pIconCtrl->GetEntry( m_nIndex );
        if ( pEntry )
            aRect = m_pIconCtrl->GetBoundingBox( pEntry );

        return aRect;
    }

    tools::Rectangle AccessibleIconChoiceCtrlEntry::GetBoundingBoxOnScreen_Impl() const
    {
        tools::Rectangle aRect;
        SvxIconChoiceCtrlEntry* pEntry = m_pIconCtrl->GetEntry( m_nIndex );
        if ( pEntry )
        {
            aRect = m_pIconCtrl->GetBoundingBox( pEntry );
            Point aTopLeft = aRect.TopLeft();
            aTopLeft += m_pIconCtrl->GetWindowExtentsRelative( nullptr ).TopLeft();
            aRect = tools::Rectangle( aTopLeft, aRect.GetSize() );
        }

        return aRect;
    }

    bool AccessibleIconChoiceCtrlEntry::IsAlive_Impl() const
    {
        return ( !rBHelper.bDisposed && !rBHelper.bInDispose && m_pIconCtrl );
    }

    bool AccessibleIconChoiceCtrlEntry::IsShowing_Impl() const
    {
        bool bShowing = false;
        Reference< XAccessibleContext > xParentContext =
            m_xParent.is() ? m_xParent->getAccessibleContext() : Reference< XAccessibleContext >();
        if( xParentContext.is() )
        {
            Reference< XAccessibleComponent > xParentComp( xParentContext, uno::UNO_QUERY );
            if( xParentComp.is() )
                bShowing = GetBoundingBox_Impl().IsOver( VCLRectangle( xParentComp->getBounds() ) );
        }

        return bShowing;
    }

    tools::Rectangle AccessibleIconChoiceCtrlEntry::GetBoundingBox()
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        return GetBoundingBox_Impl();
    }

    tools::Rectangle AccessibleIconChoiceCtrlEntry::GetBoundingBoxOnScreen()
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        return GetBoundingBoxOnScreen_Impl();
    }

    void AccessibleIconChoiceCtrlEntry::EnsureIsAlive() const
    {
        if ( !IsAlive_Impl() )
            throw lang::DisposedException();
    }

    OUString AccessibleIconChoiceCtrlEntry::implGetText()
    {
        OUString sRet;
        SvxIconChoiceCtrlEntry* pEntry = m_pIconCtrl->GetEntry( m_nIndex );
        if ( pEntry )
            sRet = pEntry->GetDisplayText();
        return sRet;
    }

    Locale AccessibleIconChoiceCtrlEntry::implGetLocale()
    {
        Locale aLocale;
        aLocale = Application::GetSettings().GetUILanguageTag().getLocale();

        return aLocale;
    }
    void AccessibleIconChoiceCtrlEntry::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
    {
        nStartIndex = 0;
        nEndIndex = 0;
    }

    // XTypeProvider


    Sequence< sal_Int8 > AccessibleIconChoiceCtrlEntry::getImplementationId()
    {
        return css::uno::Sequence<sal_Int8>();
    }

    // XComponent

    void SAL_CALL AccessibleIconChoiceCtrlEntry::disposing()
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // Send a disposing to all listeners.
        if ( m_nClientId )
        {
            sal_uInt32 nId = m_nClientId;
            m_nClientId =  0;
            comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( nId, *this );
        }

        Reference< XComponent > xComp( m_xParent, UNO_QUERY );
        if ( xComp.is() )
            xComp->removeEventListener( this );

        m_pIconCtrl = nullptr;
        m_xParent = nullptr;
    }

    // XServiceInfo

    OUString SAL_CALL AccessibleIconChoiceCtrlEntry::getImplementationName()
    {
        return OUString( "com.sun.star.comp.svtools.AccessibleIconChoiceControlEntry" );
    }

    Sequence< OUString > SAL_CALL AccessibleIconChoiceCtrlEntry::getSupportedServiceNames()
    {
        return {"com.sun.star.accessibility.AccessibleContext",
                "com.sun.star.accessibility.AccessibleComponent",
                "com.sun.star.awt.AccessibleIconChoiceControlEntry"};
    }

    sal_Bool SAL_CALL AccessibleIconChoiceCtrlEntry::supportsService( const OUString& _rServiceName )
    {
        return cppu::supportsService(this, _rServiceName);
    }

    // XAccessible

    Reference< XAccessibleContext > SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleContext(  )
    {
        EnsureIsAlive();
        return this;
    }

    // XAccessibleContext

    sal_Int32 SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleChildCount(  )
    {
        return 0; // no children
    }

    Reference< XAccessible > SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleChild( sal_Int32 )
    {
        throw IndexOutOfBoundsException();
    }

    Reference< XAccessible > SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleParent(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        return m_xParent;
    }

    sal_Int32 SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleIndexInParent(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return m_nIndex;
    }

    sal_Int16 SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleRole(  )
    {
        //return AccessibleRole::LABEL;
        return AccessibleRole::LIST_ITEM;
    }

    OUString SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleDescription(  )
    {
        // no description for every item
        return OUString();
    }

    OUString SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleName(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        return implGetText();
    }

    Reference< XAccessibleRelationSet > SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleRelationSet(  )
    {
        return new utl::AccessibleRelationSetHelper;
    }

    Reference< XAccessibleStateSet > SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleStateSet(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
        Reference< XAccessibleStateSet > xStateSet = pStateSetHelper;

        if ( IsAlive_Impl() )
        {
            pStateSetHelper->AddState( AccessibleStateType::TRANSIENT );
            pStateSetHelper->AddState( AccessibleStateType::SELECTABLE );
            pStateSetHelper->AddState( AccessibleStateType::ENABLED );
            pStateSetHelper->AddState( AccessibleStateType::SENSITIVE );
            if ( IsShowing_Impl() )
            {
                pStateSetHelper->AddState( AccessibleStateType::SHOWING );
                pStateSetHelper->AddState( AccessibleStateType::VISIBLE );
            }

            if ( m_pIconCtrl && m_pIconCtrl->GetCursor() == m_pIconCtrl->GetEntry( m_nIndex ) )
                pStateSetHelper->AddState( AccessibleStateType::SELECTED );
        }
        else
            pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

        return xStateSet;
    }

    Locale SAL_CALL AccessibleIconChoiceCtrlEntry::getLocale(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        return implGetLocale();
    }

    // XAccessibleComponent

    sal_Bool SAL_CALL AccessibleIconChoiceCtrlEntry::containsPoint( const awt::Point& rPoint )
    {
        return tools::Rectangle( Point(), GetBoundingBox().GetSize() ).IsInside( VCLPoint( rPoint ) );
    }

    Reference< XAccessible > SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleAtPoint( const awt::Point& )
    {
        return Reference< XAccessible >();
    }

    awt::Rectangle SAL_CALL AccessibleIconChoiceCtrlEntry::getBounds(  )
    {
        return AWTRectangle( GetBoundingBox() );
    }

    awt::Point SAL_CALL AccessibleIconChoiceCtrlEntry::getLocation(  )
    {
        return AWTPoint( GetBoundingBox().TopLeft() );
    }

    awt::Point SAL_CALL AccessibleIconChoiceCtrlEntry::getLocationOnScreen(  )
    {
        return AWTPoint( GetBoundingBoxOnScreen().TopLeft() );
    }

    awt::Size SAL_CALL AccessibleIconChoiceCtrlEntry::getSize(  )
    {
        return AWTSize( GetBoundingBox().GetSize() );
    }

    void SAL_CALL AccessibleIconChoiceCtrlEntry::grabFocus(  )
    {
        // do nothing, because no focus for each item
    }

    sal_Int32 AccessibleIconChoiceCtrlEntry::getForeground( )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

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

    sal_Int32 AccessibleIconChoiceCtrlEntry::getBackground(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

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

    // XAccessibleText


    awt::Rectangle SAL_CALL AccessibleIconChoiceCtrlEntry::getCharacterBounds( sal_Int32 _nIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( ( 0 > _nIndex ) || ( implGetText().getLength() <= _nIndex ) )
            throw IndexOutOfBoundsException();

        awt::Rectangle aBounds( 0, 0, 0, 0 );
        if ( m_pIconCtrl )
        {
            tools::Rectangle aItemRect = GetBoundingBox_Impl();
            tools::Rectangle aCharRect = m_pIconCtrl->GetEntryCharacterBounds( m_nIndex, _nIndex );
            aCharRect.Move( -aItemRect.Left(), -aItemRect.Top() );
            aBounds = AWTRectangle( aCharRect );
        }

        return aBounds;
    }

    sal_Int32 SAL_CALL AccessibleIconChoiceCtrlEntry::getIndexAtPoint( const awt::Point& aPoint )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        sal_Int32 nIndex = -1;
        if ( m_pIconCtrl )
        {
            vcl::ControlLayoutData aLayoutData;
            tools::Rectangle aItemRect = GetBoundingBox_Impl();
            m_pIconCtrl->RecordLayoutData( &aLayoutData, aItemRect );
            Point aPnt( VCLPoint( aPoint ) );
            aPnt += aItemRect.TopLeft();
            nIndex = aLayoutData.GetIndexForPoint( aPnt );

            long nLen = aLayoutData.m_aUnicodeBoundRects.size();
            for ( long i = 0; i < nLen; ++i )
            {
                tools::Rectangle aRect = aLayoutData.GetCharacterBounds(i);
                bool bInside = aRect.IsInside( aPnt );

                if ( bInside )
                    break;
            }
        }

        return nIndex;
    }

    sal_Bool SAL_CALL AccessibleIconChoiceCtrlEntry::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        OUString sText = implGetText();
        if  ( ( 0 > nStartIndex ) || ( sText.getLength() <= nStartIndex )
            || ( 0 > nEndIndex ) || ( sText.getLength() <= nEndIndex ) )
            throw IndexOutOfBoundsException();

        sal_Int32 nLen = nEndIndex - nStartIndex + 1;
        ::svt::OStringTransfer::CopyString( sText.copy( nStartIndex, nLen ), m_pIconCtrl );

        return true;
    }

    // XAccessibleEventBroadcaster

    void SAL_CALL AccessibleIconChoiceCtrlEntry::addAccessibleEventListener( const Reference< XAccessibleEventListener >& xListener )
    {
        if (xListener.is())
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if (!m_nClientId)
                m_nClientId = comphelper::AccessibleEventNotifier::registerClient( );
            comphelper::AccessibleEventNotifier::addEventListener( m_nClientId, xListener );
        }
    }

    void SAL_CALL AccessibleIconChoiceCtrlEntry::removeAccessibleEventListener( const Reference< XAccessibleEventListener >& xListener )
    {
        if (xListener.is())
        {
            ::osl::MutexGuard aGuard( m_aMutex );

            sal_Int32 nListenerCount = comphelper::AccessibleEventNotifier::removeEventListener( m_nClientId, xListener );
            if ( !nListenerCount )
            {
                // no listeners anymore
                // -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
                // and at least to us not firing any events anymore, in case somebody calls
                // NotifyAccessibleEvent, again
                sal_Int32 nId = m_nClientId;
                m_nClientId = 0;
                comphelper::AccessibleEventNotifier::revokeClient( nId );
            }
        }
    }

    sal_Int32 SAL_CALL AccessibleIconChoiceCtrlEntry::getCaretPosition(  )
    {
        return -1;
    }
    sal_Bool SAL_CALL AccessibleIconChoiceCtrlEntry::setCaretPosition ( sal_Int32 nIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        if ( !implIsValidRange( nIndex, nIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return false;
    }
    sal_Unicode SAL_CALL AccessibleIconChoiceCtrlEntry::getCharacter( sal_Int32 nIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::implGetCharacter( implGetText(), nIndex );
    }
    css::uno::Sequence< css::beans::PropertyValue > SAL_CALL AccessibleIconChoiceCtrlEntry::getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        OUString sText( implGetText() );

        if ( !implIsValidIndex( nIndex, sText.getLength() ) )
            throw IndexOutOfBoundsException();

        return css::uno::Sequence< css::beans::PropertyValue >();
    }
    sal_Int32 SAL_CALL AccessibleIconChoiceCtrlEntry::getCharacterCount(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return implGetText().getLength();
    }

    OUString SAL_CALL AccessibleIconChoiceCtrlEntry::getSelectedText(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OUString();
    }
    sal_Int32 SAL_CALL AccessibleIconChoiceCtrlEntry::getSelectionStart(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return 0;
    }
    sal_Int32 SAL_CALL AccessibleIconChoiceCtrlEntry::getSelectionEnd(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return 0;
    }
    sal_Bool SAL_CALL AccessibleIconChoiceCtrlEntry::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return false;
    }
    OUString SAL_CALL AccessibleIconChoiceCtrlEntry::getText(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return implGetText(  );
    }
    OUString SAL_CALL AccessibleIconChoiceCtrlEntry::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::implGetTextRange( implGetText(), nStartIndex, nEndIndex );
    }
    css::accessibility::TextSegment SAL_CALL AccessibleIconChoiceCtrlEntry::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getTextAtIndex( nIndex ,aTextType);
    }
    css::accessibility::TextSegment SAL_CALL AccessibleIconChoiceCtrlEntry::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getTextBeforeIndex( nIndex ,aTextType);
    }
    css::accessibility::TextSegment SAL_CALL AccessibleIconChoiceCtrlEntry::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        return OCommonAccessibleText::getTextBehindIndex( nIndex ,aTextType);
    }


    // XAccessibleAction

    sal_Int32 SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleActionCount(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // three actions supported
        return ACCESSIBLE_ACTION_COUNT;
    }

    sal_Bool SAL_CALL AccessibleIconChoiceCtrlEntry::doAccessibleAction( sal_Int32 nIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        bool bRet = false;
        checkActionIndex_Impl( nIndex );
        EnsureIsAlive();

        SvxIconChoiceCtrlEntry* pEntry = m_pIconCtrl->GetEntry( m_nIndex );
        if ( pEntry && !pEntry->IsSelected() )
        {
            m_pIconCtrl->SetNoSelection();
            m_pIconCtrl->SetCursor( pEntry );
            bRet = true;
        }

        return bRet;
    }

    OUString SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleActionDescription( sal_Int32 nIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        checkActionIndex_Impl( nIndex );
        EnsureIsAlive();

        return OUString( "Select" );
    }

    Reference< XAccessibleKeyBinding > AccessibleIconChoiceCtrlEntry::getAccessibleActionKeyBinding( sal_Int32 nIndex )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Reference< XAccessibleKeyBinding > xRet;
        checkActionIndex_Impl( nIndex );
        // ... which key?
        return xRet;
    }

}// namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
