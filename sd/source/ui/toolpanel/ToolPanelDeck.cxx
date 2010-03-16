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
#include "ToolPanelDrawer.hxx"
#include "taskpane/ControlContainer.hxx"
#include "framework/FrameworkHelper.hxx"
#include "TaskPaneToolPanel.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/drawing/framework/ResourceActivationMode.hpp>
/** === end UNO includes === **/

#include <svtools/toolpanel/tablayouter.hxx>
#include <tools/diagnose_ex.h>

//......................................................................................................................
namespace sd { namespace toolpanel
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
    using ::com::sun::star::drawing::framework::XResourceId;
    using ::com::sun::star::drawing::framework::ResourceActivationMode_REPLACE;
    /** === end UNO using === **/
    using ::sd::framework::FrameworkHelper;

    //==================================================================================================================
    //= ToolPanelDeck
    //==================================================================================================================
    //------------------------------------------------------------------------------------------------------------------
    ToolPanelDeck::ToolPanelDeck( Window& i_rParent, ToolPanelViewShell& i_rViewShell )
        :ToolPanelDeck_Base( i_rParent, 0 )
        ,m_rViewShell( i_rViewShell )
    {
        SetDrawersLayout();
    }

    //------------------------------------------------------------------------------------------------------------------
    ToolPanelDeck::~ToolPanelDeck()
    {
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDeck::SetDrawersLayout()
    {
        const ::svt::PDeckLayouter pLayouter( GetLayouter() );
        const ToolPanelDrawer* pDrawerLayouter = dynamic_cast< const ToolPanelDrawer* >( pLayouter.get() );
        if ( pDrawerLayouter != NULL )
            // already have the proper layout
            return;
        SetLayouter( new ToolPanelDrawer( *this, *this ) );
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
            // already have the requested layout
            return;

        if ( pTabLayouter && ( pTabLayouter->GetTabAlignment() == i_eTabAlignment ) )
        {
            // changing only the item content does not require a new layouter instance
            pTabLayouter->SetTabItemContent( i_eTabContent );
            return;
        }

        SetLayouter( new ::svt::TabDeckLayouter( *this, *this, i_eTabAlignment, i_eTabContent ) );
    }

    //------------------------------------------------------------------------------------------------------------------
    size_t ToolPanelDeck::CreateAndInsertPanel( ::std::auto_ptr< ControlFactory >& i_rControlFactory,
        const Image& i_rImage, const USHORT i_nTitleResId, const ULONG i_nHelpId, const Reference< XResourceId >& i_rPanelResourceId )
    {
        // create panel
        ::svt::PToolPanel pNewPanel( new TaskPaneToolPanel(
            *this, i_rControlFactory,
            i_rImage, i_nTitleResId, i_nHelpId,
            i_rPanelResourceId
        ) );
        // insert as new panel
        return InsertPanel( pNewPanel, GetPanelCount() );
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDeck::ActivatePanelDirectly( const ::boost::optional< size_t >& i_rPanel )
    {
        ToolPanelDeck_Base::ActivatePanel( i_rPanel );
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDeck::ActivatePanelResource( const size_t i_nPanel )
    {
        // determine resource ID for the given panel
        ::svt::PToolPanel pPanel( GetPanel( i_nPanel ) );
        const TaskPaneToolPanel* pTaskPanePanel( dynamic_cast< const TaskPaneToolPanel* >( pPanel.get() ) );
        ENSURE_OR_RETURN_VOID( pTaskPanePanel, "did not find the right panel/type at the given position" );
        const Reference< XResourceId > xPanelId( pTaskPanePanel->getResourceId() );

        // delegate the request to the configuration controller
        ::boost::shared_ptr< FrameworkHelper > pFrameworkHelper( FrameworkHelper::Instance( m_rViewShell.GetViewShellBase() ) );
        pFrameworkHelper->GetConfigurationController()->requestResourceActivation(
            xPanelId, ResourceActivationMode_REPLACE );
    }

    //------------------------------------------------------------------------------------------------------------------
    void ToolPanelDeck::ActivatePanel( const ::boost::optional< size_t >& i_rPanel )
    {
        if ( !i_rPanel )
        {
            // this is a de-activate request. Quite improbable that this really happens: We're within the overloaded
            // version of IToolPanelDeck::ActivatePanel. The only instance which has access to this IToolPanel
            // interface is the panel layouter, which is not expected to call us with a NULL panel position.
            // All other instances should now have access to this method, as it is protected in this class here.
            OSL_ENSURE( false, "ToolPanelDeck::ActivatePanel: is this legitimate?" );
            // well, handle it nonetheless.
            ActivatePanelDirectly( i_rPanel );
        }
        else
            ActivatePanelResource( *i_rPanel );
    }

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................
