/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <sal/types.h>
#include <svl/adrparse.hxx>

namespace
{
class Test : public CppUnit::TestFixture
{
    void testRfc822ExampleAddresses()
    {
        // Examples taken from section A.1 "Examples: Addresses" of
        // <https://tools.ietf.org/html/rfc822> "Standard for the Format of ARPA Internet Text
        // Messages":
        {
            SvAddressParser p("Alfred Neuman <Neuman@BBN-TENEXA>");
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), p.Count());
            CPPUNIT_ASSERT_EQUAL(OUString("Neuman@BBN-TENEXA"), p.GetEmailAddress(0));
        }
        {
            SvAddressParser p("Neuman@BBN-TENEXA");
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), p.Count());
            CPPUNIT_ASSERT_EQUAL(OUString("Neuman@BBN-TENEXA"), p.GetEmailAddress(0));
        }
        {
            SvAddressParser p("\"George, Ted\" <Shared@Group.Arpanet>");
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), p.Count());
            CPPUNIT_ASSERT_EQUAL(OUString("Shared@Group.Arpanet"), p.GetEmailAddress(0));
        }
        {
            SvAddressParser p("Wilt . (the  Stilt) Chamberlain@NBA.US");
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), p.Count());
            CPPUNIT_ASSERT_EQUAL(OUString("Wilt.Chamberlain@NBA.US"), p.GetEmailAddress(0));
        }
        {
            SvAddressParser p("Gourmets:  Pompous Person <WhoZiWhatZit@Cordon-Bleu>,\n"
                              "           Childs@WGBH.Boston, Galloping Gourmet@\n"
                              "           ANT.Down-Under (Australian National Television),\n"
                              "           Cheapie@Discount-Liquors;,\n"
                              "  Cruisers:  Port@Portugal, Jones@SEA;,\n"
                              "    Another@Somewhere.SomeOrg");
            CPPUNIT_ASSERT_EQUAL(sal_Int32(7), p.Count());
            CPPUNIT_ASSERT_EQUAL(OUString("WhoZiWhatZit@Cordon-Bleu"), p.GetEmailAddress(0));
            CPPUNIT_ASSERT_EQUAL(OUString("Childs@WGBH.Boston"), p.GetEmailAddress(1));
            CPPUNIT_ASSERT_EQUAL(OUString("Gourmet@ANT.Down-Under"), p.GetEmailAddress(2));
            CPPUNIT_ASSERT_EQUAL(OUString("Cheapie@Discount-Liquors"), p.GetEmailAddress(3));
            CPPUNIT_ASSERT_EQUAL(OUString("Port@Portugal"), p.GetEmailAddress(4));
            CPPUNIT_ASSERT_EQUAL(OUString("Jones@SEA"), p.GetEmailAddress(5));
            CPPUNIT_ASSERT_EQUAL(OUString("Another@Somewhere.SomeOrg"), p.GetEmailAddress(6));
        }
    }

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testRfc822ExampleAddresses);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
