/*************************************************************************
 *
 *  $RCSfile: salframe.h,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: cp $ $Date: 2001-03-02 07:51:54 $
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
#ifndef _SV_SALWTYPE_HXX
#include <salwtype.hxx>
#endif
#ifndef _SV_PTRSTYLE_HXX
#include <ptrstyle.hxx>
#endif

#ifndef _LIST_HXX
#include <tools/list.hxx>
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

// -=-= forwards -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
class   SalDisplay;
class   SalGraphics;
class   SalFrame;
class   SalColormap;
class   SalI18N_InputContext;

// -=-= SalFrameData =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
DECLARE_LIST( SalFrameList, SalFrame *);
class SalFrameData
{
    friend  class           SalFrame;
    friend  SalFrame* SalInstance::CreateFrame( SalFrame*, ULONG );
    friend  SalFrame* SalInstance::CreateChildFrame( SystemParentData*, ULONG );

    static Bool checkKeyReleaseForRepeat( Display*, XEvent*, XPointer pSalFrameData );

                            STDAPI( SalFrameData )

            SalFrame       *pNextFrame_;        // pointer to next frame
            SalFrame       *pFrame_;

            SalFrame*       mpParent;            // pointer to parent frame
                                    // which should never obscur this frame
             SalFrameList    maChildren;         // List of child frames

            SALFRAMEPROC    pProc_;             // callback proc
            void           *pInst_;             // instance handle for callback

            SalDisplay     *pDisplay_;
            Widget          hShell_;
            Widget          hComposite_;
            XLIB_Window     hForeignParent_;
            XLIB_Window     hForeignTopLevelWindow_;
            Widget          hNoFullscreenShell_;
            Widget          hNoFullscreenComposite_;
    static  XLIB_Window     s_aFullScreenWindow;
            // window to fall back to when no longer in fullscreen mode
            XLIB_Window     hStackingWindow_;
            // window to listen for CirculateNotify events

            XLIB_Cursor     hCursor_;
            int             nCaptured_;         // is captured

            SalGraphics    *pGraphics_;         // current frame graphics
            SalGraphics    *pFreeGraphics_;     // first free frame graphics
            XLIB_Region     pPaintRegion_;

             XLIB_Time      nReleaseTime_;      // timestamp of last key release
            USHORT          nKeyCode_;          // last key code
            USHORT          nKeyState_;         // last key state
            int             nCompose_;          // compose state

            int             nShowState_;        // show state
            int             nLeft_;             // left decoration size
            int             nTop_;              // top decoration size
            int             nRight_;            // right decoration size
            int             nBottom_;           // bottom decoration size
            int             nMaxWidth_;         // client max width
            int             nMaxHeight_;        // client max height
            int             nWidth_;            // client width
            int             nHeight_;           // client height
            Rectangle       aPosSize_;          // Shells Pos&Size
            Rectangle       aRestoreFullScreen_;
            Rectangle       aRestoreMaximize_;
            USHORT          nStyle_;
            BOOL            bAlwaysOnTop_;
            BOOL            bViewable_;
            BOOL            bMapped_;
            BOOL            bDefaultPosition_;  // client is centered initially
            int             nVisibility_;

            int             nScreenSaversTimeout_;
            Timer           maResizeTimer;

            SystemChildData maSystemChildData;

            SalI18N_InputContext *mpInputContext;

            SalGraphics    *GetGraphics();

            void            GetPosSize( Rectangle &rPosSize );
            void            SetSize   ( const Size      &rSize );
            void            SetPosSize( const Rectangle &rPosSize );
            void            Minimize();
            void            Maximize();
            void            Restore();
            void            ShowFullScreen( BOOL bFullScreen );

            void            RepositionFloatChildren();
            void            RepositionChildren();

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

    inline  void            CaptureMouse( BOOL bCapture );
    inline  void            SetPointer( PointerStyle ePointerStyle );

    inline                  SalFrameData( SalFrame *pFrame );
    inline                  ~SalFrameData();

            DECL_LINK( HandleResizeTimer, void* );
public:
            long            Dispatch( XEvent *pEvent );
            void            Init( USHORT nSalFrameStyle, SystemParentData* pParentData = NULL );
    static  XLIB_Window     GetFullScreenWindow() { return s_aFullScreenWindow; }

            SalDisplay     *GetDisplay() const { return pDisplay_; }
    inline  Display        *GetXDisplay() const;
    inline  XLIB_Window     GetDrawable() const;
    inline  XLIB_Window     GetWindow() const { return XtWindow( hComposite_ ); }
    inline  Widget          GetWidget() const { return hComposite_; }
    inline  XLIB_Window     GetShellWindow() const { return XtWindow( hShell_ ); }
    inline  Widget          GetShellWidget() const { return hShell_; }
    inline  XLIB_Window     GetForeignParent() const { return hForeignParent_; }
    inline  XLIB_Window     GetForeignTopLevelWindow() const { return hForeignTopLevelWindow_; }
    inline  XLIB_Window     GetStackingWindow() const { return hStackingWindow_; }
    inline  long            ShutDown() const
                { return pProc_( pInst_, pFrame_, SALEVENT_SHUTDOWN, 0 ); }
    inline  long            Close() const
                { return pProc_( pInst_, pFrame_, SALEVENT_CLOSE, 0 ); }
    inline  long            Call( USHORT nEvent, const void *pEvent ) const
                { return pProc_( pInst_, pFrame_, nEvent, pEvent ); }
    inline  SalFrame       *GetNextFrame() const { return pNextFrame_; }
    inline  XLIB_Cursor     GetCursor() const { return hCursor_; }
    inline  BOOL            IsCaptured() const { return nCaptured_ == 1; }
    inline  BOOL            IsWaitingForExpose() const
                                { return !!pPaintRegion_; }
    #if !defined(__synchronous_extinput__)
            void            HandleExtTextEvent (XClientMessageEvent *pEvent);
            void            PostExtTextEvent (sal_uInt16 nExtTextEventType,
                                void *pExtTextEvent);
    #endif
    inline  SalColormap    &GetColormap() const;
};

#ifdef _SV_SALDISP_HXX

inline void SalFrameData::CaptureMouse( BOOL bCapture )
{ nCaptured_ = pDisplay_->CaptureMouse( bCapture ? this : NULL ); }

inline Display *SalFrameData::GetXDisplay() const
{ return pDisplay_->GetDisplay(); }

inline XLIB_Window SalFrameData::GetDrawable() const
{ return GetWindow(); }

inline  SalColormap &SalFrameData::GetColormap() const
{ return pDisplay_->GetColormap(); }

#endif

#endif // _SV_SALFRAME_H

