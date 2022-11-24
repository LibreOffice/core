/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <unotest/filters-test.hxx>
#include <test/bootstrapfixture.hxx>

#include "helper/qahelper.hxx"

#include <docsh.hxx>
#include <inputopt.hxx>
#include <postit.hxx>
#include <document.hxx>
#include <drwlayer.hxx>
#include <userdat.hxx>
#include <formulacell.hxx>
#include <testlotus.hxx>
#include <dbdata.hxx>
#include <sortparam.hxx>
#include <scerrors.hxx>
#include <scopetools.hxx>
#include <undomanager.hxx>

#include <svx/svdpage.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

/* Implementation of Filters test */

class ScFiltersTest
    : public test::FiltersTest
    , public ScBootstrapFixture
{
public:
    ScFiltersTest();

    virtual bool load( const OUString &rFilter, const OUString &rURL,
        const OUString &rUserData, SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID, unsigned int nFilterVersion) override;
    /**
     * Ensure CVEs remain unbroken
     */
    void testCVEs();

    void testContentofz9704();
    void testTooManyColsRows();


    CPPUNIT_TEST_SUITE(ScFiltersTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testContentofz9704);
    CPPUNIT_TEST(testTooManyColsRows);

    CPPUNIT_TEST_SUITE_END();

private:
    void createFileURL(std::u16string_view aFileBase, std::u16string_view aFileExtension, OUString& rFilePath);
};

void ScFiltersTest::createFileURL(
    std::u16string_view aFileBase, std::u16string_view aFileExtension, OUString& rFilePath)
{
    // aFileBase may contain multiple segments, so use
    // GetNewAbsURL instead of insertName for them:
    INetURLObject url(m_directories.getSrcRootURL());
    url.setFinalSlash();
    url.GetNewAbsURL("sc/qa/unit/data", &url);
    url.insertName(aFileExtension, true);
    url.GetNewAbsURL(OUString::Concat(aFileBase) + aFileExtension, &url);
    rFilePath = url.GetMainURL(INetURLObject::DecodeMechanism::NONE);
}


bool ScFiltersTest::load(const OUString &rFilter, const OUString &rURL,
    const OUString &rUserData, SfxFilterFlags nFilterFlags,
    SotClipboardFormatId nClipboardID, unsigned int nFilterVersion)
{
    ScDocShellRef xDocShRef = ScBootstrapFixture::load(rURL, rFilter, rUserData,
        OUString(), nFilterFlags, nClipboardID, nFilterVersion );
    bool bLoaded = xDocShRef.is();
    //reference counting of ScDocShellRef is very confused.
    if (bLoaded)
        xDocShRef->DoClose();
    return bLoaded;
}

void ScFiltersTest::testCVEs()
{
#ifndef DISABLE_CVE_TESTS
    testDir("Quattro Pro 6.0",
        m_directories.getURLFromSrc(u"/sc/qa/unit/data/qpro/"));

    //warning, the current "sylk filter" in sc (docsh.cxx) automatically
    //chains on failure on trying as csv, rtf, etc. so "success" may
    //not indicate that it imported as .slk.
    testDir("SYLK",
        m_directories.getURLFromSrc(u"/sc/qa/unit/data/slk/"));

    testDir("MS Excel 97",
        m_directories.getURLFromSrc(u"/sc/qa/unit/data/xls/"));

    testDir("Calc Office Open XML",
        m_directories.getURLFromSrc(u"/sc/qa/unit/data/xlsx/"), OUString(), XLSX_FORMAT_TYPE);

    testDir("Calc Office Open XML",
        m_directories.getURLFromSrc(u"/sc/qa/unit/data/xlsm/"), OUString(), XLSX_FORMAT_TYPE);

    testDir("dBase",
        m_directories.getURLFromSrc(u"/sc/qa/unit/data/dbf/"));

    testDir("Lotus",
        m_directories.getURLFromSrc(u"/sc/qa/unit/data/wks/"));

#endif
}

void ScFiltersTest::testContentofz9704()
{
    OUString aFileName;
    createFileURL(u"ofz9704.", u"123", aFileName);
    SvFileStream aFileStream(aFileName, StreamMode::READ);
    TestImportWKS(aFileStream);
}

void ScFiltersTest::testTooManyColsRows()
{
    // The intentionally doc has cells beyond our MAXROW/MAXCOL, so there
    // should be a warning on load.
    OUString aFileName;
    createFileURL(u"too-many-cols-rows.", u"ods", aFileName );
    ScDocShellRef xDocSh = ScBootstrapFixture::load(aFileName, "calc8", OUString(), OUString(),
            ODS_FORMAT_TYPE, SotClipboardFormatId::STARCALC_8);

    CPPUNIT_ASSERT(xDocSh->GetErrorCode() == SCWARN_IMPORT_ROW_OVERFLOW
                   || xDocSh->GetErrorCode() == SCWARN_IMPORT_COLUMN_OVERFLOW);
    xDocSh->DoClose();

    createFileURL(u"too-many-cols-rows.", u"xlsx", aFileName );
    xDocSh = ScBootstrapFixture::load(
            aFileName, "Calc Office Open XML", OUString(), OUString(),
            XLSX_FORMAT_TYPE, SotClipboardFormatId::STARCALC_8);

    CPPUNIT_ASSERT(xDocSh->GetErrorCode() == SCWARN_IMPORT_ROW_OVERFLOW
                   || xDocSh->GetErrorCode() == SCWARN_IMPORT_COLUMN_OVERFLOW);
    xDocSh->DoClose();
}

ScFiltersTest::ScFiltersTest()
    : ScBootstrapFixture()
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
