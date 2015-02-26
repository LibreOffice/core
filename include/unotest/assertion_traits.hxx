/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_UNOTEST_ASSERTION_TRAITS_HXX
#define INCLUDED_UNOTEST_ASSERTION_TRAITS_HXX

// sal/types.h declares typedefs to signed char (sal_Int8) and unsigned char
// (sal_uInt8, sal_Bool), so better specialize CppUnit::assertion_traits for
// those two types to treat the toString() value as an integer rather than a
// character:

#include <sal/config.h>

#include <string>

namespace CppUnit {

template<> struct assertion_traits<signed char> {
    static bool equal(signed char x, signed char y) { return x == y; }

    static std::string toString(signed char x)
    { return std::to_string(static_cast<int>(x)); }
};

template<> struct assertion_traits<unsigned char> {
    static bool equal(unsigned char x, unsigned char y) { return x == y; }

    static std::string toString(unsigned char x)
    { return std::to_string(static_cast<unsigned int>(x)); }
};

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
