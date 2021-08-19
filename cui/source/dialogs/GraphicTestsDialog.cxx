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
    update_info(aTestName, aTestStatus, aTestBitmap);
    m_xTestButton->connect_clicked(LINK(this, GraphicTestEntry, HandleResultViewRequest));
    m_xContainer->show();
}

void GraphicTestEntry::update_info(const OUString& aTestName, const OUString& aTestStatus,
                                   const Bitmap& aBitmap)
{
    m_xTestLabel->set_label(aTestName);
    m_xTestButton->set_label(aTestStatus);
    m_xTestButton->set_tooltip_text(aTestName);
    m_xTestButton->set_background(
        aTestStatus == "PASSED"
            ? COL_LIGHTGREEN
            : aTestStatus == "QUIRKY" ? COL_YELLOW
                                      : aTestStatus == "FAILED" ? COL_LIGHTRED : COL_LIGHTGRAY);
    m_xResultBitmap = aBitmap;
}

IMPL_LINK(GraphicTestEntry, HandleResultViewRequest, weld::Button&, rButton, void)
{
    if (rButton.get_label() == "SKIPPED")
    {
        return;
    }
    ImageViewerDialog m_ImgVwDialog(m_xParentDialog, BitmapEx(m_xResultBitmap),
                                    rButton.get_tooltip_text());
    m_ImgVwDialog.run();
}

static void runGraphicsTests(void* pData) { static_cast<GraphicsRenderTests*>(pData)->run(true); }

GraphicsTestsDialog::GraphicsTestsDialog(weld::Container* pParent)
    : GenericDialogController(pParent, "cui/ui/graphictestdlg.ui", "GraphicTestsDialog")
    , m_xResultLog(m_xBuilder->weld_text_view("gptest_txtVW"))
    , m_xDownloadResults(m_xBuilder->weld_button("gptest_downld"))
    , m_xContainerBox(m_xBuilder->weld_box("gptest_box"))
    , m_xUpdateTimer("GraphicsTestsDialog updation timer")
{
    OUString userProfile = comphelper::BackupFileHelper::getUserProfileURL();
    m_xZipFileUrl = userProfile + "/GraphicTestResults.zip";
    m_xCreateFolderUrl = userProfile + "/GraphicTestResults";
    osl::Directory::create(m_xCreateFolderUrl);
    m_xDownloadResults->connect_clicked(LINK(this, GraphicsTestsDialog, HandleDownloadRequest));
}

IMPL_LINK_NOARG(GraphicsTestsDialog, updateTestLog, Timer*, void)
{
    GraphicsRenderTests* aTestPointer = static_cast<GraphicsRenderTests*>(m_aTestsPointer);
    if (int(aTestPointer->getTestResults().size()) < aTestPointer->getNumberOfTests())
    {
        m_xUpdateTimer.SetTimeout(1000);
        m_xUpdateTimer.Start();
        m_xResultLog->set_text(aTestPointer->getResultString() + "[Running...]");
    }
    else
    {
        m_xResultLog->set_text(aTestPointer->getResultString()
                               + "(Click on any test's status to view it's resultant bitmap.)");
    }
    while (m_nTestNumber < sal_Int32(aTestPointer->getTestResults().size()))
    {
        OUString aTestName = aTestPointer->getTestResults()[m_nTestNumber].getName();
        OUString aTestStatus = aTestPointer->getTestResults()[m_nTestNumber].getStatus();
        Bitmap aTestBitmap = aTestPointer->getTestResults()[m_nTestNumber].getBitmap();
        auto xGpTest = std::make_unique<GraphicTestEntry>(m_xContainerBox.get(), m_xDialog.get(),
                                                          aTestName, aTestStatus, aTestBitmap);
        m_xContainerBox->reorder_child(xGpTest->get_widget(), m_nTestNumber++);
        m_xGraphicTestEntries.push_back(std::move(xGpTest));
    }
}

short GraphicsTestsDialog::run()
{
    GraphicsRenderTests aTestObject;
    m_aTestsPointer = &aTestObject;
    std::thread aTestRunner(runGraphicsTests, m_aTestsPointer);
    m_xUpdateTimer.SetInvokeHandler(LINK(this, GraphicsTestsDialog, updateTestLog));
    m_xUpdateTimer.SetPriority(TaskPriority::DEFAULT_IDLE);
    m_xUpdateTimer.SetTimeout(1000);
    m_xUpdateTimer.Start();
    short aStatus = GenericDialogController::run();
    aTestRunner.join();
    return aStatus;
}

IMPL_LINK_NOARG(GraphicsTestsDialog, HandleDownloadRequest, weld::Button&, void)
{
    osl::File::remove(m_xZipFileUrl); // Remove the previous export
    try
    {
        utl::ZipPackageHelper aZipHelper(comphelper::getProcessComponentContext(), m_xZipFileUrl);
        aZipHelper.addFolderWithContent(aZipHelper.getRootFolder(), m_xCreateFolderUrl);
        aZipHelper.savePackage();
    }
    catch (const std::exception&)
    {
        std::unique_ptr<weld::MessageDialog> xBox(
            Application::CreateMessageDialog(m_xDialog.get(), VclMessageType::Warning,
                                             VclButtonsType::Ok, "Creation of Zip file failed!"));
        xBox->run();
        return;
    }
    FileExportedDialog aDialog(
        m_xDialog.get(),
        "The results have been successfully saved in the file 'GraphicTestResults.zip' !");
    aDialog.run();
}

GraphicsTestsDialog::~GraphicsTestsDialog()
{
    comphelper::DirectoryHelper::deleteDirRecursively(m_xCreateFolderUrl);
}
