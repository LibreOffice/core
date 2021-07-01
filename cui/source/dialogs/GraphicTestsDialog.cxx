/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <comphelper/backupfilehelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/DirectoryHelper.hxx>
#include <osl/file.hxx>
#include <unotools/ZipPackageHelper.hxx>
#include <GraphicsTestsDialog.hxx>
#include <vcl/svapp.hxx>
#include <vcl/test/GraphicsRenderTests.hxx>

GraphicsTestsDialog::GraphicsTestsDialog(weld::Container* pParent)
    : GenericDialogController(pParent, "cui/ui/graphictestdlg.ui", "GraphicTestsDialog")
    , m_xResultLog(m_xBuilder->weld_text_view("gptest_txtVW"))
    , m_xDownloadResults(m_xBuilder->weld_button("gptest_downld"))
{
    OUString userProfile = comphelper::BackupFileHelper::getUserProfileURL();
    m_xZipFileUrl = userProfile + "/GraphicTestResults.zip";
    m_xCreateFolderUrl = userProfile + "/GraphicTestResults";
    osl::Directory::create(m_xCreateFolderUrl);
    m_xDownloadResults->connect_clicked(LINK(this, GraphicsTestsDialog, HandleDownloadRequest));
    for (int i = 1; i <= GraphicsRenderTests::getNumberOfTests(); i++)
    {
        OString labelID("test_label_" + std::to_string(i));
        OString buttonID("test_button_" + std::to_string(i));
        m_xTestLabels.push_back(m_xBuilder->weld_label(labelID));
        m_xTestButtons.push_back(m_xBuilder->weld_button(buttonID));
        m_xTestButtons.back()->connect_clicked(
            LINK(this, GraphicsTestsDialog, HandleResultViewRequest));
    }
    runGraphicsTestandUpdateLog();
}

void GraphicsTestsDialog::runGraphicsTestandUpdateLog()
{
    GraphicsRenderTests TestObject;
    TestObject.run(true);
    OUString resultLog
        = TestObject.getResultString() + "\n(Click on any test to view its resultant bitmap image)";
    m_xResultLog->set_text(resultLog);
    m_xResultImage = TestObject.getResultBitmaps();
    int testNumber = 0;
    for (const class OUString& tests : TestObject.getPassedTests())
    {
        m_xTestLabels[testNumber]->set_label(tests);
        m_xTestButtons[testNumber]->set_label("[PASSED]");
        m_xTestButtons[testNumber]->set_tooltip_text(tests);
        m_xTestButtons[testNumber++]->set_background(COL_LIGHTGREEN);
    }
    for (class OUString& tests : TestObject.getQuirkyTests())
    {
        m_xTestLabels[testNumber]->set_label(tests);
        m_xTestButtons[testNumber]->set_label("[QUIRKY]");
        m_xTestButtons[testNumber]->set_tooltip_text(tests);
        m_xTestButtons[testNumber++]->set_background(COL_YELLOW);
    }
    for (class OUString& tests : TestObject.getFailedTests())
    {
        m_xTestLabels[testNumber]->set_label(tests);
        m_xTestButtons[testNumber]->set_label("[FAILED]");
        m_xTestButtons[testNumber]->set_tooltip_text(tests);
        m_xTestButtons[testNumber++]->set_background(COL_LIGHTRED);
    }
    for (class OUString& tests : TestObject.getSkippedTests())
    {
        m_xTestLabels[testNumber]->set_label(tests);
        m_xTestButtons[testNumber]->set_label("[SKIPPED]");
        m_xTestButtons[testNumber]->set_tooltip_text(tests);
        m_xTestButtons[testNumber++]->set_background(COL_LIGHTGRAYBLUE);
    }
}

IMPL_LINK_NOARG(GraphicsTestsDialog, HandleDownloadRequest, weld::Button&, void)
{
    osl::File::remove(m_xZipFileUrl); // Remove previous exports
    try
    {
        utl::ZipPackageHelper aZipHelper(comphelper::getProcessComponentContext(), m_xZipFileUrl);
        aZipHelper.addFolderWithContent(aZipHelper.getRootFolder(), m_xCreateFolderUrl);
        aZipHelper.savePackage();
    }
    catch (const std::exception&)
    {
        OUString Info = "Creation of Zip file failed!";
        std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(
            m_xDialog.get(), VclMessageType::Warning, VclButtonsType::Ok, Info));
        xBox->run();
        return;
    }
    FileExportedDialog aDialog(
        m_xDialog.get(),
        "The results have been successfully saved in the file 'GraphicTestResults.zip' !");
    aDialog.run();
}

IMPL_LINK(GraphicsTestsDialog, HandleResultViewRequest, weld::Button&, rButton, void)
{
    //Will launch the Imageviewer with the corresponding test Bitmap and test title.
    OUString atitle = rButton.get_tooltip_text();
    if (rButton.get_label() == "[SKIPPED]")
    {
        return;
    }
    std::unique_ptr<ImageViewerDialog> m_ImgVwDialog(
        new ImageViewerDialog(m_xDialog.get(), BitmapEx(m_xResultImage[atitle]), atitle));
    m_ImgVwDialog->run();
}

GraphicsTestsDialog::~GraphicsTestsDialog()
{
    comphelper::DirectoryHelper::deleteDirRecursively(m_xCreateFolderUrl);
}
