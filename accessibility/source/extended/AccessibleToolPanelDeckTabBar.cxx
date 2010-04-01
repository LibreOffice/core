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

#include "accessibility/extended/AccessibleToolPanelDeckTabBar.hxx"
#include "accessibility/extended/AccessibleToolPanelDeckTabBarItem.hxx"
#include "accessibility/helper/accresmgr.hxx"
#include "accessibility/helper/accessiblestrings.hrc"

/** === begin UNO includes === **/
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
/** === end UNO includes === **/

#include <svtools/toolpanel/toolpaneldeck.hxx>
#include <svtools/toolpanel/paneltabbar.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>
#include <tools/diagnose_ex.h>

#include <vector>

//......................................................................................................................
namespace accessibility
{
//......................................................................................................................

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
    using ::com::sun::star::accessibility::XAccessibleContext;
    /** === end UNO using === **/

    namespace AccessibleRole = ::com::sun::star::accessibility::AccessibleRole;
    namespace AccessibleEventId = ::com::sun::star::accessibility::AccessibleEventId;
    namespace AccessibleStateType = ::com::sun::star::accessibility::AccessibleStateType;

    typedef ::com::sun::star::awt::Point        UnoPoint;
    typedef ::com::sun::star::awt::Size         UnoSize;
    typedef ::com::sun::star::awt::Rectangle    UnoRectangle;

    //==================================================================================================================
    //= AccessibleWrapper
    //==================================================================================================================
    typedef ::cppu::WeakImplHelper1< XAccessible > AccessibleWrapper_Base;
    class AccessibleWrapper : public AccessibleWrapper_Base
    {
    public:
        AccessibleWrapper( const Reference< XAccessibleContext >& i_rContext )
            :m_xContext( i_rContext )
        {
        }

        // XAccessible
        virtual Reference< XAccessibleContext > SAL_CALL getAccessibleContext(  ) throw (RuntimeException)
        {
            return m_xContext;
        }

    private:
        const Reference< XAccessibleContext >   m_xContext;
    };

    //==================================================================================================================
    //= AccessibleToolPanelTabBar_Impl
    //==================================================================================================================
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
        ~AccessibleToolPanelTabBar_Impl();

        void    checkDisposed();
        bool    isDisposed() const { return m_pPanelDeck == NULL; }
        void    dispose();

        ::svt::IToolPanelDeck*          getPanelDeck() const { return m_pPanelDeck; }
        ::svt::PanelTabBar*             getTabBar() const { return m_pTabBar; }
        const Reference< XAccessible >& getAccessibleParent() const { return m_xAccessibleParent; }
        Reference< XAccessible >        getAccessibleChild( size_t i_nPosition );
        Reference< XAccessible >        getOwnAccessible() const;

        // IToolPanelDeckListener
        virtual void PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition );
        virtual void PanelRemoved( const size_t i_nPosition );
        virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
        virtual void LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter );
        virtual void Dying();

    private:
        AccessibleToolPanelTabBar&                  m_rAntiImpl;
        Reference< XAccessible >                    m_xAccessibleParent;
        ::svt::IToolPanelDeck*                      m_pPanelDeck;
        ::svt::PanelTabBar*                         m_pTabBar;
        ::std::vector< Reference< XAccessible > >   m_aChildren;
    };

    //------------------------------------------------------------------------------------------------------------------
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

        const String sAccessibleDescription( TK_RES_STRING( RID_STR_ACC_DESC_PANELDECL_TABBAR ) );
        i_rTabBar.SetAccessibleName( sAccessibleDescription );
        i_rTabBar.SetAccessibleDescription( sAccessibleDescription );
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelTabBar_Impl::checkDisposed()
    {
        if ( isDisposed() )
            throw DisposedException( ::rtl::OUString(), *&m_rAntiImpl );
    }

    //------------------------------------------------------------------------------------------------------------------
    AccessibleToolPanelTabBar_Impl::~AccessibleToolPanelTabBar_Impl()
    {
        if ( !isDisposed() )
            dispose();
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelTabBar_Impl::dispose()
    {
        ENSURE_OR_RETURN_VOID( !isDisposed(), "disposed twice" );
        m_pPanelDeck->RemoveListener( *this );
        m_pPanelDeck = NULL;
        m_xAccessibleParent.clear();
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > AccessibleToolPanelTabBar_Impl::getAccessibleChild( size_t i_nPosition )
    {
        ENSURE_OR_RETURN( !isDisposed(), "AccessibleToolPanelTabBar_Impl::getAccessibleChild: already disposed!", NULL );
        ENSURE_OR_RETURN( i_nPosition < m_aChildren.size(), "AccessibleToolPanelTabBar_Impl::getAccessibleChild: invalid index!", NULL );

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

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > AccessibleToolPanelTabBar_Impl::getOwnAccessible() const
    {
        Reference< XAccessible > xOwnAccessible( static_cast< XAccessible* >( m_rAntiImpl.GetVCLXWindow() ) );
        OSL_ENSURE( xOwnAccessible->getAccessibleContext() == Reference< XAccessibleContext >( &m_rAntiImpl ),
            "AccessibleToolPanelTabBar_Impl::getOwnAccessible: could not retrieve proper XAccessible for /myself!" );
        return xOwnAccessible;
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelTabBar_Impl::PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        ENSURE_OR_RETURN_VOID( i_nPosition <= m_aChildren.size(), "AccessibleToolPanelTabBar_Impl::PanelInserted: illegal position (or invalid cache!)" );
        m_aChildren.insert( m_aChildren.begin() + i_nPosition, NULL );
        // TODO: notify the event
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelTabBar_Impl::PanelRemoved( const size_t i_nPosition )
    {
        ENSURE_OR_RETURN_VOID( i_nPosition < m_aChildren.size(), "AccessibleToolPanelTabBar_Impl::PanelInserted: illegal position (or invalid cache!)" );
        m_aChildren.erase( m_aChildren.begin() + i_nPosition );
        // TODO: notify the event
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelTabBar_Impl::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
    {
        // TODO: state changes for the active descendant?
        // TODO: state changes for the items themself (assuming that they don't do this on their own)
        (void)i_rOldActive;
        (void)i_rNewActive;
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelTabBar_Impl::LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter )
    {
        m_rAntiImpl.dispose();
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelTabBar_Impl::Dying()
    {
        m_rAntiImpl.dispose();
    }

    //==================================================================================================================
    //= MethodGuard
    //==================================================================================================================
    namespace
    {
        class MethodGuard
        {
        public:
            MethodGuard( AccessibleToolPanelTabBar_Impl& i_rImpl )
                :m_aGuard( Application::GetSolarMutex() )
            {
                i_rImpl.checkDisposed();
            }
            ~MethodGuard()
            {
            }

            void clear()
            {
                m_aGuard.clear();
            }

        private:
            ::vos::OClearableGuard  m_aGuard;
        };
    }

    //==================================================================================================================
    //= AccessibleToolPanelTabBar
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    AccessibleToolPanelTabBar::AccessibleToolPanelTabBar( const Reference< XAccessible >& i_rAccessibleParent,
            ::svt::IToolPanelDeck& i_rPanelDeck, ::svt::PanelTabBar& i_rTabBar )
        :AccessibleToolPanelTabBar_Base( i_rTabBar.GetWindowPeer() )
        ,m_pImpl( new AccessibleToolPanelTabBar_Impl( *this, i_rAccessibleParent, i_rPanelDeck, i_rTabBar ) )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    AccessibleToolPanelTabBar::~AccessibleToolPanelTabBar()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleToolPanelTabBar::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        MethodGuard aGuard( *m_pImpl );
        return m_pImpl->getPanelDeck()->GetPanelCount();
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleToolPanelTabBar::getAccessibleChild( sal_Int32 i_nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *m_pImpl );
        return m_pImpl->getAccessibleChild( i_nIndex );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleToolPanelTabBar::getAccessibleParent(  ) throw (RuntimeException)
    {
        MethodGuard aGuard( *m_pImpl );
        return m_pImpl->getAccessibleParent();
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Int16 SAL_CALL AccessibleToolPanelTabBar::getAccessibleRole(  ) throw (RuntimeException)
    {
        MethodGuard aGuard( *m_pImpl );
        return AccessibleRole::PAGE_TAB_LIST;
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleToolPanelTabBar::getAccessibleAtPoint( const UnoPoint& i_rPoint ) throw (RuntimeException)
    {
        MethodGuard aGuard( *m_pImpl );

        const UnoPoint aOwnScreenPos( getLocationOnScreen() );
        const ::Point aRequestedPoint( i_rPoint.X + aOwnScreenPos.X, i_rPoint.Y + aOwnScreenPos.Y );


        for ( size_t i=0; i<m_pImpl->getPanelDeck()->GetPanelCount(); ++i )
        {
            const ::Rectangle aItemScreenRect( m_pImpl->getTabBar()->GetItemScreenRect(i) );
            if ( aItemScreenRect.IsInside( aRequestedPoint ) )
                return m_pImpl->getAccessibleChild(i);
        }

        // TODO: the buttons
        return NULL;
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AccessibleToolPanelTabBar::disposing()
    {
        AccessibleToolPanelTabBar_Base::disposing();
        m_pImpl->dispose();
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > AccessibleToolPanelTabBar::GetChildAccessible( const VclWindowEvent& i_rVclWindowEvent )
    {
        // don't let the base class generate any A11Y events from VclWindowEvent, we completely manage those
        // A11Y events ourself
        return NULL;
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelTabBar::FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& i_rStateSet )
    {
        AccessibleToolPanelTabBar_Base::FillAccessibleStateSet( i_rStateSet );
        i_rStateSet.AddState( AccessibleStateType::FOCUSABLE );

        ENSURE_OR_RETURN_VOID( !m_pImpl->isDisposed(), "AccessibleToolPanelTabBar::FillAccessibleStateSet: already disposed!" );
        if ( m_pImpl->getTabBar()->IsVertical() )
            i_rStateSet.AddState( AccessibleStateType::VERTICAL );
        else
            i_rStateSet.AddState( AccessibleStateType::HORIZONTAL );
    }

//......................................................................................................................
} // namespace accessibility
//......................................................................................................................
