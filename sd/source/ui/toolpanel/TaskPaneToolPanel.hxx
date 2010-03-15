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
    public:
        TaskPaneToolPanel(
            ToolPanelDeck& i_rPanelDeck,
            ::std::auto_ptr< ControlFactory >& i_rControlFactory,
            const Image& i_rImage,
            const USHORT i_nTitleResId,
            const ULONG i_nHelpId/*,
            const ::com::sun::star::uno::Reference< ::com::sun::star::drawing::framework::XResourceId >& i_rPanelContent*/
        );
        ~TaskPaneToolPanel();

        // IToolPanel overridables
        virtual ::rtl::OUString GetDisplayName() const;
        virtual Image GetImage() const;
        virtual void Show();
        virtual void Hide();
        virtual void SetPosSizePixel( const Rectangle& i_rPanelPlayground );
        virtual void GrabFocus();
        virtual bool HasFocus() const;
        virtual void Dispose();

    private:
        bool    impl_ensureControl();

    private:
        ToolPanelDeck*                      m_pPanelDeck;
        ::std::auto_ptr< ControlFactory >   m_pControlFactory;
        ::std::auto_ptr< TreeNode >         m_pControl;
        const Image                         m_aImage;
        const String                        m_sTitle;
        const SmartId                       m_aHelpId;
    };

//......................................................................................................................
} } // namespace sd::toolpanel
//......................................................................................................................

#endif // SD_TASKPANETOOLPANEL_HXX
