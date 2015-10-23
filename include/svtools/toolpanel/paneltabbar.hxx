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

#ifndef INCLUDED_SVTOOLS_TOOLPANEL_PANELTABBAR_HXX
#define INCLUDED_SVTOOLS_TOOLPANEL_PANELTABBAR_HXX

#include <svtools/svtdllapi.h>
#include <svtools/toolpanel/tabalignment.hxx>
#include <svtools/toolpanel/tabitemcontent.hxx>

#include <vcl/ctrl.hxx>

#include <memory>
#include <boost/optional.hpp>

class PushButton;


namespace svt
{


    class PanelTabBar_Impl;
    class IToolPanelDeck;


    //= PanelTabBar

    /** a tab bar for selecting panels

        At the moment, this control aligns the tabs vertically, this might be extended to also support a horizontal
        layout in the future.
    */
    class SVT_DLLPUBLIC PanelTabBar : public Control
    {
    public:
        PanelTabBar( vcl::Window& i_rParentWindow, IToolPanelDeck& i_rPanelDeck, const TabAlignment i_eAlignment, const TabItemContent i_eItemContent );
        virtual ~PanelTabBar();
        virtual void dispose() override;

        // attribute access
        TabItemContent  GetTabItemContent() const;
        void            SetTabItemContent( const TabItemContent& i_eItemContent );

        ::boost::optional< size_t > GetFocusedPanelItem() const;
        void                        FocusPanelItem( const size_t i_nItemPos );
        Rectangle                   GetItemScreenRect( const size_t i_nItemPos ) const;
        bool                        IsVertical() const;
        IToolPanelDeck&             GetPanelDeck() const;
        PushButton&                 GetScrollButton( const bool i_bForward );

        // Window overridables
        virtual Size    GetOptimalSize() const override;

    protected:
        // Window overridables
        virtual void    Paint( vcl::RenderContext& rRenderContext, const Rectangle& i_rRect ) override;
        virtual void    Resize() override;
        virtual void    MouseMove( const MouseEvent& i_rMouseEvent ) override;
        virtual void    MouseButtonDown( const MouseEvent& i_rMouseEvent ) override;
        virtual void    MouseButtonUp( const MouseEvent& i_rMouseEvent ) override;
        virtual void    RequestHelp( const HelpEvent& i_rHelpEvent ) override;
        virtual void    GetFocus() override;
        virtual void    LoseFocus() override;
        virtual void    KeyInput( const KeyEvent& i_rKeyEvent ) override;
        virtual void    DataChanged( const DataChangedEvent& i_rDataChanedEvent ) override;

        virtual css::uno::Reference< css::awt::XWindowPeer >
                        GetComponentInterface( bool i_bCreate ) override;

    private:
        ::std::unique_ptr< PanelTabBar_Impl > m_pImpl;
    };


} // namespace svt


#endif // INCLUDED_SVTOOLS_TOOLPANEL_PANELTABBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
