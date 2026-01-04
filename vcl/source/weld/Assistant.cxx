/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/weld/Assistant.hxx>

namespace weld
{
void Assistant::set_current_page(int nPage)
{
    disable_notify_events();
    do_set_current_page(nPage);
    enable_notify_events();
}

void Assistant::set_current_page(const OUString& rIdent)
{
    disable_notify_events();
    do_set_current_page(rIdent);
    enable_notify_events();
}

void Assistant::set_page_index(const OUString& rIdent, int nIndex)
{
    disable_notify_events();
    do_set_page_index(rIdent, nIndex);
    enable_notify_events();
}

void Assistant::set_page_title(const OUString& rIdent, const OUString& rTitle)
{
    disable_notify_events();
    do_set_page_title(rIdent, rTitle);
    enable_notify_events();
}

void Assistant::set_page_sensitive(const OUString& rIdent, bool bSensitive)
{
    disable_notify_events();
    do_set_page_sensitive(rIdent, bSensitive);
    enable_notify_events();
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
