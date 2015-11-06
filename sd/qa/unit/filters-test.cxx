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
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/document/XFilter.hpp>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>

#include <svx/svdtext.hxx>
#include <svx/svdotext.hxx>

#include "drawdoc.hxx"
#include "../source/ui/inc/DrawDocShell.hxx"

#include <osl/process.h>
#include <osl/thread.h>

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
    SfxFilter aFilter(
        rFilter,
        OUString(), nFilterFlags, nClipboardID, OUString(), 0, OUString(),
        rUserData, OUString() );
    aFilter.SetVersion(nFilterVersion);

    ::sd::DrawDocShellRef xDocShRef = new ::sd::DrawDocShell();
    SfxMedium* pSrcMed = new SfxMedium(rURL, STREAM_STD_READ);
    pSrcMed->SetFilter(&aFilter);
    bool bLoaded = xDocShRef->DoLoad(pSrcMed);
    xDocShRef->DoClose();
    return bLoaded;
}

void SdFiltersTest::testCVEs()
{
#ifndef DISABLE_CVE_TESTS
    testDir("MS PowerPoint 97",
            getURLFromSrc("/sd/qa/unit/data/ppt/"),
            "sdfilt");

    testDir("Impress Office Open XML",
            getURLFromSrc("/sd/qa/unit/data/pptx/"),
            "",  (SfxFilterFlags::IMPORT | SfxFilterFlags::ALIEN | SfxFilterFlags::STARONEFILTER));

    testDir("impress8",
            getURLFromSrc("/sd/qa/unit/data/odp/"),
            "sdfilt");

    testDir("draw8",
            getURLFromSrc("/sd/qa/unit/data/odg/"),
            "sdfilt");

    testDir("CGM - Computer Graphics Metafile",
            getURLFromSrc("/sd/qa/unit/data/cgm/"),
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
