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
class WpftDrawFilterTest : public writerperfect::test::WpftFilterTestBase
{
public:
    WpftDrawFilterTest();

    void test();

    CPPUNIT_TEST_SUITE(WpftDrawFilterTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

WpftDrawFilterTest::WpftDrawFilterTest()
    : writerperfect::test::WpftFilterTestBase("private:factory/sdraw")
{
}

void WpftDrawFilterTest::test()
{
    const writerperfect::test::WpftOptionalMap_t aMWAWOptional{
        { "Canvas_2.hqx", REQUIRE_MWAW_VERSION(0, 3, 17) },
        { "ClarisDraw.hqx", REQUIRE_MWAW_VERSION(0, 3, 5) },
        { "CorelPainter_3_win.rif", REQUIRE_MWAW_VERSION(0, 3, 15) },
        { "CorelPainter_10", REQUIRE_MWAW_VERSION(0, 3, 15) },
        { "CricketDraw_1.0.hqx", REQUIRE_MWAW_VERSION(0, 3, 8) },
        { "CricketDraw_1.1.hqx", REQUIRE_MWAW_VERSION(0, 3, 8) },
        { "DrawingTable_1.hqx", REQUIRE_MWAW_VERSION(0, 3, 17) },
        { "FreeHand_1.0.hqx", REQUIRE_MWAW_VERSION(0, 3, 8) },
        { "FreeHand_2.0.hqx", REQUIRE_MWAW_VERSION(0, 3, 8) },
        { "MacDraft_1.0.hqx", REQUIRE_MWAW_VERSION(0, 3, 5) },
        { "MacDraft_5.5.drw", REQUIRE_MWAW_VERSION(0, 3, 6) },
        { "MacDraw_0.hqx", REQUIRE_MWAW_VERSION(0, 3, 2) },
        { "MacDraw_1.hqx", REQUIRE_MWAW_VERSION(0, 3, 2) },
        { "MacDraw_II.hqx", REQUIRE_MWAW_VERSION(0, 3, 3) },
        { "MacDraw_Pro_1.0.hqx", REQUIRE_MWAW_VERSION(0, 3, 4) },
        { "ReadySetGo_1", REQUIRE_MWAW_VERSION(0, 3, 21) },
        { "Scoop_1", REQUIRE_MWAW_VERSION(0, 3, 21) },
    };
    const writerperfect::test::WpftOptionalMap_t aStarOfficeOptional{
        { "Draw_3.1.sda", REQUIRE_STAROFFICE_VERSION(0, 0, 1) },
    };

    doTest("com.sun.star.comp.Draw.CDRImportFilter", u"/writerperfect/qa/unit/data/draw/libcdr/");
    doTest("com.sun.star.comp.Draw.CMXImportFilter",
           u"/writerperfect/qa/unit/data/draw/libcdr-cmx/");
    doTest("com.sun.star.comp.Draw.FreehandImportFilter",
           u"/writerperfect/qa/unit/data/draw/libfreehand/");
    doTest("com.sun.star.comp.Draw.MSPUBImportFilter",
           u"/writerperfect/qa/unit/data/draw/libmspub/");
    doTest("com.sun.star.comp.Draw.MWAWDrawImportFilter",
           u"/writerperfect/qa/unit/data/draw/libmwaw/", aMWAWOptional);
    doTest("com.sun.star.comp.Draw.VisioImportFilter",
           u"/writerperfect/qa/unit/data/draw/libvisio/");
    doTest("com.sun.star.comp.Draw.WPGImportFilter", u"/writerperfect/qa/unit/data/draw/libwpg/");
    doTest("org.libreoffice.comp.Draw.PageMakerImportFilter",
           u"/writerperfect/qa/unit/data/draw/libpagemaker/");
    doTest("org.libreoffice.comp.Draw.StarOfficeDrawImportFilter",
           u"/writerperfect/qa/unit/data/draw/libstaroffice/", aStarOfficeOptional);
    doTest("org.libreoffice.comp.Draw.ZMFImportFilter",
           u"/writerperfect/qa/unit/data/draw/libzmf/");
}

CPPUNIT_TEST_SUITE_REGISTRATION(WpftDrawFilterTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
