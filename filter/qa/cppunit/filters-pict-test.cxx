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

#include <osl/file.hxx>
#include <osl/process.h>

#include "../../source/graphicfilter/ipict/ipict.hxx"

extern "C"
{
    SAL_DLLPUBLIC_EXPORT bool SAL_CALL
        iptGraphicImport(SvStream & rStream, Graphic & rGraphic,
        FilterConfigItem*);
}

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
        return m_directories.getURLFromSrc("/filter/qa/cppunit/data/pict/");
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
    return iptGraphicImport(aFileStream, aGraphic, nullptr);
}

void PictFilterTest::testCVEs()
{
    testDir(OUString(),
        pictURL());
}

void PictFilterTest::testDontClipTooMuch()
{
    SvFileStream aFileStream(pictURL() + "clipping-problem.pct", StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    pict::ReadPictFile(aFileStream, aGDIMetaFile);

    MetafileXmlDump dumper;
    dumper.filterAllActionTypes();
    dumper.filterActionType(MetaActionType::CLIPREGION, false);
    xmlDocPtr pDoc = dumpAndParse(dumper, aGDIMetaFile);

    CPPUNIT_ASSERT (pDoc);

    assertXPath(pDoc, "/metafile/clipregion[5]", "top", "0");
    assertXPath(pDoc, "/metafile/clipregion[5]", "left", "0");
    assertXPath(pDoc, "/metafile/clipregion[5]", "bottom", "-32767");
    assertXPath(pDoc, "/metafile/clipregion[5]", "right", "-32767");
}

CPPUNIT_TEST_SUITE_REGISTRATION(PictFilterTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
