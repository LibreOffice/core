/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>
#include <vcl/FilterConfigItem.hxx>
#include <tools/stream.hxx>
#include <vcl/graph.hxx>

#include <osl/file.hxx>
#include <osl/process.h>

extern "C"
{
    SAL_DLLPUBLIC_EXPORT sal_Bool SAL_CALL
        GraphicImport(SvStream & rStream, Graphic & rGraphic,
        FilterConfigItem*, sal_Bool);
}

using namespace ::com::sun::star;

/* Implementation of Filters test */

class PictFilterTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    PictFilterTest() : BootstrapFixture(true, false) {}

    virtual bool load(const OUString &,
        const OUString &rURL, const OUString &,
        unsigned int, unsigned int, unsigned int);

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    CPPUNIT_TEST_SUITE(PictFilterTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST_SUITE_END();
};

bool PictFilterTest::load(const OUString &,
    const OUString &rURL, const OUString &,
    unsigned int, unsigned int, unsigned int)
{
    SvFileStream aFileStream(rURL, STREAM_READ);
    Graphic aGraphic;
    return GraphicImport(aFileStream, aGraphic, NULL, 0);
}

void PictFilterTest::testCVEs()
{
    testDir(OUString(),
        getURLFromSrc("/filter/qa/cppunit/data/pict/"),
        OUString());
}

CPPUNIT_TEST_SUITE_REGISTRATION(PictFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
