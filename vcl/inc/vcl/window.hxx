/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: window.hxx,v $
 * $Revision: 1.8 $
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

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/outdev.hxx>
#include <tools/resid.hxx>
#ifndef _SV_POINTR_HXX
#include <vcl/pointr.hxx>
#endif
#include <vcl/wintypes.hxx>
#ifndef _SV_APPTYPES_HXX
#include <vcl/apptypes.hxx>
#endif
#include <vcl/inputctx.hxx>
#include <vcl/vclevent.hxx>
// Only for compatibility - because many people outside haven't included event.hxx
#ifndef _VCL_EVENT_HXX
#include <vcl/event.hxx>
#endif
#include <vcl/region.hxx>
#include <vcl/salnativewidgets.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <vcl/smartid.hxx>

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
namespace rendering {
    class XCanvas;
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

namespace vcl { struct ControlLayoutData; }

// ---------------
// - WindowTypes -
// ---------------

// Type fuer GetWindow()
#define WINDOW_PARENT                   ((USHORT)0)
#define WINDOW_FIRSTCHILD               ((USHORT)1)
#define WINDOW_LASTCHILD                ((USHORT)2)
#define WINDOW_PREV                     ((USHORT)3)
#define WINDOW_NEXT                     ((USHORT)4)
#define WINDOW_FIRSTOVERLAP             ((USHORT)5)
#define WINDOW_LASTOVERLAP              ((USHORT)6)
#define WINDOW_OVERLAP                  ((USHORT)7)
#define WINDOW_PARENTOVERLAP            ((USHORT)8)
#define WINDOW_CLIENT                   ((USHORT)9)
#define WINDOW_REALPARENT               ((USHORT)10)
#define WINDOW_FRAME                    ((USHORT)11)
#define WINDOW_BORDER                   ((USHORT)12)
#define WINDOW_FIRSTTOPWINDOWCHILD      ((USHORT)13)
#define WINDOW_LASTTOPWINDOWCHILD       ((USHORT)14)
#define WINDOW_PREVTOPWINDOWSIBLING     ((USHORT)15)
#define WINDOW_NEXTTOPWINDOWSIBLING     ((USHORT)16)

// Flags for SetPosSizePixel()
#define WINDOW_POSSIZE_X                ((USHORT)0x0001)
#define WINDOW_POSSIZE_Y                ((USHORT)0x0002)
#define WINDOW_POSSIZE_WIDTH            ((USHORT)0x0004)
#define WINDOW_POSSIZE_HEIGHT           ((USHORT)0x0008)
#define WINDOW_POSSIZE_POS              (WINDOW_POSSIZE_X | WINDOW_POSSIZE_Y)
#define WINDOW_POSSIZE_SIZE             (WINDOW_POSSIZE_WIDTH | WINDOW_POSSIZE_HEIGHT)
#define WINDOW_POSSIZE_POSSIZE          (WINDOW_POSSIZE_POS | WINDOW_POSSIZE_SIZE)
#define WINDOW_POSSIZE_ALL              (WINDOW_POSSIZE_POSSIZE)
#define WINDOW_POSSIZE_DROPDOWN         ((USHORT)0x0010)

// Flags for Show()
#define SHOW_NOPARENTUPDATE             ((USHORT)0x0001)
#define SHOW_NOFOCUSCHANGE              ((USHORT)0x0002)
#define SHOW_NOACTIVATE                 ((USHORT)0x0004)

// Flags for SetZOrder()
#define WINDOW_ZORDER_BEFOR             ((USHORT)0x0001)
#define WINDOW_ZORDER_BEHIND            ((USHORT)0x0002)
#define WINDOW_ZORDER_FIRST             ((USHORT)0x0004)
#define WINDOW_ZORDER_LAST              ((USHORT)0x0008)

// Activate-Flags
#define ACTIVATE_MODE_GRABFOCUS         ((USHORT)0x0001)

// ToTop-Flags
#define TOTOP_RESTOREWHENMIN            ((USHORT)0x0001)
#define TOTOP_FOREGROUNDTASK            ((USHORT)0x0002)
#define TOTOP_NOGRABFOCUS               ((USHORT)0x0004)
//#if 0 // _SOLAR__PRIVATE // vcl internal only
#define TOTOP_GRABFOCUSONLY             ((USHORT)0x0008)
//#endif

// Flags for Invalidate
#define INVALIDATE_CHILDREN             ((USHORT)0x0001)
#define INVALIDATE_NOCHILDREN           ((USHORT)0x0002)
#define INVALIDATE_NOERASE              ((USHORT)0x0004)
#define INVALIDATE_UPDATE               ((USHORT)0x0008)
#define INVALIDATE_TRANSPARENT          ((USHORT)0x0010)
#define INVALIDATE_NOTRANSPARENT        ((USHORT)0x0020)
#define INVALIDATE_NOCLIPCHILDREN       ((USHORT)0x4000)
// Temporaer fuer Kompatibilitaet
#define INVALIDATE_BACKGROUND           INVALIDATE_TRANSPARENT

// Flags for Validate
#define VALIDATE_CHILDREN               ((USHORT)0x0001)
#define VALIDATE_NOCHILDREN             ((USHORT)0x0002)

// Flags for Scroll
#define SCROLL_CLIP                     ((USHORT)0x0001)
#define SCROLL_CHILDREN                 ((USHORT)0x0002)
#define SCROLL_NOCHILDREN               ((USHORT)0x0004)
#define SCROLL_NOERASE                  ((USHORT)0x0008)
#define SCROLL_NOINVALIDATE             ((USHORT)0x0010)
#define SCROLL_NOWINDOWINVALIDATE       ((USHORT)0x0020)
#define SCROLL_USECLIPREGION            ((USHORT)0x0040)
#define SCROLL_UPDATE                   ((USHORT)0x0080)

// Flags for ParentClipMode
#define PARENTCLIPMODE_CLIP             ((USHORT)0x0001)
#define PARENTCLIPMODE_NOCLIP           ((USHORT)0x0002)

// Flags for Invert()
#define INVERT_HIGHLIGHT                ((USHORT)0x0001)
#define INVERT_50                       ((USHORT)0x0002)

// Flags for ShowTracking()
#define SHOWTRACK_SMALL                 ((USHORT)0x0001)
#define SHOWTRACK_BIG                   ((USHORT)0x0002)
#define SHOWTRACK_SPLIT                 ((USHORT)0x0003)
#define SHOWTRACK_OBJECT                ((USHORT)0x0004)
#define SHOWTRACK_WINDOW                ((USHORT)0x1000)
#define SHOWTRACK_CLIP                  ((USHORT)0x2000)
#define SHOWTRACK_STYLE                 ((USHORT)0x000F)

// Flags for StartTracking()
#define STARTTRACK_KEYINPUT             ((USHORT)0x0001)
#define STARTTRACK_KEYMOD               ((USHORT)0x0002)
#define STARTTRACK_NOKEYCANCEL          ((USHORT)0x0004)
#define STARTTRACK_SCROLLREPEAT         ((USHORT)0x0008)
#define STARTTRACK_BUTTONREPEAT         ((USHORT)0x0010)
#define STARTTRACK_MOUSEBUTTONDOWN      ((USHORT)0x0020)
#define STARTTRACK_FOCUSCANCEL          ((USHORT)0x0040)

// Flags for StartAutoScroll()
#define AUTOSCROLL_VERT                 ((USHORT)0x0001)
#define AUTOSCROLL_HORZ                 ((USHORT)0x0002)

// Flags for StateChanged()
typedef USHORT StateChangedType;
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
#define STATE_CHANGE_FORMAT             ((StateChangedType)17)
#define STATE_CHANGE_EXTENDEDSTYLE      ((StateChangedType)18)
#define STATE_CHANGE_USER               ((StateChangedType)10000)

// GetFocusFlags
#define GETFOCUS_TAB                    ((USHORT)0x0001)
#define GETFOCUS_CURSOR                 ((USHORT)0x0002)
#define GETFOCUS_MNEMONIC               ((USHORT)0x0004)
#define GETFOCUS_FORWARD                ((USHORT)0x0010)
#define GETFOCUS_BACKWARD               ((USHORT)0x0020)
#define GETFOCUS_AROUND                 ((USHORT)0x0040)
#define GETFOCUS_UNIQUEMNEMONIC         ((USHORT)0x0100)
#define GETFOCUS_INIT                   ((USHORT)0x0200)
#define GETFOCUS_FLOATWIN_POPUPMODEEND_CANCEL ((USHORT)0x0400)

// Draw-Flags fuer Draw()
#define WINDOW_DRAW_MONO                ((ULONG)0x00000001)
#define WINDOW_DRAW_NOBORDER            ((ULONG)0x00000002)
#define WINDOW_DRAW_NOCONTROLS          ((ULONG)0x00000004)
#define WINDOW_DRAW_NODISABLE           ((ULONG)0x00000008)
#define WINDOW_DRAW_NOMNEMONIC          ((ULONG)0x00000010)
#define WINDOW_DRAW_NOSELECTION         ((ULONG)0x00000020)
#define WINDOW_DRAW_NOFOCUS             ((ULONG)0x00000040)
#define WINDOW_DRAW_NOBACKGROUND        ((ULONG)0x00000080)
#define WINDOW_DRAW_ROLLOVER            ((ULONG)0x00000100)

// Border-Styles fuer SetBorder()
#define WINDOW_BORDER_NORMAL            ((USHORT)0x0001)
#define WINDOW_BORDER_MONO              ((USHORT)0x0002)
#define WINDOW_BORDER_ACTIVE            ((USHORT)0x0004)
#define WINDOW_BORDER_DOUBLEOUT         ((USHORT)0x0008)
#define WINDOW_BORDER_MENU              ((USHORT)0x0010)
#define WINDOW_BORDER_NOBORDER          ((USHORT)0x1000)
#define WINDOW_BORDER_REMOVEBORDER      ((USHORT)0x2000)

// DialogControl-Flags
#define WINDOW_DLGCTRL_RETURN           ((USHORT)0x0001)
#define WINDOW_DLGCTRL_WANTFOCUS        ((USHORT)0x0002)
#define WINDOW_DLGCTRL_MOD1TAB          ((USHORT)0x0004)
#define WINDOW_DLGCTRL_FLOATWIN_POPUPMODEEND_CANCEL ((USHORT)0x0008)

// GetWindowClipRegionPixel-Flags
#define WINDOW_GETCLIPREGION_NULL       ((USHORT)0x0001)
#define WINDOW_GETCLIPREGION_NOCHILDREN ((USHORT)0x0002)

// EndExtTextInput-Flags
#define EXTTEXTINPUT_END_COMPLETE       ((USHORT)0x0001)
#define EXTTEXTINPUT_END_CANCEL         ((USHORT)0x0002)

//#if 0 // _SOLAR__PRIVATE
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
#define IMPL_SEP_BUTTON_IMAGE           4

#define DLGWINDOW_PREV                  0
#define DLGWINDOW_NEXT                  1
#define DLGWINDOW_FIRST                 2
//#endif

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

class   WindowImpl;
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
    SAL_DLLPRIVATE void                ImplInit( Window* pParent, WinBits nStyle, const ::com::sun::star::uno::Any& aSystemWorkWindowToken );
    SAL_DLLPRIVATE WinBits             ImplInitRes( const ResId& rResId );
    SAL_DLLPRIVATE void                ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE void                ImplWindowRes( const ResId& rResId );
    SAL_DLLPRIVATE void                ImplSetFrameParent( const Window* pParent );
    SAL_DLLPRIVATE void                ImplInsertWindow( Window* pParent );
    SAL_DLLPRIVATE void                ImplRemoveWindow( BOOL bRemoveFrameData );
    SAL_DLLPRIVATE Window*             ImplGetWindow();
    SAL_DLLPRIVATE ImplFrameData*      ImplGetFrameData();
    SAL_DLLPRIVATE SalFrame*           ImplGetFrame() const;
    SAL_DLLPRIVATE ImplWinData*        ImplGetWinData() const;
    SAL_DLLPRIVATE SalGraphics*        ImplGetFrameGraphics() const;
    SAL_DLLPRIVATE void                ImplCallFocusChangeActivate( Window* pNewOverlapWindow, Window* pOldOverlapWindow );
    SAL_DLLPRIVATE Window*             ImplFindWindow( const Point& rFramePos );
    SAL_DLLPRIVATE USHORT              ImplHitTest( const Point& rFramePos );
    SAL_DLLPRIVATE Window*             ImplGetParent() const;
    SAL_DLLPRIVATE Window*             ImplGetClientWindow() const;
    SAL_DLLPRIVATE Window*             ImplGetBorderWindow() const;
    SAL_DLLPRIVATE Window*             ImplGetFirstOverlapWindow();
    SAL_DLLPRIVATE const Window*       ImplGetFirstOverlapWindow() const;
    SAL_DLLPRIVATE Window*             ImplGetFrameWindow() const;
    SAL_DLLPRIVATE BOOL                ImplIsRealParentPath( const Window* pWindow ) const;
    SAL_DLLPRIVATE BOOL                ImplIsChild( const Window* pWindow, BOOL bSystemWindow = FALSE ) const;
    SAL_DLLPRIVATE BOOL                ImplIsWindowOrChild( const Window* pWindow, BOOL bSystemWindow = FALSE ) const;
    SAL_DLLPRIVATE Window*             ImplGetSameParent( const Window* pWindow ) const;
    SAL_DLLPRIVATE BOOL                ImplIsDockingWindow() const;
    SAL_DLLPRIVATE BOOL                ImplIsFloatingWindow() const;
    SAL_DLLPRIVATE BOOL                ImplIsToolbox() const;
    SAL_DLLPRIVATE BOOL                ImplIsSplitter() const;
    SAL_DLLPRIVATE BOOL                ImplIsPushButton() const;
    SAL_DLLPRIVATE BOOL                ImplIsOverlapWindow() const;
    SAL_DLLPRIVATE void                ImplSetActive( BOOL bActive );
    SAL_DLLPRIVATE BOOL                ImplIsMouseTransparent() const;
    SAL_DLLPRIVATE void                ImplSetMouseTransparent( BOOL bTransparent );
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
    SAL_DLLPRIVATE void                ImplOutputToFrame( Rectangle& rRect );
    SAL_DLLPRIVATE void                ImplFrameToOutput( Rectangle& rRect );
    SAL_DLLPRIVATE BOOL                ImplSysObjClip( const Region* pOldRegion );
    SAL_DLLPRIVATE void                ImplUpdateSysObjChildsClip();
    SAL_DLLPRIVATE void                ImplUpdateSysObjOverlapsClip();
    SAL_DLLPRIVATE void                ImplUpdateSysObjClip();
    SAL_DLLPRIVATE BOOL                ImplSetClipFlagChilds( BOOL bSysObjOnlySmaller = FALSE );
    SAL_DLLPRIVATE BOOL                ImplSetClipFlagOverlapWindows( BOOL bSysObjOnlySmaller = FALSE );
    SAL_DLLPRIVATE BOOL                ImplSetClipFlag( BOOL bSysObjOnlySmaller = FALSE );
    SAL_DLLPRIVATE void                ImplIntersectWindowClipRegion( Region& rRegion );
    SAL_DLLPRIVATE void                ImplIntersectWindowRegion( Region& rRegion );
    SAL_DLLPRIVATE void                ImplExcludeWindowRegion( Region& rRegion );
    SAL_DLLPRIVATE void                ImplExcludeOverlapWindows( Region& rRegion );
    SAL_DLLPRIVATE void                ImplExcludeOverlapWindows2( Region& rRegion );
    SAL_DLLPRIVATE void                ImplClipBoundaries( Region& rRegion, BOOL bThis, BOOL bOverlaps );
    SAL_DLLPRIVATE BOOL                ImplClipChilds( Region& rRegion );
    SAL_DLLPRIVATE void                ImplClipAllChilds( Region& rRegion );
    SAL_DLLPRIVATE void                ImplClipSiblings( Region& rRegion );
    SAL_DLLPRIVATE void                ImplInitWinClipRegion();
    SAL_DLLPRIVATE void                ImplInitWinChildClipRegion();
    SAL_DLLPRIVATE Region*             ImplGetWinChildClipRegion();
    SAL_DLLPRIVATE void                ImplIntersectAndUnionOverlapWindows( const Region& rInterRegion, Region& rRegion );
    SAL_DLLPRIVATE void                ImplIntersectAndUnionOverlapWindows2( const Region& rInterRegion, Region& rRegion );
    SAL_DLLPRIVATE void                ImplCalcOverlapRegionOverlaps( const Region& rInterRegion, Region& rRegion );
    SAL_DLLPRIVATE void                ImplCalcOverlapRegion( const Rectangle& rSourceRect, Region& rRegion,
                                               BOOL bChilds, BOOL bParent, BOOL bSiblings );
    SAL_DLLPRIVATE void                ImplCallPaint( const Region* pRegion, USHORT nPaintFlags );
    SAL_DLLPRIVATE void                ImplCallOverlapPaint();
    SAL_DLLPRIVATE void                ImplPostPaint();
    SAL_DLLPRIVATE void                ImplInvalidateFrameRegion( const Region* pRegion, USHORT nFlags );
    SAL_DLLPRIVATE void                ImplInvalidateOverlapFrameRegion( const Region& rRegion );
    SAL_DLLPRIVATE void                ImplInvalidateParentFrameRegion( Region& rRegion );
    SAL_DLLPRIVATE void                ImplInvalidate( const Region* rRegion, USHORT nFlags );
    SAL_DLLPRIVATE void                ImplValidateFrameRegion( const Region* rRegion, USHORT nFlags );
    SAL_DLLPRIVATE void                ImplValidate( const Region* rRegion, USHORT nFlags );
    SAL_DLLPRIVATE void                ImplMoveInvalidateRegion( const Rectangle& rRect, long nHorzScroll, long nVertScroll, BOOL bChilds );
    SAL_DLLPRIVATE void                ImplMoveAllInvalidateRegions( const Rectangle& rRect, long nHorzScroll, long nVertScroll, BOOL bChilds );
    SAL_DLLPRIVATE void                ImplScroll( const Rectangle& rRect, long nHorzScroll, long nVertScroll, USHORT nFlags );
    SAL_DLLPRIVATE void                ImplUpdateAll( BOOL bOverlapWindows = TRUE );
    SAL_DLLPRIVATE void                ImplUpdateWindowPtr( Window* pWindow );
    SAL_DLLPRIVATE void                ImplUpdateWindowPtr();
    SAL_DLLPRIVATE void                ImplUpdateOverlapWindowPtr( BOOL bNewFrame );
    SAL_DLLPRIVATE BOOL                ImplUpdatePos();
    SAL_DLLPRIVATE void                ImplUpdateSysObjPos();
    SAL_DLLPRIVATE WindowImpl*         ImplGetWindowImpl() const { return mpWindowImpl; }
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
    SAL_DLLPRIVATE void        ImplUpdateGlobalSettings( AllSettings& rSettings, BOOL bCallHdl = TRUE );
    SAL_DLLPRIVATE void        ImplAlignChilds();
    SAL_DLLPRIVATE void        ImplPosSizeWindow( long nX, long nY, long nWidth, long nHeight, USHORT nFlags );
    SAL_DLLPRIVATE void        ImplToBottomChild();
    SAL_DLLPRIVATE void        ImplCalcToTop( ImplCalcToTopData* pPrevData );
    SAL_DLLPRIVATE void        ImplCalcChildOverlapToTop( ImplCalcToTopData* pPrevData );
    SAL_DLLPRIVATE void        ImplToTop( USHORT nFlags );
    SAL_DLLPRIVATE void        ImplStartToTop( USHORT nFlags );
    SAL_DLLPRIVATE void        ImplFocusToTop( USHORT nFlags, BOOL bReallyVisible );
    SAL_DLLPRIVATE void        ImplShowAllOverlaps();
    SAL_DLLPRIVATE void        ImplHideAllOverlaps();
    SAL_DLLPRIVATE void        ImplNotifyKeyMouseCommandEventListeners( NotifyEvent& rNEvt );
    SAL_DLLPRIVATE void        ImplCallMouseMove( USHORT nMouseCode, BOOL bModChanged = FALSE );
    SAL_DLLPRIVATE void        ImplGenerateMouseMove();
    SAL_DLLPRIVATE void        ImplGrabFocus( USHORT nFlags );
    SAL_DLLPRIVATE void        ImplInvertFocus( const Rectangle& rRect );
    SAL_DLLPRIVATE void        ImplControlFocus( USHORT nFlags = 0 );
    SAL_DLLPRIVATE Window*     ImplGetDlgWindow( USHORT n, USHORT nType, USHORT nStart = 0, USHORT nEnd = 0xFFFF, USHORT* pIndex = NULL );
    SAL_DLLPRIVATE BOOL        ImplDlgCtrl( const KeyEvent& rKEvt, BOOL bKeyInput );
    SAL_DLLPRIVATE BOOL        ImplHasDlgCtrl();
    SAL_DLLPRIVATE void        ImplDlgCtrlNextWindow();
    SAL_DLLPRIVATE void        ImplDlgCtrlFocusChanged( Window* pWindow, BOOL bGetFocus );
    SAL_DLLPRIVATE Window*     ImplFindDlgCtrlWindow( Window* pWindow );
    SAL_DLLPRIVATE long        ImplLogicUnitToPixelX( long nX, MapUnit eUnit );
    SAL_DLLPRIVATE long        ImplLogicUnitToPixelY( long nY, MapUnit eUnit );
    SAL_DLLPRIVATE BOOL        ImplIsWindowInFront( const Window* pTestWindow ) const;
    SAL_DLLPRIVATE void        ImplSaveOverlapBackground();
    SAL_DLLPRIVATE BOOL        ImplRestoreOverlapBackground( Region& rInvRegion );
    SAL_DLLPRIVATE void        ImplDeleteOverlapBackground();
    SAL_DLLPRIVATE void        ImplInvalidateAllOverlapBackgrounds();
    SAL_DLLPRIVATE static void ImplNewInputContext();
    SAL_DLLPRIVATE void        ImplCallActivateListeners(Window*);
    SAL_DLLPRIVATE void        ImplCallDeactivateListeners(Window*);
    DECL_DLLPRIVATE_LINK(      ImplHandlePaintHdl, void* );
    DECL_DLLPRIVATE_LINK(      ImplGenerateMouseMoveHdl, void* );
    DECL_DLLPRIVATE_LINK(      ImplTrackTimerHdl, Timer* );
    DECL_DLLPRIVATE_LINK(      ImplAsyncFocusHdl, void* );
    DECL_DLLPRIVATE_LINK(      ImplAsyncStateChangedHdl, void* );
    DECL_DLLPRIVATE_LINK(      ImplHideOwnerDrawWindowsHdl, void* );
    DECL_DLLPRIVATE_LINK(      ImplHandleResizeTimerHdl, void* );

    SAL_DLLPRIVATE static void ImplCalcSymbolRect( Rectangle& rRect );
    SAL_DLLPRIVATE void        ImplHandleScroll( ScrollBar* pHScrl, long nX, ScrollBar* pVScrl, long nY );
    SAL_DLLPRIVATE BOOL        ImplGetCurrentBackgroundColor( Color& rCol );
    SAL_DLLPRIVATE BOOL        ImplIsAccessibleCandidate() const;
    SAL_DLLPRIVATE BOOL        ImplIsAccessibleNativeFrame() const;
    SAL_DLLPRIVATE USHORT      ImplGetAccessibleCandidateChildWindowCount( USHORT nFirstWindowType ) const;
    SAL_DLLPRIVATE Window*     ImplGetAccessibleCandidateChild( USHORT nChild, USHORT& rChildCount, USHORT nFirstWindowType, BOOL bTopLevel = TRUE ) const;
    SAL_DLLPRIVATE BOOL        ImplRegisterAccessibleNativeFrame();
    SAL_DLLPRIVATE void        ImplRevokeAccessibleNativeFrame();
    SAL_DLLPRIVATE void        ImplCallResize();
    SAL_DLLPRIVATE void        ImplCallMove();
    SAL_DLLPRIVATE Rectangle   ImplOutputToUnmirroredAbsoluteScreenPixel( const Rectangle& rRect ) const;
    SAL_DLLPRIVATE void        ImplMirrorFramePos( Point &pt ) const;
    SAL_DLLPRIVATE long        ImplGetUnmirroredOutOffX();
    SAL_DLLPRIVATE void        ImplIncModalCount();
    SAL_DLLPRIVATE void        ImplDecModalCount();

    // retrieves the list of owner draw decorated windows for this window hiearchy
    SAL_DLLPRIVATE ::std::vector<Window *>& ImplGetOwnerDrawList();
    SAL_DLLPRIVATE Window*     ImplGetTopmostFrameWindow();

    SAL_DLLPRIVATE Rectangle   ImplGetWindowExtentsRelative( Window *pRelativeWindow, BOOL bClientOnly );
    SAL_DLLPRIVATE void        ImplNotifyIconifiedState( BOOL bIconified );
    SAL_DLLPRIVATE bool        ImplStopDnd();
    SAL_DLLPRIVATE void        ImplStartDnd();

    SAL_DLLPRIVATE static void ImplInitAppFontData( Window* pWindow );
    SAL_DLLPRIVATE void        ImplInitSalControlHandle();
    SAL_DLLPRIVATE void        ImplPaintToMetaFile( GDIMetaFile* pMtf, OutputDevice* pTargetOutDev, const Region* pOuterClip = NULL );

    SAL_DLLPRIVATE BOOL        ImplIsInTaskPaneList();
    SAL_DLLPRIVATE void        ImplIsInTaskPaneList( BOOL mbIsInTaskList );
    SAL_DLLPRIVATE ::com::sun::star::uno::Reference< ::com::sun::star::rendering::XCanvas >
                               ImplGetCanvas( const Size& rFullscreenSize, bool bFullscreen ) const;
    SAL_DLLPRIVATE void        ImplMoveControlValue( ControlType, const ImplControlValue&, const Point& ) const;

private:
    // Default construction is forbidden and not implemented.
    SAL_DLLPRIVATE             Window();

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE             Window (const Window &);
    SAL_DLLPRIVATE             Window & operator= (const Window &);

protected:
    // Single argument ctors shall be explicit.
    explicit            Window( WindowType nType );

            void        SetCompoundControl( BOOL bCompound );

            void        ImplCallEventListeners( ULONG nEvent, void* pData = NULL );
            void        CallEventListeners( ULONG nEvent, void* pData = NULL );


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
    virtual void        Paint( const Rectangle& rRect );
    virtual void        Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, ULONG nFlags );
    virtual void        Move();
    virtual void        Resize();
    virtual void        Activate();
    virtual void        Deactivate();
    virtual void        GetFocus();
    virtual void        LoseFocus();
    virtual void        RequestHelp( const HelpEvent& rHEvt );
    virtual void        Command( const CommandEvent& rCEvt );
    virtual void        Tracking( const TrackingEvent& rTEvt );
    virtual void        UserEvent( ULONG nEvent, void* pEventData );
    virtual void        StateChanged( StateChangedType nStateChange );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );
    virtual long        PreNotify( NotifyEvent& rNEvt );
    virtual long        Notify( NotifyEvent& rNEvt );
    virtual Window*     GetPreferredKeyInputWindow();

    /*virtual*/ void    AddEventListener( const Link& rEventListener );
    /*virtual*/ void    RemoveEventListener( const Link& rEventListener );
    /*virtual*/ void    AddChildEventListener( const Link& rEventListener );
    /*virtual*/ void    RemoveChildEventListener( const Link& rEventListener );

    ULONG               PostUserEvent( ULONG nEvent, void* pEventData = NULL );
    ULONG               PostUserEvent( const Link& rLink, void* pCaller = NULL );
    BOOL                PostUserEvent( ULONG& rEventId, ULONG nEvent, void* pEventData = NULL );
    BOOL                PostUserEvent( ULONG& rEventId, const Link& rLink, void* pCaller = NULL );
    void                RemoveUserEvent( ULONG nUserEvent );
    void                PostStateChanged( StateChangedType nState );

    void                IncrementLockCount();
    void                DecrementLockCount();
    BOOL                IsLocked( BOOL bChilds = FALSE ) const;

                        // returns the input language used for the last key stroke
                        // may be LANGUAGE_DONTKNOW if not supported by the OS
    LanguageType        GetInputLanguage() const;

    void                SetStyle( WinBits nStyle );
    WinBits             GetStyle() const;
    WinBits             GetPrevStyle() const;
    void                SetExtendedStyle( WinBits nExtendedStyle );
    WinBits             GetExtendedStyle() const;
    WinBits             GetPrevExtendedStyle() const;
    void                SetType( WindowType nType );
    WindowType          GetType() const;
    BOOL                IsSystemWindow() const;
    BOOL                IsDialog() const;
    BOOL                IsMenuFloatingWindow() const;
    BOOL                IsToolbarFloatingWindow() const;
    BOOL                IsTopWindow() const;
    SystemWindow*       GetSystemWindow() const;

    void                EnableAllResize( BOOL bEnable = TRUE );
    BOOL                IsAllResizeEnabled() const;

    void                SetBorderStyle( USHORT nBorderStyle );
    USHORT              GetBorderStyle() const;
    void                GetBorder( sal_Int32& rLeftBorder, sal_Int32& rTopBorder,
                                   sal_Int32& rRightBorder, sal_Int32& rBottomBorder ) const;
    Size                CalcWindowSize( const Size& rOutSz ) const;
    Size                CalcOutputSize( const Size& rWinSz ) const;
    long                CalcTitleWidth() const;

    void                EnableClipSiblings( BOOL bClipSiblings = TRUE );
    BOOL                IsClipSiblingsEnabled() const;

    void                EnableChildTransparentMode( BOOL bEnable = TRUE );
    BOOL                IsChildTransparentModeEnabled() const;

    void                SetMouseTransparent( BOOL bTransparent );
    BOOL                IsMouseTransparent() const;
    void                SetPaintTransparent( BOOL bTransparent );
    BOOL                IsPaintTransparent() const;
    void                SetDialogControlStart( BOOL bStart );
    BOOL                IsDialogControlStart() const;
    void                SetDialogControlFlags( USHORT nFlags );
    USHORT              GetDialogControlFlags() const;

    struct PointerState
    {
        ULONG   mnState;    // the button state
        Point   maPos;      // mouse position in output coordinates
    };
    PointerState        GetPointerState();
    BOOL                IsMouseOver();

    ULONG               GetCurrentModButtons();

    void                SetInputContext( const InputContext& rInputContext );
    const InputContext& GetInputContext() const;
    void                EndExtTextInput( USHORT nFlags );
    BOOL                IsExtTextInput() const;
    void                SetCursorRect( const Rectangle* pRect = NULL, long nExtTextInputWidth = 0 );
    const Rectangle*    GetCursorRect() const;
    long                GetCursorExtTextInputWidth() const;

    void                EnableChildNotify( BOOL bEnable );
    BOOL                IsChildNotify() const;

    using               OutputDevice::SetSettings;
    virtual void        SetSettings( const AllSettings& rSettings );
    virtual void        SetSettings( const AllSettings& rSettings, BOOL bChild );
    void                UpdateSettings( const AllSettings& rSettings, BOOL bChild = FALSE );
    void                NotifyAllChilds( DataChangedEvent& rDCEvt );

    void                SetPointFont( const Font& rFont );
    Font                GetPointFont() const;
    void                SetZoomedPointFont( const Font& rFont );
    long                GetDrawPixel( OutputDevice* pDev, long nPixels ) const;
    Font                GetDrawPixelFont( OutputDevice* pDev ) const;
    void                GetFontResolution( sal_Int32& nDPIX, sal_Int32& nDPIY ) const;

    void                SetControlFont();
    void                SetControlFont( const Font& rFont );
    Font                GetControlFont() const;
    BOOL                IsControlFont() const;
    void                SetControlForeground();
    void                SetControlForeground( const Color& rColor );
    Color               GetControlForeground() const;
    BOOL                IsControlForeground() const;
    void                SetControlBackground();
    void                SetControlBackground( const Color& rColor );
    Color               GetControlBackground() const;
    BOOL                IsControlBackground() const;

    void                SetParentClipMode( USHORT nMode = 0 );
    USHORT              GetParentClipMode() const;

    void                SetWindowRegionPixel();
    void                SetWindowRegionPixel( const Region& rRegion );
    const Region&       GetWindowRegionPixel() const;
    BOOL                IsWindowRegionPixel() const;
    Region              GetWindowClipRegionPixel( USHORT nFlags = 0 ) const;
    Region              GetPaintRegion() const;
    BOOL                IsInPaint() const;
    // while IsInPaint returns true ExpandPaintClipRegion adds the
    // submitted region to the paint clip region so you can
    // paint additional parts of your window if necessary
    void                ExpandPaintClipRegion( const Region& rRegion );

    void                SetParent( Window* pNewParent );
    Window*             GetParent() const;

    void                Show( BOOL bVisible = TRUE, USHORT nFlags = 0 );
    void                Hide( USHORT nFlags = 0 ) { Show( FALSE, nFlags ); }
    BOOL                IsVisible() const;
    BOOL                IsReallyVisible() const;
    // Do not use this function, use IsReallyVisible()
    BOOL                IsParentPathVisible() const;
    BOOL                IsReallyShown() const;
    BOOL                IsInInitShow() const;

    void                Enable( bool bEnable = true, bool bChild = true );
    void                Disable( bool bChild = true ) { Enable( false, bChild ); }
    BOOL                IsEnabled() const;

    void                EnableInput( BOOL bEnable = TRUE, BOOL bChild = TRUE );
    void                EnableInput( BOOL bEnable, BOOL bChild, BOOL bSysWin,
                                     const Window* pExcludeWindow = NULL );
    BOOL                IsInputEnabled() const;

    /** Override <code>EnableInput</code>. This can be necessary due to other people
        using EnableInput for whole window hierarchies.


        <code>AlwaysEnableInput</code> and <code>AlwaysDisableInput</code> are
        mutually exclusive; the last setter wins.
        @param bAlways
        sets always enabled flag

        @param bChild
        if true children are recursively set to AlwaysEnableInput
    */
    void                AlwaysEnableInput( BOOL bAlways, BOOL bChild = TRUE );
    /** returns the current AlwaysEnableInput state
    @return
    true if window is in AlwaysEnableInput state
    */
    BOOL                IsAlwaysEnableInput() const;
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
    void                AlwaysDisableInput( BOOL bAlways, BOOL bChild = TRUE );
    /** returns the current AlwaysDisableInput state
    @return
    true if window is in AlwaysEnableInput state
    */
    BOOL                IsAlwaysDisableInput() const;
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

        @returns TRUE if a child or subchild is a running modal window
    */
    BOOL                IsInModalMode() const;

    void                SetActivateMode( USHORT nMode );
    USHORT              GetActivateMode() const;

    void                ToTop( USHORT nFlags = 0 );
    void                SetZOrder( Window* pRefWindow, USHORT nFlags );
    void                EnableAlwaysOnTop( BOOL bEnable = TRUE );
    BOOL                IsAlwaysOnTopEnabled() const;

    virtual void        SetPosSizePixel( long nX, long nY,
                                         long nWidth, long nHeight,
                                         USHORT nFlags = WINDOW_POSSIZE_ALL );
    virtual void        SetPosPixel( const Point& rNewPos );
    virtual Point       GetPosPixel() const;
    virtual void        SetSizePixel( const Size& rNewSize );
    virtual Size        GetSizePixel() const;
    virtual void        SetPosSizePixel( const Point& rNewPos,
                                         const Size& rNewSize );
    virtual void        SetOutputSizePixel( const Size& rNewSize );
    BOOL                IsDefaultPos() const;
    BOOL                IsDefaultSize() const;

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
    Rectangle           GetWindowExtentsRelative( Window *pRelativeWindow );
    // window extents of the client window, coordinates to be used in SetPosPixel
    Rectangle           GetClientWindowExtentsRelative( Window *pRelativeWindow );

    virtual BOOL        IsScrollable() const;
    virtual void        Scroll( long nHorzScroll, long nVertScroll,
                                USHORT nFlags = 0 );
    virtual void        Scroll( long nHorzScroll, long nVertScroll,
                                const Rectangle& rRect, USHORT nFlags = 0 );
    virtual void        Invalidate( USHORT nFlags = 0 );
    virtual void        Invalidate( const Rectangle& rRect, USHORT nFlags = 0 );
    virtual void        Invalidate( const Region& rRegion, USHORT nFlags = 0 );
    void                Validate( USHORT nFlags = 0 );
    void                Validate( const Rectangle& rRect, USHORT nFlags = 0 );
    void                Validate( const Region& rRegion, USHORT nFlags = 0 );
    BOOL                HasPaintEvent() const;
    void                Update();
    void                Flush();
    void                Sync();

    // toggles new docking support, enabled via toolkit
    void                EnableDocking( BOOL bEnable = TRUE );
    // retrieves the single dockingmanager instance
    static DockingManager* GetDockingManager();

    void                EnablePaint( BOOL bEnable );
    BOOL                IsPaintEnabled() const;
    void                SetUpdateMode( BOOL bUpdate );
    BOOL                IsUpdateMode() const;
    void                SetParentUpdateMode( BOOL bUpdate );
    BOOL                IsParentUpdateMode() const;

    void                GrabFocus();
    BOOL                HasFocus() const;
    BOOL                HasChildPathFocus( BOOL bSystemWindow = FALSE ) const;
    BOOL                IsActive() const;
    BOOL                HasActiveChildFrame();
    USHORT              GetGetFocusFlags() const;
    void                GrabFocusToDocument();

    BOOL                IsCompoundControl() const;
    BOOL                HasCompoundControlFocus() const;

    static sal_uIntPtr  SaveFocus();
    static BOOL         EndSaveFocus( sal_uIntPtr nSaveId, BOOL bRestore = TRUE );

    void                CaptureMouse();
    void                ReleaseMouse();
    BOOL                IsMouseCaptured() const;

    void                SetPointer( const Pointer& rPointer );
    const Pointer&      GetPointer() const;
    void                EnableChildPointerOverwrite( BOOL bOverwrite = TRUE );
    BOOL                IsChildPointerOverwrite() const;
    void                SetPointerPosPixel( const Point& rPos );
    Point               GetPointerPosPixel();
    Point               GetLastPointerPosPixel();
    void                ShowPointer( BOOL bVisible );
    BOOL                IsPointerVisible() const;
    void                EnterWait();
    void                LeaveWait();
    BOOL                IsWait() const;

    void                SetCursor( Cursor* pCursor );
    Cursor*             GetCursor() const;

    void                SetZoom( const Fraction& rZoom );
    const Fraction&     GetZoom() const;
    BOOL                IsZoom() const;
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

    void                SetHelpText( const XubString& rHelpText );
    const XubString&    GetHelpText() const;

    void                SetQuickHelpText( const XubString& rHelpText );
    const XubString&    GetQuickHelpText() const;

    void                SetHelpId( ULONG nHelpId );     /// deprecated
    ULONG               GetHelpId() const;              /// deprecated
    void                SetSmartHelpId( const SmartId& aId, SmartIdUpdateMode aMode = SMART_SET_SMART );
    SmartId             GetSmartHelpId() const;

    void                SetUniqueId( ULONG nUniqueId ); /// deprecated
    ULONG               GetUniqueId() const;            /// deprecated
    void                SetSmartUniqueId( const SmartId& aId, SmartIdUpdateMode aMode = SMART_SET_SMART );
    SmartId             GetSmartUniqueId() const;
    SmartId             GetSmartUniqueOrHelpId() const;

    Window*             FindWindow( const Point& rPos ) const;

    USHORT              GetChildCount() const;
    Window*             GetChild( USHORT nChild ) const;
    Window*             GetWindow( USHORT nType ) const;
    BOOL                IsChild( const Window* pWindow, BOOL bSystemWindow = FALSE ) const;
    BOOL                IsWindowOrChild( const Window* pWindow, BOOL bSystemWindow = FALSE  ) const;

    void                SetData( void* pNewData );
    void*               GetData() const;

    // Should be merged in the next top level build !!!
    Bitmap              SnapShot( BOOL bBorder ) const;
    Bitmap              SnapShot() const;

    void                ShowFocus( const Rectangle& rRect );
    void                HideFocus();

    void                Invert( const Rectangle& rRect, USHORT nFlags = 0 );
    void                Invert( const Polygon& rPoly, USHORT nFlags = 0 );

    // transparent background for selected or checked items in toolboxes etc.
    void                DrawSelectionBackground( const Rectangle& rRect, USHORT highlight, BOOL bChecked, BOOL bDrawBorder, BOOL bDrawExtBorderOnly );
    // the same, but fills a passed Color with a text color complementing the selection background
    void                DrawSelectionBackground( const Rectangle& rRect, USHORT highlight, BOOL bChecked, BOOL bDrawBorder, BOOL bDrawExtBorderOnly, Color* pSelectionTextColor );

    void                ShowTracking( const Rectangle& rRect,
                                      USHORT nFlags = SHOWTRACK_SMALL );
    void                HideTracking();
    void                InvertTracking( const Rectangle& rRect,
                                        USHORT nFlags = SHOWTRACK_SMALL );
    void                InvertTracking( const Polygon& rPoly, USHORT nFlags = 0 );

    void                StartTracking( USHORT nFlags = 0 );
    void                EndTracking( USHORT nFlags = 0 );
    BOOL                IsTracking() const;

    void                StartAutoScroll( USHORT nFlags );
    void                EndAutoScroll();
    BOOL                IsAutoScroll() const;

    BOOL                HandleScrollCommand( const CommandEvent& rCmd,
                                             ScrollBar* pHScrl = NULL,
                                             ScrollBar* pVScrl = NULL );

    void                SaveBackground( const Point& rPos, const Size& rSize,
                                        const Point& rDestOff, VirtualDevice& rSaveDevice );

    const SystemEnvData*                      GetSystemData() const;
    ::com::sun::star::uno::Any                GetSystemDataAny() const;

    // API zum Setzen/Abfragen des Komponenteninterfaces
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > GetComponentInterface( BOOL bCreate = TRUE );
    virtual void                    SetComponentInterface( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xIFace );

    // Accessibility
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > GetAccessible( BOOL bCreate = TRUE );
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();
    void SetAccessible( ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > );

    Window* GetAccessibleParentWindow() const;
    USHORT  GetAccessibleChildWindowCount();
    Window* GetAccessibleChildWindow( USHORT n );

    void    SetAccessibleRole( USHORT nRole );
    USHORT  GetAccessibleRole() const;

    void    SetAccessibleName( const String& rName );
    String  GetAccessibleName() const;

    void    SetAccessibleDescription( const String& rDescr );
    String  GetAccessibleDescription() const;

    // to avoid sending accessibility events in cases like closing dialogs
    // by default checks complete parent path
    BOOL    IsAccessibilityEventsSuppressed( BOOL bTraverseParentPath = TRUE );

    // new Canvas
    ::com::sun::star::uno::Reference<
        ::com::sun::star::rendering::XCanvas > GetCanvas() const;
    // new Canvas
    ::com::sun::star::uno::Reference<
        ::com::sun::star::rendering::XCanvas > GetFullscreenCanvas( const Size& rFullscreenSize ) const;

    /*  records all DrawText operations within the passed rectangle;
     *  a synchronous paint is sent to achieve this
     */
    void                RecordLayoutData( vcl::ControlLayoutData* pLayout, const Rectangle& rRect );

    // Setzen und Abfragen fuer das Toolkit
    VCLXWindow*             GetWindowPeer() const;
    void                    SetWindowPeer( ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > xPeer, VCLXWindow* pVCLXWindow );

    // Merken, ob vom Toolkit erzeugt
    BOOL                    IsCreatedWithToolkit() const;
    void                    SetCreatedWithToolkit( BOOL b );

    Window*             GetLabelFor() const;
    Window*             GetLabeledBy() const;
    virtual Window* GetParentLabelFor( const Window* pLabel ) const;
    virtual Window* GetParentLabeledBy( const Window* pLabeled ) const;
    KeyEvent            GetActivationKey() const;

    // Drag and Drop interfaces
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTarget > GetDropTarget();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSource > GetDragSource();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragGestureRecognizer > GetDragGestureRecognizer();
    // only for RVP transmission
    void GetDragSourceDropTarget(::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSource >& xDragSource,::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTarget > &xDropTarget );

    // Clipboard/Selection interfaces
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > GetClipboard();
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > GetPrimarySelection();

    // Advisory Sizing - what is a good size for this widget ?
    virtual Size GetOptimalSize(WindowSizeType eType) const;

    //-------------------------------------
    //  Native Widget Rendering functions
    //-------------------------------------

    // form controls must never use native widgets, this can be toggled here
    void    EnableNativeWidget( BOOL bEnable = TRUE );
    BOOL    IsNativeWidgetEnabled() const;

    // These all just call through to the private mpWindowImpl->mpFrame functions of the same name.

    // Query the platform layer for control support
    BOOL                    IsNativeControlSupported( ControlType nType, ControlPart nPart );

    // Query the native control to determine if it was acted upon
    BOOL                HitTestNativeControl( ControlType nType,
                                      ControlPart nPart,
                                      const Region& rControlRegion,
                                      const Point& aPos,
                                      BOOL& rIsInside );

    // Request rendering of a particular control and/or part
    BOOL                DrawNativeControl(    ControlType nType,
                                      ControlPart nPart,
                                      const Region& rControlRegion,
                                      ControlState nState,
                                      const ImplControlValue& aValue,
                                      rtl::OUString aCaption );

     // Request rendering of a caption string for a control
    BOOL                DrawNativeControlText(     ControlType nType,
                                          ControlPart nPart,
                                          const Region& rControlRegion,
                                          ControlState nState,
                                          const ImplControlValue& aValue,
                                          rtl::OUString aCaption );

    // Query the native control's actual drawing region (including adornment)
    BOOL                GetNativeControlRegion(  ControlType nType,
                                          ControlPart nPart,
                                          const Region& rControlRegion,
                                          ControlState nState,
                                          const ImplControlValue& aValue,
                                          rtl::OUString aCaption,
                                          Region &rNativeBoundingRegion,
                                          Region &rNativeContentRegion );

    // a helper method for a Control's Draw method
    void PaintToDevice( OutputDevice* pDevice, const Point& rPos, const Size& rSize );

    /* mark Window for deletion in top of event queue
    */
    void doLazyDelete();
};


#endif // _SV_WINDOW_HXX
