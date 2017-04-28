/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_COMPILERPLUGINS_CLANG_TEST_CPPUNITASSERTEQUALS_HXX
#define INCLUDED_COMPILERPLUGINS_CLANG_TEST_CPPUNITASSERTEQUALS_HXX

#include "sal/config.h"

#include "rtl/ustring.hxx"

struct T { bool operator ==(T); };

void test(bool b1, bool b2, OUString const & s1, OUString const & s2, T t);

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
