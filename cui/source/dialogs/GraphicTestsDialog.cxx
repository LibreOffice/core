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
#include <svx/FileExportedDialog.hxx>
#include <unotools/ZipPackageHelper.hxx>
#include <GraphicsTestsDialog.hxx>
#include <vcl/test/GraphicsRenderTests.hxx>
#include <svl/svlresid.hxx>
#include <svl/svl.hrc>
#include <vcl/svapp.hxx>

#include <dialmgr.hxx>
#include <strings.hrc>
#include <ImageViewerDialog.hxx>

GraphicTestEntry::GraphicTestEntry(weld::Container* pParent, weld::Dialog* pDialog,
                                   OUString aTestName, OUString aTestStatus, Bitmap aTestBitmap)
    : m_xBuilder(Application::CreateBuilder(pParent, u"cui/ui/graphictestentry.ui"_ustr))
    , m_xContainer(m_xBuilder->weld_container(u"gptestbox"_ustr))
    , m_xTestLabel(m_xBuilder->weld_label(u"gptestlabel"_ustr))
    , m_xTestButton(m_xBuilder->weld_button(u"gptestbutton"_ustr))
    , m_xResultBitmap(aTestBitmap)
{
    m_xParentDialog = pDialog;
    m_xTestLabel->set_label(aTestName);
    m_xTestButton->set_label(aTestStatus);
    m_xTestButton->set_tooltip_text(aTestName);
    m_xTestButton->set_background(
        aTestStatus == SvlResId(GRTSTR_PASSED)
            ? COL_LIGHTGREEN
            : aTestStatus == SvlResId(GRTSTR_QUIRKY)
                  ? COL_YELLOW
                  : aTestStatus == SvlResId(GRTSTR_FAILED) ? COL_LIGHTRED : COL_LIGHTGRAY);
    m_xTestButton->connect_clicked(LINK(this, GraphicTestEntry, HandleResultViewRequest));
    m_xContainer->show();
}

IMPL_LINK(GraphicTestEntry, HandleResultViewRequest, weld::Button&, rButton, void)
{
    if (rButton.get_label() == SvlResId(GRTSTR_SKIPPED))
    {
        return;
    }
    ImageViewerDialog m_ImgVwDialog(m_xParentDialog, BitmapEx(m_xResultBitmap),
                                    rButton.get_tooltip_text());
    m_ImgVwDialog.run();
}

GraphicsTestsDialog::GraphicsTestsDialog(weld::Container* pParent)
    : GenericDialogController(pParent, u"cui/ui/graphictestdlg.ui"_ustr, u"GraphicTestsDialog"_ustr)
    , m_xResultLog(m_xBuilder->weld_text_view(u"gptest_txtVW"_ustr))
    , m_xDownloadResults(m_xBuilder->weld_button(u"gptest_downld"_ustr))
    , m_xContainerBox(m_xBuilder->weld_box(u"gptest_box"_ustr))
{
    OUString userProfile = comphelper::BackupFileHelper::getUserProfileURL();
    m_xZipFileUrl = userProfile + "/GraphicTestResults.zip";
    m_xCreateFolderUrl = userProfile + "/GraphicTestResults";
    osl::Directory::create(m_xCreateFolderUrl);
    m_xDownloadResults->connect_clicked(LINK(this, GraphicsTestsDialog, HandleDownloadRequest));
}

short GraphicsTestsDialog::run()
{
    GraphicsRenderTests aTestObject;
    aTestObject.run(true);
    OUString aResultLog
        = aTestObject.getResultString(true) + "\n" + CuiResId(RID_CUISTR_CLICK_RESULT);
    m_xResultLog->set_text(aResultLog);
    sal_Int32 nTestNumber = 0;
    for (VclTestResult& test : aTestObject.getTestResults())
    {
        auto xGpTest = std::make_unique<GraphicTestEntry>(m_xContainerBox.get(), m_xDialog.get(),
                                                          test.getTestName(), test.getStatus(true),
                                                          test.getBitmap());
        m_xContainerBox->reorder_child(xGpTest->get_widget(), nTestNumber++);
        m_xGraphicTestEntries.push_back(std::move(xGpTest));
    }
    return GenericDialogController::run();
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
                                             VclButtonsType::Ok, CuiResId(RID_CUISTR_ZIPFAIL)));
        xBox->run();
        return;
    }
    FileExportedDialog aDialog(m_xDialog.get(), CuiResId(RID_CUISTR_SAVED));
    aDialog.run();
}

GraphicsTestsDialog::~GraphicsTestsDialog()
{
    comphelper::DirectoryHelper::deleteDirRecursively(m_xCreateFolderUrl);
}
