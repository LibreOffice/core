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

#include "ToolPanelDeck.hxx"
#include "taskpane/ToolPanelViewShell.hxx"
#include "taskpane/ControlContainer.hxx"
#include "TaskPaneToolPanel.hxx"

#include <svtools/toolpanel/tablayouter.hxx>

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

    //==================================================================================================================
    //= ToolPanelDeck
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ToolPanelDeck::ToolPanelDeck( Window& i_rParent, ToolPanelViewShell& i_rViewShell )
        :ToolPanelDeck_Base( i_rParent, 0 )
        ,m_rViewShell( i_rViewShell )
    {
        SetTabsLayout( ::svt::TABS_RIGHT, ::svt::TABITEM_IMAGE_ONLY );
    }

    //------------------------------------------------------------------------------------------------------------------
    ToolPanelDeck::~ToolPanelDeck()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDeck::SetTabsLayout( const ::svt::TabAlignment i_eTabAlignment, const ::svt::TabItemContent i_eTabContent )
    {
        ::svt::PDeckLayouter pLayouter( GetLayouter() );
        ::svt::TabDeckLayouter* pTabLayouter = dynamic_cast< ::svt::TabDeckLayouter* >( pLayouter.get() );
        if  (   ( pTabLayouter != NULL )
            &&  ( pTabLayouter->GetTabAlignment() == i_eTabAlignment )
            &&  ( pTabLayouter->GetTabItemContent() == i_eTabContent )
            )
            return;

        if ( pTabLayouter && ( pTabLayouter->GetTabAlignment() == i_eTabAlignment ) )
        {
            // changing only the item content does not require a new layouter instance
            pTabLayouter->SetTabItemContent( i_eTabContent );
            return;
        }

        SetLayouter( new ::svt::TabDeckLayouter( *this, i_eTabAlignment, i_eTabContent ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    size_t ToolPanelDeck::CreateAndInsertPanel( ::std::auto_ptr< ControlFactory >& i_rControlFactory,
        const Image& i_rImage, const USHORT i_nTitleResId, const ULONG i_nHelpId )
    {
        // create panel
        ::svt::PToolPanel pNewPanel( new TaskPaneToolPanel(
            *this, i_rControlFactory,
            i_rImage, i_nTitleResId, i_nHelpId
        ) );
        // insert as new panel
        return InsertPanel( pNewPanel, GetPanelCount() );
    }

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................
