/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/weld/IconView.hxx>

namespace weld
{
tools::Rectangle IconView::get_rect(int pos) const
{
    if (std::unique_ptr<weld::TreeIter> pIter = get_iterator(pos))
        return get_rect(*pIter);

    return {};
}

std::unique_ptr<weld::TreeIter> IconView::get_item_at_pos(const Point& rPos)
{
    std::unique_ptr<weld::TreeIter> pIter = make_iterator();
    bool bIterValid = get_iter_first(*pIter);
    while (bIterValid)
    {
        if (get_rect(*pIter).Contains(rPos))
            return pIter;

        bIterValid = iter_next_sibling(*pIter);
    }

    return {};
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
