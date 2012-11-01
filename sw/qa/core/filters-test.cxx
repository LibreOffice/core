/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Red Hat, Inc., Caolán McNamara <caolanm@redhat.com>
 *  (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
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

SO2_DECL_REF(SwDocShell)
SO2_IMPL_REF(SwDocShell)

using namespace ::com::sun::star;

/* Implementation of Filters test */

class SwFiltersTest
    : public test::FiltersTest
    , public test::BootstrapFixture
{
public:
    virtual bool load( const rtl::OUString &rFilter, const rtl::OUString &rURL,
        const rtl::OUString &rUserData, unsigned int nFilterFlags,
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

bool SwFiltersTest::load(const rtl::OUString &rFilter, const rtl::OUString &rURL,
    const rtl::OUString &rUserData, unsigned int nFilterFlags,
        unsigned int nClipboardID, unsigned int nFilterVersion)
{
    SfxFilter* pFilter = new SfxFilter(
        rFilter, rtl::OUString(), nFilterFlags,
        nClipboardID, rtl::OUString(), 0, rtl::OUString(),
        rUserData, rtl::OUString());
    pFilter->SetVersion(nFilterVersion);

    SwDocShellRef xDocShRef = new SwDocShell;
    SfxMedium* pSrcMed = new SfxMedium(rURL, STREAM_STD_READ);
    pSrcMed->SetFilter(pFilter);

    if (rUserData == FILTER_TEXT_DLG)
    {
        pSrcMed->GetItemSet()->Put(
            SfxStringItem(SID_FILE_FILTEROPTIONS, rtl::OUString("UTF8,LF,Liberation Mono,en-US")));
    }

    bool bLoaded = xDocShRef->DoLoad(pSrcMed);
    if (xDocShRef.Is())
        xDocShRef->DoClose();
    return bLoaded;
}

void SwFiltersTest::testCVEs()
{
    testDir(rtl::OUString("Staroffice XML (Writer)"),
            getURLFromSrc("/sw/qa/core/data/xml/"),
            rtl::OUString(FILTER_XML),
            SFX_FILTER_IMPORT | SFX_FILTER_OWN | SFX_FILTER_DEFAULT,
            -1, SOFFICE_FILEFORMAT_CURRENT);

    testDir(rtl::OUString("writer8"),
            getURLFromSrc("/sw/qa/core/data/odt/"),
            rtl::OUString(FILTER_XML),
            SFX_FILTER_IMPORT | SFX_FILTER_OWN | SFX_FILTER_DEFAULT,
            -1, SOFFICE_FILEFORMAT_CURRENT);

    testDir(rtl::OUString("MS Word 97"),
            getURLFromSrc("/sw/qa/core/data/ww8/"),
            rtl::OUString(FILTER_WW8));

    testDir(rtl::OUString("Text (encoded)"),
            getURLFromSrc("/sw/qa/core/data/txt/"),
            rtl::OUString(FILTER_TEXT_DLG));
}

void SwFiltersTest::setUp()
{
    test::BootstrapFixture::setUp();

    //This is a bit of a fudge, we do this to ensure that SwGlobals::ensure,
    //which is a private symbol to us, gets called
    m_xWriterComponent =
        getMultiServiceFactory()->createInstance(rtl::OUString(
        RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.Writer.TextDocument")));
    CPPUNIT_ASSERT_MESSAGE("no writer component!", m_xWriterComponent.is());
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwFiltersTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
