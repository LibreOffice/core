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
#include <test/xmltesttools.hxx>
#include <tools/stream.hxx>
#include <vcl/gdimtf.hxx>
#include <vcl/graph.hxx>
#include <vcl/metaactiontypes.hxx>

#include <filter/PictReader.hxx>

using namespace ::com::sun::star;

/* Implementation of Filters test */

class PictFilterTest
    : public test::FiltersTest
    , public test::BootstrapFixture
    , public XmlTestTools
{
public:
    PictFilterTest() : BootstrapFixture(true, false) {}

    virtual bool load(const OUString &,
        const OUString &rURL, const OUString &,
        SfxFilterFlags, SotClipboardFormatId, unsigned int) override;

    OUString pictURL()
    {
        return m_directories.getURLFromSrc(u"/vcl/qa/cppunit/graphicfilter/data/pict/");
    }

    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    void testDontClipTooMuch();

    CPPUNIT_TEST_SUITE(PictFilterTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testDontClipTooMuch);
    CPPUNIT_TEST_SUITE_END();
};

bool PictFilterTest::load(const OUString &,
    const OUString &rURL, const OUString &,
    SfxFilterFlags, SotClipboardFormatId, unsigned int)
{
    SvFileStream aFileStream(rURL, StreamMode::READ);
    Graphic aGraphic;
    return ImportPictGraphic(aFileStream, aGraphic);
}

void PictFilterTest::testCVEs()
{
#ifndef DISABLE_CVE_TESTS
    testDir(OUString(), pictURL());
#endif
}

void PictFilterTest::testDontClipTooMuch()
{
    SvFileStream aFileStream(pictURL() + "clipping-problem.pct", StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    ReadPictFile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    dumper.filterAllActionTypes();
    dumper.filterActionType(MetaActionType::CLIPREGION, false);
    xmlDocUniquePtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT (pDoc);

    assertXPath(pDoc, "/metafile/clipregion[5]"_ostr, "top"_ostr, u"0"_ustr);
    assertXPath(pDoc, "/metafile/clipregion[5]"_ostr, "left"_ostr, u"0"_ustr);
    assertXPath(pDoc, "/metafile/clipregion[5]"_ostr, "bottom"_ostr, u"empty"_ustr);
    assertXPath(pDoc, "/metafile/clipregion[5]"_ostr, "right"_ostr, u"empty"_ustr);
}

CPPUNIT_TEST_SUITE_REGISTRATION(PictFilterTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
