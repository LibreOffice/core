/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "WpftImportTestBase.hxx"

namespace
{

class WpftCalcFilterTest : public writerperfect::test::WpftImportTestBase
{
public:
    WpftCalcFilterTest();

    void test();

    CPPUNIT_TEST_SUITE(WpftCalcFilterTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

WpftCalcFilterTest::WpftCalcFilterTest()
    : writerperfect::test::WpftImportTestBase("private:factory/scalc")
{
}

void WpftCalcFilterTest::test()
{
    doTest("com.sun.star.comp.Calc.MWAWCalcImportFilter", "/writerperfect/qa/unit/data/calc/libmwaw/");
    doTest("com.sun.star.comp.Calc.MSWorksCalcImportFilter", "/writerperfect/qa/unit/data/calc/libwps/");
    doTest("org.libreoffice.comp.Calc.NumbersImportFilter", "/writerperfect/qa/unit/data/calc/libetonyek/");
}

CPPUNIT_TEST_SUITE_REGISTRATION(WpftCalcFilterTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
