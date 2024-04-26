/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SFX2_THUMBNAILVIEW_HXX
#define INCLUDED_SFX2_THUMBNAILVIEW_HXX

#include <sfx2/dllapi.h>

#include <functional>
#include <memory>
#include <vector>

#include <vcl/customweld.hxx>

class BitmapEx;
class MouseEvent;
class KeyEvent;
class DataChangedEvent;
class ScrollBar;
class ThumbnailViewItem;
typedef ::std::vector< ThumbnailViewItem* > ThumbnailValueItemList;
class ThumbnailViewAcc;
struct ThumbnailItemAttributes;

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
    an ID. To achieve this the UserDraw handler needs to be overridden. The
    description text could be specified afterwards in case of UserDraw
    and any other items.

    Cross references

    class ListBox

    --------------------------------------------------------------------------

    WinBits

    WB_VSCROLL          A scrollbar will be always shown. The visible number of
                        lines have to be specified with SetLineCount() if this
                        flag is set.
    WB_TABSTOP          It is possible to jump into the ValueSet with the tab key.
    WB_NOTABSTOP        It is not possible to jump into the ValueSet with the
                        tab key.
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

/* ThumbnailView types */

#define THUMBNAILVIEW_ITEM_NOTFOUND  (sal_uInt16(-1))

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

class SFX2_DLLPUBLIC ThumbnailView : public weld::CustomWidgetController
{
    friend class ThumbnailViewAcc;
    friend class ThumbnailViewItemAcc;

public:
    ThumbnailView(std::unique_ptr<weld::ScrolledWindow> xWindow, std::unique_ptr<weld::Menu> xMenu);

    virtual ~ThumbnailView() override;

    ThumbnailView& operator=( ThumbnailView const & ) = delete; // MSVC workaround
    ThumbnailView( ThumbnailView const & ) = delete; // MSVC workaround

    virtual bool MouseMove(const MouseEvent& rMEvt) override;

    /// Updates information in the view; used only in RecentDocsView ATM.
    virtual void Reload() {}

    void AppendItem(std::unique_ptr<ThumbnailViewItem> pItem);

    SAL_DLLPRIVATE void RemoveItem(sal_uInt16 nItemId);

    virtual void Clear();

    // Change current thumbnail item list with new one (invalidates all pointers to a thumbnail item)
    SAL_DLLPRIVATE void updateItems(std::vector<std::unique_ptr<ThumbnailViewItem>> items);

    SAL_DLLPRIVATE size_t GetItemPos( sal_uInt16 nItemId ) const;

    SAL_DLLPRIVATE sal_uInt16 GetItemId( size_t nPos ) const;

    sal_uInt16 GetItemId( const Point& rPos ) const;

    virtual bool renameItem(ThumbnailViewItem& rItem, const OUString& sNewTitle);

    bool isDrawMnemonic() const { return mbDrawMnemonics; }

    SAL_DLLPRIVATE void setItemMaxTextLength (sal_uInt32 nLength);

    void setItemDimensions (tools::Long ItemWidth, tools::Long ThumbnailHeight,
                            tools::Long DisplayHeight, int itemPadding);

    void SelectItem( sal_uInt16 nItemId );

    SAL_DLLPRIVATE bool IsItemSelected( sal_uInt16 nItemId ) const;

    /**
     *
     * @brief deselect all current selected items.
     *
     **/

    void deselectItems ();

    SAL_DLLPRIVATE void ShowTooltips( bool bShowTooltips );

    void DrawMnemonics( bool bDrawMnemonics );

    SAL_DLLPRIVATE void filterItems(const std::function<bool (const ThumbnailViewItem*) > &func);

    void setItemStateHdl (const Link<const ThumbnailViewItem*,void> &aLink) { maItemStateHdl = aLink; }

    virtual void Resize() override;

    virtual void Show() override
    {
        mxScrolledWindow->show();
        CustomWidgetController::Show();
    }

    virtual void Hide() override
    {
        mxScrolledWindow->hide();
        CustomWidgetController::Hide();
    }

    virtual void SetDrawingArea(weld::DrawingArea* pDrawingArea) override;

    static BitmapEx readThumbnail(const OUString &msURL);

protected:

    virtual bool KeyInput( const KeyEvent& rKEvt ) override;

    virtual bool MouseButtonDown( const MouseEvent& rMEvt ) override;

    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;

    virtual void GetFocus() override;

    virtual void LoseFocus() override;

    virtual OUString RequestHelp(tools::Rectangle& rRect) override;

    virtual css::uno::Reference< css::accessibility::XAccessible > CreateAccessible() override;

    SAL_DLLPRIVATE const rtl::Reference<ThumbnailViewAcc> & getAccessible() const;

protected:

    // Drawing item related functions, override them to make your own custom ones.

    SAL_DLLPRIVATE void DrawItem (ThumbnailViewItem const *pItem);

    virtual void OnItemDblClicked (ThumbnailViewItem *pItem);

    // Set Item colors from the ThumbnailView colors
    SAL_DLLPRIVATE void UpdateColors();

protected:

    friend class ThumbnailViewAcc;
    friend class ThumbnailViewItemAcc;

    SAL_DLLPRIVATE void CalculateItemPositions (bool bScrollBarUsed = false);
    SAL_DLLPRIVATE void MakeItemVisible( sal_uInt16 nId );

    SAL_DLLPRIVATE void ImplInit();

    SAL_DLLPRIVATE void ImplDeleteItems();
    SAL_DLLPRIVATE size_t ImplGetItem( const Point& rPoint ) const;
    SAL_DLLPRIVATE ThumbnailViewItem* ImplGetItem( size_t nPos );
    SAL_DLLPRIVATE sal_uInt16 ImplGetVisibleItemCount() const;
    SAL_DLLPRIVATE ThumbnailViewItem* ImplGetVisibleItem(sal_uInt16 nVisiblePos);
    SAL_DLLPRIVATE void ImplFireAccessibleEvent( short nEventId, const css::uno::Any& rOldValue, const css::uno::Any& rNewValue );
    SAL_DLLPRIVATE bool ImplHasAccessibleListeners() const;
    DECL_DLLPRIVATE_LINK( ImplScrollHdl, weld::ScrolledWindow&, void );

protected:

    std::vector< std::unique_ptr<ThumbnailViewItem> > mItemList;
    rtl::Reference<ThumbnailViewAcc> mxAccessible;
    ThumbnailValueItemList mFilteredItemList; ///< Cache to store the filtered items
    ThumbnailValueItemList::iterator mpStartSelRange;
    tools::Long mnItemWidth;
    tools::Long mnItemHeight;
    tools::Long mnItemPadding;
    tools::Long mnThumbnailHeight;     // Maximum height of the thumbnail
    tools::Long mnDisplayHeight;       // Height of the data display box (name, etc)
    tools::Long mnVItemSpace;          // Vertical spacing between rows, -1 to use excess unused height split up between items
    tools::Long mnVisLines;
    tools::Long mnLines;

    sal_uInt16 mnCols;
    sal_uInt16 mnFirstLine;
    bool mbScroll : 1;          // Whether we need to scroll
    bool mbAllowVScrollBar : 1; // Whether to show a visible scrollbar
    bool mbHasVisibleItems : 1;
    bool mbShowTooltips : 1;
    bool mbDrawMnemonics : 1;
    bool mbSelectOnFocus : 1;
    bool mbAllowMultiSelection : 1;
    Color maFillColor;              ///< Background color of the thumbnail view widget.
    Color maTextColor;              ///< Text color.
    Color maHighlightColor;         ///< Color of the highlight (background) of the hovered item.
    Color maHighlightTextColor;     ///< Color of the text for the highlighted item.
    double mfHighlightTransparence; ///< Transparence of the highlight.

    Link<const ThumbnailViewItem*, void> maItemStateHdl;
    std::unique_ptr<ThumbnailItemAttributes> mpItemAttrs;
    std::unique_ptr<weld::ScrolledWindow> mxScrolledWindow;
    std::unique_ptr<weld::Menu> mxContextMenu;

    std::function<bool (const ThumbnailViewItem*)> maFilterFunc;
};


#endif // INCLUDED_SFX2_THUMBNAILVIEW_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
