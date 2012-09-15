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

#include "accessibility/extended/accessiblelistboxentry.hxx"
#include <svtools/svtreebx.hxx>
#include <svtools/stringtransfer.hxx>
#include <com/sun/star/awt/Point.hpp>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <vcl/svapp.hxx>
#include <vcl/controllayout.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/convert.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#define ACCESSIBLE_ACTION_COUNT 1

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
    // class AccessibleListBoxEntry -----------------------------------------------------

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;

    // -----------------------------------------------------------------------------
    // Ctor() and Dtor()
    // -----------------------------------------------------------------------------
    AccessibleListBoxEntry::AccessibleListBoxEntry( SvTreeListBox& _rListBox,
                                                    SvLBoxEntry* _pEntry,
                                                    const Reference< XAccessible >& _xParent ) :

        AccessibleListBoxEntry_BASE ( m_aMutex ),
        ListBoxAccessibleBase( _rListBox ),

        m_nClientId     ( 0 ),
        m_aParent       ( _xParent )

    {
        _rListBox.FillEntryPath( _pEntry, m_aEntryPath );
    }
    // -----------------------------------------------------------------------------
    AccessibleListBoxEntry::~AccessibleListBoxEntry()
    {
        if ( IsAlive_Impl() )
        {
            // increment ref count to prevent double call of Dtor
            osl_incrementInterlockedCount( &m_refCount );
            dispose();
        }
    }

    // -----------------------------------------------------------------------------
    Rectangle AccessibleListBoxEntry::GetBoundingBox_Impl() const
    {
        Rectangle aRect;
        SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( pEntry )
        {
            aRect = getListBox()->GetBoundingRect( pEntry );
            SvLBoxEntry* pParent = getListBox()->GetParent( pEntry );
            if ( pParent )
            {
                // position relative to parent entry
                Point aTopLeft = aRect.TopLeft();
                aTopLeft -= getListBox()->GetBoundingRect( pParent ).TopLeft();
                aRect = Rectangle( aTopLeft, aRect.GetSize() );
            }
        }

        return aRect;
    }
    // -----------------------------------------------------------------------------
    Rectangle AccessibleListBoxEntry::GetBoundingBoxOnScreen_Impl() const
    {
        Rectangle aRect;
        SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( pEntry )
        {
            aRect = getListBox()->GetBoundingRect( pEntry );
            Point aTopLeft = aRect.TopLeft();
            aTopLeft += getListBox()->GetWindowExtentsRelative( NULL ).TopLeft();
            aRect = Rectangle( aTopLeft, aRect.GetSize() );
        }

        return aRect;
    }
    // -----------------------------------------------------------------------------
    sal_Bool AccessibleListBoxEntry::IsAlive_Impl() const
    {
        return ( !rBHelper.bDisposed && !rBHelper.bInDispose && isAlive() );
    }
    // -----------------------------------------------------------------------------
    sal_Bool AccessibleListBoxEntry::IsShowing_Impl() const
    {
        Reference< XAccessible > xParent = implGetParentAccessible( );

        sal_Bool bShowing = sal_False;
        Reference< XAccessibleContext > m_xParentContext =
            xParent.is() ? xParent->getAccessibleContext() : Reference< XAccessibleContext >();
        if( m_xParentContext.is() )
        {
            Reference< XAccessibleComponent > xParentComp( m_xParentContext, uno::UNO_QUERY );
            if( xParentComp.is() )
                bShowing = GetBoundingBox_Impl().IsOver( VCLRectangle( xParentComp->getBounds() ) );
        }

        return bShowing;
    }
    // -----------------------------------------------------------------------------
    Rectangle AccessibleListBoxEntry::GetBoundingBox() throw ( lang::DisposedException )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        return GetBoundingBox_Impl();
    }
    // -----------------------------------------------------------------------------
    Rectangle AccessibleListBoxEntry::GetBoundingBoxOnScreen() throw ( lang::DisposedException )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        return GetBoundingBoxOnScreen_Impl();
    }
    // -----------------------------------------------------------------------------
    void AccessibleListBoxEntry::EnsureIsAlive() const throw ( lang::DisposedException )
    {
        if ( !IsAlive_Impl() )
            throw lang::DisposedException();
    }
    // -----------------------------------------------------------------------------
    OUString AccessibleListBoxEntry::implGetText()
    {
        OUString sRet;
        SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( pEntry )
            sRet = getListBox()->SearchEntryText( pEntry );
        return sRet;
    }
    // -----------------------------------------------------------------------------
    Locale AccessibleListBoxEntry::implGetLocale()
    {
        Locale aLocale;
        aLocale = Application::GetSettings().GetUILocale();

        return aLocale;
    }
    void AccessibleListBoxEntry::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
    {
        nStartIndex = 0;
        nEndIndex = 0;
    }
    // -----------------------------------------------------------------------------
    // XTypeProvider
    // -----------------------------------------------------------------------------
    // -----------------------------------------------------------------------------
    Sequence< sal_Int8 > AccessibleListBoxEntry::getImplementationId() throw (RuntimeException)
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
    // XComponent/ListBoxAccessibleBase
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::dispose() throw ( uno::RuntimeException )
    {
        AccessibleListBoxEntry_BASE::dispose();
    }

    // -----------------------------------------------------------------------------
    // XComponent
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::disposing()
    {
        SolarMutexGuard();
        ::osl::MutexGuard aGuard( m_aMutex );

        Reference< XAccessible > xKeepAlive( this );

        // Send a disposing to all listeners.
        if ( m_nClientId )
        {
            ::comphelper::AccessibleEventNotifier::TClientId nId = m_nClientId;
            m_nClientId =  0;
            ::comphelper::AccessibleEventNotifier::revokeClientNotifyDisposing( nId, *this );
        }

        // clean up
        {

            ListBoxAccessibleBase::disposing();
        }
        m_aParent = WeakReference< XAccessible >();
    }
    // -----------------------------------------------------------------------------
    // XServiceInfo
    // -----------------------------------------------------------------------------
    OUString SAL_CALL AccessibleListBoxEntry::getImplementationName() throw(RuntimeException)
    {
        return getImplementationName_Static();
    }
    // -----------------------------------------------------------------------------
    Sequence< OUString > SAL_CALL AccessibleListBoxEntry::getSupportedServiceNames() throw(RuntimeException)
    {
        return getSupportedServiceNames_Static();
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleListBoxEntry::supportsService( const OUString& _rServiceName ) throw (RuntimeException)
    {
        return cppu::supportsService(this, _rServiceName);
    }
    // -----------------------------------------------------------------------------
    // XServiceInfo - static methods
    // -----------------------------------------------------------------------------
    Sequence< OUString > AccessibleListBoxEntry::getSupportedServiceNames_Static(void) throw( RuntimeException )
    {
        Sequence< OUString > aSupported(3);
        aSupported[0] = "com.sun.star.accessibility.AccessibleContext";
        aSupported[1] = "com.sun.star.accessibility.AccessibleComponent";
        aSupported[2] = "com.sun.star.awt.AccessibleTreeListBoxEntry";
        return aSupported;
    }
    // -----------------------------------------------------------------------------
    OUString AccessibleListBoxEntry::getImplementationName_Static(void) throw( RuntimeException )
    {
        return OUString( "com.sun.star.comp.svtools.AccessibleTreeListBoxEntry" );
    }
    // -----------------------------------------------------------------------------
    // XAccessible
    // -----------------------------------------------------------------------------
    Reference< XAccessibleContext > SAL_CALL AccessibleListBoxEntry::getAccessibleContext(  ) throw (RuntimeException)
    {
        EnsureIsAlive();
        return this;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleContext
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
        sal_Int32 nCount = 0;
        if ( pEntry )
            nCount = getListBox()->GetLevelChildCount( pEntry );

        return nCount;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBoxEntry::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException,RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
        SvLBoxEntry* pEntry = pParent ? getListBox()->GetEntry( pParent, i ) : NULL;
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        return new AccessibleListBoxEntry( *getListBox(), pEntry, this );
    }

    // -----------------------------------------------------------------------------
    Reference< XAccessible > AccessibleListBoxEntry::implGetParentAccessible( ) const
    {
        Reference< XAccessible > xParent = (Reference< XAccessible >)m_aParent;
        if ( !xParent.is() )
        {
            OSL_ENSURE( m_aEntryPath.size(), "AccessibleListBoxEntry::getAccessibleParent: invalid path!" );
            if ( 1 == m_aEntryPath.size() )
            {   // we're a top level entry
                // -> our parent is the tree listbox itself
                if ( getListBox() )
                    xParent = getListBox()->GetAccessible( );
            }
            else
            {   // we have a entry as parent -> get it's accessible

                // shorten our access path by one
                ::std::deque< sal_Int32 > aParentPath( m_aEntryPath );
                aParentPath.pop_back();

                // get the entry for this shortened access path
                SvLBoxEntry* pParentEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
                OSL_ENSURE( pParentEntry, "AccessibleListBoxEntry::implGetParentAccessible: could not obtain a parent entry!" );

                if ( pParentEntry )
                    xParent = new AccessibleListBoxEntry( *getListBox(), pParentEntry, NULL );
                    // note that we pass NULL here as parent-accessible:
                    // this is allowed, as the AccessibleListBoxEntry class will create it's parent
                    // when needed
            }
        }

        return xParent;
    }

    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBoxEntry::getAccessibleParent(  ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        return implGetParentAccessible( );
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getAccessibleIndexInParent(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        OSL_ENSURE( !m_aEntryPath.empty(), "empty path" );
        return m_aEntryPath.empty() ? -1 : m_aEntryPath.back();
    }
    // -----------------------------------------------------------------------------
    sal_Int16 SAL_CALL AccessibleListBoxEntry::getAccessibleRole(  ) throw (RuntimeException)
    {
        return AccessibleRole::LABEL;
    }
    // -----------------------------------------------------------------------------
    OUString SAL_CALL AccessibleListBoxEntry::getAccessibleDescription(  ) throw (RuntimeException)
    {
        // no description for every item
        return OUString();
    }
    // -----------------------------------------------------------------------------
    OUString SAL_CALL AccessibleListBoxEntry::getAccessibleName(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        return implGetText();
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessibleRelationSet > SAL_CALL AccessibleListBoxEntry::getAccessibleRelationSet(  ) throw (RuntimeException)
    {
        Reference< XAccessibleRelationSet > xRelSet;
        Reference< XAccessible > xParent;
        if ( m_aEntryPath.size() > 1 ) // not a root entry
            xParent = implGetParentAccessible();
        if ( xParent.is() )
        {
            utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
            Sequence< Reference< XInterface > > aSequence(1);
            aSequence[0] = xParent;
            pRelationSetHelper->AddRelation(
                AccessibleRelation( AccessibleRelationType::NODE_CHILD_OF, aSequence ) );
            xRelSet = pRelationSetHelper;
        }
        return xRelSet;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessibleStateSet > SAL_CALL AccessibleListBoxEntry::getAccessibleStateSet(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
        Reference< XAccessibleStateSet > xStateSet = pStateSetHelper;

        if ( IsAlive_Impl() )
        {
               pStateSetHelper->AddState( AccessibleStateType::TRANSIENT );
               pStateSetHelper->AddState( AccessibleStateType::SELECTABLE );
               pStateSetHelper->AddState( AccessibleStateType::ENABLED );
            pStateSetHelper->AddState( AccessibleStateType::SENSITIVE );
            if ( getListBox()->IsInplaceEditingEnabled() )
                   pStateSetHelper->AddState( AccessibleStateType::EDITABLE );
            if ( IsShowing_Impl() )
                pStateSetHelper->AddState( AccessibleStateType::SHOWING );
            getListBox()->FillAccessibleEntryStateSet(
                getListBox()->GetEntryFromPath( m_aEntryPath ), *pStateSetHelper );
        }
        else
            pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

        return xStateSet;
    }
    // -----------------------------------------------------------------------------
    Locale SAL_CALL AccessibleListBoxEntry::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        return implGetLocale();
    }
    // -----------------------------------------------------------------------------
    // XAccessibleComponent
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleListBoxEntry::containsPoint( const awt::Point& rPoint ) throw (RuntimeException)
    {
        return Rectangle( Point(), GetBoundingBox().GetSize() ).IsInside( VCLPoint( rPoint ) );
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBoxEntry::getAccessibleAtPoint( const awt::Point& _aPoint ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        SvLBoxEntry* pEntry = getListBox()->GetEntry( VCLPoint( _aPoint ) );
        if ( !pEntry )
            throw RuntimeException();

        Reference< XAccessible > xAcc;
        AccessibleListBoxEntry* pAccEntry = new AccessibleListBoxEntry( *getListBox(), pEntry, this );
        Rectangle aRect = pAccEntry->GetBoundingBox_Impl();
        if ( aRect.IsInside( VCLPoint( _aPoint ) ) )
            xAcc = pAccEntry;
        return xAcc;
    }
    // -----------------------------------------------------------------------------
    awt::Rectangle SAL_CALL AccessibleListBoxEntry::getBounds(  ) throw (RuntimeException)
    {
        return AWTRectangle( GetBoundingBox() );
    }
    // -----------------------------------------------------------------------------
    awt::Point SAL_CALL AccessibleListBoxEntry::getLocation(  ) throw (RuntimeException)
    {
        return AWTPoint( GetBoundingBox().TopLeft() );
    }
    // -----------------------------------------------------------------------------
    awt::Point SAL_CALL AccessibleListBoxEntry::getLocationOnScreen(  ) throw (RuntimeException)
    {
        return AWTPoint( GetBoundingBoxOnScreen().TopLeft() );
    }
    // -----------------------------------------------------------------------------
    awt::Size SAL_CALL AccessibleListBoxEntry::getSize(  ) throw (RuntimeException)
    {
        return AWTSize( GetBoundingBox().GetSize() );
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::grabFocus(  ) throw (RuntimeException)
    {
        // do nothing, because no focus for each item
    }
    // -----------------------------------------------------------------------------
    sal_Int32 AccessibleListBoxEntry::getForeground(    ) throw (RuntimeException)
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
    sal_Int32 AccessibleListBoxEntry::getBackground(  ) throw (RuntimeException)
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
    awt::Rectangle SAL_CALL AccessibleListBoxEntry::getCharacterBounds( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        if ( !implIsValidIndex( nIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        awt::Rectangle aBounds( 0, 0, 0, 0 );
        SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( pEntry )
        {
            ::vcl::ControlLayoutData aLayoutData;
            Rectangle aItemRect = GetBoundingBox();
            getListBox()->RecordLayoutData( &aLayoutData, aItemRect );
            Rectangle aCharRect = aLayoutData.GetCharacterBounds( nIndex );
            aCharRect.Move( -aItemRect.Left(), -aItemRect.Top() );
            aBounds = AWTRectangle( aCharRect );
        }

        return aBounds;
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getIndexAtPoint( const awt::Point& aPoint ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        sal_Int32 nIndex = -1;
        SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( pEntry )
        {
            ::vcl::ControlLayoutData aLayoutData;
            Rectangle aItemRect = GetBoundingBox();
            getListBox()->RecordLayoutData( &aLayoutData, aItemRect );
            Point aPnt( VCLPoint( aPoint ) );
            aPnt += aItemRect.TopLeft();
            nIndex = aLayoutData.GetIndexForPoint( aPnt );
        }

        return nIndex;
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleListBoxEntry::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        String sText = getText();
        if  ( ( 0 > nStartIndex ) || ( sText.Len() <= nStartIndex )
            || ( 0 > nEndIndex ) || ( sText.Len() <= nEndIndex ) )
            throw IndexOutOfBoundsException();

        sal_Int32 nLen = nEndIndex - nStartIndex + 1;
        ::svt::OStringTransfer::CopyString( sText.Copy( (sal_uInt16)nStartIndex, (sal_uInt16)nLen ), getListBox() );

        return sal_True;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleEventBroadcaster
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::addEventListener( const Reference< XAccessibleEventListener >& xListener ) throw (RuntimeException)
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
    void SAL_CALL AccessibleListBoxEntry::removeEventListener( const Reference< XAccessibleEventListener >& xListener ) throw (RuntimeException)
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
    // -----------------------------------------------------------------------------
    // XAccessibleAction
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getAccessibleActionCount(  ) throw (RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // three actions supported
        return ACCESSIBLE_ACTION_COUNT;
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleListBoxEntry::doAccessibleAction( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        sal_Bool bRet = sal_False;
        checkActionIndex_Impl( nIndex );
        EnsureIsAlive();

        SvLBoxEntry* pEntry = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( pEntry )
        {
            if ( getListBox()->IsExpanded( pEntry ) )
                getListBox()->Collapse( pEntry );
            else
                getListBox()->Expand( pEntry );
            bRet = sal_True;
        }

        return bRet;
    }
    // -----------------------------------------------------------------------------
    OUString SAL_CALL AccessibleListBoxEntry::getAccessibleActionDescription( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        checkActionIndex_Impl( nIndex );
        EnsureIsAlive();

        static const OUString sActionDesc( "toggleExpand" );
        return sActionDesc;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessibleKeyBinding > AccessibleListBoxEntry::getAccessibleActionKeyBinding( sal_Int32 nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Reference< XAccessibleKeyBinding > xRet;
        checkActionIndex_Impl( nIndex );
        // ... which key?
        return xRet;
    }
    // -----------------------------------------------------------------------------
    // XAccessibleSelection
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::selectAccessibleChild( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
        SvLBoxEntry* pEntry = getListBox()->GetEntry( pParent, nChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        getListBox()->Select( pEntry, sal_True );
    }
    // -----------------------------------------------------------------------------
    sal_Bool SAL_CALL AccessibleListBoxEntry::isAccessibleChildSelected( sal_Int32 nChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
        SvLBoxEntry* pEntry = getListBox()->GetEntry( pParent, nChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        return getListBox()->IsSelected( pEntry );
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::clearAccessibleSelection(  ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( !pParent )
            throw RuntimeException();
        sal_Int32 nCount = getListBox()->GetLevelChildCount( pParent );
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = getListBox()->GetEntry( pParent, i );
            if ( getListBox()->IsSelected( pEntry ) )
                getListBox()->Select( pEntry, sal_False );
        }
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::selectAllAccessibleChildren(  ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( !pParent )
            throw RuntimeException();
        sal_Int32 nCount = getListBox()->GetLevelChildCount( pParent );
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = getListBox()->GetEntry( pParent, i );
            if ( !getListBox()->IsSelected( pEntry ) )
                getListBox()->Select( pEntry, sal_True );
        }
    }
    // -----------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getSelectedAccessibleChildCount(  ) throw (RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        sal_Int32 i, nSelCount = 0, nCount = 0;

        SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( !pParent )
            throw RuntimeException();
        nCount = getListBox()->GetLevelChildCount( pParent );
        for ( i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = getListBox()->GetEntry( pParent, i );
            if ( getListBox()->IsSelected( pEntry ) )
                ++nSelCount;
        }

        return nSelCount;
    }
    // -----------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleListBoxEntry::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xChild;
        sal_Int32 i, nSelCount = 0, nCount = 0;

        SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
        if ( !pParent )
            throw RuntimeException();
        nCount = getListBox()->GetLevelChildCount( pParent );
        for ( i = 0; i < nCount; ++i )
        {
            SvLBoxEntry* pEntry = getListBox()->GetEntry( pParent, i );
            if ( getListBox()->IsSelected( pEntry ) )
                ++nSelCount;

            if ( nSelCount == ( nSelectedChildIndex + 1 ) )
            {
                xChild = new AccessibleListBoxEntry( *getListBox(), pEntry, this );
                break;
            }
        }

        return xChild;
    }
    // -----------------------------------------------------------------------------
    void SAL_CALL AccessibleListBoxEntry::deselectAccessibleChild( sal_Int32 nSelectedChildIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        SvLBoxEntry* pParent = getListBox()->GetEntryFromPath( m_aEntryPath );
        SvLBoxEntry* pEntry = getListBox()->GetEntry( pParent, nSelectedChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        getListBox()->Select( pEntry, sal_False );
    }
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getCaretPosition(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        return -1;
    }
    sal_Bool SAL_CALL AccessibleListBoxEntry::setCaretPosition ( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        if ( !implIsValidRange( nIndex, nIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return sal_False;
    }
    sal_Unicode SAL_CALL AccessibleListBoxEntry::getCharacter( sal_Int32 nIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getCharacter( nIndex );
    }
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL AccessibleListBoxEntry::getCharacterAttributes( sal_Int32 nIndex, const ::com::sun::star::uno::Sequence< OUString >& ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        OUString sText( implGetText() );

        if ( !implIsValidIndex( nIndex, sText.getLength() ) )
            throw IndexOutOfBoundsException();

        return ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >();
    }
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getCharacterCount(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getCharacterCount(  );
    }

    OUString SAL_CALL AccessibleListBoxEntry::getSelectedText(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getSelectedText(  );
    }
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getSelectionStart(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getSelectionStart(  );
    }
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getSelectionEnd(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getSelectionEnd(  );
    }
    sal_Bool SAL_CALL AccessibleListBoxEntry::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return sal_False;
    }
    OUString SAL_CALL AccessibleListBoxEntry::getText(  ) throw (::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getText(  );
    }
    OUString SAL_CALL AccessibleListBoxEntry::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getTextRange( nStartIndex, nEndIndex );
    }
    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleListBoxEntry::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getTextAtIndex( nIndex ,aTextType);
    }
    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleListBoxEntry::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getTextBeforeIndex( nIndex ,aTextType);
    }
    ::com::sun::star::accessibility::TextSegment SAL_CALL AccessibleListBoxEntry::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType ) throw (::com::sun::star::lang::IndexOutOfBoundsException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException)
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        return OCommonAccessibleText::getTextBehindIndex( nIndex ,aTextType);
    }
//........................................................................
}// namespace accessibility
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
