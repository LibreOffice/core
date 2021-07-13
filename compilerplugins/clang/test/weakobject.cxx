/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "config_clang.h"

namespace cppu
{
class OWeakObject
{
};
}

class Foo1 : public cppu::OWeakObject
{
};
class Foo2 : public cppu::OWeakObject
{
};

// expected-error@+1 {{more than one copy of cppu::OWeakObject inherited [loplugin:weakobject]}}
class Foo3 : public Foo1, public Foo2
{
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
