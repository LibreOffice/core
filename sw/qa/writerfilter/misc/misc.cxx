/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <tuple>
#include <vector>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <sal/types.h>

#include <rtl/ustring.hxx>
#include <dmapper/ConversionHelper.hxx>
#include <dmapper/DomainMapperFactory.hxx>

namespace
{
class WriterfilterMiscTest : public ::CppUnit::TestFixture
{
public:
    void testTwipConversions();
    void testFieldParameters();

    CPPUNIT_TEST_SUITE(WriterfilterMiscTest);
    CPPUNIT_TEST(testTwipConversions);
    CPPUNIT_TEST(testFieldParameters);
    CPPUNIT_TEST_SUITE_END();
};

void WriterfilterMiscTest::testTwipConversions()
{
    using writerfilter::dmapper::ConversionHelper::convertTwipToMM100;
    using writerfilter::dmapper::ConversionHelper::convertTwipToMM100Unsigned;

    CPPUNIT_ASSERT_EQUAL(sal_Int32(-2), convertTwipToMM100(-1));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-17639), convertTwipToMM100(-10000));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(-70556), convertTwipToMM100(-40000));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(2), convertTwipToMM100(1));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(17639), convertTwipToMM100(10000));
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), convertTwipToMM100(40000));

    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), convertTwipToMM100Unsigned(-1));
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), convertTwipToMM100Unsigned(-10000));
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), convertTwipToMM100Unsigned(-40000));
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(2), convertTwipToMM100Unsigned(1));
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(17639), convertTwipToMM100Unsigned(10000));
    CPPUNIT_ASSERT_EQUAL(sal_uInt32(0), convertTwipToMM100Unsigned(40000));
}

void WriterfilterMiscTest::testFieldParameters()
{
    using writerfilter::dmapper::splitFieldCommand;
    std::tuple<OUString, std::vector<OUString>, std::vector<OUString>> result;

    result = splitFieldCommand(u"PAGEREF last_page");
    CPPUNIT_ASSERT_EQUAL(u"PAGEREF"_ustr, std::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), std::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(u"last_page"_ustr, std::get<1>(result)[0]);
    CPPUNIT_ASSERT(std::get<2>(result).empty());

    result = splitFieldCommand(u" PAGEREF last_page ");
    CPPUNIT_ASSERT_EQUAL(u"PAGEREF"_ustr, std::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), std::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(u"last_page"_ustr, std::get<1>(result)[0]);

    result = splitFieldCommand(u"pageref last_page");
    CPPUNIT_ASSERT(std::get<2>(result).empty());
    CPPUNIT_ASSERT_EQUAL(u"PAGEREF"_ustr, std::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), std::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(u"last_page"_ustr, std::get<1>(result)[0]);
    CPPUNIT_ASSERT(std::get<2>(result).empty());

    result = splitFieldCommand(u"pageref \"last_page\"");
    CPPUNIT_ASSERT_EQUAL(u"PAGEREF"_ustr, std::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), std::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(u"last_page"_ustr, std::get<1>(result)[0]);
    CPPUNIT_ASSERT(std::get<2>(result).empty());

    result = splitFieldCommand(u"\"PAGEREF\" \"last_page\" \"\" ");
    CPPUNIT_ASSERT_EQUAL(u"PAGEREF"_ustr, std::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(2), std::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(u"last_page"_ustr, std::get<1>(result)[0]);
    CPPUNIT_ASSERT_EQUAL(OUString(), std::get<1>(result)[1]);
    CPPUNIT_ASSERT(std::get<2>(result).empty());

    result = splitFieldCommand(u"\"PAGEREF\"\"last_page\"  ");
    CPPUNIT_ASSERT_EQUAL(u"PAGEREF"_ustr, std::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), std::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(u"last_page"_ustr, std::get<1>(result)[0]);
    CPPUNIT_ASSERT(std::get<2>(result).empty());

    result = splitFieldCommand(u"PAGEREF\"last_page\"  ");
    CPPUNIT_ASSERT_EQUAL(u"PAGEREF"_ustr, std::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), std::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(u"last_page"_ustr, std::get<1>(result)[0]);
    CPPUNIT_ASSERT(std::get<2>(result).empty());

    result = splitFieldCommand(u"\"PAGEREF\"last_page \"\"");
    CPPUNIT_ASSERT_EQUAL(u"PAGEREF"_ustr, std::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(2), std::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(u"last_page"_ustr, std::get<1>(result)[0]);
    CPPUNIT_ASSERT_EQUAL(OUString(), std::get<1>(result)[1]);
    CPPUNIT_ASSERT(std::get<2>(result).empty());

    result = splitFieldCommand(u"\"PAGEREF\"last_page \"\"");
    CPPUNIT_ASSERT_EQUAL(u"PAGEREF"_ustr, std::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(2), std::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(u"last_page"_ustr, std::get<1>(result)[0]);
    CPPUNIT_ASSERT_EQUAL(OUString(), std::get<1>(result)[1]);
    CPPUNIT_ASSERT(std::get<2>(result).empty());

    result = splitFieldCommand(u"pageref \"last\\\\pa\\\"ge\"");
    CPPUNIT_ASSERT_EQUAL(u"PAGEREF"_ustr, std::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), std::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(u"last\\pa\"ge"_ustr, std::get<1>(result)[0]);
    CPPUNIT_ASSERT(std::get<2>(result).empty());

    result = splitFieldCommand(u"PAGEREF\"last_page\"\\*");
    CPPUNIT_ASSERT_EQUAL(u"PAGEREF"_ustr, std::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), std::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(u"last_page"_ustr, std::get<1>(result)[0]);
    CPPUNIT_ASSERT_EQUAL(size_t(1), std::get<2>(result).size());
    CPPUNIT_ASSERT_EQUAL(u"\\*"_ustr, std::get<2>(result)[0]);

    result = splitFieldCommand(u"PAGEREF  last_page   \\b   foobar ");
    CPPUNIT_ASSERT_EQUAL(u"PAGEREF"_ustr, std::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), std::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(u"last_page"_ustr, std::get<1>(result)[0]);
    CPPUNIT_ASSERT_EQUAL(size_t(2), std::get<2>(result).size());
    CPPUNIT_ASSERT_EQUAL(u"\\B"_ustr, std::get<2>(result)[0]);
    CPPUNIT_ASSERT_EQUAL(u"foobar"_ustr, std::get<2>(result)[1]);

    result = splitFieldCommand(u"PAGEREF\\bfoobar\\A\"\"");
    CPPUNIT_ASSERT_EQUAL(u"PAGEREF"_ustr, std::get<0>(result));
    CPPUNIT_ASSERT(std::get<1>(result).empty());
    CPPUNIT_ASSERT_EQUAL(size_t(4), std::get<2>(result).size());
    CPPUNIT_ASSERT_EQUAL(u"\\B"_ustr, std::get<2>(result)[0]);
    CPPUNIT_ASSERT_EQUAL(u"foobar"_ustr, std::get<2>(result)[1]);
    CPPUNIT_ASSERT_EQUAL(u"\\A"_ustr, std::get<2>(result)[2]);
    CPPUNIT_ASSERT_EQUAL(OUString(), std::get<2>(result)[3]);

    for (auto prefix : { "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/", ":",
                         ";", "<", ">", "?", "@", "[", "]", "^", "_", "`", "{", "|", "}", "~" })
    {
        OUString test(OUString::createFromAscii(prefix) + "PAGE");
        result = splitFieldCommand(Concat2View(test + " "));
        CPPUNIT_ASSERT_EQUAL(test, std::get<0>(result));
    }
    result = splitFieldCommand(u"\\PAGE ");
    CPPUNIT_ASSERT_EQUAL(u"PAGE"_ustr, std::get<0>(result));
    result = splitFieldCommand(u"\\ PAGE ");
    CPPUNIT_ASSERT_EQUAL(u"\\ "_ustr, std::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(u"PAGE"_ustr, std::get<1>(result)[0]);
    result = splitFieldCommand(u"\\\\PAGE ");
    CPPUNIT_ASSERT_EQUAL(u"\\PAGE"_ustr, std::get<0>(result));
    result = splitFieldCommand(u"\"PAGE\" ");
    CPPUNIT_ASSERT_EQUAL(u"PAGE"_ustr, std::get<0>(result));
    result = splitFieldCommand(u"\"PAGE ");
    CPPUNIT_ASSERT_EQUAL(u"PAGE "_ustr, std::get<0>(result));
}

CPPUNIT_TEST_SUITE_REGISTRATION(WriterfilterMiscTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
