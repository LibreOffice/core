/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/diagnose.hxx>
#include <sal/log.hxx>

void func1();

int main()
{
    SAL_WARN(
        "bob.none",
        "message"); // expected-error@-2 {{unknown log area 'bob.none' (check or extend include/sal/log-areas.dox) [loplugin:sallogareas]}}

    SAL_WARN("xmloff", "message");

    try
    {
        func1();
    }
    catch (std::exception const&)
    {
        DBG_UNHANDLED_EXCEPTION(
            "bob.none",
            "message"); // expected-error@-2 {{unknown log area 'bob.none' (check or extend include/sal/log-areas.dox) [loplugin:sallogareas]}}
    }
    try
    {
        func1();
    }
    catch (std::exception const&)
    {
        DBG_UNHANDLED_EXCEPTION("xmloff", "message");
    }
    try
    {
        func1();
    }
    catch (std::exception const&)
    {
        DBG_UNHANDLED_EXCEPTION("xmloff");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
