/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#ifndef _SV_SALFRAME_H
#define _SV_SALFRAME_H

// -=-= #includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#include <salstd.hxx>
#include <vcl/salframe.hxx>
#include <vcl/salwtype.hxx>
#ifndef _SV_PTRSTYLE_HXX
#include <vcl/ptrstyle.hxx>
#endif
#include <vcl/salinst.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/timer.hxx>
#include <vcl/dllapi.h>

#include <salunx.h>
#include <list>

// -=-= forwards -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class   SalDisplay;
class   X11SalGraphics;
class SalGraphicsLayout;
class   SalI18N_InputContext;

namespace vcl_sal { class WMAdaptor; class NetWMAdaptor; class GnomeWMAdaptor; }

// -=-= X11SalFrame =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#define SHOWSTATE_UNKNOWN       -1
#define SHOWSTATE_MINIMIZED     0
#define SHOWSTATE_NORMAL        1
#define SHOWSTATE_HIDDEN        2

class VCL_DLLPUBLIC X11SalFrame : public SalFrame
{
    friend class vcl_sal::WMAdaptor;
    friend class vcl_sal::NetWMAdaptor;
    friend class vcl_sal::GnomeWMAdaptor;

    static X11SalFrame* s_pSaveYourselfFrame;

    X11SalFrame*    mpParent;            // pointer to parent frame
                                    // which should never obscur this frame
    bool            mbTransientForRoot;
    std::list< X11SalFrame* > maChildren;         // List of child frames

    SalDisplay     *pDisplay_;
    int             m_nScreen;
    XLIB_Window     mhWindow;
    XLIB_Window     mhShellWindow;
    XLIB_Window     mhForeignParent;
    // window to fall back to when no longer in fullscreen mode
    XLIB_Window     mhStackingWindow;
    // window to listen for CirculateNotify events

    Pixmap          mhBackgroundPixmap;

    XLIB_Cursor     hCursor_;
    int             nCaptured_;         // is captured

    X11SalGraphics  *pGraphics_;            // current frame graphics
    X11SalGraphics  *pFreeGraphics_;        // first free frame graphics

    XLIB_Time       nReleaseTime_;      // timestamp of last key release
    USHORT          nKeyCode_;          // last key code
    USHORT          nKeyState_;         // last key state
    int             nCompose_;          // compose state
    bool            mbSendExtKeyModChange;
    USHORT          mnExtKeyMod;

    int             nShowState_;        // show state
    int             nWidth_;            // client width
    int             nHeight_;           // client height
    Rectangle       maRestorePosSize;
    ULONG           nStyle_;
    SalExtStyle     mnExtStyle;
    BOOL            bAlwaysOnTop_;
    BOOL            bViewable_;
    BOOL            bMapped_;
    BOOL            mbInShow;
    BOOL            bDefaultPosition_;  // client is centered initially
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

    String          m_aTitle;

    rtl::OUString   m_sWMClass;

    SystemChildData maSystemChildData;

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
    void            SetWindowGravity (int nGravity ) const;

    void            RestackChildren( XLIB_Window* pTopLevelWindows, int nTopLevelWindows );
    void            RestackChildren();

    long            HandleKeyEvent      ( XKeyEvent         *pEvent );
    long            HandleMouseEvent    ( XEvent            *pEvent );
    long            HandleFocusEvent    ( XFocusChangeEvent *pEvent );
    long            HandleExposeEvent   ( XEvent            *pEvent );
    long            HandleSizeEvent     ( XConfigureEvent   *pEvent );
    long            HandleColormapEvent ( XColormapEvent    *pEvent );
    long            HandleMapUnmapEvent ( XEvent            *pEvent );
    long            HandleStateEvent    ( XPropertyEvent    *pEvent );
    long            HandleReparentEvent ( XReparentEvent    *pEvent );
    long            HandleClientMessage ( XClientMessageEvent*pEvent );

    DECL_LINK( HandleAlwaysOnTopRaise, void* );

    void            passOnSaveYourSelf();

    void            createNewWindow( XLIB_Window aParent, int nScreen = -1 );
    void            updateScreenNumber();

    void            setXEmbedInfo();
    void            askForXEmbedFocus( sal_Int32 i_nTimeCode );

    void            updateWMClass();
public:
    X11SalFrame( SalFrame* pParent, ULONG nSalFrameStyle, SystemParentData* pSystemParent = NULL );
    virtual ~X11SalFrame();

    long            Dispatch( XEvent *pEvent );
    void            Init( ULONG nSalFrameStyle, int nScreen = -1,
                          SystemParentData* pParentData = NULL, bool bUseGeometry = false );

    SalDisplay*             GetDisplay() const { return pDisplay_; }
    Display*                GetXDisplay() const;
    XLIB_Window             GetDrawable() const;
    int                     GetScreenNumber() const { return m_nScreen; }
    XLIB_Window             GetWindow() const { return mhWindow; }
    XLIB_Window             GetShellWindow() const { return mhShellWindow; }
    XLIB_Window             GetForeignParent() const { return mhForeignParent; }
    XLIB_Window             GetStackingWindow() const { return mhStackingWindow; }
    long                    ShutDown() const { return CallCallback( SALEVENT_SHUTDOWN, 0 ); }
    long                    Close() const { return CallCallback( SALEVENT_CLOSE, 0 ); }
              ULONG           GetStyle() const { return nStyle_; }

    inline  XLIB_Cursor     GetCursor() const { return hCursor_; }
    inline  BOOL            IsCaptured() const { return nCaptured_ == 1; }
#if !defined(__synchronous_extinput__)
    void            HandleExtTextEvent (XClientMessageEvent *pEvent);
    void            PostExtTextEvent (sal_uInt16 nExtTextEventType,
                                      void *pExtTextEvent);
#endif
    bool                    IsOverrideRedirect() const;
    bool                    IsChildWindow() const { return (nStyle_ & (SAL_FRAME_STYLE_PLUG|SAL_FRAME_STYLE_SYSTEMCHILD)) != 0; }
    bool                    IsSysChildWindow() const { return (nStyle_ & (SAL_FRAME_STYLE_SYSTEMCHILD)) != 0; }
    bool                    IsFloatGrabWindow() const;
    SalI18N_InputContext* getInputContext() const { return mpInputContext; }
    void                    getPosSize( Rectangle& rRect ) { GetPosSize( rRect ); }
    void                    setPosSize( const Rectangle& rRect ) { SetPosSize( rRect ); }
    bool                    isMapped() const { return bMapped_; }
    bool                    hasFocus() const { return mbInputFocus; }

    virtual SalGraphics*        GetGraphics();
    virtual void                ReleaseGraphics( SalGraphics* pGraphics );

    // call with true to clear graphics (setting None as drawable)
    // call with false to setup graphics with window (GetWindow())
    virtual void                updateGraphics( bool bClear );

    virtual BOOL                PostEvent( void* pData );

    virtual void                SetTitle( const XubString& rTitle );
    virtual void                SetIcon( USHORT nIcon );
    virtual void                SetMenu( SalMenu* pMenu );
    virtual void                                DrawMenuBar();

    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle );
    virtual void                Show( BOOL bVisible, BOOL bNoActivate = FALSE );
    virtual void                Enable( BOOL bEnable );
    virtual void              SetMinClientSize( long nWidth, long nHeight );
    virtual void              SetMaxClientSize( long nWidth, long nHeight );
    virtual void                SetPosSize( long nX, long nY, long nWidth, long nHeight, USHORT nFlags );
    virtual void                GetClientSize( long& rWidth, long& rHeight );
    virtual void                GetWorkArea( Rectangle& rRect );
    virtual SalFrame*           GetParent() const;
    virtual void                SetWindowState( const SalFrameState* pState );
    virtual BOOL                GetWindowState( SalFrameState* pState );
    virtual void                ShowFullScreen( BOOL bFullScreen, sal_Int32 nMonitor );
    virtual void                StartPresentation( BOOL bStart );
    virtual void                SetAlwaysOnTop( BOOL bOnTop );
    virtual void                ToTop( USHORT nFlags );
    virtual void                SetPointer( PointerStyle ePointerStyle );
    virtual void                CaptureMouse( BOOL bMouse );
    virtual void                SetPointerPos( long nX, long nY );
    using SalFrame::Flush;
    virtual void                Flush();
    virtual void                Sync();
    virtual void                SetInputContext( SalInputContext* pContext );
    virtual void                EndExtTextInput( USHORT nFlags );
    virtual String              GetKeyName( USHORT nKeyCode );
    virtual String              GetSymbolKeyName( const XubString& rFontName, USHORT nKeyCode );
    virtual BOOL                MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, KeyCode& rKeyCode );
    virtual LanguageType        GetInputLanguage();
    virtual SalBitmap*          SnapShot();
    virtual void                UpdateSettings( AllSettings& rSettings );
    virtual void                Beep( SoundType eSoundType );
    virtual const SystemEnvData*    GetSystemData() const;
    virtual SalPointerState     GetPointerState();
    virtual SalIndicatorState   GetIndicatorState();
    virtual void                SimulateKeyPress( USHORT nKeyCode );
    virtual void                SetParent( SalFrame* pNewParent );
    virtual bool                SetPluginParent( SystemParentData* pNewParent );
    virtual void                SetBackgroundBitmap( SalBitmap* pBitmap );

    virtual void                SetScreenNumber( unsigned int );
    virtual void                SetApplicationID( const rtl::OUString &rWMClass );

    // shaped system windows
    // set clip region to none (-> rectangular windows, normal state)
    virtual void                    ResetClipRegion();
    // start setting the clipregion consisting of nRects rectangles
    virtual void                    BeginSetClipRegion( ULONG nRects );
    // add a rectangle to the clip region
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight );
    // done setting up the clipregion
    virtual void                    EndSetClipRegion();

    static void SaveYourselfDone( SalFrame* );
    static Bool checkKeyReleaseForRepeat( Display*, XEvent*, XPointer pX11SalFrame );

    /// @internal
    void setPendingSizeEvent();
};

#ifdef _SV_SALDISP_HXX

inline Display *X11SalFrame::GetXDisplay() const
{ return pDisplay_->GetDisplay(); }

inline XLIB_Window X11SalFrame::GetDrawable() const
{ return GetWindow(); }

#endif

#endif // _SV_SALFRAME_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
