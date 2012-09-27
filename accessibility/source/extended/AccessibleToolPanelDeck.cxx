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


#include "accessibility/extended/AccessibleToolPanelDeck.hxx"

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <com/sun/star/lang/DisposedException.hpp>

#include <svtools/toolpanel/toolpaneldeck.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/svapp.hxx>
#include <osl/mutex.hxx>
#include <unotools/accessiblestatesethelper.hxx>
#include <tools/diagnose_ex.h>

#include <boost/noncopyable.hpp>

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

        virtual ~AccessibleToolPanelDeck_Impl();

        Reference< XAccessible >    getOwnAccessible() const;
        Reference< XAccessible >    getActivePanelAccessible();

    protected:
        // IToolPanelDeckListener
        virtual void PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition );
        virtual void PanelRemoved( const size_t i_nPosition );
        virtual void ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
        virtual void LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter );
        virtual void Dying();

    public:
        AccessibleToolPanelDeck&    m_rAntiImpl;
        Reference< XAccessible >    m_xAccessibleParent;
        ::svt::ToolPanelDeck*       m_pPanelDeck;

        Reference< XAccessible >        m_xActivePanelAccessible;
    };

    //==================================================================================================================
    //= MethodGuard
    //==================================================================================================================
    namespace
    {
        class MethodGuard
        {
        public:
            MethodGuard( AccessibleToolPanelDeck_Impl& i_rImpl )
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

    //==================================================================================================================
    //= AccessibleToolPanelDeck_Impl - implementation
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    AccessibleToolPanelDeck_Impl::AccessibleToolPanelDeck_Impl( AccessibleToolPanelDeck& i_rAntiImpl, const Reference< XAccessible >& i_rAccessibleParent,
            ::svt::ToolPanelDeck& i_rPanelDeck )
        :m_rAntiImpl( i_rAntiImpl )
        ,m_xAccessibleParent( i_rAccessibleParent )
        ,m_pPanelDeck( &i_rPanelDeck )
        ,m_xActivePanelAccessible()
    {
        m_pPanelDeck->AddListener( *this );
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
        m_xAccessibleParent.clear();
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeck_Impl::checkDisposed()
    {
        if ( isDisposed() )
            throw DisposedException( OUString(), *&m_rAntiImpl );
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
    Reference< XAccessible > AccessibleToolPanelDeck_Impl::getActivePanelAccessible()
    {
        ENSURE_OR_RETURN( !isDisposed(), "AccessibleToolPanelDeck_Impl::getActivePanelAccessible: already disposed!", NULL );

        if ( !m_xActivePanelAccessible.is() )
        {
            ::boost::optional< size_t > aActivePanel( m_pPanelDeck->GetActivePanel() );
            ENSURE_OR_RETURN( !!aActivePanel, "AccessibleToolPanelDeck_Impl::getActivePanelAccessible: this should not be called without an active panel!", NULL );
            ::svt::PToolPanel pActivePanel( m_pPanelDeck->GetPanel( *aActivePanel ) );
            ENSURE_OR_RETURN( pActivePanel.get() != NULL, "AccessibleToolPanelDeck_Impl::getActivePanelAccessible: no active panel!", NULL );
            m_xActivePanelAccessible = pActivePanel->CreatePanelAccessible( getOwnAccessible() );
            OSL_ENSURE( m_xActivePanelAccessible.is(), "AccessibleToolPanelDeck_Impl::getActivePanelAccessible: illegal accessible returned by the panel!" );
        }

        return m_xActivePanelAccessible;
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeck_Impl::PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        (void)i_pPanel;
        (void)i_nPosition;
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeck_Impl::PanelRemoved( const size_t i_nPosition )
    {
        (void)i_nPosition;
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeck_Impl::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
    {
        if ( !!i_rOldActive )
        {
            if ( !m_xActivePanelAccessible.is() )
            {
                // again, this might in theory happen if the XAccessible for the active panel has never before been requested.
                // In this case, just say that all our children are invalid, so they all must be re-requested.
                m_rAntiImpl.NotifyAccessibleEvent( AccessibleEventId::INVALIDATE_ALL_CHILDREN, Any(), Any() );
            }
            else
            {
                m_rAntiImpl.NotifyAccessibleEvent( AccessibleEventId::CHILD, makeAny( m_xActivePanelAccessible ), Any() );
            }
        }

        m_xActivePanelAccessible.clear();

        if ( !!i_rNewActive )
        {
            m_rAntiImpl.NotifyAccessibleEvent( AccessibleEventId::CHILD, Any(), makeAny( getActivePanelAccessible() ) );
        }
    }

    //------------------------------------------------------------------------------------------------------------------
    void AccessibleToolPanelDeck_Impl::LayouterChanged( const ::svt::PDeckLayouter& i_rNewLayouter )
    {
        MethodGuard aGuard( *this );

        (void)i_rNewLayouter;
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

        sal_Int32 nChildCount( m_pImpl->m_pPanelDeck->GetLayouter()->GetAccessibleChildCount() );

        ::boost::optional< size_t > aActivePanel( m_pImpl->m_pPanelDeck->GetActivePanel() );
        if ( !!aActivePanel )
            return ++nChildCount;

        return nChildCount;
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleToolPanelDeck::getAccessibleChild( sal_Int32 i_nIndex ) throw (IndexOutOfBoundsException, RuntimeException)
    {
        MethodGuard aGuard( *m_pImpl );

        const sal_Int32 nChildCount( getAccessibleChildCount() );
        if ( ( i_nIndex < 0 ) || ( i_nIndex >= nChildCount ) )
            throw IndexOutOfBoundsException( OUString(), *this );

        // first "n" children are provided by the layouter
        const size_t nLayouterCount( m_pImpl->m_pPanelDeck->GetLayouter()->GetAccessibleChildCount() );
        if ( size_t( i_nIndex ) < nLayouterCount )
            return m_pImpl->m_pPanelDeck->GetLayouter()->GetAccessibleChild(
                size_t( i_nIndex ),
                m_pImpl->getOwnAccessible()
            );

        // the last child is the XAccessible of the active panel
        return m_pImpl->getActivePanelAccessible();
    }

    //------------------------------------------------------------------------------------------------------------------
    Reference< XAccessible > SAL_CALL AccessibleToolPanelDeck::getAccessibleParent(  ) throw (RuntimeException)
    {
        MethodGuard aGuard( *m_pImpl );
        const Reference< XAccessible > xParent = implGetForeignControlledParent();
        if ( xParent.is() )
            return xParent;
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

        const ::Point aRequestedPoint( VCLUnoHelper::ConvertToVCLPoint( i_rPoint ) );
        // check the panel window itself
        const ::Window& rActivePanelAnchor( m_pImpl->m_pPanelDeck->GetPanelWindowAnchor() );
        const Rectangle aPanelAnchorArea( rActivePanelAnchor.GetPosPixel(), rActivePanelAnchor.GetOutputSizePixel() );
        if ( aPanelAnchorArea.IsInside( aRequestedPoint ) )
            // note that this assumes that the Window which actually implements the concrete panel covers
            // the complete area of its "anchor" Window. But this is ensured by the ToolPanelDeck implementation.
            return m_pImpl->getActivePanelAccessible();

        // check the XAccessible instances provided by the layouter
        try
        {
            const ::svt::PDeckLayouter pLayouter( m_pImpl->m_pPanelDeck->GetLayouter() );
            ENSURE_OR_THROW( pLayouter.get() != NULL, "invalid layouter" );

            const size_t nLayouterChildren = pLayouter->GetAccessibleChildCount();
            for ( size_t i=0; i<nLayouterChildren; ++i )
            {
                const Reference< XAccessible > xLayoutItemAccessible( pLayouter->GetAccessibleChild( i, m_pImpl->getOwnAccessible() ), UNO_SET_THROW );
                const Reference< XAccessibleComponent > xLayoutItemComponent( xLayoutItemAccessible->getAccessibleContext(), UNO_QUERY_THROW );
                const ::Rectangle aLayoutItemBounds( VCLUnoHelper::ConvertToVCLRect( xLayoutItemComponent->getBounds() ) );
                if ( aLayoutItemBounds.IsInside( aRequestedPoint ) )
                    return xLayoutItemAccessible;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        return NULL;
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
        (void)i_rVclWindowEvent;
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
