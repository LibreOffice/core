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
#include "paneldecklisteners.hxx"

#include "svtools/toolpanel/toolpaneldeck.hxx"
#include "svtools/toolpanel/tablayouter.hxx"

#include <tools/diagnose_ex.h>

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
    class ToolPanelDeck_Impl : public IToolPanelDeckListener
    {
    public:
        ToolPanelDeck_Impl( ToolPanelDeck& i_rDeck )
            :m_rDeck( i_rDeck )
            ,m_aPanels()
            ,m_pDummyPanel( new DummyPanel )
            ,m_pLayouter()
            ,m_aPanelPlayground()
        {
            m_aPanels.AddListener( *this );
        }

        ~ToolPanelDeck_Impl()
        {
            m_aPanels.RemoveListener( *this );
            m_aListeners.Dying();
        }

        PDeckLayouter       GetLayouter() const { return m_pLayouter; }
        void                SetLayouter( const PDeckLayouter& i_pNewLayouter );

        // IToolPanelDeck equivalents
        size_t              GetPanelCount() const;
        PToolPanel          GetPanel( const size_t i_nPos ) const;
        ::boost::optional< size_t >
                            GetActivePanel() const;
        void                ActivatePanel( const ::boost::optional< size_t >& i_rPanel );
        size_t              InsertPanel( const PToolPanel& i_pPanel, const size_t i_nPosition );
        PToolPanel          RemovePanel( const size_t i_nPosition );
        void                AddListener( IToolPanelDeckListener& i_rListener );
        void                RemoveListener( IToolPanelDeckListener& i_rListener );

        /// re-layouts everything
        void                LayoutAll() { ImplDoLayout(); }

        void                DoAction( const DeckAction i_eAction );

        void                FocusActivePanel();

    protected:
        // IToolPanelDeckListener
        virtual void        PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition );
        virtual void        PanelRemoved( const size_t i_nPosition );
        virtual void        ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive );
        virtual void        Dying();

    private:
        void                ImplDoLayout();
        PToolPanel          GetActiveOrDummyPanel_Impl();

    private:
        ToolPanelDeck&      m_rDeck;
        ToolPanelCollection m_aPanels;
        PToolPanel          m_pDummyPanel;
        PanelDeckListeners  m_aListeners;

        PDeckLayouter       m_pLayouter;
        Rectangle           m_aPanelPlayground;
    };

    //--------------------------------------------------------------------
    PToolPanel ToolPanelDeck_Impl::GetActiveOrDummyPanel_Impl()
    {
        ::boost::optional< size_t > aActivePanel( m_aPanels.GetActivePanel() );
        if ( !aActivePanel )
            return m_pDummyPanel;
        return m_aPanels.GetPanel( *aActivePanel );
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::SetLayouter( const PDeckLayouter& i_pNewLayouter )
    {
        ENSURE_OR_RETURN_VOID( i_pNewLayouter.get(), "invalid layouter" );

        if ( m_pLayouter.get() )
            m_pLayouter->Destroy();

        m_pLayouter = i_pNewLayouter;

        ImplDoLayout();
    }

    //--------------------------------------------------------------------
    size_t ToolPanelDeck_Impl::GetPanelCount() const
    {
        return m_aPanels.GetPanelCount();
    }

    //--------------------------------------------------------------------
    PToolPanel ToolPanelDeck_Impl::GetPanel( const size_t i_nPos ) const
    {
        return m_aPanels.GetPanel( i_nPos );
    }

    //--------------------------------------------------------------------
    ::boost::optional< size_t > ToolPanelDeck_Impl::GetActivePanel() const
    {
        return m_aPanels.GetActivePanel();
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::ActivatePanel( const ::boost::optional< size_t >& i_rPanel )
    {
        m_aPanels.ActivatePanel( i_rPanel );
    }

    //--------------------------------------------------------------------
    size_t ToolPanelDeck_Impl::InsertPanel( const PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        return m_aPanels.InsertPanel( i_pPanel, i_nPosition );
    }

    //--------------------------------------------------------------------
    PToolPanel ToolPanelDeck_Impl::RemovePanel( const size_t i_nPosition )
    {
        return m_aPanels.RemovePanel( i_nPosition );
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::ImplDoLayout()
    {
        const Rectangle aDeckPlayground( Point(), m_rDeck.GetOutputSizePixel() );

        // let the layouter do the main work
        m_aPanelPlayground = aDeckPlayground;
        OSL_ENSURE( m_pLayouter.get(), "ToolPanelDeck_Impl::ImplDoLayout: no layouter!" );
        if ( m_pLayouter.get() )
        {
            m_aPanelPlayground = m_pLayouter->Layout( aDeckPlayground );
        }

        // and position the active panel
        const PToolPanel pActive( GetActiveOrDummyPanel_Impl() );
        pActive->SetPosSizePixel( m_aPanelPlayground );
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::AddListener( IToolPanelDeckListener& i_rListener )
    {
        m_aListeners.AddListener( i_rListener );
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::RemoveListener( IToolPanelDeckListener& i_rListener )
    {
        m_aListeners.RemoveListener( i_rListener );
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::DoAction( const DeckAction i_eAction )
    {
        const size_t nPanelCount( m_aPanels.GetPanelCount() );
        ::boost::optional< size_t > aActivatePanel;
        ::boost::optional< size_t > aCurrentPanel( GetActivePanel() );

        switch ( i_eAction )
        {
        case ACTION_ACTIVATE_FIRST:
            if ( nPanelCount > 0 )
                aActivatePanel = 0;
            break;
        case ACTION_ACTIVATE_PREV:
            if ( !aCurrentPanel && ( nPanelCount > 0 ) )
                aActivatePanel = nPanelCount - 1;
            else
            if ( !!aCurrentPanel && ( *aCurrentPanel > 0 ) )
                aActivatePanel = *aCurrentPanel - 1;
            break;
        case ACTION_ACTIVATE_NEXT:
            if ( !aCurrentPanel && ( nPanelCount > 0 ) )
                aActivatePanel = 0;
            else
            if ( !!aCurrentPanel && ( *aCurrentPanel < nPanelCount - 1 ) )
                aActivatePanel = *aCurrentPanel + 1;
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
            ActivatePanel( aActivatePanel );
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
        // multiplex to our own listeners
        m_aListeners.PanelInserted( i_pPanel, i_nPosition );
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::PanelRemoved( const size_t i_nPosition )
    {
        // multiplex to our own listeners
        m_aListeners.PanelRemoved( i_nPosition );
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
    {
        // hide the old panel
        if ( !!i_rOldActive )
        {
            const PToolPanel pOldActive( m_aPanels.GetPanel( *i_rOldActive ) );
            pOldActive->Hide();
        }

        // position and show the new panel
        const PToolPanel pNewActive( !i_rNewActive ? m_pDummyPanel : m_aPanels.GetPanel( *i_rNewActive ) );
        pNewActive->SetPosSizePixel( m_aPanelPlayground );
        pNewActive->Show();
        pNewActive->GrabFocus();

        // multiplex to our own listeners
        m_aListeners.ActivePanelChanged( i_rOldActive, i_rNewActive );
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck_Impl::Dying()
    {
        // not interested in. Since the ToolPanelCollection is our member, this just means we ourself
        // are dying, and we already sent this notification in our dtor.
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
        SetLayouter( PDeckLayouter( new TabDeckLayouter( *this, *this, TABS_RIGHT, TABITEM_IMAGE_AND_TEXT ) ) );
    }

    //--------------------------------------------------------------------
    ToolPanelDeck::~ToolPanelDeck()
    {
        GetLayouter()->Destroy();

        Hide();
        for ( size_t i=0; i<GetPanelCount(); ++i )
        {
            PToolPanel pPanel( GetPanel( i ) );
            pPanel->Dispose();
        }
    }

    //--------------------------------------------------------------------
    size_t ToolPanelDeck::GetPanelCount() const
    {
        return m_pImpl->GetPanelCount();
    }

    //--------------------------------------------------------------------
    PToolPanel ToolPanelDeck::GetPanel( const size_t i_nPos ) const
    {
        return m_pImpl->GetPanel( i_nPos );
    }

    //--------------------------------------------------------------------
    ::boost::optional< size_t > ToolPanelDeck::GetActivePanel() const
    {
        return m_pImpl->GetActivePanel();
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck::ActivatePanel( const ::boost::optional< size_t >& i_rPanel )
    {
        m_pImpl->ActivatePanel( i_rPanel );
    }

    //--------------------------------------------------------------------
    size_t ToolPanelDeck::InsertPanel( const PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        return m_pImpl->InsertPanel( i_pPanel, i_nPosition );
    }

    //--------------------------------------------------------------------
    PToolPanel ToolPanelDeck::RemovePanel( const size_t i_nPosition )
    {
        return m_pImpl->RemovePanel( i_nPosition );
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
