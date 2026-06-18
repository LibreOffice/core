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
    : writerperfect::test::WpftFilterTestBase(u"private:factory/simpress"_ustr)
{
}

void WpftImpressFilterTest::test()
{
    const writerperfect::test::WpftOptionalMap_t aEtonyekOptional{
        { u"Keynote_1.key"_ustr, REQUIRE_ETONYEK_VERSION(0, 1, 8) },
        { u"Keynote_2.key"_ustr, REQUIRE_ETONYEK_VERSION(0, 1, 1) },
        { u"Keynote_3.key"_ustr, REQUIRE_ETONYEK_VERSION(0, 1, 1) },
        { u"Keynote_6.key"_ustr, REQUIRE_ETONYEK_VERSION(0, 1, 4) },
    };
    const writerperfect::test::WpftOptionalMap_t aMWAWOptional{
        { u"ClarisWorks_6.0.cwk"_ustr, REQUIRE_MWAW_VERSION(0, 3, 3) },
        { u"PowerPoint_Mac_1"_ustr, REQUIRE_MWAW_VERSION(0, 3, 9) },
        { u"PowerPoint_Mac_2"_ustr, REQUIRE_MWAW_VERSION(0, 3, 9) },
        { u"PowerPoint_Mac_3"_ustr, REQUIRE_MWAW_VERSION(0, 3, 9) },
        { u"PowerPoint_Mac_4.ppt"_ustr, REQUIRE_MWAW_VERSION(0, 3, 10) },
        { u"PowerPoint_2.ppt"_ustr, REQUIRE_MWAW_VERSION(0, 3, 10) },
        { u"PowerPoint_3.ppt"_ustr, REQUIRE_MWAW_VERSION(0, 3, 9) },
        { u"PowerPoint_4.ppt"_ustr, REQUIRE_MWAW_VERSION(0, 3, 10) },
        { u"PowerPoint_7.ppt"_ustr, REQUIRE_MWAW_VERSION(0, 3, 11) },
    };

    doTest(u"org.libreoffice.comp.Impress.KeynoteImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/impress/libetonyek/", aEtonyekOptional);
    doTest(u"com.sun.star.comp.Impress.MWAWPresentationImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/impress/libmwaw/", aMWAWOptional);
}

CPPUNIT_TEST_SUITE_REGISTRATION(WpftImpressFilterTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
