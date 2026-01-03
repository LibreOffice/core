/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/weld/TextView.hxx>

namespace weld
{
void weld::TextView::set_text(const OUString& rText)
{
    disable_notify_events();
    do_set_text(rText);
    enable_notify_events();
}

void weld::TextView::select_region(int nStartPos, int nEndPos)
{
    disable_notify_events();
    do_select_region(nStartPos, nEndPos);
    enable_notify_events();
}

void weld::TextView::replace_selection(const OUString& rText)
{
    disable_notify_events();
    do_replace_selection(rText);
    enable_notify_events();
}

int weld::TextView::get_height_rows(int nRows) const
{
    // can improve this if needed
    return get_text_height() * nRows;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
