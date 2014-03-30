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

class WpftDrawFilterTest : public writerperfect::test::WpftImportTestBase
{
public:
    WpftDrawFilterTest();

    void test();

    CPPUNIT_TEST_SUITE(WpftDrawFilterTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

WpftDrawFilterTest::WpftDrawFilterTest()
    : writerperfect::test::WpftImportTestBase("private:factory/sdraw")
{
}

void WpftDrawFilterTest::test()
{
    doTest("com.sun.star.comp.Draw.CDRImportFilter", "/writerperfect/qa/unit/data/libcdr/");
    doTest("com.sun.star.comp.Draw.CMXImportFilter", "/writerperfect/qa/unit/data/libcdr-cmx/");
    doTest("com.sun.star.comp.Draw.FreehandImportFilter", "/writerperfect/qa/unit/data/libfreehand/");
    doTest("com.sun.star.comp.Draw.MSPUBImportFilter", "/writerperfect/qa/unit/data/libmspub/");
    doTest("com.sun.star.comp.Draw.VisioImportFilter", "/writerperfect/qa/unit/data/libvisio/");
    doTest("com.sun.star.comp.Draw.WPGImportFilter", "/writerperfect/qa/unit/data/libwpg/");
}

CPPUNIT_TEST_SUITE_REGISTRATION(WpftDrawFilterTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
