/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WpftFilterTestBase.hxx"

namespace
{
class WpftCalcFilterTest : public writerperfect::test::WpftFilterTestBase
{
public:
    WpftCalcFilterTest();

    void test();

    CPPUNIT_TEST_SUITE(WpftCalcFilterTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

WpftCalcFilterTest::WpftCalcFilterTest()
    : writerperfect::test::WpftFilterTestBase(u"private:factory/scalc"_ustr)
{
}

void WpftCalcFilterTest::test()
{
    const writerperfect::test::WpftOptionalMap_t aEtonyekOptional{
        { u"Numbers_2.numbers"_ustr, REQUIRE_ETONYEK_VERSION(0, 1, 2) },
    };
    const writerperfect::test::WpftOptionalMap_t aMWAWOptional{
        { u"JazzLotus.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 17) },
        { u"Multiplan_1.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 14) },
    };
    const writerperfect::test::WpftOptionalMap_t aWpsOptional{
        { u"Lotus123_3.123"_ustr, REQUIRE_WPS_VERSION(0, 4, 5) },
        { u"Lotus123_98.123"_ustr, REQUIRE_WPS_VERSION(0, 4, 6) },
        { u"Multiplan_3"_ustr, REQUIRE_WPS_VERSION(0, 4, 9) },
        { u"QuattroPro.wb1"_ustr, REQUIRE_WPS_VERSION(0, 4, 8) },
    };

    doTest(u"com.sun.star.comp.Calc.MWAWCalcImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/calc/libmwaw/", aMWAWOptional);
    doTest(u"com.sun.star.comp.Calc.MSWorksCalcImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/calc/libwps/", aWpsOptional);
    doTest(u"org.libreoffice.comp.Calc.NumbersImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/calc/libetonyek/", aEtonyekOptional);
    doTest(u"org.libreoffice.comp.Calc.StarOfficeCalcImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/calc/libstaroffice/");
}

CPPUNIT_TEST_SUITE_REGISTRATION(WpftCalcFilterTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
