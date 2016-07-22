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
#include <sfx2/app.hxx>
#include <vcl/abstdlg.hxx>

using namespace ::com::sun::star;

/// Test opening a dialog in sfx2
class Sfx2DialogsTest : public ScreenshotTest
{
private:
    /// helper method to populate KnownDialogs, called in setUp(). Needs to be
    /// written and has to add entries to KnownDialogs
    virtual void registerKnownDialogsByID(mapType& rKnownDialogs) override;

    /// dialog creation for known dialogs by ID. Has to be implemented for
    /// each registered known dialog
    virtual VclAbstractDialog* createDialogByID(sal_uInt32 nID) override;

public:
    Sfx2DialogsTest();
    virtual ~Sfx2DialogsTest();

    virtual void setUp() override;

    // try to open a dialog
    void openAnyDialog();

    CPPUNIT_TEST_SUITE(Sfx2DialogsTest);
    CPPUNIT_TEST(openAnyDialog);
    CPPUNIT_TEST_SUITE_END();
};

Sfx2DialogsTest::Sfx2DialogsTest()
{
}

Sfx2DialogsTest::~Sfx2DialogsTest()
{
}

void Sfx2DialogsTest::setUp()
{
    ScreenshotTest::setUp();
}

void Sfx2DialogsTest::registerKnownDialogsByID(mapType& /*rKnownDialogs*/)
{
    // fill map of known dilogs
}

VclAbstractDialog* Sfx2DialogsTest::createDialogByID(sal_uInt32 /*nID*/)
{
    return nullptr;
}

void Sfx2DialogsTest::openAnyDialog()
{
    /// example how to process an input file containing the UXMLDescriptions of the dialogs
    /// to dump
    if (true)
    {
        test::Directories aDirectories;
        OUString aURL = aDirectories.getURLFromSrc("sfx2/qa/unit/data/sfx2-dialogs-test.txt");
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
        const OString aUIXMLDescription("sfx/ui/documentpropertiesdialog.ui");

        dumpDialogToPath(aUIXMLDescription);
    }
}

CPPUNIT_TEST_SUITE_REGISTRATION(Sfx2DialogsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
