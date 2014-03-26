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
        PanelTabBar( Window& i_rParentWindow, IToolPanelDeck& i_rPanelDeck, const TabAlignment i_eAlignment, const TabItemContent i_eItemContent );
        ~PanelTabBar();

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
        virtual Size    GetOptimalSize() const SAL_OVERRIDE;

    protected:
        // Window overridables
        virtual void    Paint( const Rectangle& i_rRect ) SAL_OVERRIDE;
        virtual void    Resize() SAL_OVERRIDE;
        virtual void    MouseMove( const MouseEvent& i_rMouseEvent ) SAL_OVERRIDE;
        virtual void    MouseButtonDown( const MouseEvent& i_rMouseEvent ) SAL_OVERRIDE;
        virtual void    MouseButtonUp( const MouseEvent& i_rMouseEvent ) SAL_OVERRIDE;
        virtual void    RequestHelp( const HelpEvent& i_rHelpEvent ) SAL_OVERRIDE;
        virtual void    GetFocus() SAL_OVERRIDE;
        virtual void    LoseFocus() SAL_OVERRIDE;
        virtual void    KeyInput( const KeyEvent& i_rKeyEvent ) SAL_OVERRIDE;
        virtual void    DataChanged( const DataChangedEvent& i_rDataChanedEvent ) SAL_OVERRIDE;

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >
                        GetComponentInterface( sal_Bool i_bCreate ) SAL_OVERRIDE;

    private:
        ::std::auto_ptr< PanelTabBar_Impl > m_pImpl;
    };


} // namespace svt


#endif // INCLUDED_SVTOOLS_TOOLPANEL_PANELTABBAR_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
