/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "sdmodeltestbase.hxx"

#include <SlideSorterViewShell.hxx>
#include <SlideSorter.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsClipboard.hxx>


using namespace ::com::sun::star;

/// Impress miscellaneous tests.
class SdMiscTest : public SdModelTestBase
{
public:
    void testTdf96206();

    CPPUNIT_TEST_SUITE(SdMiscTest);
    CPPUNIT_TEST(testTdf96206);
    CPPUNIT_TEST_SUITE_END();
};

void SdMiscTest::testTdf96206()
{
    // Copying/pasting slide referring to a non-default master with a text duplicated the master
    sd::DrawDocShellRef xDocShRef = loadURL(getURLFromSrc("/sd/qa/unit/data/odp/tdf96206.odp"), ODP);
//    SdDrawDocument *pDoc = xDocShRef->GetDoc();
//    CPPUNIT_ASSERT_MESSAGE( "no document", pDoc != nullptr );

    auto pSlideSorterVS = sd::slidesorter::SlideSorterViewShell::GetSlideSorter(xDocShRef->GetViewShell()->GetViewShellBase());
    auto& xSlideSorter = pSlideSorterVS->GetSlideSorter();
    auto& xController = xSlideSorter.GetController();
    xController.GetClipboard().DoCopy();
    xController.GetClipboard().DoPaste();

//    xDocShRef->DoClose();
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdMiscTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
