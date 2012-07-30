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

#ifndef _SV_SALFRAME_HXX
#define _SV_SALFRAME_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>

#ifdef __cplusplus

#include <vcl/ptrstyle.hxx>

#endif // __cplusplus
#include <salwtype.hxx>
#include <salgeom.hxx>
#include <tools/gen.hxx>
#include <vcl/region.hxx>

#include <vcl/impdel.hxx>
#include <rtl/ustring.hxx>
#include <vcl/keycod.hxx>

class AllSettings;
class SalGraphics;
class SalBitmap;
class SalMenu;
class Window;


struct SalFrameState;
struct SalInputContext;
struct SystemEnvData;

// -----------------
// - SalFrameTypes -
// -----------------

#define SAL_FRAME_TOTOP_RESTOREWHENMIN      ((sal_uInt16)0x0001)
#define SAL_FRAME_TOTOP_FOREGROUNDTASK      ((sal_uInt16)0x0002)
#define SAL_FRAME_TOTOP_GRABFOCUS           ((sal_uInt16)0x0004)
#define SAL_FRAME_TOTOP_GRABFOCUS_ONLY       ((sal_uInt16)0x0008)

#define SAL_FRAME_ENDEXTTEXTINPUT_COMPLETE  ((sal_uInt16)0x0001)
#define SAL_FRAME_ENDEXTTEXTINPUT_CANCEL    ((sal_uInt16)0x0002)


// -----------------
// - SalFrameStyle -
// -----------------

#define SAL_FRAME_STYLE_DEFAULT             ((sal_uLong)0x00000001)
#define SAL_FRAME_STYLE_MOVEABLE            ((sal_uLong)0x00000002)
#define SAL_FRAME_STYLE_SIZEABLE            ((sal_uLong)0x00000004)
#define SAL_FRAME_STYLE_CLOSEABLE           ((sal_uLong)0x00000008)

// no shadow effect on WindowsXP
#define SAL_FRAME_STYLE_NOSHADOW            ((sal_uLong)0x00000010)
// indicate tooltip windows, so they can always be topmost
#define SAL_FRAME_STYLE_TOOLTIP             ((sal_uLong)0x00000020)
// windows without windowmanager decoration, this typically only applies to floating windows
#define SAL_FRAME_STYLE_OWNERDRAWDECORATION ((sal_uLong)0x00000040)
// dialogs
#define SAL_FRAME_STYLE_DIALOG              ((sal_uLong)0x00000080)
// partial fullscreen: fullscreen on one monitor of a multimonitor display
#define SAL_FRAME_STYLE_PARTIAL_FULLSCREEN  ((sal_uLong)0x00800000)
// plugged system child window
#define SAL_FRAME_STYLE_PLUG                ((sal_uLong)0x10000000)
// system child window inside another SalFrame
#define SAL_FRAME_STYLE_SYSTEMCHILD         ((sal_uLong)0x08000000)
// floating window
#define SAL_FRAME_STYLE_FLOAT               ((sal_uLong)0x20000000)
// floating window that needs to be focusable
#define SAL_FRAME_STYLE_FLOAT_FOCUSABLE     ((sal_uLong)0x04000000)
// toolwindows should be painted with a smaller decoration
#define SAL_FRAME_STYLE_TOOLWINDOW          ((sal_uLong)0x40000000)
// the window containing the intro bitmap, aka splashscreen
#define SAL_FRAME_STYLE_INTRO               ((sal_uLong)0x80000000)

// ----------------------------------------
// - extended frame style                 -
// - (sal equivalent to extended WinBits) -
// ----------------------------------------
typedef sal_uInt64 SalExtStyle;
#define SAL_FRAME_EXT_STYLE_DOCUMENT        SalExtStyle(0x00000001)
#define SAL_FRAME_EXT_STYLE_DOCMODIFIED     SalExtStyle(0x00000002)

// ------------------------
// - Flags for SetPosSize -
// ------------------------

#define SAL_FRAME_POSSIZE_X                 ((sal_uInt16)0x0001)
#define SAL_FRAME_POSSIZE_Y                 ((sal_uInt16)0x0002)
#define SAL_FRAME_POSSIZE_WIDTH             ((sal_uInt16)0x0004)
#define SAL_FRAME_POSSIZE_HEIGHT            ((sal_uInt16)0x0008)

#ifdef __cplusplus

using ::rtl::OUString;
using ::rtl::OString;

// ------------
// - SalFrame -
// ------------

struct SystemParentData;

class VCL_PLUGIN_PUBLIC SalFrame : public vcl::DeletionNotifier
{
    // the VCL window corresponding to this frame
    Window*                 m_pWindow;
    SALFRAMEPROC            m_pProc;
public:                     // public for Sal Implementation
    SalFrame() : m_pWindow( NULL ), m_pProc( NULL ) {}
    virtual ~SalFrame();

public:                     // public for Sal Implementation
    SalFrameGeometry        maGeometry;

public:
    // SalGraphics or NULL, but two Graphics for all SalFrames
    // must be returned
    virtual SalGraphics*        GetGraphics() = 0;
    virtual void                ReleaseGraphics( SalGraphics* pGraphics ) = 0;

    // Event must be destroyed, when Frame is destroyed
    // When Event is called, SalInstance::Yield() must be returned
    virtual sal_Bool                PostEvent( void* pData ) = 0;

    virtual void                SetTitle( const rtl::OUString& rTitle ) = 0;
    virtual void                SetIcon( sal_uInt16 nIcon ) = 0;
    virtual void                SetRepresentedURL( const rtl::OUString& );
    virtual void                    SetMenu( SalMenu *pSalMenu ) = 0;
    virtual void                    DrawMenuBar() = 0;

    virtual void                SetExtendedFrameStyle( SalExtStyle nExtStyle ) = 0;

    // Before the window is visible, a resize event
    // must be sent with the correct size
    virtual void                Show( sal_Bool bVisible, sal_Bool bNoActivate = sal_False ) = 0;
    virtual void                Enable( sal_Bool bEnable ) = 0;
    // Set ClientSize and Center the Window to the desktop
    // and send/post a resize message
    virtual void                SetMinClientSize( long nWidth, long nHeight ) = 0;
    virtual void                SetMaxClientSize( long nWidth, long nHeight ) = 0;
    virtual void                SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags ) = 0;
    virtual void                GetClientSize( long& rWidth, long& rHeight ) = 0;
    virtual void                GetWorkArea( Rectangle& rRect ) = 0;
    virtual SalFrame*           GetParent() const = 0;
    // Note: x will be mirrored at parent if UI mirroring is active
    SalFrameGeometry            GetGeometry();
    const SalFrameGeometry&     GetUnmirroredGeometry() const { return maGeometry; }
    virtual void                SetWindowState( const SalFrameState* pState ) = 0;
    virtual sal_Bool                GetWindowState( SalFrameState* pState ) = 0;
    virtual void                ShowFullScreen( sal_Bool bFullScreen, sal_Int32 nDisplay ) = 0;
    // Enable/Disable ScreenSaver, SystemAgents, ...
    virtual void                StartPresentation( sal_Bool bStart ) = 0;
    // Show Window over all other Windows
    virtual void                SetAlwaysOnTop( sal_Bool bOnTop ) = 0;

    // Window to top and grab focus
    virtual void                ToTop( sal_uInt16 nFlags ) = 0;

    // this function can call with the same
    // pointer style
    virtual void                SetPointer( PointerStyle ePointerStyle ) = 0;
    virtual void                CaptureMouse( sal_Bool bMouse ) = 0;
    virtual void                SetPointerPos( long nX, long nY ) = 0;

    // flush output buffer
    virtual void                Flush( void) = 0;
    virtual void                Flush( const Rectangle& );
    // flush output buffer, wait till outstanding operations are done
    virtual void                Sync() = 0;

    virtual void                SetInputContext( SalInputContext* pContext ) = 0;
    virtual void                EndExtTextInput( sal_uInt16 nFlags ) = 0;

    virtual rtl::OUString              GetKeyName( sal_uInt16 nKeyCode ) = 0;

    // returns in 'rKeyCode' the single keycode that translates to the given unicode when using a keyboard layout of language 'aLangType'
    // returns sal_False if no mapping exists or function not supported
    // this is required for advanced menu support
    virtual sal_Bool                MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, KeyCode& rKeyCode ) = 0;

    // returns the input language used for the last key stroke
    // may be LANGUAGE_DONTKNOW if not supported by the OS
    virtual LanguageType        GetInputLanguage() = 0;

    virtual void                UpdateSettings( AllSettings& rSettings ) = 0;

    // returns system data (most prominent: window handle)
    virtual const SystemEnvData*    GetSystemData() const = 0;

    // sets a background bitmap on the frame; the implementation
    // must not make assumptions about the lifetime of the passed SalBitmap
    // but should copy its contents to an own buffer
    virtual void                SetBackgroundBitmap( SalBitmap* ) = 0;


    // get current modifier, button mask and mouse position
    struct SalPointerState
    {
        sal_uLong   mnState;
        Point   maPos;      // in frame coordinates
    };

    virtual SalPointerState     GetPointerState() = 0;

    struct SalIndicatorState
    {
        sal_uInt16  mnState;
    };

    virtual SalIndicatorState   GetIndicatorState() = 0;

    virtual void                SimulateKeyPress( sal_uInt16 nKeyCode ) = 0;

    // set new parent window
    virtual void                SetParent( SalFrame* pNewParent ) = 0;
    // reparent window to act as a plugin; implementation
    // may choose to use a new system window inetrnally
    // return false to indicate failure
    virtual bool                SetPluginParent( SystemParentData* pNewParent ) = 0;

    // move the frame to a new screen
    virtual void                SetScreenNumber( unsigned int nScreen ) = 0;

    virtual void                SetApplicationID( const rtl::OUString &rApplicationID) = 0;

    // shaped system windows
    // set clip region to none (-> rectangular windows, normal state)
    virtual void                    ResetClipRegion() = 0;
    // start setting the clipregion consisting of nRects rectangles
    virtual void                    BeginSetClipRegion( sal_uLong nRects ) = 0;
    // add a rectangle to the clip region
    virtual void                    UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) = 0;
    // done setting up the clipregion
    virtual void                    EndSetClipRegion() = 0;

    // Callbacks (indepent part in vcl/source/window/winproc.cxx)
    // for default message handling return 0
    void                        SetCallback( Window* pWindow, SALFRAMEPROC pProc )
    { m_pWindow = pWindow; m_pProc = pProc; }

    // returns the instance set
    Window*                       GetWindow() const { return m_pWindow; }

    // Call the callback set; this sometimes necessary for implementation classes
    // that should not now more than necessary about the SalFrame implementation
    // (e.g. input methods, printer update handlers).
    long                        CallCallback( sal_uInt16 nEvent, const void* pEvent ) const
    { return m_pProc ? m_pProc( m_pWindow, const_cast<SalFrame*>(this), nEvent, pEvent ) : 0; }
};



#endif // __cplusplus

#endif // _SV_SALFRAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
