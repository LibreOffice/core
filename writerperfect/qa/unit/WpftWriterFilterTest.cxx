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
    : writerperfect::test::WpftFilterTestBase("private:factory/swriter")
{
}

void WpftWriterFilterTest::test()
{
    const writerperfect::test::WpftOptionalMap_t aEBookOptional{
        { "FictionBook2.fb2.zip", REQUIRE_EBOOK_VERSION(0, 1, 1) },
    };
    const writerperfect::test::WpftOptionalMap_t aEtonyekOptional{
        { "Pages_4.pages", REQUIRE_ETONYEK_VERSION(0, 1, 2) },
        { "Pages_5.pages", REQUIRE_ETONYEK_VERSION(0, 1, 8) },
    };
    const writerperfect::test::WpftOptionalMap_t aMWAWOptional{
        { "JazzLotus.hqx", REQUIRE_MWAW_VERSION(0, 3, 17) },
        { "MaxWrite_1.hqx", REQUIRE_MWAW_VERSION(0, 3, 8) },
        { "MouseWrite_1.hqx", REQUIRE_MWAW_VERSION(0, 3, 8) },
        { "RagTime_2.1.hqx", REQUIRE_MWAW_VERSION(0, 3, 2) },
        { "RagTime_3.2.hqx", REQUIRE_MWAW_VERSION(0, 3, 2) },
        { "RagTime_5.5.rag", REQUIRE_MWAW_VERSION(0, 3, 6) },
        { "ScriptWriter", REQUIRE_MWAW_VERSION(0, 3, 21) },
        { "StudentWritingCenter", REQUIRE_MWAW_VERSION(0, 3, 20) },
        { "WordMaker", REQUIRE_MWAW_VERSION(0, 3, 20) },
    };
    const writerperfect::test::WpftOptionalMap_t aStarOfficeOptional{
        { "Writer_3.1.sdw", REQUIRE_STAROFFICE_VERSION(0, 0, 2) },
    };
    const writerperfect::test::WpftOptionalMap_t aWpsOptional{
        { "PocketWord.psw", REQUIRE_WPS_VERSION(0, 4, 12) },
        { "Word_5.0_DOS.doc", REQUIRE_WPS_VERSION(0, 4, 3) },
        { "Write_3.1.wri", REQUIRE_WPS_VERSION(0, 4, 2) },
    };

    doTest("com.sun.star.comp.Writer.AbiWordImportFilter",
           u"/writerperfect/qa/unit/data/writer/libabw/");
    doTest("org.libreoffice.comp.Writer.EBookImportFilter",
           u"/writerperfect/qa/unit/data/writer/libe-book/", aEBookOptional);
    doTest("com.sun.star.comp.Writer.MSWorksImportFilter",
           u"/writerperfect/qa/unit/data/writer/libwps/", aWpsOptional);
    doTest("com.sun.star.comp.Writer.MWAWImportFilter",
           u"/writerperfect/qa/unit/data/writer/libmwaw/", aMWAWOptional);
    doTest("org.libreoffice.comp.Writer.PagesImportFilter",
           u"/writerperfect/qa/unit/data/writer/libetonyek/", aEtonyekOptional);
    doTest("org.libreoffice.comp.Writer.StarOfficeWriterImportFilter",
           u"/writerperfect/qa/unit/data/writer/libstaroffice/", aStarOfficeOptional);
    doTest("com.sun.star.comp.Writer.WordPerfectImportFilter",
           u"/writerperfect/qa/unit/data/writer/libwpd/");
}

CPPUNIT_TEST_SUITE_REGISTRATION(WpftWriterFilterTest);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
