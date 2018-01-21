/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>
#include <com/sun/star/uno/Sequence.hxx>

#define FALSE 0

void func_ptr(int*);
void func_bool(bool); // expected-note {{method here [loplugin:pointerbool]}}
void func_salbool(sal_Bool);

void test1(int* p1)
{
    func_ptr(p1);
    func_bool(
        p1); // expected-error {{possibly unwanted implicit conversion when calling bool param [loplugin:pointerbool]}}
    // no warning expected
    func_bool(FALSE);
    func_salbool(sal_False);
    func_salbool(sal_True);
    css::uno::Sequence<sal_Bool> aSeq;
    func_bool(aSeq[0]);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
