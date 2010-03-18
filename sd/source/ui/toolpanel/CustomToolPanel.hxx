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

#ifndef SD_TOOLPANEL_CUSTOMTOOLPANEL_HXX
#define SD_TOOLPANEL_CUSTOMTOOLPANEL_HXX

#include "TaskPaneToolPanel.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/drawing/framework/XPane2.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
/** === end UNO includes === **/

#include <boost/shared_ptr.hpp>

namespace utl
{
    class OConfigurationNode;
}

namespace sd { namespace framework
{
    class FrameworkHelper;
} }

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

    class ToolPanelDeck;

    //==================================================================================================================
    //= CustomToolPanel
    //==================================================================================================================
    /** is a ::svt::IToolPanel implementation for custom tool panels, i.e. those defined in the configuration, and
        implemented by external components.
    */
    class CustomToolPanel : public TaskPaneToolPanel
    {
    public:
        CustomToolPanel(
            ToolPanelDeck& i_rPanelDeck,
            const ::utl::OConfigurationNode& i_rPanelConfig,
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >& i_rPaneResourceId,
            const ::boost::shared_ptr< framework::FrameworkHelper >& i_pFrameworkHelper
        );
        ~CustomToolPanel();

        // IToolPanel overridables
        virtual void Dispose();

    protected:
        // TaskPaneToolPanel overridables
        virtual const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >& getResourceId() const;
        virtual ::Window* getPanelWindow() const;

    private:
        bool    impl_ensurePanel() const;

    private:
        ::boost::shared_ptr< framework::FrameworkHelper >                                       m_pFrameworkHelper;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >   m_xPanelResourceId;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XPane2 >        m_xPanel;
        bool                                                                                    m_bAttemptedPanelCreation;
    };

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................

#endif // SD_TOOLPANEL_CUSTOMTOOLPANEL_HXX
