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

extern "C"
{
    SAL_DLLPUBLIC_EXPORT bool SAL_CALL
        itgGraphicImport(SvStream & rStream, Graphic & rGraphic,
        FilterConfigItem*);
}

using namespace ::com::sun::star;

/* Implementation of Filters test */

class TgaFilterTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    TgaFilterTest() : BootstrapFixture(true, false) {}

    virtual bool load(const OUString &,
        const OUString &rURL, const OUString &,
        SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    CPPUNIT_TEST_SUITE(TgaFilterTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST_SUITE_END();
};

bool TgaFilterTest::load(const OUString &,
    const OUString &rURL, const OUString &,
    SfxFilterFlags, SotClipboardFormatId, unsigned int)
{
    SvFileStream aFileStream(rURL, StreamMode::READ);
    Graphic aGraphic;
    return itgGraphicImport(aFileStream, aGraphic, nullptr);
}

void TgaFilterTest::testCVEs()
{
    testDir(OUString(),
        m_directories.getURLFromSrc(u"/filter/qa/cppunit/data/tga/"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(TgaFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
