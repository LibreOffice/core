/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <limits>
#include <vector>

#include <boost/tuple/tuple.hpp>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#include <sal/types.h>

#include <rtl/ustring.hxx>
#include <dmapper/ConversionHelper.hxx>
#include <dmapper/DomainMapper_Impl.hxx>

namespace {

class WriterfilterMiscTest
    : public ::CppUnit::TestFixture
{
public:
    virtual void setUp() override;
    virtual void tearDown() override;

    void testTwipConversions();
    void testFieldParameters();

    CPPUNIT_TEST_SUITE(WriterfilterMiscTest);
    CPPUNIT_TEST(testTwipConversions);
    CPPUNIT_TEST(testFieldParameters);
    CPPUNIT_TEST_SUITE_END();
};

void WriterfilterMiscTest::setUp()
{
}

void WriterfilterMiscTest::tearDown()
{
}

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
    using writerfilter::dmapper::lcl_SplitFieldCommand;
    boost::tuple<OUString, std::vector<OUString>, std::vector<OUString> > result;

    result = lcl_SplitFieldCommand("PAGEREF last_page");
    CPPUNIT_ASSERT_EQUAL(OUString("PAGEREF"), boost::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), boost::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(OUString("last_page"), boost::get<1>(result)[0]);
    CPPUNIT_ASSERT(boost::get<2>(result).empty());

    result = lcl_SplitFieldCommand(" PAGEREF last_page ");
    CPPUNIT_ASSERT_EQUAL(OUString("PAGEREF"), boost::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), boost::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(OUString("last_page"), boost::get<1>(result)[0]);

    result = lcl_SplitFieldCommand("pageref last_page");
    CPPUNIT_ASSERT(boost::get<2>(result).empty());
    CPPUNIT_ASSERT_EQUAL(OUString("PAGEREF"), boost::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), boost::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(OUString("last_page"), boost::get<1>(result)[0]);
    CPPUNIT_ASSERT(boost::get<2>(result).empty());

    result = lcl_SplitFieldCommand("pageref \"last_page\"");
    CPPUNIT_ASSERT_EQUAL(OUString("PAGEREF"), boost::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), boost::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(OUString("last_page"), boost::get<1>(result)[0]);
    CPPUNIT_ASSERT(boost::get<2>(result).empty());

    result = lcl_SplitFieldCommand("\"PAGEREF\" \"last_page\" \"\" ");
    CPPUNIT_ASSERT_EQUAL(OUString("PAGEREF"), boost::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(2), boost::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(OUString("last_page"), boost::get<1>(result)[0]);
    CPPUNIT_ASSERT_EQUAL(OUString(), boost::get<1>(result)[1]);
    CPPUNIT_ASSERT(boost::get<2>(result).empty());

    result = lcl_SplitFieldCommand("\"PAGEREF\"\"last_page\"  ");
    CPPUNIT_ASSERT_EQUAL(OUString("PAGEREF"), boost::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), boost::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(OUString("last_page"), boost::get<1>(result)[0]);
    CPPUNIT_ASSERT(boost::get<2>(result).empty());

    result = lcl_SplitFieldCommand("PAGEREF\"last_page\"  ");
    CPPUNIT_ASSERT_EQUAL(OUString("PAGEREF"), boost::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), boost::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(OUString("last_page"), boost::get<1>(result)[0]);
    CPPUNIT_ASSERT(boost::get<2>(result).empty());

    result = lcl_SplitFieldCommand("\"PAGEREF\"last_page \"\"");
    CPPUNIT_ASSERT_EQUAL(OUString("PAGEREF"), boost::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(2), boost::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(OUString("last_page"), boost::get<1>(result)[0]);
    CPPUNIT_ASSERT_EQUAL(OUString(), boost::get<1>(result)[1]);
    CPPUNIT_ASSERT(boost::get<2>(result).empty());

    result = lcl_SplitFieldCommand("\"PAGEREF\"last_page \"\"");
    CPPUNIT_ASSERT_EQUAL(OUString("PAGEREF"), boost::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(2), boost::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(OUString("last_page"), boost::get<1>(result)[0]);
    CPPUNIT_ASSERT_EQUAL(OUString(), boost::get<1>(result)[1]);
    CPPUNIT_ASSERT(boost::get<2>(result).empty());

    result = lcl_SplitFieldCommand("pageref \"last\\\\pa\\\"ge\"");
    CPPUNIT_ASSERT_EQUAL(OUString("PAGEREF"), boost::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), boost::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(OUString("last\\pa\"ge"), boost::get<1>(result)[0]);
    CPPUNIT_ASSERT(boost::get<2>(result).empty());

    result = lcl_SplitFieldCommand("PAGEREF\"last_page\"\\*");
    CPPUNIT_ASSERT_EQUAL(OUString("PAGEREF"), boost::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), boost::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(OUString("last_page"), boost::get<1>(result)[0]);
    CPPUNIT_ASSERT_EQUAL(size_t(1), boost::get<2>(result).size());
    CPPUNIT_ASSERT_EQUAL(OUString("\\*"), boost::get<2>(result)[0]);

    result = lcl_SplitFieldCommand("PAGEREF  last_page   \\b   foobar ");
    CPPUNIT_ASSERT_EQUAL(OUString("PAGEREF"), boost::get<0>(result));
    CPPUNIT_ASSERT_EQUAL(size_t(1), boost::get<1>(result).size());
    CPPUNIT_ASSERT_EQUAL(OUString("last_page"), boost::get<1>(result)[0]);
    CPPUNIT_ASSERT_EQUAL(size_t(2), boost::get<2>(result).size());
    CPPUNIT_ASSERT_EQUAL(OUString("\\B"), boost::get<2>(result)[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("foobar"), boost::get<2>(result)[1]);

    result = lcl_SplitFieldCommand("PAGEREF\\bfoobar\\A\"\"");
    CPPUNIT_ASSERT_EQUAL(OUString("PAGEREF"), boost::get<0>(result));
    CPPUNIT_ASSERT(boost::get<1>(result).empty());
    CPPUNIT_ASSERT_EQUAL(size_t(4), boost::get<2>(result).size());
    CPPUNIT_ASSERT_EQUAL(OUString("\\B"), boost::get<2>(result)[0]);
    CPPUNIT_ASSERT_EQUAL(OUString("foobar"), boost::get<2>(result)[1]);
    CPPUNIT_ASSERT_EQUAL(OUString("\\A"), boost::get<2>(result)[2]);
    CPPUNIT_ASSERT_EQUAL(OUString(), boost::get<2>(result)[3]);
}

CPPUNIT_TEST_SUITE_REGISTRATION(WriterfilterMiscTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
