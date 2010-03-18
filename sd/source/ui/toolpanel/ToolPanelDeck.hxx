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

#ifndef SD_TOOLPANELDECK_HXX
#define SD_TOOLPANELDECK_HXX

#include "taskpane/TaskPaneTreeNode.hxx"
#include "taskpane/TaskPaneControlFactory.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/drawing/framework/XResourceId.hpp>
/** === end UNO includes === **/

#include <svtools/toolpanel/toolpaneldeck.hxx>
#include <svtools/toolpanel/tabalignment.hxx>
#include <svtools/toolpanel/tabitemcontent.hxx>

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

    class ToolPanelViewShell;

    //==================================================================================================================
    //= ToolPanelDeck
    //==================================================================================================================
    typedef ::svt::ToolPanelDeck    ToolPanelDeck_Base;
    class ToolPanelDeck : public ToolPanelDeck_Base
    {
    public:
        ToolPanelDeck(
            ::Window& i_rParent,
            ToolPanelViewShell& i_rViewShell
        );
        ~ToolPanelDeck();

        /** activates a layout where the active panel is selected via tabs at the right/left/top/bottom (depending on
            the given TabAlignment value).
        */
        void    SetTabsLayout( const ::svt::TabAlignment i_eTabAlignment, const ::svt::TabItemContent i_eTabContent );

        /** activates the "classical" layout with the panels being represneted by stacked drawers
        */
        void    SetDrawersLayout();

        /** directly activates the given panel, without re-routing the activation request through the drawing
            framework's configuration controller.
        */
        void    ActivatePanelDirectly( const ::boost::optional< size_t >& i_rPanel );

        /** activates the given panel by delegating the acvtivation request to the drawing framework's configuration
            controller.
        */
        void    ActivatePanelResource( const size_t i_nPanel );

    protected:
        // IToolPanelDeck
        /** this method, which is part of the callback used by the PanelSelector, does not forward the request to
            the base class. Instead, it forwards it to the ToolPanelViewShell, which transforms it into a request
            to the drawing framework's configuration controller, which in turn will end up in our public
            ActivatePanelDirectly method.
        */
        virtual void        ActivatePanel( const ::boost::optional< size_t >& i_rPanel );

    private:
        ToolPanelViewShell& m_rViewShell;
    };

//......................................................................................................................
} } // sd::toolpanel
//......................................................................................................................

#endif // SD_TOOLPANELDECK_HXX
