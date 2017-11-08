/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <list>
#include <string>
#include <vector>

namespace
{
template <typename T> using Vec = std::vector<T>;
}

int main()
{
    std::list<int> v1; // expected-error {{unused variable 'v1' [loplugin:unusedvariablecheck]}}
    std::string v2; // expected-error {{unused variable 'v2' [loplugin:unusedvariablecheck]}}
    Vec<int> v3; // expected-error {{unused variable 'v3' [loplugin:unusedvariablecheck]}}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
