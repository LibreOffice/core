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
    const writerperfect::test::WpftOptionalMap_t aEtonyekOptional
    {
        {"v2.zip", REQUIRE_ETONYEK_VERSION(0, 1, 1)},
        {"v3.zip", REQUIRE_ETONYEK_VERSION(0, 1, 1)},
        {"v6.zip", REQUIRE_ETONYEK_VERSION(0, 1, 4)},
    };
    const writerperfect::test::WpftOptionalMap_t aMWAWOptional
    {
        {"ClarisWorks_6.0.cwk", REQUIRE_MWAW_VERSION(0, 3, 3)},
    };

    doTest("org.libreoffice.comp.Impress.KeynoteImportFilter", "/writerperfect/qa/unit/data/impress/libetonyek/", aEtonyekOptional);
    doTest("com.sun.star.comp.Impress.MWAWPresentationImportFilter", "/writerperfect/qa/unit/data/impress/libmwaw/", aMWAWOptional);
}

CPPUNIT_TEST_SUITE_REGISTRATION(WpftImpressFilterTest);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
