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

#include <vcl/vclenum.hxx>
#include <vcl/dllapi.h>
#include <vcl/dockwin.hxx>
#include <vcl/image.hxx>
#include <vcl/keycod.hxx>
#include <o3tl/typed_flags_set.hxx>

#include <limits>
#include <memory>
#include <vector>

namespace com { namespace sun { namespace star { namespace frame { class XFrame; } } } }
namespace com { namespace sun { namespace star { namespace frame { struct FeatureStateEvent; } } } }
template <class T> class VclStatusListener;

class Timer;
struct ImplToolItem;
struct ImplToolBoxPrivateData;
class  PopupMenu;
class VclMenuEvent;

#define TOOLBOX_STYLE_FLAT          (sal_uInt16(0x0004))

// item ids in the custom menu may not exceed this constant
#define TOOLBOX_MENUITEM_START      (sal_uInt16(0x1000))

// defines for the menubutton
enum class ToolBoxMenuType {
    NONE           = 0x0000,    // no menu at all, scrolling by spin buttons
    ClippedItems   = 0x0001,    // menu will contain "more" indicator
    Customize      = 0x0002     // menu will contain "customization" and "more" indicator
};
namespace o3tl
{
    template<> struct typed_flags<ToolBoxMenuType> : is_typed_flags<ToolBoxMenuType, 0x0003> {};
}

// small, large, size32 force an exact toolbox size for proper alignment
// DontCare will let the toolbox decide about its size
enum class ToolBoxButtonSize
{
    DontCare,
    Small,
    Large,
    Size32,
};

// ToolBoxLayoutMode::Normal   - traditional layout, items are centered in the toolbar
// ToolBoxLayoutMode::LockVert - special mode (currently used for calc input/formula
//                       bar) where item's vertical position is locked, e.g.
//                       toolbox is prevented from centering the items
enum class ToolBoxLayoutMode { Normal, LockVert };

// Position of the text when icon and text are painted
enum class ToolBoxTextPosition { Right, Bottom };

class Idle;
class VCL_DLLPUBLIC ToolBox : public DockingWindow
{
    friend class FloatingWindow;
    friend class ImplTBDragMgr;

public:
    using ImplToolItems = std::vector<ImplToolItem>;

    static constexpr auto APPEND
        = std::numeric_limits<ImplToolItems::size_type>::max();

    static constexpr auto ITEM_NOTFOUND
        = std::numeric_limits<ImplToolItems::size_type>::max();

private:
    struct ImplToolSize
    {
        long mnWidth;
        long mnHeight;
        ImplToolItems::size_type mnLines;
    };

    std::unique_ptr<ImplToolBoxPrivateData>   mpData;
    std::vector<ImplToolSize> maFloatSizes;
    std::unique_ptr<Idle>      mpIdle;
    tools::Rectangle           maUpperRect;
    tools::Rectangle           maLowerRect;
    tools::Rectangle           maPaintRect;
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
    long                mnImagesRotationAngle;
    sal_uInt16          mnLastFocusItemId;
    sal_uInt16          mnOutStyle;
    sal_uInt16          mnHighItemId;
    sal_uInt16          mnCurItemId;
    sal_uInt16          mnDownItemId;
    ImplToolItems::size_type mnCurPos;
    ImplToolItems::size_type mnLines;   // total number of toolbox lines
    ImplToolItems::size_type mnCurLine; // the currently visible line
    ImplToolItems::size_type mnCurLines; // number of lines due to line breaking
    ImplToolItems::size_type mnVisLines; // number of visible lines (for scrolling)
    ImplToolItems::size_type mnFloatLines; // number of lines during floating mode
    ImplToolItems::size_type mnDockLines;
    sal_uInt16          mnMouseModifier;
    bool                mbDrag:1,
                        mbSelection:1,
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
                        mbDragging:1,
                        mbIsKeyEvent:1,
                        mbChangingHighlight:1,
                        mbImagesMirrored:1,
                        mbLineSpacing:1;
    WindowAlign         meAlign;
    WindowAlign         meDockAlign;
    ButtonType          meButtonType;
    PointerStyle        meLastStyle;
    WinBits             mnWinStyle;
    ToolBoxLayoutMode   meLayoutMode;
    ToolBoxTextPosition meTextPosition;
    Link<ToolBox *, void> maClickHdl;
    Link<ToolBox *, void> maDoubleClickHdl;
    Link<ToolBox *, void> maActivateHdl;
    Link<ToolBox *, void> maDeactivateHdl;
    Link<ToolBox *, void> maSelectHdl;
    Link<ToolBox *, void> maMenuButtonHdl;
    Link<StateChangedType const *, void> maStateChangedHandler;
    Link<DataChangedEvent const *, void> maDataChangedHandler;
    /** StatusListener. Notifies about rotated images etc */
    rtl::Reference<VclStatusListener<ToolBox>> mpStatusListener;

public:
    using Window::ImplInit;
private:
    SAL_DLLPRIVATE void InvalidateItem(ImplToolItems::size_type nPosition);
    SAL_DLLPRIVATE void InvalidateSpin(bool bInvalidateUpper = true,
                                       bool bInvalidateLower = true);
    SAL_DLLPRIVATE void InvalidateMenuButton();

    SAL_DLLPRIVATE void            ImplInitToolBoxData();
    SAL_DLLPRIVATE void            ImplInit( vcl::Window* pParent, WinBits nStyle );
    using DockingWindow::ImplInitSettings;
    SAL_DLLPRIVATE void            ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    SAL_DLLPRIVATE ImplToolItem*   ImplGetItem( sal_uInt16 nId ) const;
    SAL_DLLPRIVATE bool            ImplCalcItem();
    SAL_DLLPRIVATE ImplToolItems::size_type ImplCalcBreaks( long nWidth, long* pMaxLineWidth, bool bCalcHorz ) const;
    SAL_DLLPRIVATE void            ImplFormat( bool bResize = false );
    SAL_DLLPRIVATE void            ImplDrawSpin(vcl::RenderContext& rRenderContext);
    SAL_DLLPRIVATE void            ImplDrawSeparator(vcl::RenderContext& rRenderContext, ImplToolItems::size_type nPos, const tools::Rectangle& rRect);
    SAL_DLLPRIVATE void            ImplDrawItem(vcl::RenderContext& rRenderContext, ImplToolItems::size_type nPos, sal_uInt16 nHighlight );
    using Window::ImplInvalidate;
    SAL_DLLPRIVATE void            ImplInvalidate( bool bNewCalc = false, bool bFullPaint = false );
    SAL_DLLPRIVATE void            ImplUpdateItem( ImplToolItems::size_type nIndex = ITEM_NOTFOUND );
    SAL_DLLPRIVATE bool            ImplHandleMouseMove( const MouseEvent& rMEvt, bool bRepeat = false );
    SAL_DLLPRIVATE bool            ImplHandleMouseButtonUp( const MouseEvent& rMEvt, bool bCancel = false );
    SAL_DLLPRIVATE void            ImplChangeHighlight( ImplToolItem const * pItem, bool bNoGrabFocus = false );
    SAL_DLLPRIVATE bool            ImplChangeHighlightUpDn( bool bUp, bool bNoCycle = false );
    SAL_DLLPRIVATE ImplToolItems::size_type ImplGetItemLine( ImplToolItem const * pCurrentItem );
    SAL_DLLPRIVATE ImplToolItem*   ImplGetFirstValidItem( ImplToolItems::size_type nLine );
    SAL_DLLPRIVATE bool            ImplOpenItem( vcl::KeyCode aKeyCode );
    SAL_DLLPRIVATE bool            ImplActivateItem( vcl::KeyCode aKeyCode );
    SAL_DLLPRIVATE void            ImplShowFocus();
    SAL_DLLPRIVATE void            ImplHideFocus();
    SAL_DLLPRIVATE void            ImplUpdateInputEnable();
    SAL_DLLPRIVATE void            ImplFillLayoutData();
    SAL_DLLPRIVATE bool            ImplHasClippedItems();
    SAL_DLLPRIVATE Point           ImplGetPopupPosition( const tools::Rectangle& rRect ) const;
    SAL_DLLPRIVATE bool            ImplIsFloatingMode() const;
    SAL_DLLPRIVATE bool            ImplIsInPopupMode() const;
    SAL_DLLPRIVATE const OUString& ImplGetHelpText( sal_uInt16 nItemId ) const;
    SAL_DLLPRIVATE Size            ImplGetOptimalFloatingSize();
    SAL_DLLPRIVATE bool            ImplHasExternalMenubutton();
    SAL_DLLPRIVATE void            ImplDrawFloatwinBorder(vcl::RenderContext& rRenderContext, ImplToolItem const * pItem );

    DECL_DLLPRIVATE_LINK(    ImplUpdateHdl, Timer*, void );
    DECL_DLLPRIVATE_LINK(    ImplCustomMenuListener, VclMenuEvent&, void );
    DECL_DLLPRIVATE_LINK(    ImplDropdownLongClickHdl, Timer*, void );

                                   ToolBox (const ToolBox &) = delete;
                          ToolBox& operator= (const ToolBox &) = delete;

public:
    SAL_DLLPRIVATE void            ImplFloatControl( bool bStart, FloatingWindow* pWindow );
    SAL_DLLPRIVATE void            ImplDisableFlatButtons();

    SAL_DLLPRIVATE int ImplGetDragWidth() const;
    static SAL_DLLPRIVATE int ImplGetDragWidth( const vcl::RenderContext& rRenderContext,
                                                bool bHorz );
    SAL_DLLPRIVATE void ImplUpdateDragArea() const;
    SAL_DLLPRIVATE void ImplCalcBorder( WindowAlign eAlign, long& rLeft, long& rTop,
                                               long& rRight, long& rBottom ) const;
    SAL_DLLPRIVATE void ImplCheckUpdate();
    static SAL_DLLPRIVATE void ImplDrawGrip(vcl::RenderContext& rRenderContext,
                                            const tools::Rectangle &aDragArea, int nDragWidth,
                                            WindowAlign eAlign, bool bHorz);

    SAL_DLLPRIVATE void ImplDrawGrip(vcl::RenderContext& rRenderContext);
    SAL_DLLPRIVATE void ImplDrawGradientBackground(vcl::RenderContext& rRenderContext);
    SAL_DLLPRIVATE bool ImplDrawNativeBackground(vcl::RenderContext& rRenderContext);
    SAL_DLLPRIVATE void ImplDrawTransparentBackground(const vcl::Region &rRegion);
    SAL_DLLPRIVATE static void ImplDrawConstantBackground(vcl::RenderContext& rRenderContext, const vcl::Region &rRegion, bool bIsInPopupMode);
    SAL_DLLPRIVATE void ImplDrawBackground(vcl::RenderContext& rRenderContext, const tools::Rectangle &rRect);

    SAL_DLLPRIVATE void ImplErase(vcl::RenderContext& rRenderContext, const tools::Rectangle &rRect, bool bHighlight, bool bHasOpenPopup = false );

    SAL_DLLPRIVATE void ImplDrawBorder(vcl::RenderContext& rRenderContext);
    SAL_DLLPRIVATE const ImplToolItem *ImplGetFirstClippedItem() const;
    SAL_DLLPRIVATE Size ImplCalcSize( ImplToolItems::size_type nCalcLines, sal_uInt16 nCalcMode = 0 );
    SAL_DLLPRIVATE void ImplCalcFloatSizes();
    SAL_DLLPRIVATE Size ImplCalcFloatSize( ImplToolItems::size_type& rLines );
    SAL_DLLPRIVATE void ImplCalcMinMaxFloatSize( Size& rMinSize, Size& rMaxSize );
    SAL_DLLPRIVATE void ImplSetMinMaxFloatSize();
    SAL_DLLPRIVATE ImplToolItems::size_type ImplCalcLines( long nToolSize ) const;
    SAL_DLLPRIVATE sal_uInt16 ImplTestLineSize( const Point& rPos ) const;
    SAL_DLLPRIVATE void ImplLineSizing( const Point& rPos, tools::Rectangle& rRect, sal_uInt16 nLineMode );
    static SAL_DLLPRIVATE ImplToolItems::size_type ImplFindItemPos( const ImplToolItem* pItem, const ImplToolItems& rList );
    SAL_DLLPRIVATE void ImplDrawMenuButton(vcl::RenderContext& rRenderContext, bool bHighlight);
    SAL_DLLPRIVATE void ImplDrawButton(vcl::RenderContext& rRenderContext, const tools::Rectangle &rRect, sal_uInt16 highlight, bool bChecked, bool bEnabled, bool bIsWindow);
    SAL_DLLPRIVATE ImplToolItems::size_type ImplCountLineBreaks() const;
    SAL_DLLPRIVATE ImplToolBoxPrivateData* ImplGetToolBoxPrivateData() const { return mpData.get(); }

protected:
    virtual void ApplySettings(vcl::RenderContext& rRenderContext) override;
    void SetCurItemId(sal_uInt16 nSet)
    {
        mnCurItemId = nSet;
    }

public:
    ToolBox(vcl::Window* pParent, WinBits nStyle = 0);
    ToolBox(vcl::Window* pParent, const OString& rID, const OUString& rUIXMLDescription,
        const css::uno::Reference<css::frame::XFrame> &rFrame = css::uno::Reference<css::frame::XFrame>());
    virtual             ~ToolBox() override;
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
    virtual void        Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void        Resize() override;
    virtual void        RequestHelp( const HelpEvent& rHEvt ) override;
    virtual bool        EventNotify( NotifyEvent& rNEvt ) override;
    virtual void        Command( const CommandEvent& rCEvt ) override;
    virtual void        StateChanged( StateChangedType nType ) override;
    virtual void        DataChanged( const DataChangedEvent& rDCEvt ) override;

    virtual void        LoseFocus() override;
    virtual void        KeyInput( const KeyEvent& rKEvt ) override;

    virtual void        ToggleFloatingMode() override;
    virtual void        StartDocking() override;
    virtual bool        Docking( const Point& rPos, tools::Rectangle& rRect ) override;
    virtual void        EndDocking( const tools::Rectangle& rRect, bool bFloatMode ) override;
    virtual void        Resizing( Size& rSize ) override;
    virtual Size        GetOptimalSize() const override;
    virtual void        doDeferredInit(WinBits nBits) override;
    virtual void        queue_resize(StateChangedType eReason = StateChangedType::Layout) override;

    /// Insert a command (like '.uno:Save').
    virtual void        InsertItem( const OUString& rCommand,
                                    const css::uno::Reference<css::frame::XFrame>& rFrame,
                                    ToolBoxItemBits nBits,
                                    const Size& rRequestedSize,
                                    ImplToolItems::size_type nPos = APPEND );
    void                InsertItem( sal_uInt16 nItemId, const Image& rImage,
                                    ToolBoxItemBits nBits = ToolBoxItemBits::NONE,
                                    ImplToolItems::size_type nPos = APPEND );
    void                InsertItem( sal_uInt16 nItemId, const Image& rImage,
                                    const OUString& rText,
                                    ToolBoxItemBits nBits = ToolBoxItemBits::NONE,
                                    ImplToolItems::size_type nPos = APPEND );
    void                InsertItem( sal_uInt16 nItemId, const OUString& rText,
                                    ToolBoxItemBits nBits = ToolBoxItemBits::NONE,
                                    ImplToolItems::size_type nPos = APPEND );
    void                InsertWindow( sal_uInt16 nItemId, vcl::Window* pWindow,
                                      ToolBoxItemBits nBits = ToolBoxItemBits::NONE,
                                      ImplToolItems::size_type nPos = APPEND );
    void                InsertSpace();
    void                InsertSeparator( ImplToolItems::size_type nPos = APPEND, sal_uInt16 nPixSize = 0 );
    void                InsertBreak( ImplToolItems::size_type nPos = APPEND );
    void                RemoveItem( ImplToolItems::size_type nPos );
    void                CopyItem( const ToolBox& rToolBox, sal_uInt16 nItemId );
    void                Clear();

    void                SetButtonType( ButtonType eNewType );
    ButtonType          GetButtonType() const { return meButtonType; }

    // sets a fixed button size (small, large or dontcare (==autosize))
    void                SetToolboxButtonSize( ToolBoxButtonSize eSize );
    ToolBoxButtonSize   GetToolboxButtonSize() const;
    vcl::ImageType      GetImageSize() const;

    void                SetAlign( WindowAlign eNewAlign = WindowAlign::Top );
    WindowAlign         GetAlign() const { return meAlign; }
    bool                IsHorizontal() const { return mbHorz; }

    void                SetLineCount( ImplToolItems::size_type nNewLines );
    void                ShowLine( bool bNext );

    ImplToolItems::size_type GetItemCount() const;
    ToolBoxItemType     GetItemType( ImplToolItems::size_type nPos ) const;
    ImplToolItems::size_type GetItemPos( sal_uInt16 nItemId ) const;
    ImplToolItems::size_type GetItemPos( const Point& rPos ) const;
    sal_uInt16          GetItemId( ImplToolItems::size_type nPos ) const;
    sal_uInt16          GetItemId( const Point& rPos ) const;
    /// Map the command name (like .uno:Save) back to item id.
    sal_uInt16          GetItemId( const OUString& rCommand ) const;
    tools::Rectangle           GetItemRect( sal_uInt16 nItemId );
    tools::Rectangle           GetItemPosRect( ImplToolItems::size_type nPos );
    tools::Rectangle const &   GetOverflowRect() const;

    /// Returns size of the bitmap / text that is inside this toolbox item.
    Size                GetItemContentSize( sal_uInt16 nItemId );

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
    void                SetItemOverlayImage( sal_uInt16 nItemId, const Image& rImage );
    Image               GetItemImage( sal_uInt16 nItemId ) const;
    void                SetItemImageAngle( sal_uInt16 nItemId, long nAngle10 );
    void                SetItemImageMirrorMode( sal_uInt16 nItemId, bool bMirror );
    void                SetItemText( sal_uInt16 nItemId, const OUString& rText );
    const OUString&     GetItemText( sal_uInt16 nItemId ) const;
    void                SetItemWindow( sal_uInt16 nItemId, vcl::Window* pNewWindow );
    vcl::Window*        GetItemWindow( sal_uInt16 nItemId ) const;
    sal_uInt16          GetHighlightItemId() const { return mnHighItemId; }

    void                StartSelection();
    void                EndSelection();

    void                SetItemDown( sal_uInt16 nItemId, bool bDown );

    void                SetItemState( sal_uInt16 nItemId, TriState eState );
    TriState            GetItemState( sal_uInt16 nItemId ) const;

    void                CheckItem( sal_uInt16 nItemId, bool bCheck = true );
    bool                IsItemChecked( sal_uInt16 nItemId ) const;

    void                EnableItem( sal_uInt16 nItemId, bool bEnable = true );
    bool                IsItemEnabled( sal_uInt16 nItemId ) const;

    void                TriggerItem( sal_uInt16 nItemId );

    /// Shows or hides items.
    void                ShowItem(sal_uInt16 nItemId, bool bVisible = true);

    /// Convenience method to hide items (via ShowItem).
    void                HideItem(sal_uInt16 nItemId) { ShowItem( nItemId, false ); }

    bool                IsItemClipped( sal_uInt16 nItemId ) const;
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

    //  window size according to current alignment, floating state and number of lines
    Size                CalcWindowSizePixel();
    //  window size according to current alignment, floating state and a given number of lines
    Size                CalcWindowSizePixel( ImplToolItems::size_type nCalcLines );
    //  window size according to current floating state and a given number of lines and a given alignment
    Size                CalcWindowSizePixel( ImplToolItems::size_type nCalcLines, WindowAlign eAlign );
    // floating window size according to number of lines (uses the number of line breaks)
    Size                CalcFloatingWindowSizePixel();
    // floating window size with a given number of lines
    Size                CalcFloatingWindowSizePixel( ImplToolItems::size_type nCalcLines );
    // automatic window size for popup mode
    Size                CalcPopupWindowSizePixel();

    // computes the smallest useful size when docked, ie with the first item visible only (+drag area and menu button)
    Size                CalcMinimumWindowSizePixel();

    ImplToolItems::size_type GetFloatingLines() const;

    void                SetStyle( WinBits nNewStyle );
    WinBits             GetStyle() const { return mnWinStyle; }

    // enable/disable undocking
    void                Lock( bool bLock );
    // read configuration to determine locking behaviour
    static bool         AlwaysLocked();

    void                SetOutStyle( sal_uInt16 nNewStyle );
    sal_uInt16          GetOutStyle() const { return mnOutStyle; }

    void                EnableCustomize( bool bEnable = true );
    bool                IsCustomize() const { return mbCustomize; }

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
    void                SetStateChangedHdl( const Link<StateChangedType const *, void>& aLink ) { maStateChangedHandler = aLink; }
    void                SetDataChangedHdl( const Link<DataChangedEvent const *, void>& aLink ) { maDataChangedHandler = aLink; }
    void                SetMenuButtonHdl( const Link<ToolBox *, void>& rLink ) { maMenuButtonHdl = rLink; }

    // support for custom menu (eg for configuration)
    // note: this menu will also be used to display currently
    //       clipped toolbox items, so you should only touch
    //       items that you added by yourself
    //       the private toolbox items will only use item ids starting from TOOLBOX_MENUITEM_START
    // to allow for customization of the menu the corresponding handler is called
    // when the menu button was clicked and before the menu is executed
    void                SetMenuType( ToolBoxMenuType aType = ToolBoxMenuType::Customize );
    ToolBoxMenuType     GetMenuType() const;
    bool                IsMenuEnabled() const;
    PopupMenu*          GetMenu() const;
    void                UpdateCustomMenu();
    void                SetMenuExecuteHdl( const Link<ToolBox *, void>& rLink );

    // open custommenu
    void                ExecuteCustomMenu( const tools::Rectangle& rRect = tools::Rectangle() );

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
    tools::Rectangle GetCharacterBounds( sal_uInt16 nItemId, long nIndex );
    // -1 is returned if no character is at that point
    // if an index is found the corresponding item id is filled in (else 0)
    long GetIndexForPoint( const Point& rPoint, sal_uInt16& rItemID );

    static Size         GetDefaultImageSize(ToolBoxButtonSize eToolBoxButtonSize);
    Size                GetDefaultImageSize() const;
    void                ChangeHighlight( ImplToolItems::size_type nPos );
    bool                ChangeHighlightUpDn( bool bUp );


    void SetToolbarLayoutMode( ToolBoxLayoutMode eLayout );
    void statusChanged(const css::frame::FeatureStateEvent& rEvent);

    void SetToolBoxTextPosition( ToolBoxTextPosition ePosition );

    void SetLineSpacing(bool b) { mbLineSpacing = b; }
};

inline void ToolBox::CheckItem( sal_uInt16 nItemId, bool bCheck )
{
    SetItemState( nItemId, bCheck ? TRISTATE_TRUE : TRISTATE_FALSE );
}

inline bool ToolBox::IsItemChecked( sal_uInt16 nItemId ) const
{
    return (GetItemState( nItemId ) == TRISTATE_TRUE);
}

inline Size ToolBox::CalcWindowSizePixel()
{
    return CalcWindowSizePixel( mnLines );
}

inline ToolBox::ImplToolItems::size_type ToolBox::GetFloatingLines() const
{
    return mnFloatLines;
}

#endif // INCLUDED_VCL_TOOLBOX_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
