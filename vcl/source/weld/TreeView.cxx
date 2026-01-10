/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/weld/TreeView.hxx>

namespace weld
{
int weld::TreeView::get_selected_index() const
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_selected())
        return get_iter_index_in_parent(*pIter);

    return -1;
}

OUString weld::TreeView::get_text(int row, int col) const
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(row))
        return get_text(*pIter, col);

    return OUString();
}

void weld::TreeView::set_text(int row, const OUString& rText, int col)
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(row))
        set_text(*pIter, rText, col);
}

void weld::TreeView::set_sensitive(int row, bool bSensitive, int col)
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(row))
        set_sensitive(*pIter, bSensitive, col);
}

bool weld::TreeView::get_sensitive(int row, int col) const
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(row))
        return get_sensitive(*pIter, col);

    return false;
}

void weld::TreeView::set_toggle(int row, TriState eState, int col)
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(row))
        set_toggle(*pIter, eState, col);
}

TriState weld::TreeView::get_toggle(int row, int col) const
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(row))
        return get_toggle(*pIter, col);

    return TRISTATE_INDET;
}

void weld::TreeView::set_image(int row, const OUString& rImage, int col)
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(row))
        set_image(*pIter, rImage, col);
}

void weld::TreeView::set_image(int row, VirtualDevice& rImage, int col)
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(row))
        set_image(*pIter, rImage, col);
}

void weld::TreeView::set_image(int row, const css::uno::Reference<css::graphic::XGraphic>& rImage,
                               int col)
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(row))
        set_image(*pIter, rImage, col);
}

void weld::TreeView::set_text_emphasis(int row, bool bOn, int col)
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(row))
        set_text_emphasis(*pIter, bOn, col);
}

bool weld::TreeView::get_text_emphasis(int row, int col) const
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(row))
        return get_text_emphasis(*pIter, col);

    return false;
}

void weld::TreeView::set_text_align(int row, TxtAlign eAlign, int col)
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(row))
        set_text_align(*pIter, eAlign, col);
}

void weld::TreeView::set_font_color(int pos, const Color& rColor)
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(pos))
        set_font_color(*pIter, rColor);
}

void weld::TreeView::scroll_to_row(int row)
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(row))
        scroll_to_row(*pIter);
}

void weld::TreeView::scroll_to_row(const TreeIter& rIter)
{
    disable_notify_events();
    do_scroll_to_row(rIter);
    enable_notify_events();
}

bool weld::TreeView::is_selected(int pos) const
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(pos))
        return is_selected(*pIter);

    return false;
}

bool weld::TreeView::iter_next(TreeIter& rIter) const
{
    if (iter_children(rIter))
        return true;

    if (iter_next_sibling(rIter))
        return true;

    // Move up level(s) until we find the level where the next node exists.
    while (iter_parent(rIter))
    {
        if (iter_next_sibling(rIter))
            return true;
    }

    return false;
}

void weld::TreeView::last_child(weld::TreeIter& rIter, int nChildren) const
{
    iter_nth_child(rIter, nChildren - 1);
    nChildren = iter_n_children(rIter);
    if (nChildren)
        last_child(rIter, nChildren);
}

bool weld::TreeView::iter_previous(weld::TreeIter& rIter) const
{
    if (iter_previous_sibling(rIter))
    {
        // Move down level(s) until we find the level where the last node exists.
        const int nChildren = iter_n_children(rIter);
        if (!nChildren)
            return true;
        last_child(rIter, nChildren);
        return true;
    }

    // Move up level
    return iter_parent(rIter);
}

bool weld::TreeView::iter_children(TreeIter& rIter) const
{
    if (get_children_on_demand(rIter))
        return false;

    return do_iter_children(rIter);
}

bool weld::TreeView::iter_has_child(const TreeIter& rIter) const
{
    std::unique_ptr<weld::TreeIter> pIter = make_iterator(&rIter);
    return iter_children(*pIter);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
