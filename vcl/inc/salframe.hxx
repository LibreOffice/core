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

#ifndef INCLUDED_VCL_INC_SALFRAME_HXX
#define INCLUDED_VCL_INC_SALFRAME_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>

#include <vcl/ptrstyle.hxx>

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

// SalFrame types
#define SAL_FRAME_TOTOP_RESTOREWHENMIN      ((sal_uInt16)0x0001)
#define SAL_FRAME_TOTOP_FOREGROUNDTASK      ((sal_uInt16)0x0002)
#define SAL_FRAME_TOTOP_GRABFOCUS           ((sal_uInt16)0x0004)
#define SAL_FRAME_TOTOP_GRABFOCUS_ONLY       ((sal_uInt16)0x0008)

#define SAL_FRAME_ENDEXTTEXTINPUT_COMPLETE  ((sal_uInt16)0x0001)
#define SAL_FRAME_ENDEXTTEXTINPUT_CANCEL    ((sal_uInt16)0x0002)

// SalFrame styles
#define SAL_FRAME_STYLE_DEFAULT             ((sal_uLong)0x00000001)
#define SAL_FRAME_STYLE_MOVEABLE            ((sal_uLong)0x00000002)
#define SAL_FRAME_STYLE_SIZEABLE            ((sal_uLong)0x00000004)
#define SAL_FRAME_STYLE_CLOSEABLE           ((sal_uLong)0x00000008)

// no shadow effect on Windows XP
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

// - extended frame style                 -
// - (sal equivalent to extended WinBits) -
typedef sal_uInt64 SalExtStyle;
#define SAL_FRAME_EXT_STYLE_DOCUMENT        SalExtStyle(0x00000001)
#define SAL_FRAME_EXT_STYLE_DOCMODIFIED     SalExtStyle(0x00000002)

// Flags for SetPosSize
#define SAL_FRAME_POSSIZE_X                 ((sal_uInt16)0x0001)
#define SAL_FRAME_POSSIZE_Y                 ((sal_uInt16)0x0002)
#define SAL_FRAME_POSSIZE_WIDTH             ((sal_uInt16)0x0004)
#define SAL_FRAME_POSSIZE_HEIGHT            ((sal_uInt16)0x0008)

struct SystemParentData;

class VCL_PLUGIN_PUBLIC SalFrame : public vcl::DeletionNotifier
{
    // the VCL window corresponding to this frame
    Window*                 m_pWindow;
    SALFRAMEPROC            m_pProc;

public:
    SalFrame() : m_pWindow( NULL ), m_pProc( NULL ) {}
    virtual ~SalFrame();

    SalFrameGeometry        maGeometry;

    // SalGraphics or NULL, but two Graphics for all SalFrames
    // must be returned
    virtual SalGraphics*    AcquireGraphics() = 0;
    virtual void            ReleaseGraphics( SalGraphics* pGraphics ) = 0;

    // Event must be destroyed, when Frame is destroyed
    // When Event is called, SalInstance::Yield() must be returned
    virtual bool        PostEvent( void* pData ) = 0;

    virtual void            SetTitle( const OUString& rTitle ) = 0;
    virtual void            SetIcon( sal_uInt16 nIcon ) = 0;
    virtual void            SetRepresentedURL( const OUString& );
    virtual void            SetMenu( SalMenu *pSalMenu ) = 0;
    virtual void            DrawMenuBar() = 0;

    virtual void            SetExtendedFrameStyle( SalExtStyle nExtStyle ) = 0;

    // Before the window is visible, a resize event
    // must be sent with the correct size
    virtual void            Show( bool bVisible, bool bNoActivate = false ) = 0;
    virtual void            Enable( bool bEnable ) = 0;
    // Set ClientSize and Center the Window to the desktop
    // and send/post a resize message
    virtual void            SetMinClientSize( long nWidth, long nHeight ) = 0;
    virtual void            SetMaxClientSize( long nWidth, long nHeight ) = 0;
    virtual void            SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags ) = 0;
    virtual void            GetClientSize( long& rWidth, long& rHeight ) = 0;
    virtual void            GetWorkArea( Rectangle& rRect ) = 0;
    virtual SalFrame*       GetParent() const = 0;
    // Note: x will be mirrored at parent if UI mirroring is active
    SalFrameGeometry        GetGeometry();
    const SalFrameGeometry& GetUnmirroredGeometry() const { return maGeometry; }

    virtual void            SetWindowState( const SalFrameState* pState ) = 0;
    virtual bool        GetWindowState( SalFrameState* pState ) = 0;
    virtual void            ShowFullScreen( bool bFullScreen, sal_Int32 nDisplay ) = 0;

    // Enable/Disable ScreenSaver, SystemAgents, ...
    virtual void            StartPresentation( bool bStart ) = 0;
    // Show Window over all other Windows
    virtual void            SetAlwaysOnTop( bool bOnTop ) = 0;

    // Window to top and grab focus
    virtual void            ToTop( sal_uInt16 nFlags ) = 0;

    // this function can call with the same
    // pointer style
    virtual void            SetPointer( PointerStyle ePointerStyle ) = 0;
    virtual void            CaptureMouse( bool bMouse ) = 0;
    virtual void            SetPointerPos( long nX, long nY ) = 0;

    // flush output buffer
    virtual void            Flush( void) = 0;
    virtual void            Flush( const Rectangle& );
    // flush output buffer, wait till outstanding operations are done
    virtual void            Sync() = 0;

    virtual void            SetInputContext( SalInputContext* pContext ) = 0;
    virtual void            EndExtTextInput( sal_uInt16 nFlags ) = 0;

    virtual OUString        GetKeyName( sal_uInt16 nKeyCode ) = 0;

    // returns in 'rKeyCode' the single keycode that translates to the given unicode when using a keyboard layout of language 'aLangType'
    // returns false if no mapping exists or function not supported
    // this is required for advanced menu support
    virtual bool        MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, KeyCode& rKeyCode ) = 0;

    // returns the input language used for the last key stroke
    // may be LANGUAGE_DONTKNOW if not supported by the OS
    virtual LanguageType    GetInputLanguage() = 0;

    virtual void            UpdateSettings( AllSettings& rSettings ) = 0;

    virtual void                Beep() = 0;

    // returns system data (most prominent: window handle)
    virtual const SystemEnvData*
                            GetSystemData() const = 0;

    // get current modifier, button mask and mouse position
    struct SalPointerState
    {
        sal_uLong   mnState;
        Point   maPos;      // in frame coordinates
    };

    virtual SalPointerState GetPointerState() = 0;

    struct SalIndicatorState
    {
        sal_uInt16  mnState;
    };

    virtual SalIndicatorState GetIndicatorState() = 0;

    virtual void            SimulateKeyPress( sal_uInt16 nKeyCode ) = 0;

    // set new parent window
    virtual void            SetParent( SalFrame* pNewParent ) = 0;
    // reparent window to act as a plugin; implementation
    // may choose to use a new system window internally
    // return false to indicate failure
    virtual bool            SetPluginParent( SystemParentData* pNewParent ) = 0;

    // move the frame to a new screen
    virtual void            SetScreenNumber( unsigned int nScreen ) = 0;

    virtual void            SetApplicationID( const OUString &rApplicationID) = 0;

    // shaped system windows
    // set clip region to none (-> rectangular windows, normal state)
    virtual void            ResetClipRegion() = 0;
    // start setting the clipregion consisting of nRects rectangles
    virtual void            BeginSetClipRegion( sal_uLong nRects ) = 0;
    // add a rectangle to the clip region
    virtual void            UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) = 0;
    // done setting up the clipregion
    virtual void            EndSetClipRegion() = 0;

    // Callbacks (indepent part in vcl/source/window/winproc.cxx)
    // for default message handling return 0
    void                    SetCallback( Window* pWindow, SALFRAMEPROC pProc )
        { m_pWindow = pWindow; m_pProc = pProc; }

    // returns the instance set
    Window*                 GetWindow() const
        { return m_pWindow; }

    // Call the callback set; this sometimes necessary for implementation classes
    // that should not know more than necessary about the SalFrame implementation
    // (e.g. input methods, printer update handlers).
    long                    CallCallback( sal_uInt16 nEvent, const void* pEvent ) const
        { return m_pProc ? long(m_pProc( m_pWindow, const_cast<SalFrame*>(this), nEvent, pEvent )) : 0; }
};

#endif // INCLUDED_VCL_INC_SALFRAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
