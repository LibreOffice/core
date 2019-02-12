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

#ifndef INCLUDED_VCL_INC_WINDOW_H
#define INCLUDED_VCL_INC_WINDOW_H

#include <sal/config.h>

#include <tools/fract.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/idle.hxx>
#include <vcl/inputctx.hxx>
#include <vcl/window.hxx>
#include <vcl/settings.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <cppuhelper/weakref.hxx>

#include <boost/optional.hpp>
#include <list>
#include <memory>
#include <vector>
#include <set>

class FixedText;
class VclSizeGroup;
class VirtualDevice;
class PhysicalFontCollection;
class ImplFontCache;
class VCLXWindow;
class WindowStateData;
class SalFrame;
class SalObject;
enum class MouseEventModifiers;
enum class MouseNotifyEvent;
enum class ActivateModeFlags;
enum class DialogControlFlags;
enum class GetFocusFlags;
enum class ParentClipMode;
enum class SalEvent;

namespace com { namespace sun { namespace star {

namespace accessibility {
    class XAccessible;
}

namespace rendering {
    class XCanvas;
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
    class XDropTargetListener;
    class XDragGestureRecognizer;
    class XDragSource;
    class XDropTarget;
}}}}}

VCL_DLLPUBLIC Size bestmaxFrameSizeForScreenSize(const Size &rScreenSize);

//return true if this window and its stack of containers are all shown
bool isVisibleInLayout(const vcl::Window *pWindow);

//return true if this window and its stack of containers are all enabled
bool isEnabledInLayout(const vcl::Window *pWindow);

bool ImplWindowFrameProc( vcl::Window* pInst, SalEvent nEvent, const void* pEvent );

struct ImplWinData
{
    boost::optional<OUString>
                        mpExtOldText;
    std::unique_ptr<ExtTextInputAttr[]>
                        mpExtOldAttrAry;
    boost::optional<tools::Rectangle>
                        mpCursorRect;
    long                mnCursorExtWidth;
    bool                mbVertical;
    std::unique_ptr<tools::Rectangle[]>
                        mpCompositionCharRects;
    long                mnCompositionCharRects;
    boost::optional<tools::Rectangle>
                        mpFocusRect;
    boost::optional<tools::Rectangle>
                        mpTrackRect;
    ShowTrackFlags      mnTrackFlags;
    sal_uInt16          mnIsTopWindow;
    bool                mbMouseOver;            //< tracks mouse over for native widget paint effect
    bool                mbEnableNativeWidget;   //< toggle native widget rendering
    ::std::list< VclPtr<vcl::Window> >
                        maTopWindowChildren;

     ImplWinData();
    ~ImplWinData();
};

struct ImplFrameData
{
    Idle                maPaintIdle;            //< paint idle handler
    Idle                maResizeIdle;          //< resize timer
    InputContext        maOldInputContext;      //< last set Input Context
    VclPtr<vcl::Window> mpNextFrame;            //< next frame window
    VclPtr<vcl::Window> mpFirstOverlap;         //< first overlap vcl::Window
    VclPtr<vcl::Window> mpFocusWin;             //< focus window (is also set, when frame doesn't have the focus)
    VclPtr<vcl::Window> mpMouseMoveWin;         //< last window, where MouseMove() called
    VclPtr<vcl::Window> mpMouseDownWin;         //< last window, where MouseButtonDown() called
    std::vector<VclPtr<vcl::Window> > maOwnerDrawList;    //< List of system windows with owner draw decoration
    std::shared_ptr<PhysicalFontCollection> mxFontCollection;   //< Font-List for this frame
    std::shared_ptr<ImplFontCache> mxFontCache; //< Font-Cache for this frame
    sal_Int32           mnDPIX;                 //< Original Screen Resolution
    sal_Int32           mnDPIY;                 //< Original Screen Resolution
    ImplSVEvent *       mnFocusId;              //< FocusId for PostUserLink
    ImplSVEvent *       mnMouseMoveId;          //< MoveId for PostUserLink
    long                mnLastMouseX;           //< last x mouse position
    long                mnLastMouseY;           //< last y mouse position
    long                mnBeforeLastMouseX;     //< last but one x mouse position
    long                mnBeforeLastMouseY;     //< last but one y mouse position
    long                mnFirstMouseX;          //< first x mouse position by mousebuttondown
    long                mnFirstMouseY;          //< first y mouse position by mousebuttondown
    long                mnLastMouseWinX;        //< last x mouse position, rel. to pMouseMoveWin
    long                mnLastMouseWinY;        //< last y mouse position, rel. to pMouseMoveWin
    sal_uInt16          mnModalMode;            //< frame based modal count (app based makes no sense anymore)
    sal_uInt64          mnMouseDownTime;        //< mouse button down time for double click
    sal_uInt16          mnClickCount;           //< mouse click count
    sal_uInt16          mnFirstMouseCode;       //< mouse code by mousebuttondown
    sal_uInt16          mnMouseCode;            //< mouse code
    MouseEventModifiers mnMouseMode;            //< mouse mode
    bool                mbHasFocus;             //< focus
    bool                mbInMouseMove;          //< is MouseMove on stack
    bool                mbMouseIn;              //> is Mouse inside the frame
    bool                mbStartDragCalled;      //< is command startdrag called
    bool                mbNeedSysWindow;        //< set, when FrameSize <= IMPL_MIN_NEEDSYSWIN
    bool                mbMinimized;            //< set, when FrameSize <= 0
    bool                mbStartFocusState;      //< FocusState, when sending the event
    bool                mbInSysObjFocusHdl;     //< within a SysChildren's GetFocus handler
    bool                mbInSysObjToTopHdl;     //< within a SysChildren's ToTop handler
    bool                mbSysObjFocus;          //< does a SysChild have focus

    css::uno::Reference< css::datatransfer::dnd::XDragSource > mxDragSource;
    css::uno::Reference< css::datatransfer::dnd::XDropTarget > mxDropTarget;
    css::uno::Reference< css::datatransfer::dnd::XDropTargetListener > mxDropTargetListener;
    css::uno::Reference< css::datatransfer::clipboard::XClipboard > mxClipboard;
    css::uno::Reference< css::datatransfer::clipboard::XClipboard > mxSelection;

    bool                mbInternalDragGestureRecognizer;
    VclPtr<VirtualDevice> mpBuffer; ///< Buffer for the double-buffering
    bool mbInBufferedPaint; ///< PaintHelper is in the process of painting into this buffer.
    tools::Rectangle maBufferedRect; ///< Rectangle in the buffer that has to be painted to the screen.

    ImplFrameData( vcl::Window *pWindow );
};

struct ImplAccessibleInfos
{
    sal_uInt16          nAccessibleRole;
    boost::optional<OUString>
                        pAccessibleName;
    boost::optional<OUString>
                        pAccessibleDescription;
    VclPtr<vcl::Window> pLabeledByWindow;
    VclPtr<vcl::Window> pLabelForWindow;
    VclPtr<vcl::Window> pMemberOfWindow;

    ImplAccessibleInfos();
    ~ImplAccessibleInfos();
};

enum AlwaysInputMode { AlwaysInputNone = 0, AlwaysInputEnabled = 1, AlwaysInputDisabled =2 };

enum class ImplPaintFlags {
    NONE             = 0x0000,
    Paint            = 0x0001,
    PaintAll         = 0x0002,
    PaintAllChildren = 0x0004,
    PaintChildren    = 0x0008,
    Erase            = 0x0010,
    CheckRtl         = 0x0020,
};
namespace o3tl {
    template<> struct typed_flags<ImplPaintFlags> : is_typed_flags<ImplPaintFlags, 0x003f> {};
}


class WindowImpl
{
private:
    WindowImpl(const WindowImpl&) = delete;
    WindowImpl& operator=(const WindowImpl&) = delete;
public:
    WindowImpl( WindowType );
    ~WindowImpl();

    ImplWinData*        mpWinData;
    ImplFrameData*      mpFrameData;
    SalFrame*           mpFrame;
    SalObject*          mpSysObj;
    VclPtr<vcl::Window> mpFrameWindow;
    VclPtr<vcl::Window> mpOverlapWindow;
    VclPtr<vcl::Window> mpBorderWindow;
    VclPtr<vcl::Window> mpClientWindow;
    VclPtr<vcl::Window> mpParent;
    VclPtr<vcl::Window> mpRealParent;
    VclPtr<vcl::Window> mpFirstChild;
    VclPtr<vcl::Window> mpLastChild;
    VclPtr<vcl::Window> mpFirstOverlap;
    VclPtr<vcl::Window> mpLastOverlap;
    VclPtr<vcl::Window> mpPrev;
    VclPtr<vcl::Window> mpNext;
    VclPtr<vcl::Window> mpNextOverlap;
    VclPtr<vcl::Window> mpLastFocusWindow;
    VclPtr<vcl::Window> mpDlgCtrlDownWindow;
    std::vector<Link<VclWindowEvent&,void>> maEventListeners;
    int mnEventListenersIteratingCount;
    std::set<Link<VclWindowEvent&,void>> maEventListenersDeleted;
    std::vector<Link<VclWindowEvent&,void>> maChildEventListeners;
    int mnChildEventListenersIteratingCount;
    std::set<Link<VclWindowEvent&,void>> maChildEventListenersDeleted;
    Link<vcl::Window&, bool> maHelpRequestHdl;

    // The canvas interface for this VCL window. Is persistent after the first GetCanvas() call
    css::uno::WeakReference< css::rendering::XCanvas >    mxCanvas;

    vcl::Cursor*        mpCursor;
    PointerStyle        maPointer;
    Fraction            maZoom;
    OUString            maText;
    std::unique_ptr<vcl::Font>
                        mpControlFont;
    Color               maControlForeground;
    Color               maControlBackground;
    sal_Int32           mnLeftBorder;
    sal_Int32           mnTopBorder;
    sal_Int32           mnRightBorder;
    sal_Int32           mnBottomBorder;
    sal_Int32           mnWidthRequest;
    sal_Int32           mnHeightRequest;
    sal_Int32           mnOptimalWidthCache;
    sal_Int32           mnOptimalHeightCache;
    long                mnX;
    long                mnY;
    long                mnAbsScreenX;
    Point               maPos;
    OString             maHelpId;
    OUString            maHelpText;
    OUString            maQuickHelpText;
    OUString            maID;
    InputContext        maInputContext;
    css::uno::Reference< css::awt::XWindowPeer > mxWindowPeer;
    css::uno::Reference< css::accessibility::XAccessible > mxAccessible;
    std::shared_ptr< VclSizeGroup > m_xSizeGroup;
    std::vector< VclPtr<FixedText> > m_aMnemonicLabels;
    std::unique_ptr<ImplAccessibleInfos> mpAccessibleInfos;
    VCLXWindow*         mpVCLXWindow;
    vcl::Region              maWinRegion;            //< region to 'shape' the VCL window (frame coordinates)
    vcl::Region              maWinClipRegion;        //< the (clipping) region that finally corresponds to the VCL window (frame coordinates)
    vcl::Region              maInvalidateRegion;     //< region that has to be redrawn (frame coordinates)
    std::unique_ptr<vcl::Region> mpChildClipRegion;  //< child clip region if CLIPCHILDREN is set (frame coordinates)
    vcl::Region*             mpPaintRegion;          //< only set during Paint() method call (window coordinates)
    WinBits             mnStyle;
    WinBits             mnPrevStyle;
    WindowExtendedStyle mnExtendedStyle;
    WindowType          mnType;
    ControlPart         mnNativeBackground;
    sal_uInt16          mnWaitCount;
    ImplPaintFlags      mnPaintFlags;
    GetFocusFlags       mnGetFocusFlags;
    ParentClipMode      mnParentClipMode;
    ActivateModeFlags   mnActivateMode;
    DialogControlFlags  mnDlgCtrlFlags;
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
    bool                mbFrame:1,
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
                        mbNoUpdate:1,
                        mbNoParentUpdate:1,
                        mbActive:1,
                        mbReallyVisible:1,
                        mbReallyShown:1,
                        mbInInitShow:1,
                        mbChildPtrOverwrite:1,
                        mbNoPtrVisible:1,
                        mbPaintFrame:1,
                        mbInPaint:1,
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
                        mbInDispose:1,
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
                        mbHelpTextDynamic:1,
                        mbFakeFocusSet:1,
                        mbHexpand:1,
                        mbVexpand:1,
                        mbExpand:1,
                        mbFill:1,
                        mbSecondary:1,
                        mbNonHomogeneous:1,
                        mbDoubleBufferingRequested:1;

    css::uno::Reference< css::uno::XInterface > mxDNDListenerContainer;

    const vcl::ILibreOfficeKitNotifier* mpLOKNotifier; ///< To emit the LOK callbacks eg. for dialog tunneling.
    vcl::LOKWindowId mnLOKWindowId; ///< ID of this specific window.
    bool mbLOKParentNotifier;
};

/// Sets up the buffer to have settings matching the window, and restores the original state in the dtor.
class PaintBufferGuard
{
    ImplFrameData* mpFrameData;
    VclPtr<vcl::Window> m_pWindow;
    bool mbBackground;
    Wallpaper maBackground;
    AllSettings maSettings;
    long mnOutOffX;
    long mnOutOffY;
    tools::Rectangle m_aPaintRect;
public:
    PaintBufferGuard(ImplFrameData* pFrameData, vcl::Window* pWindow);
    ~PaintBufferGuard();
    /// If this is called, then the dtor will also copy rRectangle to the window from the buffer, before restoring the state.
    void SetPaintRect(const tools::Rectangle& rRectangle);
    /// Returns either the frame's buffer or the window, in case of no buffering.
    vcl::RenderContext* GetRenderContext();
};

// helper methods

bool ImplHandleMouseEvent( const VclPtr<vcl::Window>& xWindow, MouseNotifyEvent nSVEvent, bool bMouseLeave,
                           long nX, long nY, sal_uInt64 nMsgTime,
                           sal_uInt16 nCode, MouseEventModifiers nMode );
void ImplHandleResize( vcl::Window* pWindow, long nNewWidth, long nNewHeight );

VCL_DLLPUBLIC void ImplWindowStateFromStr(WindowStateData& rData, const OString& rStr);
VCL_DLLPUBLIC OString ImplWindowStateToStr(const WindowStateData& rData);

#endif // INCLUDED_VCL_INC_WINDOW_H

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
