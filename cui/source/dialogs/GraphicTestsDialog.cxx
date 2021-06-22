/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <GraphicsTestsDialog.hxx>
#include <vcl/test/GraphicsRenderTests.hxx>

GraphicsTestsDialog::GraphicsTestsDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/graphictestdlg.ui", "GraphicTestsDialog")
    , m_xResultLog(m_xBuilder->weld_label("gptest_label1"))
    , m_xDownloadResults(m_xBuilder->weld_button("gptest_downld"))
{
    m_xDownloadResults->connect_clicked(LINK(this, GraphicsTestsDialog, HandleDownloadRequest));
    for (int i = 1; i <= 60; i++)
    {
        OUString atemp = "test_label_" + OUString::number(i);
        m_xTestLabels.push_back(
            m_xBuilder->weld_button(OUStringToOString(atemp, RTL_TEXTENCODING_UTF8)));
        m_xTestLabels.back()->connect_clicked(
            LINK(this, GraphicsTestsDialog, HandleResultViewRequest));
    }
    runGraphicsTestandUpdateLog();
}

void GraphicsTestsDialog::runGraphicsTestandUpdateLog()
{
    GraphicsRenderTests TestObject;
    TestObject.run(true);
    OUString aResultLog = "Graphics Backend used: " + TestObject.m_aCurGraphicsBackend
                          + "\nPassed Tests : " + OUString::number(TestObject.m_aPassed.size())
                          + "\nQuirky Tests : " + OUString::number(TestObject.m_aQuirky.size())
                          + "\nFailed Tests : " + OUString::number(TestObject.m_aFailed.size())
                          + "\nSkipped Tests : " + OUString::number(TestObject.m_aSkipped.size())
                          + "\n(Click any test to view its resultant bitmap)";
    m_xResultLog->set_tooltip_text(aResultLog);
    m_xResultImage = TestObject.m_aResultantBitmap;
    int testNumber = 0;
    for (class OString& tests : TestObject.m_aPassed)
    {
        OString atemp = tests;
        tests += " [PASSED]";
        m_xResultImage[tests] = TestObject.m_aResultantBitmap[atemp];
        m_xTestLabels[testNumber++]->set_label(
            OUString::intern(tests.getStr(), tests.getLength(), RTL_TEXTENCODING_UTF8));
    }
    for (class OString& tests : TestObject.m_aQuirky)
    {
        OString atemp = tests;
        tests += " [QUIRKY]";
        m_xResultImage[tests] = TestObject.m_aResultantBitmap[atemp];
        m_xTestLabels[testNumber++]->set_label(
            OUString::intern(tests.getStr(), tests.getLength(), RTL_TEXTENCODING_UTF8));
    }
    for (class OString& tests : TestObject.m_aFailed)
    {
        OString atemp = tests;
        tests += " [FAILED]";
        m_xResultImage[tests] = TestObject.m_aResultantBitmap[atemp];
        m_xTestLabels[testNumber++]->set_label(
            OUString::intern(tests.getStr(), tests.getLength(), RTL_TEXTENCODING_UTF8));
    }
    for (class OString& tests : TestObject.m_aSkipped)
    {
        tests += " [SKIPPED]";
        m_xSkippedTests[tests] = true;
        m_xTestLabels[testNumber++]->set_label(
            OUString::intern(tests.getStr(), tests.getLength(), RTL_TEXTENCODING_UTF8));
    }
}

IMPL_STATIC_LINK_NOARG(GraphicsTestsDialog, HandleDownloadRequest, weld::Button&, void)
{
    //TODO: Enter code for downloading the results to user's system.
    return;
}

IMPL_LINK(GraphicsTestsDialog, HandleResultViewRequest, weld::Button&, rButton, void)
{
    //Will launch the Imageviewer with the corresponding test Bitmap and test title.
    OUString atitle = rButton.get_label();
    OString atemp = OUStringToOString(atitle, RTL_TEXTENCODING_UTF8).getStr();
    if (m_xSkippedTests[atemp])
    {
        return;
    }
    std::unique_ptr<ImageViewerDialog> m_ImgVwDialog(
        new ImageViewerDialog(m_xDialog.get(), BitmapEx(m_xResultImage[atemp]), atitle));
    m_ImgVwDialog->run();
}
