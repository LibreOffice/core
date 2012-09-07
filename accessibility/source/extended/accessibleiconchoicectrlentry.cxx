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

#include <accessibility/extended/accessibleiconchoicectrlentry.hxx>
#include <svtools/ivctrl.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <vcl/svapp.hxx>
#include <vcl/controllayout.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/convert.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <svtools/stringtransfer.hxx>
#include <comphelper/accessibleeventnotifier.hxx>

#define ACCESSIBLE_ACTION_COUNT     1

namespace
{
    void checkActionIndex_Impl( sal_Int32 _nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException)
    {
        if ( _nIndex < 0 || _nIndex >= ACCESSIBLE_ACTION_COUNT )
            // only three actions
            throw ::com::sun::star::lang::IndexOutOfBoundsException();
    }
}

//........................................................................
namespace accessibility
{

    // class AccessibleIconChoiceCtrlEntry -----------------------------------------------------

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;

    // -----------------------------------------------------------------------------
    // Ctor() and Dtor()
    // -----------------------------------------------------------------------------
    AccessibleIconChoiceCtrlEntry::AccessibleIconChoiceCtrlEntry( SvtIconChoiceCtrl& _rIconCtrl,
                                                                  sal_uLong _nPos,
                                                                  const Reference< XAccessible >& _xParent ) :

        AccessibleIconChoiceCtrlEntry_BASE  ( m_aMutex ),

        m_pIconCtrl     ( &_rIconCtrl ),
        m_nIndex        ( _nPos ),
        m_nClientId     ( 0 ),
        m_xParent       ( _xParent )

    {
        osl_incrementInterlockedCount( &m_refCount );
        {
            Reference< XComponent > xComp( m_xParent, UNO_QUERY );
            if ( xComp.is() )
                xComp->addEventListener( this );
        }
        osl_decrementInterlockedCount( &m_refCount );
    }
    // -----------------------------------------------------------------------------
    void AccessibleIconChoiceCtrlEntry::disposing( const EventObject& _rSource )
throw(RuntimeException)
    {
        if ( _rSource.Source == m_xParent )
        {
            dispose();
            OSL_ENSURE( !m_xParent.is() && ( NULL == m_pIconCtrl ), "" );
        }
    }
    // -----------------------------------------------------------------------------
    AccessibleIconChoiceCtrlEntry::~AccessibleIconChoiceCtrlEntry()
    {
        if ( IsAlive_Impl() )
        {
            // increment ref count to prevent double call of Dtor
            osl_incrementInterlockedCount( &m_refCount );
            dispose();
        }
    }
    // -----------------------------------------------------------------------------
    Rectangle AccessibleIconChoiceCtrlEntry::GetBoundingBox_Impl() const
    {
        Rectangle aRect;
        SvxIconChoiceCtrlEntry* pEntry = m_pIconCtrl->GetEntry( m_nIndex );
        if ( pEntry )
            aRect = m_pIconCtrl->GetBoundingBox( pEntry );

        return aRect;
    }
    // -----------------------------------------------------------------------------
    Rectangle AccessibleIconChoiceCtrlEntry::GetBoundingBoxOnScreen_Impl() const
    {
        Rectangle aRect;
        SvxIconChoiceCtrlEntry* pEntry = m_pIconCtrl->GetEntry( m_nIndex );
        if ( pEntry )
        {
            aRect = m_pIconCtrl->GetBoundingBox( pEntry );
            Point aTopLeft = aRect.TopLeft();
            aTopLeft += m_pIconCtrl->GetWindowExtentsRelative( NULL ).TopLeft();
            aRect = Rectangle( aTopLeft, aRect.GetSize() );
        }

        return aRect;
    }
    // -----------------------------------------------------------------------------
    sal_Bool AccessibleIconChoiceCtrlEntry::IsAlive_Impl() const
    {
        return ( !rBHelper.bDisposed && !rBHelper.bInDispose && m_pIconCtrl );
    }
    // -----------------------------------------------------------------------------
    sal_Bool AccessibleIconChoiceCtrlEntry::IsShowing_Impl() const
    {
        sal_Bool bShowing = sal_False;
        Reference< XAccessibleContext > m_xParentContext =
            m_xParent.is() ? m_xParent->getAccessibleContext() : Reference< XAccessibleContext >();
        if( m_xParentContext.is() )
        {
            Reference< XAccessibleComponent > xParentComp( m_xParentContext, uno::UNO_QUERY );
            if( xParentComp.is() )
                bShowing = GetBoundingBox_Impl().IsOver( VCLRectangle( xParentComp->getBounds() ) );
        }

        return bShowing;
    }
    // -----------------------------------------------------------------------------
    Rectangle AccessibleIconChoiceCtrlEntry::GetBoundingBox() throw ( lang::DisposedException )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        return GetBoundingBox_Impl();
    }
    // -----------------------------------------------------------------------------
    Rectangle AccessibleIconChoiceCtrlEntry::GetBoundingBoxOnScreen() throw ( lang::DisposedException )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        return GetBoundingBoxOnScreen_Impl();
    }
    // -----------------------------------------------------------------------------
    void AccessibleIconChoiceCtrlEntry::EnsureIsAlive() const throw ( lang::DisposedException )
    {
        if ( !IsAlive_Impl() )
            throw lang::DisposedException();
    }
    // -----------------------------------------------------------------------------
    OUString AccessibleIconChoiceCtrlEntry::implGetText()
    {
        OUString sRet;
        SvxIconChoiceCtrlEntry* pEntry = m_pIconCtrl->GetEntry( m_nIndex );
        if ( pEntry )
            sRet = pEntry->GetDisplayText();
        return sRet;
    }
    // -----------------------------------------------------------------------------
    Locale AccessibleIconChoiceCtrlEntry::implGetLocale()
    {
        Locale aLocale;
        aLocale = Application::GetSettings().GetUILocale();

        return aLocale;
    }
    void AccessibleIconChoiceCtrlEntry::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
    {
        nStartIndex = 0;
        nEndIndex = 0;
    }
    // -----------------------------------------------------------------------------
    // XTypeProvider
    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    Sequence< sal_Int8 > AccessibleIconChoiceCtrlEntry::getImplementationId() throw (RuntimeException)
    {
        static ::cppu::OImplementationId* pId = NULL;

        if ( !pId )
        {
            ::osl::Guard< ::osl::Mutex > aGuard( m_aMutex );

            if ( !pId )
            {
                static ::cppu::OImplementationId aId;
                pId = &aId;
            }
        }
        return pId->getImplementationId();
    }
    // -----------------------------------------------------------------------------
    // XComponent
    // -----------------------------------------------------------------------------
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

        m_pIconCtrl = NULL;
        m_xParent = NULL;
    }
    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------
    OUString SAL_CALL AccessibleIconChoiceCtrlEntry::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }
    // -----------------------------------------------------------------------------
    Sequence< OUString > SAL_CALL AccessibleIconChoiceCtrlEntry::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleIconChoiceCtrlEntry::supportsService( const OUString& _rServiceName ) throw (RuntimeException)
    {
        Sequence< OUString > aSupported( getSupportedServiceNames() );
        const OUString* pSupported = aSupported.getConstArray();
        const OUString* pEnd = pSupported + aSupported.getLength();
        for ( ; pSupported != pEnd && !pSupported->equals(_rServiceName); ++pSupported )
            ;

        return pSupported != pEnd;
    }
    // -----------------------------------------------------------------------------
    // XServiceInfo - static methods
    // -----------------------------------------------------------------------------
    Sequence< OUString > AccessibleIconChoiceCtrlEntry::getSupportedServiceNames_Static(void) throw( RuntimeException )
    {
        Sequence< OUString > aSupported(3);
        aSupported[0] = "com.sun.star.accessibility.AccessibleContext";
        aSupported[1] = "com.sun.star.accessibility.AccessibleComponent";
        aSupported[2] = "com.sun.star.awt.AccessibleIconChoiceControlEntry";
        return aSupported;
    }
    // -----------------------------------------------------------------------------
    OUString AccessibleIconChoiceCtrlEntry::getImplementationName_Static(void) throw( RuntimeException )
    {
        return OUString( "com.sun.star.comp.svtools.AccessibleIconChoiceControlEntry" );
    }
    // -----------------------------------------------------------------------------
    // XAccessible
    // -----------------------------------------------------------------------------
    Reference< XAccessibleContext > SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleContext(  ) throw (RuntimeException)
    {
        EnsureIsAlive();
        return this;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleContext
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        return 0; // no children
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleChild( sal_Int32 ) throw (IndexOutOfBoundsException,RuntimeException)
    {
        throw IndexOutOfBoundsException();
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleParent(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        return m_xParent;
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleIndexInParent(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        return m_nIndex;
    }
    // -----------------------------------------------------------------------------
    sal_Int16 SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleRole(  ) throw (RuntimeException)
    {
        return AccessibleRole::LABEL;
    }
    // -----------------------------------------------------------------------------
    OUString SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleDescription(  ) throw (RuntimeException)
    {
        // no description for every item
        return OUString();
    }
    // -----------------------------------------------------------------------------
    OUString SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleName(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        return implGetText();
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessibleRelationSet > SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleRelationSet(  ) throw (RuntimeException)
    {
        return new utl::AccessibleRelationSetHelper;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessibleStateSet > SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleStateSet(  ) throw (RuntimeException)
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
    // -----------------------------------------------------------------------------
    Locale SAL_CALL AccessibleIconChoiceCtrlEntry::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        return implGetLocale();
    }
    // -----------------------------------------------------------------------------
    // XAccessibleComponent
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleIconChoiceCtrlEntry::containsPoint( const awt::Point& rPoint ) throw (RuntimeException)
    {
        return Rectangle( Point(), GetBoundingBox().GetSize() ).IsInside( VCLPoint( rPoint ) );
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleAtPoint( const awt::Point& ) throw (RuntimeException)
    {
        return Reference< XAccessible >();
    }
    // -----------------------------------------------------------------------------
    awt::Rectangle SAL_CALL AccessibleIconChoiceCtrlEntry::getBounds(  ) throw (RuntimeException)
    {
        return AWTRectangle( GetBoundingBox() );
    }
    // -----------------------------------------------------------------------------
    awt::Point SAL_CALL AccessibleIconChoiceCtrlEntry::getLocation(  ) throw (RuntimeException)
    {
        return AWTPoint( GetBoundingBox().TopLeft() );
    }
    // -----------------------------------------------------------------------------
    awt::Point SAL_CALL AccessibleIconChoiceCtrlEntry::getLocationOnScreen(  ) throw (RuntimeException)
    {
        return AWTPoint( GetBoundingBoxOnScreen().TopLeft() );
    }
    // -----------------------------------------------------------------------------
    awt::Size SAL_CALL AccessibleIconChoiceCtrlEntry::getSize(  ) throw (RuntimeException)
    {
        return AWTSize( GetBoundingBox().GetSize() );
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleIconChoiceCtrlEntry::grabFocus(  ) throw (RuntimeException)
    {
        // do nothing, because no focus for each item
    }
    // -----------------------------------------------------------------------------
    sal_Int32 AccessibleIconChoiceCtrlEntry::getForeground( ) throw (RuntimeException)
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
    // -----------------------------------------------------------------------------
    sal_Int32 AccessibleIconChoiceCtrlEntry::getBackground(  ) throw (RuntimeException)
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
    // -----------------------------------------------------------------------------
    // XAccessibleText
    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    awt::Rectangle SAL_CALL AccessibleIconChoiceCtrlEntry::getCharacterBounds( sal_Int32 _nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        if ( ( 0 > _nIndex ) || ( getCharacterCount() <= _nIndex ) )
            throw IndexOutOfBoundsException();

        awt::Rectangle aBounds( 0, 0, 0, 0 );
        if ( m_pIconCtrl )
        {
            Rectangle aItemRect = GetBoundingBox_Impl();
            Rectangle aCharRect = m_pIconCtrl->GetEntryCharacterBounds( m_nIndex, _nIndex );
            aCharRect.Move( -aItemRect.Left(), -aItemRect.Top() );
            aBounds = AWTRectangle( aCharRect );
        }

        return aBounds;
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleIconChoiceCtrlEntry::getIndexAtPoint( const awt::Point& aPoint ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        sal_Int32 nIndex = -1;
        if ( m_pIconCtrl )
        {
            ::vcl::ControlLayoutData aLayoutData;
            Rectangle aItemRect = GetBoundingBox_Impl();
            m_pIconCtrl->RecordLayoutData( &aLayoutData, aItemRect );
            Point aPnt( VCLPoint( aPoint ) );
            aPnt += aItemRect.TopLeft();
            nIndex = aLayoutData.GetIndexForPoint( aPnt );

            long nLen = aLayoutData.m_aUnicodeBoundRects.size();
            for ( long i = 0; i < nLen; ++i )
            {
                Rectangle aRect = aLayoutData.GetCharacterBounds(i);
                sal_Bool bInside = aRect.IsInside( aPnt );

                if ( bInside )
                    break;
            }
        }

        return nIndex;
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleIconChoiceCtrlEntry::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        String sText = getText();
        if  ( ( 0 > nStartIndex ) || ( sText.Len() <= nStartIndex )
            || ( 0 > nEndIndex ) || ( sText.Len() <= nEndIndex ) )
            throw IndexOutOfBoundsException();

        sal_Int32 nLen = nEndIndex - nStartIndex + 1;
        ::svt::OStringTransfer::CopyString( sText.Copy( (sal_uInt16)nStartIndex, (sal_uInt16)nLen ), m_pIconCtrl );

        return sal_True;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleEventBroadcaster
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleIconChoiceCtrlEntry::addEventListener( const Reference< XAccessibleEventListener >& xListener ) throw (RuntimeException)
    {
        if (xListener.is())
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if (!m_nClientId)
                m_nClientId = comphelper::AccessibleEventNotifier::registerClient( );
            comphelper::AccessibleEventNotifier::addEventListener( m_nClientId, xListener );
        }
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleIconChoiceCtrlEntry::removeEventListener( const Reference< XAccessibleEventListener >& xListener ) throw (RuntimeException)
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

    sal_Int32 SAL_CALL AccessibleIconChoiceCtrlEntry::getCaretPosition(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return -1;
    }
    sal_Bool SAL_CALL AccessibleIconChoiceCtrlEntry::setCaretPosition ( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        if ( !implIsValidRange( nIndex, nIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return sal_False;
    }
    sal_Unicode SAL_CALL AccessibleIconChoiceCtrlEntry::getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getCharacter( nIndex );
    }
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL AccessibleIconChoiceCtrlEntry::getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< OUString >& ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        OUString sText( implGetText() );

        if ( !implIsValidIndex( nIndex, sText.getLength() ) )
            throw IndexOutOfBoundsException();

        return ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >();
    }
    sal_Int32 SAL_CALL AccessibleIconChoiceCtrlEntry::getCharacterCount(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getCharacterCount(  );
    }

    OUString SAL_CALL AccessibleIconChoiceCtrlEntry::getSelectedText(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getSelectedText(  );
    }
    sal_Int32 SAL_CALL AccessibleIconChoiceCtrlEntry::getSelectionStart(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getSelectionStart(  );
    }
    sal_Int32 SAL_CALL AccessibleIconChoiceCtrlEntry::getSelectionEnd(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getSelectionEnd(  );
    }
    sal_Bool SAL_CALL AccessibleIconChoiceCtrlEntry::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return sal_False;
    }
    OUString SAL_CALL AccessibleIconChoiceCtrlEntry::getText(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getText(  );
    }
    OUString SAL_CALL AccessibleIconChoiceCtrlEntry::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getTextRange( nStartIndex, nEndIndex );
    }
    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleIconChoiceCtrlEntry::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getTextAtIndex( nIndex ,aTextType);
    }
    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleIconChoiceCtrlEntry::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getTextBeforeIndex( nIndex ,aTextType);
    }
    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleIconChoiceCtrlEntry::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        return OCommonAccessibleText::getTextBehindIndex( nIndex ,aTextType);
    }

    // -----------------------------------------------------------------------------
    // XAccessibleAction
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleActionCount(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // three actions supported
        return ACCESSIBLE_ACTION_COUNT;
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleIconChoiceCtrlEntry::doAccessibleAction( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        sal_Bool bRet = sal_False;
        checkActionIndex_Impl( nIndex );
        EnsureIsAlive();

        SvxIconChoiceCtrlEntry* pEntry = m_pIconCtrl->GetEntry( m_nIndex );
        if ( pEntry && !pEntry->IsSelected() )
        {
            m_pIconCtrl->SetNoSelection();
            m_pIconCtrl->SetCursor( pEntry );
            bRet = sal_True;
        }

        return bRet;
    }
    // -----------------------------------------------------------------------------
    OUString SAL_CALL AccessibleIconChoiceCtrlEntry::getAccessibleActionDescription( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        checkActionIndex_Impl( nIndex );
        EnsureIsAlive();

        static const OUString sActionDesc( "Select" );
        return sActionDesc;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessibleKeyBinding > AccessibleIconChoiceCtrlEntry::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Reference< XAccessibleKeyBinding > xRet;
        checkActionIndex_Impl( nIndex );
        // ... which key?
        return xRet;
    }
//........................................................................
}// namespace accessibility
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
