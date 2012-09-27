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


#include "accessibility/extended/AccessibleToolPanelDeckTabBarItem.hxx"

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <svtools/toolpanel/toolpaneldeck.hxx>
#include <svtools/toolpanel/paneltabbar.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>

//......................................................................................................................
namespace accessibility
{
//......................................................................................................................

    typedef ::com::sun::star::awt::Rectangle    UnoRectangle;
    typedef ::com::sun::star::awt::Point        UnoPoint;

    /** === begin UNO using === **/
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
    using ::com::sun::star::accessibility::XAccessibleRelationSet;
    using ::com::sun::star::accessibility::XAccessibleStateSet;
    using ::com::sun::star::accessibility::XAccessibleComponent;
    using ::com::sun::star::accessibility::XAccessibleExtendedComponent;
    using ::com::sun::star::awt::XFont;
    /** === end UNO using === **/

    namespace AccessibleRole = ::com::sun::star::accessibility::AccessibleRole;
    namespace AccessibleStateType = ::com::sun::star::accessibility::AccessibleStateType;
    namespace AccessibleEventId = ::com::sun::star::accessibility::AccessibleEventId;

    //==================================================================================================================
    //= AccessibleToolPanelDeckTabBarItem_Impl
    //==================================================================================================================
    class AccessibleToolPanelDeckTabBarItem_Impl : public ::svt::IToolPanelDeckListener
    {
    public:
        AccessibleToolPanelDeckTabBarItem_Impl(
            AccessibleToolPanelDeckTabBarItem& i_rAntiImpl,
            const Reference< XAccessible >& i_rAccessibleParent,
            ::svt::IToolPanelDeck& i_rPanelDeck,
            ::svt::PanelTabBar& i_rTabBar,
            const size_t i_nItemPos
        );
        virtual ~AccessibleToolPanelDeckTabBarItem_Impl();

        ::svt::PanelTabBar* getTabBar() const { return m_pTabBar; }

        // IToolPanelDeckListener
        virtual void PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition );
        virtual void PanelRemoved( const size_t i_nPosition );
        virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
        virtual void LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter );
        virtual void Dying();

    public:
        bool    isDisposed() const { return m_pPanelDeck == NULL; }
        void    checkDisposed() const;
        void    dispose();

        const Reference< XAccessible >&
                getAccessibleParent() const { return m_xAccessibleParent; }
        size_t  getItemPos() const { return m_nItemPos; }

        Reference< XAccessibleComponent >   getParentAccessibleComponent() const;
        ::svt::IToolPanelDeck*              getPanelDeck() const { return m_pPanelDeck; }
        OUString                            getPanelDisplayName();

    private:
        void impl_notifyBoundRectChanges();
        void impl_notifyStateChange( const sal_Int16 i_nLostState, const sal_Int16 i_nGainedState );

    private:
        AccessibleToolPanelDeckTabBarItem&  m_rAntiImpl;
        Reference< XAccessible >            m_xAccessibleParent;
        ::svt::IToolPanelDeck*              m_pPanelDeck;
        ::svt::PanelTabBar*                 m_pTabBar;
        size_t                              m_nItemPos;
    };

    //==================================================================================================================
    //= AccessibleToolPanelDeckTabBarItem_Impl
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    AccessibleToolPanelDeckTabBarItem_Impl::AccessibleToolPanelDeckTabBarItem_Impl( AccessibleToolPanelDeckTabBarItem& i_rAntiImpl,
            const Reference< XAccessible >& i_rAccessibleParent, ::svt::IToolPanelDeck& i_rPanelDeck, ::svt::PanelTabBar& i_rTabBar,
            const size_t i_nItemPos )
        :m_rAntiImpl( i_rAntiImpl )
        ,m_xAccessibleParent( i_rAccessibleParent )
        ,m_pPanelDeck( &i_rPanelDeck )
        ,m_pTabBar( &i_rTabBar )
        ,m_nItemPos( i_nItemPos )
    {
        m_pPanelDeck->AddListener( *this );
    }

    //------------------------------------------------------------------------------------------------------------------
    AccessibleToolPanelDeckTabBarItem_Impl::~AccessibleToolPanelDeckTabBarItem_Impl()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeckTabBarItem_Impl::checkDisposed() const
    {
        if ( isDisposed() )
            throw DisposedException( OUString(), *&m_rAntiImpl );
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeckTabBarItem_Impl::dispose()
    {
        ENSURE_OR_RETURN_VOID( !isDisposed(), "AccessibleToolPanelDeckTabBarItem_Impl::dispose: disposed twice!" );

        m_xAccessibleParent.clear();
        m_pPanelDeck->RemoveListener( *this );
        m_pPanelDeck = NULL;
        m_pTabBar = NULL;
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessibleComponent > AccessibleToolPanelDeckTabBarItem_Impl::getParentAccessibleComponent() const
    {
        Reference< XAccessible > xAccessibleParent( m_rAntiImpl.getAccessibleParent(), UNO_QUERY_THROW );
        return Reference< XAccessibleComponent >( xAccessibleParent->getAccessibleContext(), UNO_QUERY );
    }

    //------------------------------------------------------------------------------------------------------------------
    OUString AccessibleToolPanelDeckTabBarItem_Impl::getPanelDisplayName()
    {
        const ::svt::PToolPanel pPanel( m_pPanelDeck->GetPanel( getItemPos() ) );
        if ( pPanel.get() == NULL )
            throw DisposedException();
        return pPanel->GetDisplayName();
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeckTabBarItem_Impl::impl_notifyBoundRectChanges()
    {
        m_rAntiImpl.NotifyAccessibleEvent( AccessibleEventId::BOUNDRECT_CHANGED, Any(), Any() );
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeckTabBarItem_Impl::impl_notifyStateChange( const sal_Int16 i_nLostState, const sal_Int16 i_nGainedState )
    {
        m_rAntiImpl.NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED,
            i_nLostState > -1 ? makeAny( i_nLostState ) : Any(),
            i_nGainedState > -1 ? makeAny( i_nGainedState ) : Any()
        );
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeckTabBarItem_Impl::PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        (void)i_pPanel;
        if ( i_nPosition <= m_nItemPos )
            ++m_nItemPos;
        impl_notifyBoundRectChanges();
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeckTabBarItem_Impl::PanelRemoved( const size_t i_nPosition )
    {
        if ( i_nPosition == m_nItemPos )
        {
            m_rAntiImpl.dispose();
        }
        else if ( i_nPosition < m_nItemPos )
        {
            --m_nItemPos;
            impl_notifyBoundRectChanges();
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeckTabBarItem_Impl::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
    {
        if ( m_nItemPos == i_rOldActive )
        {
            impl_notifyStateChange( AccessibleStateType::ACTIVE, -1 );
            impl_notifyStateChange( AccessibleStateType::SELECTED, -1 );
        }
        else if ( m_nItemPos == i_rNewActive )
        {
            impl_notifyStateChange( -1, AccessibleStateType::ACTIVE );
            impl_notifyStateChange( -1, AccessibleStateType::SELECTED );
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeckTabBarItem_Impl::LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter )
    {
        (void)i_rNewLayouter;
        // if the tool panel deck has a new layouter, then the old layouter, and thus all items it was
        // responsible for, died. So do we.
        dispose();
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeckTabBarItem_Impl::Dying()
    {
        // if the tool panel deck is dying, then its layouter dies, so should we.
        dispose();
    }

    //==================================================================================================================
    //= ItemMethodGuard
    //==================================================================================================================
    class ItemMethodGuard
    {
    public:
        ItemMethodGuard( AccessibleToolPanelDeckTabBarItem_Impl& i_rImpl )
            :m_aGuard()
        {
            i_rImpl.checkDisposed();
        }
        ~ItemMethodGuard()
        {
        }

    private:
        SolarMutexGuard  m_aGuard;
    };

    //==================================================================================================================
    //= AccessibleToolPanelDeckTabBarItem
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    AccessibleToolPanelDeckTabBarItem::AccessibleToolPanelDeckTabBarItem( const Reference< XAccessible >& i_rAccessibleParent,
            ::svt::IToolPanelDeck& i_rPanelDeck, ::svt::PanelTabBar& i_rTabBar, const size_t i_nItemPos )
        :m_pImpl( new AccessibleToolPanelDeckTabBarItem_Impl( *this, i_rAccessibleParent, i_rPanelDeck, i_rTabBar, i_nItemPos ) )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    AccessibleToolPanelDeckTabBarItem::~AccessibleToolPanelDeckTabBarItem()
    {
    }

    //--------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleToolPanelDeckTabBarItem::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        return 0;
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleToolPanelDeckTabBarItem::getAccessibleChild( sal_Int32 i ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        (void)i;
        throw IndexOutOfBoundsException( OUString(), *this );
    }

    //--------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleToolPanelDeckTabBarItem::getAccessibleParent(  ) throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getAccessibleParent();
    }

    //--------------------------------------------------------------------
    sal_Int16 SAL_CALL AccessibleToolPanelDeckTabBarItem::getAccessibleRole(  ) throw (RuntimeException)
    {
        return AccessibleRole::PAGE_TAB;
    }

    //--------------------------------------------------------------------
    OUString SAL_CALL AccessibleToolPanelDeckTabBarItem::getAccessibleDescription(  ) throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getPanelDisplayName();
    }

    //--------------------------------------------------------------------
    OUString SAL_CALL AccessibleToolPanelDeckTabBarItem::getAccessibleName(  ) throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getPanelDisplayName();
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleRelationSet > SAL_CALL AccessibleToolPanelDeckTabBarItem::getAccessibleRelationSet(  ) throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );
        ::utl::AccessibleRelationSetHelper* pRelationSet = new utl::AccessibleRelationSetHelper;
        return pRelationSet;
    }

    //--------------------------------------------------------------------
    Reference< XAccessibleStateSet > SAL_CALL AccessibleToolPanelDeckTabBarItem::getAccessibleStateSet(  ) throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );

        ::utl::AccessibleStateSetHelper* pStateSet( new ::utl::AccessibleStateSetHelper );
        pStateSet->AddState( AccessibleStateType::FOCUSABLE );
        pStateSet->AddState( AccessibleStateType::SELECTABLE );
        pStateSet->AddState( AccessibleStateType::ICONIFIED );

        if ( m_pImpl->getItemPos() == m_pImpl->getPanelDeck()->GetActivePanel() )
        {
            pStateSet->AddState( AccessibleStateType::ACTIVE );
            pStateSet->AddState( AccessibleStateType::SELECTED );
        }

        if ( m_pImpl->getItemPos() == m_pImpl->getTabBar()->GetFocusedPanelItem() )
            pStateSet->AddState( AccessibleStateType::FOCUSED );

        if ( m_pImpl->getTabBar()->IsEnabled() )
            pStateSet->AddState( AccessibleStateType::ENABLED );

        if ( m_pImpl->getTabBar()->IsVisible() )
        {
            pStateSet->AddState( AccessibleStateType::SHOWING );
            pStateSet->AddState( AccessibleStateType::VISIBLE );
        }

        return pStateSet;
    }


    //--------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleToolPanelDeckTabBarItem::getAccessibleAtPoint( const UnoPoint& i_rLocation ) throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );
        // we do not have children ...
        (void)i_rLocation;
        return NULL;
    }

    //--------------------------------------------------------------------
    void SAL_CALL AccessibleToolPanelDeckTabBarItem::grabFocus(  ) throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );
        m_pImpl->getTabBar()->FocusPanelItem( m_pImpl->getItemPos() );
    }

    //--------------------------------------------------------------------
    ::sal_Int32 SAL_CALL AccessibleToolPanelDeckTabBarItem::getForeground(  ) throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );
        Reference< XAccessibleComponent > xParentComponent( m_pImpl->getParentAccessibleComponent(), UNO_SET_THROW );
        return xParentComponent->getForeground();
    }

    //--------------------------------------------------------------------
    ::sal_Int32 SAL_CALL AccessibleToolPanelDeckTabBarItem::getBackground(  ) throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );
        Reference< XAccessibleComponent > xParentComponent( m_pImpl->getParentAccessibleComponent(), UNO_SET_THROW );
        return xParentComponent->getBackground();
    }

    //--------------------------------------------------------------------
    Reference< XFont > SAL_CALL AccessibleToolPanelDeckTabBarItem::getFont(  ) throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );
        Reference< XAccessibleExtendedComponent > xParentComponent( m_pImpl->getParentAccessibleComponent(), UNO_QUERY_THROW );
        // TODO: strictly, this is not correct: The TabBar implementation of the TabLayouter might use
        // a different font ...
        return xParentComponent->getFont();
    }

    //--------------------------------------------------------------------
    OUString SAL_CALL AccessibleToolPanelDeckTabBarItem::getTitledBorderText(  ) throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );
        // no support
        return OUString();
    }

    //--------------------------------------------------------------------
    OUString SAL_CALL AccessibleToolPanelDeckTabBarItem::getToolTipText(  ) throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getPanelDisplayName();
    }

    //--------------------------------------------------------------------
    UnoRectangle SAL_CALL AccessibleToolPanelDeckTabBarItem::implGetBounds() throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );

        const ::Rectangle aItemScreenRect( m_pImpl->getTabBar()->GetItemScreenRect( m_pImpl->getItemPos() ) );

        Reference< XAccessibleComponent > xParentComponent( m_pImpl->getParentAccessibleComponent(), UNO_SET_THROW );
        const UnoPoint aParentLocation( xParentComponent->getLocationOnScreen() );
        return UnoRectangle(
            aItemScreenRect.Left() - aParentLocation.X,
            aItemScreenRect.Top() - aParentLocation.Y,
            aItemScreenRect.GetWidth(),
            aItemScreenRect.GetHeight()
        );
    }

    //--------------------------------------------------------------------
    void SAL_CALL AccessibleToolPanelDeckTabBarItem::disposing()
    {
        ItemMethodGuard aGuard( *m_pImpl );
        m_pImpl->dispose();
    }

//......................................................................................................................
} // namespace accessibility
//......................................................................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
