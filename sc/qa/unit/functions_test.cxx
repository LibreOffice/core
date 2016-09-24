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
#include "scdll.hxx"
#include "helper/qahelper.hxx"

#include "document.hxx"

class FunctionsTest : public ScBootstrapFixture, public test::FiltersTest
{
public:

    FunctionsTest();

    virtual void setUp() override;

    virtual void tearDown() override;

    virtual bool load(
        const OUString &rFilter,
        const OUString &rURL,
        const OUString &rUserData,
        SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID,
        unsigned int nFilterVersion) override;

    void testFormulasFODS();

    CPPUNIT_TEST_SUITE(FunctionsTest);
    CPPUNIT_TEST(testFormulasFODS);
    CPPUNIT_TEST_SUITE_END();

private:
    uno::Reference<uno::XInterface> m_xCalcComponent;
};

FunctionsTest::FunctionsTest():
    ScBootstrapFixture("sc/qa/unit/data/functions/fods")
{
}

void FunctionsTest::setUp()
{
    ScBootstrapFixture::setUp();

    // This is a bit of a fudge, we do this to ensure that ScGlobals::ensure,
    // which is a private symbol to us, gets called
    m_xCalcComponent =
        getMultiServiceFactory()->createInstance("com.sun.star.comp.Calc.SpreadsheetDocument");
    CPPUNIT_ASSERT_MESSAGE("no calc component!", m_xCalcComponent.is());
}

void FunctionsTest::tearDown()
{
    ScBootstrapFixture::tearDown();
}

bool FunctionsTest::load(const OUString& rFilter, const OUString& rURL,
        const OUString& rUserData, SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID,
        unsigned int nFilterVersion)
{
    ScDocShellRef xDocShRef = ScBootstrapFixture::load(rURL, rFilter, rUserData,
        OUString(), nFilterFlags, nClipboardID, nFilterVersion );
    CPPUNIT_ASSERT(xDocShRef.Is());

    xDocShRef->DoHardRecalc(true);

    ScDocument& rDoc = xDocShRef->GetDocument();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, rDoc.GetValue(1, 2, 0), 1e-14);

    xDocShRef->DoClose();

    return true;
}

void FunctionsTest::testFormulasFODS()
{
    OUString aDirectoryURL = m_directories.getURLFromSrc("/sc/qa/unit/data/functions/fods/");
    recursiveScan(test::pass, "OpenDocument Spreadsheet Flat XML", aDirectoryURL,
            "com.sun.star.comp.filter.OdfFlatXml,,com.sun.star.comp.Calc.XMLOasisImporter,com.sun.star.comp.Calc.XMLOasisExporter,,,true",
            FODS_FORMAT_TYPE, SotClipboardFormatId::NONE, 0, false);
}

CPPUNIT_TEST_SUITE_REGISTRATION(FunctionsTest);


CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
