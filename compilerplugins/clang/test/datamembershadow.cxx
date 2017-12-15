/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <config_clang.h>

struct Bar {
    int x; // expected-note {{superclass member here [loplugin:datamembershadow]}}
};

struct Foo : public Bar {
    int x; // expected-error {{data member x is shadowing member in superclass, through inheritance path Foo->Bar [loplugin:datamembershadow]}}
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
