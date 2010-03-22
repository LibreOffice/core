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

#ifndef SD_TASKPANETOOLPANEL_HXX
#define SD_TASKPANETOOLPANEL_HXX

#include "taskpane/TaskPaneControlFactory.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/drawing/framework/XResourceId.hpp>
/** === end UNO includes === **/

#include <svtools/toolpanel/toolpanel.hxx>

#include <vcl/image.hxx>
#include <vcl/smartid.hxx>

#include <memory>

//......................................................................................................................
namespace sd { namespace toolpanel
{
//......................................................................................................................

    class ToolPanelDeck;

    //==================================================================================================================
    //= TaskPaneToolPanel
    //==================================================================================================================
    class TaskPaneToolPanel : public ::svt::ToolPanelBase
    {
    protected:
        TaskPaneToolPanel(
            ToolPanelDeck& i_rPanelDeck,
            const String& i_rPanelName,
            const Image& i_rImage,
            const SmartId& i_rHelpId
        );
        ~TaskPaneToolPanel();

    public:
        // IToolPanel overridables
        virtual ::rtl::OUString GetDisplayName() const;
        virtual Image GetImage() const;
        virtual void Dispose();
        // those are still abstract, and waiting to be overloaded
        virtual void Activate( ::Window& i_rParentWindow ) = 0;
        virtual void Deactivate() = 0;
        virtual void SetSizePixel( const Size& i_rPanelWindowSize ) = 0;
        virtual void GrabFocus() = 0;

        // own overridables
        virtual const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >& getResourceId() const = 0;

    protected:
        bool            isDisposed() const { return m_pPanelDeck == NULL; }
        Window&         getPanelWindowAnchor();

    private:
        ToolPanelDeck*  m_pPanelDeck;
        const Image     m_aPanelImage;
        const String    m_sPanelName;
        const SmartId   m_aHelpId;
    };

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................

#endif // SD_TASKPANETOOLPANEL_HXX
