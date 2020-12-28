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
#include <com/sun/star/lang/XComponent.hpp>

#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>

#include <drawdoc.hxx>
#include <DrawDocShell.hxx>

using namespace ::com::sun::star;

/// Test loading of files to assure they do not crash on load.
class SdFiltersTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    SdFiltersTest();

    virtual bool load( const OUString &rFilter,
        const OUString &rURL, const OUString &rUserData,
        SfxFilterFlags nFilterFlags, SotClipboardFormatId nClipboardID,
        unsigned int nFilterVersion) override;

    virtual void setUp() override;
    virtual void tearDown() override;

    // Ensure CVEs remain unbroken
    void testCVEs();

    CPPUNIT_TEST_SUITE(SdFiltersTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xDrawComponent;
};

bool SdFiltersTest::load(const OUString &rFilter, const OUString &rURL,
    const OUString &rUserData, SfxFilterFlags nFilterFlags, SotClipboardFormatId nClipboardID,
    unsigned int nFilterVersion)
{
    auto pFilter = std::make_shared<SfxFilter>(
        rFilter,
        OUString(), nFilterFlags, nClipboardID, OUString(), OUString(),
        rUserData, OUString() );
    pFilter->SetVersion(nFilterVersion);

    ::sd::DrawDocShellRef xDocShRef = new ::sd::DrawDocShell(SfxObjectCreateMode::EMBEDDED, false, DocumentType::Impress);
    SfxMedium* pSrcMed = new SfxMedium(rURL, StreamMode::STD_READ);
    pSrcMed->SetFilter(pFilter);
    bool bLoaded = xDocShRef->DoLoad(pSrcMed);
    xDocShRef->DoClose();
    return bLoaded;
}

void SdFiltersTest::testCVEs()
{
#ifndef DISABLE_CVE_TESTS
    testDir("MS PowerPoint 97",
            m_directories.getURLFromSrc(u"/sd/qa/unit/data/ppt/"),
            "sdfilt");

    testDir("Impress Office Open XML",
            m_directories.getURLFromSrc(u"/sd/qa/unit/data/pptx/"),
            "",  (SfxFilterFlags::IMPORT | SfxFilterFlags::ALIEN | SfxFilterFlags::STARONEFILTER));

    testDir("impress8",
            m_directories.getURLFromSrc(u"/sd/qa/unit/data/odp/"),
            "sdfilt");

    testDir("draw8",
            m_directories.getURLFromSrc(u"/sd/qa/unit/data/odg/"),
            "sdfilt");

    testDir("CGM - Computer Graphics Metafile",
            m_directories.getURLFromSrc(u"/sd/qa/unit/data/cgm/"),
            "icg");
#endif
}

SdFiltersTest::SdFiltersTest()
{
}

void SdFiltersTest::setUp()
{
    test::BootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xDrawComponent =
        getMultiServiceFactory()->createInstance("com.sun.star.comp.Draw.PresentationDocument");
    CPPUNIT_ASSERT_MESSAGE("no impress component!", m_xDrawComponent.is());
}

void SdFiltersTest::tearDown()
{
    uno::Reference< lang::XComponent >( m_xDrawComponent, uno::UNO_QUERY_THROW )->dispose();
    test::BootstrapFixture::tearDown();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
