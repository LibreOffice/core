/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */


#include "functions_test.hxx"
#include "document.hxx"

FunctionsTest::FunctionsTest(const OUString& rPath):
    ScBootstrapFixture(rPath)
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

bool FunctionsTest::load(const OUString& rFilter, const OUString& rURL,
        const OUString& rUserData, SfxFilterFlags nFilterFlags,
        SotClipboardFormatId nClipboardID,
        unsigned int nFilterVersion)
{
    ScDocShellRef xDocShRef = ScBootstrapFixture::load(rURL, rFilter, rUserData,
        OUString(), nFilterFlags, nClipboardID, nFilterVersion );
    CPPUNIT_ASSERT(xDocShRef.is());

    xDocShRef->DoHardRecalc(true);

    ScDocument& rDoc = xDocShRef->GetDocument();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, rDoc.GetValue(1, 2, 0), 1e-14);

    xDocShRef->DoClose();

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
