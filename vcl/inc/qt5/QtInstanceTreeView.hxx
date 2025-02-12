/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include "QtInstanceWidget.hxx"

#include <QtGui/QStandardItemModel>
#include <QtWidgets/QTreeView>

class QtInstanceTreeView : public QtInstanceWidget, public virtual weld::TreeView
{
    Q_OBJECT

    QTreeView* m_pTreeView;
    QStandardItemModel* m_pModel;
    QItemSelectionModel* m_pSelectionModel;

public:
    QtInstanceTreeView(QTreeView* pTreeView);

    virtual void insert(const weld::TreeIter* pParent, int nPos, const OUString* pStr,
                        const OUString* pId, const OUString* pIconName,
                        VirtualDevice* pImageSurface, bool bChildrenOnDemand,
                        weld::TreeIter* pRet) override;

    virtual void insert_separator(int pos, const OUString& rId) override;

    virtual OUString get_selected_text() const override;
    virtual OUString get_selected_id() const override;

    virtual void enable_toggle_buttons(weld::ColumnToggleType eType) override;

    virtual void set_clicks_to_toggle(int nToggleBehavior) override;

    virtual int get_selected_index() const override;
    virtual void select(int nPos) override;
    virtual void unselect(int nPos) override;
    virtual void remove(int nPos) override;
    virtual OUString get_text(int nRow, int nCol = -1) const override;
    virtual void set_text(int nRow, const OUString& rText, int nCol = -1) override;
    virtual void set_sensitive(int row, bool bSensitive, int col = -1) override;
    virtual bool get_sensitive(int row, int col) const override;
    virtual void set_id(int row, const OUString& rId) override;
    virtual void set_toggle(int row, TriState eState, int col = -1) override;
    virtual TriState get_toggle(int row, int col = -1) const override;
    virtual void set_image(int row, const OUString& rImage, int col = -1) override;
    virtual void set_image(int row, VirtualDevice& rImage, int col = -1) override;
    virtual void set_image(int row, const css::uno::Reference<css::graphic::XGraphic>& rImage,
                           int col = -1) override;
    virtual void set_text_emphasis(int row, bool bOn, int col) override;
    virtual bool get_text_emphasis(int row, int col) const override;
    virtual void set_text_align(int row, double fAlign, int col) override;
    virtual void swap(int pos1, int pos2) override;
    virtual std::vector<int> get_selected_rows() const override;
    virtual void set_font_color(int pos, const Color& rColor) override;
    virtual void scroll_to_row(int row) override;
    virtual bool is_selected(int pos) const override;
    virtual int get_cursor_index() const override;
    virtual void set_cursor(int pos) override;

    virtual int find_text(const OUString& rText) const override;
    virtual OUString get_id(int nPos) const override;
    virtual int find_id(const OUString& rId) const override;

    virtual std::unique_ptr<weld::TreeIter> make_iterator(const weld::TreeIter* pOrig
                                                          = nullptr) const override;
    virtual void copy_iterator(const weld::TreeIter& rSource, weld::TreeIter& rDest) const override;
    virtual bool get_selected(weld::TreeIter* pIter) const override;
    virtual bool get_cursor(weld::TreeIter* pIter) const override;
    virtual void set_cursor(const weld::TreeIter& rIter) override;
    virtual bool get_iter_first(weld::TreeIter& rIter) const override;
    virtual bool iter_next_sibling(weld::TreeIter& rIter) const override;
    virtual bool iter_previous_sibling(weld::TreeIter& rIter) const override;
    virtual bool iter_next(weld::TreeIter& rIter) const override;
    virtual bool iter_previous(weld::TreeIter& rIter) const override;
    virtual bool iter_children(weld::TreeIter& rIter) const override;
    virtual bool iter_parent(weld::TreeIter& rIter) const override;
    virtual int get_iter_depth(const weld::TreeIter& rIter) const override;
    virtual int get_iter_index_in_parent(const weld::TreeIter& rIter) const override;

    virtual int iter_compare(const weld::TreeIter& a, const weld::TreeIter& b) const override;
    virtual bool iter_has_child(const weld::TreeIter& rIter) const override;
    virtual int iter_n_children(const weld::TreeIter& rIter) const override;
    virtual void remove(const weld::TreeIter& rIter) override;
    virtual void select(const weld::TreeIter& rIter) override;
    virtual void unselect(const weld::TreeIter& rIter) override;
    virtual void set_extra_row_indent(const weld::TreeIter& rIter, int nIndentLevel) override;
    virtual void set_text(const weld::TreeIter& rIter, const OUString& rStr,
                          int nCol = -1) override;
    virtual void set_sensitive(const weld::TreeIter& rIter, bool bSensitive, int col = -1) override;
    virtual bool get_sensitive(const weld::TreeIter& rIter, int col) const override;
    virtual void set_text_emphasis(const weld::TreeIter& rIter, bool bOn, int col) override;
    virtual bool get_text_emphasis(const weld::TreeIter& rIter, int col) const override;
    virtual void set_text_align(const weld::TreeIter& rIter, double fAlign, int col) override;
    virtual void set_toggle(const weld::TreeIter& rIter, TriState bOn, int col = -1) override;
    virtual TriState get_toggle(const weld::TreeIter& rIter, int col = -1) const override;
    virtual OUString get_text(const weld::TreeIter& rIter, int col = -1) const override;
    virtual void set_id(const weld::TreeIter& rIter, const OUString& rId) override;
    virtual OUString get_id(const weld::TreeIter& rIter) const override;
    virtual void set_image(const weld::TreeIter& rIter, const OUString& rImage,
                           int nCol = -1) override;
    virtual void set_image(const weld::TreeIter& rIter, VirtualDevice& rImage,
                           int col = -1) override;
    virtual void set_image(const weld::TreeIter& rIter,
                           const css::uno::Reference<css::graphic::XGraphic>& rImage,
                           int col = -1) override;
    virtual void set_font_color(const weld::TreeIter& rIter, const Color& rColor) override;
    virtual void scroll_to_row(const weld::TreeIter& rIter) override;
    virtual bool is_selected(const weld::TreeIter& rIter) const override;

    virtual void move_subtree(weld::TreeIter& rNode, const weld::TreeIter* pNewParent,
                              int nIndexInNewParent) override;

    virtual void all_foreach(const std::function<bool(weld::TreeIter&)>& func) override;
    virtual void selected_foreach(const std::function<bool(weld::TreeIter&)>& func) override;
    virtual void visible_foreach(const std::function<bool(weld::TreeIter&)>& func) override;
    virtual void bulk_insert_for_each(
        int nSourceCount, const std::function<void(weld::TreeIter&, int nSourceIndex)>& func,
        const weld::TreeIter* pParent = nullptr, const std::vector<int>* pFixedWidths = nullptr,
        bool bGoingToSetText = false) override;

    virtual bool get_row_expanded(const weld::TreeIter& rIter) const override;
    virtual void expand_row(const weld::TreeIter& rIter) override;
    virtual void collapse_row(const weld::TreeIter& rIter) override;
    virtual void set_children_on_demand(const weld::TreeIter& rIter,
                                        bool bChildrenOnDemand) override;
    virtual bool get_children_on_demand(const weld::TreeIter& rIter) const override;
    virtual void set_show_expanders(bool bShow) override;

    virtual void start_editing(const weld::TreeIter& rEntry) override;
    virtual void end_editing() override;

    virtual void enable_drag_source(rtl::Reference<TransferDataContainer>& rTransferable,
                                    sal_uInt8 eDNDConstants) override;

    virtual int n_children() const override;

    virtual void make_sorted() override;
    virtual void make_unsorted() override;
    virtual bool get_sort_order() const override;
    virtual void set_sort_order(bool bAscending) override;

    virtual void set_sort_indicator(TriState eState, int nColumn) override;
    virtual TriState get_sort_indicator(int nColumn) const override;

    virtual int get_sort_column() const override;
    virtual void set_sort_column(int nColumn) override;

    virtual void clear() override;
    virtual int get_height_rows(int nRows) const override;

    virtual void columns_autosize() override;
    virtual void set_column_fixed_widths(const std::vector<int>& rWidths) override;
    virtual void set_column_editables(const std::vector<bool>& rEditables) override;
    virtual int get_column_width(int nCol) const override;
    virtual void set_centered_column(int nCol) override;
    virtual OUString get_column_title(int nColumn) const override;
    virtual void set_column_title(int nColumn, const OUString& rTitle) override;

    virtual void set_selection_mode(SelectionMode eMode) override;
    virtual int count_selected_rows() const override;
    virtual void remove_selection() override;

    virtual bool changed_by_hover() const override;

    virtual void vadjustment_set_value(int nValue) override;
    virtual int vadjustment_get_value() const override;

    virtual void set_column_custom_renderer(int nColumn, bool bEnable) override;
    virtual void queue_draw() override;

    virtual bool get_dest_row_at_pos(const Point& rPos, weld::TreeIter* pResult, bool bDnDMode,
                                     bool bAutoScroll = true) override;
    virtual void unset_drag_dest_row() override;
    virtual tools::Rectangle get_row_area(const weld::TreeIter& rIter) const override;
    virtual weld::TreeView* get_drag_source() const override;

    using QtInstanceWidget::set_sensitive;
    using QtInstanceWidget::get_sensitive;

private:
    QModelIndex modelIndex(int nRow, int nCol = 0) const;
    QModelIndex modelIndex(const weld::TreeIter& rIter, int nCol = 0) const;
    OUString get_id(const QModelIndex& rModelIndex) const;
    static QAbstractItemView::SelectionMode mapSelectionMode(SelectionMode eMode);

private Q_SLOTS:
    void handleActivated();
    void handleSelectionChanged();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
