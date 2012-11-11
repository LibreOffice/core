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

#ifndef _VALUESET_HXX
#define _VALUESET_HXX

#include "svtools/svtdllapi.h"

#include <vcl/ctrl.hxx>
#include <vcl/virdev.hxx>
#include <vcl/timer.hxx>
#include <vector>

class MouseEvent;
class TrackingEvent;
class HelpEvent;
class KeyEvent;
class DataChangedEvent;
class ScrollBar;

struct ValueSetItem;
typedef ::std::vector< ValueSetItem* > ValueItemList;

#ifdef _SV_VALUESET_CXX
class ValueSetAcc;
class ValueItemAcc;
#endif

/*************************************************************************

Description
============

class ValueSet

This class allows the selection of an item. In the process items are
drawn side by side. The selection of items can be more clear than in a
ListBox shape for example in case of colors or samples.
The amount of columns drawn by the control and whether the items
should be encircled can be specified. Optional a NoSelection or name
field could be shown. By default image and color items are supported.
Items could be drawn by oneself if InsertItem() is only called with
an ID. To achieve this the UserDraw handler needs to be overloaded. The
description text could be specified afterwards in case of UserDraw
and any other items.

Cross references

class ListBox

--------------------------------------------------------------------------

WinBits

WB_RADIOSEL         If set the selection will be drawn like an
                    ImageRadioButton. This does only make sense if the image
                    is at least 8 pixel smaller on each side than the item
                    and also WB_DOUBLEBORDER is set and as color
                    COL_WINDOWWORKSPACE is specified.
WB_FLATVALUESET     Flat Look (if you set WB_ITEMBORDER or WB_DOUBLEBORDER,
                    then you get extra border space, but the Borders
                    aren't painted),
WB_ITEMBORDER       Items will be bordered
WB_DOUBLEBORDER     Items will be bordered twice. Additionally WB_ITEMBORDER
                    has to be set, as otherwise this WinBit wouldn't have any
                    effect. It is needed if there are items with a white
                    background, since otherwise the 3D effect wouldn't be
                    recognizable.
WB_NAMEFIELD        There is a namefield, where the name of an item will be
                    shown.
WB_NONEFIELD        There is a NoSelection field which can be selected if
                    0 is passed along with SelectItem. Respectively
                    GetSelectItemId() returns 0 if this field or nothing
                    is selected. This field shows the text which is specified
                    by SetText() respectively no one, if no text was set. With
                    SetNoSelection() the selection can be disabled.
WB_VSCROLL          A scroolbar will be always shown. The visible number of
                    lines have to be specified with SetLineCount() if this
                    flag is set.
WB_BORDER           A border will be drawn around the window.
WB_NOPOINTERFOCUS   The focus won't be gathered, if the control was pressed by
                    the mouse.
WB_TABSTOP          It is possible to jump into the ValueSet with the tab key.
WB_NOTABSTOP        It is not possible to jump into the ValueSet with the
                    tab key.
WB_NO_DIRECTSELECT  Cursor travelling doesn't call select immediately. To
                    execute the selection <RETURN> has to be pressed.
--------------------------------------------------------------------------

The number of columns must be either set with SetColCount() or
SetItemWidth(). If the number of colums is specified by SetColCount()
the width of the items will be calculated by the visible range.
If the items should have a static width, it has to be specified
with SetItemWidth(). In this case the number of columns will be calculated
by the visible range.

The number of rows is given by the number of items / number of columns. The
number of visible rows must either specified by SetLineCount() or
SetItemWidth(). If the number of visible rows is specified by SetLineCount(),
the height of the items will be calculated from the visible height. If the
items should have a fixed height it has to be specified with SetItemHeight().
In this case the number of visible rows is then calculated from the visible
height. If the number of visible rows is neither specified by SetLineCount()
nor by SetItemHeight() all rows will be shown. The height of the items will
be calculated by the visible height. If the number of visible rows is
specified by SetLineCount() or SetItemHeight() ValueSet does scroll
automatically when more lines are available, as are visible. If scrolling
should be also possible with a ScrollBar  WB_VSCROLL needs to be set.

The distance between the items can be increased by SetExtraSpacing(). The
distance, which will be shown between two items (both in x and in y), is
measured in pixels.

The exact window size for a specific item size can be calculated by
CalcWindowSizePixel(). To do this all relevant data (number of columns/...)
have to be specified and if no number of rows was set, all items need to
be inserted. If the window was created with WB_BORDER/Border=sal_True the
size has to be specified with SetOutputSizePixel(). In other cases different
size-methods can be used. With CalcItemSize() the inner and outer size of
an item could be calculated (for this the free space defined by
SetExtraSpacing() will not be included).

The background color could be specified by SetColor(), with which the image
or UserDraw items will be underlayed. If no color is specified the the color
of other windows (WindowColor) will be used for the background.

--------------------------------------------------------------------------

At first all items should be inserted and only then Show() should be called
since the output area will be precomputed. If this is not done the first
Paint will appear a little bit slower. Therefore the Control, if it is loaded
from the resource and only supplied with items during runtime, should be
loaded with Hide = sal_True and then displayed with Show().

In case of a visible Control the creation of the new output area could be
activated before Paint by calling Format().

--------------------------------------------------------------------------

If Drag and Drop will be called from the ValueSet the Command-Handler has to
be overloaded. From this StartDrag needs to be called. If this method returns
sal_True the drag-process could be initiated by  ExecuteDrag(), otherwise no
processing will take place. This method makes sure that ValueSet stops its
processing and as appropriate selects the entry. Therefore the calling of
Select-Handler within this function must be expected.

For dropping QueryDrop() and Drop() need to be overloaded and ShowDropPos()
and HideDropPos() should be called within these methods.
To show the insertion point ShowDropPos() has to be called within the
QueryDrop-Handler. ShowDropPos() also scrolls the ValueSet if the passed
position is located at the window border. Furthermore ShowDropPos() returns
the position, at which the item should be inserted respectively which
insertion point was shown. If no insertion point was determined
VALUESET_ITEM_NOTFOUND will be returned. If the window was left during dragging
or the drag process is terminated HideDropPos() should be called in any case.

--------------------------------------------------------------------------

This class is currently still in the SV-Tools. That's why the ValueSet needs
to be loaded as a Control out of the resource and the desired WinBits have
to be set (before Show) with SetStyle().

*************************************************************************/

// ------------------
// - ValueSet types -
// ------------------

#define WB_RADIOSEL             ((WinBits)0x00008000)
#define WB_ITEMBORDER           ((WinBits)0x00010000)
#define WB_DOUBLEBORDER         ((WinBits)0x00020000)
#define WB_NAMEFIELD            ((WinBits)0x00040000)
#define WB_NONEFIELD            ((WinBits)0x00080000)
#define WB_FLATVALUESET         ((WinBits)0x02000000)
#define WB_NO_DIRECTSELECT      ((WinBits)0x04000000)
#define WB_MENUSTYLEVALUESET    ((WinBits)0x08000000)

// ------------
// - ValueSet -
// ------------

#define VALUESET_APPEND         ((sal_uInt16)-1)
#define VALUESET_ITEM_NOTFOUND  ((sal_uInt16)-1)

class SVT_DLLPUBLIC ValueSet : public Control
{
private:

    VirtualDevice   maVirDev;
    Timer           maTimer;
    ValueItemList   mItemList;
    ValueSetItem*   mpNoneItem;
    ScrollBar*      mpScrBar;
    Rectangle       maNoneItemRect;
    Rectangle       maItemListRect;
    long            mnItemWidth;
    long            mnItemHeight;
    long            mnTextOffset;
    long            mnVisLines;
    long            mnLines;
    long            mnUserItemWidth;
    long            mnUserItemHeight;
    sal_uInt16          mnSelItemId;
    sal_uInt16          mnHighItemId;
    sal_uInt16          mnCols;
    sal_uInt16          mnCurCol;
    sal_uInt16          mnUserCols;
    sal_uInt16          mnUserVisLines;
    sal_uInt16          mnFirstLine;
    sal_uInt16          mnSpacing;
    sal_uInt16          mnFrameStyle;
    bool            mbFormat : 1;
    bool            mbHighlight : 1;
    bool            mbSelection : 1;
    bool            mbNoSelection : 1;
    bool            mbDrawSelection : 1;
    bool            mbBlackSel : 1;
    bool            mbDoubleSel : 1;
    bool            mbScroll : 1;
    bool            mbFullMode : 1;
    bool            mbIsTransientChildrenDisabled : 1;
    bool            mbHasVisibleItems : 1;
    Color           maColor;
    Link            maDoubleClickHdl;
    Link            maSelectHdl;
    Link            maHighlightHdl;

#ifdef _SV_VALUESET_CXX
    friend class ValueSetAcc;
    friend class ValueItemAcc;
    using Control::ImplInitSettings;
    using Window::ImplInit;
    SVT_DLLPRIVATE void         ImplInit();
    SVT_DLLPRIVATE void         ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    SVT_DLLPRIVATE void         ImplInitScrollBar();
    SVT_DLLPRIVATE void         ImplDeleteItems();
    SVT_DLLPRIVATE void         ImplFormatItem( ValueSetItem* pItem, Rectangle aRect );
    SVT_DLLPRIVATE void         ImplDrawItemText( const XubString& rStr );
    SVT_DLLPRIVATE void         ImplDrawSelect( sal_uInt16 nItemId, const bool bFocus, const bool bDrawSel );
    SVT_DLLPRIVATE void         ImplDrawSelect();
    SVT_DLLPRIVATE void         ImplHideSelect( sal_uInt16 nItemId );
    SVT_DLLPRIVATE void         ImplHighlightItem( sal_uInt16 nItemId, bool bIsSelection = true );
    SVT_DLLPRIVATE void         ImplDraw();
    using Window::ImplScroll;
    SVT_DLLPRIVATE bool         ImplScroll( const Point& rPos );
    SVT_DLLPRIVATE size_t       ImplGetItem( const Point& rPoint, bool bMove = false ) const;
    SVT_DLLPRIVATE ValueSetItem*    ImplGetItem( size_t nPos );
    SVT_DLLPRIVATE ValueSetItem*    ImplGetFirstItem();
    SVT_DLLPRIVATE sal_uInt16          ImplGetVisibleItemCount() const;
    SVT_DLLPRIVATE ValueSetItem*    ImplGetVisibleItem( sal_uInt16 nVisiblePos );
    SVT_DLLPRIVATE void         ImplInsertItem( ValueSetItem *const pItem, const size_t nPos );
    SVT_DLLPRIVATE Rectangle    ImplGetItemRect( size_t nPos ) const;
    SVT_DLLPRIVATE void            ImplFireAccessibleEvent( short nEventId, const ::com::sun::star::uno::Any& rOldValue, const ::com::sun::star::uno::Any& rNewValue );
    SVT_DLLPRIVATE bool         ImplHasAccessibleListeners();
    SVT_DLLPRIVATE void         ImplTracking( const Point& rPos, bool bRepeat );
    SVT_DLLPRIVATE void         ImplEndTracking( const Point& rPos, bool bCancel );
    DECL_DLLPRIVATE_LINK( ImplScrollHdl, ScrollBar* );
    DECL_DLLPRIVATE_LINK( ImplTimerHdl, void* );
#endif

    // Forbidden and not implemented.
    ValueSet (const ValueSet &);
    ValueSet & operator= (const ValueSet &);

protected:

    bool            StartDrag( const CommandEvent& rCEvt, Region& rRegion );

protected:

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

public:
                    ValueSet( Window* pParent, WinBits nWinStyle, bool bDisableTransientChildren = false );
                    ValueSet( Window* pParent, const ResId& rResId, bool bDisableTransientChildren = false );
                    ~ValueSet();

    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    Tracking( const TrackingEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    Paint( const Rectangle& rRect );
    virtual void    GetFocus();
    virtual void    LoseFocus();
    virtual void    Resize();
    virtual void    RequestHelp( const HelpEvent& rHEvt );
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    DataChanged( const DataChangedEvent& rDCEvt );

    virtual void    Select();
    virtual void    DoubleClick();
    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

    void            InsertItem( sal_uInt16 nItemId, const Image& rImage,
                                size_t nPos = VALUESET_APPEND );
    void            InsertItem( sal_uInt16 nItemId, const Color& rColor,
                                size_t nPos = VALUESET_APPEND );
    void            InsertItem( sal_uInt16 nItemId,
                                const Image& rImage, const XubString& rStr,
                                size_t nPos = VALUESET_APPEND );
    void            InsertItem( sal_uInt16 nItemId,
                                const Color& rColor, const XubString& rStr,
                                size_t nPos = VALUESET_APPEND );
    void            InsertItem( sal_uInt16 nItemId,
                                size_t nPos = VALUESET_APPEND );
    void            RemoveItem( sal_uInt16 nItemId );

    void            Clear();

    size_t          GetItemCount() const;
    size_t          GetItemPos( sal_uInt16 nItemId ) const;
    sal_uInt16          GetItemId( size_t nPos ) const;
    sal_uInt16          GetItemId( const Point& rPos ) const;
    Rectangle       GetItemRect( sal_uInt16 nItemId ) const;

    void            EnableFullItemMode( bool bFullMode = true );
    bool            IsFullItemModeEnabled() const { return mbFullMode; }
    void            SetColCount( sal_uInt16 nNewCols = 1 );
    sal_uInt16          GetColCount() const { return mnUserCols; }
    void            SetLineCount( sal_uInt16 nNewLines = 0 );
    sal_uInt16          GetLineCount() const { return mnUserVisLines; }
    void            SetItemWidth( long nItemWidth = 0 );
    long            GetItemWidth() const { return mnUserItemWidth; }
    void            SetItemHeight( long nLineHeight = 0 );
    long            GetItemHeight() const { return mnUserItemHeight; }
    sal_uInt16          GetFirstLine() const { return mnFirstLine; }

    void            SelectItem( sal_uInt16 nItemId );
    sal_uInt16          GetSelectItemId() const { return mnSelItemId; }
    bool            IsItemSelected( sal_uInt16 nItemId ) const
                        { return !mbNoSelection && (nItemId == mnSelItemId); }
    void            SetNoSelection();
    bool            IsNoSelection() const { return mbNoSelection; }

    void            SetItemImage( sal_uInt16 nItemId, const Image& rImage );
    Image           GetItemImage( sal_uInt16 nItemId ) const;
    void            SetItemColor( sal_uInt16 nItemId, const Color& rColor );
    Color           GetItemColor( sal_uInt16 nItemId ) const;
    void            SetItemData( sal_uInt16 nItemId, void* pData );
    void*           GetItemData( sal_uInt16 nItemId ) const;
    void            SetItemText( sal_uInt16 nItemId, const XubString& rStr );
    XubString       GetItemText( sal_uInt16 nItemId ) const;
    void            SetColor( const Color& rColor );
    void            SetColor() { SetColor( Color( COL_TRANSPARENT ) ); }
    Color           GetColor() const { return maColor; }
    bool            IsColor() const { return maColor.GetTransparency() == 0; }

    void            SetExtraSpacing( sal_uInt16 nNewSpacing );
    sal_uInt16          GetExtraSpacing() { return mnSpacing; }

    void            Format();

    void            StartSelection();
    void            EndSelection();

    Size            CalcWindowSizePixel( const Size& rItemSize,
                                         sal_uInt16 nCalcCols = 0,
                                         sal_uInt16 nCalcLines = 0 );
    Size            CalcItemSizePixel( const Size& rSize, bool bOut = true ) const;
    long            GetScrollWidth() const;

    void            SetSelectHdl( const Link& rLink ) { maSelectHdl = rLink; }
    const Link&     GetSelectHdl() const { return maSelectHdl; }
    void            SetDoubleClickHdl( const Link& rLink ) { maDoubleClickHdl = rLink; }
    const Link&     GetDoubleClickHdl() const { return maDoubleClickHdl; }

    void            SetHighlightHdl( const Link& rLink );
};

#endif  // _VALUESET_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
