/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: toolbox.hxx,v $
 * $Revision: 1.3 $
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

#ifndef _SV_TOOLBOX_HXX
#define _SV_TOOLBOX_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/dockwin.hxx>
#include <vcl/image.hxx>
#include <vcl/timer.hxx>
#include <vcl/virdev.hxx>

class UserDrawEvent;

struct ImplToolItem;
struct ImplToolSizeArray;
struct ImplToolSize;
struct ImplToolBoxPrivateData;
class  ImplTrackRect;
class  PopupMenu;

// -------------------------
// - ToolBoxCustomizeEvent -
// -------------------------

#define TOOLBOX_CUSTOMIZE_RESIZE        ((USHORT)0xFFFE)

class VCL_DLLPUBLIC ToolBoxCustomizeEvent
{
private:
    ToolBox*    mpTargetBox;
    void*       mpData;
    USHORT      mnIdFrom;
    USHORT      mnPosTo;

public:
                ToolBoxCustomizeEvent();
                ToolBoxCustomizeEvent( ToolBox* pDropBox,
                                       USHORT nId, USHORT nPos = 0,
                                       void* pUserData = NULL );

    ToolBox*    GetTargetBox() const { return mpTargetBox; }
    USHORT      GetTargetPos() const { return mnPosTo; }
    USHORT      GetSourceId() const { return mnIdFrom; }
    void*       GetData() const { return mpData; }
    BOOL        IsResized() const;
};

inline ToolBoxCustomizeEvent::ToolBoxCustomizeEvent()
{
    mpTargetBox = NULL;
    mnIdFrom    = 0;
    mnPosTo     = 0;
    mpData      = NULL;
}

inline ToolBoxCustomizeEvent::ToolBoxCustomizeEvent( ToolBox* pDropBox,
                                                     USHORT nId, USHORT nPos,
                                                     void* pUserData )
{
    mpTargetBox = pDropBox;
    mnIdFrom    = nId;
    mnPosTo     = nPos;
    mpData      = pUserData;
}

inline BOOL ToolBoxCustomizeEvent::IsResized() const
{
    if ( mnPosTo == TOOLBOX_CUSTOMIZE_RESIZE )
        return TRUE;
    else
        return FALSE;
}

// -------------------
// - ToolBoxItemBits -
// -------------------

typedef USHORT ToolBoxItemBits;

// --------------------------
// - Bits fuer ToolBoxItems -
// --------------------------

// By changes you must also change: rsc/vclrsc.hxx
#define TIB_CHECKABLE           ((ToolBoxItemBits)0x0001)
#define TIB_RADIOCHECK          ((ToolBoxItemBits)0x0002)
#define TIB_AUTOCHECK           ((ToolBoxItemBits)0x0004)
#define TIB_LEFT                ((ToolBoxItemBits)0x0008)
#define TIB_AUTOSIZE            ((ToolBoxItemBits)0x0010)
#define TIB_DROPDOWN            ((ToolBoxItemBits)0x0020)
#define TIB_REPEAT              ((ToolBoxItemBits)0x0040)
#define TIB_DROPDOWNONLY        ((ToolBoxItemBits)0x0080 | TIB_DROPDOWN)    // this button has only drop down functionality

// -----------------
// - ToolBox-Types -
// -----------------

#define TOOLBOX_STYLE_OUTBUTTON     ((USHORT)0x0001)
#define TOOLBOX_STYLE_HANDPOINTER   ((USHORT)0x0002)
#define TOOLBOX_STYLE_FLAT          ((USHORT)0x0004)

#define TOOLBOX_APPEND              ((USHORT)0xFFFF)
#define TOOLBOX_ITEM_NOTFOUND       ((USHORT)0xFFFF)

// item ids in the custom menu may not exceed this constant
#define TOOLBOX_MENUITEM_START      ((USHORT)0xE000)

// defines for the menubutton
#define TOOLBOX_MENUTYPE_NONE           ((USHORT)0x0000)    // no menu at all, scrolling by spin buttons
#define TOOLBOX_MENUTYPE_CLIPPEDITEMS   ((USHORT)0x0001)    // menu will contain "more" indicator
#define TOOLBOX_MENUTYPE_CUSTOMIZE      ((USHORT)0x0002)    // menu will contain "customization" and "more" indicator

// By changes you must also change: rsc/vclrsc.hxx
enum ButtonType { BUTTON_SYMBOL, BUTTON_TEXT, BUTTON_SYMBOLTEXT };

// By changes you must also change: rsc/vclrsc.hxx
enum ToolBoxItemType { TOOLBOXITEM_DONTKNOW, TOOLBOXITEM_BUTTON,
                       TOOLBOXITEM_SPACE, TOOLBOXITEM_SEPARATOR,
                       TOOLBOXITEM_BREAK };

// small or large force an exact toolbox size for proper alignemnt
// dontcare will let the toolbox decide about its size
enum ToolBoxButtonSize { TOOLBOX_BUTTONSIZE_DONTCARE, TOOLBOX_BUTTONSIZE_SMALL, TOOLBOX_BUTTONSIZE_LARGE };

// used for internal sizing calculations
enum FloatingSizeMode { FSMODE_AUTO, FSMODE_FAVOURWIDTH, FSMODE_FAVOURHEIGHT };

// -----------
// - ToolBox -
// -----------

class VCL_DLLPUBLIC ToolBox : public DockingWindow
{
    friend class FloatingWindow;
    friend class ImplTBDragMgr;

private:
    ImplToolBoxPrivateData*     mpData;
    VirtualDevice*              mpBtnDev; // TODO: remove unused member
    ImplToolSizeArray*  mpFloatSizeAry;
    XubString           maCvtStr;
    XubString           maNextToolBoxStr;
    ImageList           maImageList;
    Timer               maTimer;
    Rectangle           maUpperRect;
    Rectangle           maLowerRect;
    Rectangle           maNextToolRect;
    Rectangle           maOutDockRect;
    Rectangle           maInDockRect;
    Rectangle           maPaintRect;
    FloatingWindow*     mpFloatWin;
    USHORT              mnKeyModifier;
    long                mnDX;
    long                mnDY;
    long                mnMaxItemWidth;    // max item width
    long                mnMaxItemHeight;   // max item height (for standard items)
    long                mnWinHeight;    // max window height (for window items)
    long                mnBorderX;      // custom border
    long                mnBorderY;
    long                mnLeftBorder;   // inner border
    long                mnTopBorder;
    long                mnRightBorder;
    long                mnBottomBorder;
    long                mnLastResizeDY;
    long                mnActivateCount;
    USHORT              mnLastFocusItemId;
    USHORT              mnFocusPos;
    USHORT              mnOutStyle;
    USHORT              mnHighItemId;
    USHORT              mnCurItemId;
    USHORT              mnDownItemId;
    USHORT              mnCurPos;
    USHORT              mnLines;        // total number of toolbox lines
    USHORT              mnCurLine;      // the currently visible line
    USHORT              mnCurLines;     // number of lines due to line breaking
    USHORT              mnVisLines;     // number of visible lines (for scrolling)
    USHORT              mnFloatLines;   // number of lines during floating mode
    USHORT              mnDockLines;
    USHORT              mnConfigItem;
    USHORT              mnMouseClicks;
    USHORT              mnMouseModifier;
    unsigned int        mbDrag:1,
                        mbSelection:1,
                        mbCommandDrag:1,
                        mbUpper:1,
                        mbLower:1,
                        mbNextTool:1,
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
                        mbHideStatusText:1,
                        mbMenuStrings:1,
                        mbIsShift:1,
                        mbIsKeyEvent:1,
                        mbChangingHighlight:1;
    WindowAlign         meAlign;
    WindowAlign         meDockAlign;
    ButtonType          meButtonType;
    PointerStyle        meLastStyle;
    WinBits             mnWinStyle;
    Link                maClickHdl;
    Link                maDoubleClickHdl;
    Link                maActivateHdl;
    Link                maDeactivateHdl;
    Link                maHighlightHdl;
    Link                maSelectHdl;
    Link                maNextToolBoxHdl;

    public:
    using Window::ImplInit;
    private:
    SAL_DLLPRIVATE void            ImplInit( Window* pParent, WinBits nStyle );
//    #if 0 // _SOLAR__PRIVATE
    using DockingWindow::ImplInitSettings;
//    #endif
    SAL_DLLPRIVATE void            ImplInitSettings( BOOL bFont, BOOL bForeground, BOOL bBackground );
    SAL_DLLPRIVATE void            ImplLoadRes( const ResId& rResId );
    SAL_DLLPRIVATE ImplToolItem*   ImplGetItem( USHORT nId ) const;
    SAL_DLLPRIVATE BOOL            ImplCalcItem();
    SAL_DLLPRIVATE USHORT          ImplCalcBreaks( long nWidth, long* pMaxLineWidth, BOOL bCalcHorz );
    SAL_DLLPRIVATE void            ImplFormat( BOOL bResize = FALSE );
    SAL_DLLPRIVATE void            ImplDrawSpin( BOOL bUpperIn, BOOL bLowerIn );
    SAL_DLLPRIVATE void            ImplDrawNext( BOOL bIn );
    SAL_DLLPRIVATE void            ImplDrawItem( USHORT nPos, BOOL bHighlight = FALSE, BOOL bPaint = FALSE, BOOL bLayout = FALSE );
    using Window::ImplInvalidate;
    SAL_DLLPRIVATE void            ImplInvalidate( BOOL bNewCalc = FALSE, BOOL bFullPaint = FALSE );
    SAL_DLLPRIVATE void            ImplUpdateItem( USHORT nIndex = 0xFFFF );
    SAL_DLLPRIVATE void            ImplStartCustomizeMode();
    SAL_DLLPRIVATE void            ImplEndCustomizeMode();
    SAL_DLLPRIVATE const XubString& ImplConvertMenuString( const XubString& rStr );
    SAL_DLLPRIVATE BOOL            ImplHandleMouseMove( const MouseEvent& rMEvt, BOOL bRepeat = FALSE );
    SAL_DLLPRIVATE BOOL            ImplHandleMouseButtonUp( const MouseEvent& rMEvt, BOOL bCancel = FALSE );
    SAL_DLLPRIVATE void            ImplChangeHighlight( ImplToolItem* pItem, BOOL bNoGrabFocus = FALSE );
    SAL_DLLPRIVATE BOOL            ImplChangeHighlightUpDn( BOOL bUp, BOOL bNoCycle = FALSE );
    SAL_DLLPRIVATE USHORT          ImplGetItemLine( ImplToolItem* pCurrentItem );
    SAL_DLLPRIVATE ImplToolItem*   ImplGetFirstValidItem( USHORT nLine );
    SAL_DLLPRIVATE ImplToolItem*   ImplGetLastValidItem( USHORT nLine );
    SAL_DLLPRIVATE BOOL            ImplOpenItem( KeyCode aKeyCode );
    SAL_DLLPRIVATE BOOL            ImplActivateItem( KeyCode aKeyCode );
    SAL_DLLPRIVATE void            ImplShowFocus();
    SAL_DLLPRIVATE void            ImplHideFocus();
    SAL_DLLPRIVATE void            ImplUpdateInputEnable();
    SAL_DLLPRIVATE void            ImplFillLayoutData() const;
    SAL_DLLPRIVATE void            ImplUpdateCustomMenu();
    SAL_DLLPRIVATE BOOL            ImplHasClippedItems();
    SAL_DLLPRIVATE Point           ImplGetPopupPosition( const Rectangle& rRect, const Size& rSize ) const;
    SAL_DLLPRIVATE void            ImplExecuteCustomMenu();
    SAL_DLLPRIVATE BOOL            ImplIsFloatingMode() const;
    SAL_DLLPRIVATE BOOL            ImplIsInPopupMode() const;
    SAL_DLLPRIVATE const XubString& ImplGetHelpText( USHORT nItemId ) const;
    SAL_DLLPRIVATE Size            ImplGetOptimalFloatingSize( FloatingSizeMode eMode );
    SAL_DLLPRIVATE BOOL            ImplHasExternalMenubutton();
    SAL_DLLPRIVATE void            ImplDrawFloatwinBorder( ImplToolItem* pItem );

    DECL_DLLPRIVATE_LINK(          ImplCallExecuteCustomMenu, void* );
    DECL_DLLPRIVATE_LINK(          ImplUpdateHdl, void* );
    DECL_DLLPRIVATE_LINK(          ImplResetAutoSizeTriesHdl, void* );
    DECL_DLLPRIVATE_LINK(          ImplCustomMenuListener, VclMenuEvent* );
    DECL_DLLPRIVATE_LINK(          ImplDropdownLongClickHdl, ToolBox* );

//#if 0 // _SOLAR__PRIVATE
    // Copy assignment is forbidden and not implemented.
    SAL_DLLPRIVATE                 ToolBox (const ToolBox &);
    SAL_DLLPRIVATE        ToolBox& operator= (const ToolBox &);

public:
    SAL_DLLPRIVATE void            ImplFloatControl( BOOL bStart, FloatingWindow* pWindow = NULL );
    SAL_DLLPRIVATE void            ImplDisableFlatButtons();

    static SAL_DLLPRIVATE int ImplGetDragWidth( ToolBox* pThis );
    static SAL_DLLPRIVATE void ImplUpdateDragArea( ToolBox *pThis );
    static SAL_DLLPRIVATE void ImplCalcBorder( WindowAlign eAlign, long& rLeft, long& rTop,
                                               long& rRight, long& rBottom, const ToolBox *pThis );
    static SAL_DLLPRIVATE void ImplDrawGrip( ToolBox* pThis );
    static SAL_DLLPRIVATE void ImplDrawGradientBackground( ToolBox* pThis, ImplDockingWindowWrapper *pWrapper );
    static SAL_DLLPRIVATE BOOL ImplDrawNativeBackground( ToolBox* pThis, const Region &rRegion );
    static SAL_DLLPRIVATE void ImplDrawTransparentBackground( ToolBox* pThis, const Region &rRegion );
    static SAL_DLLPRIVATE void ImplDrawConstantBackground( ToolBox* pThis, const Region &rRegion, BOOL bIsInPopupMode );
    static SAL_DLLPRIVATE void ImplDrawBackground( ToolBox* pThis, const Rectangle &rRect );
    static SAL_DLLPRIVATE void ImplErase( ToolBox* pThis, const Rectangle &rRect, BOOL bHighlight = FALSE, BOOL bHasOpenPopup = FALSE );
    static SAL_DLLPRIVATE void ImplDrawBorder( ToolBox* pWin );
    static SAL_DLLPRIVATE const ImplToolItem *ImplGetFirstClippedItem( const ToolBox* pThis );
    static SAL_DLLPRIVATE Size ImplCalcSize( const ToolBox* pThis, USHORT nCalcLines, USHORT nCalcMode = 0 );
    static SAL_DLLPRIVATE void ImplCalcFloatSizes( ToolBox* pThis );
    static SAL_DLLPRIVATE Size ImplCalcFloatSize( ToolBox* pThis, USHORT& rLines );
    static SAL_DLLPRIVATE void ImplCalcMinMaxFloatSize( ToolBox* pThis, Size& rMinSize, Size& rMaxSize );
    static SAL_DLLPRIVATE void ImplSetMinMaxFloatSize( ToolBox *pThis );
    static SAL_DLLPRIVATE USHORT ImplCalcLines( ToolBox* pThis, long nToolSize );
    static SAL_DLLPRIVATE USHORT ImplTestLineSize( ToolBox* pThis, const Point& rPos );
    static SAL_DLLPRIVATE void ImplLineSizing( ToolBox* pThis, const Point& rPos, Rectangle& rRect, USHORT nLineMode );
    static SAL_DLLPRIVATE USHORT ImplFindItemPos( ToolBox* pBox, const Point& rPos );
    static SAL_DLLPRIVATE USHORT ImplFindItemPos( const ImplToolItem* pItem, const std::vector< ImplToolItem >& rList );
    static SAL_DLLPRIVATE void ImplDrawToolArrow( ToolBox* pBox, long nX, long nY, BOOL bBlack, BOOL bColTransform,
                                                  BOOL bLeft = FALSE, BOOL bTop = FALSE,
                                                  long nSize = 6 );
    static SAL_DLLPRIVATE void SetToolArrowClipregion( ToolBox* pBox, long nX, long nY,
                                                       BOOL bLeft = FALSE, BOOL bTop = FALSE,
                                                       long nSize = 6 );
    static SAL_DLLPRIVATE void ImplDrawMenubutton( ToolBox *pThis, BOOL bHighlight );
    static SAL_DLLPRIVATE USHORT ImplCountLineBreaks( const ToolBox *pThis );
    SAL_DLLPRIVATE ImplToolBoxPrivateData* ImplGetToolBoxPrivateData() const { return mpData; }
//#endif

protected:
    void                SetCurItemId(USHORT nSet) { mnCurItemId = nSet; }

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
    virtual void        NextToolBox();
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

    virtual BOOL        PrepareToggleFloatingMode();
    virtual void        ToggleFloatingMode();
    virtual void        StartDocking();
    virtual BOOL        Docking( const Point& rPos, Rectangle& rRect );
    virtual void        EndDocking( const Rectangle& rRect, BOOL bFloatMode );
    virtual void        Resizing( Size& rSize );

    void                InsertItem( const ResId& rResId,
                                    USHORT nPos = TOOLBOX_APPEND );
    void                InsertItem( USHORT nItemId, const Image& rImage,
                                    ToolBoxItemBits nBits = 0,
                                    USHORT nPos = TOOLBOX_APPEND );
    void                InsertItem( USHORT nItemId, const Image& rImage,
                                    const XubString& rText,
                                    ToolBoxItemBits nBits = 0,
                                    USHORT nPos = TOOLBOX_APPEND );
    void                InsertItem( USHORT nItemId, const XubString& rText,
                                    ToolBoxItemBits nBits = 0,
                                    USHORT nPos = TOOLBOX_APPEND );
    void                InsertWindow( USHORT nItemId, Window* pWindow,
                                      ToolBoxItemBits nBits = 0,
                                      USHORT nPos = TOOLBOX_APPEND );
    void                InsertSpace( USHORT nPos = TOOLBOX_APPEND );
    void                InsertSeparator( USHORT nPos = TOOLBOX_APPEND,
                                         USHORT nPixSize = 0 );
    void                InsertBreak( USHORT nPos = TOOLBOX_APPEND );
    void                RemoveItem( USHORT nPos );
    void                MoveItem( USHORT nItemId, USHORT nNewPos = TOOLBOX_APPEND );
    void                CopyItem( const ToolBox& rToolBox, USHORT nItemId,
                                  USHORT nNewPos = TOOLBOX_APPEND );
    void                CopyItems( const ToolBox& rToolBox );
    void                Clear();
    void                RecalcItems();

    const ImageList&    GetImageList() const { return maImageList; }
    void                SetImageList( const ImageList& rImageList );

    void                SetButtonType( ButtonType eNewType = BUTTON_SYMBOL );
    ButtonType          GetButtonType() const { return meButtonType; }

    // sets a fixed button size (small, large or dontcare (==autosize))
    void                SetToolboxButtonSize( ToolBoxButtonSize eSize );
    ToolBoxButtonSize   GetToolboxButtonSize() const;

    void                SetAlign( WindowAlign eNewAlign = WINDOWALIGN_TOP );
    WindowAlign         GetAlign() const { return meAlign; }
    BOOL                IsHorizontal() const { return mbHorz; }

    void                SetLineCount( USHORT nNewLines );
    USHORT              GetLineCount() const { return mnLines; }
    USHORT              GetCurLine() const { return mnCurLine; }
    void                ShowLine( BOOL bNext );

    // Used to enable/disable scrolling one page at a time for toolbar
    void                SetPageScroll( BOOL b );
    BOOL                GetPageScroll();

    void                SetNextToolBox( const XubString& rStr );
    const XubString&    GetNextToolBox() const { return maNextToolBoxStr; }

    USHORT              GetItemCount() const;
    ToolBoxItemType     GetItemType( USHORT nPos ) const;
    USHORT              GetItemPos( USHORT nItemId ) const;
    USHORT              GetItemPos( const Point& rPos ) const;
    USHORT              GetItemId( USHORT nPos ) const;
    USHORT              GetItemId( const Point& rPos ) const;
    Rectangle           GetItemRect( USHORT nItemId ) const;
    Rectangle           GetItemPosRect( USHORT nPos ) const;
    Rectangle           GetItemDropDownRect( USHORT nItemId ) const;
    Rectangle           GetItemPosDropDownRect( USHORT nPos ) const;

    // retrieves the optimal position to place a popup window for this item (subtoolbar or dropdown)
    Point               GetItemPopupPosition( USHORT nItemId, const Size& rSize ) const;

    Rectangle           GetScrollRect() const;
    Rectangle           GetMenubuttonRect() const;
    USHORT              GetCurItemId() const { return mnCurItemId; }
    USHORT              GetDownItemId() const { return mnDownItemId; }
    USHORT              GetClicks() const { return mnMouseClicks; }
    USHORT              GetModifier() const { return mnMouseModifier; }
    USHORT              GetKeyModifier() const { return mnKeyModifier; }

    void                SetItemBits( USHORT nItemId, ToolBoxItemBits nBits );
    ToolBoxItemBits     GetItemBits( USHORT nItemId ) const;

    void                SetItemData( USHORT nItemId, void* pNewData );
    void*               GetItemData( USHORT nItemId ) const;
    void                SetItemImage( USHORT nItemId, const Image& rImage );
    Image               GetItemImage( USHORT nItemId ) const;
    void                SetItemImageAngle( USHORT nItemId, long nAngle10 );
    long                GetItemImageAngle( USHORT nItemId ) const;
    void                SetItemImageMirrorMode( USHORT nItemId, BOOL bMirror );
    BOOL                GetItemImageMirrorMode( USHORT ) const;
    void                SetItemHighImage( USHORT nItemId, const Image& rImage );
    Image               GetItemHighImage( USHORT nItemId ) const;
    void                SetItemText( USHORT nItemId, const XubString& rText );
    const XubString&    GetItemText( USHORT nItemId ) const;
    void                SetItemWindow( USHORT nItemId, Window* pNewWindow );
    Window*             GetItemWindow( USHORT nItemId ) const;
    USHORT              GetHighlightItemId() const { return mnHighItemId; }

    void                StartSelection();
    void                EndSelection();

    void                SetItemDown( USHORT nItemId, BOOL bDown, BOOL bRelease = TRUE );
    BOOL                IsItemDown( USHORT nItemId ) const;

    void                SetItemState( USHORT nItemId, TriState eState );
    TriState            GetItemState( USHORT nItemId ) const;

    void                CheckItem( USHORT nItemId, BOOL bCheck = TRUE );
    BOOL                IsItemChecked( USHORT nItemId ) const;

    void                EnableItem( USHORT nItemId, BOOL bEnable = TRUE );
    BOOL                IsItemEnabled( USHORT nItemId ) const;

    void                TriggerItem( USHORT nItemId, BOOL bShift = FALSE, BOOL bCtrl = FALSE );
    void                ShowItem( USHORT nItemId, BOOL bVisible = TRUE );
    void                HideItem( USHORT nItemId ) { ShowItem( nItemId, FALSE ); }
    BOOL                IsItemVisible( USHORT nItemId ) const;

    void                SetItemCommand( USHORT nItemId, const XubString& rCommand );
    const XubString&    GetItemCommand( USHORT nItemId ) const;

    using Window::SetQuickHelpText;
    void                SetQuickHelpText( USHORT nItemId, const XubString& rText );
    using Window::GetQuickHelpText;
    const XubString&    GetQuickHelpText( USHORT nItemId ) const;

    void                SetHelpText( USHORT nItemId, const XubString& rText );
    const XubString&    GetHelpText( USHORT nItemId ) const;

    void                SetHelpId( USHORT nItemId, ULONG nHelpId );
    ULONG               GetHelpId( USHORT nItemId ) const;

    //  window size according to current alignment, floating state and number of lines
    Size                CalcWindowSizePixel() const;
    //  window size according to current alignment, floating state and a given number of lines
    Size                CalcWindowSizePixel( USHORT nCalcLines ) const;
    //  window size according to current floating state and a given number of lines and a given alignment
    Size                CalcWindowSizePixel( USHORT nCalcLines, WindowAlign eAlign ) const;
    // floating window size according to number of lines (uses the number of line breaks)
    Size                CalcFloatingWindowSizePixel() const;
    // floating window size with a given number of lines
    Size                CalcFloatingWindowSizePixel( USHORT nCalcLines ) const;
    // automatic window size for popoup mode
    Size                CalcPopupWindowSizePixel() const;

    // computes the smallest useful size when docked, ie with the first item visible only (+drag area and menu button)
    Size                CalcMinimumWindowSizePixel() const;

    void                SetDockingRects( const Rectangle& rOutRect,
                                         const Rectangle& rInRect );
    void                SetFloatingLines( USHORT nFloatLines );
    USHORT              GetFloatingLines() const;

    void                SetBorder( long nX, long nY );
    long                GetBorderX() const { return mnBorderX; }
    long                GetBorderY() const { return mnBorderY; }

    void                SetStyle( WinBits nNewStyle ) { mnWinStyle = nNewStyle; }
    WinBits             GetStyle() const { return mnWinStyle; }

    // enable/disable undocking
    void                Lock( BOOL bLock = TRUE );

    // read configuration to determine locking behaviour
    static BOOL         AlwaysLocked();

    void                EnableMenuStrings( BOOL bEnable = TRUE ) { mbMenuStrings = (bEnable != 0); }
    BOOL                IsMenuStringsEnabled() const { return mbMenuStrings; }

    void                SetOutStyle( USHORT nNewStyle );
    USHORT              GetOutStyle() const { return mnOutStyle; }

    void                EnableCustomize( BOOL bEnable = TRUE );
    BOOL                IsCustomize() { return mbCustomize; }
    void                StartCustomize( const Rectangle& rRect, void* pData = NULL );
    void                SetCustomizeMode( BOOL );
    BOOL                IsInCustomizeMode() const { return mbCustomizeMode; }

    static void         StartCustomizeMode();
    static void         EndCustomizeMode();
    static BOOL         IsCustomizeMode();

    void                SetHelpText( const XubString& rText )
                            { DockingWindow::SetHelpText( rText ); }
    const XubString&    GetHelpText() const
                            { return DockingWindow::GetHelpText(); }

    void                SetHelpId( ULONG nId )
                            { DockingWindow::SetHelpId( nId ); }
    ULONG               GetHelpId() const
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
    void                SetNextToolBoxHdl( const Link& rLink ) { maNextToolBoxHdl = rLink; }
    const Link&         GetNextToolBoxHdl() const { return maNextToolBoxHdl; }

    // support for custom menu (eg for configuration)
    // note: this menu will also be used to display currently
    //       clipped toolbox items, so you should only touch
    //       items that you added by yourself
    //       the private toolbox items will only use item ids starting from TOOLBOX_MENUITEM_START
    // to allow for customization of the menu the coresponding handler is called
    // when the menu button was clicked and before the menu is executed
    void                SetMenuType( USHORT aType = TOOLBOX_MENUTYPE_CUSTOMIZE );
    USHORT              GetMenuType() const;
    BOOL                IsMenuEnabled() const;
    PopupMenu*          GetMenu() const;
    void                SetMenuButtonHdl( const Link& rLink );
    const Link&         GetMenuButtonHdl() const;

    // open custommenu
    void                ExecuteCustomMenu();

    // allow Click Handler to detect special key
    BOOL                IsShift() const { return mbIsShift; }
    // allow Click Handler to distinguish between mouse and key input
    BOOL                IsKeyEvent() const { return mbIsKeyEvent; }

    // allows framework to set/query the planned popupmode
    BOOL                WillUsePopupMode() const;
    void                WillUsePopupMode( BOOL b);

    // accessibility helpers

    // gets the displayed text
    String GetDisplayText() const;
    // returns the bounding box for the character at index nIndex
    // where nIndex is relative to the starting index of the item
    // with id nItemId (in coordinates of the displaying window)
    Rectangle GetCharacterBounds( USHORT nItemId, long nIndex ) const;
    // -1 is returned if no character is at that point
    // if an index is found the corresponding item id is filled in (else 0)
    long GetIndexForPoint( const Point& rPoint, USHORT& rItemID ) const;
    // returns the number of portions in the result of GetDisplayText()
    long GetTextCount() const;
    // returns the interval [start,end] of text portion nText
    // returns [-1,-1] for an invalid text number
    Pair GetTextStartEnd( long nText ) const;
    // returns the item id for text portion nText or 0 if nText is invalid
    USHORT GetDisplayItemId( long nText ) const;

    const Size&         GetDefaultImageSize() const;
};

inline void ToolBox::CheckItem( USHORT nItemId, BOOL bCheck )
{
    SetItemState( nItemId, (bCheck) ? STATE_CHECK : STATE_NOCHECK );
}

inline BOOL ToolBox::IsItemChecked( USHORT nItemId ) const
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

inline void ToolBox::SetFloatingLines( USHORT nNewLines )
{
    mnFloatLines = nNewLines;
}

inline USHORT ToolBox::GetFloatingLines() const
{
    return mnFloatLines;
}

#endif  // _SV_TOOLBOX_HXX
