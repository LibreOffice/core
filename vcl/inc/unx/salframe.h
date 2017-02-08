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

#ifndef INCLUDED_VCL_INC_UNX_SALFRAME_H
#define INCLUDED_VCL_INC_UNX_SALFRAME_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include <unx/salunx.h>
#include <unx/saltype.h>
#include <unx/saldisp.hxx>
#include <unx/screensaverinhibitor.hxx>
#include <unx/nativewindowhandleprovider.hxx>
#include <salframe.hxx>
#include <salwtype.hxx>
#include <salinst.hxx>

#include <vcl/ptrstyle.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/timer.hxx>
#include <vclpluginapi.h>

#include <list>

class   X11SalGraphics;
class   SalI18N_InputContext;

namespace vcl_sal { class WMAdaptor; class NetWMAdaptor; class GnomeWMAdaptor; }

// X11SalFrame
#define SHOWSTATE_UNKNOWN       -1
#define SHOWSTATE_MINIMIZED     0
#define SHOWSTATE_NORMAL        1
#define SHOWSTATE_HIDDEN        2

enum class WMWindowType
{
    Normal,
    ModalDialogue,
    ModelessDialogue,
    Utility,
    Splash,
    Toolbar,
    Dock
};

class VCLPLUG_GEN_PUBLIC X11SalFrame : public SalFrame, public NativeWindowHandleProvider
{
    friend class vcl_sal::WMAdaptor;
    friend class vcl_sal::NetWMAdaptor;
    friend class vcl_sal::GnomeWMAdaptor;

    X11SalFrame*    mpParent;             // pointer to parent frame
                                          // which should never obscure this frame
    bool            mbTransientForRoot;
    std::list< X11SalFrame* > maChildren; // List of child frames

    SalDisplay     *pDisplay_;
    SalX11Screen    m_nXScreen;
    ::Window        mhWindow;
    ::Window        mhShellWindow;
    ::Window        mhForeignParent;
    // window to fall back to when no longer in fullscreen mode
    ::Window        mhStackingWindow;
    // window to listen for CirculateNotify events

    Cursor          hCursor_;
    int             nCaptured_;         // is captured

    X11SalGraphics  *pGraphics_;            // current frame graphics
    X11SalGraphics  *pFreeGraphics_;        // first free frame graphics

    sal_uInt16      nKeyCode_;          // last key code
    sal_uInt16      nKeyState_;         // last key state
    bool            mbSendExtKeyModChange;
    ModKeyFlags     mnExtKeyMod;

    int             nShowState_;        // show state
    int             nWidth_;            // client width
    int             nHeight_;           // client height
    Rectangle       maRestorePosSize;
    SalFrameStyleFlags nStyle_;
    SalExtStyle     mnExtStyle;
    bool            bAlwaysOnTop_;
    bool            bViewable_;
    bool            bMapped_;
    bool            mbInShow;
    bool            bDefaultPosition_;  // client is centered initially
    bool            m_bXEmbed;
    int             nVisibility_;
    int             m_nWorkArea;
    bool            m_bSetFocusOnMap;

    ScreenSaverInhibitor maScreenSaverInhibitor;
    Rectangle       maPaintRegion;

    Timer           maAlwaysOnTopRaiseTimer;

    // data for WMAdaptor
    WMWindowType    meWindowType;
    int             mnDecorationFlags;
    bool            mbMaximizedVert;
    bool            mbMaximizedHorz;
    bool            mbShaded;
    bool            mbFullScreen;

    // icon id
    int             mnIconID;

    OUString        m_aTitle;

    OUString        m_sWMClass;

    SystemEnvData maSystemChildData;

    SalI18N_InputContext *mpInputContext;
    Bool            mbInputFocus;

    XRectangle*     m_pClipRectangles;
    int             m_nCurClipRect;
    int             m_nMaxClipRect;

    bool mPendingSizeEvent;

    void            GetPosSize( Rectangle &rPosSize );
    void            SetSize   ( const Size      &rSize );
    void            Center();
    void            SetPosSize( const Rectangle &rPosSize );
    void            Minimize();
    void            Maximize();
    void            Restore();

    void            RestackChildren( ::Window* pTopLevelWindows, int nTopLevelWindows );
    void            RestackChildren();

    long            HandleKeyEvent      ( XKeyEvent         *pEvent );
    long            HandleMouseEvent    ( XEvent            *pEvent );
    long            HandleFocusEvent    ( XFocusChangeEvent *pEvent );
    long            HandleExposeEvent   ( XEvent            *pEvent );
    long            HandleSizeEvent     ( XConfigureEvent   *pEvent );
    long            HandleStateEvent    ( XPropertyEvent    *pEvent );
    long            HandleReparentEvent ( XReparentEvent    *pEvent );
    long            HandleClientMessage ( XClientMessageEvent*pEvent );

    DECL_LINK( HandleAlwaysOnTopRaise, Timer*, void );

    void            createNewWindow( ::Window aParent, SalX11Screen nXScreen = SalX11Screen( -1 ) );
    void            updateScreenNumber();

    void            setXEmbedInfo();
    void            askForXEmbedFocus( sal_Int32 i_nTimeCode );

    void            updateWMClass();
public:
    X11SalFrame( SalFrame* pParent, SalFrameStyleFlags nSalFrameStyle, SystemParentData* pSystemParent = nullptr );
    virtual ~X11SalFrame() override;

    long            Dispatch( XEvent *pEvent );
    void            Init( SalFrameStyleFlags nSalFrameStyle, SalX11Screen nScreen,
                          SystemParentData* pParentData, bool bUseGeometry = false );

    SalDisplay* GetDisplay() const
    {
        return pDisplay_;
    }
    Display *GetXDisplay() const
    {
        return pDisplay_->GetDisplay();
    }
    const SalX11Screen&     GetScreenNumber() const { return m_nXScreen; }
    ::Window                GetWindow() const { return mhWindow; }
    ::Window                GetShellWindow() const { return mhShellWindow; }
    ::Window                GetForeignParent() const { return mhForeignParent; }
    ::Window                GetStackingWindow() const { return mhStackingWindow; }
    void                    Close() const { CallCallback( SalEvent::Close, nullptr ); }
    SalFrameStyleFlags      GetStyle() const { return nStyle_; }

    Cursor                  GetCursor() const { return hCursor_; }
    bool                    IsCaptured() const { return nCaptured_ == 1; }
#if !defined(__synchronous_extinput__)
    void                    HandleExtTextEvent (XClientMessageEvent *pEvent);
#endif
    bool                    IsOverrideRedirect() const;
    bool                    IsChildWindow() const { return bool(nStyle_ & (SalFrameStyleFlags::PLUG|SalFrameStyleFlags::SYSTEMCHILD)); }
    bool                    IsSysChildWindow() const { return bool(nStyle_ & (SalFrameStyleFlags::SYSTEMCHILD)); }
    bool                    IsFloatGrabWindow() const;
    SalI18N_InputContext* getInputContext() const { return mpInputContext; }
    bool                    isMapped() const { return bMapped_; }
    bool                    hasFocus() const { return mbInputFocus; }

    void                    beginUnicodeSequence();
    bool                    appendUnicodeSequence( sal_Unicode );
    bool                    endUnicodeSequence();

    virtual SalGraphics*        AcquireGraphics() override;
    virtual void                ReleaseGraphics( SalGraphics* pGraphics ) override;

    // call with true to clear graphics (setting None as drawable)
    // call with false to setup graphics with window (GetWindow())
    virtual void                updateGraphics( bool bClear );

    virtual bool                PostEvent(ImplSVEvent* pData) override;

    virtual void                SetTitle( const OUString& rTitle ) override;
    virtual void                SetIcon( sal_uInt16 nIcon ) override;
    virtual void                SetMenu( SalMenu* pMenu ) override;
    virtual void                                DrawMenuBar() override;

    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle ) override;
    virtual void                Show( bool bVisible, bool bNoActivate = false ) override;
    virtual void                SetMinClientSize( long nWidth, long nHeight ) override;
    virtual void                SetMaxClientSize( long nWidth, long nHeight ) override;
    virtual void                SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags ) override;
    virtual void                GetClientSize( long& rWidth, long& rHeight ) override;
    virtual void                GetWorkArea( Rectangle& rRect ) override;
    virtual SalFrame*           GetParent() const override;
    virtual void                SetWindowState( const SalFrameState* pState ) override;
    virtual bool                GetWindowState( SalFrameState* pState ) override;
    virtual void                ShowFullScreen( bool bFullScreen, sal_Int32 nMonitor ) override;
    virtual void                StartPresentation( bool bStart ) override;
    virtual void                SetAlwaysOnTop( bool bOnTop ) override;
    virtual void                ToTop( SalFrameToTop nFlags ) override;
    virtual void                SetPointer( PointerStyle ePointerStyle ) override;
    virtual void                CaptureMouse( bool bMouse ) override;
    virtual void                SetPointerPos( long nX, long nY ) override;
    using SalFrame::Flush;
    virtual void                Flush() override;
    virtual void                SetInputContext( SalInputContext* pContext ) override;
    virtual void                EndExtTextInput( EndExtTextInputFlags nFlags ) override;
    virtual OUString              GetKeyName( sal_uInt16 nKeyCode ) override;
    virtual bool                MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode ) override;
    virtual LanguageType        GetInputLanguage() override;
    virtual void                UpdateSettings( AllSettings& rSettings ) override;
    virtual void                Beep() override;
    virtual const SystemEnvData*    GetSystemData() const override;
    virtual SalPointerState     GetPointerState() override;
    virtual KeyIndicatorState   GetIndicatorState() override;
    virtual void                SimulateKeyPress( sal_uInt16 nKeyCode ) override;
    virtual void                SetParent( SalFrame* pNewParent ) override;
    virtual bool                SetPluginParent( SystemParentData* pNewParent ) override;

    virtual void                SetScreenNumber( unsigned int ) override;
    virtual void                SetApplicationID( const OUString &rWMClass ) override;

    // shaped system windows
    // set clip region to none (-> rectangular windows, normal state)
    virtual void                    ResetClipRegion() override;
    // start setting the clipregion consisting of nRects rectangles
    virtual void                    BeginSetClipRegion( sal_uIntPtr nRects ) override;
    // add a rectangle to the clip region
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) override;
    // done setting up the clipregion
    virtual void                    EndSetClipRegion() override;

    virtual sal_uIntPtr         GetNativeWindowHandle() override;

    /// @internal
    void setPendingSizeEvent();
};

#endif // INCLUDED_VCL_INC_UNX_SALFRAME_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
