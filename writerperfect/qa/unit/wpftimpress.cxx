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

class WpftImpressFilterTest : public writerperfect::test::WpftImportTestBase
{
public:
    WpftImpressFilterTest();

    void test();

    CPPUNIT_TEST_SUITE(WpftImpressFilterTest);
    CPPUNIT_TEST(test);
    CPPUNIT_TEST_SUITE_END();
};

WpftImpressFilterTest::WpftImpressFilterTest()
    : writerperfect::test::WpftImportTestBase("private:factory/simpress")
{
}

void WpftImpressFilterTest::test()
{
    doTest("org.libreoffice.comp.Impress.KeynoteImportFilter", "/writerperfect/qa/unit/data/libetonyek/");
}

CPPUNIT_TEST_SUITE_REGISTRATION(WpftImpressFilterTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
