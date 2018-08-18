/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <rtl/ustring.hxx>

/**
 * We replicate the layout of an rtl_uString so that we can declare a static const
 * global OUString.
 */
template <int N> struct StaticHelper final
{
    oslInterlockedCount refCount;
    sal_Int32 length;
    sal_Unicode buffer[N];
};

/**
 * The magic constant indicates to the sal/rtl/strtmpl.cxx code that this instance is static
 * and therefore not subject to reference-counting
 *
 * @param name  the OUString static const var name we want to declare
 * @value value the literal string value to initialise the var with. Must be a utf-16 literal.
 */
#define DECLARE_STATIC_OUSTRING(name, value)                                                       \
    static const StaticHelper<SAL_N_ELEMENTS(value)> name##magic{ 0x40000000,                      \
                                                                  SAL_N_ELEMENTS(value) - 1,       \
                                                                  value };                         \
    static OUString const name(                                                                    \
        const_cast<rtl_uString*>(reinterpret_cast<rtl_uString const*>(&(name##magic))),            \
        SAL_NO_ACQUIRE);

namespace rtl_OUStringStatic
{
class First : public CppUnit::TestFixture
{
public:
    void test1()
    {
        DECLARE_STATIC_OUSTRING(b, u"abc");

        // do some operations on the field to be sure it's real and working
        OUString c = b;
        CPPUNIT_ASSERT_EQUAL(c, b);
        CPPUNIT_ASSERT_EQUAL(OUString("abc"), b);
        CPPUNIT_ASSERT(b.startsWith("a"));
        CPPUNIT_ASSERT(b.startsWith("ab"));
        CPPUNIT_ASSERT(b.startsWith("abc"));
        CPPUNIT_ASSERT(b.endsWith("abc"));
        CPPUNIT_ASSERT(b.endsWith("bc"));
        CPPUNIT_ASSERT(b.endsWith("c"));
    }

    CPPUNIT_TEST_SUITE(First);
    CPPUNIT_TEST(test1);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(rtl_OUStringStatic::First);

} // namespace rtl_OUString

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
