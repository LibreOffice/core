/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SFX2_TITLEDOCKWIN_HXX
#define INCLUDED_SFX2_TITLEDOCKWIN_HXX

#include <sfx2/dllapi.h>
#include <sfx2/dockwin.hxx>

#include <vcl/toolbox.hxx>
#include <vcl/vclptr.hxx>
#include <tools/svborder.hxx>

namespace sfx2
{
    class SFX2_DLLPUBLIC TitledDockingWindow : public SfxDockingWindow
    {
    public:
        TitledDockingWindow(
            SfxBindings* i_pBindings, SfxChildWindow* i_pChildWindow,
            vcl::Window* i_pParent, WinBits i_nStyle = 0
        );

        virtual ~TitledDockingWindow();
        virtual void dispose() SAL_OVERRIDE;

        /** sets a title to be displayed in the docking window
        */
        void        SetTitle( const OUString& i_rTitle );

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
        sal_uInt16  AddDropDownToolBoxItem( const OUString& i_rItemText, const OString& i_nHelpId, const Link<ToolBox *, void>& i_rCallback )
        {
            return impl_addDropDownToolBoxItem( i_rItemText, i_nHelpId, i_rCallback );
        }

        void        SetEndDockingHdl( const Link<>& i_rEndDockingHdl ) { m_aEndDockingHdl = i_rEndDockingHdl; }
        const Link<>& GetEndDockingHdl() const { return m_aEndDockingHdl; }

        /** resets the toolbox. Upon return, the only item in the toolbox is the closer.
        */
        void    ResetToolBox()
        {
            impl_resetToolBox();
        }

        /** returns the content window, which is to be used as parent window for any content to be displayed
            in the docking window.
        */
        vcl::Window&       GetContentWindow()          { return *m_aContentWindow.get(); }
        const vcl::Window& GetContentWindow() const    { return *m_aContentWindow.get(); }

        ToolBox&        GetToolBox()        { return *m_aToolbox.get(); }
        const ToolBox&  GetToolBox() const  { return *m_aToolbox.get(); }

        /** Return the border that is painted around the inner window as
            decoration.
        */
        SvBorder        GetDecorationBorder() const  { return m_aBorder; }

    protected:
        // Window overridables
        virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& i_rArea) SAL_OVERRIDE;
        virtual void Resize() SAL_OVERRIDE;
        virtual void StateChanged( StateChangedType i_nType ) SAL_OVERRIDE;
        virtual void DataChanged( const DataChangedEvent& i_rDataChangedEvent ) SAL_OVERRIDE;
        virtual void SetText( const OUString& i_rText ) SAL_OVERRIDE;

        // DockingWindow overridables
        void EndDocking(const Rectangle& rRect, bool bFloatMode) SAL_OVERRIDE;

        // own overridables
        virtual void onLayoutDone();

        virtual void ApplySettings(vcl::RenderContext& rRenderContext) SAL_OVERRIDE;
    protected:
        /** internal version of ResetToolBox
        */
        void    impl_resetToolBox();

        /** internal version of AddDropDownToolBoxItem
        */
        sal_uInt16  impl_addDropDownToolBoxItem( const OUString& i_rItemText, const OString& i_nHelpId, const Link<ToolBox *, void>& i_rCallback );

        /** returns the current title.

            If no title has been set via SetTitle, then the window text (Window::GetText) is returned.
        */
        OUString  impl_getTitle() const;

    private:
        DECL_LINK_TYPED( OnToolboxItemSelected, ToolBox*, void );

        void    impl_construct();
        void    impl_layout();
        void    impl_scheduleLayout();

    private:
        OUString            m_sTitle;
        VclPtr<ToolBox>     m_aToolbox;
        VclPtr<Window>      m_aContentWindow;

        Link<>              m_aEndDockingHdl;

        /** The border that is painted around the inner window.  The bevel
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


} // namespace sfx2


#endif // INCLUDED_SFX2_TITLEDOCKWIN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
