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

#ifndef SD_TOOLPANEL_STANDARDTOOLPANEL_HXX
#define SD_TOOLPANEL_STANDARDTOOLPANEL_HXX

#include "TaskPaneToolPanel.hxx"

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

    //==================================================================================================================
    //= StandardToolPanel
    //==================================================================================================================
    /** an IToolPanel implementation for one of the standard (aka built-in) tool panels
    */
    class StandardToolPanel : public TaskPaneToolPanel
    {
    public:
        StandardToolPanel(
            ToolPanelDeck& i_rPanelDeck,
            ::std::auto_ptr< ControlFactory >& i_rControlFactory,
            const USHORT i_nTitleResId,
            const Image& i_rImage,
            const ULONG i_nHelpId,
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >& i_rPanelResourceId
        );
        ~StandardToolPanel();

        // IToolPanel overridables
        virtual void Activate( ::Window& i_rParentWindow );
        virtual void Deactivate();
        virtual void SetSizePixel( const Size& i_rPanelWindowSize );
        virtual void GrabFocus();
        virtual void Dispose();

        // TaskPaneToolPanel overridables
        virtual const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >& getResourceId() const;

    private:
        bool        impl_ensureControl();
        ::Window*   impl_getPanelWindow() const;

    private:
        ::std::auto_ptr< ControlFactory >   m_pControlFactory;
        ::std::auto_ptr< TreeNode >         m_pControl;
        ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >
                                            m_xPanelResourceId;
    };

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................

#endif // SD_TOOLPANEL_STANDARDTOOLPANEL_HXX
