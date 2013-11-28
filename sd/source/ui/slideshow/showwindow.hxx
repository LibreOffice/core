/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    //Overload the sd::Window's CreateAccessible to create a different accessible object
    virtual ::com::sun::star::uno::Reference<
        ::com::sun::star::accessibility::XAccessible>
        CreateAccessible (void);

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
