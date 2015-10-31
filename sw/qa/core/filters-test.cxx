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

#include <comphelper/processfactory.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <sfx2/sfxsids.hrc>

#include <svl/stritem.hxx>
#include <unotools/tempfile.hxx>

#include "init.hxx"
#include "iodetect.hxx"
#include "swtypes.hxx"
#include "doc.hxx"
#include "docsh.hxx"
#include "shellres.hxx"
#include "docufld.hxx"

typedef tools::SvRef<SwDocShell> SwDocShellRef;

using namespace ::com::sun::star;

/* Implementation of Filters test */

class SwFiltersTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    virtual bool load( const OUString &rFilter, const OUString &rURL,
        const OUString &rUserData, SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID, unsigned int nFilterVersion) override;
    virtual bool save( const OUString &rFilter, const OUString &rURL,
        const OUString &rUserData, SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID, unsigned int nFilterVersion) override;
    virtual void setUp() override;

    // Ensure CVEs remain unbroken
    void testCVEs();

    CPPUNIT_TEST_SUITE(SwFiltersTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST_SUITE_END();

private:
    bool filter( const OUString &rFilter, const OUString &rURL,
        const OUString &rUserData, SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID, unsigned int nFilterVersion, bool bExport);
    uno::Reference<uno::XInterface> m_xWriterComponent;
};

bool SwFiltersTest::load(const OUString &rFilter, const OUString &rURL,
    const OUString &rUserData, SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID, unsigned int nFilterVersion)
{
    return filter(rFilter, rURL, rUserData, nFilterFlags, nClipboardID, nFilterVersion, false);
}

bool SwFiltersTest::save(const OUString &rFilter, const OUString &rURL,
    const OUString &rUserData, SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID, unsigned int nFilterVersion)
{
    return filter(rFilter, rURL, rUserData, nFilterFlags, nClipboardID, nFilterVersion, true);
}

bool SwFiltersTest::filter(const OUString &rFilter, const OUString &rURL,
    const OUString &rUserData, SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID, unsigned int nFilterVersion, bool bExport)
{
    SfxFilter* pFilter = new SfxFilter(
        rFilter, OUString(), nFilterFlags,
        nClipboardID, OUString(), 0, OUString(),
        rUserData, OUString());
    pFilter->SetVersion(nFilterVersion);

    SwDocShellRef xDocShRef = new SwDocShell;
    SfxMedium* pSrcMed = new SfxMedium(rURL, STREAM_STD_READ);

    const SfxFilter* pImportFilter = 0;
    SfxFilter* pExportFilter = 0;
    if (bExport)
    {
        SfxGetpApp()->GetFilterMatcher().GuessFilter(*pSrcMed, &pImportFilter, SfxFilterFlags::IMPORT, SfxFilterFlags::NONE);
        pExportFilter = pFilter;
    }
    else
        pImportFilter = pFilter;

    pSrcMed->SetFilter(pImportFilter);

    if (rUserData == FILTER_TEXT_DLG)
    {
        pSrcMed->GetItemSet()->Put(
            SfxStringItem(SID_FILE_FILTEROPTIONS, OUString("UTF8,LF,Liberation Mono,en-US")));
    }

    bool bLoaded = xDocShRef->DoLoad(pSrcMed);
    if (!bExport)
    {
        if (xDocShRef.Is())
            xDocShRef->DoClose();
        return bLoaded;
    }

    // How come an error may be set, and still DoLoad() returns success? Strange...
    if (bLoaded)
        xDocShRef->ResetError();

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    SfxMedium aDstMed(aTempFile.GetURL(), STREAM_STD_WRITE);
    aDstMed.SetFilter(pExportFilter);
    bool bSaved = xDocShRef->DoSaveAs(aDstMed);
    if (xDocShRef.Is())
        xDocShRef->DoClose();
    return bSaved;
}

#define isstorage SotClipboardFormatId::STRING

void SwFiltersTest::testCVEs()
{
    testDir(OUString("StarOffice XML (Writer)"),
            getURLFromSrc("/sw/qa/core/data/xml/"),
            OUString(FILTER_XML),
            SfxFilterFlags::IMPORT | SfxFilterFlags::OWN | SfxFilterFlags::DEFAULT,
            isstorage, SOFFICE_FILEFORMAT_CURRENT);

    testDir(OUString("writer8"),
            getURLFromSrc("/sw/qa/core/data/odt/"),
            OUString(FILTER_XML),
            SfxFilterFlags::IMPORT | SfxFilterFlags::OWN | SfxFilterFlags::DEFAULT,
            isstorage, SOFFICE_FILEFORMAT_CURRENT);

    testDir(OUString("MS Word 97"),
            getURLFromSrc("/sw/qa/core/data/ww8/"),
            OUString(FILTER_WW8));

    testDir(OUString("MS WinWord 6.0"),
            getURLFromSrc("/sw/qa/core/data/ww6/"),
            OUString(sWW6));

    testDir(OUString("MS WinWord 5"),
            getURLFromSrc("/sw/qa/core/data/ww5/"),
            OUString(sWW5));

    testDir(OUString("Text (encoded)"),
            getURLFromSrc("/sw/qa/core/data/txt/"),
            OUString(FILTER_TEXT_DLG));

    testDir(OUString("MS Word 2007 XML"),
            getURLFromSrc("/sw/qa/core/data/ooxml/"),
            OUString(),
            SfxFilterFlags::STARONEFILTER);

    testDir(OUString("Rich Text Format"),
            getURLFromSrc("/sw/qa/core/data/rtf/"),
            OUString(),
            SfxFilterFlags::STARONEFILTER);

    testDir(OUString("HTML"),
            getURLFromSrc("/sw/qa/core/data/html/"),
            OUString(sHTML));

    testDir("T602Document",
            getURLFromSrc("/sw/qa/core/data/602/"),
            OUString(),
            SfxFilterFlags::STARONEFILTER);

    testDir("Rich Text Format",
            getURLFromSrc("/sw/qa/core/exportdata/rtf/"),
            OUString(),
            SfxFilterFlags::STARONEFILTER,
            SotClipboardFormatId::NONE,
            0,
            /*bExport=*/true);

    testDir("HTML",
            getURLFromSrc("/sw/qa/core/exportdata/html/"),
            OUString(sHTML),
            SfxFilterFlags::NONE,
            SotClipboardFormatId::NONE,
            0,
            /*bExport=*/true);

    testDir("MS Word 2007 XML",
            getURLFromSrc("/sw/qa/core/exportdata/ooxml/"),
            OUString(),
            SfxFilterFlags::STARONEFILTER,
            SotClipboardFormatId::NONE,
            0,
            /*bExport=*/true);

}

void SwFiltersTest::setUp()
{
    test::BootstrapFixture::setUp();

    //This is a bit of a fudge, we do this to ensure that SwGlobals::ensure,
    //which is a private symbol to us, gets called
    m_xWriterComponent =
        getMultiServiceFactory()->createInstance("com.sun.star.comp.Writer.TextDocument");
    CPPUNIT_ASSERT_MESSAGE("no writer component!", m_xWriterComponent.is());
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
