/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 */

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>

#include <osl/file.hxx>
#include <osl/process.h>

#include <vcl/graphicfilter.hxx>

using namespace ::com::sun::star;

/* Implementation of Filters test */

class VclFiltersTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    VclFiltersTest() : BootstrapFixture(true, false) {}

    virtual bool load(const OUString &,
        const OUString &rURL, const OUString &,
        unsigned int, unsigned int, unsigned int);

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    CPPUNIT_TEST_SUITE(VclFiltersTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST_SUITE_END();
};

bool VclFiltersTest::load(const OUString &,
    const OUString &rURL, const OUString &,
    unsigned int, unsigned int, unsigned int)
{
    GraphicFilter aGraphicFilter(false);
    SvFileStream aFileStream(rURL, STREAM_READ);
    Graphic aGraphic;
    return aGraphicFilter.ImportGraphic(aGraphic, rURL, aFileStream) == 0;
}

void VclFiltersTest::testCVEs()
{
#ifndef DISABLE_CVE_TESTS
    testDir(OUString(),
        getURLFromSrc("/vcl/qa/cppunit/graphicfilter/data/wmf/"),
        OUString());

    testDir(OUString(),
        getURLFromSrc("/vcl/qa/cppunit/graphicfilter/data/emf/"),
        OUString());

    testDir(OUString(),
        getURLFromSrc("/vcl/qa/cppunit/graphicfilter/data/sgv/"),
        OUString());

    testDir(OUString(),
        getURLFromSrc("/vcl/qa/cppunit/graphicfilter/data/png/"),
        OUString());

    testDir(OUString(),
        getURLFromSrc("/vcl/qa/cppunit/graphicfilter/data/jpg/"),
        OUString());

    testDir(OUString(),
        getURLFromSrc("/vcl/qa/cppunit/graphicfilter/data/gif/"),
        OUString());

    testDir(OUString(),
        getURLFromSrc("/vcl/qa/cppunit/graphicfilter/data/bmp/"),
        OUString());
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(VclFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
