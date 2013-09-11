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

#include <cppuhelper/implbase1.hxx>

#include <comphelper/processfactory.hxx>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <sfx2/sfxsids.hrc>

#include <svl/stritem.hxx>

#include "init.hxx"
#include "iodetect.hxx"
#include "swtypes.hxx"
#include "doc.hxx"
#include "docsh.hxx"
#include "shellres.hxx"
#include "docufld.hxx"

SV_DECL_REF(SwDocShell)
SV_IMPL_REF(SwDocShell)

using namespace ::com::sun::star;

/* Implementation of Filters test */

class SwFiltersTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    virtual bool load( const OUString &rFilter, const OUString &rURL,
        const OUString &rUserData, unsigned int nFilterFlags,
        unsigned int nClipboardID, unsigned int nFilterVersion);
    virtual void setUp();

    // Ensure CVEs remain unbroken
    void testCVEs();

    CPPUNIT_TEST_SUITE(SwFiltersTest);
    CPPUNIT_TEST(testCVEs);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xWriterComponent;
};

bool SwFiltersTest::load(const OUString &rFilter, const OUString &rURL,
    const OUString &rUserData, unsigned int nFilterFlags,
        unsigned int nClipboardID, unsigned int nFilterVersion)
{
    SfxFilter* pFilter = new SfxFilter(
        rFilter, OUString(), nFilterFlags,
        nClipboardID, OUString(), 0, OUString(),
        rUserData, OUString());
    pFilter->SetVersion(nFilterVersion);

    SwDocShellRef xDocShRef = new SwDocShell;
    SfxMedium* pSrcMed = new SfxMedium(rURL, STREAM_STD_READ);
    pSrcMed->SetFilter(pFilter);

    if (rUserData == FILTER_TEXT_DLG)
    {
        pSrcMed->GetItemSet()->Put(
            SfxStringItem(SID_FILE_FILTEROPTIONS, OUString("UTF8,LF,Liberation Mono,en-US")));
    }

    bool bLoaded = xDocShRef->DoLoad(pSrcMed);
    if (xDocShRef.Is())
        xDocShRef->DoClose();
    return bLoaded;
}

#define isstorage 1

void SwFiltersTest::testCVEs()
{
    testDir(OUString("Staroffice XML (Writer)"),
            getURLFromSrc("/sw/qa/core/data/xml/"),
            OUString(FILTER_XML),
            SFX_FILTER_IMPORT | SFX_FILTER_OWN | SFX_FILTER_DEFAULT,
            isstorage, SOFFICE_FILEFORMAT_CURRENT);

    testDir(OUString("writer8"),
            getURLFromSrc("/sw/qa/core/data/odt/"),
            OUString(FILTER_XML),
            SFX_FILTER_IMPORT | SFX_FILTER_OWN | SFX_FILTER_DEFAULT,
            isstorage, SOFFICE_FILEFORMAT_CURRENT);

    testDir(OUString("MS Word 97"),
            getURLFromSrc("/sw/qa/core/data/ww8/"),
            OUString(FILTER_WW8));

    testDir(OUString("MS WinWord 5"),
            getURLFromSrc("/sw/qa/core/data/ww5/"),
            OUString(sWW5));

    testDir(OUString("Text (encoded)"),
            getURLFromSrc("/sw/qa/core/data/txt/"),
            OUString(FILTER_TEXT_DLG));

    testDir(OUString("MS Word 2007 XML"),
            getURLFromSrc("/sw/qa/core/data/ooxml/"),
            OUString(),
            SFX_FILTER_STARONEFILTER);

    testDir(OUString("Rich Text Format"),
            getURLFromSrc("/sw/qa/core/data/rtf/"),
            OUString(),
            SFX_FILTER_STARONEFILTER);

    testDir(OUString("HTML"),
            getURLFromSrc("/sw/qa/core/data/html/"),
            OUString(sHTML));
}

void SwFiltersTest::setUp()
{
    test::BootstrapFixture::setUp();

    //This is a bit of a fudge, we do this to ensure that SwGlobals::ensure,
    //which is a private symbol to us, gets called
    m_xWriterComponent =
        getMultiServiceFactory()->createInstance(OUString(
        "com.sun.star.comp.Writer.TextDocument"));
    CPPUNIT_ASSERT_MESSAGE("no writer component!", m_xWriterComponent.is());
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
