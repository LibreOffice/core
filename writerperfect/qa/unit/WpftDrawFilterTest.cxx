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
    const writerperfect::test::WpftOptionalMap_t aMWAWOptional
    {
        {"ClarisDraw.hqx", REQUIRE_MWAW_VERSION(0, 3, 5)},
        {"MacDraft_1.0.hqx", REQUIRE_MWAW_VERSION(0, 3, 5)},
        {"MacDraft_5.5.drw", REQUIRE_MWAW_VERSION(0, 3, 6)},
        {"MacDraw_0.hqx", REQUIRE_MWAW_VERSION(0, 3, 2)},
        {"MacDraw_1.hqx", REQUIRE_MWAW_VERSION(0, 3, 2)},
        {"MacDraw_II.hqx", REQUIRE_MWAW_VERSION(0, 3, 3)},
        {"MacDraw_Pro_1.0.hqx", REQUIRE_MWAW_VERSION(0, 3, 4)},
    };

    doTest("com.sun.star.comp.Draw.CDRImportFilter", "/writerperfect/qa/unit/data/draw/libcdr/");
    doTest("com.sun.star.comp.Draw.CMXImportFilter", "/writerperfect/qa/unit/data/draw/libcdr-cmx/");
    doTest("com.sun.star.comp.Draw.FreehandImportFilter", "/writerperfect/qa/unit/data/draw/libfreehand/");
    doTest("com.sun.star.comp.Draw.MSPUBImportFilter", "/writerperfect/qa/unit/data/draw/libmspub/");
    doTest("com.sun.star.comp.Draw.MWAWDrawImportFilter", "/writerperfect/qa/unit/data/draw/libmwaw/", aMWAWOptional);
    doTest("com.sun.star.comp.Draw.VisioImportFilter", "/writerperfect/qa/unit/data/draw/libvisio/");
    doTest("com.sun.star.comp.Draw.WPGImportFilter", "/writerperfect/qa/unit/data/draw/libwpg/");
    doTest("org.libreoffice.comp.Draw.PageMakerImportFilter", "/writerperfect/qa/unit/data/draw/libpagemaker/");
}

CPPUNIT_TEST_SUITE_REGISTRATION(WpftDrawFilterTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
