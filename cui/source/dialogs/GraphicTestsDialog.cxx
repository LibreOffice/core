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

GraphicTestEntry::GraphicTestEntry(weld::Container* pParent, weld::Dialog* pDialog,
                                   OUString aTestName, OUString aTestStatus, Bitmap aTestBitmap)
    : m_xBuilder(Application::CreateBuilder(pParent, "cui/ui/graphictestentry.ui"))
    , m_xContainer(m_xBuilder->weld_container("gptestbox"))
    , m_xTestLabel(m_xBuilder->weld_label("gptestlabel"))
    , m_xTestButton(m_xBuilder->weld_button("gptestbutton"))
    , m_xResultBitmap(aTestBitmap)
{
    m_xParentDialog = pDialog;
    m_xTestLabel->set_label(aTestName);
    m_xTestLabel->set_background(COL_BLUE);
    m_xTestButton->set_label(aTestStatus);
    m_xTestButton->set_background(aTestStatus == "PASSED"
                                      ? COL_GREEN
                                      : aTestStatus == "QUIRKY"
                                            ? COL_YELLOW
                                            : aTestStatus == "FAILED" ? COL_LIGHTRED : COL_GRAY);
    m_xTestButton->connect_clicked(LINK(this, GraphicTestEntry, HandleResultViewRequest));
    m_xContainer->show();
}

IMPL_LINK(GraphicTestEntry, HandleResultViewRequest, weld::Button&, rButton, void)
{
    if (rButton.get_label() == "SKIPPED")
    {
        return;
    }
    std::unique_ptr<ImageViewerDialog> m_ImgVwDialog(
        new ImageViewerDialog(m_xParentDialog, BitmapEx(m_xResultBitmap), rButton.get_label()));
    m_ImgVwDialog->run();
}

GraphicsTestsDialog::GraphicsTestsDialog(weld::Window* pParent)
    : GenericDialogController(pParent, "cui/ui/graphictestdlg.ui", "GraphicTestsDialog")
    , m_xResultLog(m_xBuilder->weld_text_view("gptest_txtVW"))
    , m_xDownloadResults(m_xBuilder->weld_button("gptest_downld"))
    , m_xContainerBox(m_xBuilder->weld_box("gptest_box"))
{
    m_xDownloadResults->connect_clicked(LINK(this, GraphicsTestsDialog, HandleDownloadRequest));
}

short GraphicsTestsDialog::run()
{
    GraphicsRenderTests TestObject;
    TestObject.run(true);
    OUString resultLog
        = TestObject.getResultString() + "\n(Click on any test to view its resultant bitmap image)";
    m_xResultLog->set_text(resultLog);
    sal_Int32 testNumber = 0;
    for (const class OUString& tests : TestObject.getPassedTests())
    {
        auto xGpTest
            = std::make_unique<GraphicTestEntry>(m_xContainerBox.get(), m_xDialog.get(), tests,
                                                 "PASSED", TestObject.getResultBitmaps()[tests]);
        m_xContainerBox->reorder_child(xGpTest->get_widget(), testNumber++);
    }
    for (class OUString& tests : TestObject.getQuirkyTests())
    {
        auto xGpTest
            = std::make_unique<GraphicTestEntry>(m_xContainerBox.get(), m_xDialog.get(), tests,
                                                 "QUIRKY", TestObject.getResultBitmaps()[tests]);
        m_xContainerBox->reorder_child(xGpTest->get_widget(), testNumber++);
    }
    for (class OUString& tests : TestObject.getFailedTests())
    {
        auto xGpTest
            = std::make_unique<GraphicTestEntry>(m_xContainerBox.get(), m_xDialog.get(), tests,
                                                 "FAILED", TestObject.getResultBitmaps()[tests]);
        m_xContainerBox->reorder_child(xGpTest->get_widget(), testNumber++);
    }
    for (class OUString& tests : TestObject.getSkippedTests())
    {
        auto xGpTest
            = std::make_unique<GraphicTestEntry>(m_xContainerBox.get(), m_xDialog.get(), tests,
                                                 "SKIPPED", TestObject.getResultBitmaps()[tests]);
        m_xContainerBox->reorder_child(xGpTest->get_widget(), testNumber++);
    }
    return GenericDialogController::run();
}

IMPL_STATIC_LINK_NOARG(GraphicsTestsDialog, HandleDownloadRequest, weld::Button&, void)
{
    //TODO: Enter code for downloading the results to user's system.
    return;
}
