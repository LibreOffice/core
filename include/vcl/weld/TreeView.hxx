/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/dllapi.h>
#include <vcl/weld/ItemView.hxx>

enum class SelectionMode;
enum class TxtAlign;

namespace weld
{
enum class ColumnToggleType
{
    Check,
    Radio
};

/* Model column indexes are considered to begin at 0, but with special columns
   before index 0. A expander image column (and an additional optional toggle
   button column when enable_toggle_buttons is used). Column index -1 is
   reserved to access those columns.
*/
class VCL_DLLPUBLIC TreeView : virtual public ItemView
{
    friend class ::LOKTrigger;

public:
    typedef std::pair<const TreeIter&, int> iter_col;
    typedef std::pair<const TreeIter&, OUString> iter_string;
    // OUString is the id of the row, it may be null to measure the height of a generic line
    typedef std::pair<vcl::RenderContext&, const OUString&> get_size_args;
    typedef std::tuple<vcl::RenderContext&, const tools::Rectangle&, bool, const OUString&>
        render_args;

private:
    Link<TreeView&, void> m_aSelectionChangedHdl;
    Link<TreeView&, bool> m_aRowActivatedHdl;
    Link<int, void> m_aColumnClickedHdl;
    Link<const iter_col&, void> m_aRadioToggleHdl;
    Link<const TreeIter&, bool> m_aEditingStartedHdl;
    Link<const iter_string&, bool> m_aEditingDoneHdl;
    // if handler returns false, the expansion of the row is refused
    Link<const TreeIter&, bool> m_aExpandingHdl;
    // if handler returns false, the collapse of the row is refused
    Link<const TreeIter&, bool> m_aCollapsingHdl;
    Link<TreeView&, void> m_aVisibleRangeChangedHdl;
    Link<TreeView&, void> m_aModelChangedHdl;

protected:
    // if handler returns true, drag is disallowed, consumer can change bool
    // arg to false to disable the treeview default dnd icon
    Link<bool&, bool> m_aDragBeginHdl;
    std::function<int(const weld::TreeIter&, const weld::TreeIter&)> m_aCustomSort;

protected:
    void signal_selection_changed()
    {
        if (notify_events_disabled())
            return;
        m_aSelectionChangedHdl.Call(*this);
    }

    bool signal_row_activated()
    {
        if (notify_events_disabled())
            return true;
        return m_aRowActivatedHdl.Call(*this);
    }

    void signal_column_clicked(int nColumn) { m_aColumnClickedHdl.Call(nColumn); }
    bool signal_expanding(const TreeIter& rIter)
    {
        return !m_aExpandingHdl.IsSet() || m_aExpandingHdl.Call(rIter);
    }
    bool signal_collapsing(const TreeIter& rIter)
    {
        return !m_aCollapsingHdl.IsSet() || m_aCollapsingHdl.Call(rIter);
    }
    void signal_visible_range_changed()
    {
        if (notify_events_disabled())
            return;
        m_aVisibleRangeChangedHdl.Call(*this);
    }

    void signal_model_changed()
    {
        if (notify_events_disabled())
            return;
        m_aModelChangedHdl.Call(*this);
    }

    void signal_toggled(const iter_col& rIterCol) { m_aRadioToggleHdl.Call(rIterCol); }

    bool signal_editing_started(const TreeIter& rIter) { return m_aEditingStartedHdl.Call(rIter); }

    bool signal_editing_done(const iter_string& rIterText)
    {
        return m_aEditingDoneHdl.Call(rIterText);
    }

    Link<const TreeIter&, OUString> m_aQueryTooltipHdl;

    OUString signal_query_tooltip(const TreeIter& rIter)
    {
        if (notify_events_disabled())
            return {};
        return m_aQueryTooltipHdl.Call(rIter);
    }

    Link<render_args, void> m_aRenderHdl;
    void signal_custom_render(vcl::RenderContext& rDevice, const tools::Rectangle& rRect,
                              bool bSelected, const OUString& rId)
    {
        m_aRenderHdl.Call(render_args(rDevice, rRect, bSelected, rId));
    }

    Link<get_size_args, Size> m_aGetSizeHdl;
    Size signal_custom_get_size(vcl::RenderContext& rDevice, const OUString& rId)
    {
        return m_aGetSizeHdl.Call(get_size_args(rDevice, rId));
    }

    virtual void do_insert(const TreeIter* pParent, int pos, const OUString* pStr,
                           const OUString* pId, const OUString* pIconName,
                           VirtualDevice* pImageSurface, bool bChildrenOnDemand, TreeIter* pRet)
        = 0;
    virtual void do_insert_separator(int pos, const OUString& rId) = 0;
    using weld::ItemView::do_set_cursor;
    virtual void do_set_cursor(int pos) = 0;
    virtual void do_scroll_to_row(const TreeIter& rIter) = 0;
    virtual bool do_iter_children(TreeIter& rIter) const = 0;
    virtual void do_set_children_on_demand(const TreeIter& rIter, bool bChildrenOnDemand) = 0;
    virtual void do_remove_selection() = 0;

public:
    virtual void connect_query_tooltip(const Link<const TreeIter&, OUString>& rLink)
    {
        assert(!m_aQueryTooltipHdl.IsSet() || !rLink.IsSet());
        m_aQueryTooltipHdl = rLink;
    }

    // see 'expanding on-demand node details' for bChildrenOnDemand of true
    void insert(const TreeIter* pParent, int pos, const OUString* pStr, const OUString* pId,
                const OUString* pIconName, VirtualDevice* pImageSurface, bool bChildrenOnDemand,
                TreeIter* pRet)
    {
        disable_notify_events();
        do_insert(pParent, pos, pStr, pId, pIconName, pImageSurface, bChildrenOnDemand, pRet);
        enable_notify_events();
    }

    void insert(int nRow, TreeIter* pRet = nullptr)
    {
        insert(nullptr, nRow, nullptr, nullptr, nullptr, nullptr, false, pRet);
    }

    void append(TreeIter* pRet = nullptr) { insert(-1, pRet); }

    void insert(int pos, const OUString& rStr, const OUString* pId, const OUString* pIconName,
                VirtualDevice* pImageSurface)
    {
        insert(nullptr, pos, &rStr, pId, pIconName, pImageSurface, false, nullptr);
    }
    void insert_text(int pos, const OUString& rStr)
    {
        insert(nullptr, pos, &rStr, nullptr, nullptr, nullptr, false, nullptr);
    }
    void append_text(const OUString& rStr)
    {
        insert(nullptr, -1, &rStr, nullptr, nullptr, nullptr, false, nullptr);
    }
    void append(const OUString& rId, const OUString& rStr)
    {
        insert(nullptr, -1, &rStr, &rId, nullptr, nullptr, false, nullptr);
    }
    void append(const OUString& rId, const OUString& rStr, const OUString& rImage)
    {
        insert(nullptr, -1, &rStr, &rId, &rImage, nullptr, false, nullptr);
    }
    void append(const TreeIter* pParent, const OUString& rStr)
    {
        insert(pParent, -1, &rStr, nullptr, nullptr, nullptr, false, nullptr);
    }

    void insert_separator(int pos, const OUString& rId)
    {
        disable_notify_events();
        do_insert_separator(pos, rId);
        enable_notify_events();
    }

    void append_separator(const OUString& rId) { insert_separator(-1, rId); }

    void connect_selection_changed(const Link<TreeView&, void>& rLink)
    {
        m_aSelectionChangedHdl = rLink;
    }

    /* A row is "activated" when the user double clicks a treeview row. It may
       also be emitted when a row is selected and Space or Enter is pressed.

       a return of "true" means the activation has been handled, a "false" propagates
       the activation to the default handler which expands/collapses the row, if possible.
    */
    void connect_row_activated(const Link<TreeView&, bool>& rLink) { m_aRowActivatedHdl = rLink; }

    // Argument is a pair of iter, col describing the toggled node
    void connect_toggled(const Link<const iter_col&, void>& rLink) { m_aRadioToggleHdl = rLink; }

    void connect_column_clicked(const Link<int, void>& rLink) { m_aColumnClickedHdl = rLink; }
    void connect_model_changed(const Link<TreeView&, void>& rLink) { m_aModelChangedHdl = rLink; }

    // call before inserting any content and connecting to toggle signals,
    // an pre-inserted checkbutton column will exist at the start of every row
    // inserted after this call which can be accessed with col index -1
    virtual void enable_toggle_buttons(ColumnToggleType eType) = 0;

    virtual void set_clicks_to_toggle(int nToggleBehavior) = 0;

    int get_selected_index() const;

    // col index -1 gets the first text column
    OUString get_text(int row, int col = -1) const;
    virtual OUString get_text(const TreeIter& rIter, int col = -1) const = 0;

    // col index -1 sets the first text column
    void set_text(int row, const OUString& rText, int col = -1);
    virtual void set_text(const TreeIter& rIter, const OUString& rStr, int col = -1) = 0;

    // col index -1 sets all columns
    void set_sensitive(int row, bool bSensitive, int col = -1);
    virtual void set_sensitive(const TreeIter& rIter, bool bSensitive, int col = -1) = 0;

    bool get_sensitive(int row, int col) const;
    virtual bool get_sensitive(const TreeIter& rIter, int col) const = 0;

    // col index -1 sets the expander toggle, enable_toggle_buttons must have been called to create that column
    void set_toggle(int row, TriState eState, int col = -1);
    virtual void set_toggle(const TreeIter& rIter, TriState bOn, int col = -1) = 0;

    // col index -1 gets the expander toggle, enable_toggle_buttons must have been called to create that column
    TriState get_toggle(int row, int col = -1) const;
    virtual TriState get_toggle(const TreeIter& rIter, int col = -1) const = 0;

    // col index -1 sets the expander image
    void set_image(int row, const OUString& rImage, int col = -1);
    virtual void set_image(const TreeIter& rIter, const OUString& rImage, int col = -1) = 0;
    // col index -1 sets the expander image
    void set_image(int row, VirtualDevice& rImage, int col = -1);
    virtual void set_image(const TreeIter& rIter, VirtualDevice& rImage, int col = -1) = 0;
    // col index -1 sets the expander image
    void set_image(int row, const css::uno::Reference<css::graphic::XGraphic>& rImage,
                   int col = -1);
    virtual void set_image(const TreeIter& rIter,
                           const css::uno::Reference<css::graphic::XGraphic>& rImage, int col = -1)
        = 0;

    // col index -1 sets text emphasis for all columns
    void set_text_emphasis(int row, bool bOn, int col);
    virtual void set_text_emphasis(const TreeIter& rIter, bool bOn, int col) = 0;
    bool get_text_emphasis(int row, int col) const;
    virtual bool get_text_emphasis(const TreeIter& rIter, int col) const = 0;

    void set_text_align(int row, TxtAlign eAlign, int col);
    virtual void set_text_align(const TreeIter& rIter, TxtAlign eAlign, int col) = 0;

    virtual void swap(int pos1, int pos2) = 0;
    virtual std::vector<int> get_selected_rows() const = 0;

    void set_font_color(int pos, const Color& rColor);
    virtual void set_font_color(const TreeIter& rIter, const Color& rColor) = 0;

    // scroll to make given row visible, this will also expand all parent rows
    // of the row as necessary to make the row visible
    void scroll_to_row(int row);
    void scroll_to_row(const TreeIter& rIter);

    bool is_selected(int pos) const;
    virtual bool is_selected(const TreeIter& rIter) const = 0;

    using weld::ItemView::set_cursor;
    void set_cursor(int pos)
    {
        disable_notify_events();
        do_set_cursor(pos);
        enable_notify_events();
    }

    //by text
    virtual int find_text(const OUString& rText) const = 0;
    //Don't select when frozen, select after thaw. Note selection doesn't survive a freeze.
    void select_text(const OUString& rText) { select(find_text(rText)); }
    void remove_text(const OUString& rText) { remove(find_text(rText)); }
    std::vector<OUString> get_selected_rows_text() const
    {
        std::vector<int> aRows(get_selected_rows());
        std::vector<OUString> aRet;
        aRet.reserve(aRows.size());
        for (auto a : aRows)
            aRet.push_back(get_text(a));
        return aRet;
    }

    //by id
    virtual int find_id(const OUString& rId) const = 0;
    //Don't select when frozen, select after thaw. Note selection doesn't survive a freeze.
    void select_id(const OUString& rId) { select(find_id(rId)); }
    void remove_id(const OUString& rText) { remove(find_id(rText)); }

    virtual void copy_iterator(const TreeIter& rSource, TreeIter& rDest) const = 0;

    // set iter to point to previous node at the current level
    virtual bool iter_previous_sibling(TreeIter& rIter) const = 0;
    // set iter to point to next node, depth first, then sibling
    bool iter_next(TreeIter& rIter) const;
    // set iter to point to previous node, sibling first then depth
    bool iter_previous(TreeIter& rIter) const;
    // set iter to point to first child node
    bool iter_children(TreeIter& rIter) const;
    bool iter_nth_sibling(TreeIter& rIter, int nChild) const
    {
        bool bRet = true;
        for (int i = 0; i < nChild && bRet; ++i)
            bRet = iter_next_sibling(rIter);
        return bRet;
    }
    bool iter_nth_child(TreeIter& rIter, int nChild) const
    {
        if (!iter_children(rIter))
            return false;
        return iter_nth_sibling(rIter, nChild);
    }
    virtual bool iter_parent(TreeIter& rIter) const = 0;
    virtual int get_iter_depth(const TreeIter& rIter) const = 0;
    /* Compares two paths. If a appears before b in a tree, then -1 is returned.
       If b appears before a , then 1 is returned. If the two nodes are equal,
       then 0 is returned.
    */
    virtual int iter_compare(const TreeIter& rIterA, const TreeIter& rIterB) const;
    bool iter_has_child(const TreeIter& rIter) const;
    // returns the number of direct children rIter has
    virtual int iter_n_children(const TreeIter& rIter) const = 0;

    //visually indent this row as if it was at get_iter_depth() + nIndentLevel
    virtual void set_extra_row_indent(const TreeIter& rIter, int nIndentLevel) = 0;

    virtual void move_subtree(TreeIter& rNode, const TreeIter* pNewParent, int nIndexInNewParent)
        = 0;

    // call func on each element until func returns true or we run out of elements
    virtual void all_foreach(const std::function<bool(TreeIter&)>& func) = 0;
    // call func on each visible element until func returns true or we run out of elements
    virtual void visible_foreach(const std::function<bool(TreeIter&)>& func) = 0;
    // clear the children of pParent (whole tree if nullptr),
    // then add nSourceCount rows under pParent, call func on each row
    // inserted with an arg of the index that this row will be when bulk insert
    // ends.
    //
    // this enables inserting the entries backwards in models where that is faster,
    //
    // pFixedWidths is optional, when present each matching entry col text
    // width will not be measured, and the fixed width used instead. Use
    // sparingly because wider text than the fixed width is clipped and cannot
    // be scrolled into view horizontally.
    // @param bGoingToSetText needs to be true, if you plan to call set_text inside the insert func.
    virtual void bulk_insert_for_each(int nSourceCount,
                                      const std::function<void(TreeIter&, int nSourceIndex)>& func,
                                      const weld::TreeIter* pParent = nullptr,
                                      const std::vector<int>* pFixedWidths = nullptr,
                                      bool bGoingToSetText = false)
        = 0;

    /* expanding on-demand node details

    When a node is added with children-on-demand (typically via 'insert' with
    bChildrenOnDemand of true), then it shows an expander indicator even if
    it doesn't have any "real" child entries (yet).

    (Depending on the underlying toolkit, implementations may in reality give
    the children-on-demand node a 'placeholder' child entry to indicate the
    load-on-demand state and ensure the treeview draws/shows the expander
    indicator even when there are no "real" entries yet. In that case, this
    child doesn't exist for the purposes of any of the iterator methods,
    e.g. iter_has_child on an on-demand node which hasn't been expanded yet
    is false. Likewise the rest of the iterator methods skip over or otherwise
    ignore that node.)

    Normal usage is the user clicks on the expander, the expansion mechanism
    disables on-demand nodes (set_children_on_demand(false)) and calls
    any installed expanding-callback (installable via connect_expanding) which
    has the opportunity to populate the node with children.

    If you decide to directly populate the children of an on-demand node
    outside of the expanding-callback then you also need to explicitly disable
    on-demand mode with set_children_on_demand(false); otherwise the treeview
    is in an inconsistent state.  */

    virtual bool get_row_expanded(const TreeIter& rIter) const = 0;
    // expand row will first trigger the callback set via connect_expanding before expanding
    virtual void expand_row(const TreeIter& rIter) = 0;
    // collapse row will first trigger the callback set via connect_collapsing before collapsing
    virtual void collapse_row(const TreeIter& rIter) = 0;

    // set the empty node to appear as if it has children, true is equivalent
    // to 'insert' with a bChildrenOnDemand of true. See notes above.
    void set_children_on_demand(const TreeIter& rIter, bool bChildrenOnDemand)
    {
        disable_notify_events();
        do_set_children_on_demand(rIter, bChildrenOnDemand);
        enable_notify_events();
    }

    // return if the node is configured to be populated on-demand
    virtual bool get_children_on_demand(const TreeIter& rIter) const = 0;
    // set if the expanders are shown or not
    virtual void set_show_expanders(bool bShow) = 0;

    void connect_expanding(const Link<const TreeIter&, bool>& rLink) { m_aExpandingHdl = rLink; }
    void connect_collapsing(const Link<const TreeIter&, bool>& rLink) { m_aCollapsingHdl = rLink; }

    // rStartLink returns true to allow editing, false to disallow
    // rEndLink returns true to accept the edit, false to reject
    virtual void connect_editing(const Link<const TreeIter&, bool>& rStartLink,
                                 const Link<const iter_string&, bool>& rEndLink)
    {
        assert(rStartLink.IsSet() == rEndLink.IsSet() && "should be both on or both off");
        m_aEditingStartedHdl = rStartLink;
        m_aEditingDoneHdl = rEndLink;
    }

    virtual void start_editing(const weld::TreeIter& rEntry) = 0;
    virtual void end_editing() = 0;

    virtual void connect_visible_range_changed(const Link<TreeView&, void>& rLink)
    {
        assert(!m_aVisibleRangeChangedHdl.IsSet() || !rLink.IsSet());
        m_aVisibleRangeChangedHdl = rLink;
    }

    virtual void enable_drag_source(rtl::Reference<TransferDataContainer>& rTransferable,
                                    sal_uInt8 eDNDConstants)
        = 0;

    void connect_drag_begin(const Link<bool&, bool>& rLink) { m_aDragBeginHdl = rLink; }

    // afterwards, entries will be in default ascending sort order
    virtual void make_sorted() = 0;
    virtual void make_unsorted() = 0;
    virtual bool get_sort_order() const = 0;
    virtual void set_sort_order(bool bAscending) = 0;

    // TRUE ascending, FALSE, descending, INDET, neither (off)
    virtual void set_sort_indicator(TriState eState, int nColumn) = 0;
    virtual TriState get_sort_indicator(int nColumn) const = 0;

    virtual int get_sort_column() const = 0;
    virtual void set_sort_column(int nColumn) = 0;

    virtual void
    set_sort_func(const std::function<int(const weld::TreeIter&, const weld::TreeIter&)>& func)
    {
        m_aCustomSort = func;
    }

    virtual int get_height_rows(int nRows) const = 0;

    virtual void columns_autosize() = 0;
    virtual void set_column_fixed_widths(const std::vector<int>& rWidths) = 0;
    virtual void set_column_editables(const std::vector<bool>& rEditables) = 0;
    virtual int get_column_width(int nCol) const = 0;
    virtual void set_centered_column(int nCol) = 0;
    virtual OUString get_column_title(int nColumn) const = 0;
    virtual void set_column_title(int nColumn, const OUString& rTitle) = 0;

    int get_checkbox_column_width() const { return get_approximate_digit_width() * 3 + 6; }

    virtual void set_selection_mode(SelectionMode eMode) = 0;
    virtual int count_selected_rows() const = 0;

    // remove the selected nodes
    void remove_selection()
    {
        disable_notify_events();
        do_remove_selection();
        enable_notify_events();
    }

    // only meaningful is call this from a "changed" callback, true if the change
    // was due to mouse hovering over the entry
    virtual bool changed_by_hover() const = 0;

    virtual void vadjustment_set_value(int value) = 0;
    virtual int vadjustment_get_value() const = 0;

    // for custom rendering a cell
    void connect_custom_get_size(const Link<get_size_args, Size>& rLink) { m_aGetSizeHdl = rLink; }
    void connect_custom_render(const Link<render_args, void>& rLink) { m_aRenderHdl = rLink; }
    // call set_column_custom_renderer after setting custom callbacks
    virtual void set_column_custom_renderer(int nColumn, bool bEnable) = 0;
    // redraw all rows, typically only useful with custom rendering to redraw due to external
    // state change
    virtual void queue_draw() = 0;

    /* with bDnDMode false simply return the row under the point
     *
     * with bDnDMode true:
     * a) return the row which should be dropped on, which may
     *    be different from the row the mouse is over in some backends where
     *    positioning the mouse on the bottom half of a row indicates to drop
     *    after the row
     * b) dnd highlight the dest row
     */
    virtual std::unique_ptr<weld::TreeIter> get_dest_row_at_pos(const Point& rPos, bool bDnDMode,
                                                                bool bAutoScroll = true)
        = 0;
    virtual void unset_drag_dest_row() = 0;
    virtual tools::Rectangle get_row_area(const weld::TreeIter& rIter) const = 0;
    // for dragging and dropping between TreeViews, return the active source
    virtual TreeView* get_drag_source() const = 0;

    using Widget::set_sensitive;
    using Widget::get_sensitive;

private:
    void last_child(weld::TreeIter& rIter, int nChildren) const;
};
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
