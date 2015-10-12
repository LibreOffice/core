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

#ifndef INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_TOOLPANELDRAWER_HXX
#define INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_TOOLPANELDRAWER_HXX

#include <vcl/window.hxx>
#include <vcl/virdev.hxx>
#include <vcl/vclptr.hxx>

namespace svt
{
    class ToolPanelDrawer;

    /** serves a single purpose - let ZoomText read the drawers ...

        Strange enough, ZoomText does not read the drawers when they get the focus (in none of the combinations
        of AccessibleRoles I tried), except when it does have an AccessibleChild with the role LABEL. To "inject"
        such a child into the A11Y hierarchy, we use this window here.

        (We could also inject the A11Y component on the A11Y level only, but this would mean additional code. With
        this approach here, VCL/toolkit will take care of creating and maintaining the A11Y component for us.)
    */
    class DrawerVisualization : public vcl::Window
    {
    public:
        explicit DrawerVisualization(ToolPanelDrawer& i_rParent);

    protected:
        // Window overridables
        virtual void Paint(vcl::RenderContext& rRenderContext, const Rectangle& i_rBoundingBox) override;

    private:
        ToolPanelDrawer& m_rDrawer;
    };

    class ToolPanelDrawer : public vcl::Window
    {
    public:
        ToolPanelDrawer(vcl::Window& i_rParent, const OUString& i_rTitle);
        virtual ~ToolPanelDrawer();
        virtual void dispose() override;

        long GetPreferredHeightPixel() const;
        void SetExpanded(const bool i_bExpanded);
        bool IsExpanded() const
        {
                return m_bExpanded;
        }

        void Paint(vcl::RenderContext& rRenderContext);

    protected:
        // Window overridables
        virtual void GetFocus() override;
        virtual void LoseFocus() override;
        virtual void Resize() override;
        virtual void DataChanged( const DataChangedEvent& i_rEvent ) override;
        virtual void MouseButtonDown( const MouseEvent& i_rMouseEvent ) override;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >
                     GetComponentInterface( bool i_bCreate ) override;

        virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;

    private:
        Rectangle   impl_calcTextBoundingBox() const;
        Rectangle   impl_calcTitleBarBox( const Rectangle& i_rTextBox ) const;
        void        impl_paintBackground( const Rectangle& i_rTitleBarBox );
        DrawTextFlags impl_getTextStyle() const;
        void        impl_paintFocusIndicator( const Rectangle& i_rTextBox );
        Rectangle   impl_paintExpansionIndicator( const Rectangle& i_rTextBox );
        Image       impl_getExpansionIndicator() const;

        // don't expose SetText. Our text is used as AccessibleName/Desc, and those are not expected to change.
        using Window::SetText;
        using Window::Paint;

    private:
        ScopedVclPtr<VirtualDevice> m_pPaintDevice;
        VclPtr<DrawerVisualization> m_aVisualization;
        bool m_bFocused : 1;
        bool m_bExpanded : 1;
    };
} // namespace svt

#endif // INCLUDED_SVTOOLS_SOURCE_TOOLPANEL_TOOLPANELDRAWER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
