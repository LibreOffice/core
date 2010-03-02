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

#include "precompiled_svtools.hxx"

#include "dummypanel.hxx"
#include "toolpanelcollection.hxx"

#include "svtools/toolpanel/toolpaneldeck.hxx"
#include "svtools/toolpanel/tablayouter.hxx"

#include <boost/optional.hpp>

//........................................................................
namespace svt
{
//........................................................................

    enum DeckAction
    {
        /// activates the first panel
        ACTION_ACTIVATE_FIRST,
        // activates the panel after the currently active panel
        ACTION_ACTIVATE_NEXT,
        // activates the panel before the currently active panel
        ACTION_ACTIVATE_PREV,
        // activates the last panel
        ACTION_ACTIVATE_LAST,

        // toggles the focus between the active panel and the panel selector
        ACTION_TOGGLE_FOCUS,
    };

    //====================================================================
    //= ToolPanelDeck_Impl
    //====================================================================
    class ToolPanelDeck_Impl : public IToolPanelContainerListener
    {
    public:
        ToolPanelDeck_Impl( ToolPanelDeck& i_rDeck )
            :m_rDeck( i_rDeck )
            ,m_pPanels( new( ToolPanelCollection ) )
            ,m_pDummyPanel( new DummyPanel )
            ,m_aActivePanel()
            ,m_pLayouter()
            ,m_aPanelPlayground()
        {
            m_pPanels->AddListener( *this );
        }

        PToolPanelContainer GetPanels() const { return m_pPanels; }

        size_t              GetActivePanel() const;
        void                ActivatePanel( const size_t i_nPanel );

        PDeckLayouter       GetLayouter() const { return m_pLayouter; }
        void                SetLayouter( const PDeckLayouter& i_pNewLayouter );

        void                AddListener( IToolPanelDeckListener& i_rListener );
        void                RemoveListener( IToolPanelDeckListener& i_rListener );

        /// re-layouts everything
        void    LayoutAll() { ImplDoLayout(); }

        void                DoAction( const DeckAction i_eAction );

        void                FocusActivePanel();

    protected:
        // IToolPanelContainerListener
        virtual void        PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition );

    private:
        void                ImplDoLayout();
        PToolPanel          GetActiveOrDummyPanel_Impl();

    private:
        ToolPanelDeck&      m_rDeck;

        PToolPanelContainer m_pPanels;
        PToolPanel          m_pDummyPanel;
        ::boost::optional< size_t >
                            m_aActivePanel;

        PDeckLayouter       m_pLayouter;
        Rectangle           m_aPanelPlayground;

        ::std::vector< IToolPanelDeckListener* >
                            m_aListeners;
    };

    //--------------------------------------------------------------------
    PToolPanel ToolPanelDeck_Impl::GetActiveOrDummyPanel_Impl()
    {
        if ( !m_aActivePanel )
            return m_pDummyPanel;
        return m_pPanels->GetPanel( *m_aActivePanel );
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::SetLayouter( const PDeckLayouter& i_pNewLayouter )
    {
        OSL_ENSURE( i_pNewLayouter.get(), "ToolPanelDeck_Impl::SetLayouter: invalid layouter!" );
        if ( !i_pNewLayouter.get() )
            return;
        m_pLayouter = i_pNewLayouter;

        ImplDoLayout();
    }

    //--------------------------------------------------------------------
    size_t ToolPanelDeck_Impl::GetActivePanel() const
    {
        OSL_PRECOND( !!m_aActivePanel, "ToolPanelDeck_Impl::GetActivePanel: no active panel (yet)!" );
        if ( !m_aActivePanel )
            return 0;
        return *m_aActivePanel;
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::ActivatePanel( const size_t i_nPanel )
    {
        OSL_ENSURE( i_nPanel < m_pPanels->GetPanelCount(), "ToolPanelDeck_Impl::ActivatePanel: illegal panel no.!" );
        if ( i_nPanel >= m_pPanels->GetPanelCount() )
            return;

        if ( m_aActivePanel == i_nPanel )
            return;

        // hide the old panel
        const PToolPanel pOldActive( GetActiveOrDummyPanel_Impl() );
        pOldActive->Hide();

        const ::boost::optional< size_t > aOldPanel( m_aActivePanel );
        m_aActivePanel = i_nPanel;

        // position and show the new panel
        const PToolPanel pNewActive( GetActiveOrDummyPanel_Impl() );
        pNewActive->SetPosSizePixel( m_aPanelPlayground );
        pNewActive->Show();
        pNewActive->GrabFocus();

        // notify listeners
        for (   ::std::vector< IToolPanelDeckListener* >::iterator loop = m_aListeners.begin();
                loop != m_aListeners.end();
                ++loop
            )
        {
            (*loop)->ActivePanelChanged( aOldPanel, *m_aActivePanel );
        }
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::ImplDoLayout()
    {
        const Rectangle aDeckPlayground( Point(), m_rDeck.GetOutputSizePixel() );

        // let the layouter do the main work
        m_aPanelPlayground = aDeckPlayground;
        OSL_ENSURE( m_pLayouter.get(), "ToolPanelDeck_Impl::ImplDoLayout: no layouter!" );
        if ( m_pLayouter.get() )
            m_aPanelPlayground = m_pLayouter->Layout( aDeckPlayground );

        // and position the active panel
        const PToolPanel pActive( GetActiveOrDummyPanel_Impl() );
        pActive->SetPosSizePixel( m_aPanelPlayground );
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::AddListener( IToolPanelDeckListener& i_rListener )
    {
        m_aListeners.push_back( &i_rListener );
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::RemoveListener( IToolPanelDeckListener& i_rListener )
    {
        for (   ::std::vector< IToolPanelDeckListener* >::iterator lookup = m_aListeners.begin();
                lookup != m_aListeners.end();
                ++lookup
            )
        {
            if ( *lookup == &i_rListener )
            {
                m_aListeners.erase( lookup );
                return;
            }
        }
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::DoAction( const DeckAction i_eAction )
    {
        ::boost::optional< size_t > aActivatePanel;
        const size_t nPanelCount( GetPanels()->GetPanelCount() );
        const size_t nActivePanel( GetActivePanel() );

        switch ( i_eAction )
        {
        case ACTION_ACTIVATE_FIRST:
            if ( nPanelCount > 0 )
                aActivatePanel = 0;
            break;
        case ACTION_ACTIVATE_PREV:
            if ( nActivePanel > 0 )
                aActivatePanel = nActivePanel - 1;
            break;
        case ACTION_ACTIVATE_NEXT:
            if ( nActivePanel < nPanelCount - 1 )
                aActivatePanel = nActivePanel + 1;
            break;
        case ACTION_ACTIVATE_LAST:
            if ( nPanelCount > 0 )
                aActivatePanel = nPanelCount - 1;
            break;
        case ACTION_TOGGLE_FOCUS:
            {
                PToolPanel pActivePanel( GetActiveOrDummyPanel_Impl() );
                if ( !pActivePanel->HasFocus() )
                    pActivePanel->GrabFocus();
                else
                    GetLayouter()->SetFocusToPanelSelector();
            }
            break;
        }

        if ( !!aActivatePanel )
        {
            ActivatePanel( *aActivatePanel );
        }
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::FocusActivePanel()
    {
        PToolPanel pActivePanel( GetActiveOrDummyPanel_Impl() );
        pActivePanel->GrabFocus();
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        if ( !!m_aActivePanel )
        {
            if ( i_nPosition <= *m_aActivePanel )
                ++*m_aActivePanel;
        }
    }

    //====================================================================
    //= ToolPanelDeck
    //====================================================================
    //--------------------------------------------------------------------
    ToolPanelDeck::ToolPanelDeck( Window& i_rParent, const WinBits i_nStyle )
        :Control( &i_rParent, i_nStyle )
        ,m_pImpl( new ToolPanelDeck_Impl( *this ) )
    {
        // use a default layouter
        SetLayouter( PDeckLayouter( new TabDeckLayouter( TABS_RIGHT, *this ) ) );
    }

    //--------------------------------------------------------------------
    ToolPanelDeck::~ToolPanelDeck()
    {
        GetLayouter()->Destroy();
    }

    //--------------------------------------------------------------------
    size_t ToolPanelDeck::GetActivePanel() const
    {
        return m_pImpl->GetActivePanel();
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck::ActivatePanel( const size_t i_nPanel )
    {
        m_pImpl->ActivatePanel( i_nPanel );
    }

    //--------------------------------------------------------------------
    PDeckLayouter ToolPanelDeck::GetLayouter() const
    {
        return m_pImpl->GetLayouter();
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck::SetLayouter( const PDeckLayouter& i_pNewLayouter )
    {
        return m_pImpl->SetLayouter( i_pNewLayouter );
    }

    //--------------------------------------------------------------------
    PToolPanelContainer ToolPanelDeck::GetPanels() const
    {
        return m_pImpl->GetPanels();
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck::AddListener( IToolPanelDeckListener& i_rListener )
    {
        m_pImpl->AddListener( i_rListener );
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck::RemoveListener( IToolPanelDeckListener& i_rListener )
    {
        m_pImpl->RemoveListener( i_rListener );
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck::Resize()
    {
        Control::Resize();
        m_pImpl->LayoutAll();
    }

    //--------------------------------------------------------------------
    long ToolPanelDeck::Notify( NotifyEvent& i_rNotifyEvent )
    {
        bool bHandled = false;
        if ( i_rNotifyEvent.GetType() == EVENT_KEYINPUT )
        {
            const KeyEvent* pEvent = i_rNotifyEvent.GetKeyEvent();
            const KeyCode& rKeyCode = pEvent->GetKeyCode();
            if ( rKeyCode.GetModifier() == KEY_MOD1 )
            {
                bHandled = true;
                switch ( rKeyCode.GetCode() )
                {
                case KEY_HOME:
                    m_pImpl->DoAction( ACTION_ACTIVATE_FIRST );
                    break;
                case KEY_PAGEUP:
                    m_pImpl->DoAction( ACTION_ACTIVATE_PREV );
                    break;
                case KEY_PAGEDOWN:
                    m_pImpl->DoAction( ACTION_ACTIVATE_NEXT );
                    break;
                case KEY_END:
                    m_pImpl->DoAction( ACTION_ACTIVATE_LAST );
                    break;
                default:
                    bHandled = false;
                    break;
                }
            }
            else if ( rKeyCode.GetModifier() == ( KEY_MOD1 | KEY_SHIFT ) )
            {
                if ( rKeyCode.GetCode() == KEY_E )
                {
                    m_pImpl->DoAction( ACTION_TOGGLE_FOCUS );
                    bHandled = true;
                }
            }
        }

        if ( bHandled )
            return 1;

        return Control::Notify( i_rNotifyEvent );
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck::GetFocus()
    {
        Control::GetFocus();
        m_pImpl->FocusActivePanel();
    }

//........................................................................
} // namespace svt
//........................................................................
