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

#include "precompiled_sd.hxx"

#include "ToolPanelDrawer.hxx"
#include "taskpane/TitleBar.hxx"

#include <tools/diagnose_ex.h>

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

    //==================================================================================================================
    //= ToolPanelDrawer
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ToolPanelDrawer::ToolPanelDrawer( ::Window& i_rParentWindow, ::svt::IToolPanelDeck& i_rPanels )
        :m_rParentWindow( i_rParentWindow )
        ,m_rPanelDeck( i_rPanels )
        ,m_aDrawers()
    {
        m_rPanelDeck.AddListener( *this );

        // simulate PanelInserted events for the panels which are already there
        for ( size_t i=0; i<m_rPanelDeck.GetPanelCount(); ++i )
            PanelInserted( m_rPanelDeck.GetPanel( i ), i );
    }

    //------------------------------------------------------------------------------------------------------------------
    ToolPanelDrawer::~ToolPanelDrawer()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    IMPLEMENT_IREFERENCE( ToolPanelDrawer )

    //------------------------------------------------------------------------------------------------------------------
    Rectangle ToolPanelDrawer::Layout( const Rectangle& i_rDeckPlayground )
    {
        const size_t nPanelCount( m_rPanelDeck.GetPanelCount() );
        if ( nPanelCount == 0 )
            return i_rDeckPlayground;

        const int nWidth( i_rDeckPlayground.GetWidth() );
        const ::boost::optional< size_t > aActivePanel( m_rPanelDeck.GetActivePanel() );

        // arrange the title bars which are *above* the active panel (or *all* if there is no active panel), plus
        // the title bar of the active panel itself
        Point aUpperDrawerPos( i_rDeckPlayground.TopLeft() );
        const size_t nUpperBound = !!aActivePanel ? *aActivePanel : nPanelCount - 1;
        for ( size_t i=0; i<=nUpperBound; ++i )
        {
            sal_uInt32 nDrawerHeight = m_aDrawers[i]->GetPreferredHeight( nWidth );
            m_aDrawers[i]->GetWindow()->SetPosSizePixel(
                aUpperDrawerPos, Size( nWidth, nDrawerHeight ) );
            aUpperDrawerPos.Move( 0, nDrawerHeight );
        }

        // arrange title bars which are below the active panel (or *none* if there is no active panel)
        Point aLowerDrawerPos( i_rDeckPlayground.BottomLeft() );
        for ( size_t j = nPanelCount - 1; j > nUpperBound; --j )
        {
            sal_uInt32 nDrawerHeight = m_aDrawers[j]->GetPreferredHeight( nWidth );
            m_aDrawers[j]->GetWindow()->SetPosSizePixel(
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

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::Destroy()
    {
        while ( !m_aDrawers.empty() )
            impl_removeTitleBar( 0 );
        m_rPanelDeck.RemoveListener( *this );
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::SetFocusToPanelSelector()
    {
        const size_t nPanelCount( m_rPanelDeck.GetPanelCount() );
        if ( !nPanelCount )
            // nothing to focus
            return;
        ::boost::optional< size_t > aActivePanel( m_rPanelDeck.GetActivePanel() );
        if ( !aActivePanel )
            aActivePanel = 0;
        ENSURE_OR_RETURN_VOID( *aActivePanel < m_aDrawers.size(), "ToolPanelDrawer::SetFocusToPanelSelector: invalid active panel, or inconsistent drawers!" );
        m_aDrawers[ *aActivePanel ]->GetWindow()->GrabFocus();
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::PanelInserted( const ::svt::PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        OSL_PRECOND( i_nPosition <= m_aDrawers.size(), "ToolPanelDrawer::PanelInserted: inconsistency!" );

        ::boost::shared_ptr< TitleBar > pTitleBar( new TitleBar( &m_rParentWindow, i_pPanel->GetDisplayName(), TitleBar::TBT_CONTROL_TITLE, true ) );
        pTitleBar->GetWindow()->Show();
        pTitleBar->GetWindow()->AddEventListener( LINK( this, ToolPanelDrawer, OnWindowEvent ) );
        m_aDrawers.insert( m_aDrawers.begin() + i_nPosition, pTitleBar );
        impl_triggerRearrange();
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::PanelRemoved( const size_t i_nPosition )
    {
        impl_removeTitleBar( i_nPosition );
        impl_triggerRearrange();
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::impl_triggerRearrange() const
    {
        // this is somewhat hacky, it assumes that the parent of our panels is a tool panel deck, which, in its
        // Resize implementation, rearrances all elements.
        m_rParentWindow.Resize();
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
    {
        if ( !!i_rOldActive )
        {
            OSL_ENSURE( *i_rOldActive < m_aDrawers.size(), "ToolPanelDrawer::ActivePanelChanged: illegal old index!" );
            m_aDrawers[ *i_rOldActive ]->Expand( false );
        }

        if ( !!i_rNewActive )
        {
            OSL_ENSURE( *i_rNewActive < m_aDrawers.size(), "ToolPanelDrawer::ActivePanelChanged: illegal new index!" );
            m_aDrawers[ *i_rNewActive ]->Expand( true );
        }

        impl_triggerRearrange();
    }

    //------------------------------------------------------------------------------------------------------------------
    size_t ToolPanelDrawer::impl_getPanelPositionFromWindow( const Window* i_pTitleBarWindow ) const
    {
        for (   ::std::vector< PTitleBar >::const_iterator drawerPos = m_aDrawers.begin();
                drawerPos != m_aDrawers.end();
                ++drawerPos
            )
        {
            if ( (*drawerPos)->GetWindow() == i_pTitleBarWindow )
                return drawerPos - m_aDrawers.begin();
        }
        return m_aDrawers.size();
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::impl_removeTitleBar( const size_t i_nPosition )
    {
        OSL_PRECOND( i_nPosition < m_aDrawers.size(), "ToolPanelDrawer::impl_removeTitleBar: invalid panel position!" );
        m_aDrawers[ i_nPosition ]->GetWindow()->RemoveEventListener( LINK( this, ToolPanelDrawer, OnWindowEvent ) );
        OSL_ENSURE( m_aDrawers[ i_nPosition ].unique(), "ToolPanelDrawer::impl_removeTitleBar: somebody else is still holding a reference!" );
        m_aDrawers.erase( m_aDrawers.begin() + i_nPosition );
    }

    //------------------------------------------------------------------------------------------------------------------
    IMPL_LINK( ToolPanelDrawer, OnWindowEvent, VclSimpleEvent*, i_pEvent )
    {
        const VclWindowEvent* pWindowEvent = PTR_CAST( VclWindowEvent, i_pEvent );
        ENSURE_OR_RETURN( pWindowEvent, "no WindowEvent", 0L );

        bool bActivatePanel = false;
        switch ( pWindowEvent->GetId() )
        {
            case VCLEVENT_WINDOW_MOUSEBUTTONUP:
            {
                const MouseEvent* pMouseEvent = static_cast< const MouseEvent* >( pWindowEvent->GetData() );
                ENSURE_OR_RETURN( pMouseEvent, "no mouse event with MouseButtonUp", 0L );
                if ( pMouseEvent->GetButtons() == MOUSE_LEFT )
                {
                    bActivatePanel = true;
                }
            }
            break;
            case VCLEVENT_WINDOW_KEYINPUT:
            {
                const KeyEvent* pKeyEvent = static_cast< const KeyEvent* >( pWindowEvent->GetData() );
                ENSURE_OR_RETURN( pKeyEvent, "no key event with KeyInput", 0L );
                const KeyCode& rKeyCode( pKeyEvent->GetKeyCode() );
                if ( ( rKeyCode.GetModifier() == 0 ) && ( rKeyCode.GetCode() == KEY_RETURN ) )
                {
                    bActivatePanel = true;
                }
            }
            break;
        }
        if ( bActivatePanel )
        {
            const size_t nPanelPos = impl_getPanelPositionFromWindow( pWindowEvent->GetWindow() );
            m_rPanelDeck.ActivatePanel( nPanelPos );
            return 1L;
        }
        return 0L;
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDrawer::Dying()
    {
        Destroy();
    }

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................
