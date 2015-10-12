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


#include "dummypanel.hxx"
#include "toolpanelcollection.hxx"
#include "paneldecklisteners.hxx"
#include "toolpaneldeckpeer.hxx"
#include <svtools/toolpanel/toolpaneldeck.hxx>
#include <svtools/toolpanel/tablayouter.hxx>
#include <svtools/toolpanel/drawerlayouter.hxx>

#include <com/sun/star/accessibility/XAccessible.hpp>
#include <com/sun/star/accessibility/AccessibleRole.hpp>

#include <tools/diagnose_ex.h>
#include <vcl/vclptr.hxx>

#include <boost/optional.hpp>


namespace svt
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::accessibility::XAccessible;
    using ::com::sun::star::awt::XWindowPeer;
    using ::com::sun::star::uno::UNO_SET_THROW;

    namespace AccessibleRole = ::com::sun::star::accessibility::AccessibleRole;

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


    //= ToolPanelDeck_Impl

    class ToolPanelDeck_Impl : public IToolPanelDeckListener
    {
    public:
        explicit ToolPanelDeck_Impl( ToolPanelDeck& i_rDeck )
            :m_rDeck( i_rDeck )
            ,m_aPanelAnchor( VclPtr<vcl::Window>::Create(&i_rDeck, WB_DIALOGCONTROL | WB_CHILDDLGCTRL) )
            ,m_aPanels()
            ,m_pDummyPanel( new DummyPanel )
            ,m_pLayouter()
            ,m_bInDtor( false )
        {
            m_aPanels.AddListener( *this );
            m_aPanelAnchor->Show();
            m_aPanelAnchor->SetAccessibleRole( AccessibleRole::PANEL );
        }

        virtual ~ToolPanelDeck_Impl()
        {
            m_bInDtor = true;
        }

        PDeckLayouter       GetLayouter() const { return m_pLayouter; }
        void                SetLayouter( const PDeckLayouter& i_pNewLayouter );

        vcl::Window&             GetPanelWindowAnchor()       { return *m_aPanelAnchor.get(); }

        bool                IsDead() const { return m_bInDtor; }

        /// notifies our listeners that we're going to die. Only to be called from with our anti-impl's destructor
        void                NotifyDying()
        {
            m_aPanels.RemoveListener( *this );
            m_aListeners.Dying();
        }

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

        bool                FocusActivePanel();

    protected:
        // IToolPanelDeckListener
        virtual void        PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition ) override;
        virtual void        PanelRemoved( const size_t i_nPosition ) override;
        virtual void        ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive ) override;
        virtual void        LayouterChanged( const PDeckLayouter& i_rNewLayouter ) override;
        virtual void        Dying() override;

    private:
        void                ImplDoLayout();
        PToolPanel          GetActiveOrDummyPanel_Impl();

    private:
        ToolPanelDeck&      m_rDeck;
        VclPtr<vcl::Window> m_aPanelAnchor;
        ToolPanelCollection m_aPanels;
        PToolPanel          m_pDummyPanel;
        PanelDeckListeners  m_aListeners;
        PDeckLayouter       m_pLayouter;
        bool                m_bInDtor;
    };


    PToolPanel ToolPanelDeck_Impl::GetActiveOrDummyPanel_Impl()
    {
        ::boost::optional< size_t > aActivePanel( m_aPanels.GetActivePanel() );
        if ( !aActivePanel )
            return m_pDummyPanel;
        return m_aPanels.GetPanel( *aActivePanel );
    }


    void ToolPanelDeck_Impl::SetLayouter( const PDeckLayouter& i_pNewLayouter )
    {
        ENSURE_OR_RETURN_VOID( i_pNewLayouter.get(), "invalid layouter" );

        if ( m_pLayouter.get() )
            m_pLayouter->Destroy();

        m_pLayouter = i_pNewLayouter;

        ImplDoLayout();

        m_aListeners.LayouterChanged( m_pLayouter );
    }


    size_t ToolPanelDeck_Impl::GetPanelCount() const
    {
        return m_aPanels.GetPanelCount();
    }


    PToolPanel ToolPanelDeck_Impl::GetPanel( const size_t i_nPos ) const
    {
        return m_aPanels.GetPanel( i_nPos );
    }


    ::boost::optional< size_t > ToolPanelDeck_Impl::GetActivePanel() const
    {
        return m_aPanels.GetActivePanel();
    }


    void ToolPanelDeck_Impl::ActivatePanel( const ::boost::optional< size_t >& i_rPanel )
    {
        m_aPanels.ActivatePanel( i_rPanel );
    }


    size_t ToolPanelDeck_Impl::InsertPanel( const PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        return m_aPanels.InsertPanel( i_pPanel, i_nPosition );
    }


    PToolPanel ToolPanelDeck_Impl::RemovePanel( const size_t i_nPosition )
    {
        return m_aPanels.RemovePanel( i_nPosition );
    }


    void ToolPanelDeck_Impl::ImplDoLayout()
    {
        const Rectangle aDeckPlayground( Point(), m_rDeck.GetOutputSizePixel() );

        // ask the layouter what is left for our panel, and position the panel container window appropriately
        Rectangle aPlaygroundArea( aDeckPlayground );
        OSL_ENSURE( m_pLayouter.get(), "ToolPanelDeck_Impl::ImplDoLayout: no layouter!" );
        if ( m_pLayouter.get() )
        {
            aPlaygroundArea = m_pLayouter->Layout( aDeckPlayground );
        }
        m_aPanelAnchor->SetPosSizePixel( aPlaygroundArea.TopLeft(), aPlaygroundArea.GetSize() );

        // position the active panel
        const PToolPanel pActive( GetActiveOrDummyPanel_Impl() );
        pActive->SetSizePixel( m_aPanelAnchor->GetOutputSizePixel() );
    }


    void ToolPanelDeck_Impl::AddListener( IToolPanelDeckListener& i_rListener )
    {
        m_aListeners.AddListener( i_rListener );
    }


    void ToolPanelDeck_Impl::RemoveListener( IToolPanelDeckListener& i_rListener )
    {
        m_aListeners.RemoveListener( i_rListener );
    }


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
            else if ( !!aCurrentPanel && ( *aCurrentPanel > 0 ) )
                aActivatePanel = *aCurrentPanel - 1;
            break;
        case ACTION_ACTIVATE_NEXT:
            if ( !aCurrentPanel && ( nPanelCount > 0 ) )
                aActivatePanel = 0;
            else if ( !!aCurrentPanel && ( *aCurrentPanel < nPanelCount - 1 ) )
                aActivatePanel = *aCurrentPanel + 1;
            break;
        case ACTION_ACTIVATE_LAST:
            if ( nPanelCount > 0 )
                aActivatePanel = nPanelCount - 1;
            break;
        case ACTION_TOGGLE_FOCUS:
            {
                PToolPanel pActivePanel( GetActiveOrDummyPanel_Impl() );
                if ( !m_aPanelAnchor->HasChildPathFocus() )
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


    bool ToolPanelDeck_Impl::FocusActivePanel()
    {
        ::boost::optional< size_t > aActivePanel( m_aPanels.GetActivePanel() );
        if ( !aActivePanel )
            return false;

        PToolPanel pActivePanel( m_aPanels.GetPanel( *aActivePanel ) );
        pActivePanel->GrabFocus();
        return true;
    }


    void ToolPanelDeck_Impl::PanelInserted( const PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        // multiplex to our own listeners
        m_aListeners.PanelInserted( i_pPanel, i_nPosition );
    }


    void ToolPanelDeck_Impl::PanelRemoved( const size_t i_nPosition )
    {
        // multiplex to our own listeners
        m_aListeners.PanelRemoved( i_nPosition );
    }


    void ToolPanelDeck_Impl::ActivePanelChanged( const ::boost::optional< size_t >& i_rOldActive, const ::boost::optional< size_t >& i_rNewActive )
    {
        // hide the old panel
        if ( !!i_rOldActive )
        {
            const PToolPanel pOldActive( m_aPanels.GetPanel( *i_rOldActive ) );
            pOldActive->Deactivate();
        }

        // position and show the new panel
        const PToolPanel pNewActive( !i_rNewActive ? m_pDummyPanel : m_aPanels.GetPanel( *i_rNewActive ) );
        pNewActive->Activate( *m_aPanelAnchor.get() );
        pNewActive->GrabFocus();

        // resize the panel (cannot guarantee it has ever been resized before
        pNewActive->SetSizePixel( m_aPanelAnchor->GetOutputSizePixel() );

        // multiplex to our own listeners
        m_aListeners.ActivePanelChanged( i_rOldActive, i_rNewActive );
    }


    void ToolPanelDeck_Impl::LayouterChanged( const PDeckLayouter& i_rNewLayouter )
    {
        // not interested in
        (void)i_rNewLayouter;
    }


    void ToolPanelDeck_Impl::Dying()
    {
        // not interested in. Since the ToolPanelCollection is our member, this just means we ourself
        // are dying, and we already sent this notification in our dtor.
    }


    //= ToolPanelDeck


    ToolPanelDeck::ToolPanelDeck( vcl::Window& i_rParent, const WinBits i_nStyle )
        :Control( &i_rParent, i_nStyle )
        ,m_pImpl( new ToolPanelDeck_Impl( *this ) )
    {
        // use a default layouter
//        SetLayouter( PDeckLayouter( new TabDeckLayouter( *this, *this, TABS_RIGHT, TABITEM_IMAGE_AND_TEXT ) ) );
        SetLayouter( PDeckLayouter( new DrawerDeckLayouter( *this, *this ) ) );
    }


    ToolPanelDeck::~ToolPanelDeck()
    {
        disposeOnce();
    }

    void ToolPanelDeck::dispose()
    {
        m_pImpl->NotifyDying();
        GetLayouter()->Destroy();

        Hide();
        for ( size_t i=0; i<GetPanelCount(); ++i )
        {
            PToolPanel pPanel( GetPanel( i ) );
            pPanel->Dispose();
        }
        Control::dispose();
    }


    size_t ToolPanelDeck::GetPanelCount() const
    {
        return m_pImpl->GetPanelCount();
    }


    PToolPanel ToolPanelDeck::GetPanel( const size_t i_nPos ) const
    {
        return m_pImpl->GetPanel( i_nPos );
    }


    ::boost::optional< size_t > ToolPanelDeck::GetActivePanel() const
    {
        return m_pImpl->GetActivePanel();
    }


    void ToolPanelDeck::ActivatePanel( const ::boost::optional< size_t >& i_rPanel )
    {
        m_pImpl->ActivatePanel( i_rPanel );
    }


    size_t ToolPanelDeck::InsertPanel( const PToolPanel& i_pPanel, const size_t i_nPosition )
    {
        return m_pImpl->InsertPanel( i_pPanel, i_nPosition );
    }


    PToolPanel ToolPanelDeck::RemovePanel( const size_t i_nPosition )
    {
        return m_pImpl->RemovePanel( i_nPosition );
    }


    PDeckLayouter ToolPanelDeck::GetLayouter() const
    {
        return m_pImpl->GetLayouter();
    }


    void ToolPanelDeck::SetLayouter( const PDeckLayouter& i_pNewLayouter )
    {
        return m_pImpl->SetLayouter( i_pNewLayouter );
    }


    void ToolPanelDeck::AddListener( IToolPanelDeckListener& i_rListener )
    {
        m_pImpl->AddListener( i_rListener );
    }


    void ToolPanelDeck::RemoveListener( IToolPanelDeckListener& i_rListener )
    {
        m_pImpl->RemoveListener( i_rListener );
    }


    vcl::Window& ToolPanelDeck::GetPanelWindowAnchor()
    {
        return m_pImpl->GetPanelWindowAnchor();
    }


    const vcl::Window& ToolPanelDeck::GetPanelWindowAnchor() const
    {
        return m_pImpl->GetPanelWindowAnchor();
    }


    void ToolPanelDeck::Resize()
    {
        Control::Resize();
        m_pImpl->LayoutAll();
    }


    bool ToolPanelDeck::Notify( NotifyEvent& i_rNotifyEvent )
    {
        bool bHandled = false;
        if ( i_rNotifyEvent.GetType() == MouseNotifyEvent::KEYINPUT )
        {
            const KeyEvent* pEvent = i_rNotifyEvent.GetKeyEvent();
            const vcl::KeyCode& rKeyCode = pEvent->GetKeyCode();
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
            return true;

        return Control::Notify( i_rNotifyEvent );
    }


    void ToolPanelDeck::GetFocus()
    {
        Control::GetFocus();
        if ( m_pImpl->IsDead() )
            return;
        if ( !m_pImpl->FocusActivePanel() )
        {
            PDeckLayouter pLayouter( GetLayouter() );
            ENSURE_OR_RETURN_VOID( pLayouter.get(), "ToolPanelDeck::GetFocus: no layouter?!" );
            pLayouter->SetFocusToPanelSelector();
        }
    }


    Reference< XWindowPeer > ToolPanelDeck::GetComponentInterface( bool i_bCreate )
    {
        Reference< XWindowPeer > xWindowPeer( Control::GetComponentInterface( false ) );
        if ( !xWindowPeer.is() && i_bCreate )
        {
            xWindowPeer.set( new ToolPanelDeckPeer( *this ) );
            SetComponentInterface( xWindowPeer );
        }
        return xWindowPeer;
    }


} // namespace svt


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
