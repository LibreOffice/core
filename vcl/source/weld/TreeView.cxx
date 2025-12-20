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
    std::unique_ptr<weld::TreeIter> pIter = make_iterator();
    if (get_selected(pIter.get()))
        return get_iter_index_in_parent(*pIter);

    return -1;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
