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

#pragma once

#include <svtools/svtdllapi.h>
#include <vcl/wintypes.hxx>
#include <vcl/customweld.hxx>
#include <vcl/image.hxx>
#include <rtl/ref.hxx>
#include <memory>
#include <vector>

class UserDrawEvent;
class VirtualDevice;
class ValueSetAcc;

struct ValueSetItem;

enum class DrawFrameStyle;

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
an ID. To achieve this the UserDraw handler needs to be overridden. The
description text could be specified afterwards in case of UserDraw
and any other items.

Cross references

class ListBox

--------------------------------------------------------------------------

WinBits

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
                    GetSelectedItemId() returns 0 if this field or nothing
                    is selected. This field shows the text which is specified
                    by SetText() respectively no one, if no text was set. With
                    SetNoSelection() the selection can be disabled.
WB_VSCROLL          A scrollbar will be always shown. The visible number of
                    lines have to be specified with SetLineCount() if this
                    flag is set.
WB_BORDER           A border will be drawn around the window.
WB_TABSTOP          It is possible to jump into the ValueSet with the tab key.
WB_NOTABSTOP        It is not possible to jump into the ValueSet with the
                    tab key.
WB_NO_DIRECTSELECT  Cursor travelling doesn't call select immediately. To
                    execute the selection <RETURN> has to be pressed.
--------------------------------------------------------------------------

The number of columns must be either set with SetColCount() or
SetItemWidth(). If the number of columns is specified by SetColCount()
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
or UserDraw items will be underlaid. If no color is specified the color
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
be overridden. From this StartDrag needs to be called. If this method returns
sal_True the drag-process could be initiated by  ExecuteDrag(), otherwise no
processing will take place. This method makes sure that ValueSet stops its
processing and as appropriate selects the entry. Therefore the calling of
Select-Handler within this function must be expected.

For dropping QueryDrop() and Drop() need to be overridden and ShowDropPos()
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

typedef std::vector<std::unique_ptr<ValueSetItem>> ValueItemList;

#define WB_ITEMBORDER           (WinBits(0x00010000))
#define WB_DOUBLEBORDER         (WinBits(0x00020000))
#define WB_NAMEFIELD            (WinBits(0x00040000))
#define WB_NONEFIELD            (WinBits(0x00080000))
#define WB_FLATVALUESET         (WinBits(0x02000000))
#define WB_NO_DIRECTSELECT      (WinBits(0x04000000))
#define WB_MENUSTYLEVALUESET    (WinBits(0x08000000))

#define VALUESET_APPEND         (size_t(-1))
#define VALUESET_ITEM_NOTFOUND  (size_t(-1))

class SVT_DLLPUBLIC ValueSet : public weld::CustomWidgetController
{
private:
    ScopedVclPtr<VirtualDevice> maVirDev;
    rtl::Reference<ValueSetAcc> mxAccessible;
    ValueItemList   mItemList;
    std::unique_ptr<ValueSetItem> mpNoneItem;
    std::unique_ptr<weld::ScrolledWindow> mxScrolledWindow;
    tools::Rectangle  maNoneItemRect;
    tools::Rectangle  maItemListRect;
    tools::Long            mnItemWidth;
    tools::Long            mnItemHeight;
    tools::Long            mnTextOffset;
    tools::Long            mnVisLines;
    tools::Long            mnLines;
    tools::Long            mnUserItemWidth;
    tools::Long            mnUserItemHeight;
    sal_uInt16      mnSelItemId;
    int             mnSavedItemId;
    sal_uInt16      mnHighItemId;
    sal_uInt16      mnCols;
    sal_uInt16      mnCurCol;
    sal_uInt16      mnUserCols;
    sal_uInt16      mnUserVisLines;
    sal_uInt16      mnFirstLine;
    sal_uInt16      mnSpacing;
    sal_uInt16      mnMargin;
    DrawFrameStyle  mnFrameStyle;
    Color           maColor;
    OUString        maText;
    WinBits         mnStyle;
    Link<ValueSet*,void>  maDoubleClickHdl;
    Link<ValueSet*,void>  maSelectHdl;

    bool            mbFormat : 1;
    bool            mbHighlight : 1;
    bool            mbNoSelection : 1;
    bool            mbDoubleSel : 1;
    bool            mbScroll : 1;
    bool            mbFullMode : 1;
    bool            mbEdgeBlending : 1;
    bool            mbHasVisibleItems : 1;

    friend class ValueItemAcc;
    friend class ValueSetAcc;

    SVT_DLLPRIVATE void         ImplDeleteItems();
    SVT_DLLPRIVATE void         ImplFormatItem(vcl::RenderContext const & rRenderContext, ValueSetItem* pItem, tools::Rectangle aRect);
    SVT_DLLPRIVATE void         ImplDrawItemText(vcl::RenderContext& rRenderContext, const OUString& rStr);
    // nItemId is the item to draw selected, but if nothing is selected something else may be drawn as selected instead, the item to draw
    // selected is returned
    SVT_DLLPRIVATE ValueSetItem* ImplGetDrawSelectItem(sal_uInt16 nItemId, const bool bFocus, tools::Rectangle& rRect);
    SVT_DLLPRIVATE void         ImplDrawSelect(vcl::RenderContext& rRenderContext,
                                               const tools::Rectangle& rRect, const ValueSetItem* pItem,
                                               const bool bFocus, const bool bDrawSel,
                                               const bool bSelected, const bool bHover);
    SVT_DLLPRIVATE void         ImplDrawSelect(vcl::RenderContext& rRenderContext);
    SVT_DLLPRIVATE void         ImplHighlightItem(sal_uInt16 nItemId);
    SVT_DLLPRIVATE void         ImplDraw(vcl::RenderContext& rRenderContext);
    SVT_DLLPRIVATE size_t       ImplGetItem( const Point& rPoint ) const;
    SVT_DLLPRIVATE ValueSetItem*    ImplGetItem( size_t nPos );
    SVT_DLLPRIVATE ValueSetItem*    ImplGetFirstItem();
    SVT_DLLPRIVATE sal_uInt16          ImplGetVisibleItemCount() const;
    SVT_DLLPRIVATE void         ImplInsertItem( std::unique_ptr<ValueSetItem> pItem, const size_t nPos );
    SVT_DLLPRIVATE tools::Rectangle    ImplGetItemRect( size_t nPos ) const;
    SVT_DLLPRIVATE void         ImplFireAccessibleEvent( short nEventId, const css::uno::Any& rOldValue, const css::uno::Any& rNewValue );
    SVT_DLLPRIVATE bool         ImplHasAccessibleListeners() const;
    SVT_DLLPRIVATE void         ImplTracking(bool bLeaveWindow, const Point& rPos);
    SVT_DLLPRIVATE void         QueueReformat();
    SVT_DLLPRIVATE void         SetFirstLine(sal_uInt16 nNewFirstLine); // set mnFirstLine and update scrollbar to match
    SVT_DLLPRIVATE void         RecalcScrollBar();
    SVT_DLLPRIVATE bool         TurnOffScrollBar();
    SVT_DLLPRIVATE void         TurnOnScrollBar();
    DECL_DLLPRIVATE_LINK(ImplScrollHdl, weld::ScrolledWindow&, void);

    Size           GetLargestItemSize();

    ValueSet (const ValueSet &) = delete;
    ValueSet & operator= (const ValueSet &) = delete;

protected:
    virtual rtl::Reference<comphelper::OAccessible> CreateAccessible() override;

public:
    ValueSet(std::unique_ptr<weld::ScrolledWindow> pScrolledWindow);
    virtual         ~ValueSet() override;

    virtual void    SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    virtual bool    MouseButtonDown( const MouseEvent& rMEvt ) override;
    virtual bool    MouseButtonUp( const MouseEvent& rMEvt ) override;
    virtual bool    MouseMove( const MouseEvent& rMEvt ) override;
    virtual bool    KeyInput( const KeyEvent& rKEvt ) override;
    virtual void    Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual void    GetFocus() override;
    virtual void    LoseFocus() override;
    virtual void    Resize() override;
    virtual void    StyleUpdated() override;
    virtual void    Show() override;
    virtual void    Hide() override;
    virtual OUString RequestHelp(tools::Rectangle& rHelpRect) override;

    virtual void    Select();
    virtual void    UserDraw( const UserDrawEvent& rUDEvt );

    OUString const & GetText() const { return maText; }
    void            SetText(const OUString& rText) { maText = rText; }
    void            SetStyle(WinBits nStyle);
    WinBits         GetStyle() const { return mnStyle; }

    void SetOptimalSize();

    /// Insert @rImage item.
    void            InsertItem(sal_uInt16 nItemId, const Image& rImage);
    /// Insert @rImage item with @rStr as either a legend or tooltip depending on @bShowLegend.
    void            InsertItem(sal_uInt16 nItemId, const Image& rImage,
                               const OUString& rStr, size_t nPos = VALUESET_APPEND, bool bShowLegend = false);
    /// Insert an @rColor item with @rStr tooltip.
    void            InsertItem(sal_uInt16 nItemId, const Color& rColor,
                               const OUString& rStr);
    /// Insert an User Drawn item.
    void            InsertItem(sal_uInt16 nItemId, size_t nPos = VALUESET_APPEND);
    /// Insert an User Drawn item with @rStr tooltip.
    void            InsertItem(sal_uInt16 nItemId, const OUString& rStr, size_t nPos);
    void            RemoveItem(sal_uInt16 nItemId);

    void            Clear();

    size_t          GetItemCount() const;
    size_t          GetItemPos( sal_uInt16 nItemId ) const;
    sal_uInt16      GetItemId( size_t nPos ) const;
    sal_uInt16      GetItemId( const Point& rPos ) const;
    tools::Rectangle       GetItemRect( sal_uInt16 nItemId ) const;
    void            EnableFullItemMode( bool bFullMode );

    void            SetColCount( sal_uInt16 nNewCols = 1 );
    sal_uInt16      GetColCount() const
    {
        return mnUserCols;
    }
    void            SetLineCount( sal_uInt16 nNewLines = 0 );
    sal_uInt16      GetLineCount() const
    {
        return mnUserVisLines;
    }
    void           SetItemWidth( tools::Long nItemWidth );
    void           SetItemHeight( tools::Long nLineHeight );

    void           SelectItem( sal_uInt16 nItemId );
    sal_uInt16     GetSelectedItemId() const
    {
        return mnSelItemId;
    }
    size_t         GetSelectItemPos() const
    {
        return GetItemPos( mnSelItemId );
    }
    bool IsItemSelected( sal_uInt16 nItemId ) const
    {
        return !mbNoSelection && (nItemId == mnSelItemId);
    }
    void SetNoSelection();
    bool IsNoSelection() const
    {
        return mbNoSelection;
    }
    sal_uInt16 GetHighlightedItemId() const { return mnHighItemId; }

    void            SetItemImage( sal_uInt16 nItemId, const Image& rImage );
    Image           GetItemImage( sal_uInt16 nItemId ) const;
    void            SetItemColor( sal_uInt16 nItemId, const Color& rColor );
    Color           GetItemColor( sal_uInt16 nItemId ) const;
    void            SetItemData( sal_uInt16 nItemId, void* pData );
    void*           GetItemData( sal_uInt16 nItemId ) const;
    void            SetItemText( sal_uInt16 nItemId, const OUString& rStr );
    const OUString & GetItemText( sal_uInt16 nItemId ) const;
    void            SetColor( const Color& rColor );
    void            SetColor()
    {
        SetColor(COL_TRANSPARENT);
    }
    bool            IsColor() const
    {
        return !maColor.IsTransparent();
    }

    void            SetExtraSpacing( sal_uInt16 nNewSpacing );
    void            SetMargin( sal_uInt16 nNewMargin );

    void            Format(vcl::RenderContext const & rRenderContext);
    void            SetFormat();

    Size            CalcWindowSizePixel(const Size& rItemSize,
                                        sal_uInt16 nCalcCols = 0,
                                        sal_uInt16 nCalcLines = 0) const;
    Size            CalcItemSizePixel(const Size& rSize) const;
    int             GetScrollWidth() const;

    void            SetSelectHdl(const Link<ValueSet*,void>& rLink)
    {
        maSelectHdl = rLink;
    }

    void            SetDoubleClickHdl(const Link<ValueSet*,void>& rLink)
    {
        maDoubleClickHdl = rLink;
    }

    bool GetEdgeBlending() const
    {
        return mbEdgeBlending;
    }
    void SetEdgeBlending(bool bNew);

    void SaveValue()
    {
        mnSavedItemId = IsNoSelection() ? -1 : GetSelectedItemId();
    }

    bool IsValueChangedFromSaved() const
    {
        int nItemId = IsNoSelection() ? -1 : GetSelectedItemId();
        return mnSavedItemId != nItemId;
    }

    virtual FactoryFunction GetUITestFactory() const override;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
