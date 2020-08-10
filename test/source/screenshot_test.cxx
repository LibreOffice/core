/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <iostream>

#include <test/screenshot_test.hxx>

#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/abstdlg.hxx>
#include <vcl/pngwrite.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>
#include <tools/stream.hxx>


namespace {
    void splitHelpId( const OString& rHelpId, OUString& rDirname, OUString &rBasename )
    {
        sal_Int32 nIndex = rHelpId.lastIndexOf( '/' );

        if( nIndex > 0 )
            rDirname = OStringToOUString( rHelpId.copy( 0, nIndex ), RTL_TEXTENCODING_UTF8 );

        if( rHelpId.getLength() > nIndex+1 )
            rBasename= OStringToOUString( rHelpId.copy( nIndex+1 ), RTL_TEXTENCODING_UTF8 );
    }
}

using namespace css;
using namespace css::uno;

    /// the target directory for screenshots
constexpr OUStringLiteral g_aScreenshotDirectory("screenshots");

ScreenshotTest::ScreenshotTest()
    : maKnownDialogs()
    , maParent(nullptr, "vcl/ui/screenshotparent.ui", "ScreenShot")
    , mxParentWidget(maParent.getDialog()->weld_content_area())
{
    maCurrentLanguage = OUString::fromUtf8(getenv("LO_TEST_LOCALE"));
}

ScreenshotTest::~ScreenshotTest()
{
}

void ScreenshotTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop = css::frame::Desktop::create( comphelper::getComponentContext(getMultiServiceFactory()) );
    CPPUNIT_ASSERT_MESSAGE("no desktop!", mxDesktop.is());

    osl::Directory::create( m_directories.getURLFromWorkdir( g_aScreenshotDirectory)) ;

    // initialize maKnownDialogs
    if (maKnownDialogs.empty())
    {
        registerKnownDialogsByID(maKnownDialogs);
    }
}

void ScreenshotTest::implSaveScreenshot(const BitmapEx& rScreenshot, const OString& rScreenshotId)
{
    OUString aDirname, aBasename;
    splitHelpId(rScreenshotId, aDirname, aBasename);
    aDirname = g_aScreenshotDirectory + "/" + aDirname +
               ( (maCurrentLanguage == "en-US") ? OUString() : "/" + maCurrentLanguage );

    auto const dirUrl = m_directories.getURLFromWorkdir(aDirname);
    auto const e = osl::Directory::createPath(dirUrl);
    if (e != osl::FileBase::E_EXIST) {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OUStringToOString(
                "Failed to create " + dirUrl, RTL_TEXTENCODING_UTF8).getStr(),
            osl::FileBase::E_None, e);
    }

    auto const pngUrl = OUString(dirUrl + "/" + aBasename + ".png");
    SvFileStream aNew(pngUrl, StreamMode::WRITE | StreamMode::TRUNC);
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString("Failed to open <" + pngUrl + ">: " + OUString::number(sal_uInt32(aNew.GetErrorCode())), RTL_TEXTENCODING_UTF8).getStr(), aNew.IsOpen());

    std::cout << "saving " << pngUrl << ":\n";
    vcl::PNGWriter aPNGWriter(rScreenshot);
    aPNGWriter.Write(aNew);
}

void ScreenshotTest::saveScreenshot(VclAbstractDialog const & rDialog)
{
    const BitmapEx aScreenshot(rDialog.createScreenshot());

    if (!aScreenshot.IsEmpty())
    {
        const OString aScreenshotId = rDialog.GetScreenshotId();

        if (!aScreenshotId.isEmpty())
        {
            implSaveScreenshot(aScreenshot, aScreenshotId);
        }
    }
}

void ScreenshotTest::saveScreenshot(weld::Window& rDialog)
{
    VclPtr<VirtualDevice> xDialogSurface(rDialog.screenshot());
    const BitmapEx aScreenshot(xDialogSurface->GetBitmapEx(Point(), xDialogSurface->GetOutputSizePixel()));

    if (!aScreenshot.IsEmpty())
    {
        const OString aScreenshotId = rDialog.get_help_id();
        assert(!aScreenshotId.isEmpty());
        implSaveScreenshot(aScreenshot, aScreenshotId);
    }
}

VclPtr<VclAbstractDialog> ScreenshotTest::createDialogByName(const OString& rName)
{
    const mapType::const_iterator aHit = maKnownDialogs.find(rName);

    if (aHit != maKnownDialogs.end())
    {
        return createDialogByID((*aHit).second);
    }

    return VclPtr<VclAbstractDialog>();
}

void ScreenshotTest::dumpDialogToPath(VclAbstractDialog& rDialog)
{
    const std::vector<OString> aPageDescriptions(rDialog.getAllPageUIXMLDescriptions());

    if (!aPageDescriptions.empty())
    {
        for (size_t a(0); a < aPageDescriptions.size(); a++)
        {
            if (rDialog.selectPageByUIXMLDescription(aPageDescriptions[a]))
            {
                saveScreenshot(rDialog);
            }
            else
            {
                CPPUNIT_ASSERT(false);
            }
        }
    }
    else
    {
        saveScreenshot(rDialog);
    }
}

void ScreenshotTest::dumpDialogToPath(weld::Builder& rBuilder)
{
    std::unique_ptr<weld::Window> xDialog(rBuilder.create_screenshot_window());

    auto xTabCtrl = rBuilder.weld_notebook("tabcontrol");

    int nPages = xTabCtrl ? xTabCtrl->get_n_pages() : 0;
    if (nPages)
    {
        for (int i = 0; i < nPages; ++i)
        {
            OString sIdent(xTabCtrl->get_page_ident(i));
            xTabCtrl->set_current_page(sIdent);
            if (xTabCtrl->get_current_page_ident() == sIdent)
            {
                OString sOrigHelpId(xDialog->get_help_id());
                // skip empty pages
                weld::Container* pPage = xTabCtrl->get_page(sIdent);
                OString sBuildableName(pPage->get_buildable_name());
                if (!sBuildableName.isEmpty() && !sBuildableName.startsWith("__"))
                    xDialog->set_help_id(pPage->get_help_id());
                saveScreenshot(*xDialog);
                xDialog->set_help_id(sOrigHelpId);
            }
            else
            {
                CPPUNIT_ASSERT(false);
            }
        }
    }
    else
    {
        saveScreenshot(*xDialog);
    }
}

void ScreenshotTest::dumpDialogToPath(const OString& rUIXMLDescription)
{
    if (rUIXMLDescription.isEmpty())
        return;

    bool bNonConforming = rUIXMLDescription == "modules/swriter/ui/sidebarstylepresets.ui" ||
                          rUIXMLDescription == "modules/swriter/ui/sidebartheme.ui" ||
                          rUIXMLDescription == "modules/swriter/ui/notebookbar.ui" ||
                          rUIXMLDescription == "modules/scalc/ui/sidebaralignment.ui" ||
                          rUIXMLDescription == "modules/scalc/ui/sidebarcellappearance.ui" ||
                          rUIXMLDescription == "modules/scalc/ui/sidebarnumberformat.ui" ||
                          rUIXMLDescription == "sfx/ui/helpbookmarkpage.ui" ||
                          rUIXMLDescription == "sfx/ui/helpcontentpage.ui" ||
                          rUIXMLDescription == "sfx/ui/helpindexpage.ui" ||
                          rUIXMLDescription == "sfx/ui/helpsearchpage.ui" ||
                          rUIXMLDescription == "sfx/ui/startcenter.ui" ||
                          rUIXMLDescription == "svx/ui/datanavigator.ui" ||
                          rUIXMLDescription == "svx/ui/xformspage.ui" ||
                          rUIXMLDescription == "modules/dbreport/ui/conditionwin.ui";
    if (bNonConforming) // skip these broken ones
        return;
    std::unique_ptr<weld::Builder> xBuilder(Application::CreateBuilder(mxParentWidget.get(), OStringToOUString(rUIXMLDescription, RTL_TEXTENCODING_UTF8)));
    dumpDialogToPath(*xBuilder);
}

void ScreenshotTest::processAllKnownDialogs()
{
    for (const auto& rDialog : getKnownDialogs())
    {
        ScopedVclPtr<VclAbstractDialog> pDlg(createDialogByID(rDialog.second));

        if (pDlg)
        {
            // known dialog, dump screenshot to path
            dumpDialogToPath(*pDlg);
        }
        else
        {
            // unknown dialog, should not happen in this basic loop.
            // You have probably forgotten to add a case and
            // implementation to createDialogByID, please do this
        }
    }
}

void ScreenshotTest::processDialogBatchFile(const OUString& rFile)
{
    test::Directories aDirectories;
    const OUString aURL(aDirectories.getURLFromSrc(rFile));
    SvFileStream aStream(aURL, StreamMode::READ);
    OString aNextUIFile;
    const OString aComment("#");

    while (aStream.ReadLine(aNextUIFile))
    {
        if (!aNextUIFile.isEmpty() && !aNextUIFile.startsWith(aComment))
        {
            std::cout << "processing " << aNextUIFile << ":\n";

            // first check if it's a known dialog
            ScopedVclPtr<VclAbstractDialog> pDlg(createDialogByName(aNextUIFile));

            if (pDlg)
            {
                // known dialog, dump screenshot to path
                dumpDialogToPath(*pDlg);
            }
            else
            {
                // unknown dialog, try fallback to generic created
                // Builder-generated instance. Keep in mind that Dialogs
                // using this mechanism will probably not be layouted well
                // since the setup/initialization part is missing. Thus,
                // only use for fallback when only the UI file is available.
                dumpDialogToPath(aNextUIFile);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
