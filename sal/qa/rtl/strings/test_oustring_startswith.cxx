/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <rtl/ustring.hxx>

namespace test::oustring {

class StartsWith: public CppUnit::TestFixture
{
private:
    void startsWith();

    CPPUNIT_TEST_SUITE(StartsWith);
    CPPUNIT_TEST(startsWith);
    CPPUNIT_TEST_SUITE_END();
};

}

CPPUNIT_TEST_SUITE_REGISTRATION(test::oustring::StartsWith);

void test::oustring::StartsWith::startsWith()
{
    CPPUNIT_ASSERT( u"foobar"_ustr.startsWith( "foo" ));
    CPPUNIT_ASSERT( !u"foo"_ustr.startsWith( "foobar" ));
    CPPUNIT_ASSERT( !u"foobar"_ustr.startsWith( "oo" ));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
