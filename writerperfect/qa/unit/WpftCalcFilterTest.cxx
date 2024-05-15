/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
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
        { "Numbers_2.numbers", REQUIRE_ETONYEK_VERSION(0, 1, 2) },
    };
    const writerperfect::test::WpftOptionalMap_t aMWAWOptional{
        { "JazzLotus.hqx", REQUIRE_MWAW_VERSION(0, 3, 17) },
        { "Multiplan_1.hqx", REQUIRE_MWAW_VERSION(0, 3, 14) },
    };
    const writerperfect::test::WpftOptionalMap_t aWpsOptional{
        { "Lotus123_3.123", REQUIRE_WPS_VERSION(0, 4, 5) },
        { "Lotus123_98.123", REQUIRE_WPS_VERSION(0, 4, 6) },
        { "Multiplan_3", REQUIRE_WPS_VERSION(0, 4, 9) },
        { "QuattroPro.wb1", REQUIRE_WPS_VERSION(0, 4, 8) },
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
