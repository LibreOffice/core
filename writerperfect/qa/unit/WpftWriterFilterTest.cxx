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

class WpftWriterFilterTest : public writerperfect::test::WpftImportTestBase
{
public:
    WpftWriterFilterTest();

    void test();

    CPPUNIT_TEST_SUITE(WpftWriterFilterTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

WpftWriterFilterTest::WpftWriterFilterTest()
    : writerperfect::test::WpftImportTestBase("private:factory/swriter")
{
}

void WpftWriterFilterTest::test()
{
    doTest("com.sun.star.comp.Writer.AbiWordImportFilter", "/writerperfect/qa/unit/data/writer/libabw/");
    doTest("org.libreoffice.comp.Writer.EBookImportFilter", "/writerperfect/qa/unit/data/writer/libe-book/");
    doTest("com.sun.star.comp.Writer.MSWorksImportFilter", "/writerperfect/qa/unit/data/writer/libwps/");
    doTest("com.sun.star.comp.Writer.MWAWImportFilter", "/writerperfect/qa/unit/data/writer/libmwaw/");
    doTest("org.libreoffice.comp.Writer.PagesImportFilter", "/writerperfect/qa/unit/data/writer/libetonyek/");
    doTest("com.sun.star.comp.Writer.WordPerfectImportFilter", "/writerperfect/qa/unit/data/writer/libwpd/");
}

CPPUNIT_TEST_SUITE_REGISTRATION(WpftWriterFilterTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
