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


#include <svtools/toolpanel/drawerlayouter.hxx>
#include "toolpaneldrawer.hxx"

#include <com/sun/star/accessibility/XAccessible.hpp>

#include <comphelper/accimplaccess.hxx>
#include <tools/diagnose_ex.h>


namespace svt
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::accessibility::XAccessible;


    //= DrawerDeckLayouter


    DrawerDeckLayouter::DrawerDeckLayouter( vcl::Window& i_rParentWindow, IToolPanelDeck& i_rPanels )
        :m_rParentWindow( i_rParentWindow )
        ,m_rPanelDeck( i_rPanels )
        ,m_aDrawers()
        ,m_aLastKnownActivePanel()
    {
        m_rPanelDeck.AddListener( *this );

        // simulate PanelInserted events for the panels which are already there
        for ( size_t i=0; i<m_rPanelDeck.GetPanelCount(); ++i )
            PanelInserted( m_rPanelDeck.GetPanel( i ), i );
    }


    DrawerDeckLayouter::~DrawerDeckLayouter()
    {
    }


    Rectangle DrawerDeckLayouter::Layout( const Rectangle& i_rDeckPlayground )
    {
        const size_t nPanelCount( m_rPanelDeck.GetPanelCount() );
        if ( nPanelCount == 0 )
            return i_rDeckPlayground;

        const int nWidth( i_rDeckPlayground.GetWidth() );
        ::boost::optional< size_t > aActivePanel( m_rPanelDeck.GetActivePanel() );
        if ( !aActivePanel )
            aActivePanel = m_aLastKnownActivePanel;

        // arrange the title bars which are *above* the active panel (or *all* if there is no active panel), plus
        // the title bar of the active panel itself
        Point aUpperDrawerPos( i_rDeckPlayground.TopLeft() );
        const size_t nUpperBound = !!aActivePanel ? *aActivePanel : nPanelCount - 1;
        for ( size_t i=0; i<=nUpperBound; ++i )
        {
            long const nDrawerHeight = m_aDrawers[i]->GetPreferredHeightPixel();
            m_aDrawers[i]->SetPosSizePixel(
                aUpperDrawerPos, Size( nWidth, nDrawerHeight ) );
            aUpperDrawerPos.Move( 0, nDrawerHeight );
        }

        // arrange title bars which are below the active panel (or *none* if there is no active panel)
        Point aLowerDrawerPos( i_rDeckPlayground.BottomLeft() );
        for ( size_t j = nPanelCount - 1; j > nUpperBound; --j )
        {
            long const nDrawerHeight = m_aDrawers[j]->GetPreferredHeightPixel();
            m_aDrawers[j]->SetPosSizePixel(
                Point( aLowerDrawerPos.X(), aLowerDrawerPos.Y() - nDrawerHeight + 1 ),
                Size( nWidth, nDrawerHeight )
            );
            aLowerDrawerPos.Move( 0, -nDrawerHeight );
        }

        // fincally calculate the rectangle for the active panel
        return Rectangle(
            aUpperDrawerPos,
            Size( nWidth, aLowerDrawerPos.Y() - aUpperDrawerPos.Y() + 1 )
        );
    }


    void DrawerDeckLayouter::Destroy()
    {
        while ( !m_aDrawers.empty() )
            impl_removeDrawer( 0 );
        m_rPanelDeck.RemoveListener( *this );
    }


    void DrawerDeckLayouter::SetFocusToPanelSelector()
    {
        const size_t nPanelCount( m_rPanelDeck.GetPanelCount() );
        if ( !nPanelCount )
            // nothing to focus
            return;
        ::boost::optional< size_t > aActivePanel( m_rPanelDeck.GetActivePanel() );
        if ( !aActivePanel )
            aActivePanel = 0;
        ENSURE_OR_RETURN_VOID( *aActivePanel < m_aDrawers.size(), "DrawerDeckLayouter::SetFocusToPanelSelector: invalid active panel, or inconsistent drawers!" );
        m_aDrawers[ *aActivePanel ]->GrabFocus();
    }


    size_t DrawerDeckLayouter::GetAccessibleChildCount() const
    {
        return m_aDrawers.size();
    }


    Reference< XAccessible > DrawerDeckLayouter::GetAccessibleChild( const size_t i_nChildIndex, const Reference< XAccessible >& i_rParentAccessible )
    {
        ENSURE_OR_RETURN( i_nChildIndex < m_aDrawers.size(), "illegal index", NULL );

        VclPtr<ToolPanelDrawer> pDrawer( m_aDrawers[ i_nChildIndex ] );

        Reference< XAccessible > xItemAccessible = pDrawer->GetAccessible( false );
        if ( !xItemAccessible.is() )
        {
            xItemAccessible = pDrawer->GetAccessible();
            ENSURE_OR_RETURN( xItemAccessible.is(), "illegal accessible provided by the drawer implementation!", NULL );
            OSL_VERIFY( ::comphelper::OAccessibleImplementationAccess::setAccessibleParent( xItemAccessible->getAccessibleContext(),
                i_rParentAccessible ) );
        }

        return xItemAccessible;
    }


    void DrawerDeckLayouter::PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        OSL_PRECOND( i_nPosition <= m_aDrawers.size(), "DrawerDeckLayouter::PanelInserted: inconsistency!" );

        VclPtrInstance<ToolPanelDrawer> pDrawer( m_rParentWindow, i_pPanel->GetDisplayName() );
        pDrawer->SetHelpId( i_pPanel->GetHelpID() );
        // proper Z-Order
        if ( i_nPosition == 0 )
        {
            pDrawer->SetZOrder( NULL, ZOrderFlags::First );
        }
        else
        {
            ToolPanelDrawer* pFirstDrawer( m_aDrawers[ i_nPosition - 1 ] );
            pDrawer->SetZOrder( pFirstDrawer, ZOrderFlags::Behind );
        }

        pDrawer->Show();
        pDrawer->AddEventListener( LINK( this, DrawerDeckLayouter, OnWindowEvent ) );
        m_aDrawers.insert( m_aDrawers.begin() + i_nPosition, pDrawer );
        impl_triggerRearrange();
    }


    void DrawerDeckLayouter::PanelRemoved( const size_t i_nPosition )
    {
        impl_removeDrawer( i_nPosition );
        impl_triggerRearrange();
    }


    void DrawerDeckLayouter::impl_triggerRearrange() const
    {
        // this is somewhat hacky, it assumes that the parent of our panels is a tool panel deck, which, in its
        // Resize implementation, rearrances all elements.
        m_rParentWindow.Resize();
    }


    void DrawerDeckLayouter::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
    {
        if ( !!i_rOldActive )
        {
            OSL_ENSURE( *i_rOldActive < m_aDrawers.size(), "DrawerDeckLayouter::ActivePanelChanged: illegal old index!" );
            m_aDrawers[ *i_rOldActive ]->SetExpanded( false );
        }

        if ( !!i_rNewActive )
        {
            OSL_ENSURE( *i_rNewActive < m_aDrawers.size(), "DrawerDeckLayouter::ActivePanelChanged: illegal new index!" );
            m_aDrawers[ *i_rNewActive ]->SetExpanded( true );
        }

        impl_triggerRearrange();
    }


    void DrawerDeckLayouter::LayouterChanged( const PDeckLayouter& i_rNewLayouter )
    {
        // not interested in
        (void)i_rNewLayouter;
    }


    size_t DrawerDeckLayouter::impl_getPanelPositionFromWindow( const vcl::Window* i_pDrawerWindow ) const
    {
        for ( auto drawerPos = m_aDrawers.begin(); drawerPos != m_aDrawers.end(); ++drawerPos )
        {
            if ( drawerPos->get() == i_pDrawerWindow )
                return drawerPos - m_aDrawers.begin();
        }
        return m_aDrawers.size();
    }


    void DrawerDeckLayouter::impl_removeDrawer( const size_t i_nPosition )
    {
        OSL_PRECOND( i_nPosition < m_aDrawers.size(), "DrawerDeckLayouter::impl_removeDrawer: invalid panel position!" );
        m_aDrawers[ i_nPosition ]->RemoveEventListener( LINK( this, DrawerDeckLayouter, OnWindowEvent ) );
        m_aDrawers.erase( m_aDrawers.begin() + i_nPosition );
    }


    IMPL_LINK_TYPED( DrawerDeckLayouter, OnWindowEvent, VclWindowEvent&, rWindowEvent, void )
    {
        bool bActivatePanel = false;
        switch ( rWindowEvent.GetId() )
        {
            case VCLEVENT_WINDOW_MOUSEBUTTONUP:
            {
                const MouseEvent* pMouseEvent = static_cast< const MouseEvent* >( rWindowEvent.GetData() );
                ENSURE_OR_RETURN_VOID( pMouseEvent, "no mouse event with MouseButtonUp" );
                if ( pMouseEvent->GetButtons() == MOUSE_LEFT )
                {
                    bActivatePanel = true;
                }
            }
            break;
            case VCLEVENT_WINDOW_KEYINPUT:
            {
                const KeyEvent* pKeyEvent = static_cast< const KeyEvent* >( rWindowEvent.GetData() );
                ENSURE_OR_RETURN_VOID( pKeyEvent, "no key event with KeyInput" );
                const vcl::KeyCode& rKeyCode( pKeyEvent->GetKeyCode() );
                if ( ( rKeyCode.GetModifier() == 0 ) && ( rKeyCode.GetCode() == KEY_RETURN ) )
                {
                    bActivatePanel = true;
                }
            }
            break;
        }
        if ( bActivatePanel )
        {
            const size_t nPanelPos = impl_getPanelPositionFromWindow( rWindowEvent.GetWindow() );
            if ( nPanelPos != m_rPanelDeck.GetActivePanel() )
            {
                m_rPanelDeck.ActivatePanel( nPanelPos );
            }
            else
            {
                PToolPanel pPanel( m_rPanelDeck.GetPanel( nPanelPos ) );
                pPanel->GrabFocus();
            }
        }
    }


    void DrawerDeckLayouter::Dying()
    {
        Destroy();
    }


} // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
