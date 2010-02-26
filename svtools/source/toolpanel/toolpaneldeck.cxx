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

#include "svtools/toolpanel/toolpaneldeck.hxx"
#include "svtools/toolpanel/tablayouter.hxx"

#include <boost/optional.hpp>

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= ToolPanelDeck_Impl
    //====================================================================
    class ToolPanelDeck_Impl
    {
    public:
        ToolPanelDeck_Impl( ToolPanelDeck& i_rDeck )
            :m_rDeck( i_rDeck )
            ,m_aPanels()
            ,m_pDummyPanel( new DummyPanel )
            ,m_aActivePanel()
            ,m_pLayouter()
            ,m_aPanelPlayground()
        {
            // use a default layouter
            m_pLayouter.reset( new TabDeckLayouter( TABS_RIGHT ) );

            // add as listener to the panels collection - we're interested in panels
            // being added and removed
            // TODO
        }

        const ToolPanelCollection&  GetPanels() const   { return m_aPanels; }
              ToolPanelCollection&  GetPanels()         { return m_aPanels; }

        size_t          GetActivePanel() const;
        void            ActivatePanel( const size_t i_nPanel );

        PDeckLayouter   GetLayouter() const { return m_pLayouter; }
        void            SetLayouter( const PDeckLayouter& i_pNewLayouter );

        /// re-layouts everything
        void    LayoutAll() { ImplDoLayout(); }

    private:
        void    ImplDoLayout();

    private:
        PToolPanel  GetActiveOrDummyPanel_Impl();

    private:
        ToolPanelDeck&      m_rDeck;

        ToolPanelCollection m_aPanels;
        PToolPanel          m_pDummyPanel;
        ::boost::optional< size_t >
                            m_aActivePanel;

        PDeckLayouter       m_pLayouter;
        Rectangle           m_aPanelPlayground;
    };

    //--------------------------------------------------------------------
    PToolPanel ToolPanelDeck_Impl::GetActiveOrDummyPanel_Impl()
    {
        if ( !m_aActivePanel )
            return m_pDummyPanel;
        return m_aPanels.GetPanel( *m_aActivePanel );
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
        OSL_ENSURE( i_nPanel < m_aPanels.GetPanelCount(), "ToolPanelDeck_Impl::ActivatePanel: illegal panel no.!" );
        if ( i_nPanel >= m_aPanels.GetPanelCount() )
            return;

        if ( !!m_aActivePanel && ( *m_aActivePanel == i_nPanel ) )
            return;

        // hide the old panel
        const PToolPanel pOldActive( GetActiveOrDummyPanel_Impl() );
        pOldActive->Hide();

        m_aActivePanel = i_nPanel;

        // position and show the new panel
        const PToolPanel pNewActive( GetActiveOrDummyPanel_Impl() );
        pNewActive->SetPosSizePixel( m_aPanelPlayground );
        pNewActive->Show();

        // tell the layouter - it might need to adjust its visuals to the new situation
        // TODO
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

    //====================================================================
    //= ToolPanelDeck
    //====================================================================
    //--------------------------------------------------------------------
    ToolPanelDeck::ToolPanelDeck( Window& i_rParent, const WinBits i_nStyle )
        :Control( &i_rParent, i_nStyle )
        ,m_pImpl( new ToolPanelDeck_Impl( *this ) )
    {
    }

    //--------------------------------------------------------------------
    ToolPanelDeck::~ToolPanelDeck()
    {
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
    const ToolPanelCollection& ToolPanelDeck::GetPanels() const
    {
        return m_pImpl->GetPanels();
    }

    //--------------------------------------------------------------------
    ToolPanelCollection& ToolPanelDeck::GetPanels()
    {
        return m_pImpl->GetPanels();
    }

    //--------------------------------------------------------------------
    void ToolPanelDeck::Resize()
    {
        Control::Resize();
        m_pImpl->LayoutAll();
    }

//........................................................................
} // namespace svt
//........................................................................
