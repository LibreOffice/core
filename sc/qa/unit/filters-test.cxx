/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include "helper/scfiltertestbase.hxx"

#include <docsh.hxx>
#include <document.hxx>
#include <testlotus.hxx>

#include <osl/file.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <tools/stream.hxx>
#include <tools/urlobj.hxx>
#include <unotools/tempfile.hxx>

using namespace ::com::sun::star;

/* Implementation of Filters test */

class ScFiltersTest
    : public ScFilterTestBase
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
    void testTdf90299();

    CPPUNIT_TEST_SUITE(ScFiltersTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST(testContentofz9704);
    CPPUNIT_TEST(testTdf90299);

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
    ScDocShellRef xDocShRef = loadDoc(rURL, rFilter, rUserData,
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

void ScFiltersTest::testTdf90299()
{
    const OUString aTmpDirectory1URL = utl::CreateTempURL(nullptr, true);
    const OUString aTmpDirectory2URL = utl::CreateTempURL(nullptr, true);
    const OUString aSavedFileURL = utl::CreateTempURL(&aTmpDirectory1URL);

    OUString aReferencedFileURL;
    OUString aReferencingFileURL;
    createFileURL(u"tdf90299.", u"xls", aReferencingFileURL);

    auto eError = osl::File::copy(aReferencingFileURL, aTmpDirectory1URL + "/tdf90299.xls");
    CPPUNIT_ASSERT_EQUAL(osl::File::E_None, eError);

    aReferencingFileURL = aTmpDirectory1URL + "/tdf90299.xls";
    aReferencedFileURL = aTmpDirectory1URL + "/dummy.xls";

    ScDocShellRef xShell = loadDoc(aReferencingFileURL, "MS Excel 97", OUString(), OUString(),
            XLS_FORMAT_TYPE, SotClipboardFormatId::STARCALC_8);

    ScDocument& rDoc = xShell->GetDocument();
    CPPUNIT_ASSERT_EQUAL(OUString("='" + aReferencedFileURL + "'#$Sheet1.A1"), rDoc.GetFormula(0, 0, 0));

    aReferencingFileURL = aSavedFileURL;

    SfxMedium aStoreMedium(aReferencingFileURL, StreamMode::STD_WRITE);

    auto pExportFilter = std::make_shared<SfxFilter>(
        "MS Excel 97", OUString(), XLS_FORMAT_TYPE, SotClipboardFormatId::NONE, OUString(),
        OUString(), OUString(), "private:factory/scalc*");
    pExportFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);

    aStoreMedium.SetFilter(pExportFilter);

    xShell->DoSaveAs(aStoreMedium);
    xShell->DoClose();

    eError = osl::File::copy(aReferencingFileURL, aTmpDirectory2URL + "/tdf90299.xls");
    CPPUNIT_ASSERT_EQUAL(osl::File::E_None, eError);

    aReferencingFileURL = aTmpDirectory2URL + "/tdf90299.xls";
    aReferencedFileURL = aTmpDirectory2URL + "/dummy.xls";

    xShell = loadDoc(aReferencingFileURL, "MS Excel 97", OUString(), OUString(),
            XLS_FORMAT_TYPE, SotClipboardFormatId::STARCALC_8);
    ScDocument& rDoc2 = xShell->GetDocument();
    CPPUNIT_ASSERT_EQUAL(OUString("='" + aReferencedFileURL + "'#$Sheet1.A1"), rDoc2.GetFormula(0, 0, 0));

    xShell->DoClose();
}

ScFiltersTest::ScFiltersTest()
    : ScFilterTestBase()
{
}

CPPUNIT_TEST_SUITE_REGISTRATION(ScFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
