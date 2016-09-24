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
    SAL_DLLPUBLIC_EXPORT bool SAL_CALL
        ipdGraphicImport(SvStream & rStream, Graphic & rGraphic,
        FilterConfigItem*);
}

using namespace ::com::sun::star;

/* Implementation of Filters test */

class PsdFilterTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    PsdFilterTest() : BootstrapFixture(true, false) {}

    virtual bool load(const OUString &,
        const OUString &rURL, const OUString &,
        SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    CPPUNIT_TEST_SUITE(PsdFilterTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST_SUITE_END();
};

bool PsdFilterTest::load(const OUString &,
    const OUString &rURL, const OUString &,
    SfxFilterFlags, SotClipboardFormatId, unsigned int)
{
    SvFileStream aFileStream(rURL, StreamMode::READ);
    Graphic aGraphic;
    return ipdGraphicImport(aFileStream, aGraphic, nullptr);
}

void PsdFilterTest::testCVEs()
{
    testDir(OUString(),
        m_directories.getURLFromSrc("/filter/qa/cppunit/data/psd/"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(PsdFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
