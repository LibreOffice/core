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

#pragma once

#include <sfx2/dockwin.hxx>
#include <tools/svborder.hxx>
#include <vcl/vclptr.hxx>

class ToolBox;

namespace sd
{
    class TitledDockingWindow final : public SfxDockingWindow
    {
    public:
        /** Create a new docking window.
            @param pBindings
                Used, among others, to determine the ViewShellBase and
                PaneManager that manage the new docking window.
            @param pChildWindow
                This child window is the logical container for the new docking
                window.
            @param pParent
                The parent window of the new docking window.
            @param rsTitle
                the initial title
        */
        TitledDockingWindow(
            SfxBindings* i_pBindings,
            SfxChildWindow* i_pChildWindow,
            vcl::Window* i_pParent,
            const OUString& rsTitle
        );

        virtual ~TitledDockingWindow() override;
        virtual void dispose() override;

        /** returns the content window, which is to be used as parent window for any content to be displayed
            in the docking window.
        */
        vcl::Window&       GetContentWindow()          { return *m_aContentWindow; }
        const vcl::Window& GetContentWindow() const    { return *m_aContentWindow; }

        /** Return the border that is painted around the inner window as
            decoration.
        */
        const SvBorder&  GetDecorationBorder() const  { return m_aBorder; }

        /** When docked the given range is passed to the parent SplitWindow.
        */
        void SetValidSizeRange (const Range& rValidSizeRange);

        enum Orientation { HorizontalOrientation, VerticalOrientation, UnknownOrientation };
        /** When the TitledDockingWindow is docked and managed by a split window
            it can derive its orientation from the orientation of the split
            window and return either HorizontalOrientation or
            VerticalOrientation.
            Otherwise UnknownOrientation is returned.
        */
        Orientation GetOrientation() const;

    private:
        // Window overridables
        virtual void Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& i_rArea) override;
        virtual void Resize() override;
        virtual void StateChanged( StateChangedType i_nType ) override;
        virtual void DataChanged( const DataChangedEvent& i_rDataChangedEvent ) override;
        virtual void SetText( const OUString& i_rText ) override;
        virtual void MouseButtonDown (const MouseEvent& rEvent) override;
        virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;
        /** internal version of ResetToolBox
        */
        void    impl_resetToolBox();

        DECL_LINK(OnToolboxItemSelected, ToolBox*, void);

        void    impl_layout();

        OUString            m_sTitle;
        VclPtr<ToolBox>     m_aToolbox;
        VclPtr<Window>      m_aContentWindow;

        /** The border that is painted around the inner window.  The bevel
            shadow lines are part of the border, so where the border is 0 no
            such line is painted.
        */
        SvBorder            m_aBorder;

        /** Height of the title bar.  Calculated in impl_layout().
        */
        int                 m_nTitleBarHeight;

    };

} // namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
