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

#ifndef _SV_TOOLBOX_HXX
#define _SV_TOOLBOX_HXX

#include <tools/solar.h>
#include <rsc/rsc-vcl-shared-types.hxx>
#include <vcl/dllapi.h>
#include <vcl/dockwin.hxx>
#include <vcl/image.hxx>
#include <vcl/timer.hxx>
#include <vector>

#include <com/sun/star/frame/XFrame.hpp>

class UserDrawEvent;

struct ImplToolItem;
struct ImplToolSize;
struct ImplToolBoxPrivateData;
class  ImplTrackRect;
class  PopupMenu;

namespace vcl
{
    class IImageListProvider;
}

// -------------------------
// - ToolBoxCustomizeEvent -
// -------------------------

#define TOOLBOX_CUSTOMIZE_RESIZE        ((sal_uInt16)0xFFFE)

class VCL_DLLPUBLIC ToolBoxCustomizeEvent
{
private:
    ToolBox*    mpTargetBox;
    void*       mpData;
    sal_uInt16      mnIdFrom;
    sal_uInt16      mnPosTo;

public:
                ToolBoxCustomizeEvent();
                ToolBoxCustomizeEvent( ToolBox* pDropBox,
                                       sal_uInt16 nId, sal_uInt16 nPos = 0,
                                       void* pUserData = NULL );

    ToolBox*    GetTargetBox() const { return mpTargetBox; }
    sal_uInt16      GetTargetPos() const { return mnPosTo; }
    sal_uInt16      GetSourceId() const { return mnIdFrom; }
    void*       GetData() const { return mpData; }
    sal_Bool        IsResized() const;
};

inline ToolBoxCustomizeEvent::ToolBoxCustomizeEvent()
{
    mpTargetBox = NULL;
    mnIdFrom    = 0;
    mnPosTo     = 0;
    mpData      = NULL;
}

inline ToolBoxCustomizeEvent::ToolBoxCustomizeEvent( ToolBox* pDropBox,
                                                     sal_uInt16 nId, sal_uInt16 nPos,
                                                     void* pUserData )
{
    mpTargetBox = pDropBox;
    mnIdFrom    = nId;
    mnPosTo     = nPos;
    mpData      = pUserData;
}

inline sal_Bool ToolBoxCustomizeEvent::IsResized() const
{
    if ( mnPosTo == TOOLBOX_CUSTOMIZE_RESIZE )
        return sal_True;
    else
        return sal_False;
}

// -----------------
// - ToolBox-Types -
// -----------------

#define TOOLBOX_STYLE_FLAT          ((sal_uInt16)0x0004)

#define TOOLBOX_APPEND              ((sal_uInt16)0xFFFF)
#define TOOLBOX_ITEM_NOTFOUND       ((sal_uInt16)0xFFFF)

// item ids in the custom menu may not exceed this constant
#define TOOLBOX_MENUITEM_START      ((sal_uInt16)0xE000)

// defines for the menubutton
#define TOOLBOX_MENUTYPE_NONE           ((sal_uInt16)0x0000)    // no menu at all, scrolling by spin buttons
#define TOOLBOX_MENUTYPE_CLIPPEDITEMS   ((sal_uInt16)0x0001)    // menu will contain "more" indicator
#define TOOLBOX_MENUTYPE_CUSTOMIZE      ((sal_uInt16)0x0002)    // menu will contain "customization" and "more" indicator

// small or large force an exact toolbox size for proper alignemnt
// dontcare will let the toolbox decide about its size
enum ToolBoxButtonSize { TOOLBOX_BUTTONSIZE_DONTCARE, TOOLBOX_BUTTONSIZE_SMALL, TOOLBOX_BUTTONSIZE_LARGE };

// TBX_LAYOUT_NORMAL   - traditional layout, items are centered in the toolbar
// TBX_LAYOUT_LOCKVERT - special mode (currently used for calc input/formula
//                       bar) where item's vertical position is locked, e.g.
//                       toolbox is prevented from centering the items
enum ToolBoxLayoutMode { TBX_LAYOUT_NORMAL, TBX_LAYOUT_LOCKVERT };

struct ImplToolSize
{
    long mnWidth;
    long mnHeight;
    sal_uInt16 mnLines;
};

// -----------
// - ToolBox -
// -----------

class VCL_DLLPUBLIC ToolBox : public DockingWindow
{
    friend class FloatingWindow;
    friend class ImplTBDragMgr;

private:
    ImplToolBoxPrivateData*     mpData;
    std::vector<ImplToolSize> maFloatSizes;
    ImageList           maImageList;
    Timer               maTimer;
    Rectangle           maUpperRect;
    Rectangle           maLowerRect;
    Rectangle           maOutDockRect;
    Rectangle           maInDockRect;
    Rectangle           maPaintRect;
    FloatingWindow*     mpFloatWin;
    sal_uInt16              mnKeyModifier;
    long                mnDX;
    long                mnDY;
    long                mnMaxItemWidth;    // max item width
    long                mnMaxItemHeight;   // max item height (for standard items)
    long                mnWinHeight;    // max window height (for window items)
    long                mnLeftBorder;   // inner border
    long                mnTopBorder;
    long                mnRightBorder;
    long                mnBottomBorder;
    long                mnLastResizeDY;
    long                mnActivateCount;
    sal_uInt16              mnLastFocusItemId;
    sal_uInt16              mnFocusPos;
    sal_uInt16              mnOutStyle;
    sal_uInt16              mnHighItemId;
    sal_uInt16              mnCurItemId;
    sal_uInt16              mnDownItemId;
    sal_uInt16              mnCurPos;
    sal_uInt16              mnLines;        // total number of toolbox lines
    sal_uInt16              mnCurLine;      // the currently visible line
    sal_uInt16              mnCurLines;     // number of lines due to line breaking
    sal_uInt16              mnVisLines;     // number of visible lines (for scrolling)
    sal_uInt16              mnFloatLines;   // number of lines during floating mode
    sal_uInt16              mnDockLines;
    sal_uInt16              mnConfigItem;
    sal_uInt16              mnMouseClicks;
    sal_uInt16              mnMouseModifier;
    unsigned int        mbDrag:1,
                        mbSelection:1,
                        mbCommandDrag:1,
                        mbUpper:1,
                        mbLower:1,
                        mbIn:1,
                        mbCalc:1,
                        mbFormat:1,
                        mbFullPaint:1,
                        mbHorz:1,
                        mbScroll:1,
                        mbLastFloatMode:1,
                        mbCustomize:1,
                        mbCustomizeMode:1,
                        mbDragging:1,
                        mbMenuStrings:1,
                        mbIsShift:1,
                        mbIsKeyEvent:1,
                        mbChangingHighlight:1;
    WindowAlign         meAlign;
    WindowAlign         meDockAlign;
    ButtonType          meButtonType;
    PointerStyle        meLastStyle;
    WinBits             mnWinStyle;
    ToolBoxLayoutMode   meLayoutMode;
    Link                maClickHdl;
    Link                maDoubleClickHdl;
    Link                maActivateHdl;
    Link                maDeactivateHdl;
    Link                maHighlightHdl;
    Link                maSelectHdl;
    Link                maCommandHandler;
    Link                maStateChangedHandler;
    Link                maDataChangedHandler;

    public:
    using Window::ImplInit;
    private:
    SAL_DLLPRIVATE void            ImplInit( Window* pParent, WinBits nStyle );
    using DockingWindow::ImplInitSettings;
    SAL_DLLPRIVATE void            ImplInitSettings( sal_Bool bFont, sal_Bool bForeground, sal_Bool bBackground );
    SAL_DLLPRIVATE void            ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE ImplToolItem*   ImplGetItem( sal_uInt16 nId ) const;
    SAL_DLLPRIVATE sal_Bool            ImplCalcItem();
    SAL_DLLPRIVATE sal_uInt16          ImplCalcBreaks( long nWidth, long* pMaxLineWidth, sal_Bool bCalcHorz );
    SAL_DLLPRIVATE void            ImplFormat( sal_Bool bResize = sal_False );
    SAL_DLLPRIVATE void            ImplDrawSpin( sal_Bool bUpperIn, sal_Bool bLowerIn );
    SAL_DLLPRIVATE void            ImplDrawSeparator( sal_uInt16 nPos, Rectangle rRect );
    SAL_DLLPRIVATE void            ImplDrawItem( sal_uInt16 nPos, sal_uInt16 nHighlight = 0, sal_Bool bPaint = sal_False, sal_Bool bLayout = sal_False );
    using Window::ImplInvalidate;
    SAL_DLLPRIVATE void            ImplInvalidate( sal_Bool bNewCalc = sal_False, sal_Bool bFullPaint = sal_False );
    SAL_DLLPRIVATE void            ImplUpdateItem( sal_uInt16 nIndex = 0xFFFF );
    SAL_DLLPRIVATE const OUString ImplConvertMenuString( const XubString& rStr );
    SAL_DLLPRIVATE sal_Bool            ImplHandleMouseMove( const MouseEvent& rMEvt, sal_Bool bRepeat = sal_False );
    SAL_DLLPRIVATE sal_Bool            ImplHandleMouseButtonUp( const MouseEvent& rMEvt, sal_Bool bCancel = sal_False );
    SAL_DLLPRIVATE void            ImplChangeHighlight( ImplToolItem* pItem, sal_Bool bNoGrabFocus = sal_False );
    SAL_DLLPRIVATE sal_Bool            ImplChangeHighlightUpDn( sal_Bool bUp, sal_Bool bNoCycle = sal_False );
    SAL_DLLPRIVATE sal_uInt16          ImplGetItemLine( ImplToolItem* pCurrentItem );
    SAL_DLLPRIVATE ImplToolItem*   ImplGetFirstValidItem( sal_uInt16 nLine );
    SAL_DLLPRIVATE sal_Bool            ImplOpenItem( KeyCode aKeyCode );
    SAL_DLLPRIVATE sal_Bool            ImplActivateItem( KeyCode aKeyCode );
    SAL_DLLPRIVATE void            ImplShowFocus();
    SAL_DLLPRIVATE void            ImplHideFocus();
    SAL_DLLPRIVATE void            ImplUpdateInputEnable();
    SAL_DLLPRIVATE void            ImplFillLayoutData() const;
    SAL_DLLPRIVATE sal_Bool            ImplHasClippedItems();
    SAL_DLLPRIVATE Point           ImplGetPopupPosition( const Rectangle& rRect, const Size& rSize ) const;
    SAL_DLLPRIVATE void            ImplExecuteCustomMenu();
    SAL_DLLPRIVATE sal_Bool            ImplIsFloatingMode() const;
    SAL_DLLPRIVATE sal_Bool            ImplIsInPopupMode() const;
    SAL_DLLPRIVATE const OUString& ImplGetHelpText( sal_uInt16 nItemId ) const;
    SAL_DLLPRIVATE Size            ImplGetOptimalFloatingSize();
    SAL_DLLPRIVATE sal_Bool            ImplHasExternalMenubutton();
    SAL_DLLPRIVATE void            ImplDrawFloatwinBorder( ImplToolItem* pItem );

    DECL_DLLPRIVATE_LINK(          ImplCallExecuteCustomMenu, void* );
    DECL_DLLPRIVATE_LINK(          ImplUpdateHdl, void* );
    DECL_DLLPRIVATE_LINK(          ImplResetAutoSizeTriesHdl, void* );
    DECL_DLLPRIVATE_LINK(          ImplCustomMenuListener, VclMenuEvent* );
    DECL_DLLPRIVATE_LINK(          ImplDropdownLongClickHdl, void* );

    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE                 ToolBox (const ToolBox &);
    SAL_DLLPRIVATE        ToolBox& operator= (const ToolBox &);

    SAL_DLLPRIVATE void            ImplUpdateImageList(); // called if StateChanged
public:
    SAL_DLLPRIVATE void            ImplFloatControl( sal_Bool bStart, FloatingWindow* pWindow = NULL );
    SAL_DLLPRIVATE void            ImplDisableFlatButtons();

    static SAL_DLLPRIVATE int ImplGetDragWidth( ToolBox* pThis );
    static SAL_DLLPRIVATE void ImplUpdateDragArea( ToolBox *pThis );
    static SAL_DLLPRIVATE void ImplCalcBorder( WindowAlign eAlign, long& rLeft, long& rTop,
                                               long& rRight, long& rBottom, const ToolBox *pThis );
    static SAL_DLLPRIVATE void ImplDrawGrip( ToolBox* pThis );
    static SAL_DLLPRIVATE void ImplDrawGradientBackground( ToolBox* pThis, ImplDockingWindowWrapper *pWrapper );
    static SAL_DLLPRIVATE sal_Bool ImplDrawNativeBackground( ToolBox* pThis, const Region &rRegion );
    static SAL_DLLPRIVATE void ImplDrawTransparentBackground( ToolBox* pThis, const Region &rRegion );
    static SAL_DLLPRIVATE void ImplDrawConstantBackground( ToolBox* pThis, const Region &rRegion, sal_Bool bIsInPopupMode );
    static SAL_DLLPRIVATE void ImplDrawBackground( ToolBox* pThis, const Rectangle &rRect );
    static SAL_DLLPRIVATE void ImplErase( ToolBox* pThis, const Rectangle &rRect, sal_Bool bHighlight = sal_False, sal_Bool bHasOpenPopup = sal_False );
    static SAL_DLLPRIVATE void ImplDrawBorder( ToolBox* pWin );
    static SAL_DLLPRIVATE const ImplToolItem *ImplGetFirstClippedItem( const ToolBox* pThis );
    static SAL_DLLPRIVATE Size ImplCalcSize( const ToolBox* pThis, sal_uInt16 nCalcLines, sal_uInt16 nCalcMode = 0 );
    static SAL_DLLPRIVATE void ImplCalcFloatSizes( ToolBox* pThis );
    static SAL_DLLPRIVATE Size ImplCalcFloatSize( ToolBox* pThis, sal_uInt16& rLines );
    static SAL_DLLPRIVATE void ImplCalcMinMaxFloatSize( ToolBox* pThis, Size& rMinSize, Size& rMaxSize );
    static SAL_DLLPRIVATE void ImplSetMinMaxFloatSize( ToolBox *pThis );
    static SAL_DLLPRIVATE sal_uInt16 ImplCalcLines( ToolBox* pThis, long nToolSize );
    static SAL_DLLPRIVATE sal_uInt16 ImplTestLineSize( ToolBox* pThis, const Point& rPos );
    static SAL_DLLPRIVATE void ImplLineSizing( ToolBox* pThis, const Point& rPos, Rectangle& rRect, sal_uInt16 nLineMode );
    static SAL_DLLPRIVATE sal_uInt16 ImplFindItemPos( ToolBox* pBox, const Point& rPos );
    static SAL_DLLPRIVATE sal_uInt16 ImplFindItemPos( const ImplToolItem* pItem, const std::vector< ImplToolItem >& rList );
    static SAL_DLLPRIVATE void ImplDrawMenubutton( ToolBox *pThis, sal_Bool bHighlight );
    static SAL_DLLPRIVATE sal_uInt16 ImplCountLineBreaks( const ToolBox *pThis );
    SAL_DLLPRIVATE ImplToolBoxPrivateData* ImplGetToolBoxPrivateData() const { return mpData; }

protected:
    void                SetCurItemId(sal_uInt16 nSet) { mnCurItemId = nSet; }

public:
                        ToolBox( Window* pParent, WinBits nStyle = 0 );
                        ToolBox( Window* pParent, const ResId& rResId );
                        ~ToolBox();

    virtual void        Click();
    virtual void        DoubleClick();
    virtual void        Activate();
    virtual void        Deactivate();
    virtual void        Highlight();
    virtual void        Select();
    virtual void        Customize( const ToolBoxCustomizeEvent& rCEvt );
    virtual void        UserDraw( const UserDrawEvent& rUDEvt );

    virtual void        MouseButtonDown( const MouseEvent& rMEvt );
    virtual void        MouseButtonUp( const MouseEvent& rMEvt );
    virtual void        MouseMove( const MouseEvent& rMEvt );
    virtual void        Tracking( const TrackingEvent& rTEvt );
    virtual void        Paint( const Rectangle& rRect );
    virtual void        Move();
    virtual void        Resize();
    virtual void        RequestHelp( const HelpEvent& rHEvt );
    virtual long        Notify( NotifyEvent& rNEvt );
    virtual void        Command( const CommandEvent& rCEvt );
    virtual void        StateChanged( StateChangedType nType );
    virtual void        DataChanged( const DataChangedEvent& rDCEvt );

    virtual void        GetFocus();
    virtual void        LoseFocus();
    virtual void        KeyInput( const KeyEvent& rKEvt );

    virtual sal_Bool        PrepareToggleFloatingMode();
    virtual void        ToggleFloatingMode();
    virtual void        StartDocking();
    virtual sal_Bool        Docking( const Point& rPos, Rectangle& rRect );
    virtual void        EndDocking( const Rectangle& rRect, sal_Bool bFloatMode );
    virtual void        Resizing( Size& rSize );
    virtual Size        GetOptimalSize() const;

    void                InsertItem( const ResId& rResId,
                                    sal_uInt16 nPos = TOOLBOX_APPEND );
    /// Insert a command (like '.uno:Save').
    virtual void        InsertItem( const OUString& rCommand,
                                    const com::sun::star::uno::Reference<com::sun::star::frame::XFrame>& rFrame,
                                    ToolBoxItemBits nBits = 0,
                                    const Size& rRequestedSize = Size(),
                                    sal_uInt16 nPos = TOOLBOX_APPEND );
    void                InsertItem( sal_uInt16 nItemId, const Image& rImage,
                                    ToolBoxItemBits nBits = 0,
                                    sal_uInt16 nPos = TOOLBOX_APPEND );
    void                InsertItem( sal_uInt16 nItemId, const Image& rImage,
                                    const XubString& rText,
                                    ToolBoxItemBits nBits = 0,
                                    sal_uInt16 nPos = TOOLBOX_APPEND );
    void                InsertItem( sal_uInt16 nItemId, const XubString& rText,
                                    ToolBoxItemBits nBits = 0,
                                    sal_uInt16 nPos = TOOLBOX_APPEND );
    void                InsertWindow( sal_uInt16 nItemId, Window* pWindow,
                                      ToolBoxItemBits nBits = 0,
                                      sal_uInt16 nPos = TOOLBOX_APPEND );
    void                InsertSpace( sal_uInt16 nPos = TOOLBOX_APPEND );
    void                InsertSeparator( sal_uInt16 nPos = TOOLBOX_APPEND,
                                         sal_uInt16 nPixSize = 0 );
    void                InsertBreak( sal_uInt16 nPos = TOOLBOX_APPEND );
    void                RemoveItem( sal_uInt16 nPos );
    void                CopyItem( const ToolBox& rToolBox, sal_uInt16 nItemId,
                                  sal_uInt16 nNewPos = TOOLBOX_APPEND );
    void                Clear();

    const ImageList&    GetImageList() const { return maImageList; }
    void                SetImageList( const ImageList& rImageList );

    void                SetButtonType( ButtonType eNewType = BUTTON_SYMBOL );
    ButtonType          GetButtonType() const { return meButtonType; }

    // sets a fixed button size (small, large or dontcare (==autosize))
    void                SetToolboxButtonSize( ToolBoxButtonSize eSize );
    ToolBoxButtonSize   GetToolboxButtonSize() const;

    void                SetAlign( WindowAlign eNewAlign = WINDOWALIGN_TOP );
    WindowAlign         GetAlign() const { return meAlign; }
    sal_Bool                IsHorizontal() const { return mbHorz; }

    void                SetLineCount( sal_uInt16 nNewLines );
    sal_uInt16              GetLineCount() const { return mnLines; }
    sal_uInt16              GetCurLine() const { return mnCurLine; }
    void                ShowLine( sal_Bool bNext );

    // Used to enable/disable scrolling one page at a time for toolbar
    void                SetPageScroll( sal_Bool b );

    sal_uInt16              GetItemCount() const;
    ToolBoxItemType     GetItemType( sal_uInt16 nPos ) const;
    sal_uInt16              GetItemPos( sal_uInt16 nItemId ) const;
    sal_uInt16              GetItemPos( const Point& rPos ) const;
    sal_uInt16              GetItemId( sal_uInt16 nPos ) const;
    sal_uInt16              GetItemId( const Point& rPos ) const;
    /// Map the command name (like .uno:Save) back to item id.
    sal_uInt16          GetItemId( const OUString& rCommand ) const;
    Rectangle           GetItemRect( sal_uInt16 nItemId ) const;
    Rectangle           GetItemPosRect( sal_uInt16 nPos ) const;

    /// Returns size of the bitmap / text that is inside this toolbox item.
    Size                GetItemContentSize( sal_uInt16 nItemId ) const;

    /// Retrieves the optimal position to place a popup window for this item (subtoolbar or dropdown)
    Point               GetItemPopupPosition( sal_uInt16 nItemId, const Size& rSize ) const;

    Rectangle           GetScrollRect() const;
    sal_uInt16              GetCurItemId() const { return mnCurItemId; }
    sal_uInt16              GetDownItemId() const { return mnDownItemId; }
    sal_uInt16              GetClicks() const { return mnMouseClicks; }
    sal_uInt16              GetModifier() const { return mnMouseModifier; }
    sal_uInt16              GetKeyModifier() const { return mnKeyModifier; }

    void                SetItemBits( sal_uInt16 nItemId, ToolBoxItemBits nBits );
    ToolBoxItemBits     GetItemBits( sal_uInt16 nItemId ) const;

    void                SetItemData( sal_uInt16 nItemId, void* pNewData );
    void*               GetItemData( sal_uInt16 nItemId ) const;
    void                SetItemImage( sal_uInt16 nItemId, const Image& rImage );
    Image               GetItemImage( sal_uInt16 nItemId ) const;
    void                SetItemImageAngle( sal_uInt16 nItemId, long nAngle10 );
    void                SetItemImageMirrorMode( sal_uInt16 nItemId, sal_Bool bMirror );
    void                SetItemText( sal_uInt16 nItemId, const OUString& rText );
    const OUString&     GetItemText( sal_uInt16 nItemId ) const;
    void                SetItemWindow( sal_uInt16 nItemId, Window* pNewWindow );
    Window*             GetItemWindow( sal_uInt16 nItemId ) const;
    sal_uInt16              GetHighlightItemId() const { return mnHighItemId; }

    void                StartSelection();
    void                EndSelection();

    void                SetItemDown( sal_uInt16 nItemId, sal_Bool bDown, sal_Bool bRelease = sal_True );

    void                SetItemState( sal_uInt16 nItemId, TriState eState );
    TriState            GetItemState( sal_uInt16 nItemId ) const;

    void                CheckItem( sal_uInt16 nItemId, sal_Bool bCheck = sal_True );
    sal_Bool                IsItemChecked( sal_uInt16 nItemId ) const;

    void                EnableItem( sal_uInt16 nItemId, sal_Bool bEnable = sal_True );
    sal_Bool                IsItemEnabled( sal_uInt16 nItemId ) const;

    void                TriggerItem( sal_uInt16 nItemId, sal_Bool bShift = sal_False, sal_Bool bCtrl = sal_False );
    void                ShowItem( sal_uInt16 nItemId, sal_Bool bVisible = sal_True );
    void                HideItem( sal_uInt16 nItemId ) { ShowItem( nItemId, sal_False ); }
    sal_Bool                IsItemVisible( sal_uInt16 nItemId ) const;
    sal_Bool                IsItemReallyVisible( sal_uInt16 nItemId ) const;

    void                SetItemCommand( sal_uInt16 nItemId, const OUString& rCommand );
    const OUString      GetItemCommand( sal_uInt16 nItemId ) const;

    using Window::SetQuickHelpText;
    void                SetQuickHelpText( sal_uInt16 nItemId, const OUString& rText );
    using Window::GetQuickHelpText;
    const OUString&     GetQuickHelpText( sal_uInt16 nItemId ) const;

    void                SetHelpText( sal_uInt16 nItemId, const OUString& rText );
    const OUString&     GetHelpText( sal_uInt16 nItemId ) const;

    void                SetHelpId( sal_uInt16 nItemId, const OString& rHelpId );
    OString             GetHelpId( sal_uInt16 nItemId ) const;

    //  window size according to current alignment, floating state and number of lines
    Size                CalcWindowSizePixel() const;
    //  window size according to current alignment, floating state and a given number of lines
    Size                CalcWindowSizePixel( sal_uInt16 nCalcLines ) const;
    //  window size according to current floating state and a given number of lines and a given alignment
    Size                CalcWindowSizePixel( sal_uInt16 nCalcLines, WindowAlign eAlign ) const;
    // floating window size according to number of lines (uses the number of line breaks)
    Size                CalcFloatingWindowSizePixel() const;
    // floating window size with a given number of lines
    Size                CalcFloatingWindowSizePixel( sal_uInt16 nCalcLines ) const;
    // automatic window size for popoup mode
    Size                CalcPopupWindowSizePixel() const;

    // computes the smallest useful size when docked, ie with the first item visible only (+drag area and menu button)
    Size                CalcMinimumWindowSizePixel() const;

    void                SetDockingRects( const Rectangle& rOutRect,
                                         const Rectangle& rInRect );
    void                SetFloatingLines( sal_uInt16 nFloatLines );
    sal_uInt16              GetFloatingLines() const;

    void                SetStyle( WinBits nNewStyle ) { mnWinStyle = nNewStyle; }
    WinBits             GetStyle() const { return mnWinStyle; }

    // enable/disable undocking
    void                Lock( sal_Bool bLock = sal_True );

    // read configuration to determine locking behaviour
    static sal_Bool         AlwaysLocked();

    void                EnableMenuStrings( sal_Bool bEnable = sal_True ) { mbMenuStrings = (bEnable != 0); }
    sal_Bool                IsMenuStringsEnabled() const { return mbMenuStrings; }

    void                SetOutStyle( sal_uInt16 nNewStyle );
    sal_uInt16              GetOutStyle() const { return mnOutStyle; }

    void                EnableCustomize( sal_Bool bEnable = sal_True );
    sal_Bool                IsCustomize() { return mbCustomize; }
    sal_Bool                IsInCustomizeMode() const { return mbCustomizeMode; }

    void                SetHelpText( const OUString& rText )
                            { DockingWindow::SetHelpText( rText ); }
    const OUString&     GetHelpText() const
                            { return DockingWindow::GetHelpText(); }

    void                SetHelpId( const OString& rId )
                            { DockingWindow::SetHelpId( rId ); }
    const OString&      GetHelpId() const
                            { return DockingWindow::GetHelpId(); }

    void                SetClickHdl( const Link& rLink ) { maClickHdl = rLink; }
    const Link&         GetClickHdl() const { return maClickHdl; }
    void                SetDoubleClickHdl( const Link& rLink ) { maDoubleClickHdl = rLink; }
    const Link&         GetDoubleClickHdl() const { return maDoubleClickHdl; }
    void                SetDropdownClickHdl( const Link& rLink );
    const Link&         GetDropdownClickHdl() const;
    void                SetActivateHdl( const Link& rLink ) { maActivateHdl = rLink; }
    const Link&         GetActivateHdl() const { return maActivateHdl; }
    void                SetDeactivateHdl( const Link& rLink ) { maDeactivateHdl = rLink; }
    const Link&         GetDeactivateHdl() const { return maDeactivateHdl; }
    void                SetHighlightHdl( const Link& rLink ) { maHighlightHdl = rLink; }
    const Link&         GetHighlightHdl() const { return maHighlightHdl; }
    void                SetSelectHdl( const Link& rLink ) { maSelectHdl = rLink; }
    const Link&         GetSelectHdl() const { return maSelectHdl; }
    void                SetCommandHdl( const Link& aLink ) { maCommandHandler = aLink; }
    const Link&         GetCommandHdl() const { return maCommandHandler; }
    void                SetStateChangedHdl( const Link& aLink ) { maStateChangedHandler = aLink; }
    const Link&         GetStateChangedHdl() const { return maStateChangedHandler; }
    void                SetDataChangedHdl( const Link& aLink ) { maDataChangedHandler = aLink; }
    const Link&         GetDataChangedHdl() { return maDataChangedHandler; }

    // support for custom menu (eg for configuration)
    // note: this menu will also be used to display currently
    //       clipped toolbox items, so you should only touch
    //       items that you added by yourself
    //       the private toolbox items will only use item ids starting from TOOLBOX_MENUITEM_START
    // to allow for customization of the menu the coresponding handler is called
    // when the menu button was clicked and before the menu is executed
    void                SetMenuType( sal_uInt16 aType = TOOLBOX_MENUTYPE_CUSTOMIZE );
    sal_uInt16              GetMenuType() const;
    sal_Bool                IsMenuEnabled() const;
    PopupMenu*          GetMenu() const;
    void                UpdateCustomMenu();
    void                SetMenuButtonHdl( const Link& rLink );

    // open custommenu
    void                ExecuteCustomMenu();

    // allow Click Handler to detect special key
    bool                IsShift() const { return mbIsShift; }
    // allow Click Handler to distinguish between mouse and key input
    sal_Bool                IsKeyEvent() const { return mbIsKeyEvent; }

    // allows framework to set/query the planned popupmode
    sal_Bool                WillUsePopupMode() const;
    void                WillUsePopupMode( sal_Bool b);

    // accessibility helpers

    // gets the displayed text
    OUString GetDisplayText() const;
    // returns the bounding box for the character at index nIndex
    // where nIndex is relative to the starting index of the item
    // with id nItemId (in coordinates of the displaying window)
    Rectangle GetCharacterBounds( sal_uInt16 nItemId, long nIndex ) const;
    // -1 is returned if no character is at that point
    // if an index is found the corresponding item id is filled in (else 0)
    long GetIndexForPoint( const Point& rPoint, sal_uInt16& rItemID ) const;

    static const Size&  GetDefaultImageSize(bool bLarge);
    const Size&         GetDefaultImageSize() const;
    void                ChangeHighlight( sal_uInt16 nPos );

    void SetImageListProvider(vcl::IImageListProvider* _pProvider);
    void SetToolbarLayoutMode( ToolBoxLayoutMode eLayout );
};

inline void ToolBox::CheckItem( sal_uInt16 nItemId, sal_Bool bCheck )
{
    SetItemState( nItemId, (bCheck) ? STATE_CHECK : STATE_NOCHECK );
}

inline sal_Bool ToolBox::IsItemChecked( sal_uInt16 nItemId ) const
{
    return (GetItemState( nItemId ) == STATE_CHECK);
}

inline Size ToolBox::CalcWindowSizePixel() const
{
    return CalcWindowSizePixel( mnLines );
}

inline Rectangle ToolBox::GetScrollRect() const
{
    return maUpperRect.GetUnion( maLowerRect );
}

inline void ToolBox::SetDockingRects( const Rectangle& rOutRect,
                                      const Rectangle& rInRect )
{
    maOutDockRect = rOutRect;
    maInDockRect = rInRect;
}

inline void ToolBox::SetFloatingLines( sal_uInt16 nNewLines )
{
    mnFloatLines = nNewLines;
}

inline sal_uInt16 ToolBox::GetFloatingLines() const
{
    return mnFloatLines;
}

#endif  // _SV_TOOLBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
