/*************************************************************************
 *
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

#ifndef SD_SHOW_WINDOW_HXX
#define SD_SHOW_WINDOW_HXX

#include <rtl/ref.hxx>
#include <sal/types.h>
#include <vcl/timer.hxx>
#include <vcl/graph.hxx>

#ifndef SD_WINDOW_HXX
#include "Window.hxx"
#endif

#include "slideshowimpl.hxx"

// ----------------
// - SdShowWindow -
// ----------------


namespace sd {

class SlideshowImpl;
class PreviewWindow;
class ViewShell;

// -----------
// - Defines -
// -----------

#define SLIDE_NO_TIMEOUT SAL_MAX_INT32

// ---------
// - Enums -
// ---------

enum ShowWindowMode
{
    SHOWWINDOWMODE_NORMAL = 0,
    SHOWWINDOWMODE_PAUSE = 1,
    SHOWWINDOWMODE_END = 2,
    SHOWWINDOWMODE_BLANK = 3,
    SHOWWINDOWMODE_PREVIEW = 4
};

//class ShowWindowImpl;

class ShowWindow
    : public ::sd::Window
{
//  friend class ShowWindowImpl;
public:
    ShowWindow ( const ::rtl::Reference< ::sd::SlideshowImpl >& xController, ::Window* pParent );
    virtual ~ShowWindow (void);

       sal_Bool         SetEndMode();
    sal_Bool            SetPauseMode( sal_Int32 nPageIndexToRestart, sal_Int32 nTimeoutSec = SLIDE_NO_TIMEOUT, Graphic* pLogo = NULL );
    sal_Bool            SetBlankMode( sal_Int32 nPageIndexToRestart, const Color& rBlankColor );

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
//  virtual void    GrabFocus();

    virtual void    KeyInput(const KeyEvent& rKEvt);
    virtual void    MouseMove(const MouseEvent& rMEvt);
    virtual void    MouseButtonUp(const MouseEvent& rMEvt);
    virtual void    MouseButtonDown(const MouseEvent& rMEvt);
    virtual void    Paint(const Rectangle& rRect);
    virtual long    Notify(NotifyEvent& rNEvt);

    void            TerminateShow();
    void            RestartShow();

private:
    void            DrawPauseScene( sal_Bool bTimeoutOnly );
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
    sal_Bool            mbShowNavigatorAfterSpecialMode;
    Rectangle       maPresArea;
    bool            mbMouseAutoHide;
    bool            mbMouseCursorHidden;
    sal_uLong           mnFirstMouseMove;

                    DECL_LINK( PauseTimeoutHdl, Timer* pTimer );
                    DECL_LINK( MouseTimeoutHdl, Timer* pTimer );
                    DECL_LINK( EventHdl, VclWindowEvent* pEvent );

    ::rtl::Reference< SlideshowImpl > mxController;
};

} // end of namespace sd

#endif
