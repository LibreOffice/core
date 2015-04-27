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

#include <config_features.h>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/outdev.hxx>
#include <tools/resid.hxx>
#include <vcl/pointr.hxx>
#include <tools/wintypes.hxx>
#include <rsc/rsc-vcl-shared-types.hxx>
#include <vcl/apptypes.hxx>
#include <vcl/cursor.hxx>
#include <vcl/inputctx.hxx>
#include <vcl/keycodes.hxx>
#include <vcl/region.hxx>
#include <vcl/salnativewidgets.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ref.hxx>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <memory>
#ifdef DBG_UTIL
#include <typeinfo>
#include <typeindex>
#include <type_traits>
#endif

class VirtualDevice;
struct ImplDelData;
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

namespace com {
namespace sun {
namespace star {
namespace accessibility {
    class XAccessible;
}}}}

namespace com {
namespace sun {
namespace star {
namespace beans {
    struct PropertyValue;
}}}}

namespace com {
namespace sun {
namespace star {
namespace rendering {
    class XCanvas;
    class XSpriteCanvas;
}}}}

namespace com {
namespace sun {
namespace star {
namespace awt {
    class XWindowPeer;
    class XWindow;
}
namespace uno {
    class Any;
    class XInterface;
}
namespace datatransfer {
namespace clipboard {
    class XClipboard;
}

namespace dnd {
    class XDragGestureRecognizer;
    class XDragSource;
    class XDropTarget;
} } } } }

namespace vcl {
    struct ControlLayoutData;
}

namespace svt { class PopupWindowControllerImpl; }

template<class T> class VclPtr;


// - WindowTypes -


// Type fuer GetWindow()
#define WINDOW_PARENT                   ((sal_uInt16)0)
#define WINDOW_FIRSTCHILD               ((sal_uInt16)1)
#define WINDOW_LASTCHILD                ((sal_uInt16)2)
#define WINDOW_PREV                     ((sal_uInt16)3)
#define WINDOW_NEXT                     ((sal_uInt16)4)
#define WINDOW_FIRSTOVERLAP             ((sal_uInt16)5)
#define WINDOW_LASTOVERLAP              ((sal_uInt16)6)
#define WINDOW_OVERLAP                  ((sal_uInt16)7)
#define WINDOW_PARENTOVERLAP            ((sal_uInt16)8)
#define WINDOW_CLIENT                   ((sal_uInt16)9)
#define WINDOW_REALPARENT               ((sal_uInt16)10)
#define WINDOW_FRAME                    ((sal_uInt16)11)
#define WINDOW_BORDER                   ((sal_uInt16)12)
#define WINDOW_FIRSTTOPWINDOWCHILD      ((sal_uInt16)13)
#define WINDOW_LASTTOPWINDOWCHILD       ((sal_uInt16)14)
#define WINDOW_PREVTOPWINDOWSIBLING     ((sal_uInt16)15)
#define WINDOW_NEXTTOPWINDOWSIBLING     ((sal_uInt16)16)

// Flags for setPosSizePixel()
#define WINDOW_POSSIZE_X                ((sal_uInt16)0x0001)
#define WINDOW_POSSIZE_Y                ((sal_uInt16)0x0002)
#define WINDOW_POSSIZE_WIDTH            ((sal_uInt16)0x0004)
#define WINDOW_POSSIZE_HEIGHT           ((sal_uInt16)0x0008)
#define WINDOW_POSSIZE_POS              (WINDOW_POSSIZE_X | WINDOW_POSSIZE_Y)
#define WINDOW_POSSIZE_SIZE             (WINDOW_POSSIZE_WIDTH | WINDOW_POSSIZE_HEIGHT)
#define WINDOW_POSSIZE_POSSIZE          (WINDOW_POSSIZE_POS | WINDOW_POSSIZE_SIZE)
#define WINDOW_POSSIZE_ALL              (WINDOW_POSSIZE_POSSIZE)
#define WINDOW_POSSIZE_DROPDOWN         ((sal_uInt16)0x0010)

// Flags for Show()
#define SHOW_NOPARENTUPDATE             ((sal_uInt16)0x0001)
#define SHOW_NOFOCUSCHANGE              ((sal_uInt16)0x0002)
#define SHOW_NOACTIVATE                 ((sal_uInt16)0x0004)
#define SHOW_FOREGROUNDTASK             ((sal_uInt16)0x0008)

// Flags for SetZOrder()
#define WINDOW_ZORDER_BEFOR             ((sal_uInt16)0x0001)
#define WINDOW_ZORDER_BEHIND            ((sal_uInt16)0x0002)
#define WINDOW_ZORDER_FIRST             ((sal_uInt16)0x0004)
#define WINDOW_ZORDER_LAST              ((sal_uInt16)0x0008)

// Activate-Flags
#define ACTIVATE_MODE_GRABFOCUS         ((sal_uInt16)0x0001)

// ToTop-Flags
#define TOTOP_RESTOREWHENMIN            ((sal_uInt16)0x0001)
#define TOTOP_FOREGROUNDTASK            ((sal_uInt16)0x0002)
#define TOTOP_NOGRABFOCUS               ((sal_uInt16)0x0004)
#define TOTOP_GRABFOCUSONLY             ((sal_uInt16)0x0008)

// Flags for Invalidate
#define INVALIDATE_CHILDREN             ((sal_uInt16)0x0001)
#define INVALIDATE_NOCHILDREN           ((sal_uInt16)0x0002)
#define INVALIDATE_NOERASE              ((sal_uInt16)0x0004)
#define INVALIDATE_UPDATE               ((sal_uInt16)0x0008)
#define INVALIDATE_TRANSPARENT          ((sal_uInt16)0x0010)
#define INVALIDATE_NOTRANSPARENT        ((sal_uInt16)0x0020)
#define INVALIDATE_NOCLIPCHILDREN       ((sal_uInt16)0x4000)
// Temporaer fuer Kompatibilitaet
#define INVALIDATE_BACKGROUND           INVALIDATE_TRANSPARENT

// Flags for Validate
#define VALIDATE_CHILDREN               ((sal_uInt16)0x0001)
#define VALIDATE_NOCHILDREN             ((sal_uInt16)0x0002)

// Flags for Scroll
#define SCROLL_CLIP                     ((sal_uInt16)0x0001)
#define SCROLL_CHILDREN                 ((sal_uInt16)0x0002)
#define SCROLL_NOCHILDREN               ((sal_uInt16)0x0004)
#define SCROLL_NOERASE                  ((sal_uInt16)0x0008)
#define SCROLL_NOINVALIDATE             ((sal_uInt16)0x0010)
#define SCROLL_NOWINDOWINVALIDATE       ((sal_uInt16)0x0020)
#define SCROLL_USECLIPREGION            ((sal_uInt16)0x0040)
#define SCROLL_UPDATE                   ((sal_uInt16)0x0080)

// Flags for ParentClipMode
#define PARENTCLIPMODE_CLIP             ((sal_uInt16)0x0001)
#define PARENTCLIPMODE_NOCLIP           ((sal_uInt16)0x0002)

// Flags for Invert()
#define INVERT_HIGHLIGHT                ((sal_uInt16)0x0001)
#define INVERT_50                       ((sal_uInt16)0x0002)

// Flags for ShowTracking()
#define SHOWTRACK_SMALL                 ((sal_uInt16)0x0001)
#define SHOWTRACK_BIG                   ((sal_uInt16)0x0002)
#define SHOWTRACK_SPLIT                 ((sal_uInt16)0x0003)
#define SHOWTRACK_OBJECT                ((sal_uInt16)0x0004)
#define SHOWTRACK_WINDOW                ((sal_uInt16)0x1000)
#define SHOWTRACK_CLIP                  ((sal_uInt16)0x2000)
#define SHOWTRACK_STYLE                 ((sal_uInt16)0x000F)

// Flags for StartTracking()
#define STARTTRACK_KEYINPUT             ((sal_uInt16)0x0001)
#define STARTTRACK_KEYMOD               ((sal_uInt16)0x0002)
#define STARTTRACK_NOKEYCANCEL          ((sal_uInt16)0x0004)
#define STARTTRACK_SCROLLREPEAT         ((sal_uInt16)0x0008)
#define STARTTRACK_BUTTONREPEAT         ((sal_uInt16)0x0010)
#define STARTTRACK_MOUSEBUTTONDOWN      ((sal_uInt16)0x0020)
#define STARTTRACK_FOCUSCANCEL          ((sal_uInt16)0x0040)

// Flags for StartAutoScroll()
#define AUTOSCROLL_VERT                 ((sal_uInt16)0x0001)
#define AUTOSCROLL_HORZ                 ((sal_uInt16)0x0002)

// Flags for StateChanged()
enum class StateChangedType : sal_uInt16
{
    InitShow           = 1,
    Visible            = 2,
    UpdateMode         = 3,
    Enable             = 4,
    Text               = 5,
    Image              = 6,
    Data               = 7,
    State              = 8,
    Style              = 9,
    Zoom               = 10,
    Border             = 11,
    Transparent        = 12,
    ControlFont        = 13,
    ControlForeground  = 14,
    ControlBackground  = 15,
    ReadOnly           = 16,
    ExtendedStyle      = 17,
    Mirroring          = 18,
    Layout             = 19,
    ControlFocus       = 20,
    User               = 10000
};

// GetFocusFlags
#define GETFOCUS_TAB                    ((sal_uInt16)0x0001)
#define GETFOCUS_CURSOR                 ((sal_uInt16)0x0002)
#define GETFOCUS_MNEMONIC               ((sal_uInt16)0x0004)
#define GETFOCUS_F6                     ((sal_uInt16)0x0008)
#define GETFOCUS_FORWARD                ((sal_uInt16)0x0010)
#define GETFOCUS_BACKWARD               ((sal_uInt16)0x0020)
#define GETFOCUS_AROUND                 ((sal_uInt16)0x0040)
#define GETFOCUS_UNIQUEMNEMONIC         ((sal_uInt16)0x0100)
#define GETFOCUS_INIT                   ((sal_uInt16)0x0200)
#define GETFOCUS_FLOATWIN_POPUPMODEEND_CANCEL ((sal_uInt16)0x0400)


// Draw-Flags fuer Draw()
#define WINDOW_DRAW_MONO                ((sal_uLong)0x00000001)
#define WINDOW_DRAW_NOBORDER            ((sal_uLong)0x00000002)
#define WINDOW_DRAW_NOCONTROLS          ((sal_uLong)0x00000004)
#define WINDOW_DRAW_NODISABLE           ((sal_uLong)0x00000008)
#define WINDOW_DRAW_NOMNEMONIC          ((sal_uLong)0x00000010)
#define WINDOW_DRAW_NOSELECTION         ((sal_uLong)0x00000020)
#define WINDOW_DRAW_NOFOCUS             ((sal_uLong)0x00000040)
#define WINDOW_DRAW_NOBACKGROUND        ((sal_uLong)0x00000080)
#define WINDOW_DRAW_ROLLOVER            ((sal_uLong)0x00000100)

// DialogControl-Flags
#define WINDOW_DLGCTRL_RETURN           ((sal_uInt16)0x0001)
#define WINDOW_DLGCTRL_WANTFOCUS        ((sal_uInt16)0x0002)
#define WINDOW_DLGCTRL_MOD1TAB          ((sal_uInt16)0x0004)
#define WINDOW_DLGCTRL_FLOATWIN_POPUPMODEEND_CANCEL ((sal_uInt16)0x0008)

// GetWindowClipRegionPixel-Flags
#define WINDOW_GETCLIPREGION_NULL       ((sal_uInt16)0x0001)
#define WINDOW_GETCLIPREGION_NOCHILDREN ((sal_uInt16)0x0002)

// EndExtTextInput-Flags
#define EXTTEXTINPUT_END_COMPLETE       ((sal_uInt16)0x0001)
#define EXTTEXTINPUT_END_CANCEL         ((sal_uInt16)0x0002)

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

#define DLGWINDOW_PREV                  0
#define DLGWINDOW_NEXT                  1
#define DLGWINDOW_FIRST                 2


// - Window -


#ifdef DBG_UTIL
const char* ImplDbgCheckWindow( const void* pObj );
#endif

inline bool ImplDoTiledRendering()
{
#if !HAVE_FEATURE_DESKTOP && !defined(ANDROID)
    // We do tiled rendering only for iOS at the moment, actually, but
    // let's see what happens if we assume it for Android, too.
    // (That comment doesn't match what the code does, does it?)
    return true;
#else
    // We need some way to know globally if this process will use
    // tiled rendering or not. Or should this be a per-window setting?
    // Or what?
    return false;
#endif
}



namespace vcl { class Window; }
vcl::Window* ImplFindWindow( const SalFrame* pFrame, Point& rSalFramePos );

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

struct WindowResHeader
{
    sal_uLong nObjMask;
    OString aHelpId;
    sal_uLong nRSStyle;
};

namespace vcl {

typedef OutputDevice RenderContext;

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

    WindowImpl* mpWindowImpl;

#ifdef DBG_UTIL
    friend const char* ::ImplDbgCheckWindow( const void* pObj );
#endif
    friend vcl::Window* ::ImplFindWindow( const SalFrame* pFrame, Point& rSalFramePos );

public:

    DECL_DLLPRIVATE_LINK_TYPED( ImplHandlePaintHdl, Idle*, void );
    DECL_DLLPRIVATE_LINK(      ImplGenerateMouseMoveHdl, void* );
    DECL_DLLPRIVATE_LINK_TYPED( ImplTrackTimerHdl, Timer*, void );
    DECL_DLLPRIVATE_LINK(      ImplAsyncFocusHdl, void* );
    DECL_DLLPRIVATE_LINK_TYPED( ImplHandleResizeTimerHdl, Idle*, void );
    DECL_DLLPRIVATE_LINK(      ImplHideOwnerDrawWindowsHdl, void* );


    SAL_DLLPRIVATE static void          ImplInitAppFontData( vcl::Window* pWindow );

    SAL_DLLPRIVATE vcl::Window*         ImplGetFrameWindow() const;
    SalFrame*                           ImplGetFrame() const;
    SAL_DLLPRIVATE ImplFrameData*       ImplGetFrameData();

    SAL_DLLPRIVATE vcl::Window*         ImplGetWindow();
    SAL_DLLPRIVATE ImplWinData*         ImplGetWinData() const;
    SAL_DLLPRIVATE vcl::Window*         ImplGetClientWindow() const;
    SAL_DLLPRIVATE vcl::Window*         ImplGetDlgWindow( sal_uInt16 n, sal_uInt16 nType, sal_uInt16 nStart = 0, sal_uInt16 nEnd = 0xFFFF, sal_uInt16* pIndex = NULL );
    SAL_DLLPRIVATE vcl::Window*         ImplGetParent() const;
    SAL_DLLPRIVATE vcl::Window*         ImplFindWindow( const Point& rFramePos );

    SAL_DLLPRIVATE void                 ImplInvalidateFrameRegion( const vcl::Region* pRegion, sal_uInt16 nFlags );
    SAL_DLLPRIVATE void                 ImplInvalidateOverlapFrameRegion( const vcl::Region& rRegion );

    SAL_DLLPRIVATE bool                 ImplSetClipFlag( bool bSysObjOnlySmaller = false );

    SAL_DLLPRIVATE bool                 ImplIsWindowOrChild( const vcl::Window* pWindow, bool bSystemWindow = false ) const;
    SAL_DLLPRIVATE bool                 ImplIsChild( const vcl::Window* pWindow, bool bSystemWindow = false ) const;
    SAL_DLLPRIVATE bool                 ImplIsFloatingWindow() const;
    SAL_DLLPRIVATE bool                 ImplIsPushButton() const;
    SAL_DLLPRIVATE bool                 ImplIsSplitter() const;
    SAL_DLLPRIVATE bool                 ImplIsOverlapWindow() const;

    SAL_DLLPRIVATE void                 ImplIsInTaskPaneList( bool mbIsInTaskList );

    SAL_DLLPRIVATE WindowImpl*          ImplGetWindowImpl() const { return mpWindowImpl; }

    SAL_DLLPRIVATE Point                ImplFrameToOutput( const Point& rPos );

    SAL_DLLPRIVATE void                 ImplGrabFocus( sal_uInt16 nFlags );
    SAL_DLLPRIVATE void                 ImplGrabFocusToDocument( sal_uInt16 nFlags );
    SAL_DLLPRIVATE void                 ImplInvertFocus( const Rectangle& rRect );

    SAL_DLLPRIVATE PointerStyle         ImplGetMousePointer() const;
    SAL_DLLPRIVATE void                 ImplCallMouseMove( sal_uInt16 nMouseCode, bool bModChanged = false );
    SAL_DLLPRIVATE void                 ImplGenerateMouseMove();

    SAL_DLLPRIVATE void                 ImplNotifyKeyMouseCommandEventListeners( NotifyEvent& rNEvt );
    SAL_DLLPRIVATE void                 ImplNotifyIconifiedState( bool bIconified );

    SAL_DLLPRIVATE void                 ImplUpdateAll( bool bOverlapWindows = true );

    SAL_DLLPRIVATE void                 ImplDeleteOverlapBackground();

    SAL_DLLPRIVATE void                 ImplControlFocus( sal_uInt16 nFlags = 0 );

    SAL_DLLPRIVATE void                 ImplMirrorFramePos( Point &pt ) const;

    SAL_DLLPRIVATE void                 ImplPosSizeWindow( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags );

    SAL_DLLPRIVATE void                 ImplAddDel( ImplDelData* pDel );
    SAL_DLLPRIVATE void                 ImplRemoveDel( ImplDelData* pDel );

    SAL_DLLPRIVATE void                 ImplCallResize();
    SAL_DLLPRIVATE void                 ImplCallMove();

    SAL_DLLPRIVATE void                 ImplIncModalCount();
    SAL_DLLPRIVATE void                 ImplDecModalCount();

    SAL_DLLPRIVATE static void          ImplCalcSymbolRect( Rectangle& rRect );

protected:

    /** This is intended to be used to clear any locally held references to other Window-subclass objects */
    virtual void                        dispose() SAL_OVERRIDE;

    SAL_DLLPRIVATE void                 ImplInit( vcl::Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData );

    SAL_DLLPRIVATE Point                ImplOutputToFrame( const Point& rPos );

    SAL_DLLPRIVATE void                 ImplInvalidateParentFrameRegion( vcl::Region& rRegion );
    SAL_DLLPRIVATE void                 ImplValidateFrameRegion( const vcl::Region* rRegion, sal_uInt16 nFlags );
    SAL_DLLPRIVATE void                 ImplValidate( const vcl::Region* rRegion, sal_uInt16 nFlags );
    SAL_DLLPRIVATE void                 ImplMoveInvalidateRegion( const Rectangle& rRect, long nHorzScroll, long nVertScroll, bool bChildren );
    SAL_DLLPRIVATE void                 ImplMoveAllInvalidateRegions( const Rectangle& rRect, long nHorzScroll, long nVertScroll, bool bChildren );

    SAL_DLLPRIVATE vcl::Window*         ImplGetBorderWindow() const;

    SAL_DLLPRIVATE void                 ImplInvalidate( const vcl::Region* rRegion, sal_uInt16 nFlags );

    SAL_DLLPRIVATE sal_uInt16           ImplHitTest( const Point& rFramePos );

    SAL_DLLPRIVATE void                 ImplSetMouseTransparent( bool bTransparent );

    SAL_DLLPRIVATE void                 ImplScroll( const Rectangle& rRect, long nHorzScroll, long nVertScroll, sal_uInt16 nFlags );

    SAL_DLLPRIVATE void                 ImplSaveOverlapBackground();
    SAL_DLLPRIVATE bool                 ImplRestoreOverlapBackground( vcl::Region& rInvRegion );
    SAL_DLLPRIVATE void                 ImplInvalidateAllOverlapBackgrounds();

    SAL_DLLPRIVATE bool                 ImplSetClipFlagChildren( bool bSysObjOnlySmaller = false );
    SAL_DLLPRIVATE bool                 ImplSetClipFlagOverlapWindows( bool bSysObjOnlySmaller = false );

    SAL_DLLPRIVATE WinBits              ImplInitRes( const ResId& rResId );
    SAL_DLLPRIVATE WindowResHeader      ImplLoadResHeader( const ResId& rResId );
    SAL_DLLPRIVATE void                 ImplLoadRes( const ResId& rResId );

    SAL_DLLPRIVATE void                 PushPaintHelper(PaintHelper *pHelper);
    SAL_DLLPRIVATE void                 PopPaintHelper(PaintHelper *pHelper);

private:

    SAL_DLLPRIVATE void                 ImplInitWindowData( WindowType nType );

    SAL_DLLPRIVATE void                 getFrameDev( const Point& rPt, const Point& rDevPt, const Size& rDevSize,
                                                     OutputDevice& rOutDev );


    SAL_DLLPRIVATE void                 drawFrameDev( const Point& rPt, const Point& rDevPt, const Size& rDevSize,
                                                      const OutputDevice& rOutDev, const vcl::Region& rRegion );

    SAL_DLLPRIVATE void                 ImplSetFrameParent( const vcl::Window* pParent );

    SAL_DLLPRIVATE void                 ImplInsertWindow( vcl::Window* pParent );
    SAL_DLLPRIVATE void                 ImplRemoveWindow( bool bRemoveFrameData );

    SAL_DLLPRIVATE SalGraphics*         ImplGetFrameGraphics() const;

    SAL_DLLPRIVATE void                 ImplCallFocusChangeActivate( vcl::Window* pNewOverlapWindow, vcl::Window* pOldOverlapWindow );
    SAL_DLLPRIVATE vcl::Window*         ImplGetFirstOverlapWindow();
    SAL_DLLPRIVATE const vcl::Window*   ImplGetFirstOverlapWindow() const;

    SAL_DLLPRIVATE bool                 ImplIsRealParentPath( const vcl::Window* pWindow ) const;

    SAL_DLLPRIVATE bool                 ImplTestMousePointerSet();

    SAL_DLLPRIVATE void                 ImplResetReallyVisible();
    SAL_DLLPRIVATE void                 ImplSetReallyVisible();

    SAL_DLLPRIVATE void                 ImplCallInitShow();

    SAL_DLLPRIVATE void                 ImplInitResolutionSettings();

    SAL_DLLPRIVATE void                 ImplPointToLogic( vcl::Font& rFont ) const;
    SAL_DLLPRIVATE void                 ImplLogicToPoint( vcl::Font& rFont ) const;

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
                                                               bool bChildren, bool bParent, bool bSiblings );

    SAL_DLLPRIVATE void                 ImplCallPaint( const vcl::Region* pRegion, sal_uInt16 nPaintFlags );
    SAL_DLLPRIVATE void                 ImplCallOverlapPaint();
    SAL_DLLPRIVATE void                 ImplPostPaint();

    SAL_DLLPRIVATE void                 ImplUpdateWindowPtr( vcl::Window* pWindow );
    SAL_DLLPRIVATE void                 ImplUpdateWindowPtr();
    SAL_DLLPRIVATE void                 ImplUpdateOverlapWindowPtr( bool bNewFrame );

    SAL_DLLPRIVATE bool                 ImplUpdatePos();
    SAL_DLLPRIVATE void                 ImplUpdateSysObjPos();

    SAL_DLLPRIVATE void                 ImplUpdateGlobalSettings( AllSettings& rSettings, bool bCallHdl = true );

    SAL_DLLPRIVATE void                 ImplAlignChildren();
    SAL_DLLPRIVATE void                 ImplToBottomChild();

    SAL_DLLPRIVATE void                 ImplCalcToTop( ImplCalcToTopData* pPrevData );
    SAL_DLLPRIVATE void                 ImplToTop( sal_uInt16 nFlags );
    SAL_DLLPRIVATE void                 ImplStartToTop( sal_uInt16 nFlags );
    SAL_DLLPRIVATE void                 ImplFocusToTop( sal_uInt16 nFlags, bool bReallyVisible );

    SAL_DLLPRIVATE void                 ImplShowAllOverlaps();
    SAL_DLLPRIVATE void                 ImplHideAllOverlaps();

    SAL_DLLPRIVATE bool                 ImplDlgCtrl( const KeyEvent& rKEvt, bool bKeyInput );
    SAL_DLLPRIVATE bool                 ImplHasDlgCtrl();
    SAL_DLLPRIVATE void                 ImplDlgCtrlNextWindow();
    SAL_DLLPRIVATE void                 ImplDlgCtrlFocusChanged( vcl::Window* pWindow, bool bGetFocus );
    SAL_DLLPRIVATE vcl::Window*         ImplFindDlgCtrlWindow( vcl::Window* pWindow );

    SAL_DLLPRIVATE long                 ImplLogicUnitToPixelX( long nX, MapUnit eUnit );
    SAL_DLLPRIVATE long                 ImplLogicUnitToPixelY( long nY, MapUnit eUnit );

    SAL_DLLPRIVATE bool                 ImplIsWindowInFront( const vcl::Window* pTestWindow ) const;

    SAL_DLLPRIVATE static void          ImplNewInputContext();

    SAL_DLLPRIVATE void                 ImplCallActivateListeners(vcl::Window*);
    SAL_DLLPRIVATE void                 ImplCallDeactivateListeners(vcl::Window*);

    SAL_DLLPRIVATE void                 ImplHandleScroll( ScrollBar* pHScrl, long nX, ScrollBar* pVScrl, long nY );

    SAL_DLLPRIVATE Rectangle            ImplOutputToUnmirroredAbsoluteScreenPixel( const Rectangle& rRect ) const;
    SAL_DLLPRIVATE long                 ImplGetUnmirroredOutOffX();

    // retrieves the list of owner draw decorated windows for this window hiearchy
    SAL_DLLPRIVATE ::std::vector<VclPtr<vcl::Window> >& ImplGetOwnerDrawList();

    SAL_DLLPRIVATE vcl::Window*         ImplGetTopmostFrameWindow();

    SAL_DLLPRIVATE Rectangle            ImplGetWindowExtentsRelative( vcl::Window *pRelativeWindow, bool bClientOnly ) const;

    SAL_DLLPRIVATE bool                 ImplStopDnd();
    SAL_DLLPRIVATE void                 ImplStartDnd();

    SAL_DLLPRIVATE void                 ImplPaintToDevice( ::OutputDevice* pTargetOutDev, const Point& rPos );

    SAL_DLLPRIVATE ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas >
                                        ImplGetCanvas( const Size& rFullscreenSize, bool bFullscreen, bool bSpriteCanvas ) const;

public:
    virtual vcl::Region                 GetActiveClipRegion() const SAL_OVERRIDE;

protected:
    // Single argument ctors shall be explicit.
    explicit                            Window( WindowType nType );

            void                        SetCompoundControl( bool bCompound );

#ifdef DBG_UTIL
    template<typename T>
    inline void                         CallEventListeners( sal_uLong nEvent, T pData,
                                                            typename std::enable_if<std::is_pointer<T>::value >::type* = 0)
    {
        static_assert(!std::is_void<typename std::remove_pointer<T>::type>::value, "no void *");
        CallEventListeners(nEvent, typeid(T), reinterpret_cast<void*>(pData));
    }

    template<typename T>
    inline void                         CallEventListeners( sal_uLong nEvent, T pData,
                                                            typename std::enable_if<std::is_integral<T>::value >::type* = 0)
    { CallEventListeners(nEvent, typeid(T), reinterpret_cast<void*>(sal::static_int_cast<sal_IntPtr>(pData)) ); }

    inline void                         CallEventListeners( sal_uLong nEvent )
    { CallEventListeners(nEvent, typeid(std::nullptr_t), NULL); }

            void                        CallEventListeners( sal_uLong nEvent, std::type_index nTypeIndex, void * pData);
#else
            void                        CallEventListeners( sal_uLong nEvent, void * pData = 0);
#endif
protected:
    static  void                        FireVclEvent( VclSimpleEvent* pEvent );

    virtual bool                        AcquireGraphics() const SAL_OVERRIDE;
    virtual void                        ReleaseGraphics( bool bRelease = true ) SAL_OVERRIDE;

    virtual void                        InitClipRegion() SAL_OVERRIDE;

    // FIXME: this is a hack to workaround missing layout functionality
    SAL_DLLPRIVATE void                 ImplAdjustNWFSizes();

    virtual void                        CopyDeviceArea( SalTwoRect& aPosAry, sal_uInt32 nFlags) SAL_OVERRIDE;
    virtual void                        ClipToPaintRegion( Rectangle& rDstRect ) SAL_OVERRIDE;
    virtual bool                        UsePolyPolygonForComplexGradient() SAL_OVERRIDE;

    virtual void                        DrawGradientWallpaper( long nX, long nY, long nWidth, long nHeight, const Wallpaper& rWallpaper ) SAL_OVERRIDE;

public:
    bool                                HasMirroredGraphics() const SAL_OVERRIDE;

public:
    // Single argument ctors shall be explicit.
    explicit                            Window( vcl::Window* pParent, WinBits nStyle = 0 );

                                        Window( vcl::Window* pParent, const ResId& rResId );
    virtual                             ~Window();

    ::OutputDevice const*               GetOutDev() const;
    ::OutputDevice*                     GetOutDev();

    virtual void                        EnableRTL ( bool bEnable = true ) SAL_OVERRIDE;
    virtual void                        MouseMove( const MouseEvent& rMEvt );
    virtual void                        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void                        MouseButtonUp( const MouseEvent& rMEvt );
    virtual void                        KeyInput( const KeyEvent& rKEvt );
    virtual void                        KeyUp( const KeyEvent& rKEvt );
    virtual void                        PrePaint();
    virtual void                        Paint(vcl::RenderContext& rRenderContext, const Rectangle& rRect);
    virtual void                        Erase() SAL_OVERRIDE;
    virtual void                        Erase( const Rectangle& rRect ) SAL_OVERRIDE { ::OutputDevice::Erase( rRect ); }

    virtual void                        Draw( ::OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
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
    virtual bool                        Notify( NotifyEvent& rNEvt );
    virtual vcl::Window*                GetPreferredKeyInputWindow();

    void                                AddEventListener( const Link<>& rEventListener );
    void                                RemoveEventListener( const Link<>& rEventListener );
    void                                AddChildEventListener( const Link<>& rEventListener );
    void                                RemoveChildEventListener( const Link<>& rEventListener );

    ImplSVEvent *                       PostUserEvent( const Link<>& rLink, void* pCaller = NULL, bool bReferenceLink = false );
    void                                RemoveUserEvent( ImplSVEvent * nUserEvent );

    void                                IncrementLockCount();
    void                                DecrementLockCount();
    bool                                IsLocked( bool bChildren = false ) const;

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

    void                                EnableAllResize( bool bEnable = true );

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
    void                                SetDialogControlFlags( sal_uInt16 nFlags );
    sal_uInt16                          GetDialogControlFlags() const;

    struct PointerState
    {
        sal_uLong   mnState;    // the button state
        Point   maPos;      // mouse position in output coordinates
    };
    PointerState                        GetPointerState();
    bool                                IsMouseOver();

    sal_uLong                           GetCurrentModButtons();

    void                                SetInputContext( const InputContext& rInputContext );
    const InputContext&                 GetInputContext() const;
    void                                EndExtTextInput( sal_uInt16 nFlags );
    void                                SetCursorRect( const Rectangle* pRect = NULL, long nExtTextInputWidth = 0 );
    const Rectangle*                    GetCursorRect() const;
    long                                GetCursorExtTextInputWidth() const;

    void                                SetCompositionCharRect( const Rectangle* pRect, long nCompositionLength, bool bVertical = false );

    using                               ::OutputDevice::SetSettings;
    virtual void                        SetSettings( const AllSettings& rSettings ) SAL_OVERRIDE;
    void                                SetSettings( const AllSettings& rSettings, bool bChild );
    void                                UpdateSettings( const AllSettings& rSettings, bool bChild = false );
    void                                NotifyAllChildren( DataChangedEvent& rDCEvt );

    void                                SetPointFont( const vcl::Font& rFont );
    vcl::Font                           GetPointFont() const;
    void                                SetZoomedPointFont( const vcl::Font& rFont );
    long                                GetDrawPixel( ::OutputDevice* pDev, long nPixels ) const;
    vcl::Font                           GetDrawPixelFont( ::OutputDevice* pDev ) const;

    void                                SetControlFont();
    void                                SetControlFont( const vcl::Font& rFont );
    vcl::Font                           GetControlFont() const;
    bool                                IsControlFont() const;
    void                                SetControlForeground();
    void                                SetControlForeground( const Color& rColor );
    Color                               GetControlForeground() const;
    bool                                IsControlForeground() const;
    void                                SetControlBackground();
    void                                SetControlBackground( const Color& rColor );
    Color                               GetControlBackground() const;
    bool                                IsControlBackground() const;

    void                                SetParentClipMode( sal_uInt16 nMode = 0 );
    sal_uInt16                          GetParentClipMode() const;

    void                                SetWindowRegionPixel();
    void                                SetWindowRegionPixel( const vcl::Region& rRegion );
    const vcl::Region&                  GetWindowRegionPixel() const;
    bool                                IsWindowRegionPixel() const;
    vcl::Region                         GetWindowClipRegionPixel( sal_uInt16 nFlags = 0 ) const;
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

    void                                Show( bool bVisible = true, sal_uInt16 nFlags = 0 );
    void                                Hide() { Show( false ); }
    bool                                IsVisible() const;
    bool                                IsReallyVisible() const;
    bool                                IsReallyShown() const;
    bool                                IsInInitShow() const;

    void                                Enable( bool bEnable = true, bool bChild = true );
    void                                Disable( bool bChild = true ) { Enable( false, bChild ); }
    bool                                IsEnabled() const;

    void                                EnableInput( bool bEnable = true, bool bChild = true );
    void                                EnableInput( bool bEnable, bool bChild, bool bSysWin,
                                                     const vcl::Window* pExcludeWindow = NULL );
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

    /**
     *  Necessary for calc ref input handling from modal dialogs
     */
    bool                                IsInModalNonRefMode() const;

    void                                SetActivateMode( sal_uInt16 nMode );
    sal_uInt16                          GetActivateMode() const;

    void                                ToTop( sal_uInt16 nFlags = 0 );
    void                                SetZOrder( vcl::Window* pRefWindow, sal_uInt16 nFlags );
    void                                EnableAlwaysOnTop( bool bEnable = true );
    bool                                IsAlwaysOnTopEnabled() const;

    virtual void                        setPosSizePixel( long nX, long nY,
                                                         long nWidth, long nHeight,
                                                         sal_uInt16 nFlags = WINDOW_POSSIZE_ALL );
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
    //  window extents including border and decoratrion
    Rectangle                           GetWindowExtentsRelative( vcl::Window *pRelativeWindow ) const;
    // window extents of the client window, coordinates to be used in SetPosPixel
    Rectangle                           GetClientWindowExtentsRelative( vcl::Window *pRelativeWindow ) const;

    bool                                IsScrollable() const;
    virtual void                        Scroll( long nHorzScroll, long nVertScroll,
                                                sal_uInt16 nFlags = 0 );
    void                                Scroll( long nHorzScroll, long nVertScroll,
                                                const Rectangle& rRect, sal_uInt16 nFlags = 0 );
    virtual void                        Invalidate( sal_uInt16 nFlags = 0 );
    virtual void                        Invalidate( const Rectangle& rRect, sal_uInt16 nFlags = 0 );
    virtual void                        Invalidate( const vcl::Region& rRegion, sal_uInt16 nFlags = 0 );
    void                                Validate( sal_uInt16 nFlags = 0 );
    bool                                HasPaintEvent() const;
    void                                Update();
    void                                Flush();
    void                                Sync();

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
    sal_uInt16                          GetGetFocusFlags() const;
    void                                GrabFocusToDocument();

    /**
     * Set this when you need to act as if the window has focus even if it
     * doesn't.  This is necessary for implementing tab stops inside floating
     * windows, but floating windows don't get focus from the system.
     */
    void                                SetFakeFocus( bool bFocus );

    bool                                IsCompoundControl() const;

    static sal_uIntPtr                  SaveFocus();
    static bool                         EndSaveFocus( sal_uIntPtr nSaveId, bool bRestore = true );

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

    void                                SetUniqueId( const OString& );
    const OString&                      GetUniqueId() const;

    vcl::Window*                        FindWindow( const Point& rPos ) const;

    sal_uInt16                          GetChildCount() const;
    vcl::Window*                        GetChild( sal_uInt16 nChild ) const;
    vcl::Window*                        GetWindow( sal_uInt16 nType ) const;
    bool                                IsChild( const vcl::Window* pWindow, bool bSystemWindow = false ) const;
    bool                                IsWindowOrChild( const vcl::Window* pWindow, bool bSystemWindow = false  ) const;

    void                                SetData( void* pNewData );
    void*                               GetData() const;

    /// Add all children to @rAllChildren recursively.
    SAL_DLLPRIVATE void                 CollectChildren(::std::vector<vcl::Window *>& rAllChildren );

    void                                ShowFocus( const Rectangle& rRect );
    void                                HideFocus();

    void                                Invert( const Rectangle& rRect, sal_uInt16 nFlags = 0 );
    void                                Invert( const Polygon& rPoly, sal_uInt16 nFlags = 0 );

    // transparent background for selected or checked items in toolboxes etc.
    void                                DrawSelectionBackground( const Rectangle& rRect, sal_uInt16 highlight, bool bChecked, bool bDrawBorder, bool bDrawExtBorderOnly );
    // the same, but fills a passed Color with a text color complementing the selection background
    void                                DrawSelectionBackground( const Rectangle& rRect, sal_uInt16 highlight, bool bChecked, bool bDrawBorder, bool bDrawExtBorderOnly, Color* pSelectionTextColor );
    // support rounded edges in the selection rect
    void                                DrawSelectionBackground( const Rectangle& rRect, sal_uInt16 highlight, bool bChecked, bool bDrawBorder, bool bDrawExtBorderOnly, long nCornerRadius, Color* pSelectionTextColor, Color* pPaintColor );

    void                                ShowTracking( const Rectangle& rRect,
                                                      sal_uInt16 nFlags = SHOWTRACK_SMALL );
    void                                HideTracking();
    void                                InvertTracking( const Rectangle& rRect,
                                                        sal_uInt16 nFlags = SHOWTRACK_SMALL );
    void                                InvertTracking( const Polygon& rPoly, sal_uInt16 nFlags = 0 );

    void                                StartTracking( sal_uInt16 nFlags = 0 );
    void                                EndTracking( sal_uInt16 nFlags = 0 );
    bool                                IsTracking() const;

    void                                StartAutoScroll( sal_uInt16 nFlags );
    void                                EndAutoScroll();

    bool                                HandleScrollCommand( const CommandEvent& rCmd,
                                                             ScrollBar* pHScrl = NULL,
                                                             ScrollBar* pVScrl = NULL );

    void                                SaveBackground( const Point& rPos, const Size& rSize,
                                                        const Point& rDestOff, VirtualDevice& rSaveDevice );

    const SystemEnvData*                GetSystemData() const;
    ::com::sun::star::uno::Any          GetSystemDataAny() const;

    // API to set/query the component interfaces
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer >
                                        GetComponentInterface( bool bCreate = true );

    void                        SetComponentInterface( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xIFace );

    /** @name Accessibility
     */
    ///@{
public:

    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                                        GetAccessible( bool bCreate = true );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible >
                                        CreateAccessible();

    void                                SetAccessible( ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > );

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

    // Deprecated - can use SetAccessibleRelationLabelFor/By nowadys
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
    SAL_DLLPRIVATE sal_uInt16           ImplGetAccessibleCandidateChildWindowCount( sal_uInt16 nFirstWindowType ) const;
    SAL_DLLPRIVATE vcl::Window*         ImplGetAccessibleCandidateChild( sal_uInt16 nChild, sal_uInt16& rChildCount, sal_uInt16 nFirstWindowType, bool bTopLevel = true ) const;
    SAL_DLLPRIVATE bool                 ImplRegisterAccessibleNativeFrame();
    SAL_DLLPRIVATE void                 ImplRevokeAccessibleNativeFrame();
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
    ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas >
                                        GetCanvas() const;
    /// request XSpriteCanvas render interface for this window
    ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XSpriteCanvas >
                                        GetSpriteCanvas() const;

    /*  records all DrawText operations within the passed rectangle;
     *  a synchronous paint is sent to achieve this
     */
    void                                RecordLayoutData( vcl::ControlLayoutData* pLayout, const Rectangle& rRect );

    // set and retrieve for Toolkit
    VCLXWindow*                         GetWindowPeer() const;
    void                                SetWindowPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xPeer, VCLXWindow* pVCLXWindow );

    // remember if it was generated by Toolkit
    bool                                IsCreatedWithToolkit() const;
    void                                SetCreatedWithToolkit( bool b );

    // Drag and Drop interfaces
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTarget > GetDropTarget();
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSource > GetDragSource();
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragGestureRecognizer > GetDragGestureRecognizer();

    // Clipboard/Selection interfaces
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > GetClipboard();
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > GetPrimarySelection();

    /*
     * Widgets call this to inform their owner container that the widget wants
     * to renegotiate its size. Should be called when a widget has a new size
     * request. e.g. a FixedText Control gets a new label.
     *
     * akin to gtk_widget_queue_resize
     */
    virtual void queue_resize(StateChangedType eReason = StateChangedType::Layout);

    /*
     * Sets the "width-request" property
     *
     * Override for width request of the widget, or -1 if natural request
     * should be used.
     *
     * @see get_preferred_size, set_width_request
     */
    void set_height_request(sal_Int32 nHeightRequest);
    sal_Int32 get_height_request() const;

    /*
     * Sets the "height-request" property
     *
     * Override for height request of the widget, or -1 if natural request
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
    void add_to_size_group(std::shared_ptr<VclSizeGroup> xGroup);
    void remove_from_all_size_groups();

    /*
     * add/remove mnemonic label
     */
    void add_mnemonic_label(FixedText *pLabel);
    void remove_mnemonic_label(FixedText *pLabel);
    std::vector<VclPtr<FixedText> > list_mnemonic_labels() const;

    /*
     * Move this widget to be the nNewPosition'd child of its parent
     */
    void reorderWithinParent(sal_uInt16 nNewPosition);


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
};

}

// Only for compatibility - because many people outside haven't included event.hxx
// These require Window to be defined for VclPtr<Window>
#include <vcl/vclevent.hxx>
#include <vcl/event.hxx>

#endif // INCLUDED_VCL_WINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
