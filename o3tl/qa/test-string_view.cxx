/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <iostream>
#include <string_view>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <o3tl/string_view.hxx>
#include <rtl/string.hxx>
#include <rtl/ustring.hxx>

// gcc 11.2.0 triggers a spurious -Werror=stringop-overread
#if !(__GNUC__ == 11 && __GNUC_MINOR__ == 2)

namespace CppUnit
{
template <> struct assertion_traits<std::u16string_view>
{
    static bool equal(std::u16string_view x, std::u16string_view y) { return x == y; }

    static std::string toString(std::u16string_view x)
    {
        return OUStringToOString(x, RTL_TEXTENCODING_UTF8).getStr();
    }
};
}

namespace
{
OString ostringEmpty() { return {}; } // avoid loplugin:stringview
OString ostringDoof() { return "doof"; } // avoid loplugin:stringview
OString ostringFoo() { return "foo"; } // avoid loplugin:stringview
OString ostringFoobars() { return "foobars"; } // avoid loplugin:stringview
OString ostringFood() { return "food"; } // avoid loplugin:stringview
OString ostringOof() { return "oof"; } // avoid loplugin:stringview
OString ostringSraboof() { return "sraboof"; } // avoid loplugin:stringview
OUString oustringEmpty() { return {}; } // avoid loplugin:stringview
OUString oustringDoof() { return "doof"; } // avoid loplugin:stringview
OUString oustringFoo() { return "foo"; } // avoid loplugin:stringview
OUString oustringFoobars() { return "foobars"; } // avoid loplugin:stringview
OUString oustringFood() { return "food"; } // avoid loplugin:stringview
OUString oustringOof() { return "oof"; } // avoid loplugin:stringview
OUString oustringSraboof() { return "sraboof"; } // avoid loplugin:stringview

class Test : public CppUnit::TestFixture
{
private:
    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testStartsWith);
    CPPUNIT_TEST(testStartsWithRest);
    CPPUNIT_TEST(testEndsWith);
    CPPUNIT_TEST(testEndsWithRest);
    CPPUNIT_TEST_SUITE_END();

    void testStartsWith()
    {
        using namespace std::string_view_literals;
        CPPUNIT_ASSERT(o3tl::starts_with(""sv, ""sv));
        CPPUNIT_ASSERT(!o3tl::starts_with(""sv, "foo"sv));
        CPPUNIT_ASSERT(o3tl::starts_with("foobar"sv, ""sv));
        CPPUNIT_ASSERT(o3tl::starts_with("foobar"sv, "foo"sv));
        CPPUNIT_ASSERT(!o3tl::starts_with("foobar"sv, "food"sv));
        CPPUNIT_ASSERT(!o3tl::starts_with("foobar"sv, "foobars"sv));
        CPPUNIT_ASSERT(!o3tl::starts_with(""sv, 'f'));
        CPPUNIT_ASSERT(o3tl::starts_with("foobar"sv, 'f'));
        CPPUNIT_ASSERT(!o3tl::starts_with("foobar"sv, 'g'));
        CPPUNIT_ASSERT(o3tl::starts_with(""sv, ""));
        CPPUNIT_ASSERT(!o3tl::starts_with(""sv, "foo"));
        CPPUNIT_ASSERT(o3tl::starts_with("foobar"sv, ""));
        CPPUNIT_ASSERT(o3tl::starts_with("foobar"sv, "foo"));
        CPPUNIT_ASSERT(!o3tl::starts_with("foobar"sv, "food"));
        CPPUNIT_ASSERT(!o3tl::starts_with("foobar"sv, "foobars"));
        CPPUNIT_ASSERT(o3tl::starts_with(""sv, ostringEmpty()));
        CPPUNIT_ASSERT(!o3tl::starts_with(""sv, ostringFoo()));
        CPPUNIT_ASSERT(o3tl::starts_with("foobar"sv, ostringEmpty()));
        CPPUNIT_ASSERT(o3tl::starts_with("foobar"sv, ostringFoo()));
        CPPUNIT_ASSERT(!o3tl::starts_with("foobar"sv, ostringFood()));
        CPPUNIT_ASSERT(!o3tl::starts_with("foobar"sv, ostringFoobars()));
        CPPUNIT_ASSERT(o3tl::starts_with(u""sv, u""sv));
        CPPUNIT_ASSERT(!o3tl::starts_with(u""sv, u"foo"sv));
        CPPUNIT_ASSERT(o3tl::starts_with(u"foobar"sv, u""sv));
        CPPUNIT_ASSERT(o3tl::starts_with(u"foobar"sv, u"foo"sv));
        CPPUNIT_ASSERT(!o3tl::starts_with(u"foobar"sv, u"food"sv));
        CPPUNIT_ASSERT(!o3tl::starts_with(u"foobar"sv, u"foobars"sv));
        CPPUNIT_ASSERT(!o3tl::starts_with(u""sv, u'f'));
        CPPUNIT_ASSERT(o3tl::starts_with(u"foobar"sv, u'f'));
        CPPUNIT_ASSERT(!o3tl::starts_with(u"foobar"sv, u'g'));
        CPPUNIT_ASSERT(o3tl::starts_with(u""sv, u""));
        CPPUNIT_ASSERT(!o3tl::starts_with(u""sv, u"foo"));
        CPPUNIT_ASSERT(o3tl::starts_with(u"foobar"sv, u""));
        CPPUNIT_ASSERT(o3tl::starts_with(u"foobar"sv, u"foo"));
        CPPUNIT_ASSERT(!o3tl::starts_with(u"foobar"sv, u"food"));
        CPPUNIT_ASSERT(!o3tl::starts_with(u"foobar"sv, u"foobars"));
        CPPUNIT_ASSERT(o3tl::starts_with(u""sv, oustringEmpty()));
        CPPUNIT_ASSERT(!o3tl::starts_with(u""sv, oustringFoo()));
        CPPUNIT_ASSERT(o3tl::starts_with(u"foobar"sv, oustringEmpty()));
        CPPUNIT_ASSERT(o3tl::starts_with(u"foobar"sv, oustringFoo()));
        CPPUNIT_ASSERT(!o3tl::starts_with(u"foobar"sv, oustringFood()));
        CPPUNIT_ASSERT(!o3tl::starts_with(u"foobar"sv, oustringFoobars()));
    }

    void testStartsWithRest()
    {
        using namespace std::string_view_literals;
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with(""sv, ""sv, &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with(""sv, "foo"sv, &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with("foobar"sv, ""sv, &rest));
            CPPUNIT_ASSERT_EQUAL("foobar"sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with("foobar"sv, "foo"sv, &rest));
            CPPUNIT_ASSERT_EQUAL("bar"sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with("foobar"sv, "food"sv, &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with("foobar"sv, "foobars"sv, &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with(""sv, 'f', &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with("foobar"sv, 'f', &rest));
            CPPUNIT_ASSERT_EQUAL("oobar"sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with("foobar"sv, 'g', &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with(""sv, "", &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with(""sv, "foo", &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with("foobar"sv, "", &rest));
            CPPUNIT_ASSERT_EQUAL("foobar"sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with("foobar"sv, "foo", &rest));
            CPPUNIT_ASSERT_EQUAL("bar"sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with("foobar"sv, "food", &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with("foobar"sv, "foobars", &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with(""sv, ostringEmpty(), &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with(""sv, ostringFoo(), &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with("foobar"sv, ostringEmpty(), &rest));
            CPPUNIT_ASSERT_EQUAL("foobar"sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with("foobar"sv, ostringFoo(), &rest));
            CPPUNIT_ASSERT_EQUAL("bar"sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with("foobar"sv, ostringFood(), &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with("foobar"sv, ostringFoobars(), &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with(u""sv, u""sv, &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with(u""sv, u"foo"sv, &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with(u"foobar"sv, u""sv, &rest));
            CPPUNIT_ASSERT_EQUAL(u"foobar"sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with(u"foobar"sv, u"foo"sv, &rest));
            CPPUNIT_ASSERT_EQUAL(u"bar"sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with(u"foobar"sv, u"food"sv, &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with(u"foobar"sv, u"foobars"sv, &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with(u""sv, u'f', &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with(u"foobar"sv, u'f', &rest));
            CPPUNIT_ASSERT_EQUAL(u"oobar"sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with(u"foobar"sv, u'g', &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with(u""sv, u"", &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with(u""sv, u"foo", &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with(u"foobar"sv, u"", &rest));
            CPPUNIT_ASSERT_EQUAL(u"foobar"sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with(u"foobar"sv, u"foo", &rest));
            CPPUNIT_ASSERT_EQUAL(u"bar"sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with(u"foobar"sv, u"food", &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with(u"foobar"sv, u"foobars", &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with(u""sv, oustringEmpty(), &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with(u""sv, oustringFoo(), &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with(u"foobar"sv, oustringEmpty(), &rest));
            CPPUNIT_ASSERT_EQUAL(u"foobar"sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::starts_with(u"foobar"sv, oustringFoo(), &rest));
            CPPUNIT_ASSERT_EQUAL(u"bar"sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with(u"foobar"sv, oustringFood(), &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::starts_with(u"foobar"sv, oustringFoobars(), &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
    }

    void testEndsWith()
    {
        using namespace std::string_view_literals;
        CPPUNIT_ASSERT(o3tl::ends_with(""sv, ""sv));
        CPPUNIT_ASSERT(!o3tl::ends_with(""sv, "oof"sv));
        CPPUNIT_ASSERT(o3tl::ends_with("raboof"sv, ""sv));
        CPPUNIT_ASSERT(o3tl::ends_with("raboof"sv, "oof"sv));
        CPPUNIT_ASSERT(!o3tl::ends_with("raboof"sv, "doof"sv));
        CPPUNIT_ASSERT(!o3tl::ends_with("raboof"sv, "sraboof"sv));
        CPPUNIT_ASSERT(!o3tl::ends_with(""sv, 'f'));
        CPPUNIT_ASSERT(o3tl::ends_with("raboof"sv, 'f'));
        CPPUNIT_ASSERT(!o3tl::ends_with("raboof"sv, 'g'));
        CPPUNIT_ASSERT(o3tl::ends_with(""sv, ""));
        CPPUNIT_ASSERT(!o3tl::ends_with(""sv, "oof"));
        CPPUNIT_ASSERT(o3tl::ends_with("raboof"sv, ""));
        CPPUNIT_ASSERT(o3tl::ends_with("raboof"sv, "oof"));
        CPPUNIT_ASSERT(!o3tl::ends_with("raboof"sv, "doof"));
        CPPUNIT_ASSERT(!o3tl::ends_with("raboof"sv, "sraboof"));
        CPPUNIT_ASSERT(o3tl::ends_with(""sv, ostringEmpty()));
        CPPUNIT_ASSERT(!o3tl::ends_with(""sv, ostringOof()));
        CPPUNIT_ASSERT(o3tl::ends_with("raboof"sv, ostringEmpty()));
        CPPUNIT_ASSERT(o3tl::ends_with("raboof"sv, ostringOof()));
        CPPUNIT_ASSERT(!o3tl::ends_with("raboof"sv, ostringDoof()));
        CPPUNIT_ASSERT(!o3tl::ends_with("raboof"sv, ostringSraboof()));
        CPPUNIT_ASSERT(o3tl::ends_with(u""sv, u""sv));
        CPPUNIT_ASSERT(!o3tl::ends_with(u""sv, u"oof"sv));
        CPPUNIT_ASSERT(o3tl::ends_with(u"raboof"sv, u""sv));
        CPPUNIT_ASSERT(o3tl::ends_with(u"raboof"sv, u"oof"sv));
        CPPUNIT_ASSERT(!o3tl::ends_with(u"raboof"sv, u"doof"sv));
        CPPUNIT_ASSERT(!o3tl::ends_with(u"raboof"sv, u"sraboof"sv));
        CPPUNIT_ASSERT(!o3tl::ends_with(u""sv, u'f'));
        CPPUNIT_ASSERT(o3tl::ends_with(u"raboof"sv, u'f'));
        CPPUNIT_ASSERT(!o3tl::ends_with(u"raboof"sv, u'g'));
        CPPUNIT_ASSERT(o3tl::ends_with(u""sv, u""));
        CPPUNIT_ASSERT(!o3tl::ends_with(u""sv, u"oof"));
        CPPUNIT_ASSERT(o3tl::ends_with(u"raboof"sv, u""));
        CPPUNIT_ASSERT(o3tl::ends_with(u"raboof"sv, u"oof"));
        CPPUNIT_ASSERT(!o3tl::ends_with(u"raboof"sv, u"doof"));
        CPPUNIT_ASSERT(!o3tl::ends_with(u"raboof"sv, u"sraboof"));
        CPPUNIT_ASSERT(o3tl::ends_with(u""sv, oustringEmpty()));
        CPPUNIT_ASSERT(!o3tl::ends_with(u""sv, oustringOof()));
        CPPUNIT_ASSERT(o3tl::ends_with(u"raboof"sv, oustringEmpty()));
        CPPUNIT_ASSERT(o3tl::ends_with(u"raboof"sv, oustringOof()));
        CPPUNIT_ASSERT(!o3tl::ends_with(u"raboof"sv, oustringDoof()));
        CPPUNIT_ASSERT(!o3tl::ends_with(u"raboof"sv, oustringSraboof()));
    }

    void testEndsWithRest()
    {
        using namespace std::string_view_literals;
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with(""sv, ""sv, &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with(""sv, "oof"sv, &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with("raboof"sv, ""sv, &rest));
            CPPUNIT_ASSERT_EQUAL("raboof"sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with("raboof"sv, "oof"sv, &rest));
            CPPUNIT_ASSERT_EQUAL("rab"sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with("raboof"sv, "doof"sv, &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with("raboof"sv, "sraboof"sv, &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with(""sv, 'f', &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with("raboof"sv, 'f', &rest));
            CPPUNIT_ASSERT_EQUAL("raboo"sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with("raboof"sv, 'g', &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with(""sv, "", &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with(""sv, "oof", &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with("raboof"sv, "", &rest));
            CPPUNIT_ASSERT_EQUAL("raboof"sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with("raboof"sv, "oof", &rest));
            CPPUNIT_ASSERT_EQUAL("rab"sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with("raboof"sv, "doof", &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with("raboof"sv, "sraboof", &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with(""sv, ostringEmpty(), &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with(""sv, ostringOof(), &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with("raboof"sv, ostringEmpty(), &rest));
            CPPUNIT_ASSERT_EQUAL("raboof"sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with("raboof"sv, ostringOof(), &rest));
            CPPUNIT_ASSERT_EQUAL("rab"sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with("raboof"sv, ostringDoof(), &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with("raboof"sv, ostringSraboof(), &rest));
            CPPUNIT_ASSERT_EQUAL(""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with(u""sv, u""sv, &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with(u""sv, u"oof"sv, &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with(u"raboof"sv, u""sv, &rest));
            CPPUNIT_ASSERT_EQUAL(u"raboof"sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with(u"raboof"sv, u"oof"sv, &rest));
            CPPUNIT_ASSERT_EQUAL(u"rab"sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with(u"raboof"sv, u"doof"sv, &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with(u"raboof"sv, u"sraboof"sv, &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with(u""sv, u'f', &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with(u"raboof"sv, u'f', &rest));
            CPPUNIT_ASSERT_EQUAL(u"raboo"sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with(u"raboof"sv, u'g', &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with(u""sv, u"", &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with(u""sv, u"oof", &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with(u"raboof"sv, u"", &rest));
            CPPUNIT_ASSERT_EQUAL(u"raboof"sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with(u"raboof"sv, u"oof", &rest));
            CPPUNIT_ASSERT_EQUAL(u"rab"sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with(u"raboof"sv, u"doof", &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with(u"raboof"sv, u"sraboof", &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with(u""sv, oustringEmpty(), &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with(u""sv, oustringOof(), &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with(u"raboof"sv, oustringEmpty(), &rest));
            CPPUNIT_ASSERT_EQUAL(u"raboof"sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(o3tl::ends_with(u"raboof"sv, oustringOof(), &rest));
            CPPUNIT_ASSERT_EQUAL(u"rab"sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with(u"raboof"sv, oustringDoof(), &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
        {
            std::u16string_view rest;
            CPPUNIT_ASSERT(!o3tl::ends_with(u"raboof"sv, oustringSraboof(), &rest));
            CPPUNIT_ASSERT_EQUAL(u""sv, rest);
        }
    }
};

CPPUNIT_TEST_SUITE_REGISTRATION(Test);
}
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
