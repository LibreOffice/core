/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <swmodeltestbase.hxx>

#include <comphelper/classids.hxx>
#include <svtools/embedhlp.hxx>
#include <svx/svdpage.hxx>
#include <tools/globname.hxx>
#include <unotest/bootstrapfixturebase.hxx>
#include <vcl/gdimtf.hxx>

#include <wrtsh.hxx>
#include <fmtanchr.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <drawdoc.hxx>
#include <dcontact.hxx>

static char const DATA_DIRECTORY[] = "/sw/qa/core/objectpositioning/data/";

/// Covers sw/source/core/objectpositioning/ fixes.
class SwCoreObjectpositioningTest : public SwModelTestBase
{
};

CPPUNIT_TEST_FIXTURE(SwCoreObjectpositioningTest, testOverlapCrash)
{
    // Load a document with 2 images.
    load(DATA_DIRECTORY, "overlap-crash.odt");

    // Change their anchor type to to-char.
    uno::Reference<beans::XPropertySet> xShape1(getShape(1), uno::UNO_QUERY);
    xShape1->setPropertyValue("AnchorType", uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));
    uno::Reference<beans::XPropertySet> xShape2(getShape(1), uno::UNO_QUERY);
    xShape2->setPropertyValue("AnchorType", uno::makeAny(text::TextContentAnchorType_AT_CHARACTER));

    // Insert a new paragraph at the start.
    SwXTextDocument* pTextDoc = dynamic_cast<SwXTextDocument*>(mxComponent.get());
    SwDocShell* pDocShell = pTextDoc->GetDocShell();
    SwWrtShell* pWrtShell = pDocShell->GetWrtShell();
    // Without the accompanying fix in place, this test would have crashed.
    pWrtShell->SplitNode();
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
