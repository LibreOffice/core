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
#include <com/sun/star/drawing/framework/XPane.hpp>
#include <com/sun/star/drawing/framework/XResourceId.hpp>
#include <com/sun/star/drawing/framework/XResource.hpp>
#include <com/sun/star/ui/XToolPanel.hpp>
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
        virtual void Activate( ::Window& i_rParentWindow );
        virtual void Deactivate();
        virtual void SetSizePixel( const Size& i_rPanelWindowSize );
        virtual void GrabFocus();
        virtual void Dispose();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                    CreatePanelAccessible(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >& i_rParentAccessible
                    );

        /** locks (aka prevents) the access to the associated XResource object
        */
        void    LockResourceAccess();
        /** unlocks (aka allows) the access to the associated XResource object
        */
        void    UnlockResourceAccess();

    protected:
        // TaskPaneToolPanel overridables
        virtual const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >& getResourceId() const;

    private:
        void    impl_ensurePanel();

    private:
        ::boost::shared_ptr< framework::FrameworkHelper >                                       m_pFrameworkHelper;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >   m_xPanelResourceId;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResource >     m_xResource;
        ::com::sun::star::uno::Reference< ::com::sun::star::ui::XToolPanel >                    m_xToolPanel;
        bool                                                                                    m_bAttemptedPanelCreation;
        sal_uInt32                                                                              m_nResourceAccessLock;
    };

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................

#endif // SD_TOOLPANEL_CUSTOMTOOLPANEL_HXX
