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

#include "accessibility/extended/AccessibleToolPanelDeckTabBar.hxx"
#include "accessibility/extended/AccessibleToolPanelDeckTabBarItem.hxx"
#include "accessibility/helper/accresmgr.hxx"
#include "accessibility/helper/accessiblestrings.hrc"

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <cppuhelper/implbase.hxx>
#include <svtools/toolpanel/toolpaneldeck.hxx>
#include <svtools/toolpanel/paneltabbar.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/button.hxx>
#include <osl/mutex.hxx>
#include <tools/diagnose_ex.h>
#include <boost/noncopyable.hpp>

#include <vector>

namespace accessibility
{
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::accessibility::XAccessible;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::lang::IndexOutOfBoundsException;
    using ::com::sun::star::accessibility::XAccessibleContext;

    namespace AccessibleRole = ::com::sun::star::accessibility::AccessibleRole;
    namespace AccessibleEventId = ::com::sun::star::accessibility::AccessibleEventId;
    namespace AccessibleStateType = ::com::sun::star::accessibility::AccessibleStateType;

    // AccessibleWrapper
    typedef ::cppu::WeakImplHelper< XAccessible > AccessibleWrapper_Base;
    class AccessibleWrapper : public AccessibleWrapper_Base
    {
    public:
        explicit AccessibleWrapper( const Reference< XAccessibleContext >& i_rContext )
            :m_xContext( i_rContext )
        {
        }

        // XAccessible
        virtual Reference< XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (RuntimeException, std::exception) override
        {
            return m_xContext;
        }

    private:
        const Reference< XAccessibleContext >   m_xContext;
    };

    // AccessibleToolPanelTabBar_Impl
    class AccessibleToolPanelTabBar_Impl    :public ::boost::noncopyable
                                            ,public ::svt::IToolPanelDeckListener
    {
    public:
        AccessibleToolPanelTabBar_Impl(
            AccessibleToolPanelTabBar& i_rAntiImpl,
            const Reference< XAccessible >& i_rAccessibleParent,
            ::svt::IToolPanelDeck& i_rPanelDeck,
            ::svt::PanelTabBar& i_rTabBar
        );
        virtual ~AccessibleToolPanelTabBar_Impl();

        void    checkDisposed();
        bool    isDisposed() const { return m_pPanelDeck == NULL; }
        void    dispose();

        ::svt::IToolPanelDeck*          getPanelDeck() const { return m_pPanelDeck; }
        ::svt::PanelTabBar*             getTabBar() const { return m_pTabBar; }
        const Reference< XAccessible >& getAccessibleParent() const { return m_xAccessibleParent; }
        Reference< XAccessible >        getAccessiblePanelItem( size_t i_nPosition );
        Reference< XAccessible >        getOwnAccessible() const;

    protected:
        // IToolPanelDeckListener
        virtual void PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition ) override;
        virtual void PanelRemoved( const size_t i_nPosition ) override;
        virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive ) override;
        virtual void LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter ) override;
        virtual void Dying() override;

        DECL_LINK_TYPED( OnWindowEvent, VclWindowEvent&, void );

    private:
        AccessibleToolPanelTabBar&                  m_rAntiImpl;
        Reference< XAccessible >                    m_xAccessibleParent;
        ::svt::IToolPanelDeck*                      m_pPanelDeck;
        VclPtr< ::svt::PanelTabBar >                m_pTabBar;
        ::std::vector< Reference< XAccessible > >   m_aChildren;
    };

    AccessibleToolPanelTabBar_Impl::AccessibleToolPanelTabBar_Impl( AccessibleToolPanelTabBar& i_rAntiImpl,
            const Reference< XAccessible >& i_rAccessibleParent, ::svt::IToolPanelDeck& i_rPanelDeck, ::svt::PanelTabBar& i_rTabBar )
        :m_rAntiImpl( i_rAntiImpl )
        ,m_xAccessibleParent( i_rAccessibleParent )
        ,m_pPanelDeck( &i_rPanelDeck )
        ,m_pTabBar( &i_rTabBar )
        ,m_aChildren()
    {
        m_pPanelDeck->AddListener( *this );
        m_aChildren.resize( m_pPanelDeck->GetPanelCount() );

        const OUString sAccessibleDescription( TK_RES_STRING( RID_STR_ACC_DESC_PANELDECL_TABBAR ) );
        i_rTabBar.SetAccessibleName( sAccessibleDescription );
        i_rTabBar.SetAccessibleDescription( sAccessibleDescription );

        i_rTabBar.GetScrollButton( true ).AddEventListener( LINK( this, AccessibleToolPanelTabBar_Impl, OnWindowEvent ) );
        i_rTabBar.GetScrollButton( false ).AddEventListener( LINK( this, AccessibleToolPanelTabBar_Impl, OnWindowEvent ) );
    }

    void AccessibleToolPanelTabBar_Impl::checkDisposed()
    {
        if ( isDisposed() )
            throw DisposedException( OUString(), *&m_rAntiImpl );
    }

    AccessibleToolPanelTabBar_Impl::~AccessibleToolPanelTabBar_Impl()
    {
        if ( !isDisposed() )
            dispose();
    }

    void AccessibleToolPanelTabBar_Impl::dispose()
    {
        ENSURE_OR_RETURN_VOID( !isDisposed(), "disposed twice" );
        m_pPanelDeck->RemoveListener( *this );
        m_pPanelDeck = NULL;

        m_pTabBar->GetScrollButton( true ).RemoveEventListener( LINK( this, AccessibleToolPanelTabBar_Impl, OnWindowEvent ) );
        m_pTabBar->GetScrollButton( false ).RemoveEventListener( LINK( this, AccessibleToolPanelTabBar_Impl, OnWindowEvent ) );
        m_pTabBar.clear();

        m_xAccessibleParent.clear();
    }

    Reference< XAccessible > AccessibleToolPanelTabBar_Impl::getAccessiblePanelItem( size_t i_nPosition )
    {
        ENSURE_OR_RETURN( !isDisposed(), "AccessibleToolPanelTabBar_Impl::getAccessiblePanelItem: already disposed!", NULL );
        ENSURE_OR_RETURN( i_nPosition < m_aChildren.size(), "AccessibleToolPanelTabBar_Impl::getAccessiblePanelItem: invalid index!", NULL );

        Reference< XAccessible >& rAccessibleChild( m_aChildren[ i_nPosition ] );
        if ( !rAccessibleChild.is() )
        {
            ::rtl::Reference< AccessibleToolPanelDeckTabBarItem > pAccesibleItemContext( new AccessibleToolPanelDeckTabBarItem(
                getOwnAccessible(), *m_pPanelDeck, *m_pTabBar, i_nPosition ) );
            rAccessibleChild.set( new AccessibleWrapper( pAccesibleItemContext.get() ) );
            pAccesibleItemContext->lateInit( rAccessibleChild );
        }
        return rAccessibleChild;
    }

    Reference< XAccessible > AccessibleToolPanelTabBar_Impl::getOwnAccessible() const
    {
        Reference< XAccessible > xOwnAccessible( static_cast< XAccessible* >( m_rAntiImpl.GetVCLXWindow() ) );
        OSL_ENSURE( xOwnAccessible->getAccessibleContext() == Reference< XAccessibleContext >( &m_rAntiImpl ),
            "AccessibleToolPanelTabBar_Impl::getOwnAccessible: could not retrieve proper XAccessible for /myself!" );
        return xOwnAccessible;
    }

    void AccessibleToolPanelTabBar_Impl::PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        ENSURE_OR_RETURN_VOID( i_nPosition <= m_aChildren.size(), "AccessibleToolPanelTabBar_Impl::PanelInserted: illegal position (or invalid cache!)" );
        (void)i_pPanel;
        m_aChildren.insert( m_aChildren.begin() + i_nPosition, Reference< XAccessible >() );
        m_rAntiImpl.NotifyAccessibleEvent( AccessibleEventId::CHILD, Any(), makeAny( getAccessiblePanelItem( i_nPosition ) ) );
    }

    void AccessibleToolPanelTabBar_Impl::PanelRemoved( const size_t i_nPosition )
    {
        ENSURE_OR_RETURN_VOID( i_nPosition < m_aChildren.size(), "AccessibleToolPanelTabBar_Impl::PanelInserted: illegal position (or invalid cache!)" );

        const Reference< XAccessible > xOldChild( getAccessiblePanelItem( i_nPosition ) );
        m_aChildren.erase( m_aChildren.begin() + i_nPosition );
        m_rAntiImpl.NotifyAccessibleEvent( AccessibleEventId::CHILD, makeAny( xOldChild ), Any() );
    }

    void AccessibleToolPanelTabBar_Impl::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
    {
        (void)i_rOldActive;
        (void)i_rNewActive;
    }

    void AccessibleToolPanelTabBar_Impl::LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter )
    {
        (void)i_rNewLayouter;
        m_rAntiImpl.dispose();
    }

    void AccessibleToolPanelTabBar_Impl::Dying()
    {
        m_rAntiImpl.dispose();
    }

    IMPL_LINK_TYPED( AccessibleToolPanelTabBar_Impl, OnWindowEvent, VclWindowEvent&, rWindowEvent, void )
    {
        ENSURE_OR_RETURN_VOID( !isDisposed(), "AccessibleToolPanelTabBar_Impl::OnWindowEvent: already disposed!" );

        const bool bForwardButton = ( rWindowEvent.GetWindow() == &m_pTabBar->GetScrollButton( true ) );
        const bool bBackwardButton = ( rWindowEvent.GetWindow() == &m_pTabBar->GetScrollButton( false ) );
        ENSURE_OR_RETURN_VOID( bForwardButton || bBackwardButton, "AccessibleToolPanelTabBar_Impl::OnWindowEvent: where does this come from?" );

        const bool bShow = ( rWindowEvent.GetId() == VCLEVENT_WINDOW_SHOW );
        const bool bHide = ( rWindowEvent.GetId() == VCLEVENT_WINDOW_HIDE );
        if ( !bShow && !bHide )
            // not interested in events other than visibility changes
            return;

        const Reference< XAccessible > xButtonAccessible( m_pTabBar->GetScrollButton( bForwardButton ).GetAccessible() );
        const Any aOldChild( bHide ? xButtonAccessible : Reference< XAccessible >() );
        const Any aNewChild( bShow ? xButtonAccessible : Reference< XAccessible >() );
        m_rAntiImpl.NotifyAccessibleEvent( AccessibleEventId::CHILD, aOldChild, aNewChild );
    }

    // MethodGuard
    namespace
    {
        class MethodGuard
        {
        public:
            explicit MethodGuard( AccessibleToolPanelTabBar_Impl& i_rImpl )
                :m_aGuard()
            {
                i_rImpl.checkDisposed();
            }
            ~MethodGuard()
            {
            }

        private:
            SolarMutexGuard  m_aGuard;
        };
    }

    // AccessibleToolPanelTabBar
    AccessibleToolPanelTabBar::AccessibleToolPanelTabBar( const Reference< XAccessible >& i_rAccessibleParent,
            ::svt::IToolPanelDeck& i_rPanelDeck, ::svt::PanelTabBar& i_rTabBar )
        :VCLXAccessibleComponent( i_rTabBar.GetWindowPeer() )
        ,m_xImpl( new AccessibleToolPanelTabBar_Impl( *this, i_rAccessibleParent, i_rPanelDeck, i_rTabBar ) )
    {
    }

    AccessibleToolPanelTabBar::~AccessibleToolPanelTabBar()
    {
    }

    sal_Int32 SAL_CALL AccessibleToolPanelTabBar::getAccessibleChildCount(  ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *m_xImpl );

        const bool bHasScrollBack = m_xImpl->getTabBar()->GetScrollButton( false ).IsVisible();
        const bool bHasScrollForward = m_xImpl->getTabBar()->GetScrollButton( true ).IsVisible();

        return  m_xImpl->getPanelDeck()->GetPanelCount()
            +   ( bHasScrollBack ? 1 : 0 )
            +   ( bHasScrollForward ? 1 : 0 );
    }

    Reference< XAccessible > SAL_CALL AccessibleToolPanelTabBar::getAccessibleChild( sal_Int32 i_nIndex ) throw (IndexOutOfBoundsException, RuntimeException, std::exception)
    {
        MethodGuard aGuard( *m_xImpl );

        const bool bHasScrollBack = m_xImpl->getTabBar()->GetScrollButton( false ).IsVisible();
        const bool bHasScrollForward = m_xImpl->getTabBar()->GetScrollButton( true ).IsVisible();

        const bool bScrollBackRequested = ( bHasScrollBack && ( i_nIndex == 0 ) );
        const bool bScrollForwardRequested = ( bHasScrollForward && ( i_nIndex == getAccessibleChildCount() - 1 ) );
        OSL_ENSURE( !( bScrollBackRequested && bScrollForwardRequested ), "AccessibleToolPanelTabBar::getAccessibleChild: ouch!" );

        if ( bScrollBackRequested || bScrollForwardRequested )
        {
            Reference< XAccessible > xScrollButtonAccessible( m_xImpl->getTabBar()->GetScrollButton( bScrollForwardRequested ).GetAccessible() );
            ENSURE_OR_RETURN( xScrollButtonAccessible.is(), "AccessibleToolPanelTabBar::getAccessibleChild: invalid button accessible!", NULL );
        #if OSL_DEBUG_LEVEL > 0
            Reference< XAccessibleContext > xScrollButtonContext( xScrollButtonAccessible->getAccessibleContext() );
            ENSURE_OR_RETURN( xScrollButtonContext.is(), "AccessibleToolPanelTabBar::getAccessibleChild: invalid button accessible context!", xScrollButtonAccessible );
            OSL_ENSURE( xScrollButtonContext->getAccessibleParent() == m_xImpl->getOwnAccessible(),
                "AccessibleToolPanelTabBar::getAccessibleChild: wrong parent at the button's accessible!" );
        #endif
            return xScrollButtonAccessible;
        }

        return m_xImpl->getAccessiblePanelItem( i_nIndex - ( bHasScrollBack ? 1 : 0 ) );
    }

    Reference< XAccessible > SAL_CALL AccessibleToolPanelTabBar::getAccessibleParent(  ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *m_xImpl );
        return m_xImpl->getAccessibleParent();
    }

    sal_Int16 SAL_CALL AccessibleToolPanelTabBar::getAccessibleRole(  ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *m_xImpl );
        return AccessibleRole::PAGE_TAB_LIST;
    }

    namespace
    {
        bool lcl_covers( const vcl::Window& i_rWindow, const ::Point& i_rPoint )
        {
            const Rectangle aWindowBounds( i_rWindow.GetWindowExtentsRelative( i_rWindow.GetParent() ) );
            return aWindowBounds.IsInside( i_rPoint );
        }
    }

    Reference< XAccessible > SAL_CALL AccessibleToolPanelTabBar::getAccessibleAtPoint( const css::awt::Point& i_rPoint ) throw (RuntimeException, std::exception)
    {
        MethodGuard aGuard( *m_xImpl );

        // check the tab items
        const css::awt::Point aOwnScreenPos( getLocationOnScreen() );
        const ::Point aRequestedScreenPoint( i_rPoint.X + aOwnScreenPos.X, i_rPoint.Y + aOwnScreenPos.Y );

        for ( size_t i=0; i<m_xImpl->getPanelDeck()->GetPanelCount(); ++i )
        {
            const ::Rectangle aItemScreenRect( m_xImpl->getTabBar()->GetItemScreenRect(i) );
            if ( aItemScreenRect.IsInside( aRequestedScreenPoint ) )
                return m_xImpl->getAccessiblePanelItem(i);
        }

        // check the scroll buttons
        const ::Point aRequestedClientPoint( VCLUnoHelper::ConvertToVCLPoint( i_rPoint ) );

        const bool bHasScrollBack = m_xImpl->getTabBar()->GetScrollButton( false ).IsVisible();
        if ( bHasScrollBack && lcl_covers( m_xImpl->getTabBar()->GetScrollButton( false ), aRequestedClientPoint ) )
            return m_xImpl->getTabBar()->GetScrollButton( false ).GetAccessible();

        const bool bHasScrollForward = m_xImpl->getTabBar()->GetScrollButton( true ).IsVisible();
        if ( bHasScrollForward && lcl_covers( m_xImpl->getTabBar()->GetScrollButton( true ), aRequestedClientPoint ) )
            return m_xImpl->getTabBar()->GetScrollButton( true ).GetAccessible();

        // no hit
        return NULL;
    }

    void SAL_CALL AccessibleToolPanelTabBar::disposing()
    {
        VCLXAccessibleComponent::disposing();
        m_xImpl->dispose();
    }

    Reference< XAccessible > AccessibleToolPanelTabBar::GetChildAccessible( const VclWindowEvent& i_rVclWindowEvent )
    {
        // don't let the base class generate any A11Y events from VclWindowEvent, we completely manage those
        // A11Y events ourself
        (void)i_rVclWindowEvent;
        return NULL;
    }

    void AccessibleToolPanelTabBar::FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& i_rStateSet )
    {
        VCLXAccessibleComponent::FillAccessibleStateSet( i_rStateSet );
        i_rStateSet.AddState( AccessibleStateType::FOCUSABLE );

        ENSURE_OR_RETURN_VOID( !m_xImpl->isDisposed(), "AccessibleToolPanelTabBar::FillAccessibleStateSet: already disposed!" );
        if ( m_xImpl->getTabBar()->IsVertical() )
            i_rStateSet.AddState( AccessibleStateType::VERTICAL );
        else
            i_rStateSet.AddState( AccessibleStateType::HORIZONTAL );
    }
} // namespace accessibility

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
