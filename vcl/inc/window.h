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

#ifndef _SV_WINDOW_H
#define _SV_WINDOW_H

#include <vector>
#include <tools/solar.h>
#include <vcl/outdev.hxx>
#include <vcl/timer.hxx>
#ifndef _SV_INPUTCTX_HXX
#include <vcl/inputctx.hxx>
#endif
#ifndef _SV_POINTR_HXX
#include <vcl/pointr.hxx>
#endif
#include <tools/wintypes.hxx>
#include <vcl/vclevent.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <cppuhelper/weakref.hxx>

#include <vcl/salnativewidgets.hxx>

#include <list>

struct SalPaintEvent;
struct ImplDelData;
struct ImplAccessibleInfos;

class Window;
class VirtualDevice;
class Cursor;
class ImplDevFontList;
class ImplFontCache;
class VCLXWindow;
class SalFrame;
class SalObject;


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
    class XDropTargetListener;
    class XDragGestureRecognizer;
    class XDragSource;
    class XDropTarget;
} } } } }

namespace vcl {
    struct ControlLayoutData;
    struct ExtWindowImpl;
}


// --------------
// - Prototypes -
// --------------

long ImplWindowFrameProc( Window* pInst, SalFrame* pFrame, sal_uInt16 nEvent, const void* pEvent );

// -----------
// - HitTest -
// -----------

#define WINDOW_HITTEST_INSIDE           ((sal_uInt16)0x0001)
#define WINDOW_HITTEST_TRANSPARENT      ((sal_uInt16)0x0002)

// ---------------
// - ImplWinData -
// ---------------

struct ImplWinData
{
    UniString*          mpExtOldText;
    sal_uInt16*             mpExtOldAttrAry;
    Rectangle*          mpCursorRect;
    long                mnCursorExtWidth;
    Rectangle*          mpFocusRect;
    Rectangle*          mpTrackRect;
    sal_uInt16              mnTrackFlags;
    sal_uInt16              mnIsTopWindow;
    sal_Bool                mbMouseOver;          // tracks mouse over for native widget paint effect
    sal_Bool                mbEnableNativeWidget; // toggle native widget rendering
    ::std::list< Window* >
                        maTopWindowChildren;
};

// -------------------
// - ImplOverlapData -
// -------------------

struct ImplOverlapData
{
    VirtualDevice*      mpSaveBackDev;      // Gesicherte Hintergrund-Bitmap
    Region*             mpSaveBackRgn;      // Gesicherte Region, was invalidiert werden muss
    Window*             mpNextBackWin;      // Naechstes Fenster mit Hintergrund-Sicherung
    sal_uIntPtr               mnSaveBackSize;     // Groesse Bitmap fuer Hintergrund-Sicherung
    sal_Bool                mbSaveBack;         // sal_True: Background sichern
    sal_uInt8                mnTopLevel;         // Level for Overlap-Window
};

// -----------------
// - ImplFrameData -
// -----------------

struct ImplFrameData
{
    Timer               maPaintTimer;       // paint timer
    Timer               maResizeTimer;      // resize timer
    InputContext        maOldInputContext;  // Last set Input Context
    Window*             mpNextFrame;        // next frame window
    Window*             mpFirstOverlap;     // first overlap window
    Window*             mpFocusWin;         // focus window (is also set, when frame doesn't have the focous)
    Window*             mpMouseMoveWin;     // last window, where MouseMove() called
    Window*             mpMouseDownWin;     // last window, where MouseButtonDown() called
    Window*             mpFirstBackWin;     // Erstes Overlap-Window mit Hintergrund-Sicherung
    ::std::vector<Window *> maOwnerDrawList; // List of system windows with owner draw decoration
    ImplDevFontList*    mpFontList;         // Font-List for this frame
    ImplFontCache*      mpFontCache;        // Font-Cache for this frame
    sal_Int32           mnDPIX;             // Original Screen Resolution
    sal_Int32           mnDPIY;             // Original Screen Resolution
    ImplMapRes          maMapUnitRes;       // for LogicUnitToPixel
    sal_uIntPtr               mnAllSaveBackSize;  // Groesse aller Bitmaps fuer Hintergrund-Sicherung
    sal_uIntPtr               mnFocusId;          // FocusId for PostUserLink
    sal_uIntPtr               mnMouseMoveId;      // MoveId for PostUserLink
    long                mnLastMouseX;       // last x mouse position
    long                mnLastMouseY;       // last y mouse position
    long                mnBeforeLastMouseX; // last but one x mouse position
    long                mnBeforeLastMouseY; // last but one y mouse position
    long                mnFirstMouseX;      // first x mouse position by mousebuttondown
    long                mnFirstMouseY;      // first y mouse position by mousebuttondown
    long                mnLastMouseWinX;    // last x mouse position, rel. to pMouseMoveWin
    long                mnLastMouseWinY;    // last y mouse position, rel. to pMouseMoveWin
    sal_uInt16              mnModalMode;        // frame based modal count (app based makes no sense anymore)
    sal_uIntPtr               mnMouseDownTime;    // mouse button down time for double click
    sal_uInt16              mnClickCount;       // mouse click count
    sal_uInt16              mnFirstMouseCode;   // mouse code by mousebuttondown
    sal_uInt16              mnMouseCode;        // mouse code
    sal_uInt16              mnMouseMode;        // mouse mode
    MapUnit             meMapUnit;          // last MapUnit for LogicUnitToPixel
    sal_Bool                mbHasFocus;         // focus
    sal_Bool                mbInMouseMove;      // is MouseMove on stack
    sal_Bool                mbMouseIn;          // is Mouse inside the frame
    sal_Bool                mbStartDragCalled;  // is command startdrag called
    sal_Bool                mbNeedSysWindow;    // set, when FrameSize <= IMPL_MIN_NEEDSYSWIN
    sal_Bool                mbMinimized;        // set, when FrameSize <= 0
    sal_Bool                mbStartFocusState;  // FocusState, beim abschicken des Events
    sal_Bool                mbInSysObjFocusHdl; // Innerhalb vom GetFocus-Handler eines SysChildren
    sal_Bool                mbInSysObjToTopHdl; // Innerhalb vom ToTop-Handler eines SysChildren
    sal_Bool                mbSysObjFocus;      // Hat ein SysChild den Focus

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragSource > mxDragSource;
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTarget > mxDropTarget;
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener > mxDropTargetListener;
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > mxClipboard;
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > mxSelection;

    sal_Bool                mbInternalDragGestureRecognizer;
};

// -----------------------
// - ImplAccessibleInfos -
// -----------------------

struct ImplAccessibleInfos
{
    sal_uInt16 nAccessibleRole;
    String* pAccessibleName;
    String* pAccessibleDescription;
    Window* pLabeledByWindow;
    Window* pLabelForWindow;
    Window* pMemberOfWindow;

    ImplAccessibleInfos();
    ~ImplAccessibleInfos();
};


// ---------------
// - WindowImpl -
// ---------------

enum AlwaysInputMode { AlwaysInputNone = 0, AlwaysInputEnabled = 1, AlwaysInputDisabled =2 };

class WindowImpl
{
public:
    WindowImpl();
    ~WindowImpl();

    ImplWinData*        mpWinData;
    ImplOverlapData*    mpOverlapData;
    ImplFrameData*      mpFrameData;
    SalFrame*           mpFrame;
    SalObject*          mpSysObj;
    Window*             mpFrameWindow;
    Window*             mpOverlapWindow;
    Window*             mpBorderWindow;
    Window*             mpClientWindow;
    Window*             mpParent;
    Window*             mpRealParent;
    Window*             mpFirstChild;
    Window*             mpLastChild;
    Window*             mpFirstOverlap;
    Window*             mpLastOverlap;
    Window*             mpPrev;
    Window*             mpNext;
    Window*             mpNextOverlap;
    Window*             mpLastFocusWindow;
    Window*             mpDlgCtrlDownWindow;
    VclEventListeners   maEventListeners;
    VclEventListeners   maChildEventListeners;

    // The canvas interface for this VCL window. Is persistent after the first GetCanvas() call
    ::com::sun::star::uno::WeakReference< ::com::sun::star::rendering::XCanvas >    mxCanvas;

    ImplDelData*        mpFirstDel;
    void*               mpUserData;
    vcl::ExtWindowImpl* mpExtImpl;
    Cursor*             mpCursor;
    Pointer             maPointer;
    Fraction            maZoom;
    XubString           maText;
    Font*               mpControlFont;
    Color               maControlForeground;
    Color               maControlBackground;
    sal_Int32           mnLeftBorder;
    sal_Int32           mnTopBorder;
    sal_Int32           mnRightBorder;
    sal_Int32           mnBottomBorder;
    sal_Int32           mnWidthRequest;
    sal_Int32           mnHeightRequest;
    long                mnX;
    long                mnY;
    long                mnAbsScreenX;
    Point               maPos;
    rtl::OString        maHelpId;
    rtl::OString        maUniqId;
    XubString           maHelpText;
    XubString           maQuickHelpText;
    InputContext        maInputContext;
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindowPeer > mxWindowPeer;
    ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > mxAccessible;
    ImplAccessibleInfos* mpAccessibleInfos;
    VCLXWindow*         mpVCLXWindow;
    Region              maWinRegion;        // region to 'shape' the VCL window (frame coordinates)
    Region              maWinClipRegion;    // the (clipping) region that finally corresponds to the VCL window (frame coordinates)
    Region              maInvalidateRegion; // region that has to be redrawn (frame coordinates)
    Region*             mpChildClipRegion;  // child clip region if CLIPCHILDREN is set (frame coordinates)
    Region*             mpPaintRegion;      // only set during Paint() method call (window coordinates)
    WinBits             mnStyle;
    WinBits             mnPrevStyle;
    WinBits             mnExtendedStyle;
    WinBits             mnPrevExtendedStyle;
    WindowType          mnType;
    ControlPart         mnNativeBackground;
    sal_uInt16              mnWaitCount;
    sal_uInt16              mnPaintFlags;
    sal_uInt16              mnGetFocusFlags;
    sal_uInt16              mnParentClipMode;
    sal_uInt16              mnActivateMode;
    sal_uInt16              mnDlgCtrlFlags;
    sal_uInt16              mnLockCount;
    AlwaysInputMode     meAlwaysInputMode;
    VclAlign            meHalign;
    VclAlign            meValign;
    VclPackType         mePackType;
    sal_Int32           mnPadding;
    sal_Int32           mnGridHeight;
    sal_Int32           mnGridLeftAttach;
    sal_Int32           mnGridTopAttach;
    sal_Int32           mnGridWidth;
    sal_Int32           mnBorderWidth;
    sal_Int32           mnMarginLeft;
    sal_Int32           mnMarginRight;
    sal_Int32           mnMarginTop;
    sal_Int32           mnMarginBottom;
    sal_uInt8           mbFrame:1,
                        mbBorderWin:1,
                        mbOverlapWin:1,
                        mbSysWin:1,
                        mbDialog:1,
                        mbDockWin:1,
                        mbFloatWin:1,
                        mbPushButton:1,
                        mbVisible:1,
                        mbDisabled:1,
                        mbInputDisabled:1,
                        mbDropDisabled:1,
                        mbNoUpdate:1,
                        mbNoParentUpdate:1,
                        mbActive:1,
                        mbParentActive:1,
                        mbReallyVisible:1,
                        mbReallyShown:1,
                        mbInInitShow:1,
                        mbChildNotify:1,
                        mbChildPtrOverwrite:1,
                        mbNoPtrVisible:1,
                        mbPaintFrame:1,
                        mbInPaint:1,
                        mbMouseMove:1,
                        mbMouseButtonDown:1,
                        mbMouseButtonUp:1,
                        mbKeyInput:1,
                        mbKeyUp:1,
                        mbCommand:1,
                        mbDefPos:1,
                        mbDefSize:1,
                        mbCallMove:1,
                        mbCallResize:1,
                        mbWaitSystemResize:1,
                        mbInitWinClipRegion:1,
                        mbInitChildRegion:1,
                        mbWinRegion:1,
                        mbClipChildren:1,
                        mbClipSiblings:1,
                        mbChildTransparent:1,
                        mbPaintTransparent:1,
                        mbMouseTransparent:1,
                        mbDlgCtrlStart:1,
                        mbFocusVisible:1,
                        mbTrackVisible:1,
                        mbUseNativeFocus:1,
                        mbNativeFocusVisible:1,
                        mbInShowFocus:1,
                        mbInHideFocus:1,
                        mbControlForeground:1,
                        mbControlBackground:1,
                        mbAlwaysOnTop:1,
                        mbCompoundControl:1,
                        mbCompoundControlHasFocus:1,
                        mbPaintDisabled:1,
                        mbAllResize:1,
                        mbInDtor:1,
                        mbExtTextInput:1,
                        mbInFocusHdl:1,
                        mbOverlapVisible:1,
                        mbCreatedWithToolkit:1,
                        mbToolBox:1,
                        mbSplitter:1,
                        mbSuppressAccessibilityEvents:1,
                        mbMenuFloatingWindow:1,
                        mbDrawSelectionBackground:1,
                        mbIsInTaskPaneList:1,
                        mbToolbarFloatingWindow:1,
                        mbCallHandlersDuringInputDisabled:1,
                        mbDisableAccessibleLabelForRelation:1,
                        mbDisableAccessibleLabeledByRelation:1,
                        mbHelpTextDynamic:1,
                        mbFakeFocusSet:1,
                        mbHexpand:1,
                        mbVexpand:1,
                        mbExpand:1,
                        mbFill:1;

    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > mxDNDListenerContainer;
};

// -----------------
// - Hilfsmethoden -
// -----------------

long ImplHandleMouseEvent( Window* pWindow, sal_uInt16 nSVEvent, sal_Bool bMouseLeave,
                           long nX, long nY, sal_uIntPtr nMsgTime,
                           sal_uInt16 nCode, sal_uInt16 nMode );
void ImplHandleResize( Window* pWindow, long nNewWidth, long nNewHeight );

#endif // _SV_WINDOW_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
