/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef THUMBNAILVIEW_HXX
#define THUMBNAILVIEW_HXX

#include "sfx2/dllapi.h"

#include <vector>
#include <boost/function.hpp>

#include <comphelper/processfactory.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <sfx2/thumbnailviewitem.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/timer.hxx>
#include <vcl/pngread.hxx>

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/embed/StorageFactory.hpp>

class BitmapEx;
class MouseEvent;
class TrackingEvent;
class HelpEvent;
class KeyEvent;
class DataChangedEvent;
class ScrollBar;
typedef ::std::vector< ThumbnailViewItem* > ThumbnailValueItemList;

struct ThumbnailItemAttributes;
class ThumbnailViewAcc;
class ThumbnailViewItemAcc;

namespace drawinglayer {
    namespace processor2d {
        class BaseProcessor2D;
    }
}

/*************************************************************************

    Description
    ============

    class ThumbnailView

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

    WB_VSCROLL          A scroolbar will be always shown. The visible number of
                        lines have to be specified with SetLineCount() if this
                        flag is set.
    WB_TABSTOP          It is possible to jump into the ValueSet with the tab key.
    WB_NOTABSTOP        It is not possible to jump into the ValueSet with the
                        tab key.
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
    or UserDraw items will be underlayed. If no color is specified the color
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

/* ThumbnailView types */

#define THUMBNAILVIEW_APPEND         ((sal_uInt16)-1)
#define THUMBNAILVIEW_ITEM_NOTFOUND  ((sal_uInt16)-1)

// Display all the available items in the thumbnail.
class ViewFilterAll
{
public:

    bool operator () (const ThumbnailViewItem*) const
    {
        return true;
    }
};

/**
 *
 *  Class to display thumbnails with their names below their respective icons
 *
 **/

class SFX2_DLLPUBLIC ThumbnailView : public Control
{
public:

    ThumbnailView ( Window* pParent, WinBits nWinStyle = WB_TABSTOP, bool bDisableTransientChildren = false );

    ThumbnailView ( Window* pParent, const ResId& rResId, bool bDisableTransientChildren = false );

    virtual ~ThumbnailView ();

    virtual void        MouseMove( const MouseEvent& rMEvt );

    void AppendItem (ThumbnailViewItem *pItem);

    void RemoveItem( sal_uInt16 nItemId );

    void Clear();

    // Change current thumbnail item list with new one (invalidates all pointers to a thumbnail item)
    void updateItems(const std::vector<ThumbnailViewItem *> &items);

    size_t GetItemPos( sal_uInt16 nItemId ) const;

    sal_uInt16 GetItemId( size_t nPos ) const;

    sal_uInt16 GetItemId( const Point& rPos ) const;

    sal_uInt16 getNextItemId () const;

    long GetItemWidth() const { return mnItemWidth; }

    long GetItemHeight() const { return mnItemHeight; }

    void setItemMaxTextLength (sal_uInt32 nLength);

    void setItemDimensions (long ItemWidth, long ThumbnailHeight,
                            long DisplayHeight, int itemPadding);

    sal_uInt16 GetFirstLine() const { return mnFirstLine; }

    void SelectItem( sal_uInt16 nItemId );

    void DeselectItem( sal_uInt16 nItemId );

    bool IsItemSelected( sal_uInt16 nItemId ) const;

    /**
     *
     * @brief deselect all current selected items.
     *
     **/

    void deselectItems ();

    OUString GetItemText( sal_uInt16 nItemId ) const;

    void ShowTooltips( bool bShowTooltips );

    void SetColor( const Color& rColor );

    void SetColor() { SetColor( Color( COL_TRANSPARENT ) ); }

    Color GetColor() const { return maColor; }

    bool IsColor() const { return maColor.GetTransparency() == 0; }

    void filterItems (const boost::function<bool (const ThumbnailViewItem*) > &func);

    void sortItems (const boost::function<bool (const ThumbnailViewItem*,
                                                const ThumbnailViewItem*) > &func);

    void setItemStateHdl (const Link &aLink) { maItemStateHdl = aLink; }

    virtual void Resize();

    virtual bool renameItem(ThumbnailViewItem* pItem, OUString sNewTitle);

    static BitmapEx readThumbnail(const OUString &msURL);

protected:

    virtual void KeyInput( const KeyEvent& rKEvt );

    virtual void MouseButtonDown( const MouseEvent& rMEvt );

    virtual void MouseButtonUp( const MouseEvent& rMEvt );

    virtual void Command( const CommandEvent& rCEvt );

    virtual void Paint( const Rectangle& rRect );

    virtual void GetFocus();

    virtual void LoseFocus();

    virtual void StateChanged( StateChangedType nStateChange );

    virtual void DataChanged( const DataChangedEvent& rDCEvt );

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

protected:

    // Drawing item related functions, override them to make your own custom ones.

    virtual void DrawItem (ThumbnailViewItem *pItem);

    virtual void OnItemDblClicked (ThumbnailViewItem *pItem);

protected:

    friend class ThumbnailViewAcc;
    friend class ThumbnailViewItemAcc;
    using Control::ImplInitSettings;
    using Window::ImplInit;

    void CalculateItemPositions ();
    void MakeItemVisible( sal_uInt16 nId );

    SFX2_DLLPRIVATE void         ImplInit();
    SFX2_DLLPRIVATE void         ImplInitSettings( bool bFont, bool bForeground, bool bBackground );
    SFX2_DLLPRIVATE void         ImplInitScrollBar();
    SFX2_DLLPRIVATE void         ImplDeleteItems();
    SFX2_DLLPRIVATE void         ImplDraw();
    SFX2_DLLPRIVATE size_t       ImplGetItem( const Point& rPoint, bool bMove = false ) const;
    SFX2_DLLPRIVATE ThumbnailViewItem*    ImplGetItem( size_t nPos );
    SFX2_DLLPRIVATE sal_uInt16          ImplGetVisibleItemCount() const;
    SFX2_DLLPRIVATE ThumbnailViewItem*    ImplGetVisibleItem( sal_uInt16 nVisiblePos );
    SFX2_DLLPRIVATE void            ImplFireAccessibleEvent( short nEventId, const ::com::sun::star::uno::Any& rOldValue, const ::com::sun::star::uno::Any& rNewValue );
    SFX2_DLLPRIVATE bool         ImplHasAccessibleListeners();
    DECL_DLLPRIVATE_LINK( ImplScrollHdl, ScrollBar* );

    DECL_LINK(OnItemSelected, ThumbnailViewItem*);

protected:

    ThumbnailValueItemList mItemList;
    ThumbnailValueItemList mFilteredItemList; ///< Cache to store the filtered items
    ThumbnailValueItemList::iterator mpStartSelRange;
    ScrollBar* mpScrBar;
    Rectangle maItemListRect;
    long mnHeaderHeight;
    long mnItemWidth;
    long mnItemHeight;
    long mnItemPadding;
    long mnThumbnailHeight;     // Maximum height of the thumbnail
    long mnDisplayHeight;       // Height of the data display box (name, etc)
    long mnVisLines;
    long mnLines;

    int mnFineness;

    sal_uInt16 mnHighItemId;
    sal_uInt16 mnCols;
    sal_uInt16 mnFirstLine;
    sal_uInt16 mnSpacing;
    bool mbScroll : 1;
    bool mbIsTransientChildrenDisabled : 1;
    bool mbHasVisibleItems : 1;
    bool mbShowTooltips : 1;
    Color maColor;

    Link maItemStateHdl;
    ThumbnailItemAttributes *mpItemAttrs;
    drawinglayer::processor2d::BaseProcessor2D *mpProcessor;
    boost::function<bool (const ThumbnailViewItem*) > maFilterFunc;
};

#endif // THUMBNAILVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
