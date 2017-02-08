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

#ifndef INCLUDED_VCL_WINDOW_HXX
#define INCLUDED_VCL_WINDOW_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/outdev.hxx>
#include <tools/resid.hxx>
#include <vcl/pointr.hxx>
#include <tools/wintypes.hxx>
#include <rsc/rsc-vcl-shared-types.hxx>
#include <vcl/inputtypes.hxx>
#include <vcl/cursor.hxx>
#include <vcl/inputctx.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/region.hxx>
#include <vcl/salnativewidgets.hxx>
#include <vcl/uitest/factory.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <memory>

class VirtualDevice;
struct ImplSVEvent;
struct ImplWinData;
struct ImplFrameData;
struct ImplCalcToTopData;
struct SystemEnvData;
struct SystemParentData;
class ImplBorderWindow;
class Idle;
class Timer;
class DockingManager;
class ScrollBar;
class Bitmap;
class FixedText;
class Image;
class MouseEvent;
class KeyEvent;
class CommandEvent;
class TrackingEvent;
class HelpEvent;
class DataChangedEvent;
class VclSimpleEvent;
class NotifyEvent;
class SystemWindow;
class SalFrame;
class MenuFloatingWindow;
class VCLXWindow;
class VclWindowEvent;
enum class ImplPaintFlags;
enum class VclEventId;

namespace com { namespace sun { namespace star {
namespace accessibility {
    class XAccessible;
}
namespace beans {
    struct PropertyValue;
}
namespace rendering {
    class XCanvas;
    class XSpriteCanvas;
}
namespace awt {
    class XWindowPeer;
    class XWindow;
}
namespace uno {
    class Any;
    class XInterface;
}
namespace datatransfer { namespace clipboard {
    class XClipboard;
}
namespace dnd {
    class XDragGestureRecognizer;
    class XDragSource;
    class XDropTarget;
}}}}}

namespace vcl {
    struct ControlLayoutData;
}

namespace svt { class PopupWindowControllerImpl; }

template<class T> class VclPtr;

enum class TrackingEventFlags
{
    NONE           = 0x0000,
    Cancel         = 0x0001,
    Key            = 0x0002,
    Focus          = 0x0004,
    Repeat         = 0x0100,
    End            = 0x1000,
    DontCallHdl    = 0x8000,
};
namespace o3tl
{
    template<> struct typed_flags<TrackingEventFlags> : is_typed_flags<TrackingEventFlags, 0x9107> {};
}


// Type fuer GetWindow()
enum class GetWindowType
{
    Parent                   =  0,
    FirstChild               =  1,
    LastChild                =  2,
    Prev                     =  3,
    Next                     =  4,
    FirstOverlap             =  5,
    Overlap                  =  7,
    ParentOverlap            =  8,
    Client                   =  9,
    RealParent               = 10,
    Frame                    = 11,
    Border                   = 12,
    FirstTopWindowChild      = 13,
    NextTopWindowSibling     = 16,
};

// Flags for setPosSizePixel()
// These must match the definitions in css::awt::PosSize
enum class PosSizeFlags
{
    NONE             = 0x0000,
    X                = 0x0001,
    Y                = 0x0002,
    Width            = 0x0004,
    Height           = 0x0008,
    Pos              = X | Y,
    Size             = Width | Height,
    PosSize          = Pos | Size,
    All              = PosSize,
};

namespace o3tl
{
    template<> struct typed_flags<PosSizeFlags> : is_typed_flags<PosSizeFlags, 0x000f> {};
}

// Flags for Show()
enum class ShowFlags
{
    NONE                       = 0x0000,
    NoParentUpdate             = 0x0001,
    NoFocusChange              = 0x0002,
    NoActivate                 = 0x0004,
    ForegroundTask             = 0x0008,
};
namespace o3tl
{
    template<> struct typed_flags<ShowFlags> : is_typed_flags<ShowFlags, 0x000f> {};
}

// Flags for SetZOrder()
enum class ZOrderFlags
{
    NONE              = 0x0000,
    Before            = 0x0001,
    Behind            = 0x0002,
    First             = 0x0004,
    Last              = 0x0008,
};
namespace o3tl
{
    template<> struct typed_flags<ZOrderFlags> : is_typed_flags<ZOrderFlags, 0x000f> {};
}

// Activate-Flags
enum class ActivateModeFlags
{
    NONE        = 0,
    GrabFocus   = 0x0001,
};
namespace o3tl
{
    template<> struct typed_flags<ActivateModeFlags> : is_typed_flags<ActivateModeFlags, 0x0001> {};
}

// ToTop-Flags
enum class ToTopFlags
{
    NONE            = 0x0000,
    RestoreWhenMin  = 0x0001,
    ForegroundTask  = 0x0002,
    NoGrabFocus     = 0x0004,
    GrabFocusOnly   = 0x0008,
};
namespace o3tl
{
    template<> struct typed_flags<ToTopFlags> : is_typed_flags<ToTopFlags, 0x000f> {};
}

// Flags for Invalidate
// must match css::awt::InvalidateStyle
enum class InvalidateFlags
{
    NONE                 = 0x0000,
    Children             = 0x0001,
    NoChildren           = 0x0002,
    NoErase              = 0x0004,
    Update               = 0x0008,
    Transparent          = 0x0010,
    NoTransparent        = 0x0020,
    NoClipChildren       = 0x4000,
};
namespace o3tl
{
    template<> struct typed_flags<InvalidateFlags> : is_typed_flags<InvalidateFlags, 0x403f> {};
}

// Flags for Validate
enum class ValidateFlags
{
    NONE                = 0x0000,
    Children            = 0x0001,
    NoChildren          = 0x0002
};
namespace o3tl
{
    template<> struct typed_flags<ValidateFlags> : is_typed_flags<ValidateFlags, 0x0003> {};
}

// Flags for Scroll
enum class ScrollFlags
{
    NONE                     = 0x0000,
    Clip                     = 0x0001,
    Children                 = 0x0002,
    NoChildren               = 0x0004,
    UseClipRegion            = 0x0008,
    Update                   = 0x0010,
};
namespace o3tl
{
    template<> struct typed_flags<ScrollFlags> : is_typed_flags<ScrollFlags, 0x001f> {};
}

// Flags for ParentClipMode
enum class ParentClipMode
{
    NONE             = 0x0000,
    Clip             = 0x0001,
    NoClip           = 0x0002,
};
namespace o3tl
{
    template<> struct typed_flags<ParentClipMode> : is_typed_flags<ParentClipMode, 0x0003> {};
}

// Flags for ShowTracking()
enum class ShowTrackFlags {
    NONE                  = 0x0000,
    Small                 = 0x0001,
    Big                   = 0x0002,
    Split                 = 0x0003,
    Object                = 0x0004,
    StyleMask             = 0x000F,
    TrackWindow           = 0x1000,
    Clip                  = 0x2000,
};
namespace o3tl
{
    template<> struct typed_flags<ShowTrackFlags> : is_typed_flags<ShowTrackFlags, 0x300f> {};
}

// Flags for StartTracking()
enum class StartTrackingFlags
{
    NONE                 = 0x0000,
    KeyInput             = 0x0001,
    KeyMod               = 0x0002,
    NoKeyCancel          = 0x0004,
    ScrollRepeat         = 0x0008,
    ButtonRepeat         = 0x0010,
    MouseButtonDown      = 0x0020,
    FocusCancel          = 0x0040,
};

namespace o3tl
{
    template<> struct typed_flags<StartTrackingFlags> : is_typed_flags<StartTrackingFlags, 0x007f> {};
}

// Flags for StartAutoScroll()
enum class StartAutoScrollFlags
{
    NONE                 = 0x0000,
    Vert                 = 0x0001,
    Horz                 = 0x0002,
};
namespace o3tl
{
    template<> struct typed_flags<StartAutoScrollFlags> : is_typed_flags<StartAutoScrollFlags, 0x0003> {};
}

// Flags for StateChanged()
enum class StateChangedType : sal_uInt16
{
    InitShow           = 1,
    Visible            = 2,
    UpdateMode         = 3,
    Enable             = 4,
    Text               = 5,
    Data               = 7,
    State              = 8,
    Style              = 9,
    Zoom               = 10,
    ControlFont        = 13,
    ControlForeground  = 14,
    ControlBackground  = 15,
    ReadOnly           = 16,
    ExtendedStyle      = 17,
    Mirroring          = 18,
    Layout             = 19,
    ControlFocus       = 20
};

// GetFocusFlags
// must match constants in css:awt::FocusChangeReason
enum class GetFocusFlags
{
    NONE                   = 0x0000,
    Tab                    = 0x0001,
    CURSOR                 = 0x0002, // avoid name-clash with X11 #define
    Mnemonic               = 0x0004,
    F6                     = 0x0008,
    Forward                = 0x0010,
    Backward               = 0x0020,
    Around                 = 0x0040,
    UniqueMnemonic         = 0x0100,
    Init                   = 0x0200,
    FloatWinPopupModeEndCancel = 0x0400,
};
namespace o3tl
{
    template<> struct typed_flags<GetFocusFlags> : is_typed_flags<GetFocusFlags, 0x077f> {};
}


// Flags for Draw()
enum class DrawFlags
{
    NONE                = 0x0000,
    Mono                = 0x0001,
    NoBorder            = 0x0002,
    NoControls          = 0x0004,
    NoDisable           = 0x0008,
    NoMnemonic          = 0x0010,
    NoSelection         = 0x0020,
    NoBackground        = 0x0040,
    NoRollover          = 0x0080,
};
namespace o3tl
{
    template<> struct typed_flags<DrawFlags> : is_typed_flags<DrawFlags, 0x00ff> {};
}

// DialogControl-Flags
enum class DialogControlFlags
{
    NONE                       = 0x0000,
    Return                     = 0x0001,
    WantFocus                  = 0x0002,
    FloatWinPopupModeEndCancel = 0x0004,
};
namespace o3tl
{
    template<> struct typed_flags<DialogControlFlags> : is_typed_flags<DialogControlFlags, 0x0007> {};
}

// EndExtTextInput() Flags
enum class EndExtTextInputFlags
{
    NONE           = 0x0000,
    Complete       = 0x0001
};
namespace o3tl
{
    template<> struct typed_flags<EndExtTextInputFlags> : is_typed_flags<EndExtTextInputFlags, 0x0001> {};
}

#define IMPL_MINSIZE_BUTTON_WIDTH       70
#define IMPL_MINSIZE_BUTTON_HEIGHT      22
#define IMPL_EXTRA_BUTTON_WIDTH         18
#define IMPL_EXTRA_BUTTON_HEIGHT        10
#define IMPL_SEP_BUTTON_X               5
#define IMPL_SEP_BUTTON_Y               5
#define IMPL_MINSIZE_MSGBOX_WIDTH       150
#define IMPL_DIALOG_OFFSET              5
#define IMPL_DIALOG_BAR_OFFSET          3
#define IMPL_MSGBOX_OFFSET_EXTRA_X      0
#define IMPL_MSGBOX_OFFSET_EXTRA_Y      2
#define IMPL_SEP_MSGBOX_IMAGE           8

// ImplGetDlgWindow()
enum class GetDlgWindowType
{
    Prev, Next, First
};


#ifdef DBG_UTIL
const char* ImplDbgCheckWindow( const void* pObj );
#endif

namespace vcl { class Window; }
namespace vcl { class Cursor; }
class Dialog;
class WindowImpl;
class PaintHelper;
class VclBuilder;
class VclSizeGroup;
class OutputDevice;
class Application;
class SystemWindow;
class WorkWindow;
class Dialog;
class MessBox;
class DockingWindow;
class FloatingWindow;
class GroupBox;
class PushButton;
class RadioButton;
class SystemChildWindow;
class ImplBorderWindow;
class VclBuilder;
class ImplDockingWindowWrapper;
class ImplPopupFloatWin;
class MenuFloatingWindow;
class LifecycleTest;

namespace svt { class PopupWindowControllerImpl; }
enum class WindowHitTest {
    NONE        = 0x0000,
    Inside      = 0x0001,
    Transparent = 0x0002
};
namespace o3tl {
    template<> struct typed_flags<WindowHitTest> : is_typed_flags<WindowHitTest, 0x0003> {};
};

namespace vcl {

class VCL_DLLPUBLIC RenderTools
{
public:
    // transparent background for selected or checked items in toolboxes etc.
    // + selection Color with a text color complementing the selection background
    // + rounded edge
    static void DrawSelectionBackground(vcl::RenderContext& rRenderContext, vcl::Window& rWindow,
                                        const Rectangle& rRect, sal_uInt16 nHighlight,
                                        bool bChecked, bool bDrawBorder, bool bDrawExtBorderOnly,
                                        Color* pSelectionTextColor = nullptr, long nCornerRadius = 0,
                                        Color* pPaintColor = nullptr);
};

class VCL_DLLPUBLIC Window : public ::OutputDevice, public Resource
{
    friend class ::vcl::Cursor;
    friend class ::OutputDevice;
    friend class ::Application;
    friend class ::SystemWindow;
    friend class ::WorkWindow;
    friend class ::Dialog;
    friend class ::MessBox;
    friend class ::DockingWindow;
    friend class ::FloatingWindow;
    friend class ::GroupBox;
    friend class ::PushButton;
    friend class ::RadioButton;
    friend class ::SystemChildWindow;
    friend class ::ImplBorderWindow;
    friend class ::VclBuilder;
    friend class ::PaintHelper;
    friend class ::LifecycleTest;

    // TODO: improve missing functionality
    // only required because of SetFloatingMode()
    friend class ::ImplDockingWindowWrapper;
    friend class ::ImplPopupFloatWin;
    friend class ::MenuFloatingWindow;

    friend class ::svt::PopupWindowControllerImpl;

private:
    // NOTE: to remove many dependencies of other modules
    //       to this central file, all members are now hidden
    //       in the WindowImpl class and all inline functions
    //       were removed.
    //       (WindowImpl is a pImpl pattern)

    //       Please do *not* add new members or inline functions to class Window,
    //       but use class WindowImpl instead

    std::unique_ptr<WindowImpl> mpWindowImpl;

#ifdef DBG_UTIL
    friend const char* ::ImplDbgCheckWindow( const void* pObj );
#endif

public:

    DECL_DLLPRIVATE_LINK( ImplHandlePaintHdl, Timer*, void );
    DECL_DLLPRIVATE_LINK( ImplGenerateMouseMoveHdl, void*, void );
    DECL_DLLPRIVATE_LINK( ImplTrackTimerHdl, Timer*, void );
    DECL_DLLPRIVATE_LINK( ImplAsyncFocusHdl, void*, void );
    DECL_DLLPRIVATE_LINK( ImplHandleResizeTimerHdl, Timer*, void );


    SAL_DLLPRIVATE static void          ImplInitAppFontData( vcl::Window* pWindow );

    SAL_DLLPRIVATE vcl::Window*         ImplGetFrameWindow() const;
    SalFrame*                           ImplGetFrame() const;
    SAL_DLLPRIVATE ImplFrameData*       ImplGetFrameData();

    SAL_DLLPRIVATE vcl::Window*         ImplGetWindow();
    SAL_DLLPRIVATE ImplWinData*         ImplGetWinData() const;
    SAL_DLLPRIVATE vcl::Window*         ImplGetClientWindow() const;
    SAL_DLLPRIVATE vcl::Window*         ImplGetDlgWindow( sal_uInt16 n, GetDlgWindowType nType, sal_uInt16 nStart = 0, sal_uInt16 nEnd = 0xFFFF, sal_uInt16* pIndex = nullptr );
    SAL_DLLPRIVATE vcl::Window*         ImplGetParent() const;
    SAL_DLLPRIVATE vcl::Window*         ImplFindWindow( const Point& rFramePos );

    SAL_DLLPRIVATE void                 ImplInvalidateFrameRegion( const vcl::Region* pRegion, InvalidateFlags nFlags );
    SAL_DLLPRIVATE void                 ImplInvalidateOverlapFrameRegion( const vcl::Region& rRegion );

    SAL_DLLPRIVATE bool                 ImplSetClipFlag( bool bSysObjOnlySmaller = false );

    SAL_DLLPRIVATE bool                 ImplIsWindowOrChild( const vcl::Window* pWindow, bool bSystemWindow = false ) const;
    SAL_DLLPRIVATE bool                 ImplIsChild( const vcl::Window* pWindow, bool bSystemWindow = false ) const;
    SAL_DLLPRIVATE bool                 ImplIsFloatingWindow() const;
    SAL_DLLPRIVATE bool                 ImplIsPushButton() const;
    SAL_DLLPRIVATE bool                 ImplIsSplitter() const;
    SAL_DLLPRIVATE bool                 ImplIsOverlapWindow() const;

    SAL_DLLPRIVATE void                 ImplIsInTaskPaneList( bool mbIsInTaskList );

    SAL_DLLPRIVATE WindowImpl*          ImplGetWindowImpl() const { return mpWindowImpl.get(); }

    SAL_DLLPRIVATE Point                ImplFrameToOutput( const Point& rPos );

    SAL_DLLPRIVATE void                 ImplGrabFocus( GetFocusFlags nFlags );
    SAL_DLLPRIVATE void                 ImplGrabFocusToDocument( GetFocusFlags nFlags );
    SAL_DLLPRIVATE void                 ImplInvertFocus( const Rectangle& rRect );

    SAL_DLLPRIVATE PointerStyle         ImplGetMousePointer() const;
    SAL_DLLPRIVATE void                 ImplCallMouseMove( sal_uInt16 nMouseCode, bool bModChanged = false );
    SAL_DLLPRIVATE void                 ImplGenerateMouseMove();

    SAL_DLLPRIVATE void                 ImplNotifyKeyMouseCommandEventListeners( NotifyEvent& rNEvt );
    SAL_DLLPRIVATE void                 ImplNotifyIconifiedState( bool bIconified );

    SAL_DLLPRIVATE void                 ImplUpdateAll();

    SAL_DLLPRIVATE void                 ImplControlFocus( GetFocusFlags nFlags = GetFocusFlags::NONE );

    SAL_DLLPRIVATE void                 ImplMirrorFramePos( Point &pt ) const;

    SAL_DLLPRIVATE void                 ImplPosSizeWindow( long nX, long nY, long nWidth, long nHeight, PosSizeFlags nFlags );

    SAL_DLLPRIVATE void                 ImplCallResize();
    SAL_DLLPRIVATE void                 ImplCallMove();

    SAL_DLLPRIVATE void                 ImplIncModalCount();
    SAL_DLLPRIVATE void                 ImplDecModalCount();

    SAL_DLLPRIVATE static void          ImplCalcSymbolRect( Rectangle& rRect );

protected:

    /** This is intended to be used to clear any locally held references to other Window-subclass objects */
    virtual void                        dispose() override;

    SAL_DLLPRIVATE void                 ImplInit( vcl::Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData );

    SAL_DLLPRIVATE Point                ImplOutputToFrame( const Point& rPos );

    SAL_DLLPRIVATE void                 ImplInvalidateParentFrameRegion( vcl::Region& rRegion );
    SAL_DLLPRIVATE void                 ImplValidateFrameRegion( const vcl::Region* rRegion, ValidateFlags nFlags );
    SAL_DLLPRIVATE void                 ImplValidate();
    SAL_DLLPRIVATE void                 ImplMoveInvalidateRegion( const Rectangle& rRect, long nHorzScroll, long nVertScroll, bool bChildren );
    SAL_DLLPRIVATE void                 ImplMoveAllInvalidateRegions( const Rectangle& rRect, long nHorzScroll, long nVertScroll, bool bChildren );

    SAL_DLLPRIVATE vcl::Window*         ImplGetBorderWindow() const;

    SAL_DLLPRIVATE void                 ImplInvalidate( const vcl::Region* rRegion, InvalidateFlags nFlags );

    SAL_DLLPRIVATE WindowHitTest        ImplHitTest( const Point& rFramePos );

    SAL_DLLPRIVATE void                 ImplSetMouseTransparent( bool bTransparent );

    SAL_DLLPRIVATE void                 ImplScroll( const Rectangle& rRect, long nHorzScroll, long nVertScroll, ScrollFlags nFlags );

    SAL_DLLPRIVATE bool                 ImplSetClipFlagChildren( bool bSysObjOnlySmaller );
    SAL_DLLPRIVATE bool                 ImplSetClipFlagOverlapWindows( bool bSysObjOnlySmaller = false );

    SAL_DLLPRIVATE void                 PushPaintHelper(PaintHelper* pHelper, vcl::RenderContext& rRenderContext);
    SAL_DLLPRIVATE void                 PopPaintHelper(PaintHelper* pHelper);

private:

    SAL_DLLPRIVATE void                 ImplSetFrameParent( const vcl::Window* pParent );

    SAL_DLLPRIVATE void                 ImplInsertWindow( vcl::Window* pParent );
    SAL_DLLPRIVATE void                 ImplRemoveWindow( bool bRemoveFrameData );

    SAL_DLLPRIVATE SalGraphics*         ImplGetFrameGraphics() const;

    SAL_DLLPRIVATE static void          ImplCallFocusChangeActivate( vcl::Window* pNewOverlapWindow, vcl::Window* pOldOverlapWindow );
    SAL_DLLPRIVATE vcl::Window*         ImplGetFirstOverlapWindow();
    SAL_DLLPRIVATE const vcl::Window*   ImplGetFirstOverlapWindow() const;

    SAL_DLLPRIVATE bool                 ImplIsRealParentPath( const vcl::Window* pWindow ) const;

    SAL_DLLPRIVATE bool                 ImplTestMousePointerSet();

    SAL_DLLPRIVATE void                 ImplResetReallyVisible();
    SAL_DLLPRIVATE void                 ImplSetReallyVisible();

    SAL_DLLPRIVATE void                 ImplCallInitShow();

    SAL_DLLPRIVATE void                 ImplInitResolutionSettings();

    SAL_DLLPRIVATE void                 ImplPointToLogic(vcl::RenderContext& rRenderContext, vcl::Font& rFont) const;
    SAL_DLLPRIVATE void                 ImplLogicToPoint(vcl::RenderContext& rRenderContext, vcl::Font& rFont) const;

    SAL_DLLPRIVATE bool                 ImplSysObjClip( const vcl::Region* pOldRegion );
    SAL_DLLPRIVATE void                 ImplUpdateSysObjChildrenClip();
    SAL_DLLPRIVATE void                 ImplUpdateSysObjOverlapsClip();
    SAL_DLLPRIVATE void                 ImplUpdateSysObjClip();

    SAL_DLLPRIVATE void                 ImplIntersectWindowClipRegion( vcl::Region& rRegion );
    SAL_DLLPRIVATE void                 ImplIntersectWindowRegion( vcl::Region& rRegion );
    SAL_DLLPRIVATE void                 ImplExcludeWindowRegion( vcl::Region& rRegion );
    SAL_DLLPRIVATE void                 ImplExcludeOverlapWindows( vcl::Region& rRegion );
    SAL_DLLPRIVATE void                 ImplExcludeOverlapWindows2( vcl::Region& rRegion );

    SAL_DLLPRIVATE void                 ImplClipBoundaries( vcl::Region& rRegion, bool bThis, bool bOverlaps );
    SAL_DLLPRIVATE bool                 ImplClipChildren( vcl::Region& rRegion );
    SAL_DLLPRIVATE void                 ImplClipAllChildren( vcl::Region& rRegion );
    SAL_DLLPRIVATE void                 ImplClipSiblings( vcl::Region& rRegion );

    SAL_DLLPRIVATE void                 ImplInitWinClipRegion();
    SAL_DLLPRIVATE void                 ImplInitWinChildClipRegion();
    SAL_DLLPRIVATE vcl::Region*         ImplGetWinChildClipRegion();

    SAL_DLLPRIVATE void                 ImplIntersectAndUnionOverlapWindows( const vcl::Region& rInterRegion, vcl::Region& rRegion );
    SAL_DLLPRIVATE void                 ImplIntersectAndUnionOverlapWindows2( const vcl::Region& rInterRegion, vcl::Region& rRegion );
    SAL_DLLPRIVATE void                 ImplCalcOverlapRegionOverlaps( const vcl::Region& rInterRegion, vcl::Region& rRegion );
    SAL_DLLPRIVATE void                 ImplCalcOverlapRegion( const Rectangle& rSourceRect, vcl::Region& rRegion,
                                                               bool bChildren, bool bSiblings );

    /** Invoke the actual painting.

        This function is kind of recursive - it may be called from the
        PaintHelper destructor; and on the other hand it creates PaintHelper
        that (when destructed) calls other ImplCallPaint()'s.
    */
    SAL_DLLPRIVATE void                 ImplCallPaint(const vcl::Region* pRegion, ImplPaintFlags nPaintFlags);

    SAL_DLLPRIVATE void                 ImplCallOverlapPaint();

    SAL_DLLPRIVATE void                 ImplUpdateWindowPtr( vcl::Window* pWindow );
    SAL_DLLPRIVATE void                 ImplUpdateWindowPtr();
    SAL_DLLPRIVATE void                 ImplUpdateOverlapWindowPtr( bool bNewFrame );

    SAL_DLLPRIVATE bool                 ImplUpdatePos();
    SAL_DLLPRIVATE void                 ImplUpdateSysObjPos();

    SAL_DLLPRIVATE void                 ImplUpdateGlobalSettings( AllSettings& rSettings, bool bCallHdl = true );

    SAL_DLLPRIVATE void                 ImplToBottomChild();

    SAL_DLLPRIVATE void                 ImplCalcToTop( ImplCalcToTopData* pPrevData );
    SAL_DLLPRIVATE void                 ImplToTop( ToTopFlags nFlags );
    SAL_DLLPRIVATE void                 ImplStartToTop( ToTopFlags nFlags );
    SAL_DLLPRIVATE void                 ImplFocusToTop( ToTopFlags nFlags, bool bReallyVisible );

    SAL_DLLPRIVATE void                 ImplShowAllOverlaps();
    SAL_DLLPRIVATE void                 ImplHideAllOverlaps();

    SAL_DLLPRIVATE bool                 ImplDlgCtrl( const KeyEvent& rKEvt, bool bKeyInput );
    SAL_DLLPRIVATE bool                 ImplHasDlgCtrl();
    SAL_DLLPRIVATE void                 ImplDlgCtrlNextWindow();
    SAL_DLLPRIVATE void                 ImplDlgCtrlFocusChanged( vcl::Window* pWindow, bool bGetFocus );
    SAL_DLLPRIVATE vcl::Window*         ImplFindDlgCtrlWindow( vcl::Window* pWindow );

    SAL_DLLPRIVATE static void          ImplNewInputContext();

    SAL_DLLPRIVATE void                 ImplCallActivateListeners(vcl::Window*);
    SAL_DLLPRIVATE void                 ImplCallDeactivateListeners(vcl::Window*);

    SAL_DLLPRIVATE static void          ImplHandleScroll( ScrollBar* pHScrl, long nX, ScrollBar* pVScrl, long nY );

    SAL_DLLPRIVATE Rectangle            ImplOutputToUnmirroredAbsoluteScreenPixel( const Rectangle& rRect ) const;
    SAL_DLLPRIVATE long                 ImplGetUnmirroredOutOffX();

    // retrieves the list of owner draw decorated windows for this window hiearchy
    SAL_DLLPRIVATE ::std::vector<VclPtr<vcl::Window> >& ImplGetOwnerDrawList();

    SAL_DLLPRIVATE vcl::Window*         ImplGetTopmostFrameWindow();

    SAL_DLLPRIVATE Rectangle            ImplGetWindowExtentsRelative( vcl::Window *pRelativeWindow, bool bClientOnly ) const;

    SAL_DLLPRIVATE bool                 ImplStopDnd();
    SAL_DLLPRIVATE void                 ImplStartDnd();

    SAL_DLLPRIVATE void                 ImplPaintToDevice( ::OutputDevice* pTargetOutDev, const Point& rPos );

    SAL_DLLPRIVATE css::uno::Reference< css::rendering::XCanvas >
                                        ImplGetCanvas( bool bSpriteCanvas ) const;

public:
    virtual vcl::Region                 GetActiveClipRegion() const override;

protected:
    // Single argument ctors shall be explicit.
    explicit                            Window( WindowType nType );

            void                        SetCompoundControl( bool bCompound );

            void                        CallEventListeners( VclEventId nEvent, void* pData = nullptr );
    static  void                        FireVclEvent( VclSimpleEvent& rEvent );

    virtual bool                        AcquireGraphics() const override;
    virtual void                        ReleaseGraphics( bool bRelease = true ) override;

    virtual void                        InitClipRegion() override;

    // FIXME: this is a hack to workaround missing layout functionality
    SAL_DLLPRIVATE void                 ImplAdjustNWFSizes();

    virtual void                        CopyDeviceArea( SalTwoRect& aPosAry, bool bWindowInvalidate) override;
    virtual void                        ClipToPaintRegion( Rectangle& rDstRect ) override;
    virtual bool                        UsePolyPolygonForComplexGradient() override;

    virtual void ApplySettings(vcl::RenderContext& rRenderContext);
public:
    bool                                HasMirroredGraphics() const override;

public:
    // Single argument ctors shall be explicit.
    explicit                            Window( vcl::Window* pParent, WinBits nStyle = 0 );

    virtual                             ~Window() override;

    ::OutputDevice const*               GetOutDev() const;
    ::OutputDevice*                     GetOutDev();

    virtual void                        EnableRTL ( bool bEnable = true ) override;
    virtual void                        MouseMove( const MouseEvent& rMEvt );
    virtual void                        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void                        MouseButtonUp( const MouseEvent& rMEvt );
    virtual void                        KeyInput( const KeyEvent& rKEvt );
    virtual void                        KeyUp( const KeyEvent& rKEvt );
    virtual void                        PrePaint(vcl::RenderContext& rRenderContext);
    virtual void                        Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect);
    virtual void                        PostPaint(vcl::RenderContext& rRenderContext);

    using OutputDevice::Erase;
    void                                Erase(vcl::RenderContext& rRenderContext);

    virtual void                        Draw( ::OutputDevice* pDev, const Point& rPos, const Size& rSize, DrawFlags nFlags );
    virtual void                        Move();
    virtual void                        Resize();
    virtual void                        Activate();
    virtual void                        Deactivate();
    virtual void                        GetFocus();
    virtual void                        LoseFocus();
    virtual void                        RequestHelp( const HelpEvent& rHEvt );
    virtual void                        Command( const CommandEvent& rCEvt );
    virtual void                        Tracking( const TrackingEvent& rTEvt );
    virtual void                        StateChanged( StateChangedType nStateChange );
    virtual void                        DataChanged( const DataChangedEvent& rDCEvt );
    virtual bool                        PreNotify( NotifyEvent& rNEvt );
    virtual bool                        EventNotify( NotifyEvent& rNEvt );

    // These methods call the relevant virtual method when not in/post dispose
    void                                CompatGetFocus();
    void                                CompatLoseFocus();
    void                                CompatStateChanged( StateChangedType nStateChange );
    void                                CompatDataChanged( const DataChangedEvent& rDCEvt );
    bool                                CompatPreNotify( NotifyEvent& rNEvt );
    bool                                CompatNotify( NotifyEvent& rNEvt );

    void                                AddEventListener( const Link<VclWindowEvent&,void>& rEventListener );
    void                                RemoveEventListener( const Link<VclWindowEvent&,void>& rEventListener );
    void                                AddChildEventListener( const Link<VclWindowEvent&,void>& rEventListener );
    void                                RemoveChildEventListener( const Link<VclWindowEvent&,void>& rEventListener );

    ImplSVEvent *                       PostUserEvent( const Link<void*,void>& rLink, void* pCaller = nullptr, bool bReferenceLink = false );
    void                                RemoveUserEvent( ImplSVEvent * nUserEvent );

    void                                IncrementLockCount();
    void                                DecrementLockCount();
    bool                                IsLocked() const;

                                        // returns the input language used for the last key stroke
                                        // may be LANGUAGE_DONTKNOW if not supported by the OS
    LanguageType                        GetInputLanguage() const;

    void                                SetStyle( WinBits nStyle );
    WinBits                             GetStyle() const;
    WinBits                             GetPrevStyle() const;
    void                                SetExtendedStyle( WinBits nExtendedStyle );
    WinBits                             GetExtendedStyle() const;
    void                                SetType( WindowType nType );
    WindowType                          GetType() const;
    bool                                IsSystemWindow() const;
    bool                                IsDockingWindow() const;
    bool                                IsDialog() const;
    bool                                IsMenuFloatingWindow() const;
    bool                                IsToolbarFloatingWindow() const;
    bool                                IsTopWindow() const;
    bool                                IsDisposed() const;
    SystemWindow*                       GetSystemWindow() const;

    /// Can the widget derived from this Window do the double-buffering via RenderContext properly?
    bool                                SupportsDoubleBuffering() const;
    /// Enable/disable double-buffering of the frame window and all its children.
    void                                RequestDoubleBuffering(bool bRequest);

    void                                EnableAllResize();

    void                                SetBorderStyle( WindowBorderStyle nBorderStyle );
    WindowBorderStyle                   GetBorderStyle() const;
    void                                GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                                   sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;
    Size                                CalcWindowSize( const Size& rOutSz ) const;
    Size                                CalcOutputSize( const Size& rWinSz ) const;
    long                                CalcTitleWidth() const;

    void                                EnableClipSiblings( bool bClipSiblings = true );

    void                                EnableChildTransparentMode( bool bEnable = true );
    bool                                IsChildTransparentModeEnabled() const;

    void                                SetMouseTransparent( bool bTransparent );
    bool                                IsMouseTransparent() const;
    void                                SetPaintTransparent( bool bTransparent );
    bool                                IsPaintTransparent() const;
    void                                SetDialogControlStart( bool bStart );
    bool                                IsDialogControlStart() const;
    void                                SetDialogControlFlags( DialogControlFlags nFlags );
    DialogControlFlags                  GetDialogControlFlags() const;

    struct PointerState
    {
        sal_uLong   mnState;    // the button state
        Point   maPos;      // mouse position in output coordinates
    };
    PointerState                        GetPointerState();
    bool                                IsMouseOver();

    void                                SetInputContext( const InputContext& rInputContext );
    const InputContext&                 GetInputContext() const;
    void                                EndExtTextInput();
    void                                SetCursorRect( const Rectangle* pRect = nullptr, long nExtTextInputWidth = 0 );
    const Rectangle*                    GetCursorRect() const;
    long                                GetCursorExtTextInputWidth() const;

    void                                SetCompositionCharRect( const Rectangle* pRect, long nCompositionLength, bool bVertical = false );

    using                               ::OutputDevice::SetSettings;
    virtual void                        SetSettings( const AllSettings& rSettings ) override;
    void                                SetSettings( const AllSettings& rSettings, bool bChild );
    void                                UpdateSettings( const AllSettings& rSettings, bool bChild = false );
    void                                NotifyAllChildren( DataChangedEvent& rDCEvt );

    void                                SetPointFont(vcl::RenderContext& rRenderContext, const vcl::Font& rFont);
    vcl::Font                           GetPointFont(vcl::RenderContext& rRenderContext) const;
    void                                SetZoomedPointFont(vcl::RenderContext& rRenderContext, const vcl::Font& rFont);
    long                                GetDrawPixel( ::OutputDevice* pDev, long nPixels ) const;
    vcl::Font                           GetDrawPixelFont( ::OutputDevice* pDev ) const;

    void SetControlFont();
    void SetControlFont( const vcl::Font& rFont );
    vcl::Font GetControlFont() const;
    bool IsControlFont() const;
    void ApplyControlFont(vcl::RenderContext& rRenderContext, const vcl::Font& rDefaultFont);

    void SetControlForeground();
    void SetControlForeground(const Color& rColor);
    const Color& GetControlForeground() const;
    bool IsControlForeground() const;
    void ApplyControlForeground(vcl::RenderContext& rRenderContext, const Color& rDefaultColor);

    void SetControlBackground();
    void SetControlBackground( const Color& rColor );
    const Color& GetControlBackground() const;
    bool IsControlBackground() const;
    void ApplyControlBackground(vcl::RenderContext& rRenderContext, const Color& rDefaultColor);

    void                                SetParentClipMode( ParentClipMode nMode = ParentClipMode::NONE );
    ParentClipMode                      GetParentClipMode() const;

    void                                SetWindowRegionPixel();
    void                                SetWindowRegionPixel( const vcl::Region& rRegion );
    const vcl::Region&                  GetWindowRegionPixel() const;
    bool                                IsWindowRegionPixel() const;
    vcl::Region                         GetWindowClipRegionPixel() const;
    vcl::Region                         GetPaintRegion() const;
    bool                                IsInPaint() const;
    // while IsInPaint returns true ExpandPaintClipRegion adds the
    // submitted region to the paint clip region so you can
    // paint additional parts of your window if necessary
    void                                ExpandPaintClipRegion( const vcl::Region& rRegion );

    void                                SetParent( vcl::Window* pNewParent );
    vcl::Window*                        GetParent() const;
    // return the dialog we are contained in or NULL if un-contained
    Dialog*                             GetParentDialog() const;

    void                                Show( bool bVisible = true, ShowFlags nFlags = ShowFlags::NONE );
    void                                Hide() { Show( false ); }
    bool                                IsVisible() const;
    bool                                IsReallyVisible() const;
    bool                                IsReallyShown() const;
    bool                                IsInInitShow() const;

    void                                Enable( bool bEnable = true, bool bChild = true );
    void                                Disable( bool bChild = true ) { Enable( false, bChild ); }
    bool                                IsEnabled() const;

    void                                EnableInput( bool bEnable = true, bool bChild = true );
    void                                EnableInput( bool bEnable, const vcl::Window* pExcludeWindow );
    bool                                IsInputEnabled() const;

    /** Override <code>EnableInput</code>. This can be necessary due to other people
        using EnableInput for whole window hierarchies.


        <code>AlwaysEnableInput</code> and <code>AlwaysDisableInput</code> are
        mutually exclusive; the last setter wins.
        @param bAlways
        sets always enabled flag

        @param bChild
        if true children are recursively set to AlwaysEnableInput
    */
    void                                AlwaysEnableInput( bool bAlways, bool bChild = true );
    /** returns the current AlwaysEnableInput state
    @return
    true if window is in AlwaysEnableInput state
    */
    bool                                IsAlwaysEnableInput() const;
    /** Override <code>EnableInput</code>, counterpart to AlwaysEnableInput.
        Windows with AlwaysDisableInput will not get key events even if enabled
        and input enabled.This can be necessary due to other people using EnableInput
        for whole window hierarchies.

        <code>AlwaysEnableInput</code> and <code>AlwaysDisableInput</code> are
        mutually exclusive; the last setter wins.

        @param bAlways
        sets always disable flag

        @param bChild
        if true children are recursively set to AlwaysDisableInput
    */
    void                                AlwaysDisableInput( bool bAlways, bool bChild = true );

    /** usually event handlers (see AddEventListener and AddChildEventListener)
    are not called on disabled, modal or input disabled windows. There are however rare cases
    in which one wants a Window or rather one of its Control subclasses to
    not evaluate events but still react to those events externally. In these
    rare cases call SetCallHandlersOnInputDisabled( true ) to have your handler
    called anyway.

    Currently only mouse events get this special treatment.

    Use this sparingly, chances are if you want to use it you're working around
    the real problem.

    @param bCall
    Enable/Disable calling event handlers for this disabled, modal or input disabled window.
    This call is implicity done recursively for possible child windows.
    */
    void                                SetCallHandlersOnInputDisabled( bool bCall );
    /** get state of SetCallHandlersOnInputDisabled

    @returns whether handlers are called regardless of input enabled state
    */
    bool                                IsCallHandlersOnInputDisabled() const;
    /** A window is in modal mode if one of its children or subchildren
        is a running modal window (a modal dialog)

        @returns sal_True if a child or subchild is a running modal window
    */
    bool                                IsInModalMode() const;

    void                                SetActivateMode( ActivateModeFlags nMode );
    ActivateModeFlags                   GetActivateMode() const;

    void                                ToTop( ToTopFlags nFlags = ToTopFlags::NONE );
    void                                SetZOrder( vcl::Window* pRefWindow, ZOrderFlags nFlags );
    void                                EnableAlwaysOnTop( bool bEnable = true );
    bool                                IsAlwaysOnTopEnabled() const;

    virtual void                        setPosSizePixel( long nX, long nY,
                                                         long nWidth, long nHeight,
                                                         PosSizeFlags nFlags = PosSizeFlags::All );
    virtual void                        SetPosPixel( const Point& rNewPos );
    virtual Point                       GetPosPixel() const;
    virtual void                        SetSizePixel( const Size& rNewSize );
    virtual Size                        GetSizePixel() const;
    virtual void                        SetPosSizePixel( const Point& rNewPos,
                                                         const Size& rNewSize );
    virtual void                        SetOutputSizePixel( const Size& rNewSize );
    bool                                IsDefaultPos() const;
    bool                                IsDefaultSize() const;

    // those conversion routines might deliver different results during UI mirroring
    Point                               OutputToScreenPixel( const Point& rPos ) const;
    Point                               ScreenToOutputPixel( const Point& rPos ) const;
    //  the normalized screen methods work independent from UI mirroring
    Point                               OutputToNormalizedScreenPixel( const Point& rPos ) const;
    Point                               NormalizedScreenToOutputPixel( const Point& rPos ) const;
    Point                               OutputToAbsoluteScreenPixel( const Point& rPos ) const;
    Point                               AbsoluteScreenToOutputPixel( const Point& rPos ) const;
    Rectangle                           GetDesktopRectPixel() const;
    //  window extents including border and decoration
    Rectangle                           GetWindowExtentsRelative( vcl::Window *pRelativeWindow ) const;
    // window extents of the client window, coordinates to be used in SetPosPixel
    Rectangle                           GetClientWindowExtentsRelative() const;

    bool                                IsScrollable() const;
    virtual void                        Scroll( long nHorzScroll, long nVertScroll,
                                                ScrollFlags nFlags = ScrollFlags::NONE );
    void                                Scroll( long nHorzScroll, long nVertScroll,
                                                const Rectangle& rRect, ScrollFlags nFlags = ScrollFlags::NONE );
    virtual void                        Invalidate( InvalidateFlags nFlags = InvalidateFlags::NONE );
    virtual void                        Invalidate( const Rectangle& rRect, InvalidateFlags nFlags = InvalidateFlags::NONE );
    virtual void                        Invalidate( const vcl::Region& rRegion, InvalidateFlags nFlags = InvalidateFlags::NONE );
    void                                Validate();
    bool                                HasPaintEvent() const;
    void                                Update();
    void                                Flush();

    // toggles new docking support, enabled via toolkit
    void                                EnableDocking( bool bEnable = true );
    // retrieves the single dockingmanager instance
    static DockingManager*              GetDockingManager();

    void                                EnablePaint( bool bEnable );
    bool                                IsPaintEnabled() const;
    void                                SetUpdateMode( bool bUpdate );
    bool                                IsUpdateMode() const;
    void                                SetParentUpdateMode( bool bUpdate );

    void                                GrabFocus();
    bool                                HasFocus() const;
    bool                                HasChildPathFocus( bool bSystemWindow = false ) const;
    bool                                IsActive() const;
    bool                                HasActiveChildFrame();
    GetFocusFlags                       GetGetFocusFlags() const;
    void                                GrabFocusToDocument();

    /**
     * Set this when you need to act as if the window has focus even if it
     * doesn't.  This is necessary for implementing tab stops inside floating
     * windows, but floating windows don't get focus from the system.
     */
    void                                SetFakeFocus( bool bFocus );

    bool                                IsCompoundControl() const;

    static VclPtr<vcl::Window>          SaveFocus();
    static void                         EndSaveFocus(const VclPtr<vcl::Window>& xFocusWin);

    void                                CaptureMouse();
    void                                ReleaseMouse();
    bool                                IsMouseCaptured() const;

    void                                SetPointer( const Pointer& rPointer );
    const Pointer&                      GetPointer() const;
    void                                EnableChildPointerOverwrite( bool bOverwrite );
    void                                SetPointerPosPixel( const Point& rPos );
    Point                               GetPointerPosPixel();
    Point                               GetLastPointerPosPixel();
    /// Similar to SetPointerPosPixel(), but sets the frame data's last mouse position instead.
    void                                SetLastMousePos(const Point& rPos);
    void                                ShowPointer( bool bVisible );
    void                                EnterWait();
    void                                LeaveWait();
    bool                                IsWait() const;

    void                                SetCursor( vcl::Cursor* pCursor );
    vcl::Cursor*                        GetCursor() const;

    void                                SetZoom( const Fraction& rZoom );
    const Fraction&                     GetZoom() const;
    bool                                IsZoom() const;
    long                                CalcZoom( long n ) const;

    virtual void                        SetText( const OUString& rStr );
    virtual OUString                    GetText() const;
    // return the actual text displayed
    // this may have e.g. accelerators removed or portions
    // replaced by ellipses
    virtual OUString                    GetDisplayText() const;
    // gets the visible background color. for transparent windows
    // this may be the parent's background color; for controls
    // this may be a child's background color (e.g. ListBox)
    virtual const Wallpaper&            GetDisplayBackground() const;

    void                                SetHelpText( const OUString& rHelpText );
    const OUString&                     GetHelpText() const;

    void                                SetQuickHelpText( const OUString& rHelpText );
    const OUString&                     GetQuickHelpText() const;

    void                                SetHelpId( const OString& );
    const OString&                      GetHelpId() const;

    /** String ID of this window for the purpose of creating a screenshot

        In default implementation this ID is the same as HelpId. Override this method
        in windows (dialogs,tabpages) that need different IDs for different configurations
        they can be in

        @return screenshot ID of this window
    */
    virtual OString                     GetScreenshotId() const;

    vcl::Window*                        FindWindow( const Point& rPos ) const;

    sal_uInt16                          GetChildCount() const;
    vcl::Window*                        GetChild( sal_uInt16 nChild ) const;
    vcl::Window*                        GetWindow( GetWindowType nType ) const;
    bool                                IsChild( const vcl::Window* pWindow, bool bSystemWindow = false ) const;
    bool                                IsWindowOrChild( const vcl::Window* pWindow, bool bSystemWindow = false  ) const;

    /// Add all children to rAllChildren recursively.
    SAL_DLLPRIVATE void                 CollectChildren(::std::vector<vcl::Window *>& rAllChildren );

    virtual void                        ShowFocus(const Rectangle& rRect);
    void                                HideFocus();

    // transparent background for selected or checked items in toolboxes etc.
    void                                DrawSelectionBackground( const Rectangle& rRect, sal_uInt16 highlight, bool bChecked, bool bDrawBorder );

    void                                ShowTracking( const Rectangle& rRect,
                                                      ShowTrackFlags nFlags = ShowTrackFlags::Small );
    void                                HideTracking();
    void                                InvertTracking( const Rectangle& rRect, ShowTrackFlags nFlags );
    void                                InvertTracking( const tools::Polygon& rPoly, ShowTrackFlags nFlags );

    void                                StartTracking( StartTrackingFlags nFlags = StartTrackingFlags::NONE );
    void                                EndTracking( TrackingEventFlags nFlags = TrackingEventFlags::NONE );
    bool                                IsTracking() const;

    void                                StartAutoScroll( StartAutoScrollFlags nFlags );
    void                                EndAutoScroll();

    bool                                HandleScrollCommand( const CommandEvent& rCmd,
                                                             ScrollBar* pHScrl,
                                                             ScrollBar* pVScrl );

    void                                SaveBackground( const Point& rPos, const Size& rSize,
                                                        const Point& rDestOff, VirtualDevice& rSaveDevice );

    virtual const SystemEnvData*        GetSystemData() const;
    css::uno::Any                       GetSystemDataAny() const;

    // API to set/query the component interfaces
    virtual css::uno::Reference< css::awt::XWindowPeer >
                                        GetComponentInterface( bool bCreate = true );

    void                        SetComponentInterface( css::uno::Reference< css::awt::XWindowPeer > const & xIFace );

    /** @name Accessibility
     */
    ///@{
public:

    css::uno::Reference< css::accessibility::XAccessible >
                                        GetAccessible( bool bCreate = true );

    virtual css::uno::Reference< css::accessibility::XAccessible >
                                        CreateAccessible();

    void                                SetAccessible( const css::uno::Reference< css::accessibility::XAccessible >& );

    vcl::Window*                        GetAccessibleParentWindow() const;
    sal_uInt16                          GetAccessibleChildWindowCount();
    vcl::Window*                        GetAccessibleChildWindow( sal_uInt16 n );

    void                                SetAccessibleRole( sal_uInt16 nRole );
    sal_uInt16                          GetAccessibleRole() const;

    void                                SetAccessibleName( const OUString& rName );
    OUString                            GetAccessibleName() const;

    void                                SetAccessibleDescription( const OUString& rDescr );
    OUString                            GetAccessibleDescription() const;

    void                                SetAccessibleRelationLabeledBy( vcl::Window* pLabeledBy );
    vcl::Window*                        GetAccessibleRelationLabeledBy() const;

    void                                SetAccessibleRelationLabelFor( vcl::Window* pLabelFor );
    vcl::Window*                        GetAccessibleRelationLabelFor() const;

    void                                SetAccessibleRelationMemberOf( vcl::Window* pMemberOf );
    vcl::Window*                        GetAccessibleRelationMemberOf() const;


    // to avoid sending accessibility events in cases like closing dialogs
    // by default checks complete parent path
    bool                                IsAccessibilityEventsSuppressed( bool bTraverseParentPath = true );
    void                                SetAccessibilityEventsSuppressed(bool bSuppressed);

    // Deprecated - can use SetAccessibleRelationLabelFor/By nowadays
    virtual vcl::Window*                GetParentLabelFor( const vcl::Window* pLabel ) const;
    virtual vcl::Window*                GetParentLabeledBy( const vcl::Window* pLabeled ) const;
    KeyEvent                            GetActivationKey() const;

protected:

    // These eventually are supposed to go when everything is converted to .ui
    SAL_DLLPRIVATE vcl::Window*         getLegacyNonLayoutAccessibleRelationMemberOf() const;
    SAL_DLLPRIVATE vcl::Window*         getLegacyNonLayoutAccessibleRelationLabeledBy() const;
    SAL_DLLPRIVATE vcl::Window*         getLegacyNonLayoutAccessibleRelationLabelFor() const;

    // Let Label override the code part of GetAccessibleRelationLabelFor
    virtual vcl::Window*                getAccessibleRelationLabelFor() const;
    virtual sal_uInt16                  getDefaultAccessibleRole() const;
    virtual OUString                    getDefaultAccessibleName() const;

    /*
     * Advisory Sizing - what is a good size for this widget
     *
     * Retrieves the preferred size of a widget ignoring
     * "width-request" and "height-request" properties.
     *
     * Implement this in sub-classes to tell layout
     * the preferred widget size.
     *
     * Use get_preferred_size to retrieve this value
     * cached and mediated via height and width requests
     */
    virtual Size GetOptimalSize() const;
    /// clear OptimalSize cache
    void InvalidateSizeCache();
private:

    SAL_DLLPRIVATE bool                 ImplIsAccessibleCandidate() const;
    SAL_DLLPRIVATE bool                 ImplIsAccessibleNativeFrame() const;
    ///@}

    /*
     * Retrieves the preferred size of a widget taking
     * into account the "width-request" and "height-request" properties.
     *
     * Overrides the result of GetOptimalSize to honor the
     * width-request and height-request properties.
     *
     * So the same as get_ungrouped_preferred_size except
     * it ignores groups. A building block of get_preferred_size
     * that access the size cache
     *
     * @see get_preferred_size
     */
    Size get_ungrouped_preferred_size() const;
public:
    /// request XCanvas render interface for this window
    css::uno::Reference< css::rendering::XCanvas >
                                        GetCanvas() const;
    /// request XSpriteCanvas render interface for this window
    css::uno::Reference< css::rendering::XSpriteCanvas >
                                        GetSpriteCanvas() const;

    /*  records all DrawText operations within the passed rectangle;
     *  a synchronous paint is sent to achieve this
     */
    void                                RecordLayoutData( vcl::ControlLayoutData* pLayout, const Rectangle& rRect );

    // set and retrieve for Toolkit
    VCLXWindow*                         GetWindowPeer() const;
    void                                SetWindowPeer( css::uno::Reference< css::awt::XWindowPeer > const & xPeer, VCLXWindow* pVCLXWindow );

    // remember if it was generated by Toolkit
    bool                                IsCreatedWithToolkit() const;
    void                                SetCreatedWithToolkit( bool b );

    // Drag and Drop interfaces
    css::uno::Reference< css::datatransfer::dnd::XDropTarget > GetDropTarget();
    css::uno::Reference< css::datatransfer::dnd::XDragSource > GetDragSource();
    css::uno::Reference< css::datatransfer::dnd::XDragGestureRecognizer > GetDragGestureRecognizer();

    // Clipboard/Selection interfaces
    css::uno::Reference< css::datatransfer::clipboard::XClipboard > GetClipboard();
    /// Sets a custom clipboard for the window's frame, instead of creating it on-demand using css::datatransfer::clipboard::SystemClipboard.
    void SetClipboard(css::uno::Reference<css::datatransfer::clipboard::XClipboard> const & xClipboard);
    css::uno::Reference< css::datatransfer::clipboard::XClipboard > GetPrimarySelection();

    /*
     * Widgets call this to inform their owner container that the widget wants
     * to renegotiate its size. Should be called when a widget has a new size
     * request. e.g. a FixedText Control gets a new label.
     *
     * akin to gtk_widget_queue_resize
     */
    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout);

    /*
     * Sets the "height-request" property
     *
     * Override for height request of the widget, or -1 if natural request
     * should be used.
     *
     * @see get_preferred_size, set_width_request
     */
    void set_height_request(sal_Int32 nHeightRequest);
    sal_Int32 get_height_request() const;

    /*
     * Sets the "width-request" property
     *
     * Override for width request of the widget, or -1 if natural request
     * should be used.
     *
     * @see get_preferred_size, set_height_request
     */
    void set_width_request(sal_Int32 nWidthRequest);
    sal_Int32 get_width_request() const;

    /*
     * Retrieves the preferred size of a widget taking
     * into account the "width-request" and "height-request" properties.
     *
     * Overrides the result of GetOptimalSize to honor the
     * width-request and height-request properties.
     *
     * @see GetOptimalSize
     *
     * akin to gtk_widget_get_preferred_size
     */
    Size get_preferred_size() const;

    /*
     * How to horizontally align this widget
     */
    VclAlign get_halign() const;
    void set_halign(VclAlign eAlign);

    /*
     * How to vertically align this widget
     */
    VclAlign get_valign() const;
    void set_valign(VclAlign eAlign);

    /*
     * Whether the widget would like to use any available extra horizontal
     * space.
     */
    bool get_hexpand() const;
    void set_hexpand(bool bExpand);

    /*
     * Whether the widget would like to use any available extra vertical
     * space.
     */
    bool get_vexpand() const;
    void set_vexpand(bool bExpand);

    /*
     * Whether the widget would like to use any available extra space.
     */
    bool get_expand() const;
    void set_expand(bool bExpand);

    /*
     * Whether the widget should receive extra space when the parent grows
     */
    bool get_fill() const;
    void set_fill(bool bFill);

    void set_border_width(sal_Int32 nBorderWidth);
    sal_Int32 get_border_width() const;

    void set_margin_left(sal_Int32 nWidth);
    sal_Int32 get_margin_left() const;

    void set_margin_right(sal_Int32 nWidth);
    sal_Int32 get_margin_right() const;

    void set_margin_top(sal_Int32 nWidth);
    sal_Int32 get_margin_top() const;

    void set_margin_bottom(sal_Int32 nWidth);
    sal_Int32 get_margin_bottom() const;

    /*
     * How the widget is packed with reference to the start or end of the parent
     */
    VclPackType get_pack_type() const;
    void set_pack_type(VclPackType ePackType);

    /*
     * The extra space to put between the widget and its neighbors
     */
    sal_Int32 get_padding() const;
    void set_padding(sal_Int32 nPadding);

    /*
     * The number of columns that the widget spans
     */
    sal_Int32 get_grid_width() const;
    void set_grid_width(sal_Int32 nCols);

    /*
     * The column number to attach the left side of the widget to
     */
    sal_Int32 get_grid_left_attach() const;
    void set_grid_left_attach(sal_Int32 nAttach);

    /*
     * The number of row that the widget spans
     */
    sal_Int32 get_grid_height() const;
    void set_grid_height(sal_Int32 nRows);

    /*
     * The row number to attach the top side of the widget to
     */
    sal_Int32 get_grid_top_attach() const;
    void set_grid_top_attach(sal_Int32 nAttach);

    /*
     * If true this child appears in a secondary layout group of children
     * e.g. help buttons in a buttonbox
     */
    bool get_secondary() const;
    void set_secondary(bool bSecondary);

    /*
     * If true this child is exempted from homogenous sizing
     * e.g. special button in a buttonbox
     */
    bool get_non_homogeneous() const;
    void set_non_homogeneous(bool bNonHomogeneous);

    /*
     * Sets a widget property
     *
     * @return false if property is unknown
     */
    virtual bool set_property(const OString &rKey, const OString &rValue);

    /*
     * Sets a font attribute
     *
     * @return false if attribute is unknown
     */
    bool set_font_attribute(const OString &rKey, const OString &rValue);

    /*
     * Adds this widget to the xGroup VclSizeGroup
     *
     */
    void add_to_size_group(const std::shared_ptr<VclSizeGroup>& xGroup);
    void remove_from_all_size_groups();

    /*
     * add/remove mnemonic label
     */
    void add_mnemonic_label(FixedText *pLabel);
    void remove_mnemonic_label(FixedText *pLabel);
    const std::vector<VclPtr<FixedText> >& list_mnemonic_labels() const;

    /*
     * Move this widget to be the nNewPosition'd child of its parent
     */
    void reorderWithinParent(sal_uInt16 nNewPosition);

    /**
     * Sets an ID.
     */
    void set_id(const OUString& rID);

    /**
     * Get the ID of the window.
     */
    const OUString& get_id() const;


    //  Native Widget Rendering functions


    // form controls must never use native widgets, this can be toggled here
    void    EnableNativeWidget( bool bEnable = true );
    bool    IsNativeWidgetEnabled() const;

    // a helper method for a Control's Draw method
    void PaintToDevice( ::OutputDevice* pDevice, const Point& rPos, const Size& rSize );

    /* mark Window for deletion in top of event queue
    */
    void doLazyDelete();


    //  Keyboard access functions


    /** Query the states of keyboard indicators - Caps Lock, Num Lock and
        Scroll Lock.  Use the following mask to retrieve the state of each
        indicator:

            KeyIndicatorState::CAPS_LOCK
            KeyIndicatorState::NUM_LOCK
            KeyIndicatorState::SCROLL_LOCK
      */
    KeyIndicatorState GetIndicatorState() const;

    void SimulateKeyPress( sal_uInt16 nKeyCode ) const;

    virtual OUString GetSurroundingText() const;
    virtual Selection GetSurroundingTextSelection() const;

    virtual FactoryFunction GetUITestFactory() const;
};

}

// Only for compatibility - because many people outside haven't included event.hxx
// These require Window to be defined for VclPtr<Window>
#include <vcl/vclevent.hxx>
#include <vcl/event.hxx>

#endif // INCLUDED_VCL_WINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
