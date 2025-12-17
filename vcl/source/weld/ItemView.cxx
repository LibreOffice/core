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
void ItemView::select(int pos)
{
    disable_notify_events();
    do_select(pos);
    enable_notify_events();
}

void ItemView::unselect(int pos)
{
    disable_notify_events();
    do_unselect(pos);
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
