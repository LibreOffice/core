/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <test/screenshot_test.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
//#include <com/sun/star/lang/XComponent.hpp>
//#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
//#include <com/sun/star/presentation/AnimationEffect.hpp>
//#include <com/sun/star/presentation/ClickAction.hpp>

#include <sfx2/app.hxx>
//#include <sfx2/docfilt.hxx>
//#include <sfx2/docfile.hxx>
//#include <sfx2/sfxmodelfactory.hxx>
//#include <svl/stritem.hxx>
//#include <svl/aeitem.hxx>
//#include <editeng/colritem.hxx>
//#include <svx/svdtext.hxx>
//#include <svx/svdotext.hxx>
//#include <vcl/sound.hxx>

//#include "drawdoc.hxx"
//#include "../source/ui/inc/DrawDocShell.hxx"

// #include <osl/process.h>
// #include <osl/thread.h>
// #include <osl/file.hxx>

// #include "sdabstdlg.hxx"
// #include <vcl/pngwrite.hxx>
// #include "unomodel.hxx"
// #include "ViewShell.hxx"
// #include "Window.hxx"
// #include "drawview.hxx"
// #include "DrawViewShell.hxx"
// #include <app.hrc>
// #include "stlsheet.hxx"
// #include "sdattr.hrc"
// #include "strings.hrc"
// #include "sdresid.hxx"
// #include "sdattr.hxx"
// #include "prltempl.hrc"

//#include <comphelper/servicehelper.hxx>
//#include <com/sun/star/frame/Desktop.hpp>
//#include <com/sun/star/frame/XDesktop2.hpp>
//#include <comphelper/processfactory.hxx>
//#include <unotest/macros_test.hxx>

#include <vcl/abstdlg.hxx>

using namespace ::com::sun::star;

/// Test opening a dialog in sd
class SwDialogsTest : public ScreenshotTest
{
private:
    /// helper method to populate KnownDialogs, called in setUp(). Needs to be
    /// written and has to add entries to KnownDialogs
    virtual void registerKnownDialogsByID(mapType& rKnownDialogs) override;

    /// dialog creation for known dialogs by ID. Has to be implemented for
    /// each registered known dialog
    virtual VclAbstractDialog* createDialogByID(sal_uInt32 nID) override;

public:
    SwDialogsTest();
    virtual ~SwDialogsTest();

    virtual void setUp() override;

    // try to open a dialog
    void openAnyDialog();

    CPPUNIT_TEST_SUITE(SwDialogsTest);
    CPPUNIT_TEST(openAnyDialog);
    CPPUNIT_TEST_SUITE_END();
};

SwDialogsTest::SwDialogsTest()
{
}

SwDialogsTest::~SwDialogsTest()
{
}

void SwDialogsTest::setUp()
{
    ScreenshotTest::setUp();
}

void SwDialogsTest::registerKnownDialogsByID(mapType& /*rKnownDialogs*/)
{
    // fill map of known dilogs
}

VclAbstractDialog* SwDialogsTest::createDialogByID(sal_uInt32 /*nID*/)
{
    return nullptr;
}

void SwDialogsTest::openAnyDialog()
{
    /// example how to process an input file containing the UXMLDescriptions of the dialogs
    /// to dump
    if (true)
    {
        test::Directories aDirectories;
        OUString aURL = aDirectories.getURLFromSrc("sw/qa/unit/data/sw-dialogs-test.txt");
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

    /// example how to dump all known dialogs
    if (false)
    {
        // example for SfxTabDialog: 5 -> "modules/sdraw/ui/drawpagedialog.ui"
        // example for TabDialog: 22 -> "modules/simpress/ui/headerfooterdialog.ui"
        // example for self-adapted wizard: 0 -> "modules/simpress/ui/publishingdialog.ui"
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

    /// example how to dump a dialog using fallback functionality
    if (false)
    {
        // unknown dialog, try fallback to generic created
        // VclBuilder-generated instance. Keep in mind that Dialogs
        // using this mechanism will probably not be layouted well
        // since the setup/initialization part is missing. Thus,
        // only use for fallback when only the UI file is available.
        //
        // Take any example here, it's only for demonstration - using
        // even a known one to demonstrate the fallback possibility
        const OString aUIXMLDescription("modules/swriter/ui/abstractdialog.ui");

        dumpDialogToPath(aUIXMLDescription);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwDialogsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
