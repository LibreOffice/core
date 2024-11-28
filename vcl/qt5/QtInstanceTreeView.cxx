/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <QtInstanceTreeView.hxx>
#include <QtInstanceTreeView.moc>

#include <vcl/qt/QtUtils.hxx>

QtInstanceTreeView::QtInstanceTreeView(QTreeView* pTreeView)
    : QtInstanceWidget(pTreeView)
    , m_pTreeView(pTreeView)
{
    assert(m_pTreeView);
}

void QtInstanceTreeView::insert(const weld::TreeIter*, int, const OUString*, const OUString*,
                                const OUString*, VirtualDevice*, bool, weld::TreeIter*)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::insert_separator(int, const OUString&)
{
    assert(false && "Not implemented yet");
}

OUString QtInstanceTreeView::get_selected_text() const
{
    assert(false && "Not implemented yet");
    return OUString();
}

OUString QtInstanceTreeView::get_selected_id() const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceTreeView::enable_toggle_buttons(weld::ColumnToggleType)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_clicks_to_toggle(int) { assert(false && "Not implemented yet"); }

int QtInstanceTreeView::get_selected_index() const
{
    assert(false && "Not implemented yet");
    return -1;
}

void QtInstanceTreeView::select(int) { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::unselect(int) { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::remove(int) { assert(false && "Not implemented yet"); }

OUString QtInstanceTreeView::get_text(int, int) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceTreeView::set_text(int, const OUString&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_sensitive(int, bool, int) { assert(false && "Not implemented yet"); }

bool QtInstanceTreeView::get_sensitive(int, int) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::set_id(int, const OUString&) { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::set_toggle(int, TriState, int) { assert(false && "Not implemented yet"); }

TriState QtInstanceTreeView::get_toggle(int, int) const
{
    assert(false && "Not implemented yet");
    return TRISTATE_INDET;
}

void QtInstanceTreeView::set_image(int, const OUString&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_image(int, VirtualDevice&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_image(int, const css::uno::Reference<css::graphic::XGraphic>&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_text_emphasis(int, bool, int)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceTreeView::get_text_emphasis(int, int) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::set_text_align(int, double, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::swap(int, int) { assert(false && "Not implemented yet"); }

std::vector<int> QtInstanceTreeView::get_selected_rows() const
{
    assert(false && "Not implemented yet");
    return std::vector<int>();
}

void QtInstanceTreeView::set_font_color(int, const Color&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::scroll_to_row(int) { assert(false && "Not implemented yet"); }

bool QtInstanceTreeView::is_selected(int) const
{
    assert(false && "Not implemented yet");
    return false;
}

int QtInstanceTreeView::get_cursor_index() const
{
    assert(false && "Not implemented yet");
    return -1;
}

void QtInstanceTreeView::set_cursor(int) { assert(false && "Not implemented yet"); }

int QtInstanceTreeView::find_text(const OUString&) const
{
    assert(false && "Not implemented yet");
    return -1;
}

OUString QtInstanceTreeView::get_id(int) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

int QtInstanceTreeView::find_id(const OUString&) const
{
    assert(false && "Not implemented yet");
    return -1;
}

std::unique_ptr<weld::TreeIter> QtInstanceTreeView::make_iterator(const weld::TreeIter*) const
{
    assert(false && "Not implemented yet");
    return nullptr;
}

void QtInstanceTreeView::copy_iterator(const weld::TreeIter&, weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
}

bool QtInstanceTreeView::get_selected(weld::TreeIter*) const
{
    assert(false && "Not implemented yet");
    return false;
}

bool QtInstanceTreeView::get_cursor(weld::TreeIter*) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::set_cursor(const weld::TreeIter&)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceTreeView::get_iter_first(weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

bool QtInstanceTreeView::iter_next_sibling(weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

bool QtInstanceTreeView::iter_previous_sibling(weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

bool QtInstanceTreeView::iter_next(weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

bool QtInstanceTreeView::iter_previous(weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

bool QtInstanceTreeView::iter_children(weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

bool QtInstanceTreeView::iter_parent(weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

int QtInstanceTreeView::get_iter_depth(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return -1;
}

int QtInstanceTreeView::get_iter_index_in_parent(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return -1;
}

int QtInstanceTreeView::iter_compare(const weld::TreeIter&, const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return 0;
}

bool QtInstanceTreeView::iter_has_child(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

int QtInstanceTreeView::iter_n_children(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return -1;
}

void QtInstanceTreeView::remove(const weld::TreeIter&) { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::select(const weld::TreeIter&) { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::unselect(const weld::TreeIter&) { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::set_extra_row_indent(const weld::TreeIter&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_text(const weld::TreeIter&, const OUString&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_sensitive(const weld::TreeIter&, bool, int)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceTreeView::get_sensitive(const weld::TreeIter&, int) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::set_text_emphasis(const weld::TreeIter&, bool, int)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceTreeView::get_text_emphasis(const weld::TreeIter&, int) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::set_text_align(const weld::TreeIter&, double, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_toggle(const weld::TreeIter&, TriState, int)
{
    assert(false && "Not implemented yet");
}

TriState QtInstanceTreeView::get_toggle(const weld::TreeIter&, int) const
{
    assert(false && "Not implemented yet");
    return TRISTATE_INDET;
}

OUString QtInstanceTreeView::get_text(const weld::TreeIter&, int) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceTreeView::set_id(const weld::TreeIter&, const OUString&)
{
    assert(false && "Not implemented yet");
}

OUString QtInstanceTreeView::get_id(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceTreeView::set_image(const weld::TreeIter&, const OUString&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_image(const weld::TreeIter&, VirtualDevice&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_image(const weld::TreeIter&,
                                   const css::uno::Reference<css::graphic::XGraphic>&, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_font_color(const weld::TreeIter&, const Color&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::scroll_to_row(const weld::TreeIter&)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceTreeView::is_selected(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::move_subtree(weld::TreeIter&, const weld::TreeIter*, int)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::all_foreach(const std::function<bool(weld::TreeIter&)>&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::selected_foreach(const std::function<bool(weld::TreeIter&)>&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::visible_foreach(const std::function<bool(weld::TreeIter&)>&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::bulk_insert_for_each(
    int, const std::function<void(weld::TreeIter&, int nSourceIndex)>&, const weld::TreeIter*,
    const std::vector<int>*, bool)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceTreeView::get_row_expanded(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::expand_row(const weld::TreeIter&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::collapse_row(const weld::TreeIter&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_children_on_demand(const weld::TreeIter&, bool)
{
    assert(false && "Not implemented yet");
}

bool QtInstanceTreeView::get_children_on_demand(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::set_show_expanders(bool) { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::start_editing(const weld::TreeIter&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::end_editing() { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::enable_drag_source(rtl::Reference<TransferDataContainer>&, sal_uInt8)
{
    assert(false && "Not implemented yet");
}

int QtInstanceTreeView::n_children() const
{
    assert(false && "Not implemented yet");
    return 0;
}

void QtInstanceTreeView::make_sorted() { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::make_unsorted() { assert(false && "Not implemented yet"); }

bool QtInstanceTreeView::get_sort_order() const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::set_sort_order(bool) { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::set_sort_indicator(TriState, int)
{
    assert(false && "Not implemented yet");
}

TriState QtInstanceTreeView::get_sort_indicator(int) const
{
    assert(false && "Not implemented yet");
    return TRISTATE_INDET;
}

int QtInstanceTreeView::get_sort_column() const
{
    assert(false && "Not implemented yet");
    return -1;
}

void QtInstanceTreeView::set_sort_column(int) { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::clear() { assert(false && "Not implemented yet"); }

int QtInstanceTreeView::get_height_rows(int) const
{
    assert(false && "Not implemented yet");
    return 0;
}

void QtInstanceTreeView::columns_autosize() { assert(false && "Not implemented yet"); }

void QtInstanceTreeView::set_column_fixed_widths(const std::vector<int>&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_column_editables(const std::vector<bool>&)
{
    assert(false && "Not implemented yet");
}

int QtInstanceTreeView::get_column_width(int) const
{
    assert(false && "Not implemented yet");
    return 0;
}

void QtInstanceTreeView::set_centered_column(int) { assert(false && "Not implemented yet"); }

OUString QtInstanceTreeView::get_column_title(int) const
{
    assert(false && "Not implemented yet");
    return OUString();
}

void QtInstanceTreeView::set_column_title(int, const OUString&)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::set_selection_mode(SelectionMode eMode)
{
    SolarMutexGuard g;
    GetQtInstance().RunInMainThread(
        [&] { m_pTreeView->setSelectionMode(mapSelectionMode(eMode)); });
}

int QtInstanceTreeView::count_selected_rows() const
{
    assert(false && "Not implemented yet");
    return 0;
}

void QtInstanceTreeView::remove_selection() { assert(false && "Not implemented yet"); }

bool QtInstanceTreeView::changed_by_hover() const
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::vadjustment_set_value(int) { assert(false && "Not implemented yet"); }

int QtInstanceTreeView::vadjustment_get_value() const
{
    assert(false && "Not implemented yet");
    return -1;
}

void QtInstanceTreeView::set_column_custom_renderer(int, bool)
{
    assert(false && "Not implemented yet");
}

void QtInstanceTreeView::queue_draw() { assert(false && "Not implemented yet"); }

bool QtInstanceTreeView::get_dest_row_at_pos(const Point&, weld::TreeIter*, bool, bool)
{
    assert(false && "Not implemented yet");
    return false;
}

void QtInstanceTreeView::unset_drag_dest_row() { assert(false && "Not implemented yet"); }

tools::Rectangle QtInstanceTreeView::get_row_area(const weld::TreeIter&) const
{
    assert(false && "Not implemented yet");
    return tools::Rectangle();
}

weld::TreeView* QtInstanceTreeView::get_drag_source() const
{
    assert(false && "Not implemented yet");
    return nullptr;
}

QAbstractItemView::SelectionMode QtInstanceTreeView::mapSelectionMode(SelectionMode eMode)
{
    switch (eMode)
    {
        case SelectionMode::NONE:
            return QAbstractItemView::NoSelection;
        case SelectionMode::Single:
            return QAbstractItemView::SingleSelection;
        case SelectionMode::Range:
            return QAbstractItemView::ContiguousSelection;
        case SelectionMode::Multiple:
            return QAbstractItemView::ExtendedSelection;
        default:
            assert(false && "unhandled selection mode");
            return QAbstractItemView::SingleSelection;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
