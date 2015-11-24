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

#ifndef INCLUDED_VCL_TOOLBOX_HXX
#define INCLUDED_VCL_TOOLBOX_HXX

#include <com/sun/star/frame/XFrame.hpp>
#include <rsc/rsc-vcl-shared-types.hxx>
#include <vcl/dllapi.h>
#include <vcl/dockwin.hxx>
#include <vcl/image.hxx>
#include <vcl/timer.hxx>
#include <vcl/idle.hxx>
#include <vector>

class UserDrawEvent;

struct ImplToolItem;
struct ImplToolSize;
struct ImplToolBoxPrivateData;
class  ImplTrackRect;
class  PopupMenu;

#define TOOLBOX_STYLE_FLAT          ((sal_uInt16)0x0004)

#define TOOLBOX_APPEND              ((sal_uInt16)0xFFFF)
#define TOOLBOX_ITEM_NOTFOUND       ((sal_uInt16)0xFFFF)

// item ids in the custom menu may not exceed this constant
#define TOOLBOX_MENUITEM_START      ((sal_uInt16)0x1000)

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

class VCL_DLLPUBLIC ToolBox : public DockingWindow
{
    friend class FloatingWindow;
    friend class ImplTBDragMgr;

private:
    ImplToolBoxPrivateData*   mpData;
    std::vector<ImplToolSize> maFloatSizes;
    ImageList           maImageList;
    Idle                maIdle;
    Rectangle           maUpperRect;
    Rectangle           maLowerRect;
    Rectangle           maOutDockRect;
    Rectangle           maInDockRect;
    Rectangle           maPaintRect;
    VclPtr<FloatingWindow> mpFloatWin;
    sal_uInt16          mnKeyModifier;
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
    sal_uInt16          mnLastFocusItemId;
    sal_uInt16          mnFocusPos;
    sal_uInt16          mnOutStyle;
    sal_uInt16          mnHighItemId;
    sal_uInt16          mnCurItemId;
    sal_uInt16          mnDownItemId;
    sal_uInt16          mnCurPos;
    sal_uInt16          mnLines;        // total number of toolbox lines
    sal_uInt16          mnCurLine;      // the currently visible line
    sal_uInt16          mnCurLines;     // number of lines due to line breaking
    sal_uInt16          mnVisLines;     // number of visible lines (for scrolling)
    sal_uInt16          mnFloatLines;   // number of lines during floating mode
    sal_uInt16          mnDockLines;
    sal_uInt16          mnConfigItem;
    sal_uInt16          mnMouseClicks;
    sal_uInt16          mnMouseModifier;
    bool                mbDrag:1,
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
    Link<ToolBox *, void> maClickHdl;
    Link<ToolBox *, void> maDoubleClickHdl;
    Link<ToolBox *, void> maActivateHdl;
    Link<ToolBox *, void> maDeactivateHdl;
    Link<ToolBox *, void> maSelectHdl;
    Link<CommandEvent const *, void> maCommandHandler;
    Link<StateChangedType const *, void> maStateChangedHandler;
    Link<DataChangedEvent const *, void> maDataChangedHandler;

public:
    using Window::ImplInit;
private:
    SAL_DLLPRIVATE void InvalidateItem(sal_uInt16 nPosition);
    SAL_DLLPRIVATE void InvalidateSpin(bool bUpperIn, bool bLowerIn);
    SAL_DLLPRIVATE void InvalidateMenuButton();

    SAL_DLLPRIVATE void            ImplInit( vcl::Window* pParent, WinBits nStyle );
    using DockingWindow::ImplInitSettings;
    SAL_DLLPRIVATE void            ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    SAL_DLLPRIVATE void            ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE ImplToolItem*   ImplGetItem( sal_uInt16 nId ) const;
    SAL_DLLPRIVATE bool            ImplCalcItem();
    SAL_DLLPRIVATE sal_uInt16      ImplCalcBreaks( long nWidth, long* pMaxLineWidth, bool bCalcHorz );
    SAL_DLLPRIVATE void            ImplFormat( bool bResize = false );
    SAL_DLLPRIVATE void            ImplDrawSpin(vcl::RenderContext& rRenderContext, bool bUpperIn, bool bLowerIn);
    SAL_DLLPRIVATE void            ImplDrawSeparator(vcl::RenderContext& rRenderContext, sal_uInt16 nPos, const Rectangle& rRect);
    SAL_DLLPRIVATE void            ImplDrawItem(vcl::RenderContext& rRenderContext, sal_uInt16 nPos, sal_uInt16 nHighlight = 0, bool bPaint = false, bool bLayout = false );
    using Window::ImplInvalidate;
    SAL_DLLPRIVATE void            ImplInvalidate( bool bNewCalc = false, bool bFullPaint = false );
    SAL_DLLPRIVATE void            ImplUpdateItem( sal_uInt16 nIndex = 0xFFFF );
    SAL_DLLPRIVATE const OUString  ImplConvertMenuString( const OUString& rStr );
    SAL_DLLPRIVATE bool            ImplHandleMouseMove( const MouseEvent& rMEvt, bool bRepeat = false );
    SAL_DLLPRIVATE bool            ImplHandleMouseButtonUp( const MouseEvent& rMEvt, bool bCancel = false );
    SAL_DLLPRIVATE void            ImplChangeHighlight( ImplToolItem* pItem, bool bNoGrabFocus = false );
    SAL_DLLPRIVATE bool            ImplChangeHighlightUpDn( bool bUp, bool bNoCycle = false );
    SAL_DLLPRIVATE sal_uInt16      ImplGetItemLine( ImplToolItem* pCurrentItem );
    SAL_DLLPRIVATE ImplToolItem*   ImplGetFirstValidItem( sal_uInt16 nLine );
    SAL_DLLPRIVATE bool            ImplOpenItem( vcl::KeyCode aKeyCode );
    SAL_DLLPRIVATE bool            ImplActivateItem( vcl::KeyCode aKeyCode );
    SAL_DLLPRIVATE void            ImplShowFocus();
    SAL_DLLPRIVATE void            ImplHideFocus();
    SAL_DLLPRIVATE void            ImplUpdateInputEnable();
    SAL_DLLPRIVATE void            ImplFillLayoutData() const;
    SAL_DLLPRIVATE bool            ImplHasClippedItems();
    SAL_DLLPRIVATE Point           ImplGetPopupPosition( const Rectangle& rRect, const Size& rSize ) const;
    SAL_DLLPRIVATE void            ImplExecuteCustomMenu();
    SAL_DLLPRIVATE bool            ImplIsFloatingMode() const;
    SAL_DLLPRIVATE bool            ImplIsInPopupMode() const;
    SAL_DLLPRIVATE const OUString& ImplGetHelpText( sal_uInt16 nItemId ) const;
    SAL_DLLPRIVATE Size            ImplGetOptimalFloatingSize();
    SAL_DLLPRIVATE bool            ImplHasExternalMenubutton();
    SAL_DLLPRIVATE void            ImplDrawFloatwinBorder(vcl::RenderContext& rRenderContext, ImplToolItem* pItem );

    DECL_DLLPRIVATE_LINK_TYPED(    ImplCallExecuteCustomMenu, void*, void );
    DECL_DLLPRIVATE_LINK_TYPED(    ImplUpdateHdl, Idle*, void );
    DECL_DLLPRIVATE_LINK_TYPED(    ImplCustomMenuListener, VclMenuEvent&, void );
    DECL_DLLPRIVATE_LINK_TYPED(    ImplDropdownLongClickHdl, Timer*, void );

                                   ToolBox (const ToolBox &) = delete;
                          ToolBox& operator= (const ToolBox &) = delete;

public:
    SAL_DLLPRIVATE void            ImplFloatControl( bool bStart, FloatingWindow* pWindow = nullptr );
    SAL_DLLPRIVATE void            ImplDisableFlatButtons();

    static SAL_DLLPRIVATE int ImplGetDragWidth( ToolBox* pThis );
    static SAL_DLLPRIVATE void ImplUpdateDragArea( ToolBox *pThis );
    static SAL_DLLPRIVATE void ImplCalcBorder( WindowAlign eAlign, long& rLeft, long& rTop,
                                               long& rRight, long& rBottom, const ToolBox *pThis );

    SAL_DLLPRIVATE void ImplDrawGrip(vcl::RenderContext& rRenderContext);
    SAL_DLLPRIVATE void ImplDrawGradientBackground(vcl::RenderContext& rRenderContext, ImplDockingWindowWrapper *pWrapper);
    SAL_DLLPRIVATE bool ImplDrawNativeBackground(vcl::RenderContext& rRenderContext, const vcl::Region &rRegion);
    SAL_DLLPRIVATE void ImplDrawTransparentBackground(vcl::RenderContext& rRenderContext, const vcl::Region &rRegion);
    SAL_DLLPRIVATE void ImplDrawConstantBackground(vcl::RenderContext& rRenderContext, const vcl::Region &rRegion, bool bIsInPopupMode);
    SAL_DLLPRIVATE void ImplDrawBackground(vcl::RenderContext& rRenderContext, const Rectangle &rRect);

    SAL_DLLPRIVATE void ImplErase(vcl::RenderContext& rRenderContext, const Rectangle &rRect, bool bHighlight = false, bool bHasOpenPopup = false );

    SAL_DLLPRIVATE void ImplDrawBorder(vcl::RenderContext& rRenderContext);
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
    SAL_DLLPRIVATE void ImplDrawMenuButton(vcl::RenderContext& rRenderContext, bool bHighlight);
    SAL_DLLPRIVATE void ImplDrawButton(vcl::RenderContext& rRenderContext, const Rectangle &rRect, sal_uInt16 highlight, bool bChecked, bool bEnabled, bool bIsWindow);
    static SAL_DLLPRIVATE sal_uInt16 ImplCountLineBreaks( const ToolBox *pThis );
    SAL_DLLPRIVATE ImplToolBoxPrivateData* ImplGetToolBoxPrivateData() const { return mpData; }

protected:
    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;
    void SetCurItemId(sal_uInt16 nSet)
    {
        mnCurItemId = nSet;
    }

public:
                        ToolBox( vcl::Window* pParent, WinBits nStyle = 0 );
                        ToolBox( vcl::Window* pParent, const ResId& rResId );
    virtual             ~ToolBox();
    virtual void        dispose() override;

    virtual void        Click();
    void                DoubleClick();
    virtual void        Activate() override;
    virtual void        Deactivate() override;
    void                Highlight();
    virtual void        Select();

    virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual void        MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual void        MouseMove( const MouseEvent& rMEvt ) override;
    virtual void        Tracking( const TrackingEvent& rTEvt ) override;
    virtual void        Paint( vcl::RenderContext& rRenderContext, const Rectangle& rRect ) override;
    virtual void        Move() override;
    virtual void        Resize() override;
    virtual void        RequestHelp( const HelpEvent& rHEvt ) override;
    virtual bool        Notify( NotifyEvent& rNEvt ) override;
    virtual void        Command( const CommandEvent& rCEvt ) override;
    virtual void        StateChanged( StateChangedType nType ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void        GetFocus() override;
    virtual void        LoseFocus() override;
    virtual void        KeyInput( const KeyEvent& rKEvt ) override;

    virtual bool        PrepareToggleFloatingMode() override;
    virtual void        ToggleFloatingMode() override;
    virtual void        StartDocking() override;
    virtual bool        Docking( const Point& rPos, Rectangle& rRect ) override;
    virtual void        EndDocking( const Rectangle& rRect, bool bFloatMode ) override;
    virtual void        Resizing( Size& rSize ) override;
    virtual Size        GetOptimalSize() const override;

    void                InsertItem( const ResId& rResId,
                                    sal_uInt16 nPos = TOOLBOX_APPEND );
    /// Insert a command (like '.uno:Save').
    virtual void        InsertItem( const OUString& rCommand,
                                    const css::uno::Reference<css::frame::XFrame>& rFrame,
                                    ToolBoxItemBits nBits = ToolBoxItemBits::NONE,
                                    const Size& rRequestedSize = Size(),
                                    sal_uInt16 nPos = TOOLBOX_APPEND );
    void                InsertItem( sal_uInt16 nItemId, const Image& rImage,
                                    ToolBoxItemBits nBits = ToolBoxItemBits::NONE,
                                    sal_uInt16 nPos = TOOLBOX_APPEND );
    void                InsertItem( sal_uInt16 nItemId, const Image& rImage,
                                    const OUString& rText,
                                    ToolBoxItemBits nBits = ToolBoxItemBits::NONE,
                                    sal_uInt16 nPos = TOOLBOX_APPEND );
    void                InsertItem( sal_uInt16 nItemId, const OUString& rText,
                                    ToolBoxItemBits nBits = ToolBoxItemBits::NONE,
                                    sal_uInt16 nPos = TOOLBOX_APPEND );
    void                InsertWindow( sal_uInt16 nItemId, vcl::Window* pWindow,
                                      ToolBoxItemBits nBits = ToolBoxItemBits::NONE,
                                      sal_uInt16 nPos = TOOLBOX_APPEND );
    void                InsertSpace( sal_uInt16 nPos = TOOLBOX_APPEND );
    void                InsertSeparator( sal_uInt16 nPos = TOOLBOX_APPEND, sal_uInt16 nPixSize = 0 );
    void                InsertBreak( sal_uInt16 nPos = TOOLBOX_APPEND );
    void                RemoveItem( sal_uInt16 nPos );
    void                CopyItem( const ToolBox& rToolBox, sal_uInt16 nItemId, sal_uInt16 nNewPos = TOOLBOX_APPEND );
    void                Clear();

    const ImageList&    GetImageList() const { return maImageList; }
    void                SetImageList( const ImageList& rImageList );

    void                SetButtonType( ButtonType eNewType = ButtonType::SYMBOLONLY );
    ButtonType          GetButtonType() const { return meButtonType; }

    // sets a fixed button size (small, large or dontcare (==autosize))
    void                SetToolboxButtonSize( ToolBoxButtonSize eSize );
    ToolBoxButtonSize   GetToolboxButtonSize() const;

    void                SetAlign( WindowAlign eNewAlign = WindowAlign::Top );
    WindowAlign         GetAlign() const { return meAlign; }
    bool                IsHorizontal() const { return mbHorz; }

    void                SetLineCount( sal_uInt16 nNewLines );
    void                ShowLine( bool bNext );

    // Used to enable/disable scrolling one page at a time for toolbar
    void                SetPageScroll( bool b );

    sal_uInt16          GetItemCount() const;
    ToolBoxItemType     GetItemType( sal_uInt16 nPos ) const;
    sal_uInt16          GetItemPos( sal_uInt16 nItemId ) const;
    sal_uInt16          GetItemPos( const Point& rPos ) const;
    sal_uInt16          GetItemId( sal_uInt16 nPos ) const;
    sal_uInt16          GetItemId( const Point& rPos ) const;
    /// Map the command name (like .uno:Save) back to item id.
    sal_uInt16          GetItemId( const OUString& rCommand ) const;
    Rectangle           GetItemRect( sal_uInt16 nItemId ) const;
    Rectangle           GetItemPosRect( sal_uInt16 nPos ) const;

    /// Returns size of the bitmap / text that is inside this toolbox item.
    Size                GetItemContentSize( sal_uInt16 nItemId ) const;

    /// Retrieves the optimal position to place a popup window for this item (subtoolbar or dropdown)
    Point               GetItemPopupPosition( sal_uInt16 nItemId, const Size& rSize ) const;

    sal_uInt16          GetCurItemId() const { return mnCurItemId; }
    sal_uInt16          GetDownItemId() const { return mnDownItemId; }
    sal_uInt16          GetModifier() const { return mnMouseModifier; }
    sal_uInt16          GetKeyModifier() const { return mnKeyModifier; }

    void                SetItemBits( sal_uInt16 nItemId, ToolBoxItemBits nBits );
    ToolBoxItemBits     GetItemBits( sal_uInt16 nItemId ) const;

    void                SetItemExpand( sal_uInt16 nItemId, bool bExpand );

    void                SetItemData( sal_uInt16 nItemId, void* pNewData );
    void*               GetItemData( sal_uInt16 nItemId ) const;
    void                SetItemImage( sal_uInt16 nItemId, const Image& rImage );
    Image               GetItemImage( sal_uInt16 nItemId ) const;
    Image               GetItemImageOriginal( sal_uInt16 nItemId ) const;
    void                SetItemImageAngle( sal_uInt16 nItemId, long nAngle10 );
    void                SetItemImageMirrorMode( sal_uInt16 nItemId, bool bMirror );
    void                SetItemText( sal_uInt16 nItemId, const OUString& rText );
    const OUString&     GetItemText( sal_uInt16 nItemId ) const;
    void                SetItemWindow( sal_uInt16 nItemId, vcl::Window* pNewWindow );
    vcl::Window*        GetItemWindow( sal_uInt16 nItemId ) const;
    sal_uInt16          GetHighlightItemId() const { return mnHighItemId; }

    void                StartSelection();
    void                EndSelection();

    void                SetItemDown( sal_uInt16 nItemId, bool bDown, bool bRelease = true );

    void                SetItemState( sal_uInt16 nItemId, TriState eState );
    TriState            GetItemState( sal_uInt16 nItemId ) const;

    void                CheckItem( sal_uInt16 nItemId, bool bCheck = true );
    bool                IsItemChecked( sal_uInt16 nItemId ) const;

    void                EnableItem( sal_uInt16 nItemId, bool bEnable = true );
    bool                IsItemEnabled( sal_uInt16 nItemId ) const;

    void                TriggerItem( sal_uInt16 nItemId, bool bShift = false, bool bCtrl = false );

    /// Shows or hides items.
    void                ShowItem(sal_uInt16 nItemId, bool bVisible = true);
    /// Overload to provide ShowItem via command id.
    void                ShowItem(const OUString& rCommand, bool bVisible = true) { ShowItem(GetItemId(rCommand), bVisible); }

    /// Convenience method to hide items (via ShowItem).
    void                HideItem(sal_uInt16 nItemId) { ShowItem( nItemId, false ); }
    /// Overload to provide HideItem via command id.
    void                HideItem(const OUString& rCommand) { ShowItem(rCommand, false); }

    bool                IsItemVisible( sal_uInt16 nItemId ) const;
    bool                IsItemReallyVisible( sal_uInt16 nItemId ) const;

    void                SetItemCommand( sal_uInt16 nItemId, const OUString& rCommand );
    const OUString      GetItemCommand( sal_uInt16 nItemId ) const;

    using Window::SetQuickHelpText;
    void                SetQuickHelpText( sal_uInt16 nItemId, const OUString& rText );
    using Window::GetQuickHelpText;
    OUString            GetQuickHelpText( sal_uInt16 nItemId ) const;

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
    // automatic window size for popup mode
    Size                CalcPopupWindowSizePixel() const;

    // computes the smallest useful size when docked, ie with the first item visible only (+drag area and menu button)
    Size                CalcMinimumWindowSizePixel() const;

    void                SetFloatingLines( sal_uInt16 nFloatLines );
    sal_uInt16          GetFloatingLines() const;

    void                SetStyle( WinBits nNewStyle );
    WinBits             GetStyle() const { return mnWinStyle; }

    // enable/disable undocking
    void                Lock( bool bLock = true );
    // read configuration to determine locking behaviour
    static bool         AlwaysLocked();

    void                EnableMenuStrings( bool bEnable = true ) { mbMenuStrings = bEnable; }

    void                SetOutStyle( sal_uInt16 nNewStyle );
    sal_uInt16          GetOutStyle() const { return mnOutStyle; }

    void                EnableCustomize( bool bEnable = true );
    bool                IsCustomize() { return mbCustomize; }

    using DockingWindow::SetHelpText;
    using DockingWindow::GetHelpText;
    using DockingWindow::SetHelpId;
    using DockingWindow::GetHelpId;

    void                SetClickHdl( const Link<ToolBox *, void>& rLink ) { maClickHdl = rLink; }
    void                SetDoubleClickHdl( const Link<ToolBox *, void>& rLink ) { maDoubleClickHdl = rLink; }
    void                SetDropdownClickHdl( const Link<ToolBox *, void>& rLink );
    void                SetActivateHdl( const Link<ToolBox *, void>& rLink ) { maActivateHdl = rLink; }
    void                SetDeactivateHdl( const Link<ToolBox *, void>& rLink ) { maDeactivateHdl = rLink; }
    void                SetSelectHdl( const Link<ToolBox *, void>& rLink ) { maSelectHdl = rLink; }
    const Link<ToolBox *, void>& GetSelectHdl() const { return maSelectHdl; }
    void                SetCommandHdl( const Link<CommandEvent const *, void>& aLink ) { maCommandHandler = aLink; }
    void                SetStateChangedHdl( const Link<StateChangedType const *, void>& aLink ) { maStateChangedHandler = aLink; }
    void                SetDataChangedHdl( const Link<DataChangedEvent const *, void>& aLink ) { maDataChangedHandler = aLink; }

    // support for custom menu (eg for configuration)
    // note: this menu will also be used to display currently
    //       clipped toolbox items, so you should only touch
    //       items that you added by yourself
    //       the private toolbox items will only use item ids starting from TOOLBOX_MENUITEM_START
    // to allow for customization of the menu the coresponding handler is called
    // when the menu button was clicked and before the menu is executed
    void                SetMenuType( sal_uInt16 aType = TOOLBOX_MENUTYPE_CUSTOMIZE );
    sal_uInt16          GetMenuType() const;
    bool                IsMenuEnabled() const;
    PopupMenu*          GetMenu() const;
    void                UpdateCustomMenu();
    void                SetMenuButtonHdl( const Link<ToolBox *, void>& rLink );

    // open custommenu
    void                ExecuteCustomMenu();

    // allow Click Handler to distinguish between mouse and key input
    bool                IsKeyEvent() const { return mbIsKeyEvent; }

    // allows framework to set/query the planned popupmode
    bool                WillUsePopupMode() const;
    void                WillUsePopupMode( bool b);

    // accessibility helpers

    // gets the displayed text
    OUString GetDisplayText() const override;
    // returns the bounding box for the character at index nIndex
    // where nIndex is relative to the starting index of the item
    // with id nItemId (in coordinates of the displaying window)
    Rectangle GetCharacterBounds( sal_uInt16 nItemId, long nIndex ) const;
    // -1 is returned if no character is at that point
    // if an index is found the corresponding item id is filled in (else 0)
    long GetIndexForPoint( const Point& rPoint, sal_uInt16& rItemID ) const;

    static Size         GetDefaultImageSize(bool bLarge);
    Size                GetDefaultImageSize() const;
    void                ChangeHighlight( sal_uInt16 nPos );

    void SetToolbarLayoutMode( ToolBoxLayoutMode eLayout );
};

inline void ToolBox::CheckItem( sal_uInt16 nItemId, bool bCheck )
{
    SetItemState( nItemId, (bCheck) ? TRISTATE_TRUE : TRISTATE_FALSE );
}

inline bool ToolBox::IsItemChecked( sal_uInt16 nItemId ) const
{
    return (GetItemState( nItemId ) == TRISTATE_TRUE);
}

inline Size ToolBox::CalcWindowSizePixel() const
{
    return CalcWindowSizePixel( mnLines );
}


inline void ToolBox::SetFloatingLines( sal_uInt16 nNewLines )
{
    mnFloatLines = nNewLines;
}

inline sal_uInt16 ToolBox::GetFloatingLines() const
{
    return mnFloatLines;
}

#endif // INCLUDED_VCL_TOOLBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
