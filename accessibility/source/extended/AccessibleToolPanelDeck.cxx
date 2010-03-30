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

#include "accessibility/extended/AccessibleToolPanelDeck.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
/** === end UNO includes === **/

#include <svtools/toolpanel/toolpaneldeck.hxx>
#include <comphelper/sharedmutex.hxx>
#include <toolkit/helper/externallock.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <vcl/svapp.hxx>
#include <vos/mutex.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <tools/diagnose_ex.h>

#include <boost/noncopyable.hpp>
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
    using ::com::sun::star::accessibility::XAccessibleContext;
    using ::com::sun::star::lang::DisposedException;
    using ::com::sun::star::lang::IndexOutOfBoundsException;
    using ::com::sun::star::lang::Locale;
    using ::com::sun::star::accessibility::XAccessibleRelationSet;
    using ::com::sun::star::accessibility::XAccessibleStateSet;
    using ::com::sun::star::accessibility::IllegalAccessibleComponentStateException;
    using ::com::sun::star::awt::XFont;
    /** === end UNO using === **/
    namespace AccessibleRole = ::com::sun::star::accessibility::AccessibleRole;
    namespace AccessibleEventId = ::com::sun::star::accessibility::AccessibleEventId;
    namespace AccessibleStateType = ::com::sun::star::accessibility::AccessibleStateType;

    typedef ::com::sun::star::awt::Rectangle    UnoRectangle;
    typedef ::com::sun::star::awt::Point        UnoPoint;

    //==================================================================================================================
    //= AccessibleToolPanelDeck_Impl - declaration
    //==================================================================================================================
    class AccessibleToolPanelDeck_Impl  :public ::boost::noncopyable
                                        ,public ::svt::IToolPanelDeckListener
    {
    public:
        AccessibleToolPanelDeck_Impl(
            AccessibleToolPanelDeck& i_rAntiImpl,
            const Reference< XAccessible >& i_rAccessibleParent,
            ::svt::ToolPanelDeck& i_rPanelDeck
        );

        void    checkDisposed();
        bool    isDisposed() const { return m_pPanelDeck == NULL; }
        void    dispose();

        ~AccessibleToolPanelDeck_Impl();

        Reference< XAccessible >    getOwnAccessible() const;
        Reference< XAccessible >    getPanelItemAccessible( const size_t i_nPosition, const bool i_bCreate );

    protected:
        // IToolPanelDeckListener
        virtual void PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition );
        virtual void PanelRemoved( const size_t i_nPosition );
        virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
        virtual void LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter );
        virtual void Dying();

    public:
        AccessibleToolPanelDeck&        m_rAntiImpl;
        const Reference< XAccessible >  m_xAccessibleParent;
        ::svt::ToolPanelDeck*           m_pPanelDeck;

        typedef ::std::vector< Reference< XAccessible > > AccessibleChildren;
        AccessibleChildren              m_aPanelItemCache;
    };

    //==================================================================================================================
    //= MethodGuard
    //==================================================================================================================
    class MethodGuard
    {
    public:
        MethodGuard( AccessibleToolPanelDeck_Impl& i_rImpl )
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

    //==================================================================================================================
    //= AccessibleToolPanelDeck_Impl - implementation
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    AccessibleToolPanelDeck_Impl::AccessibleToolPanelDeck_Impl( AccessibleToolPanelDeck& i_rAntiImpl, const Reference< XAccessible >& i_rAccessibleParent,
            ::svt::ToolPanelDeck& i_rPanelDeck )
        :m_rAntiImpl( i_rAntiImpl )
        ,m_xAccessibleParent( i_rAccessibleParent )
        ,m_pPanelDeck( &i_rPanelDeck )
    {
        m_pPanelDeck->AddListener( *this );
        m_aPanelItemCache.resize( m_pPanelDeck->GetPanelCount() );
    }

    //------------------------------------------------------------------------------------------------------------------
    AccessibleToolPanelDeck_Impl::~AccessibleToolPanelDeck_Impl()
    {
        if ( !isDisposed() )
            dispose();
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeck_Impl::dispose()
    {
        ENSURE_OR_RETURN_VOID( !isDisposed(), "disposed twice" );
        m_pPanelDeck->RemoveListener( *this );
        m_pPanelDeck = NULL;
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeck_Impl::checkDisposed()
    {
        if ( isDisposed() )
            throw DisposedException( ::rtl::OUString(), *&m_rAntiImpl );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > AccessibleToolPanelDeck_Impl::getOwnAccessible() const
    {
        Reference< XAccessible > xOwnAccessible( static_cast< XAccessible* >( m_rAntiImpl.GetVCLXWindow() ) );
        OSL_ENSURE( xOwnAccessible->getAccessibleContext() == Reference< XAccessibleContext >( &m_rAntiImpl ),
            "AccessibleToolPanelDeck_Impl::getOwnAccessible: could not retrieve proper XAccessible for /myself!" );
        return xOwnAccessible;
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > AccessibleToolPanelDeck_Impl::getPanelItemAccessible( const size_t i_nPosition, const bool i_bCreate )
    {
        ENSURE_OR_RETURN( !isDisposed(), "AccessibleToolPanelDeck_Impl::getPanelItemAccessible: already disposed!", NULL );
        ENSURE_OR_RETURN( i_nPosition < m_pPanelDeck->GetPanelCount(), "AccessibleToolPanelDeck_Impl::getPanelItemAccessible: invalid position!", NULL );
        ENSURE_OR_RETURN( i_nPosition < m_aPanelItemCache.size(), "AccessibleToolPanelDeck_Impl::getPanelItemAccessible: invalid cache!", NULL );

        Reference< XAccessible >& rxChildAccessible( m_aPanelItemCache[ i_nPosition ] );
        if ( !rxChildAccessible.is() && i_bCreate )
        {
            ::svt::PDeckLayouter pLayouter( m_pPanelDeck->GetLayouter() );
            ENSURE_OR_THROW( pLayouter.get() != NULL, "unexpected NULL layouter for the panel deck" );

            rxChildAccessible.set(
                pLayouter->GetPanelItemAccessible( i_nPosition, getOwnAccessible() )
            );
            OSL_ENSURE( rxChildAccessible.is(), "AccessibleToolPanelDeck_Impl::getPanelItemAccessible: illegal accessible returned by the deck layouter!" );
        }
        return rxChildAccessible;
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeck_Impl::PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        (void)i_pPanel;

        MethodGuard aGuard( *this );

        // reserve some space in our cache
        m_aPanelItemCache.insert( m_aPanelItemCache.begin() + i_nPosition, NULL );
        const Reference< XAccessible > xNewPanelItemChild( getPanelItemAccessible( i_nPosition, true ) );

        aGuard.clear();
        m_rAntiImpl.NotifyAccessibleEvent( AccessibleEventId::CHILD, Any(), makeAny( xNewPanelItemChild ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeck_Impl::PanelRemoved( const size_t i_nPosition )
    {
        MethodGuard aGuard( *this );

        ENSURE_OR_RETURN_VOID( i_nPosition < m_aPanelItemCache.size(), "AccessibleToolPanelDeck_Impl::PanelRemoved: invalid cache!" );

        // *first* obtain the cached XAccessible of the child
        const Reference< XAccessible > xOldPanelItemChild( getPanelItemAccessible( i_nPosition, false ) );
        // *then* update the cache
        m_aPanelItemCache.erase( m_aPanelItemCache.begin() + i_nPosition );

        // finally notify
        aGuard.clear();

        OSL_ENSURE( xOldPanelItemChild.is(), "AccessibleToolPanelDeck_Impl::PanelRemoved: hmmm ..." );
        if ( !xOldPanelItemChild.is() )
        {
            // this might, in theory, happen when the respective child has never been accessed before. Since we do
            // not have *any* chance of retrieving the child, we broadcast a ... more generic event
            m_rAntiImpl.NotifyAccessibleEvent( AccessibleEventId::INVALIDATE_ALL_CHILDREN, Any(), Any() );
        }
        else
        {
            m_rAntiImpl.NotifyAccessibleEvent( AccessibleEventId::CHILD, makeAny( xOldPanelItemChild ), Any() );
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeck_Impl::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
    {
        // TODO
        (void)i_rOldActive;
        (void)i_rNewActive;
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeck_Impl::LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter )
    {
        MethodGuard aGuard( *this );

        m_rAntiImpl.NotifyAccessibleEvent( AccessibleEventId::INVALIDATE_ALL_CHILDREN, Any(), Any() );
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeck_Impl::Dying()
    {
        // the tool panel deck is dying, so dispose ourself
        m_rAntiImpl.dispose();
    }

    //==================================================================================================================
    //= AccessibleToolPanelDeck
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    AccessibleToolPanelDeck::AccessibleToolPanelDeck( const Reference< XAccessible >& i_rAccessibleParent,
            ::svt::ToolPanelDeck& i_rPanelDeck )
        :AccessibleToolPanelDeck_Base( i_rPanelDeck.GetWindowPeer() )
        ,m_pImpl( new AccessibleToolPanelDeck_Impl( *this, i_rAccessibleParent, i_rPanelDeck ) )
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    AccessibleToolPanelDeck::~AccessibleToolPanelDeck()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Int32 SAL_CALL AccessibleToolPanelDeck::getAccessibleChildCount(  ) throw (RuntimeException)
    {
        MethodGuard aGuard( *m_pImpl );

        return sal_Int32( m_pImpl->m_pPanelDeck->GetPanelCount() );
        // TODO: this should probably be +1 - the active panel itself should also be a child ...
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleToolPanelDeck::getAccessibleChild( sal_Int32 i_nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *m_pImpl );

        if ( ( i_nIndex < 0 ) || ( i_nIndex >= m_pImpl->m_pPanelDeck->GetPanelCount() ) )
            throw IndexOutOfBoundsException( ::rtl::OUString(), *this );

        return m_pImpl->getPanelItemAccessible( i_nIndex, true );
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleToolPanelDeck::getAccessibleParent(  ) throw (RuntimeException)
    {
        MethodGuard aGuard( *m_pImpl );
        return m_pImpl->m_xAccessibleParent;
    }

    //------------------------------------------------------------------------------------------------------------------
    sal_Int16 SAL_CALL AccessibleToolPanelDeck::getAccessibleRole(  ) throw (RuntimeException)
    {
        MethodGuard aGuard( *m_pImpl );
        return AccessibleRole::PANEL;
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleToolPanelDeck::getAccessibleAtPoint( const UnoPoint& i_rPoint ) throw (RuntimeException)
    {
        MethodGuard aGuard( *m_pImpl );

        // check whether the given point contains a panel item
        ::svt::PDeckLayouter pLayouter( m_pImpl->m_pPanelDeck->GetLayouter() );
        ENSURE_OR_THROW( pLayouter.get() != NULL, "unexpected NULL layouter for the panel deck" );

        const UnoPoint aLocation( getLocationOnScreen() );
        const ::Point aRequestedScreenLocation( i_rPoint.X + aLocation.X, i_rPoint.Y + aLocation.Y );

        ::boost::optional< size_t > aPanelItemPos( pLayouter->GetPanelItemFromScreenPos( aRequestedScreenLocation ) );
        if ( !aPanelItemPos )
            // TODO: check the panel window itself
            return NULL;

        return getAccessibleChild( *aPanelItemPos );
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AccessibleToolPanelDeck::grabFocus(  ) throw (RuntimeException)
    {
        MethodGuard aGuard( *m_pImpl );
        m_pImpl->m_pPanelDeck->GrabFocus();
    }

    //------------------------------------------------------------------------------------------------------------------
    void SAL_CALL AccessibleToolPanelDeck::disposing()
    {
        AccessibleToolPanelDeck_Base::disposing();
        m_pImpl->dispose();
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > AccessibleToolPanelDeck::GetChildAccessible( const VclWindowEvent& i_rVclWindowEvent )
    {
        // don't let the base class generate any A11Y events from VclWindowEvent, we completely manage those
        // A11Y events ourself
        return NULL;
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeck::FillAccessibleStateSet( ::utl::AccessibleStateSetHelper& i_rStateSet )
    {
        AccessibleToolPanelDeck_Base::FillAccessibleStateSet( i_rStateSet );
        if ( m_pImpl->isDisposed() )
        {
            i_rStateSet.AddState( AccessibleStateType::DEFUNC );
        }
        else
        {
            i_rStateSet.AddState( AccessibleStateType::FOCUSABLE );
        }
    }

//......................................................................................................................
} // namespace accessibility
//......................................................................................................................
