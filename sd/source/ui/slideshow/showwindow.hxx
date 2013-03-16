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

#ifndef SD_SHOW_WINDOW_HXX
#define SD_SHOW_WINDOW_HXX

#include <rtl/ref.hxx>
#include <sal/types.h>
#include <vcl/timer.hxx>
#include <vcl/graph.hxx>

#include "Window.hxx"

#include "slideshowimpl.hxx"

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
    ShowWindow ( const ::rtl::Reference< ::sd::SlideshowImpl >& xController, ::Window* pParent );
    virtual ~ShowWindow (void);

    bool         SetEndMode();
    bool            SetPauseMode( sal_Int32 nPageIndexToRestart, sal_Int32 nTimeoutSec = SLIDE_NO_TIMEOUT, Graphic* pLogo = NULL );
    bool            SetBlankMode( sal_Int32 nPageIndexToRestart, const Color& rBlankColor );

    const Color&        GetBlankColor() const { return maShowBackground.GetColor(); }

    void            SetPreviewMode();
    void            SetPresentationArea( const Rectangle& rPresArea );

    void            SetMouseAutoHide( bool bMouseAutoHide ) { mbMouseAutoHide = bMouseAutoHide; }

    ShowWindowMode  GetShowWindowMode() const { return meShowWindowMode; }

    void            RestartShow( sal_Int32 nPageIndexToRestart );

    virtual void    Move();
    virtual void    Resize();
    virtual void    GetFocus();
    virtual void    LoseFocus();

    virtual void    KeyInput(const KeyEvent& rKEvt);
    virtual void    MouseMove(const MouseEvent& rMEvt);
    virtual void    MouseButtonUp(const MouseEvent& rMEvt);
    virtual void    MouseButtonDown(const MouseEvent& rMEvt);
    virtual void    Paint(const Rectangle& rRect);
    virtual long    Notify(NotifyEvent& rNEvt);

    void            TerminateShow();
    void            RestartShow();

private:
    void            DrawPauseScene( bool bTimeoutOnly );
    void            DrawEndScene();
    void            DrawBlankScene();

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
    Rectangle       maPresArea;
    bool            mbMouseAutoHide;
    bool            mbMouseCursorHidden;
    sal_uLong           mnFirstMouseMove;

                    DECL_LINK( PauseTimeoutHdl, Timer* pTimer );
                    DECL_LINK(MouseTimeoutHdl, void *);
                    DECL_LINK( EventHdl, VclWindowEvent* pEvent );

    ::rtl::Reference< SlideshowImpl > mxController;
};

} // end of namespace sd

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
