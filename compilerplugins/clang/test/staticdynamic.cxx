/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

struct ClassA
{
    virtual ~ClassA() {}
};

struct ClassB : public ClassA
{
    void foo() {}
};

void f1(ClassA* p1)
{
    // expected-note@+1 {{static_cast here [loplugin:staticdynamic]}}
    static_cast<ClassB*>(p1)->foo();
    // expected-error@+1 {{dynamic_cast after static_cast [loplugin:staticdynamic]}}
    dynamic_cast<ClassB*>(p1)->foo();
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
