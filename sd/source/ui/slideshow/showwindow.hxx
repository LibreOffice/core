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

#ifndef INCLUDED_SD_SOURCE_UI_SLIDESHOW_SHOWWINDOW_HXX
#define INCLUDED_SD_SOURCE_UI_SLIDESHOW_SHOWWINDOW_HXX

#include <rtl/ref.hxx>
#include <sal/types.h>
#include <vcl/timer.hxx>
#include <vcl/graph.hxx>

#include <Window.hxx>

namespace sd {

class SlideshowImpl;

#define SLIDE_NO_TIMEOUT SAL_MAX_INT32

enum ShowWindowMode
{
    SHOWWINDOWMODE_NORMAL = 0,
    SHOWWINDOWMODE_PAUSE = 1,
    SHOWWINDOWMODE_END = 2,
    SHOWWINDOWMODE_BLANK = 3,
    SHOWWINDOWMODE_PREVIEW = 4
};

class ShowWindow
    : public ::sd::Window
{

public:
    ShowWindow ( const ::rtl::Reference< ::sd::SlideshowImpl >& xController, vcl::Window* pParent );
    virtual ~ShowWindow() override;
    virtual void dispose() override;

    void            SetEndMode();
    bool            SetPauseMode( sal_Int32 nTimeoutSec, Graphic const * pLogo = nullptr );
    bool            SetBlankMode( sal_Int32 nPageIndexToRestart, const Color& rBlankColor );

    const Color&        GetBlankColor() const { return maShowBackground.GetColor(); }

    void            SetPreviewMode();

    void            SetMouseAutoHide( bool bMouseAutoHide ) { mbMouseAutoHide = bMouseAutoHide; }

    ShowWindowMode  GetShowWindowMode() const { return meShowWindowMode; }

    void            RestartShow( sal_Int32 nPageIndexToRestart );

    virtual void    LoseFocus() override;

    virtual void    KeyInput(const KeyEvent& rKEvt) override;
    virtual void    MouseMove(const MouseEvent& rMEvt) override;
    virtual void    MouseButtonUp(const MouseEvent& rMEvt) override;
    virtual void    MouseButtonDown(const MouseEvent& rMEvt) override;
    virtual void    Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect) override;
    /// Override the sd::Window's CreateAccessible to create a different accessible object
    virtual css::uno::Reference<css::accessibility::XAccessible>
        CreateAccessible() override;

    void            TerminateShow();
    void            RestartShow();

private:
    void            DrawPauseScene( bool bTimeoutOnly );
    void            DrawEndScene();

    void            DeleteWindowFromPaintView();
    void            AddWindowToPaintView();

private:
    Timer           maPauseTimer;
    Timer           maMouseTimer;
    Wallpaper       maShowBackground;
    Graphic         maLogo;
    sal_uLong           mnPauseTimeout;
    sal_Int32       mnRestartPageIndex;
    ShowWindowMode  meShowWindowMode;
    bool            mbShowNavigatorAfterSpecialMode;
    bool            mbMouseAutoHide;
    bool            mbMouseCursorHidden;
    sal_uInt64      mnFirstMouseMove;

                    DECL_LINK( PauseTimeoutHdl, Timer*, void );
                    DECL_LINK(MouseTimeoutHdl, Timer *, void);
                    DECL_LINK( EventHdl, VclWindowEvent&, void );

    ::rtl::Reference< SlideshowImpl > mxController;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
