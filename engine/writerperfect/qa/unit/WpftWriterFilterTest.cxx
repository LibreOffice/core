/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the Collabora Office project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <config_vclplug.h>

#include "WpftFilterTestBase.hxx"

namespace
{
#if defined _WIN32 && USE_HEADLESS_CODE
constexpr bool bComApartmentNeededButMissing = true;
#else
constexpr bool bComApartmentNeededButMissing = false;
#endif

class WpftWriterFilterTest : public writerperfect::test::WpftFilterTestBase
{
public:
    WpftWriterFilterTest();

    void test();

    CPPUNIT_TEST_SUITE(WpftWriterFilterTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

WpftWriterFilterTest::WpftWriterFilterTest()
    : writerperfect::test::WpftFilterTestBase(u"private:factory/swriter"_ustr)
{
}

void WpftWriterFilterTest::test()
{
    const writerperfect::test::WpftOptionalMap_t aEBookOptional{
        { u"FictionBook2.fb2.zip"_ustr, REQUIRE_EBOOK_VERSION(0, 1, 1) },
    };
    const writerperfect::test::WpftOptionalMap_t aEtonyekOptional{
        { u"Pages_4.pages"_ustr, REQUIRE_ETONYEK_VERSION(0, 1, 2) },
        { u"Pages_5.pages"_ustr, REQUIRE_ETONYEK_VERSION(0, 1, 8) },
    };
    const writerperfect::test::WpftOptionalMap_t aMWAWOptional{
        { u"JazzLotus.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 17) },
        { u"MaxWrite_1.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 8) },
        { u"MouseWrite_1.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 8) },
        { u"RagTime_2.1.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 2) },
        { u"RagTime_3.2.hqx"_ustr, REQUIRE_MWAW_VERSION(0, 3, 2) },
        { u"RagTime_5.5.rag"_ustr, REQUIRE_MWAW_VERSION(0, 3, 6) },
        { u"ScriptWriter"_ustr, REQUIRE_MWAW_VERSION(0, 3, 21) },
        { u"StudentWritingCenter"_ustr, REQUIRE_MWAW_VERSION(0, 3, 20) },
        { u"WordMaker"_ustr, REQUIRE_MWAW_VERSION(0, 3, 20) },
    };
    const writerperfect::test::WpftOptionalMap_t aStarOfficeOptional{
        { u"Writer_3.1.sdw"_ustr, REQUIRE_STAROFFICE_VERSION(0, 0, 2) },
    };
    const writerperfect::test::WpftOptionalMap_t aWpsOptional{
        { u"PocketWord.psw"_ustr, REQUIRE_WPS_VERSION(0, 4, 12) },
        { u"Word_5.0_DOS.doc"_ustr, REQUIRE_WPS_VERSION(0, 4, 3) },
        // The embedded OLE object needs a COM apartment, absent in the headless test code
        { u"Write_3.1.wri"_ustr, REQUIRE_WPS_VERSION(0, 4, 2) && !bComApartmentNeededButMissing },
    };

    doTest(u"com.sun.star.comp.Writer.AbiWordImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/writer/libabw/");
    doTest(u"org.libreoffice.comp.Writer.EBookImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/writer/libe-book/", aEBookOptional);
    doTest(u"com.sun.star.comp.Writer.MSWorksImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/writer/libwps/", aWpsOptional);
    doTest(u"com.sun.star.comp.Writer.MWAWImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/writer/libmwaw/", aMWAWOptional);
    doTest(u"org.libreoffice.comp.Writer.PagesImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/writer/libetonyek/", aEtonyekOptional);
    doTest(u"org.libreoffice.comp.Writer.StarOfficeWriterImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/writer/libstaroffice/", aStarOfficeOptional);
    doTest(u"com.sun.star.comp.Writer.WordPerfectImportFilter"_ustr,
           u"/writerperfect/qa/unit/data/writer/libwpd/");
}

CPPUNIT_TEST_SUITE_REGISTRATION(WpftWriterFilterTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
