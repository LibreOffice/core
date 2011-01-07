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

#ifndef SFX_TITLEDOCKWIN_HXX
#define SFX_TITLEDOCKWIN_HXX

#include "sfx2/dllapi.h"
#include "sfx2/dockwin.hxx"

#include <vcl/toolbox.hxx>
#include <tools/svborder.hxx>

//......................................................................................................................
namespace sfx2
{
//......................................................................................................................

    //==================================================================================================================
    //= TitledDockingWindow
    //==================================================================================================================
    class SFX2_DLLPUBLIC TitledDockingWindow : public SfxDockingWindow
    {
    public:
        TitledDockingWindow(
            SfxBindings* i_pBindings, SfxChildWindow* i_pChildWindow,
            Window* i_pParent, WinBits i_nStyle = 0
        );

        TitledDockingWindow( SfxBindings* i_pBindings, SfxChildWindow* i_pChildWindow,
            Window* i_pParent, const ResId& i_rResId
        );

        virtual ~TitledDockingWindow();

        /** sets a title to be displayed in the docking window
        */
        void        SetTitle( const String& i_rTitle );
        /** returns the current title displayed in the docking window

            Note that if you never called SetTitle before, then this method will not return an empty string,
            but the window text (Window::GetText), since this is what is displayed as title then.
        */
        String      GetTitle() const;

        /** adds a drop down item to the toolbox. Usually, this is used to add some kind of menu to the toolbox.

            @param i_rItemText
                the text to display for the item
            @param i_nHelpId
                the help ID for the new toolbox item
            @param i_rCallback
                the callback to invoke when the drop item has been clicked
            @return
                the ID of the newly created toolbox item
        */
        sal_uInt16  AddDropDownToolBoxItem( const String& i_rItemText, const rtl::OString& i_nHelpId, const Link& i_rCallback )
        {
            return impl_addDropDownToolBoxItem( i_rItemText, i_nHelpId, i_rCallback );
        }

        void        SetEndDockingHdl( const Link& i_rEndDockingHdl ) { m_aEndDockingHdl = i_rEndDockingHdl; }
        const Link& GetEndDockingHdl() const { return m_aEndDockingHdl; }

        /** resets the toolbox. Upon return, the only item in the toolbox is the closer.
        */
        void    ResetToolBox()
        {
            impl_resetToolBox();
        }

        /** returns the content window, which is to be used as parent window for any content to be displayed
            in the docking window.
        */
        ::Window&       GetContentWindow()          { return m_aContentWindow; }
        const ::Window& GetContentWindow() const    { return m_aContentWindow; }

        ToolBox&        GetToolBox()        { return m_aToolbox; }
        const ToolBox&  GetToolBox() const  { return m_aToolbox; }

        /** Return the border that is painted around the inner window as
            decoration.
        */
        SvBorder        GetDecorationBorder (void) const  { return m_aBorder; }

    protected:
        // Window overridables
        virtual void Paint( const Rectangle& i_rArea );
        virtual void Resize();
        virtual void StateChanged( StateChangedType i_nType );
        virtual void DataChanged( const DataChangedEvent& i_rDataChangedEvent );
        virtual void SetText( const String& i_rText );

        // DockingWindow overridables
        void EndDocking( const Rectangle& rRect, sal_Bool bFloatMode );

        // own overridables
        virtual void onLayoutDone();

    protected:
        /** internal version of ResetToolBox
        */
        void    impl_resetToolBox();

        /** internal version of AddDropDownToolBoxItem
        */
        sal_uInt16  impl_addDropDownToolBoxItem( const String& i_rItemText, const rtl::OString& i_nHelpId, const Link& i_rCallback );

        /** returns the current title.

            If no title has been set via SetTitle, then the window text (Window::GetText) is returned.
        */
        String  impl_getTitle() const;

    private:
        DECL_LINK( OnToolboxItemSelected, ToolBox* );

        void    impl_construct();
        void    impl_layout();
        void    impl_scheduleLayout();

    private:
        String              m_sTitle;
        ToolBox             m_aToolbox;
        Window              m_aContentWindow;

        Link                m_aEndDockingHdl;

        /** The border that is painted arround the inner window.  The bevel
            shadow lines are part of the border, so where the border is 0 no
            such line is painted.
        */
        SvBorder            m_aBorder;

        /** Remember that a layout is pending, i.e. Resize() has been called
            since the last Paint().
        */
        bool                m_bLayoutPending;

        /** Height of the title bar.  Calculated in impl_layout().
        */
        int                 m_nTitleBarHeight;

    };

//......................................................................................................................
} // namespace sfx2
//......................................................................................................................

#endif // SFX_TITLEDOCKWIN_HXX
