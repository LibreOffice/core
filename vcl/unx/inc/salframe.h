/*************************************************************************
 *
 *  $RCSfile: salframe.h,v $
 *
 *  $Revision: 1.40 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-23 10:05:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _SV_SALFRAME_H
#define _SV_SALFRAME_H

// -=-= #includes -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#ifndef _SALSTD_HXX
#include <salstd.hxx>
#endif
#ifndef _SV_SALFRAME_HXX
#include <salframe.hxx>
#endif
#ifndef _SV_SALWTYPE_HXX
#include <salwtype.hxx>
#endif
#ifndef _SV_PTRSTYLE_HXX
#include <ptrstyle.hxx>
#endif

#ifndef _SV_SALINST_HXX
#include <salinst.hxx>
#endif

#ifndef _SV_SYSDATA_HXX
#include <sysdata.hxx>
#endif

#ifndef _SV_TIMER_HXX
#include <timer.hxx>
#endif

#include <salunx.h>
#include <list>

// -=-= forwards -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class   SalDisplay;
class   X11SalGraphics;
class SalGraphicsLayout;
class   SalI18N_InputContext;

namespace vcl_sal { class WMAdaptor; class NetWMAdaptor; class GnomeWMAdaptor; }

class SalFrameDelData
{
    private:

        sal_Bool            mbDeleted;
        SalFrameDelData*    mpNext;

    public:
                            SalFrameDelData () : mbDeleted(sal_False), mpNext(NULL)
                                         {}
        void                Delete ()    { mbDeleted = sal_True; }
        sal_Bool            IsDeleted () { return mbDeleted; }
        void                SetNext (SalFrameDelData* pData)
                                         { mpNext = pData; }
        SalFrameDelData*    GetNext ()   { return mpNext; }
};

// -=-= X11SalFrame =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
#define SHOWSTATE_UNKNOWN       -1
#define SHOWSTATE_MINIMIZED     0
#define SHOWSTATE_NORMAL        1
#define SHOWSTATE_HIDDEN        2

class X11SalFrame : public SalFrame
{
    friend class vcl_sal::WMAdaptor;
    friend class vcl_sal::NetWMAdaptor;
    friend class vcl_sal::GnomeWMAdaptor;

    static Bool checkKeyReleaseForRepeat( Display*, XEvent*, XPointer pX11SalFrame );

    static X11SalFrame* s_pSaveYourselfFrame;

    X11SalFrame*    mpParent;            // pointer to parent frame
                                    // which should never obscur this frame
    bool            mbTransientForRoot;
    std::list< X11SalFrame* > maChildren;         // List of child frames

    SalDisplay     *pDisplay_;
    XLIB_Window     mhWindow;
    XLIB_Window     mhShellWindow;
    XLIB_Window     mhForeignParent;
    // window to fall back to when no longer in fullscreen mode
    XLIB_Window     mhStackingWindow;
    // window to listen for CirculateNotify events

    XLIB_Cursor     hCursor_;
    int             nCaptured_;         // is captured

    X11SalGraphics  *pGraphics_;            // current frame graphics
    X11SalGraphics  *pFreeGraphics_;        // first free frame graphics

    XLIB_Time       nReleaseTime_;      // timestamp of last key release
    USHORT          nKeyCode_;          // last key code
    USHORT          nKeyState_;         // last key state
    int             nCompose_;          // compose state
    bool            mbKeyMenu;
    bool            mbSendExtKeyModChange;
    USHORT          mnExtKeyMod;

    int             nShowState_;        // show state
    int             nWidth_;            // client width
    int             nHeight_;           // client height
    Rectangle       maRestorePosSize;
    ULONG           nStyle_;
    BOOL            bAlwaysOnTop_;
    BOOL            bViewable_;
    BOOL            bMapped_;
    BOOL            mbInShow;
    BOOL            bDefaultPosition_;  // client is centered initially
    int             nVisibility_;

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

    SystemChildData maSystemChildData;

    SalI18N_InputContext *mpInputContext;
    bool            mbDeleteInputContext;
    Bool            mbInputFocus;
    SalFrameDelData *mpDeleteData;
    void            NotifyDeleteData ();

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
public:
    X11SalFrame( SalFrame* pParent, ULONG nSalFrameStyle, SystemParentData* pSystemParent = NULL );
    virtual ~X11SalFrame();

    long            Dispatch( XEvent *pEvent );
    void            Init( ULONG nSalFrameStyle, SystemParentData* pParentData = NULL );

    SalDisplay*             GetDisplay() const { return pDisplay_; }
    Display*                GetXDisplay() const;
    XLIB_Window             GetDrawable() const;
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
    bool                    IsFloatGrabWindow() const;
    SalI18N_InputContext* getInputContext() const { return mpInputContext; }
    void                    getPosSize( Rectangle& rRect ) { GetPosSize( rRect ); }
    void                    setPosSize( const Rectangle& rRect ) { SetPosSize( rRect ); }
    bool                    isMapped() const { return bMapped_; }
    void            RegisterDeleteData (SalFrameDelData *pData);
    void            UnregisterDeleteData (SalFrameDelData *pData);

    virtual SalGraphics*        GetGraphics();
    virtual void                ReleaseGraphics( SalGraphics* pGraphics );

    virtual BOOL                PostEvent( void* pData );

    virtual void                SetTitle( const XubString& rTitle );
    virtual void                SetIcon( USHORT nIcon );
    virtual void                SetMenu( SalMenu* pMenu );
    virtual void                                DrawMenuBar();

    virtual void                Show( BOOL bVisible, BOOL bNoActivate = FALSE );
    virtual void                Enable( BOOL bEnable );
    virtual void              SetMinClientSize( long nWidth, long nHeight );
    virtual void                SetPosSize( long nX, long nY, long nWidth, long nHeight, USHORT nFlags );
    virtual void                GetClientSize( long& rWidth, long& rHeight );
    virtual void                GetWorkArea( Rectangle& rRect );
    virtual SalFrame*           GetParent() const;
    virtual void                SetWindowState( const SalFrameState* pState );
    virtual BOOL                GetWindowState( SalFrameState* pState );
    virtual void                ShowFullScreen( BOOL bFullScreen );
    virtual void                StartPresentation( BOOL bStart );
    virtual void                SetAlwaysOnTop( BOOL bOnTop );
    virtual void                ToTop( USHORT nFlags );
    virtual void                SetPointer( PointerStyle ePointerStyle );
    virtual void                CaptureMouse( BOOL bMouse );
    virtual void                SetPointerPos( long nX, long nY );
    virtual void                Flush();
    virtual void                Sync();
    virtual void                SetInputContext( SalInputContext* pContext );
    virtual void                EndExtTextInput( USHORT nFlags );
    virtual String              GetKeyName( USHORT nKeyCode );
    virtual String              GetSymbolKeyName( const XubString& rFontName, USHORT nKeyCode );
    virtual LanguageType        GetInputLanguage();
    virtual SalBitmap*          SnapShot();
    virtual void                UpdateSettings( AllSettings& rSettings );
    virtual void                Beep( SoundType eSoundType );
    virtual const SystemEnvData*    GetSystemData() const;
    virtual SalPointerState     GetPointerState();
    virtual void                SetParent( SalFrame* pNewParent );
    virtual bool                SetPluginParent( SystemParentData* pNewParent );

    static void SaveYourselfDone( SalFrame* );
};

#ifdef _SV_SALDISP_HXX

inline Display *X11SalFrame::GetXDisplay() const
{ return pDisplay_->GetDisplay(); }

inline XLIB_Window X11SalFrame::GetDrawable() const
{ return GetWindow(); }

#endif

#endif // _SV_SALFRAME_H

