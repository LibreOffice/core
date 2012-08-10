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

#ifndef _SV_WINDOW_HXX
#define _SV_WINDOW_HXX

#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/outdev.hxx>
#include <tools/resid.hxx>
#include <vcl/pointr.hxx>
#include <tools/wintypes.hxx>
#include <vcl/apptypes.hxx>
#include <vcl/inputctx.hxx>
#include <vcl/vclevent.hxx>
// Only for compatibility - because many people outside haven't included event.hxx
#include <vcl/event.hxx>
#include <vcl/region.hxx>
#include <vcl/salnativewidgets.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <boost/shared_ptr.hpp>

class VirtualDevice;
struct ImplDelData;
struct ImplWinData;
struct ImplOverlapData;
struct ImplFrameData;
struct ImplCalcToTopData;
struct SystemEnvData;
struct SystemParentData;
class ImplBorderWindow;
class VirtualDevice;
class Timer;
class Cursor;
class DockingManager;
class ScrollBar;
class Bitmap;
class Image;
class MouseEvent;
class KeyEvent;
class CommandEvent;
class TrackingEvent;
class HelpEvent;
class DataChangedEvent;
class NotifyEvent;
class SystemWindow;
class SalFrame;
class SalObject;
class MenuFloatingWindow;
class UNOWindowData;
// Nur fuer ExecuteDrag:
struct IDataObject;
class VCLXWindow;
struct ImplAccessibleInfos;

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
    class WindowArranger;
    struct ExtWindowImpl;
}

namespace svt { class PopupWindowControllerImpl; }

// ---------------
// - WindowTypes -
// ---------------

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

// Flags for SetPosSizePixel()
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
typedef sal_uInt16 StateChangedType;
#define STATE_CHANGE_INITSHOW           ((StateChangedType)1)
#define STATE_CHANGE_VISIBLE            ((StateChangedType)2)
#define STATE_CHANGE_UPDATEMODE         ((StateChangedType)3)
#define STATE_CHANGE_ENABLE             ((StateChangedType)4)
#define STATE_CHANGE_TEXT               ((StateChangedType)5)
#define STATE_CHANGE_IMAGE              ((StateChangedType)6)
#define STATE_CHANGE_DATA               ((StateChangedType)7)
#define STATE_CHANGE_STATE              ((StateChangedType)8)
#define STATE_CHANGE_STYLE              ((StateChangedType)9)
#define STATE_CHANGE_ZOOM               ((StateChangedType)10)
#define STATE_CHANGE_BORDER             ((StateChangedType)11)
#define STATE_CHANGE_TRANSPARENT        ((StateChangedType)12)
#define STATE_CHANGE_CONTROLFONT        ((StateChangedType)13)
#define STATE_CHANGE_CONTROLFOREGROUND  ((StateChangedType)14)
#define STATE_CHANGE_CONTROLBACKGROUND  ((StateChangedType)15)
#define STATE_CHANGE_READONLY           ((StateChangedType)16)
#define STATE_CHANGE_EXTENDEDSTYLE      ((StateChangedType)17)
#define STATE_CHANGE_MIRRORING          ((StateChangedType)18)
#define STATE_CHANGE_USER               ((StateChangedType)10000)

// GetFocusFlags
#define GETFOCUS_TAB                    ((sal_uInt16)0x0001)
#define GETFOCUS_CURSOR                 ((sal_uInt16)0x0002)
#define GETFOCUS_MNEMONIC               ((sal_uInt16)0x0004)
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

// Border-Styles fuer SetBorder()
#define WINDOW_BORDER_NORMAL            ((sal_uInt16)0x0001)
#define WINDOW_BORDER_MONO              ((sal_uInt16)0x0002)
#define WINDOW_BORDER_MENU              ((sal_uInt16)0x0010)
#define WINDOW_BORDER_NWF               ((sal_uInt16)0x0020)
#define WINDOW_BORDER_NOBORDER          ((sal_uInt16)0x1000)
#define WINDOW_BORDER_REMOVEBORDER      ((sal_uInt16)0x2000)

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
#define IMPL_MINSIZE_MSGBOX_HEIGHT      80
#define IMPL_DIALOG_OFFSET              5
#define IMPL_DIALOG_BAR_OFFSET          3
#define IMPL_MSGBOX_OFFSET_EXTRA_X      0
#define IMPL_MSGBOX_OFFSET_EXTRA_Y      2
#define IMPL_SEP_MSGBOX_IMAGE           8

#define DLGWINDOW_PREV                  0
#define DLGWINDOW_NEXT                  1
#define DLGWINDOW_FIRST                 2

enum WindowSizeType {
    WINDOWSIZE_MINIMUM,
    WINDOWSIZE_PREFERRED,
    WINDOWSIZE_MAXIMUM
};

// ----------
// - Window -
// ----------

#ifdef DBG_UTIL
const char* ImplDbgCheckWindow( const void* pObj );
#endif

class BitmapEx; // FIXME: really the SetBackgroundBitmap belongs in a toplevel 'window'
class WindowImpl;
class VCL_DLLPUBLIC Window : public OutputDevice
{
    friend class Cursor;
    friend class OutputDevice;
    friend class Application;
    friend class SystemWindow;
    friend class WorkWindow;
    friend class Dialog;
    friend class MessBox;
    friend class DockingWindow;
    friend class FloatingWindow;
    friend class GroupBox;
    friend class PushButton;
    friend class RadioButton;
    friend class SystemChildWindow;
    friend class ImplBorderWindow;

    // TODO: improve missing functionality
    // only required because of SetFloatingMode()
    friend class ImplDockingWindowWrapper;
    friend class ImplPopupFloatWin;
    friend class MenuFloatingWindow;

    friend class svt::PopupWindowControllerImpl;

private:
    // NOTE: to remove many dependencies of other modules
    //       to this central file, all members are now hidden
    //       in the WindowImpl class and all inline functions
    //       were removed
    //
    //       Please do *not* add new members or inline functions to class Window,
    //       but use class WindowImpl instead
    //
    WindowImpl* mpWindowImpl;

    SAL_DLLPRIVATE void ImplInitWindowData( WindowType nType );

#ifdef DBG_UTIL
    friend const char* ImplDbgCheckWindow( const void* pObj );
#endif
    friend Window* ImplFindWindow( const SalFrame* pFrame, Point& rSalFramePos );
public:
    SAL_DLLPRIVATE void                ImplInit( Window* pParent, WinBits nStyle, SystemParentData* pSystemParentData );
    SAL_DLLPRIVATE WinBits             ImplInitRes( const ResId& rResId );
    SAL_DLLPRIVATE void                ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE void                ImplWindowRes( const ResId& rResId );
    SAL_DLLPRIVATE void                ImplSetFrameParent( const Window* pParent );
    SAL_DLLPRIVATE void                ImplInsertWindow( Window* pParent );
    SAL_DLLPRIVATE void                ImplRemoveWindow( sal_Bool bRemoveFrameData );
    SAL_DLLPRIVATE Window*             ImplGetWindow();
    SAL_DLLPRIVATE ImplFrameData*      ImplGetFrameData();
                   SalFrame*           ImplGetFrame() const;
    SAL_DLLPRIVATE ImplWinData*        ImplGetWinData() const;
    SAL_DLLPRIVATE SalGraphics*        ImplGetFrameGraphics() const;
    SAL_DLLPRIVATE void                ImplCallFocusChangeActivate( Window* pNewOverlapWindow, Window* pOldOverlapWindow );
    SAL_DLLPRIVATE Window*             ImplFindWindow( const Point& rFramePos );
    SAL_DLLPRIVATE sal_uInt16              ImplHitTest( const Point& rFramePos );
    SAL_DLLPRIVATE Window*             ImplGetParent() const;
    SAL_DLLPRIVATE Window*             ImplGetClientWindow() const;
    SAL_DLLPRIVATE Window*             ImplGetBorderWindow() const;
    SAL_DLLPRIVATE Window*             ImplGetFirstOverlapWindow();
    SAL_DLLPRIVATE const Window*       ImplGetFirstOverlapWindow() const;
    SAL_DLLPRIVATE Window*             ImplGetFrameWindow() const;
    SAL_DLLPRIVATE sal_Bool                ImplIsRealParentPath( const Window* pWindow ) const;
    SAL_DLLPRIVATE sal_Bool                ImplIsChild( const Window* pWindow, sal_Bool bSystemWindow = sal_False ) const;
    SAL_DLLPRIVATE sal_Bool                ImplIsWindowOrChild( const Window* pWindow, sal_Bool bSystemWindow = sal_False ) const;
    SAL_DLLPRIVATE sal_Bool                ImplIsDockingWindow() const;
    SAL_DLLPRIVATE sal_Bool                ImplIsFloatingWindow() const;
    SAL_DLLPRIVATE sal_Bool                ImplIsSplitter() const;
    SAL_DLLPRIVATE sal_Bool                ImplIsPushButton() const;
    SAL_DLLPRIVATE sal_Bool                ImplIsOverlapWindow() const;
    SAL_DLLPRIVATE void                ImplSetMouseTransparent( sal_Bool bTransparent );
    SAL_DLLPRIVATE int                 ImplTestMousePointerSet();
    SAL_DLLPRIVATE PointerStyle        ImplGetMousePointer() const;
    SAL_DLLPRIVATE void                ImplResetReallyVisible();
    SAL_DLLPRIVATE void                ImplSetReallyVisible();
    SAL_DLLPRIVATE void                ImplCallInitShow();
    SAL_DLLPRIVATE void                ImplAddDel( ImplDelData* pDel );
    SAL_DLLPRIVATE void                ImplRemoveDel( ImplDelData* pDel );
    SAL_DLLPRIVATE void                ImplInitResolutionSettings();
    SAL_DLLPRIVATE void                ImplPointToLogic( Font& rFont ) const;
    SAL_DLLPRIVATE void                ImplLogicToPoint( Font& rFont ) const;
    SAL_DLLPRIVATE Point               ImplOutputToFrame( const Point& rPos );
    SAL_DLLPRIVATE Point               ImplFrameToOutput( const Point& rPos );
    SAL_DLLPRIVATE sal_Bool                ImplSysObjClip( const Region* pOldRegion );
    SAL_DLLPRIVATE void                ImplUpdateSysObjChildrenClip();
    SAL_DLLPRIVATE void                ImplUpdateSysObjOverlapsClip();
    SAL_DLLPRIVATE void                ImplUpdateSysObjClip();
    SAL_DLLPRIVATE sal_Bool                ImplSetClipFlagChildren( sal_Bool bSysObjOnlySmaller = sal_False );
    SAL_DLLPRIVATE sal_Bool                ImplSetClipFlagOverlapWindows( sal_Bool bSysObjOnlySmaller = sal_False );
    SAL_DLLPRIVATE sal_Bool                ImplSetClipFlag( sal_Bool bSysObjOnlySmaller = sal_False );
    SAL_DLLPRIVATE void                ImplIntersectWindowClipRegion( Region& rRegion );
    SAL_DLLPRIVATE void                ImplIntersectWindowRegion( Region& rRegion );
    SAL_DLLPRIVATE void                ImplExcludeWindowRegion( Region& rRegion );
    SAL_DLLPRIVATE void                ImplExcludeOverlapWindows( Region& rRegion );
    SAL_DLLPRIVATE void                ImplExcludeOverlapWindows2( Region& rRegion );
    SAL_DLLPRIVATE void                ImplClipBoundaries( Region& rRegion, sal_Bool bThis, sal_Bool bOverlaps );
    SAL_DLLPRIVATE sal_Bool                ImplClipChildren( Region& rRegion );
    SAL_DLLPRIVATE void                ImplClipAllChildren( Region& rRegion );
    SAL_DLLPRIVATE void                ImplClipSiblings( Region& rRegion );
    SAL_DLLPRIVATE void                ImplInitWinClipRegion();
    SAL_DLLPRIVATE void                ImplInitWinChildClipRegion();
    SAL_DLLPRIVATE Region*             ImplGetWinChildClipRegion();
    SAL_DLLPRIVATE void                ImplIntersectAndUnionOverlapWindows( const Region& rInterRegion, Region& rRegion );
    SAL_DLLPRIVATE void                ImplIntersectAndUnionOverlapWindows2( const Region& rInterRegion, Region& rRegion );
    SAL_DLLPRIVATE void                ImplCalcOverlapRegionOverlaps( const Region& rInterRegion, Region& rRegion );
    SAL_DLLPRIVATE void                ImplCalcOverlapRegion( const Rectangle& rSourceRect, Region& rRegion,
                                               sal_Bool bChildren, sal_Bool bParent, sal_Bool bSiblings );
    SAL_DLLPRIVATE void                ImplCallPaint( const Region* pRegion, sal_uInt16 nPaintFlags );
    SAL_DLLPRIVATE void                ImplCallOverlapPaint();
    SAL_DLLPRIVATE void                ImplPostPaint();
    SAL_DLLPRIVATE void                ImplInvalidateFrameRegion( const Region* pRegion, sal_uInt16 nFlags );
    SAL_DLLPRIVATE void                ImplInvalidateOverlapFrameRegion( const Region& rRegion );
    SAL_DLLPRIVATE void                ImplInvalidateParentFrameRegion( Region& rRegion );
    SAL_DLLPRIVATE void                ImplInvalidate( const Region* rRegion, sal_uInt16 nFlags );
    SAL_DLLPRIVATE void                ImplValidateFrameRegion( const Region* rRegion, sal_uInt16 nFlags );
    SAL_DLLPRIVATE void                ImplValidate( const Region* rRegion, sal_uInt16 nFlags );
    SAL_DLLPRIVATE void                ImplMoveInvalidateRegion( const Rectangle& rRect, long nHorzScroll, long nVertScroll, sal_Bool bChildren );
    SAL_DLLPRIVATE void                ImplMoveAllInvalidateRegions( const Rectangle& rRect, long nHorzScroll, long nVertScroll, sal_Bool bChildren );
    SAL_DLLPRIVATE void                ImplScroll( const Rectangle& rRect, long nHorzScroll, long nVertScroll, sal_uInt16 nFlags );
    SAL_DLLPRIVATE void                ImplUpdateAll( sal_Bool bOverlapWindows = sal_True );
    SAL_DLLPRIVATE void                ImplUpdateWindowPtr( Window* pWindow );
    SAL_DLLPRIVATE void                ImplUpdateWindowPtr();
    SAL_DLLPRIVATE void                ImplUpdateOverlapWindowPtr( sal_Bool bNewFrame );
    SAL_DLLPRIVATE sal_Bool                ImplUpdatePos();
    SAL_DLLPRIVATE void                ImplUpdateSysObjPos();
    SAL_DLLPRIVATE WindowImpl*         ImplGetWindowImpl() const { return mpWindowImpl; }
    SAL_DLLPRIVATE void                ImplFreeExtWindowImpl();
    // creates ExtWindowImpl on demand, but may return NULL (e.g. if mbInDtor)
    SAL_DLLPRIVATE vcl::ExtWindowImpl* ImplGetExtWindowImpl() const;
    SAL_DLLPRIVATE void                ImplDeleteOwnedChildren();
    /** check whether a font is suitable for UI

    The font to be tested will be checked whether it could display a
    localized test string. If this is not the case, then the font
    is deemed unsuitable as UI font.

    @param rFont
    the font to be tested

    @returns
    <TRUE/> if the font can be used as UI font
    <FALSE/> if the font is unsuitable as UI font
     */
    SAL_DLLPRIVATE bool        ImplCheckUIFont( const Font& rFont );
    SAL_DLLPRIVATE void        ImplUpdateGlobalSettings( AllSettings& rSettings, sal_Bool bCallHdl = sal_True );
    SAL_DLLPRIVATE void        ImplAlignChildren();
    SAL_DLLPRIVATE void        ImplPosSizeWindow( long nX, long nY, long nWidth, long nHeight, sal_uInt16 nFlags );
    SAL_DLLPRIVATE void        ImplToBottomChild();
    SAL_DLLPRIVATE void        ImplCalcToTop( ImplCalcToTopData* pPrevData );
    SAL_DLLPRIVATE void        ImplToTop( sal_uInt16 nFlags );
    SAL_DLLPRIVATE void        ImplStartToTop( sal_uInt16 nFlags );
    SAL_DLLPRIVATE void        ImplFocusToTop( sal_uInt16 nFlags, sal_Bool bReallyVisible );
    SAL_DLLPRIVATE void        ImplShowAllOverlaps();
    SAL_DLLPRIVATE void        ImplHideAllOverlaps();
    SAL_DLLPRIVATE void        ImplNotifyKeyMouseCommandEventListeners( NotifyEvent& rNEvt );
    SAL_DLLPRIVATE void        ImplCallMouseMove( sal_uInt16 nMouseCode, sal_Bool bModChanged = sal_False );
    SAL_DLLPRIVATE void        ImplGenerateMouseMove();
    SAL_DLLPRIVATE void        ImplGrabFocus( sal_uInt16 nFlags );
    SAL_DLLPRIVATE void        ImplInvertFocus( const Rectangle& rRect );
    SAL_DLLPRIVATE void        ImplControlFocus( sal_uInt16 nFlags = 0 );
    SAL_DLLPRIVATE Window*     ImplGetDlgWindow( sal_uInt16 n, sal_uInt16 nType, sal_uInt16 nStart = 0, sal_uInt16 nEnd = 0xFFFF, sal_uInt16* pIndex = NULL );
    SAL_DLLPRIVATE sal_Bool        ImplDlgCtrl( const KeyEvent& rKEvt, sal_Bool bKeyInput );
    SAL_DLLPRIVATE sal_Bool        ImplHasDlgCtrl();
    SAL_DLLPRIVATE void        ImplDlgCtrlNextWindow();
    SAL_DLLPRIVATE void        ImplDlgCtrlFocusChanged( Window* pWindow, sal_Bool bGetFocus );
    SAL_DLLPRIVATE Window*     ImplFindDlgCtrlWindow( Window* pWindow );
    SAL_DLLPRIVATE long        ImplLogicUnitToPixelX( long nX, MapUnit eUnit );
    SAL_DLLPRIVATE long        ImplLogicUnitToPixelY( long nY, MapUnit eUnit );
    SAL_DLLPRIVATE sal_Bool        ImplIsWindowInFront( const Window* pTestWindow ) const;
    SAL_DLLPRIVATE void        ImplSaveOverlapBackground();
    SAL_DLLPRIVATE sal_Bool        ImplRestoreOverlapBackground( Region& rInvRegion );
    SAL_DLLPRIVATE void        ImplDeleteOverlapBackground();
    SAL_DLLPRIVATE void        ImplInvalidateAllOverlapBackgrounds();
    SAL_DLLPRIVATE static void ImplNewInputContext();
    SAL_DLLPRIVATE void        ImplCallActivateListeners(Window*);
    SAL_DLLPRIVATE void        ImplCallDeactivateListeners(Window*);
    DECL_DLLPRIVATE_LINK(      ImplHandlePaintHdl, void* );
    DECL_DLLPRIVATE_LINK(      ImplGenerateMouseMoveHdl, void* );
    DECL_DLLPRIVATE_LINK(      ImplTrackTimerHdl, Timer* );
    DECL_DLLPRIVATE_LINK(      ImplAsyncFocusHdl, void* );
    DECL_DLLPRIVATE_LINK(      ImplHideOwnerDrawWindowsHdl, void* );
    DECL_DLLPRIVATE_LINK(      ImplHandleResizeTimerHdl, void* );

    SAL_DLLPRIVATE static void ImplCalcSymbolRect( Rectangle& rRect );
    SAL_DLLPRIVATE void        ImplHandleScroll( ScrollBar* pHScrl, long nX, ScrollBar* pVScrl, long nY );
    SAL_DLLPRIVATE sal_Bool        ImplIsAccessibleCandidate() const;
    SAL_DLLPRIVATE sal_Bool        ImplIsAccessibleNativeFrame() const;
    SAL_DLLPRIVATE sal_uInt16      ImplGetAccessibleCandidateChildWindowCount( sal_uInt16 nFirstWindowType ) const;
    SAL_DLLPRIVATE Window*     ImplGetAccessibleCandidateChild( sal_uInt16 nChild, sal_uInt16& rChildCount, sal_uInt16 nFirstWindowType, sal_Bool bTopLevel = sal_True ) const;
    SAL_DLLPRIVATE sal_Bool        ImplRegisterAccessibleNativeFrame();
    SAL_DLLPRIVATE void        ImplRevokeAccessibleNativeFrame();
    SAL_DLLPRIVATE void        ImplCallResize();
    SAL_DLLPRIVATE void        ImplExtResize();
    SAL_DLLPRIVATE void        ImplCallMove();
    SAL_DLLPRIVATE Rectangle   ImplOutputToUnmirroredAbsoluteScreenPixel( const Rectangle& rRect ) const;
    SAL_DLLPRIVATE void        ImplMirrorFramePos( Point &pt ) const;
    SAL_DLLPRIVATE long        ImplGetUnmirroredOutOffX();
    SAL_DLLPRIVATE void        ImplIncModalCount();
    SAL_DLLPRIVATE void        ImplDecModalCount();

    // retrieves the list of owner draw decorated windows for this window hiearchy
    SAL_DLLPRIVATE ::std::vector<Window *>& ImplGetOwnerDrawList();
    SAL_DLLPRIVATE Window*     ImplGetTopmostFrameWindow();

    SAL_DLLPRIVATE Rectangle   ImplGetWindowExtentsRelative( Window *pRelativeWindow, sal_Bool bClientOnly ) const;
    SAL_DLLPRIVATE void        ImplNotifyIconifiedState( sal_Bool bIconified );
    SAL_DLLPRIVATE bool        ImplStopDnd();
    SAL_DLLPRIVATE void        ImplStartDnd();

    SAL_DLLPRIVATE static void ImplInitAppFontData( Window* pWindow );
    SAL_DLLPRIVATE void        ImplPaintToDevice( OutputDevice* pTargetOutDev, const Point& rPos );

    SAL_DLLPRIVATE void        ImplIsInTaskPaneList( sal_Bool mbIsInTaskList );
    SAL_DLLPRIVATE ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas >
                               ImplGetCanvas( const Size& rFullscreenSize, bool bFullscreen, bool bSpriteCanvas ) const;

private:
    // Default construction is forbidden and not implemented.
    SAL_DLLPRIVATE             Window();

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE             Window (const Window &);
    SAL_DLLPRIVATE             Window & operator= (const Window &);

protected:
    // Single argument ctors shall be explicit.
    explicit            Window( WindowType nType );

            void        SetCompoundControl( sal_Bool bCompound );

            void        ImplCallEventListeners( sal_uLong nEvent, void* pData = NULL );
            void        CallEventListeners( sal_uLong nEvent, void* pData = NULL );
            void        FireVclEvent( VclSimpleEvent* pEvent );

    // FIXME: this is a hack to workaround missing layout functionality
    SAL_DLLPRIVATE void ImplAdjustNWFSizes();
public:
    // Single argument ctors shall be explicit.
    explicit            Window( Window* pParent, WinBits nStyle = 0 );

                        Window( Window* pParent, const ResId& rResId );
    virtual             ~Window();

    virtual void        MouseMove( const MouseEvent& rMEvt );
    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        MouseButtonUp( const MouseEvent& rMEvt );
    virtual void        KeyInput( const KeyEvent& rKEvt );
    virtual void        KeyUp( const KeyEvent& rKEvt );
    virtual void        PrePaint();
    virtual void        Paint( const Rectangle& rRect );

    virtual void        PostPaint();
    virtual void        Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void        Move();
    virtual void        Resize();
    virtual void        Activate();
    virtual void        Deactivate();
    virtual void        GetFocus();
    virtual void        LoseFocus();
    virtual void        RequestHelp( const HelpEvent& rHEvt );
    virtual void        Command( const CommandEvent& rCEvt );
    virtual void        Tracking( const TrackingEvent& rTEvt );
    virtual void        UserEvent( sal_uLong nEvent, void* pEventData );
    virtual void        StateChanged( StateChangedType nStateChange );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
    virtual long        PreNotify( NotifyEvent& rNEvt );
    virtual long        Notify( NotifyEvent& rNEvt );
    virtual Window*     GetPreferredKeyInputWindow();

    /*virtual*/ void    AddEventListener( const Link& rEventListener );
    /*virtual*/ void    RemoveEventListener( const Link& rEventListener );
    /*virtual*/ void    AddChildEventListener( const Link& rEventListener );
    /*virtual*/ void    RemoveChildEventListener( const Link& rEventListener );

    sal_uLong               PostUserEvent( const Link& rLink, void* pCaller = NULL );
    sal_Bool                PostUserEvent( sal_uLong& rEventId, sal_uLong nEvent, void* pEventData = NULL );
    sal_Bool                PostUserEvent( sal_uLong& rEventId, const Link& rLink, void* pCaller = NULL );
    void                RemoveUserEvent( sal_uLong nUserEvent );

    void                IncrementLockCount();
    void                DecrementLockCount();
    sal_Bool                IsLocked( sal_Bool bChildren = sal_False ) const;

                        // returns the input language used for the last key stroke
                        // may be LANGUAGE_DONTKNOW if not supported by the OS
    LanguageType        GetInputLanguage() const;

    void                SetStyle( WinBits nStyle );
    WinBits             GetStyle() const;
    WinBits             GetPrevStyle() const;
    void                SetExtendedStyle( WinBits nExtendedStyle );
    WinBits             GetExtendedStyle() const;
    void                SetType( WindowType nType );
    WindowType          GetType() const;
    sal_Bool                IsSystemWindow() const;
    sal_Bool                IsDialog() const;
    sal_Bool                IsMenuFloatingWindow() const;
    sal_Bool                IsToolbarFloatingWindow() const;
    sal_Bool                IsTopWindow() const;
    SystemWindow*       GetSystemWindow() const;

    void                EnableAllResize( sal_Bool bEnable = sal_True );

    void                SetBorderStyle( sal_uInt16 nBorderStyle );
    sal_uInt16              GetBorderStyle() const;
    void                GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                   sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;
    Size                CalcWindowSize( const Size& rOutSz ) const;
    Size                CalcOutputSize( const Size& rWinSz ) const;
    long                CalcTitleWidth() const;

    void                EnableClipSiblings( sal_Bool bClipSiblings = sal_True );

    void                EnableChildTransparentMode( sal_Bool bEnable = sal_True );
    sal_Bool                IsChildTransparentModeEnabled() const;

    void                SetMouseTransparent( sal_Bool bTransparent );
    sal_Bool                IsMouseTransparent() const;
    void                SetPaintTransparent( sal_Bool bTransparent );
    sal_Bool                IsPaintTransparent() const;
    void                SetDialogControlStart( sal_Bool bStart );
    sal_Bool                IsDialogControlStart() const;
    void                SetDialogControlFlags( sal_uInt16 nFlags );
    sal_uInt16              GetDialogControlFlags() const;

    struct PointerState
    {
        sal_uLong   mnState;    // the button state
        Point   maPos;      // mouse position in output coordinates
    };
    PointerState        GetPointerState();
    sal_Bool                IsMouseOver();

    sal_uLong               GetCurrentModButtons();

    void                SetInputContext( const InputContext& rInputContext );
    const InputContext& GetInputContext() const;
    void                EndExtTextInput( sal_uInt16 nFlags );
    void                SetCursorRect( const Rectangle* pRect = NULL, long nExtTextInputWidth = 0 );
    const Rectangle*    GetCursorRect() const;
    long                GetCursorExtTextInputWidth() const;

    using               OutputDevice::SetSettings;
    virtual void        SetSettings( const AllSettings& rSettings );
    virtual void        SetSettings( const AllSettings& rSettings, sal_Bool bChild );
    void                UpdateSettings( const AllSettings& rSettings, sal_Bool bChild = sal_False );
    void                NotifyAllChildren( DataChangedEvent& rDCEvt );

    void                SetPointFont( const Font& rFont );
    Font                GetPointFont() const;
    void                SetZoomedPointFont( const Font& rFont );
    long                GetDrawPixel( OutputDevice* pDev, long nPixels ) const;
    Font                GetDrawPixelFont( OutputDevice* pDev ) const;

    void                SetControlFont();
    void                SetControlFont( const Font& rFont );
    Font                GetControlFont() const;
    sal_Bool                IsControlFont() const;
    void                SetControlForeground();
    void                SetControlForeground( const Color& rColor );
    Color               GetControlForeground() const;
    sal_Bool                IsControlForeground() const;
    void                SetControlBackground();
    void                SetControlBackground( const Color& rColor );
    Color               GetControlBackground() const;
    sal_Bool                IsControlBackground() const;

    void                SetParentClipMode( sal_uInt16 nMode = 0 );
    sal_uInt16              GetParentClipMode() const;

    void                SetWindowRegionPixel();
    void                SetWindowRegionPixel( const Region& rRegion );
    const Region&       GetWindowRegionPixel() const;
    sal_Bool                IsWindowRegionPixel() const;
    Region              GetWindowClipRegionPixel( sal_uInt16 nFlags = 0 ) const;
    Region              GetPaintRegion() const;
    sal_Bool                IsInPaint() const;
    // while IsInPaint returns true ExpandPaintClipRegion adds the
    // submitted region to the paint clip region so you can
    // paint additional parts of your window if necessary
    void                ExpandPaintClipRegion( const Region& rRegion );

    void                SetParent( Window* pNewParent );
    Window*             GetParent() const;
    // return the dialog we are contained in or NULL if un-contained
    Window*             GetParentDialog() const;

    void                Show( sal_Bool bVisible = sal_True, sal_uInt16 nFlags = 0 );
    void                Hide( sal_uInt16 nFlags = 0 ) { Show( sal_False, nFlags ); }
    sal_Bool                IsVisible() const;
    sal_Bool                IsReallyVisible() const;
    sal_Bool                IsReallyShown() const;
    sal_Bool                IsInInitShow() const;

    void                Enable( bool bEnable = true, bool bChild = true );
    void                Disable( bool bChild = true ) { Enable( false, bChild ); }
    sal_Bool                IsEnabled() const;

    void                EnableInput( sal_Bool bEnable = sal_True, sal_Bool bChild = sal_True );
    void                EnableInput( sal_Bool bEnable, sal_Bool bChild, sal_Bool bSysWin,
                                     const Window* pExcludeWindow = NULL );
    sal_Bool                IsInputEnabled() const;

    /** Override <code>EnableInput</code>. This can be necessary due to other people
        using EnableInput for whole window hierarchies.


        <code>AlwaysEnableInput</code> and <code>AlwaysDisableInput</code> are
        mutually exclusive; the last setter wins.
        @param bAlways
        sets always enabled flag

        @param bChild
        if true children are recursively set to AlwaysEnableInput
    */
    void                AlwaysEnableInput( sal_Bool bAlways, sal_Bool bChild = sal_True );
    /** returns the current AlwaysEnableInput state
    @return
    true if window is in AlwaysEnableInput state
    */
    sal_Bool                IsAlwaysEnableInput() const;
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
    void                AlwaysDisableInput( sal_Bool bAlways, sal_Bool bChild = sal_True );

    /** usually event handlers (see AddEventListener and AddChildEventListener)
    are not called on disabled, modal or input disabled windows. There are however rare cases
    in which one wants a Window or rather one of its Control subclasses to
    not evaluate events but still react to those events externally. In these
    rare cases call SetCallHandlersOnInputDisabled( true ) to have your handler
    called anyway.

    Currently only mouse events get this special treatment.

    Use this sparingly, chances are if you want to use it you're wroking around
    the real problem.

    @param bCall
    Enable/Disable calling event handlers for this disabled, modal or input disabled window.
    This call is implicity done recursively for possible child windows.
    */
    void                SetCallHandlersOnInputDisabled( bool bCall );
    /** get state of SetCallHandlersOnInputDisabled

    @returns whether handlers are called regardless of input enabled state
    */
    bool                IsCallHandlersOnInputDisabled() const;
    /** A window is in modal mode if one of its children or subchildren
        is a running modal window (a modal dialog)

        @returns sal_True if a child or subchild is a running modal window
    */
    sal_Bool                IsInModalMode() const;

    void                SetActivateMode( sal_uInt16 nMode );
    sal_uInt16              GetActivateMode() const;

    void                ToTop( sal_uInt16 nFlags = 0 );
    void                SetZOrder( Window* pRefWindow, sal_uInt16 nFlags );
    void                EnableAlwaysOnTop( sal_Bool bEnable = sal_True );
    sal_Bool                IsAlwaysOnTopEnabled() const;

    virtual void        SetPosSizePixel( long nX, long nY,
                                         long nWidth, long nHeight,
                                         sal_uInt16 nFlags = WINDOW_POSSIZE_ALL );
    virtual void        SetPosPixel( const Point& rNewPos );
    virtual Point       GetPosPixel() const;
    virtual void        SetSizePixel( const Size& rNewSize );
    virtual Size        GetSizePixel() const;
    virtual void        SetPosSizePixel( const Point& rNewPos,
                                         const Size& rNewSize );
    virtual void        SetOutputSizePixel( const Size& rNewSize );
    sal_Bool                IsDefaultPos() const;
    sal_Bool                IsDefaultSize() const;

    // those conversion routines might deliver different results during UI mirroring
    Point               OutputToScreenPixel( const Point& rPos ) const;
    Point               ScreenToOutputPixel( const Point& rPos ) const;
    //  the normalized screen methods work independent from UI mirroring
    Point               OutputToNormalizedScreenPixel( const Point& rPos ) const;
    Point               NormalizedScreenToOutputPixel( const Point& rPos ) const;
    Point               OutputToAbsoluteScreenPixel( const Point& rPos ) const;
    Point               AbsoluteScreenToOutputPixel( const Point& rPos ) const;
    Rectangle           GetDesktopRectPixel() const;
    //  window extents including border and decoratrion
    Rectangle           GetWindowExtentsRelative( Window *pRelativeWindow ) const;
    // window extents of the client window, coordinates to be used in SetPosPixel
    Rectangle           GetClientWindowExtentsRelative( Window *pRelativeWindow ) const;

    virtual sal_Bool        IsScrollable() const;
    virtual void        Scroll( long nHorzScroll, long nVertScroll,
                                sal_uInt16 nFlags = 0 );
    virtual void        Scroll( long nHorzScroll, long nVertScroll,
                                const Rectangle& rRect, sal_uInt16 nFlags = 0 );
    virtual void        Invalidate( sal_uInt16 nFlags = 0 );
    virtual void        Invalidate( const Rectangle& rRect, sal_uInt16 nFlags = 0 );
    virtual void        Invalidate( const Region& rRegion, sal_uInt16 nFlags = 0 );
    void                Validate( sal_uInt16 nFlags = 0 );
    sal_Bool                HasPaintEvent() const;
    void                Update();
    void                Flush();
    void                Sync();

    // toggles new docking support, enabled via toolkit
    void                EnableDocking( sal_Bool bEnable = sal_True );
    // retrieves the single dockingmanager instance
    static DockingManager* GetDockingManager();

    void                EnablePaint( sal_Bool bEnable );
    sal_Bool                IsPaintEnabled() const;
    void                SetUpdateMode( sal_Bool bUpdate );
    sal_Bool                IsUpdateMode() const;
    void                SetParentUpdateMode( sal_Bool bUpdate );

    void                GrabFocus();
    sal_Bool                HasFocus() const;
    sal_Bool                HasChildPathFocus( sal_Bool bSystemWindow = sal_False ) const;
    sal_Bool                IsActive() const;
    sal_Bool                HasActiveChildFrame();
    sal_uInt16              GetGetFocusFlags() const;
    void                GrabFocusToDocument();

    /**
     * Set this when you need to act as if the window has focus even if it
     * doesn't.  This is necessary for implementing tab stops inside floating
     * windows, but floating windows don't get focus from the system.
     */
    void                SetFakeFocus( bool bFocus );

    sal_Bool                IsCompoundControl() const;

    static sal_uIntPtr  SaveFocus();
    static sal_Bool         EndSaveFocus( sal_uIntPtr nSaveId, sal_Bool bRestore = sal_True );

    void                CaptureMouse();
    void                ReleaseMouse();
    sal_Bool                IsMouseCaptured() const;

    void                SetPointer( const Pointer& rPointer );
    const Pointer&      GetPointer() const;
    void                EnableChildPointerOverwrite( sal_Bool bOverwrite = sal_True );
    void                SetPointerPosPixel( const Point& rPos );
    Point               GetPointerPosPixel();
    Point               GetLastPointerPosPixel();
    void                ShowPointer( sal_Bool bVisible );
    void                EnterWait();
    void                LeaveWait();
    sal_Bool                IsWait() const;

    void                SetCursor( Cursor* pCursor );
    Cursor*             GetCursor() const;

    void                SetZoom( const Fraction& rZoom );
    const Fraction&     GetZoom() const;
    sal_Bool                IsZoom() const;
    long                CalcZoom( long n ) const;

    virtual void      SetText( const XubString& rStr );
    virtual String      GetText() const;
    // return the actual text displayed
    // this may have e.g. accellerators removed or portions
    // replaced by ellipsis
    virtual String      GetDisplayText() const;
    // gets the visible background color. for transparent windows
    // this may be the parent's background color; for controls
    // this may be a child's background color (e.g. ListBox)
    virtual const Wallpaper& GetDisplayBackground() const;

    void                SetBackgroundBitmap( const BitmapEx& rBitmapEx );

    void                SetHelpText( const XubString& rHelpText );
    const XubString&    GetHelpText() const;

    void                SetQuickHelpText( const XubString& rHelpText );
    const XubString&    GetQuickHelpText() const;

    void                SetHelpId( const rtl::OString& );
    const rtl::OString& GetHelpId() const;

    void                SetUniqueId( const rtl::OString& );
    const rtl::OString& GetUniqueId() const;

    Window*             FindWindow( const Point& rPos ) const;

    sal_uInt16              GetChildCount() const;
    Window*             GetChild( sal_uInt16 nChild ) const;
    Window*             GetWindow( sal_uInt16 nType ) const;
    sal_Bool                IsChild( const Window* pWindow, sal_Bool bSystemWindow = sal_False ) const;
    sal_Bool                IsWindowOrChild( const Window* pWindow, sal_Bool bSystemWindow = sal_False  ) const;

    void                SetData( void* pNewData );
    void*               GetData() const;

    void                ShowFocus( const Rectangle& rRect );
    void                HideFocus();

    void                Invert( const Rectangle& rRect, sal_uInt16 nFlags = 0 );
    void                Invert( const Polygon& rPoly, sal_uInt16 nFlags = 0 );

    // transparent background for selected or checked items in toolboxes etc.
    void                DrawSelectionBackground( const Rectangle& rRect, sal_uInt16 highlight, sal_Bool bChecked, sal_Bool bDrawBorder, sal_Bool bDrawExtBorderOnly );
    // the same, but fills a passed Color with a text color complementing the selection background
    void                DrawSelectionBackground( const Rectangle& rRect, sal_uInt16 highlight, sal_Bool bChecked, sal_Bool bDrawBorder, sal_Bool bDrawExtBorderOnly, Color* pSelectionTextColor );
    // support rounded edges in the selection rect
    void                DrawSelectionBackground( const Rectangle& rRect, sal_uInt16 highlight, sal_Bool bChecked, sal_Bool bDrawBorder, sal_Bool bDrawExtBorderOnly, long nCornerRadius, Color* pSelectionTextColor, Color* pPaintColor );

    void                ShowTracking( const Rectangle& rRect,
                                      sal_uInt16 nFlags = SHOWTRACK_SMALL );
    void                HideTracking();
    void                InvertTracking( const Rectangle& rRect,
                                        sal_uInt16 nFlags = SHOWTRACK_SMALL );
    void                InvertTracking( const Polygon& rPoly, sal_uInt16 nFlags = 0 );

    void                StartTracking( sal_uInt16 nFlags = 0 );
    void                EndTracking( sal_uInt16 nFlags = 0 );
    sal_Bool                IsTracking() const;

    void                StartAutoScroll( sal_uInt16 nFlags );
    void                EndAutoScroll();

    sal_Bool                HandleScrollCommand( const CommandEvent& rCmd,
                                             ScrollBar* pHScrl = NULL,
                                             ScrollBar* pVScrl = NULL );

    void                SaveBackground( const Point& rPos, const Size& rSize,
                                        const Point& rDestOff, VirtualDevice& rSaveDevice );

    const SystemEnvData*                      GetSystemData() const;
    ::com::sun::star::uno::Any                GetSystemDataAny() const;

    // API to set/query the component interfaces
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > GetComponentInterface( sal_Bool bCreate = sal_True );
    virtual void                    SetComponentInterface( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xIFace );

    // Accessibility
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > GetAccessible( sal_Bool bCreate = sal_True );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();
    void SetAccessible( ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > );

    Window* GetAccessibleParentWindow() const;
    sal_uInt16  GetAccessibleChildWindowCount();
    Window* GetAccessibleChildWindow( sal_uInt16 n );

    void    SetAccessibleRole( sal_uInt16 nRole );
    sal_uInt16  GetAccessibleRole() const;

    void    SetAccessibleName( const String& rName );
    String  GetAccessibleName() const;

    void    SetAccessibleDescription( const String& rDescr );
    String  GetAccessibleDescription() const;

    void    SetAccessibleRelationLabeledBy( Window* pLabeledBy );
    Window* GetAccessibleRelationLabeledBy() const;

    Window* GetAccessibleRelationLabelFor() const;

    void    SetAccessibleRelationMemberOf( Window* pMemberOf );

    // to avoid sending accessibility events in cases like closing dialogs
    // by default checks complete parent path
    sal_Bool    IsAccessibilityEventsSuppressed( sal_Bool bTraverseParentPath = sal_True );
    void    SetAccessibilityEventsSuppressed(sal_Bool bSuppressed);

    /// request XCanvas render interface for this window
    ::com::sun::star::uno::Reference<
        ::com::sun::star::rendering::XCanvas > GetCanvas() const;
    /// request XSpriteCanvas render interface for this window
    ::com::sun::star::uno::Reference<
        ::com::sun::star::rendering::XSpriteCanvas > GetSpriteCanvas() const;

    /*  records all DrawText operations within the passed rectangle;
     *  a synchronous paint is sent to achieve this
     */
    void                RecordLayoutData( vcl::ControlLayoutData* pLayout, const Rectangle& rRect );

    // set and retrieve for Toolkit
    VCLXWindow*             GetWindowPeer() const;
    void                    SetWindowPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xPeer, VCLXWindow* pVCLXWindow );

    // remember if it was generated by Toolkit
    sal_Bool                IsCreatedWithToolkit() const;
    void                    SetCreatedWithToolkit( sal_Bool b );

            // Deprecated - can use SetAccessibleRelationLabelFor/By nowadys
    virtual Window* GetParentLabelFor( const Window* pLabel ) const;
    virtual Window* GetParentLabeledBy( const Window* pLabeled ) const;
    KeyEvent            GetActivationKey() const;

    // Drag and Drop interfaces
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTarget > GetDropTarget();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSource > GetDragSource();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragGestureRecognizer > GetDragGestureRecognizer();

    // Clipboard/Selection interfaces
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > GetClipboard();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > GetPrimarySelection();

    // Advisory Sizing - what is a good size for this widget ?
    virtual Size GetOptimalSize(WindowSizeType eType) const;

    //-------------------------------------
    //  Native Widget Rendering functions
    //-------------------------------------

    // form controls must never use native widgets, this can be toggled here
    void    EnableNativeWidget( sal_Bool bEnable = sal_True );
    sal_Bool    IsNativeWidgetEnabled() const;

    // a helper method for a Control's Draw method
    void PaintToDevice( OutputDevice* pDevice, const Point& rPos, const Size& rSize );

    /* mark Window for deletion in top of event queue
    */
    void doLazyDelete();

    //-------------------------------------
    //  Keyboard access functions
    //-------------------------------------

    /** Query the states of keyboard indicators - Caps Lock, Num Lock and
        Scroll Lock.  Use the following mask to retrieve the state of each
        indicator:

            INDICATOR_CAPS_LOCK
            INDICATOR_NUM_LOCK
            INDICATOR_SCROLL_LOCK
      */
    sal_uInt16 GetIndicatorState() const;

    void SimulateKeyPress( sal_uInt16 nKeyCode ) const;

    virtual rtl::OUString GetSurroundingText() const;
    virtual Selection GetSurroundingTextSelection() const;

    // ExtImpl

    // layouting
    boost::shared_ptr< vcl::WindowArranger > getLayout();

    /* add a child Window
       addWindow will do the following things
       - insert the passed window into the child list (equivalent to i_pWin->SetParent( this ))
       - mark the window as "owned", meaning that the added Window will be destroyed by
         the parent's desctructor.
         This means: do not pass in member windows or stack objects here. Do not cause
         the destructor of the added window to be called in any way.

         to avoid ownership pass i_bTakeOwnership as "false"
    */
    void addWindow( Window* i_pWin, bool i_bTakeOwnership = true );

    /* return the identifier of this window
    */
    const rtl::OUString& getIdentifier() const;

    /* returns the first found descendant that matches
       the passed identifier or NULL
    */
    Window* findWindow( const rtl::OUString& ) const;

    /* get/set properties
       this will contain window properties (like visible, enabled)
       as well as properties of derived classes (e.g. text of Edit fields)
    */
    virtual com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue > getProperties() const;
    /*
    */
    virtual void setProperties( const com::sun::star::uno::Sequence< com::sun::star::beans::PropertyValue >& );
};


#endif // _SV_WINDOW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
