/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <GraphicsTestsDialog.hxx>

GraphicsTestsDialog::GraphicsTestsDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/graphictestdlg.ui", "GraphicTestsDialog")
    , m_xResultLog(m_xBuilder->weld_text_view("gptestresults"))
    , m_xDownloadResults(m_xBuilder->weld_button("gptest_downld"))
{
    m_xResultLog->set_text("Running tests...");
    m_xDownloadResults->connect_clicked(LINK(this, GraphicsTestsDialog, HandleDownloadRequest));
    runGraphicsTestandUpdateLog();
}

void GraphicsTestsDialog::runGraphicsTestandUpdateLog()
{
    GraphicsRenderTests TestObject;
    TestObject.run();
    OString writeResults;
    OUString awriteResults
        = "    --General Info--\nGraphics Backend used : " + TestObject.m_aCurGraphicsBackend
          + "\nPassed Tests : " + OUString::number(TestObject.m_aPassed.size())
          + "\nQuirky Tests : " + OUString::number(TestObject.m_aQuirky.size())
          + "\nFailed Tests : " + OUString::number(TestObject.m_aFailed.size())
          + "\nSkipped Tests : " + OUString::number(TestObject.m_aSkipped.size());
    writeResults = OUStringToOString(awriteResults, RTL_TEXTENCODING_UTF8);
    writeResults += "\n\n    --Test Details--\n";
    for (const class OString& tests : TestObject.m_aPassed)
    {
        writeResults += tests + " [PASSED]\n";
    }
    for (const class OString& tests : TestObject.m_aQuirky)
    {
        writeResults += tests + " [QUIRKY]\n";
    }
    for (const class OString& tests : TestObject.m_aFailed)
    {
        writeResults += tests + " [FAILED]\n";
    }
    for (const class OString& tests : TestObject.m_aSkipped)
    {
        writeResults += tests + " [SKIPPED]\n";
    }
    m_xResultLog->set_text(
        OUString::intern(writeResults.getStr(), writeResults.getLength(), RTL_TEXTENCODING_UTF8));
}

IMPL_STATIC_LINK_NOARG(GraphicsTestsDialog, HandleDownloadRequest, weld::Button&, void)
{
    //TODO: Enter code for downloading the results to user's system.
}
