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

#include <extended/accessiblelistboxentry.hxx>
#include <extended/accessiblelistbox.hxx>
#include <vcl/treelistbox.hxx>
#include <svtools/stringtransfer.hxx>
#include <vcl/toolkit/svlbitm.hxx>
#include <com/sun/star/awt/Rectangle.hpp>
#include <com/sun/star/accessibility/AccessibleRelationType.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/IndexOutOfBoundsException.hpp>
#include <i18nlangtag/languagetag.hxx>
#include <vcl/svapp.hxx>
#include <vcl/toolkit/controllayout.hxx>
#include <vcl/settings.hxx>
#include <toolkit/helper/convert.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/accessibleeventnotifier.hxx>
#include <helper/accresmgr.hxx>
#include <strings.hrc>

#define ACCESSIBLE_ACTION_COUNT 1

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
    // class AccessibleListBoxEntry -----------------------------------------------------

    using namespace ::com::sun::star::accessibility;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star;
    using namespace ::comphelper;


    // Ctor() and Dtor()

    AccessibleListBoxEntry::AccessibleListBoxEntry( SvTreeListBox& _rListBox,
                                                    SvTreeListEntry& rEntry,
                                                    AccessibleListBox & rListBox)
        : AccessibleListBoxEntry_BASE( m_aMutex )

        , m_pTreeListBox( &_rListBox )
        , m_pSvLBoxEntry(&rEntry)
        , m_nClientId( 0 )
        , m_wListBox(&rListBox)
        , m_rListBox(rListBox)
    {
        m_pTreeListBox->AddEventListener( LINK( this, AccessibleListBoxEntry, WindowEventListener ) );
        _rListBox.FillEntryPath( m_pSvLBoxEntry, m_aEntryPath );
    }

    AccessibleListBoxEntry::~AccessibleListBoxEntry()
    {
        if ( IsAlive_Impl() )
        {
            // increment ref count to prevent double call of Dtor
            osl_atomic_increment( &m_refCount );
            dispose();
        }
    }

    IMPL_LINK( AccessibleListBoxEntry, WindowEventListener, VclWindowEvent&, rEvent, void )
    {
        OSL_ENSURE( rEvent.GetWindow() , "AccessibleListBoxEntry::WindowEventListener: no event window!" );
        OSL_ENSURE( rEvent.GetWindow() == m_pTreeListBox, "AccessibleListBoxEntry::WindowEventListener: where did this come from?" );

        if ( m_pTreeListBox == nullptr )
            return;

        switch ( rEvent.GetId() )
        {
            case  VclEventId::ObjectDying :
            {
                if ( m_pTreeListBox )
                    m_pTreeListBox->RemoveEventListener( LINK( this, AccessibleListBoxEntry, WindowEventListener ) );
                m_pTreeListBox = nullptr;
                dispose();
                break;
            }
            default: break;
        }
    }

    void AccessibleListBoxEntry::NotifyAccessibleEvent( sal_Int16 _nEventId,
                                                   const css::uno::Any& _aOldValue,
                                                   const css::uno::Any& _aNewValue )
    {
        Reference< uno::XInterface > xSource( *this );
        AccessibleEventObject aEventObj( xSource, _nEventId, _aNewValue, _aOldValue );

        if (m_nClientId)
            comphelper::AccessibleEventNotifier::addEvent( m_nClientId, aEventObj );
    }


    tools::Rectangle AccessibleListBoxEntry::GetBoundingBox_Impl() const
    {
        tools::Rectangle aRect;
        SvTreeListEntry* pEntry = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
        if ( pEntry )
        {
            aRect = m_pTreeListBox->GetBoundingRect( pEntry );
            SvTreeListEntry* pParent = m_pTreeListBox->GetParent( pEntry );
            if ( pParent )
            {
                // position relative to parent entry
                Point aTopLeft = aRect.TopLeft();
                aTopLeft -= m_pTreeListBox->GetBoundingRect( pParent ).TopLeft();
                aRect = tools::Rectangle( aTopLeft, aRect.GetSize() );
            }
        }

        return aRect;
    }

    tools::Rectangle AccessibleListBoxEntry::GetBoundingBoxOnScreen_Impl() const
    {
        tools::Rectangle aRect;
        SvTreeListEntry* pEntry = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
        if ( pEntry )
        {
            aRect = m_pTreeListBox->GetBoundingRect( pEntry );
            Point aTopLeft = aRect.TopLeft();
            aTopLeft += m_pTreeListBox->GetWindowExtentsRelative( nullptr ).TopLeft();
            aRect = tools::Rectangle( aTopLeft, aRect.GetSize() );
        }

        return aRect;
    }

    bool AccessibleListBoxEntry::IsAlive_Impl() const
    {
        return !rBHelper.bDisposed && !rBHelper.bInDispose && (m_pTreeListBox != nullptr);
    }

    bool AccessibleListBoxEntry::IsShowing_Impl() const
    {
        Reference< XAccessible > xParent = implGetParentAccessible( );

        bool bShowing = false;
        Reference< XAccessibleContext > xParentContext =
            xParent.is() ? xParent->getAccessibleContext() : Reference< XAccessibleContext >();
        if( xParentContext.is() )
        {
            Reference< XAccessibleComponent > xParentComp( xParentContext, uno::UNO_QUERY );
            if( xParentComp.is() )
                bShowing = GetBoundingBox_Impl().IsOver( VCLRectangle( xParentComp->getBounds() ) );
        }

        return bShowing;
    }

    tools::Rectangle AccessibleListBoxEntry::GetBoundingBox()
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        return GetBoundingBox_Impl();
    }

    tools::Rectangle AccessibleListBoxEntry::GetBoundingBoxOnScreen()
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        return GetBoundingBoxOnScreen_Impl();
    }

    void AccessibleListBoxEntry::EnsureIsAlive() const
    {
        if ( !IsAlive_Impl() )
            throw lang::DisposedException();
    }

    OUString AccessibleListBoxEntry::implGetText()
    {
        OUString sRet;
        SvTreeListEntry* pEntry = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
        if ( pEntry )
            sRet = SvTreeListBox::SearchEntryTextWithHeadTitle( pEntry );
        return sRet;
    }

    Locale AccessibleListBoxEntry::implGetLocale()
    {
        return Application::GetSettings().GetUILanguageTag().getLocale();
    }
    void AccessibleListBoxEntry::implGetSelection( sal_Int32& nStartIndex, sal_Int32& nEndIndex )
    {
        nStartIndex = 0;
        nEndIndex = 0;
    }

    // XTypeProvider


    Sequence< sal_Int8 > AccessibleListBoxEntry::getImplementationId()
    {
        return css::uno::Sequence<sal_Int8>();
    }


    // XComponent

    void SAL_CALL AccessibleListBoxEntry::disposing()
    {
        SolarMutexGuard aSolarGuard;
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
        m_wListBox.clear();

        if ( m_pTreeListBox )
            m_pTreeListBox->RemoveEventListener( LINK( this, AccessibleListBoxEntry, WindowEventListener ) );
        m_pTreeListBox = nullptr;
    }

    // XServiceInfo

    OUString SAL_CALL AccessibleListBoxEntry::getImplementationName()
    {
        return "com.sun.star.comp.svtools.AccessibleTreeListBoxEntry";
    }

    Sequence< OUString > SAL_CALL AccessibleListBoxEntry::getSupportedServiceNames()
    {
        return {"com.sun.star.accessibility.AccessibleContext",
                "com.sun.star.accessibility.AccessibleComponent",
                "com.sun.star.awt.AccessibleTreeListBoxEntry"};
    }

    sal_Bool SAL_CALL AccessibleListBoxEntry::supportsService( const OUString& _rServiceName )
    {
        return cppu::supportsService(this, _rServiceName);
    }

    // XAccessible

    Reference< XAccessibleContext > SAL_CALL AccessibleListBoxEntry::getAccessibleContext(  )
    {
        EnsureIsAlive();
        return this;
    }

    // XAccessibleContext

    sal_Int32 SAL_CALL AccessibleListBoxEntry::getAccessibleChildCount(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        SvTreeListEntry* pEntry = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
        sal_Int32 nCount = 0;
        if ( pEntry )
            nCount = m_pTreeListBox->GetLevelChildCount( pEntry );

        return nCount;
    }

    Reference< XAccessible > SAL_CALL AccessibleListBoxEntry::getAccessibleChild( sal_Int32 i )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        SvTreeListEntry* pEntry = GetRealChild(i);
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        uno::Reference<XAccessible> xListBox(m_wListBox);
        assert(xListBox.is());

        return m_rListBox.implGetAccessible(*pEntry).get();
    }

    Reference< XAccessible > AccessibleListBoxEntry::implGetParentAccessible( ) const
    {
        Reference< XAccessible > xParent;
        if ( !xParent.is() )
        {
            OSL_ENSURE( m_aEntryPath.size(), "AccessibleListBoxEntry::getAccessibleParent: invalid path!" );
            if ( m_aEntryPath.size() == 1 )
            {   // we're a top level entry
                // -> our parent is the tree listbox itself
                if ( m_pTreeListBox )
                    xParent = m_pTreeListBox->GetAccessible( );
            }
            else
            {   // we have an entry as parent -> get its accessible

                // shorten our access path by one
                std::deque< sal_Int32 > aParentPath( m_aEntryPath );
                aParentPath.pop_back();

                // get the entry for this shortened access path
                SvTreeListEntry* pParentEntry = m_pTreeListBox->GetEntryFromPath( aParentPath );
                OSL_ENSURE( pParentEntry, "AccessibleListBoxEntry::implGetParentAccessible: could not obtain a parent entry!" );

                if ( pParentEntry )
                    pParentEntry = m_pTreeListBox->GetParent(pParentEntry);
                if ( pParentEntry )
                {
                    uno::Reference<XAccessible> xListBox(m_wListBox);
                    assert(xListBox.is());
                    return m_rListBox.implGetAccessible(*pParentEntry).get();
                    // the AccessibleListBoxEntry class will create its parent
                    // when needed
                }
            }
        }

        return xParent;
    }


    Reference< XAccessible > SAL_CALL AccessibleListBoxEntry::getAccessibleParent(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        return implGetParentAccessible( );
    }

    sal_Int32 SAL_CALL AccessibleListBoxEntry::getAccessibleIndexInParent(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        OSL_ENSURE( !m_aEntryPath.empty(), "empty path" );
        return m_aEntryPath.empty() ? -1 : m_aEntryPath.back();
    }

    sal_Int32 AccessibleListBoxEntry::GetRoleType() const
    {
        sal_Int32 nCase = 0;
        SvTreeListEntry* pEntry = m_pTreeListBox->GetEntry(0);
        if ( pEntry )
        {
            if( pEntry->HasChildrenOnDemand() || m_pTreeListBox->GetChildCount(pEntry) > 0  )
            {
                nCase = 1;
                return nCase;
            }
        }

        bool bHasButtons = (m_pTreeListBox->GetStyle() & WB_HASBUTTONS)!=0;
        if( !(m_pTreeListBox->GetTreeFlags() & SvTreeFlags::CHKBTN) )
        {
            if( bHasButtons )
                nCase = 1;
        }
        else
        {
            if( bHasButtons )
                nCase = 2;
            else
                nCase = 3;
        }
        return nCase;
    }

    sal_Int16 SAL_CALL AccessibleListBoxEntry::getAccessibleRole(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        SvTreeListBox* pBox = m_pTreeListBox;
        if(pBox)
        {
            SvTreeFlags treeFlag = pBox->GetTreeFlags();
            if(treeFlag & SvTreeFlags::CHKBTN )
            {
                SvTreeListEntry* pEntry = pBox->GetEntryFromPath( m_aEntryPath );
                SvButtonState eState = pBox->GetCheckButtonState( pEntry );
                switch( eState )
                {
                case SvButtonState::Checked:
                case SvButtonState::Unchecked:
                    return AccessibleRole::CHECK_BOX;
                case SvButtonState::Tristate:
                default:
                    return AccessibleRole::LABEL;
                }
            }
            if (GetRoleType() == 0)
                return AccessibleRole::LIST_ITEM;
            else
                //o is: return AccessibleRole::LABEL;
                return AccessibleRole::TREE_ITEM;
        }
        return AccessibleRole::UNKNOWN;
    }

    OUString SAL_CALL AccessibleListBoxEntry::getAccessibleDescription(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        SvTreeListEntry* pEntry = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
        if( getAccessibleRole() == AccessibleRole::TREE_ITEM )
        {
            return OUString();
        }
        //want to count the real column number in the list box.
        sal_uInt16 iRealItemCount = 0;
        sal_uInt16 iCount = 0;
        sal_uInt16 iTotleItemCount = pEntry->ItemCount();
        while( iCount < iTotleItemCount )
        {
            const SvLBoxItem& rItem = pEntry->GetItem( iCount );
            if ( rItem.GetType() == SvLBoxItemType::String &&
                 !static_cast<const SvLBoxString&>( rItem ).GetText().isEmpty() )
            {
                iRealItemCount++;
            }
            iCount++;
        }
        if(iRealItemCount<=1  )
        {
            return OUString();
        }
        else
        {
            return SvTreeListBox::SearchEntryTextWithHeadTitle( pEntry );
        }
    }

    OUString SAL_CALL AccessibleListBoxEntry::getAccessibleName(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        return implGetText();
    }

    Reference< XAccessibleRelationSet > SAL_CALL AccessibleListBoxEntry::getAccessibleRelationSet(  )
    {
        Reference< XAccessibleRelationSet > xRelSet;
        Reference< XAccessible > xParent;
        if ( m_aEntryPath.size() > 1 ) // not a root entry
            xParent = implGetParentAccessible();
        if ( xParent.is() )
        {
            utl::AccessibleRelationSetHelper* pRelationSetHelper = new utl::AccessibleRelationSetHelper;
            Sequence< Reference< XInterface > > aSequence { xParent };
            pRelationSetHelper->AddRelation(
                AccessibleRelation( AccessibleRelationType::NODE_CHILD_OF, aSequence ) );
            xRelSet = pRelationSetHelper;
        }
        return xRelSet;
    }

    Reference< XAccessibleStateSet > SAL_CALL AccessibleListBoxEntry::getAccessibleStateSet(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        utl::AccessibleStateSetHelper* pStateSetHelper = new utl::AccessibleStateSetHelper;
        Reference< XAccessibleStateSet > xStateSet = pStateSetHelper;

        if ( IsAlive_Impl() )
        {
            switch(getAccessibleRole())
            {
                case AccessibleRole::LABEL:
                    pStateSetHelper->AddState( AccessibleStateType::TRANSIENT );
                    pStateSetHelper->AddState( AccessibleStateType::SELECTABLE );
                    pStateSetHelper->AddState( AccessibleStateType::ENABLED );
                    if (m_pTreeListBox->IsInplaceEditingEnabled())
                        pStateSetHelper->AddState( AccessibleStateType::EDITABLE );
                    if (IsShowing_Impl())
                        pStateSetHelper->AddState( AccessibleStateType::SHOWING );
                    break;
                case AccessibleRole::CHECK_BOX:
                    pStateSetHelper->AddState( AccessibleStateType::TRANSIENT );
                    pStateSetHelper->AddState( AccessibleStateType::SELECTABLE );
                    pStateSetHelper->AddState( AccessibleStateType::ENABLED );
                    if (IsShowing_Impl())
                        pStateSetHelper->AddState( AccessibleStateType::SHOWING );
                    break;
            }
            SvTreeListEntry *pEntry = m_pTreeListBox->GetEntryFromPath(m_aEntryPath);
            if (pEntry)
                m_pTreeListBox->FillAccessibleEntryStateSet(pEntry, *pStateSetHelper);
        }
        else
            pStateSetHelper->AddState( AccessibleStateType::DEFUNC );

        return xStateSet;
    }

    Locale SAL_CALL AccessibleListBoxEntry::getLocale(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        return implGetLocale();
    }

    // XAccessibleComponent

    sal_Bool SAL_CALL AccessibleListBoxEntry::containsPoint( const awt::Point& rPoint )
    {
        return tools::Rectangle( Point(), GetBoundingBox().GetSize() ).IsInside( VCLPoint( rPoint ) );
    }

    Reference< XAccessible > SAL_CALL AccessibleListBoxEntry::getAccessibleAtPoint( const awt::Point& _aPoint )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();
        SvTreeListEntry* pEntry = m_pTreeListBox->GetEntry( VCLPoint( _aPoint ) );
        if ( !pEntry )
            throw RuntimeException();

        Reference< XAccessible > xAcc;
        uno::Reference<XAccessible> xListBox(m_wListBox);
        assert(xListBox.is());
        auto pAccEntry = m_rListBox.implGetAccessible(*pEntry);
        tools::Rectangle aRect = pAccEntry->GetBoundingBox_Impl();
        if ( aRect.IsInside( VCLPoint( _aPoint ) ) )
            xAcc = pAccEntry.get();
        return xAcc;
    }

    awt::Rectangle SAL_CALL AccessibleListBoxEntry::getBounds(  )
    {
        return AWTRectangle( GetBoundingBox() );
    }

    awt::Point SAL_CALL AccessibleListBoxEntry::getLocation(  )
    {
        return AWTPoint( GetBoundingBox().TopLeft() );
    }

    awt::Point SAL_CALL AccessibleListBoxEntry::getLocationOnScreen(  )
    {
        return AWTPoint( GetBoundingBoxOnScreen().TopLeft() );
    }

    awt::Size SAL_CALL AccessibleListBoxEntry::getSize(  )
    {
        return AWTSize( GetBoundingBox().GetSize() );
    }

    void SAL_CALL AccessibleListBoxEntry::grabFocus(  )
    {
        // do nothing, because no focus for each item
    }

    sal_Int32 AccessibleListBoxEntry::getForeground(    )
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

    sal_Int32 AccessibleListBoxEntry::getBackground(  )
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


    awt::Rectangle SAL_CALL AccessibleListBoxEntry::getCharacterBounds( sal_Int32 nIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        if ( !implIsValidIndex( nIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        awt::Rectangle aBounds( 0, 0, 0, 0 );
        SvTreeListEntry* pEntry = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
        if ( pEntry )
        {
            vcl::ControlLayoutData aLayoutData;
            tools::Rectangle aItemRect = GetBoundingBox();
            m_pTreeListBox->RecordLayoutData( &aLayoutData, aItemRect );
            tools::Rectangle aCharRect = aLayoutData.GetCharacterBounds( nIndex );
            aCharRect.Move( -aItemRect.Left(), -aItemRect.Top() );
            aBounds = AWTRectangle( aCharRect );
        }

        return aBounds;
    }

    sal_Int32 SAL_CALL AccessibleListBoxEntry::getIndexAtPoint( const awt::Point& aPoint )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        if(aPoint.X==0 && aPoint.Y==0) return 0;

        sal_Int32 nIndex = -1;
        SvTreeListEntry* pEntry = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
        if ( pEntry )
        {
            vcl::ControlLayoutData aLayoutData;
            tools::Rectangle aItemRect = GetBoundingBox();
            m_pTreeListBox->RecordLayoutData( &aLayoutData, aItemRect );
            Point aPnt( VCLPoint( aPoint ) );
            aPnt += aItemRect.TopLeft();
            nIndex = aLayoutData.GetIndexForPoint( aPnt );
        }

        return nIndex;
    }

    sal_Bool SAL_CALL AccessibleListBoxEntry::copyText( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        OUString sText = implGetText();
        if  ( ( 0 > nStartIndex ) || ( sText.getLength() <= nStartIndex )
            || ( 0 > nEndIndex ) || ( sText.getLength() <= nEndIndex ) )
            throw IndexOutOfBoundsException();

        sal_Int32 nLen = nEndIndex - nStartIndex + 1;
        ::svt::OStringTransfer::CopyString( sText.copy( nStartIndex, nLen ), m_pTreeListBox );

        return true;
    }

    sal_Bool SAL_CALL AccessibleListBoxEntry::scrollSubstringTo( sal_Int32, sal_Int32, AccessibleScrollType )
    {
        return false;
    }

    // XAccessibleEventBroadcaster

    void SAL_CALL AccessibleListBoxEntry::addAccessibleEventListener( const Reference< XAccessibleEventListener >& xListener )
    {
        if (xListener.is())
        {
            ::osl::MutexGuard aGuard( m_aMutex );
            if (!m_nClientId)
                m_nClientId = comphelper::AccessibleEventNotifier::registerClient( );
            comphelper::AccessibleEventNotifier::addEventListener( m_nClientId, xListener );
        }
    }

    void SAL_CALL AccessibleListBoxEntry::removeAccessibleEventListener( const Reference< XAccessibleEventListener >& xListener )
    {
        if (!xListener.is())
            return;

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

    // XAccessibleAction

    sal_Int32 SAL_CALL AccessibleListBoxEntry::getAccessibleActionCount(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        // three actions supported
        SvTreeFlags treeFlag = m_pTreeListBox->GetTreeFlags();
        bool bHasButtons = (m_pTreeListBox->GetStyle() & WB_HASBUTTONS)!=0;
        if( (treeFlag & SvTreeFlags::CHKBTN) && !bHasButtons)
        {
            sal_Int16 role = getAccessibleRole();
            if ( role == AccessibleRole::CHECK_BOX )
                return 2;
            else if ( role == AccessibleRole::LABEL )
                return 0;
        }
        else
            return ACCESSIBLE_ACTION_COUNT;
        return 0;
    }

    sal_Bool SAL_CALL AccessibleListBoxEntry::doAccessibleAction( sal_Int32 nIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        bool bRet = false;
        checkActionIndex_Impl( nIndex );
        EnsureIsAlive();

        SvTreeFlags treeFlag = m_pTreeListBox->GetTreeFlags();
        if( nIndex == 0 && (treeFlag & SvTreeFlags::CHKBTN) )
        {
            if(getAccessibleRole() == AccessibleRole::CHECK_BOX)
            {
                SvTreeListEntry* pEntry = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
                SvButtonState state = m_pTreeListBox->GetCheckButtonState( pEntry );
                if ( state == SvButtonState::Checked )
                    m_pTreeListBox->SetCheckButtonState(pEntry, SvButtonState::Unchecked);
                else if (state == SvButtonState::Unchecked)
                    m_pTreeListBox->SetCheckButtonState(pEntry, SvButtonState::Checked);
            }
        }
        else if( (nIndex == 1 && (treeFlag & SvTreeFlags::CHKBTN) ) || (nIndex == 0) )
        {
            SvTreeListEntry* pEntry = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
            if ( pEntry )
            {
                if ( m_pTreeListBox->IsExpanded( pEntry ) )
                    m_pTreeListBox->Collapse( pEntry );
                else
                    m_pTreeListBox->Expand( pEntry );
                bRet = true;
            }
        }

        return bRet;
    }

    OUString SAL_CALL AccessibleListBoxEntry::getAccessibleActionDescription( sal_Int32 nIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        checkActionIndex_Impl( nIndex );
        EnsureIsAlive();

        SvTreeListEntry* pEntry = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
        SvButtonState state = m_pTreeListBox->GetCheckButtonState( pEntry );
        SvTreeFlags treeFlag = m_pTreeListBox->GetTreeFlags();
        if(nIndex == 0 && (treeFlag & SvTreeFlags::CHKBTN))
        {
            if(getAccessibleRole() == AccessibleRole::CHECK_BOX)
            {
                if ( state == SvButtonState::Checked )
                    return "UnCheck";
                else if (state == SvButtonState::Unchecked)
                    return "Check";
            }
            else
            {
                //Sometimes, a List or Tree may have both checkbox and label at the same time
                return OUString();
            }
        }else if( (nIndex == 1 && (treeFlag & SvTreeFlags::CHKBTN)) || nIndex == 0 )
        {
            if( pEntry->HasChildren() || pEntry->HasChildrenOnDemand() )
                return m_pTreeListBox->IsExpanded( pEntry ) ?
                AccResId(STR_SVT_ACC_ACTION_COLLAPSE) :
                AccResId(STR_SVT_ACC_ACTION_EXPAND);
            return OUString();

        }
        throw IndexOutOfBoundsException();
    }

    Reference< XAccessibleKeyBinding > AccessibleListBoxEntry::getAccessibleActionKeyBinding( sal_Int32 nIndex )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Reference< XAccessibleKeyBinding > xRet;
        checkActionIndex_Impl( nIndex );
        // ... which key?
        return xRet;
    }

    // XAccessibleSelection

    void SAL_CALL AccessibleListBoxEntry::selectAccessibleChild( sal_Int32 nChildIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        SvTreeListEntry* pEntry = GetRealChild(nChildIndex);
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        m_pTreeListBox->Select( pEntry );
    }

    sal_Bool SAL_CALL AccessibleListBoxEntry::isAccessibleChildSelected( sal_Int32 nChildIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        SvTreeListEntry* pParent = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
        SvTreeListEntry* pEntry = m_pTreeListBox->GetEntry( pParent, nChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        return m_pTreeListBox->IsSelected( pEntry );
    }

    void SAL_CALL AccessibleListBoxEntry::clearAccessibleSelection(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        SvTreeListEntry* pParent = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
        if ( !pParent )
            throw RuntimeException();
        sal_Int32 nCount = m_pTreeListBox->GetLevelChildCount( pParent );
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            SvTreeListEntry* pEntry = m_pTreeListBox->GetEntry( pParent, i );
            if ( m_pTreeListBox->IsSelected( pEntry ) )
                m_pTreeListBox->Select( pEntry, false );
        }
    }

    void SAL_CALL AccessibleListBoxEntry::selectAllAccessibleChildren(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        SvTreeListEntry* pParent = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
        if ( !pParent )
            throw RuntimeException();
        sal_Int32 nCount = m_pTreeListBox->GetLevelChildCount( pParent );
        for ( sal_Int32 i = 0; i < nCount; ++i )
        {
            SvTreeListEntry* pEntry = m_pTreeListBox->GetEntry( pParent, i );
            if ( !m_pTreeListBox->IsSelected( pEntry ) )
                m_pTreeListBox->Select( pEntry );
        }
    }

    sal_Int32 SAL_CALL AccessibleListBoxEntry::getSelectedAccessibleChildCount(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        sal_Int32 i, nSelCount = 0, nCount = 0;

        SvTreeListEntry* pParent = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
        if ( !pParent )
            throw RuntimeException();
        nCount = m_pTreeListBox->GetLevelChildCount( pParent );
        for ( i = 0; i < nCount; ++i )
        {
            SvTreeListEntry* pEntry = m_pTreeListBox->GetEntry( pParent, i );
            if ( m_pTreeListBox->IsSelected( pEntry ) )
                ++nSelCount;
        }

        return nSelCount;
    }

    Reference< XAccessible > SAL_CALL AccessibleListBoxEntry::getSelectedAccessibleChild( sal_Int32 nSelectedChildIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        if ( nSelectedChildIndex < 0 || nSelectedChildIndex >= getSelectedAccessibleChildCount() )
            throw IndexOutOfBoundsException();

        Reference< XAccessible > xChild;
        sal_Int32 i, nSelCount = 0, nCount = 0;

        SvTreeListEntry* pParent = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
        if ( !pParent )
            throw RuntimeException();
        nCount = m_pTreeListBox->GetLevelChildCount( pParent );
        for ( i = 0; i < nCount; ++i )
        {
            SvTreeListEntry* pEntry = m_pTreeListBox->GetEntry( pParent, i );
            if ( m_pTreeListBox->IsSelected( pEntry ) )
                ++nSelCount;

            if ( nSelCount == ( nSelectedChildIndex + 1 ) )
            {
                uno::Reference<XAccessible> xListBox(m_wListBox);
                assert(xListBox.is());
                xChild = m_rListBox.implGetAccessible(*pEntry).get();
                break;
            }
        }

        return xChild;
    }

    void SAL_CALL AccessibleListBoxEntry::deselectAccessibleChild( sal_Int32 nSelectedChildIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );

        EnsureIsAlive();

        SvTreeListEntry* pParent = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
        SvTreeListEntry* pEntry = m_pTreeListBox->GetEntry( pParent, nSelectedChildIndex );
        if ( !pEntry )
            throw IndexOutOfBoundsException();

        m_pTreeListBox->Select( pEntry, false );
    }
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getCaretPosition(  )
    {
        return -1;
    }
    sal_Bool SAL_CALL AccessibleListBoxEntry::setCaretPosition ( sal_Int32 nIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        if ( !implIsValidRange( nIndex, nIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return false;
    }
    sal_Unicode SAL_CALL AccessibleListBoxEntry::getCharacter( sal_Int32 nIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::implGetCharacter( implGetText(), nIndex );
    }
    css::uno::Sequence< css::beans::PropertyValue > SAL_CALL AccessibleListBoxEntry::getCharacterAttributes( sal_Int32 nIndex, const css::uno::Sequence< OUString >& )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        OUString sText( implGetText() );

        if ( !implIsValidIndex( nIndex, sText.getLength() ) )
            throw IndexOutOfBoundsException();

        return css::uno::Sequence< css::beans::PropertyValue >();
    }
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getCharacterCount(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return implGetText().getLength();
    }

    OUString SAL_CALL AccessibleListBoxEntry::getSelectedText(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OUString();
    }
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getSelectionStart(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return 0;
    }
    sal_Int32 SAL_CALL AccessibleListBoxEntry::getSelectionEnd(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return 0;
    }
    sal_Bool SAL_CALL AccessibleListBoxEntry::setSelection( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        if ( !implIsValidRange( nStartIndex, nEndIndex, implGetText().getLength() ) )
            throw IndexOutOfBoundsException();

        return false;
    }
    OUString SAL_CALL AccessibleListBoxEntry::getText(  )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return implGetText(  );
    }
    OUString SAL_CALL AccessibleListBoxEntry::getTextRange( sal_Int32 nStartIndex, sal_Int32 nEndIndex )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::implGetTextRange( implGetText(), nStartIndex, nEndIndex );
    }
    css::accessibility::TextSegment SAL_CALL AccessibleListBoxEntry::getTextAtIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getTextAtIndex( nIndex ,aTextType);
    }
    css::accessibility::TextSegment SAL_CALL AccessibleListBoxEntry::getTextBeforeIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();
        return OCommonAccessibleText::getTextBeforeIndex( nIndex ,aTextType);
    }
    css::accessibility::TextSegment SAL_CALL AccessibleListBoxEntry::getTextBehindIndex( sal_Int32 nIndex, sal_Int16 aTextType )
    {
        SolarMutexGuard aSolarGuard;
        ::osl::MutexGuard aGuard( m_aMutex );
        EnsureIsAlive();

        return OCommonAccessibleText::getTextBehindIndex( nIndex ,aTextType);
    }

    // XAccessibleValue


    Any AccessibleListBoxEntry::getCurrentValue(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        Any aValue;
        sal_Int32 level = static_cast<sal_Int32>(m_aEntryPath.size()) - 1;
        level = level < 0 ?  0: level;
        aValue <<= level;
        return aValue;
    }


    sal_Bool AccessibleListBoxEntry::setCurrentValue( const Any& aNumber )
    {
        ::osl::MutexGuard aGuard( m_aMutex );


        bool bReturn = false;
        SvTreeListBox* pBox = m_pTreeListBox;
        if(getAccessibleRole() == AccessibleRole::CHECK_BOX)
        {
            SvTreeListEntry* pEntry = pBox->GetEntryFromPath( m_aEntryPath );
            if ( pEntry )
            {
                sal_Int32 nValue(0), nValueMin(0), nValueMax(0);
                aNumber >>= nValue;
                getMinimumValue() >>= nValueMin;
                getMaximumValue() >>= nValueMax;

                if ( nValue < nValueMin )
                    nValue = nValueMin;
                else if ( nValue > nValueMax )
                    nValue = nValueMax;

                pBox->SetCheckButtonState(pEntry,  static_cast<SvButtonState>(nValue) );
                bReturn = true;
            }
        }

        return bReturn;
    }


    Any AccessibleListBoxEntry::getMaximumValue(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Any aValue;
        // SvTreeListBox* pBox = m_pTreeListBox;
        switch(getAccessibleRole())
        {
            case AccessibleRole::CHECK_BOX:
                aValue <<= sal_Int32(1);
                break;
            case AccessibleRole::LABEL:
            default:
                break;
        }

        return aValue;
    }


    Any AccessibleListBoxEntry::getMinimumValue(  )
    {
        ::osl::MutexGuard aGuard( m_aMutex );

        Any aValue;
        // SvTreeListBox* pBox = m_pTreeListBox;
        switch(getAccessibleRole())
        {
            case AccessibleRole::CHECK_BOX:
                aValue <<= sal_Int32(0);
                break;
            case AccessibleRole::LABEL:
            default:
                break;
        }

        return aValue;
    }


    SvTreeListEntry* AccessibleListBoxEntry::GetRealChild(sal_Int32 nIndex)
    {
        SvTreeListEntry* pEntry = nullptr;
        SvTreeListEntry* pParent = m_pTreeListBox->GetEntryFromPath( m_aEntryPath );
        if (pParent)
        {
            pEntry = m_pTreeListBox->GetEntry( pParent, nIndex );
            if ( !pEntry && getAccessibleChildCount() > 0 )
            {
                m_pTreeListBox->RequestingChildren(pParent);
                pEntry = m_pTreeListBox->GetEntry( pParent, nIndex );
            }
        }
        return pEntry;
    }

}// namespace accessibility


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
