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
    : writerperfect::test::WpftFilterTestBase(u"private:factory/sdraw"_ustr)
{
}

void WpftDrawFilterTest::test()
{
    const writerperfect::test::WpftOptionalMap_t aMWAWOptional{
        { u"Canvas_2.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 17) },
        { u"ClarisDraw.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 5) },
        { u"CorelPainter_3_win.rif"_ustr, REQUIRE_MWAW_VERSION(0, 3, 15) },
        { u"CorelPainter_10"_ustr, REQUIRE_MWAW_VERSION(0, 3, 15) },
        { u"CricketDraw_1.0.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 8) },
        { u"CricketDraw_1.1.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 8) },
        { u"DrawingTable_1.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 17) },
        { u"FreeHand_1.0.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 8) },
        { u"FreeHand_2.0.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 8) },
        { u"MacDraft_1.0.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 5) },
        { u"MacDraft_5.5.drw"_ustr, REQUIRE_MWAW_VERSION(0, 3, 6) },
        { u"MacDraw_0.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 2) },
        { u"MacDraw_1.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 2) },
        { u"MacDraw_II.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 3) },
        { u"MacDraw_Pro_1.0.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 4) },
        { u"ReadySetGo_1"_ustr, REQUIRE_MWAW_VERSION(0, 3, 21) },
        { u"Scoop_1"_ustr, REQUIRE_MWAW_VERSION(0, 3, 21) },
    };
    const writerperfect::test::WpftOptionalMap_t aStarOfficeOptional{
        { u"Draw_3.1.sda"_ustr, REQUIRE_STAROFFICE_VERSION(0, 0, 1) },
    };

    doTest(u"com.sun.star.comp.Draw.CDRImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/draw/libcdr/");
    doTest(u"com.sun.star.comp.Draw.CMXImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/draw/libcdr-cmx/");
    doTest(u"com.sun.star.comp.Draw.FreehandImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/draw/libfreehand/");
    doTest(u"com.sun.star.comp.Draw.MSPUBImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/draw/libmspub/");
    doTest(u"com.sun.star.comp.Draw.MWAWDrawImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/draw/libmwaw/", aMWAWOptional);
    doTest(u"com.sun.star.comp.Draw.VisioImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/draw/libvisio/");
    doTest(u"com.sun.star.comp.Draw.WPGImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/draw/libwpg/");
    doTest(u"org.libreoffice.comp.Draw.PageMakerImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/draw/libpagemaker/");
    doTest(u"org.libreoffice.comp.Draw.StarOfficeDrawImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/draw/libstaroffice/", aStarOfficeOptional);
    doTest(u"org.libreoffice.comp.Draw.ZMFImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/draw/libzmf/");
}

CPPUNIT_TEST_SUITE_REGISTRATION(WpftDrawFilterTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
