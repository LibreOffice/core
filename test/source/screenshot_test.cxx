    /* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "test/screenshot_test.hxx"

#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/frame/Desktop.hpp>
#include <comphelper/processfactory.hxx>
#include <vcl/abstdlg.hxx>
#include <vcl/pngwrite.hxx>
#include <vcl/svapp.hxx>

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

ScreenshotTest::ScreenshotTest()
:   m_aScreenshotDirectory("/workdir/screenshots/"),
    maKnownDialogs()
{
}

ScreenshotTest::~ScreenshotTest()
{
}

void ScreenshotTest::setUp()
{
    test::BootstrapFixture::setUp();

    mxDesktop = css::frame::Desktop::create( comphelper::getComponentContext(getMultiServiceFactory()) );
    CPPUNIT_ASSERT_MESSAGE("no desktop!", mxDesktop.is());

    osl::FileBase::RC err = osl::Directory::create( m_directories.getURLFromSrc( m_aScreenshotDirectory ) );
    CPPUNIT_ASSERT_MESSAGE( "Failed to create screenshot directory", (err == osl::FileBase::E_None || err == osl::FileBase::E_EXIST) );

    // initialize maKnownDialogs
    if (maKnownDialogs.empty())
    {
        registerKnownDialogsByID(maKnownDialogs);
    }
}

void ScreenshotTest::tearDown()
{
    test::BootstrapFixture::tearDown();
}

void ScreenshotTest::implSaveScreenshot(const Bitmap& rScreenshot, const OString& rScreenshotId)
{
    OUString aDirname, aBasename;
    splitHelpId(rScreenshotId, aDirname, aBasename);
    aDirname = m_aScreenshotDirectory + aDirname;

    osl::FileBase::RC err = osl::Directory::createPath(m_directories.getURLFromSrc(aDirname));
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString("Failed to create " + aDirname, RTL_TEXTENCODING_UTF8).getStr(),
        (err == osl::FileBase::E_None || err == osl::FileBase::E_EXIST));

    OUString aFullPath = m_directories.getSrcRootPath() + aDirname + "/" + aBasename + ".png";
    SvFileStream aNew(aFullPath, StreamMode::WRITE | StreamMode::TRUNC);
    CPPUNIT_ASSERT_MESSAGE(OUStringToOString("Failed to open " + OUString::number(aNew.GetErrorCode()), RTL_TEXTENCODING_UTF8).getStr(), aNew.IsOpen());

    vcl::PNGWriter aPNGWriter(rScreenshot);
    aPNGWriter.Write(aNew);
}

void ScreenshotTest::saveScreenshot(VclAbstractDialog& rDialog)
{
    const Bitmap aScreenshot(rDialog.createScreenshot());

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
    const Bitmap aScreenshot(rDialog.createScreenshot());

    if (!aScreenshot.IsEmpty())
    {
        const OString aScreenshotId = rDialog.GetScreenshotId();

        if (!aScreenshotId.isEmpty())
        {
            implSaveScreenshot(aScreenshot, aScreenshotId);
        }
    }
}

VclAbstractDialog* ScreenshotTest::createDialogByName(const OString& rName)
{
    VclAbstractDialog* pRetval = nullptr;
    const mapType::const_iterator aHit = maKnownDialogs.find(rName);

    if (aHit != maKnownDialogs.end())
    {
        return createDialogByID((*aHit).second);
    }

    return pRetval;
}

void ScreenshotTest::dumpDialogToPath(VclAbstractDialog& rDialog)
{
    const std::vector<OString> aPageDescriptions(rDialog.getAllPageUIXMLDescriptions());

    if (aPageDescriptions.size())
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

    if (aPageDescriptions.size())
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
            VclBuilder aBuilder(pDialog, VclBuilderContainer::getUIRootDir(), OStringToOUString(rUIXMLDescription, RTL_TEXTENCODING_UTF8));
            vcl::Window *pRoot = aBuilder.get_widget_root();
            Dialog *pRealDialog = dynamic_cast<Dialog*>(pRoot);

            if (!pRealDialog)
            {
                pRealDialog = pDialog;
            }

            pRealDialog->SetText("LibreOffice DialogScreenshot");
            pRealDialog->SetStyle(pDialog->GetStyle() | WB_CLOSEABLE);

            dumpDialogToPath(*pRealDialog);
        }

        pDialog.disposeAndClear();
    }
}

void ScreenshotTest::processAllKnownDialogs()
{
    for (mapType::const_iterator i = getKnownDialogs().begin(); i != getKnownDialogs().end(); i++)
    {
        std::unique_ptr<VclAbstractDialog> pDlg(createDialogByID((*i).second));

        if (pDlg)
        {
            // known dialog, dump screenshot to path
            dumpDialogToPath(*pDlg);
        }
        else
        {
            // unknown dialog, should not happen in this basic loop.
            // You have probably forgotten to add a case and
            // implementastion to createDialogByID, please do this
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
            // first check if it's a known dialog
            std::unique_ptr<VclAbstractDialog> pDlg(createDialogByName(aNextUIFile));

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
