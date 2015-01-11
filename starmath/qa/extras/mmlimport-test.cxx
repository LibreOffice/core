/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>

#include <sfx2/docfile.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/sfxmodelfactory.hxx>

#include <document.hxx>
#include <smdll.hxx>

namespace {

using namespace ::com::sun::star;

typedef tools::SvRef<SmDocShell> SmDocShellRef;

class Test : public test::BootstrapFixture
{
public:
    virtual void setUp() SAL_OVERRIDE;
    virtual void tearDown() SAL_OVERRIDE;

    void testSimple();

    CPPUNIT_TEST_SUITE(Test);
    CPPUNIT_TEST(testSimple);
    CPPUNIT_TEST_SUITE_END();

private:
    void loadURL(const OUString &rURL)
    {
        // Cf.
        // filter/source/config/fragments/filters/MathML_XML__Math_.xcu
        SfxFilter* pFilter = new SfxFilter(MATHML_XML,
                                           OUString(),
                                           SFX_FILTER_IMPORT | SFX_FILTER_EXPORT | SFX_FILTER_TEMPLATE,
                                           SFX_FILTER_IMPORT | SFX_FILTER_USESOPTIONS,
                                           "MathML 1.01",
                                           0,
                                           OUString(),
                                           OUString(),
                                           "private:factory/smath*");
        pFilter->SetVersion(SOFFICE_FILEFORMAT_60);

        mxDocShell = new SmDocShell(SFXMODEL_STANDARD |
                                    SFXMODEL_DISABLE_EMBEDDED_SCRIPTS |
                                    SFXMODEL_DISABLE_DOCUMENT_RECOVERY);

        SfxMedium* pSrcMed = new SfxMedium(rURL, STREAM_STD_READ);
        pSrcMed->SetFilter(pFilter);
        pSrcMed->UseInteractionHandler(false);
        bool bLoaded = mxDocShell->DoLoad(pSrcMed);
        CPPUNIT_ASSERT_MESSAGE(OUStringToOString("failed to load " + rURL, RTL_TEXTENCODING_UTF8).getStr(),
                               bLoaded);
    }

    SmDocShellRef mxDocShell;
};

void Test::setUp()
{
    BootstrapFixture::setUp();
    SmGlobals::ensure();
}

void Test::tearDown()
{
    if (mxDocShell) mxDocShell->DoClose();
    BootstrapFixture::tearDown();
}

void Test::testSimple()
{
    loadURL(getURLFromSrc("starmath/qa/extras/data/simple.mml"));
}

CPPUNIT_TEST_SUITE_REGISTRATION(Test);

}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
