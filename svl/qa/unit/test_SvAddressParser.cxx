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
            SvAddressParser p(u"Alfred Neuman <Neuman@BBN-TENEXA>"_ustr);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), p.Count());
            CPPUNIT_ASSERT_EQUAL(u"Neuman@BBN-TENEXA"_ustr, p.GetEmailAddress(0));
        }
        {
            SvAddressParser p(u"Neuman@BBN-TENEXA"_ustr);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), p.Count());
            CPPUNIT_ASSERT_EQUAL(u"Neuman@BBN-TENEXA"_ustr, p.GetEmailAddress(0));
        }
        {
            SvAddressParser p(u"\"George, Ted\" <Shared@Group.Arpanet>"_ustr);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), p.Count());
            CPPUNIT_ASSERT_EQUAL(u"Shared@Group.Arpanet"_ustr, p.GetEmailAddress(0));
        }
        {
            SvAddressParser p(u"Wilt . (the  Stilt) Chamberlain@NBA.US"_ustr);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(1), p.Count());
            CPPUNIT_ASSERT_EQUAL(u"Wilt.Chamberlain@NBA.US"_ustr, p.GetEmailAddress(0));
        }
        {
            SvAddressParser p(u"Gourmets:  Pompous Person <WhoZiWhatZit@Cordon-Bleu>,\n"
                              "           Childs@WGBH.Boston, Galloping Gourmet@\n"
                              "           ANT.Down-Under (Australian National Television),\n"
                              "           Cheapie@Discount-Liquors;,\n"
                              "  Cruisers:  Port@Portugal, Jones@SEA;,\n"
                              "    Another@Somewhere.SomeOrg"_ustr);
            CPPUNIT_ASSERT_EQUAL(sal_Int32(7), p.Count());
            CPPUNIT_ASSERT_EQUAL(u"WhoZiWhatZit@Cordon-Bleu"_ustr, p.GetEmailAddress(0));
            CPPUNIT_ASSERT_EQUAL(u"Childs@WGBH.Boston"_ustr, p.GetEmailAddress(1));
            CPPUNIT_ASSERT_EQUAL(u"Gourmet@ANT.Down-Under"_ustr, p.GetEmailAddress(2));
            CPPUNIT_ASSERT_EQUAL(u"Cheapie@Discount-Liquors"_ustr, p.GetEmailAddress(3));
            CPPUNIT_ASSERT_EQUAL(u"Port@Portugal"_ustr, p.GetEmailAddress(4));
            CPPUNIT_ASSERT_EQUAL(u"Jones@SEA"_ustr, p.GetEmailAddress(5));
            CPPUNIT_ASSERT_EQUAL(u"Another@Somewhere.SomeOrg"_ustr, p.GetEmailAddress(6));
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
