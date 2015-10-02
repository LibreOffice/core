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

#include <prex.h>
#include <postx.h>

#include <unx/salunx.h>
#include <unx/saltype.h>
#include <unx/saldisp.hxx>
#include <unx/x11windowprovider.hxx>
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

class VCLPLUG_GEN_PUBLIC X11SalFrame : public SalFrame, public X11WindowProvider
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

    Pixmap          mhBackgroundPixmap;

    Cursor          hCursor_;
    int             nCaptured_;         // is captured

    X11SalGraphics  *pGraphics_;            // current frame graphics
    X11SalGraphics  *pFreeGraphics_;        // first free frame graphics

    sal_uInt16      nKeyCode_;          // last key code
    sal_uInt16      nKeyState_;         // last key state
    int             nCompose_;          // compose state
    bool            mbSendExtKeyModChange;
    sal_uInt16      mnExtKeyMod;

    int             nShowState_;        // show state
    int             nWidth_;            // client width
    int             nHeight_;           // client height
    Rectangle       maRestorePosSize;
    sal_uIntPtr         nStyle_;
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

    int             nScreenSaversTimeout_;
    Rectangle       maPaintRegion;

    Timer           maAlwaysOnTopRaiseTimer;

    // data for WMAdaptor
    int             meWindowType;
    int             mnDecorationFlags;
    bool            mbMaximizedVert;
    bool            mbMaximizedHorz;
    bool            mbShaded;
    bool            mbFullScreen;

    // icon id
    int             mnIconID;

    OUString          m_aTitle;

    OUString   m_sWMClass;

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

    DECL_LINK_TYPED( HandleAlwaysOnTopRaise, Timer*, void );

    void            createNewWindow( ::Window aParent, SalX11Screen nXScreen = SalX11Screen( -1 ) );
    void            updateScreenNumber();

    void            setXEmbedInfo();
    void            askForXEmbedFocus( sal_Int32 i_nTimeCode );

    void            updateWMClass();
public:
    X11SalFrame( SalFrame* pParent, sal_uIntPtr nSalFrameStyle, SystemParentData* pSystemParent = NULL );
    virtual ~X11SalFrame();

    long            Dispatch( XEvent *pEvent );
    void            Init( sal_uIntPtr nSalFrameStyle, SalX11Screen nScreen = SalX11Screen( -1 ),
                          SystemParentData* pParentData = NULL, bool bUseGeometry = false );

    SalDisplay* GetDisplay() const
    {
        return pDisplay_;
    }
    Display *GetXDisplay() const
    {
        return pDisplay_->GetDisplay();
    }
    SalX11Screen            GetScreenNumber() const { return m_nXScreen; }
    ::Window                GetWindow() const { return mhWindow; }
    ::Window                GetShellWindow() const { return mhShellWindow; }
    ::Window                GetForeignParent() const { return mhForeignParent; }
    ::Window                GetStackingWindow() const { return mhStackingWindow; }
    long                    Close() const { return CallCallback( SALEVENT_CLOSE, 0 ); }
              sal_uIntPtr           GetStyle() const { return nStyle_; }

    Cursor          GetCursor() const { return hCursor_; }
    bool            IsCaptured() const { return nCaptured_ == 1; }
#if !defined(__synchronous_extinput__)
    void            HandleExtTextEvent (XClientMessageEvent *pEvent);
#endif
    bool                    IsOverrideRedirect() const;
    bool                    IsChildWindow() const { return (nStyle_ & (SAL_FRAME_STYLE_PLUG|SAL_FRAME_STYLE_SYSTEMCHILD)) != 0; }
    bool                    IsSysChildWindow() const { return (nStyle_ & (SAL_FRAME_STYLE_SYSTEMCHILD)) != 0; }
    bool                    IsFloatGrabWindow() const;
    SalI18N_InputContext* getInputContext() const { return mpInputContext; }
    bool                    isMapped() const { return bMapped_; }
    bool                    hasFocus() const { return mbInputFocus; }

    void                    beginUnicodeSequence();
    bool                    appendUnicodeSequence( sal_Unicode );
    bool                    endUnicodeSequence();

    virtual SalGraphics*        AcquireGraphics() SAL_OVERRIDE;
    virtual void                ReleaseGraphics( SalGraphics* pGraphics ) SAL_OVERRIDE;

    // call with true to clear graphics (setting None as drawable)
    // call with false to setup graphics with window (GetWindow())
    virtual void                updateGraphics( bool bClear );

    virtual bool                PostEvent(ImplSVEvent* pData) SAL_OVERRIDE;

    virtual void                SetTitle( const OUString& rTitle ) SAL_OVERRIDE;
    virtual void                SetIcon( sal_uInt16 nIcon ) SAL_OVERRIDE;
    virtual void                SetMenu( SalMenu* pMenu ) SAL_OVERRIDE;
    virtual void                                DrawMenuBar() SAL_OVERRIDE;

    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle ) SAL_OVERRIDE;
    virtual void                Show( bool bVisible, bool bNoActivate = false ) SAL_OVERRIDE;
    virtual void                SetMinClientSize( long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual void                SetMaxClientSize( long nWidth, long nHeight ) SAL_OVERRIDE;
    virtual void                SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags ) SAL_OVERRIDE;
    virtual void                GetClientSize( long& rWidth, long& rHeight ) SAL_OVERRIDE;
    virtual void                GetWorkArea( Rectangle& rRect ) SAL_OVERRIDE;
    virtual SalFrame*           GetParent() const SAL_OVERRIDE;
    virtual void                SetWindowState( const SalFrameState* pState ) SAL_OVERRIDE;
    virtual bool                GetWindowState( SalFrameState* pState ) SAL_OVERRIDE;
    virtual void                ShowFullScreen( bool bFullScreen, sal_Int32 nMonitor ) SAL_OVERRIDE;
    virtual void                StartPresentation( bool bStart ) SAL_OVERRIDE;
    virtual void                SetAlwaysOnTop( bool bOnTop ) SAL_OVERRIDE;
    virtual void                ToTop( sal_uInt16 nFlags ) SAL_OVERRIDE;
    virtual void                SetPointer( PointerStyle ePointerStyle ) SAL_OVERRIDE;
    virtual void                CaptureMouse( bool bMouse ) SAL_OVERRIDE;
    virtual void                SetPointerPos( long nX, long nY ) SAL_OVERRIDE;
    using SalFrame::Flush;
    virtual void                Flush() SAL_OVERRIDE;
    virtual void                Sync() SAL_OVERRIDE;
    virtual void                SetInputContext( SalInputContext* pContext ) SAL_OVERRIDE;
    virtual void                EndExtTextInput( EndExtTextInputFlags nFlags ) SAL_OVERRIDE;
    virtual OUString              GetKeyName( sal_uInt16 nKeyCode ) SAL_OVERRIDE;
    virtual bool                MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode ) SAL_OVERRIDE;
    virtual LanguageType        GetInputLanguage() SAL_OVERRIDE;
    virtual void                UpdateSettings( AllSettings& rSettings ) SAL_OVERRIDE;
    virtual void                Beep() SAL_OVERRIDE;
    virtual const SystemEnvData*    GetSystemData() const SAL_OVERRIDE;
    virtual SalPointerState     GetPointerState() SAL_OVERRIDE;
    virtual KeyIndicatorState   GetIndicatorState() SAL_OVERRIDE;
    virtual void                SimulateKeyPress( sal_uInt16 nKeyCode ) SAL_OVERRIDE;
    virtual void                SetParent( SalFrame* pNewParent ) SAL_OVERRIDE;
    virtual bool                SetPluginParent( SystemParentData* pNewParent ) SAL_OVERRIDE;

    virtual void                SetScreenNumber( unsigned int ) SAL_OVERRIDE;
    virtual void                SetApplicationID( const OUString &rWMClass ) SAL_OVERRIDE;

    // shaped system windows
    // set clip region to none (-> rectangular windows, normal state)
    virtual void                    ResetClipRegion() SAL_OVERRIDE;
    // start setting the clipregion consisting of nRects rectangles
    virtual void                    BeginSetClipRegion( sal_uIntPtr nRects ) SAL_OVERRIDE;
    // add a rectangle to the clip region
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) SAL_OVERRIDE;
    // done setting up the clipregion
    virtual void                    EndSetClipRegion() SAL_OVERRIDE;

    virtual Window GetX11Window() SAL_OVERRIDE;

    /// @internal
    void setPendingSizeEvent();
};

#endif // INCLUDED_VCL_INC_UNX_SALFRAME_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
