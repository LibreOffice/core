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

#include "impdel.hxx"
#include "salwtype.hxx"
#include "salgeom.hxx"

#include <vcl/help.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <vcl/window.hxx>
    // complete vcl::Window for SalFrame::CallCallback under -fsanitize=function

class AllSettings;
class SalGraphics;
class SalBitmap;
class SalMenu;

struct SalFrameState;
struct SalInputContext;
struct SystemEnvData;

// SalFrame types
enum class SalFrameToTop {
    NONE             = 0x00,
    RestoreWhenMin   = 0x01,
    ForegroundTask   = 0x02,
    GrabFocus        = 0x04,
    GrabFocusOnly    = 0x08
};
namespace o3tl {
    template<> struct typed_flags<SalFrameToTop> : is_typed_flags<SalFrameToTop, 0x0f> {};
};

namespace vcl { class KeyCode; }

namespace weld
{
    class Window;
}

enum class FloatWinPopupFlags;

// SalFrame styles
enum class SalFrameStyleFlags
{
    NONE                = 0x00000000,
    DEFAULT             = 0x00000001,
    MOVEABLE            = 0x00000002,
    SIZEABLE            = 0x00000004,
    CLOSEABLE           = 0x00000008,
    // no shadow effect on Windows XP
    NOSHADOW            = 0x00000010,
    // indicate tooltip windows, so they can always be topmost
    TOOLTIP             = 0x00000020,
    // windows without windowmanager decoration, this typically only applies to floating windows
    OWNERDRAWDECORATION = 0x00000040,
    // dialogs
    DIALOG              = 0x00000080,
    // the window containing the intro bitmap, aka splashscreen
    INTRO               = 0x00000100,
    // partial fullscreen: fullscreen on one monitor of a multimonitor display
    PARTIAL_FULLSCREEN  = 0x00800000,
    // system child window inside another SalFrame
    SYSTEMCHILD         = 0x08000000,
    // plugged system child window
    PLUG                = 0x10000000,
    // floating window
    FLOAT               = 0x20000000,
    // toolwindows should be painted with a smaller decoration
    TOOLWINDOW          = 0x40000000,
};

namespace o3tl {
    template<> struct typed_flags<SalFrameStyleFlags> : is_typed_flags<SalFrameStyleFlags, 0x788001ff> {};
};

// Extended frame style (sal equivalent to extended WinBits)
typedef sal_uInt64 SalExtStyle;
#define SAL_FRAME_EXT_STYLE_DOCUMENT        SalExtStyle(0x00000001)
#define SAL_FRAME_EXT_STYLE_DOCMODIFIED     SalExtStyle(0x00000002)

// Flags for SetPosSize
#define SAL_FRAME_POSSIZE_X                 (sal_uInt16(0x0001))
#define SAL_FRAME_POSSIZE_Y                 (sal_uInt16(0x0002))
#define SAL_FRAME_POSSIZE_WIDTH             (sal_uInt16(0x0004))
#define SAL_FRAME_POSSIZE_HEIGHT            (sal_uInt16(0x0008))

struct SystemParentData;
struct ImplSVEvent;

/// A SalFrame is a system window (e.g. an X11 window).
class VCL_PLUGIN_PUBLIC SalFrame
    : public vcl::DeletionNotifier
    , public SalGeometryProvider
{
private:
    // the VCL window corresponding to this frame
    VclPtr<vcl::Window>     m_pWindow;
    SALFRAMEPROC            m_pProc;
protected:
    mutable std::unique_ptr<weld::Window> m_xFrameWeld;
public:
                            SalFrame();
    virtual                 ~SalFrame() override;

    SalFrameGeometry        maGeometry;

    // SalGeometryProvider
    virtual long GetWidth() const override { return maGeometry.nWidth; }
    virtual long GetHeight() const override { return maGeometry.nHeight; }
    virtual bool IsOffScreen() const override { return false; }

    // SalGraphics or NULL, but two Graphics for all SalFrames
    // must be returned
    virtual SalGraphics*    AcquireGraphics() = 0;
    virtual void            ReleaseGraphics( SalGraphics* pGraphics ) = 0;

    // Event must be destroyed, when Frame is destroyed
    // When Event is called, SalInstance::Yield() must be returned
    virtual bool            PostEvent(std::unique_ptr<ImplSVEvent> pData) = 0;

    virtual void            SetTitle( const OUString& rTitle ) = 0;
    virtual void            SetIcon( sal_uInt16 nIcon ) = 0;
    virtual void            SetRepresentedURL( const OUString& );
    virtual void            SetMenu( SalMenu *pSalMenu ) = 0;
    virtual void            DrawMenuBar() = 0;

    virtual void            SetExtendedFrameStyle( SalExtStyle nExtStyle ) = 0;

    // Before the window is visible, a resize event
    // must be sent with the correct size
    virtual void            Show( bool bVisible, bool bNoActivate = false ) = 0;

    // Set ClientSize and Center the Window to the desktop
    // and send/post a resize message
    virtual void            SetMinClientSize( long nWidth, long nHeight ) = 0;
    virtual void            SetMaxClientSize( long nWidth, long nHeight ) = 0;
    virtual void            SetPosSize( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags ) = 0;
    virtual void            GetClientSize( long& rWidth, long& rHeight ) = 0;
    virtual void            GetWorkArea( tools::Rectangle& rRect ) = 0;
    virtual SalFrame*       GetParent() const = 0;
    // Note: x will be mirrored at parent if UI mirroring is active
    SalFrameGeometry        GetGeometry();
    const SalFrameGeometry& GetUnmirroredGeometry() const { return maGeometry; }

    virtual void            SetWindowState( const SalFrameState* pState ) = 0;
    // if this returns false the structure is uninitialised
    [[nodiscard]]
    virtual bool            GetWindowState( SalFrameState* pState ) = 0;
    virtual void            ShowFullScreen( bool bFullScreen, sal_Int32 nDisplay ) = 0;
    virtual void            PositionByToolkit( const tools::Rectangle&, FloatWinPopupFlags ) {};

    // Enable/Disable ScreenSaver, SystemAgents, ...
    virtual void            StartPresentation( bool bStart ) = 0;
    // Show Window over all other Windows
    virtual void            SetAlwaysOnTop( bool bOnTop ) = 0;

    // Window to top and grab focus
    virtual void            ToTop( SalFrameToTop nFlags ) = 0;

    // this function can call with the same
    // pointer style
    virtual void            SetPointer( PointerStyle ePointerStyle ) = 0;
    virtual void            CaptureMouse( bool bMouse ) = 0;
    virtual void            SetPointerPos( long nX, long nY ) = 0;

    // flush output buffer
    virtual void            Flush() = 0;
    virtual void            Flush( const tools::Rectangle& );

    virtual void            SetInputContext( SalInputContext* pContext ) = 0;
    virtual void            EndExtTextInput( EndExtTextInputFlags nFlags ) = 0;

    virtual OUString        GetKeyName( sal_uInt16 nKeyCode ) = 0;

    // returns in 'rKeyCode' the single keycode that translates to the given unicode when using a keyboard layout of language 'aLangType'
    // returns false if no mapping exists or function not supported
    // this is required for advanced menu support
    virtual bool            MapUnicodeToKeyCode( sal_Unicode aUnicode, LanguageType aLangType, vcl::KeyCode& rKeyCode ) = 0;

    // returns the input language used for the last key stroke
    // may be LANGUAGE_DONTKNOW if not supported by the OS
    virtual LanguageType    GetInputLanguage() = 0;

    virtual void            UpdateSettings( AllSettings& rSettings ) = 0;

    virtual void            Beep() = 0;

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

    virtual KeyIndicatorState GetIndicatorState() = 0;

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
    virtual void            BeginSetClipRegion( sal_uInt32 nRects ) = 0;
    // add a rectangle to the clip region
    virtual void            UnionClipRegion( long nX, long nY, long nWidth, long nHeight ) = 0;
    // done setting up the clipregion
    virtual void            EndSetClipRegion() = 0;

    virtual void            SetModal(bool /*bModal*/)
    {
    }

    virtual bool            GetModal() const
    {
        return false;
    }

    // return true to indicate tooltips are shown natively, false otherwise
    virtual bool            ShowTooltip(const OUString& /*rHelpText*/, const tools::Rectangle& /*rHelpArea*/)
    {
        return false;
    }

    // return !0 to indicate popovers are shown natively, 0 otherwise
    virtual void*           ShowPopover(const OUString& /*rHelpText*/, vcl::Window* /*pParent*/, const tools::Rectangle& /*rHelpArea*/, QuickHelpFlags /*nFlags*/)
    {
        return nullptr;
    }

    // return true to indicate popovers are shown natively, false otherwise
    virtual bool            UpdatePopover(void* /*nId*/, const OUString& /*rHelpText*/, vcl::Window* /*pParent*/, const tools::Rectangle& /*rHelpArea*/)
    {
        return false;
    }

    // return true to indicate popovers are shown natively, false otherwise
    virtual bool            HidePopover(void* /*nId*/)
    {
        return false;
    }

    virtual weld::Window*   GetFrameWeld() const;

    // Callbacks (indepent part in vcl/source/window/winproc.cxx)
    // for default message handling return 0
    void                    SetCallback( vcl::Window* pWindow, SALFRAMEPROC pProc );

    // returns the instance set
    vcl::Window*            GetWindow() const { return m_pWindow; }

    // Call the callback set; this sometimes necessary for implementation classes
    // that should not know more than necessary about the SalFrame implementation
    // (e.g. input methods, printer update handlers).
    bool                    CallCallback( SalEvent nEvent, const void* pEvent ) const
        { return m_pProc && m_pProc( m_pWindow, nEvent, pEvent ); }
};

#ifdef _WIN32
bool HasAtHook();
#endif

#endif // INCLUDED_VCL_INC_SALFRAME_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
