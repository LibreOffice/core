/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/weld/ItemView.hxx>

namespace weld
{
OUString ItemView::get_id(int pos) const
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(pos))
        return get_id(*pIter);

    return OUString();
}

void ItemView::set_id(int pos, const OUString& rId)
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(pos))
        return set_id(*pIter, rId);
}

void ItemView::set_cursor(const TreeIter& rIter)
{
    disable_notify_events();
    do_set_cursor(rIter);
    enable_notify_events();
}

void ItemView::select(int pos)
{
    if (pos == -1 || (pos == 0 && n_children() == 0))
    {
        unselect_all();
        return;
    }

    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(pos))
        return select(*pIter);
}

void ItemView::select(const TreeIter& rIter)
{
    disable_notify_events();
    do_select(rIter);
    enable_notify_events();
}

void ItemView::unselect(int pos)
{
    if (pos == -1)
    {
        select_all();
        return;
    }

    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(pos))
        return unselect(*pIter);
}

void ItemView::unselect(const TreeIter& rIter)
{
    disable_notify_events();
    do_unselect(rIter);
    enable_notify_events();
}

void ItemView::select_all()
{
    disable_notify_events();
    do_select_all();
    enable_notify_events();
}

void ItemView::unselect_all()
{
    disable_notify_events();
    do_unselect_all();
    enable_notify_events();
}

void ItemView::clear()
{
    disable_notify_events();
    do_clear();
    enable_notify_events();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
