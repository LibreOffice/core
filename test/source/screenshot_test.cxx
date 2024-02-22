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
#include <vcl/filter/PngImageWriter.hxx>
#include <vcl/svapp.hxx>
#include <vcl/virdev.hxx>
#include <vcl/weld.hxx>
#include <tools/stream.hxx>


namespace {
    void splitHelpId( std::u16string_view rHelpId, std::u16string_view& rDirname, std::u16string_view& rBasename )
    {
        size_t nIndex = rHelpId.rfind( '/' );

        if( nIndex != 0 && nIndex != std::u16string_view::npos)
            rDirname = rHelpId.substr( 0, nIndex );

        if (nIndex == std::u16string_view::npos)
        {
            if( rHelpId.size() > 0 )
                rBasename = rHelpId;
        }
        else
        {
            if( rHelpId.size() > nIndex+1 )
                rBasename = rHelpId.substr( nIndex+1 );
        }
    }
}

using namespace css;
using namespace css::uno;

    /// the target directory for screenshots
constexpr OUString g_aScreenshotDirectory(u"screenshots"_ustr);

ScreenshotTest::ScreenshotTest()
    : maParent(nullptr, "vcl/ui/screenshotparent.ui", "ScreenShot")
    , mxParentWidget(maParent.getDialog()->weld_content_area())
{
    if (auto const env = getenv("LO_TEST_LOCALE")) {
        maCurrentLanguage = OUString::fromUtf8(env);
    }
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

void ScreenshotTest::implSaveScreenshot(const BitmapEx& rScreenshot, std::u16string_view rScreenshotId)
{
    std::u16string_view aSplitDirname, aBasename;
    splitHelpId(rScreenshotId, aSplitDirname, aBasename);
    OUString aDirname = g_aScreenshotDirectory + "/" + aSplitDirname +
               ( (maCurrentLanguage == "en-US") ? OUString() : "/" + maCurrentLanguage );

    auto const dirUrl = m_directories.getURLFromWorkdir(aDirname);
    auto const e = osl::Directory::createPath(dirUrl);
    if (e != osl::FileBase::E_EXIST) {
        CPPUNIT_ASSERT_EQUAL_MESSAGE(
            OString("Failed to create " + OUStringToOString(dirUrl, RTL_TEXTENCODING_UTF8))
                .getStr(),
            osl::FileBase::E_None, e);
    }

    auto const pngUrl = OUString(dirUrl + "/" + aBasename + ".png");
    SvFileStream aNew(pngUrl, StreamMode::WRITE | StreamMode::TRUNC);
    CPPUNIT_ASSERT_MESSAGE(OString("Failed to open <" + OUStringToOString(pngUrl, RTL_TEXTENCODING_UTF8) + ">: " + OString::number(sal_uInt32(aNew.GetErrorCode()))).getStr(), aNew.IsOpen());

    std::cout << "saving " << pngUrl << ":\n";
    vcl::PngImageWriter aPNGWriter(aNew);
    aPNGWriter.write(rScreenshot);
}

void ScreenshotTest::saveScreenshot(VclAbstractDialog const & rDialog)
{
    const BitmapEx aScreenshot(rDialog.createScreenshot());

    if (!aScreenshot.IsEmpty())
    {
        const OUString aScreenshotId = rDialog.GetScreenshotId();

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
        const OUString aScreenshotId = rDialog.get_help_id();
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
    const std::vector<OUString> aPageDescriptions(rDialog.getAllPageUIXMLDescriptions());

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
            OUString sIdent(xTabCtrl->get_page_ident(i));
            xTabCtrl->set_current_page(sIdent);
            if (xTabCtrl->get_current_page_ident() == sIdent)
            {
                OUString sOrigHelpId(xDialog->get_help_id());
                // skip empty pages
                weld::Container* pPage = xTabCtrl->get_page(sIdent);
                OUString sBuildableName(pPage->get_buildable_name());
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

void ScreenshotTest::dumpDialogToPath(std::string_view rUIXMLDescription)
{
    if (rUIXMLDescription.empty())
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

void ScreenshotTest::processDialogBatchFile(std::u16string_view rFile)
{
    test::Directories aDirectories;
    const OUString aURL(aDirectories.getURLFromSrc(rFile));
    SvFileStream aStream(aURL, StreamMode::READ);
    OString aNextUIFile;
    static constexpr OStringLiteral aComment("#");

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
