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

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <sfx2/fcontnr.hxx>

#include <svl/stritem.hxx>
#include <unotools/tempfile.hxx>

#include <iodetect.hxx>
#include <docsh.hxx>

typedef tools::SvRef<SwDocShell> SwDocShellRef;

using namespace ::com::sun::star;

/* Implementation of Filters test */

class SwFiltersTest2 : public test::FiltersTest, public test::BootstrapFixture
{
public:
    virtual bool load(const OUString& rFilter, const OUString& rURL, const OUString& rUserData,
                      SfxFilterFlags nFilterFlags, SotClipboardFormatId nClipboardID,
                      unsigned int nFilterVersion) override;
    virtual bool save(const OUString& rFilter, const OUString& rURL, const OUString& rUserData,
                      SfxFilterFlags nFilterFlags, SotClipboardFormatId nClipboardID,
                      unsigned int nFilterVersion) override;
    virtual void setUp() override;

    // Ensure CVEs remain unbroken
    void testCVEs();

    CPPUNIT_TEST_SUITE(SwFiltersTest2);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST_SUITE_END();

private:
    bool filter(const OUString& rFilter, const OUString& rURL, const OUString& rUserData,
                SfxFilterFlags nFilterFlags, SotClipboardFormatId nClipboardID,
                unsigned int nFilterVersion, bool bExport);
    uno::Reference<uno::XInterface> m_xWriterComponent;
};

bool SwFiltersTest2::load(const OUString& rFilter, const OUString& rURL, const OUString& rUserData,
                          SfxFilterFlags nFilterFlags, SotClipboardFormatId nClipboardID,
                          unsigned int nFilterVersion)
{
    return filter(rFilter, rURL, rUserData, nFilterFlags, nClipboardID, nFilterVersion, false);
}

bool SwFiltersTest2::save(const OUString& rFilter, const OUString& rURL, const OUString& rUserData,
                          SfxFilterFlags nFilterFlags, SotClipboardFormatId nClipboardID,
                          unsigned int nFilterVersion)
{
    return filter(rFilter, rURL, rUserData, nFilterFlags, nClipboardID, nFilterVersion, true);
}

bool SwFiltersTest2::filter(const OUString& rFilter, const OUString& rURL,
                            const OUString& rUserData, SfxFilterFlags nFilterFlags,
                            SotClipboardFormatId nClipboardID, unsigned int nFilterVersion,
                            bool bExport)
{
    auto pFilter = std::make_shared<SfxFilter>(rFilter, OUString(), nFilterFlags, nClipboardID,
                                               OUString(), OUString(), rUserData, OUString());
    pFilter->SetVersion(nFilterVersion);

    SwDocShellRef xDocShRef = new SwDocShell;
    SfxMedium* pSrcMed = new SfxMedium(rURL, StreamMode::STD_READ);

    std::shared_ptr<const SfxFilter> pImportFilter;
    std::shared_ptr<const SfxFilter> pExportFilter;
    if (bExport)
    {
        SfxGetpApp()->GetFilterMatcher().GuessFilter(*pSrcMed, pImportFilter,
                                                     SfxFilterFlags::IMPORT, SfxFilterFlags::NONE);
        pExportFilter = pFilter;
    }
    else
        pImportFilter = pFilter;

    pSrcMed->SetFilter(pImportFilter);

    if (rUserData == FILTER_TEXT_DLG)
    {
        pSrcMed->GetItemSet()->Put(
            SfxStringItem(SID_FILE_FILTEROPTIONS, "UTF8,LF,Liberation Mono,en-US"));
    }

    bool bLoaded = xDocShRef->DoLoad(pSrcMed);
    if (!bExport)
    {
        if (xDocShRef.is())
            xDocShRef->DoClose();
        return bLoaded;
    }

    // How come an error may be set, and still DoLoad() returns success? Strange...
    if (bLoaded)
        xDocShRef->ResetError();

    utl::TempFile aTempFile;
    aTempFile.EnableKillingFile();
    SfxMedium aDstMed(aTempFile.GetURL(), StreamMode::STD_WRITE);
    aDstMed.SetFilter(pExportFilter);
    bool bSaved = xDocShRef->DoSaveAs(aDstMed);
    if (xDocShRef.is())
        xDocShRef->DoClose();
    return bSaved;
}

void SwFiltersTest2::testCVEs()
{
    testDir("MS WinWord 6.0", m_directories.getURLFromSrc(u"/sw/qa/core/data/ww6/"), sWW6);
}

void SwFiltersTest2::setUp()
{
    test::BootstrapFixture::setUp();

    //This is a bit of a fudge, we do this to ensure that SwGlobals::ensure,
    //which is a private symbol to us, gets called
    m_xWriterComponent
        = getMultiServiceFactory()->createInstance("com.sun.star.comp.Writer.TextDocument");
    CPPUNIT_ASSERT_MESSAGE("no writer component!", m_xWriterComponent.is());
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwFiltersTest2);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
