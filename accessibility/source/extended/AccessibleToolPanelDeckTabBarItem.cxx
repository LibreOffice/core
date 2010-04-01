/*************************************************************************
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

#include "precompiled_accessibility.hxx"

#include "accessibility/extended/AccessibleToolPanelDeckTabBarItem.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
/** === end UNO includes === **/

#include <svtools/toolpanel/toolpaneldeck.hxx>
#include <svtools/toolpanel/tablayouter.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <unotools/accessiblerelationsethelper.hxx>
#include <tools/diagnose_ex.h>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>

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
            const ::rtl::Reference< ::svt::TabDeckLayouter >& i_pLayouter,
            const size_t i_nItemPos
        );
        ~AccessibleToolPanelDeckTabBarItem_Impl();

        // IToolPanelDeckListener
        virtual void PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition );
        virtual void PanelRemoved( const size_t i_nPosition );
        virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
        virtual void LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter );
        virtual void Dying();

    public:
        bool    isDisposed() const { return m_pLayouter == NULL; }
        void    checkDisposed() const;
        void    dispose();

        const Reference< XAccessible >&
                getAccessibleParent() const { return m_xAccessibleParent; }
        const ::rtl::Reference< ::svt::TabDeckLayouter >&
                getLayouter() const { return m_pLayouter; }
        size_t  getItemPos() const { return m_nItemPos; }

        Reference< XAccessibleComponent >
                getParentAccessibleComponent() const;
        ::rtl::OUString
                getPanelDisplayName();

    private:
        void impl_notifyBoundRectChanges();
        void impl_notifyStateChange( const sal_Int16 i_nLostState, const sal_Int16 i_nGainedState );

    private:
        AccessibleToolPanelDeckTabBarItem&          m_rAntiImpl;
        const Reference< XAccessible >              m_xAccessibleParent;
        ::rtl::Reference< ::svt::TabDeckLayouter >  m_pLayouter;
        size_t                                      m_nItemPos;
    };

    //==================================================================================================================
    //= AccessibleToolPanelDeckTabBarItem_Impl
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    AccessibleToolPanelDeckTabBarItem_Impl::AccessibleToolPanelDeckTabBarItem_Impl( AccessibleToolPanelDeckTabBarItem& i_rAntiImpl,
            const Reference< XAccessible >& i_rAccessibleParent, const ::rtl::Reference< ::svt::TabDeckLayouter >& i_pLayouter,
            const size_t i_nItemPos )
        :m_rAntiImpl( i_rAntiImpl )
        ,m_xAccessibleParent( i_rAccessibleParent )
        ,m_pLayouter( i_pLayouter )
        ,m_nItemPos( i_nItemPos )
    {
        ENSURE_OR_THROW( m_pLayouter.get() != NULL, "illegal layouter!" );

        ::svt::IToolPanelDeck& rPanels( m_pLayouter->GetPanelDeck() );
        rPanels.AddListener( *this );
    }

    //------------------------------------------------------------------------------------------------------------------
    AccessibleToolPanelDeckTabBarItem_Impl::~AccessibleToolPanelDeckTabBarItem_Impl()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeckTabBarItem_Impl::checkDisposed() const
    {
        if ( isDisposed() )
            throw DisposedException( ::rtl::OUString(), *&m_rAntiImpl );
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeckTabBarItem_Impl::dispose()
    {
        ENSURE_OR_RETURN_VOID( !isDisposed(), "AccessibleToolPanelDeckTabBarItem_Impl::dispose: disposed twice!" );

        ::svt::IToolPanelDeck& rPanels( m_pLayouter->GetPanelDeck() );
        rPanels.RemoveListener( *this );

        m_pLayouter = NULL;
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessibleComponent > AccessibleToolPanelDeckTabBarItem_Impl::getParentAccessibleComponent() const
    {
        Reference< XAccessible > xAccessibleParent( m_rAntiImpl.getAccessibleParent(), UNO_QUERY_THROW );
        return Reference< XAccessibleComponent >( xAccessibleParent->getAccessibleContext(), UNO_QUERY );
    }

    //------------------------------------------------------------------------------------------------------------------
    ::rtl::OUString AccessibleToolPanelDeckTabBarItem_Impl::getPanelDisplayName()
    {
        const ::rtl::Reference< ::svt::TabDeckLayouter > pLayouter( getLayouter() );
        const ::svt::IToolPanelDeck& rPanels( pLayouter->GetPanelDeck() );
        const ::svt::PToolPanel pPanel( rPanels.GetPanel( getItemPos() ) );
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
            :m_aGuard( Application::GetSolarMutex() )
        {
            i_rImpl.checkDisposed();
        }
        ~ItemMethodGuard()
        {
        }

        void clear()
        {
            m_aGuard.clear();
        }

    private:
        ::vos::OClearableGuard  m_aGuard;
    };

    //==================================================================================================================
    //= AccessibleToolPanelDeckTabBarItem
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    AccessibleToolPanelDeckTabBarItem::AccessibleToolPanelDeckTabBarItem( const Reference< XAccessible >& i_rAccessibleParent,
            const ::rtl::Reference< ::svt::TabDeckLayouter >& i_pLayouter, const size_t i_nItemPos )
        :m_pImpl( new AccessibleToolPanelDeckTabBarItem_Impl( *this, i_rAccessibleParent, i_pLayouter, i_nItemPos ) )
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
        throw IndexOutOfBoundsException( ::rtl::OUString(), *this );
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
        return AccessibleRole::LIST_ITEM;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AccessibleToolPanelDeckTabBarItem::getAccessibleDescription(  ) throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getPanelDisplayName();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AccessibleToolPanelDeckTabBarItem::getAccessibleName(  ) throw (RuntimeException)
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

        const ::rtl::Reference< ::svt::TabDeckLayouter > pLayouter( m_pImpl->getLayouter() );
        const ::svt::IToolPanelDeck& rPanels( pLayouter->GetPanelDeck() );

        ::utl::AccessibleStateSetHelper* pStateSet( new ::utl::AccessibleStateSetHelper );
        pStateSet->AddState( AccessibleStateType::FOCUSABLE );
        pStateSet->AddState( AccessibleStateType::SELECTABLE );
        pStateSet->AddState( AccessibleStateType::ICONIFIED );

        if ( m_pImpl->getItemPos() == rPanels.GetActivePanel() )
        {
            pStateSet->AddState( AccessibleStateType::ACTIVE );
            pStateSet->AddState( AccessibleStateType::SELECTED );
        }

        if ( m_pImpl->getItemPos() == pLayouter->GetFocusedPanelItem() )
            pStateSet->AddState( AccessibleStateType::FOCUSED );

        if ( pLayouter->IsPanelSelectorEnabled() )
            pStateSet->AddState( AccessibleStateType::ENABLED );

        if ( pLayouter->IsPanelSelectorVisible() )
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
        return NULL;
    }

    //--------------------------------------------------------------------
    void SAL_CALL AccessibleToolPanelDeckTabBarItem::grabFocus(  ) throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );
        m_pImpl->getLayouter()->FocusPanelItem( m_pImpl->getItemPos() );
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
    ::rtl::OUString SAL_CALL AccessibleToolPanelDeckTabBarItem::getTitledBorderText(  ) throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );
        // no support
        return ::rtl::OUString();
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL AccessibleToolPanelDeckTabBarItem::getToolTipText(  ) throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );
        return m_pImpl->getPanelDisplayName();
    }

    //--------------------------------------------------------------------
    UnoRectangle SAL_CALL AccessibleToolPanelDeckTabBarItem::implGetBounds() throw (RuntimeException)
    {
        ItemMethodGuard aGuard( *m_pImpl );
        ::rtl::Reference< ::svt::TabDeckLayouter > pLayouter( m_pImpl->getLayouter() );

        const ::Rectangle aItemScreenRect( pLayouter->GetItemScreenRect( m_pImpl->getItemPos() ) );

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
