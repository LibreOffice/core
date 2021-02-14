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
#include <filter/EpsReader.hxx>

using namespace css;

/* Implementation of Filters test */

class EpsFilterTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    EpsFilterTest() : BootstrapFixture(true, false) {}

    virtual bool load(const OUString &,
        const OUString &rURL, const OUString &,
        SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    CPPUNIT_TEST_SUITE(EpsFilterTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST_SUITE_END();
};

bool EpsFilterTest::load(const OUString &,
    const OUString &rURL, const OUString &,
    SfxFilterFlags, SotClipboardFormatId, unsigned int)
{
    SvFileStream aFileStream(rURL, StreamMode::READ);
    Graphic aGraphic;
    return ImportEpsGraphic(aFileStream, aGraphic);
}

void EpsFilterTest::testCVEs()
{
#ifndef DISABLE_CVE_TESTS
    testDir(OUString(),
        m_directories.getURLFromSrc(u"/vcl/qa/cppunit/graphicfilter/data/eps/"));
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(EpsFilterTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
