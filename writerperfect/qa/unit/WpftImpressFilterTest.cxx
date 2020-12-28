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
class WpftImpressFilterTest : public writerperfect::test::WpftFilterTestBase
{
public:
    WpftImpressFilterTest();

    void test();

    CPPUNIT_TEST_SUITE(WpftImpressFilterTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

WpftImpressFilterTest::WpftImpressFilterTest()
    : writerperfect::test::WpftFilterTestBase("private:factory/simpress")
{
}

void WpftImpressFilterTest::test()
{
    const writerperfect::test::WpftOptionalMap_t aEtonyekOptional{
        { "Keynote_1.key", REQUIRE_ETONYEK_VERSION(0, 1, 8) },
        { "Keynote_2.key", REQUIRE_ETONYEK_VERSION(0, 1, 1) },
        { "Keynote_3.key", REQUIRE_ETONYEK_VERSION(0, 1, 1) },
        { "Keynote_6.key", REQUIRE_ETONYEK_VERSION(0, 1, 4) },
    };
    const writerperfect::test::WpftOptionalMap_t aMWAWOptional{
        { "ClarisWorks_6.0.cwk", REQUIRE_MWAW_VERSION(0, 3, 3) },
        { "PowerPoint_Mac_1", REQUIRE_MWAW_VERSION(0, 3, 9) },
        { "PowerPoint_Mac_2", REQUIRE_MWAW_VERSION(0, 3, 9) },
        { "PowerPoint_Mac_3", REQUIRE_MWAW_VERSION(0, 3, 9) },
        { "PowerPoint_Mac_4.ppt", REQUIRE_MWAW_VERSION(0, 3, 10) },
        { "PowerPoint_2.ppt", REQUIRE_MWAW_VERSION(0, 3, 10) },
        { "PowerPoint_3.ppt", REQUIRE_MWAW_VERSION(0, 3, 9) },
        { "PowerPoint_4.ppt", REQUIRE_MWAW_VERSION(0, 3, 10) },
        { "PowerPoint_7.ppt", REQUIRE_MWAW_VERSION(0, 3, 11) },
    };

    doTest("org.libreoffice.comp.Impress.KeynoteImportFilter",
           u"/writerperfect/qa/unit/data/impress/libetonyek/", aEtonyekOptional);
    doTest("com.sun.star.comp.Impress.MWAWPresentationImportFilter",
           u"/writerperfect/qa/unit/data/impress/libmwaw/", aMWAWOptional);
}

CPPUNIT_TEST_SUITE_REGISTRATION(WpftImpressFilterTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
