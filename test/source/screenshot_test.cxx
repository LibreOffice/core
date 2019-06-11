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

#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/abstdlg.hxx>
#include <vcl/pngwrite.hxx>
#include <vcl/svapp.hxx>
#include <unotools/configmgr.hxx>
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
static constexpr OUStringLiteral g_aScreenshotDirectory("screenshots");

ScreenshotTest::ScreenshotTest()
    : maKnownDialogs()
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

void ScreenshotTest::saveScreenshot(Dialog& rDialog)
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

void ScreenshotTest::dumpDialogToPath(Dialog& rDialog)
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

void ScreenshotTest::dumpDialogToPath(const OString& rUIXMLDescription)
{
    if (!rUIXMLDescription.isEmpty())
    {
        VclPtrInstance<Dialog> pDialog(Application::GetDefDialogParent(), WB_STDDIALOG | WB_SIZEABLE, Dialog::InitFlag::NoParent);

        {
            VclPtr<vcl::Window> aOwnedToplevel;

            bool bLegacy;
            bLegacy = rUIXMLDescription != "cui/ui/textanimtabpage.ui" &&
                      rUIXMLDescription != "cui/ui/areatabpage.ui" &&
                      rUIXMLDescription != "modules/schart/ui/tp_3D_SceneIllumination.ui";
            std::unique_ptr<VclBuilder> xBuilder(new VclBuilder(pDialog, VclBuilderContainer::getUIRootDir(), OStringToOUString(rUIXMLDescription, RTL_TEXTENCODING_UTF8), OString(), css::uno::Reference<css::frame::XFrame>(), bLegacy));
            vcl::Window *pRoot = xBuilder->get_widget_root();
            Dialog *pRealDialog = dynamic_cast<Dialog*>(pRoot);

            if (!pRealDialog)
            {
                pRealDialog = pDialog;
            }
            else
            {
                aOwnedToplevel.set(pRoot);
                xBuilder->drop_ownership(pRoot);
            }

            pRealDialog->SetText(utl::ConfigManager::getProductName());
            pRealDialog->SetStyle(pDialog->GetStyle() | WB_CLOSEABLE);

            dumpDialogToPath(*pRealDialog);

            if (VclBuilderContainer* pOwnedToplevel = dynamic_cast<VclBuilderContainer*>(aOwnedToplevel.get()))
                pOwnedToplevel->m_pUIBuilder = std::move(xBuilder);
            aOwnedToplevel.disposeAndClear();
        }

        pDialog.disposeAndClear();
    }
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
                // VclBuilder-generated instance. Keep in mind that Dialogs
                // using this mechanism will probably not be layouted well
                // since the setup/initialization part is missing. Thus,
                // only use for fallback when only the UI file is available.
                dumpDialogToPath(aNextUIFile);
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
