/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: showwindow.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2008-04-03 14:11:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef SD_SHOW_WINDOW_HXX
#define SD_SHOW_WINDOW_HXX

#include <rtl/ref.hxx>
#include <limits.h>
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

#define SLIDE_NO_TIMEOUT LONG_MAX

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

       BOOL         SetEndMode();
    BOOL            SetPauseMode( sal_Int32 nPageIndexToRestart, sal_Int32 nTimeoutSec = SLIDE_NO_TIMEOUT, Graphic* pLogo = NULL );
    BOOL            SetBlankMode( sal_Int32 nPageIndexToRestart, const Color& rBlankColor );

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
    void            DrawPauseScene( BOOL bTimeoutOnly );
    void            DrawEndScene();
    void            DrawBlankScene();

    void            DeleteWindowFromPaintView();
    void            AddWindowToPaintView();

private:
    Timer           maPauseTimer;
    Timer           maMouseTimer;
    Wallpaper       maShowBackground;
    Graphic         maLogo;
    ULONG           mnPauseTimeout;
    sal_Int32       mnRestartPageIndex;
    ShowWindowMode  meShowWindowMode;
    BOOL            mbShowNavigatorAfterSpecialMode;
    Rectangle       maPresArea;
    bool            mbMouseAutoHide;
    bool            mbMouseCursorHidden;
    ULONG           mnFirstMouseMove;

                    DECL_LINK( PauseTimeoutHdl, Timer* pTimer );
                    DECL_LINK( MouseTimeoutHdl, Timer* pTimer );
                    DECL_LINK( EventHdl, VclWindowEvent* pEvent );

    ::rtl::Reference< SlideshowImpl > mxController;
};

} // end of namespace sd

#endif
