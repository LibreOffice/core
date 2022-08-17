/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/diagnose_ex.hxx>
#include <sal/log.hxx>

void func1();

int main()
{
    try
    {
        func1();
    }
    catch (std::exception const&)
    {
        SAL_WARN("xmloff", "message");
        DBG_UNHANDLED_EXCEPTION(
            "xmloff",
            "message"); // expected-error@-2 {{DBG_UNHANDLED_EXCEPTION must be first statement in catch block [loplugin:dbgunhandledexception]}}
    }
    try
    {
        func1();
    }
    catch (std::exception const&)
    {
        DBG_UNHANDLED_EXCEPTION("xmloff", "message");
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
