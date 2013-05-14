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

/* Implementation of Filters test */

using namespace ::com::sun::star;

class SdFiltersTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    SdFiltersTest();

    ::sd::DrawDocShellRef loadURL( const OUString &rURL );
    virtual bool load( const OUString &rFilter,
        const OUString &rURL, const OUString &rUserData,
        unsigned int nFilterFlags, unsigned int nClipboardID,
        unsigned int nFilterVersion);

    virtual void setUp();
    virtual void tearDown();

    // Ensure CVEs remain unbroken
    void testCVEs();

    CPPUNIT_TEST_SUITE(SdFiltersTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<document::XFilter> m_xFilter;
    uno::Reference<uno::XInterface> m_xDrawComponent;
};

#define PPTX_FORMAT_TYPE 268959811

struct FileFormat {
    const char* pName; const char* pFilterName; const char* pTypeName; sal_uLong nFormatType;
};

// cf. sc/qa/unit/filters-test.cxx and filters/...*.xcu to fill out.
FileFormat aFileFormats[] = {
    { "pptx" , "Impress MS PowerPoint 2007 XML", "MS PowerPoint 2007 XML", PPTX_FORMAT_TYPE },
    { 0, 0, 0, 0 }
};

::sd::DrawDocShellRef SdFiltersTest::loadURL( const OUString &rURL )
{
    FileFormat *pFmt = NULL;

    for (size_t i = 0; i < SAL_N_ELEMENTS (aFileFormats); i++)
    {
        pFmt = aFileFormats + i;
        if (pFmt->pName && rURL.endsWithIgnoreAsciiCaseAsciiL (pFmt->pName, strlen (pFmt->pName)))
            break;
    }
    CPPUNIT_ASSERT_MESSAGE( "missing filter info", pFmt && pFmt->pName != NULL );

    sal_uInt32 nFormat = 0;
    if (pFmt->nFormatType)
        nFormat = SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS;
    SfxFilter* aFilter = new SfxFilter(
        OUString::createFromAscii( pFmt->pFilterName ),
        OUString(), pFmt->nFormatType, nFormat,
        OUString::createFromAscii( pFmt->pTypeName ),
        0, OUString(), OUString(), /* userdata */
        OUString("private:factory/sdraw*") );
    aFilter->SetVersion(SOFFICE_FILEFORMAT_CURRENT);

    ::sd::DrawDocShellRef xDocShRef = new ::sd::DrawDocShell();
    SfxMedium* pSrcMed = new SfxMedium(rURL, STREAM_STD_READ);
    pSrcMed->SetFilter(aFilter);
    if ( !xDocShRef->DoLoad(pSrcMed) )
    {
        if (xDocShRef.Is())
            xDocShRef->DoClose();
        CPPUNIT_ASSERT_MESSAGE( "failed to load", false );
    }

    return xDocShRef;
}

bool SdFiltersTest::load(const OUString &rFilter, const OUString &rURL,
    const OUString &rUserData, unsigned int nFilterFlags, unsigned int nClipboardID,
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
    testDir(OUString("MS PowerPoint 97"),
            getURLFromSrc("/sd/qa/unit/data/ppt/"),
            OUString("sdfilt"));

    testDir(OUString("draw8"),
            getURLFromSrc("/sd/qa/unit/data/odg/"),
            OUString("sdfilt"));
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
