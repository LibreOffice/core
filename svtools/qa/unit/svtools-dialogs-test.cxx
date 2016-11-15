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

/// Test opening a dialog in svtools
class SvtoolsDialogsTest : public ScreenshotTest
{
private:
    /// helper method to populate KnownDialogs, called in setUp(). Needs to be
    /// written and has to add entries to KnownDialogs
    virtual void registerKnownDialogsByID(mapType& rKnownDialogs) override;

    /// dialog creation for known dialogs by ID. Has to be implemented for
    /// each registered known dialog
    virtual VclPtr<VclAbstractDialog> createDialogByID(sal_uInt32 nID) override;

public:
    SvtoolsDialogsTest();
    virtual ~SvtoolsDialogsTest() override;

    // try to open a dialog
    void openAnyDialog();

    CPPUNIT_TEST_SUITE(SvtoolsDialogsTest);
    CPPUNIT_TEST(openAnyDialog);
    CPPUNIT_TEST_SUITE_END();
};

SvtoolsDialogsTest::SvtoolsDialogsTest()
{
}

SvtoolsDialogsTest::~SvtoolsDialogsTest()
{
}

void SvtoolsDialogsTest::registerKnownDialogsByID(mapType& /*rKnownDialogs*/)
{
    // fill map of known dialogs
}

VclPtr<VclAbstractDialog> SvtoolsDialogsTest::createDialogByID(sal_uInt32 /*nID*/)
{
    return nullptr;
}

void SvtoolsDialogsTest::openAnyDialog()
{
    /// process input file containing the UXMLDescriptions of the dialogs to dump
    processDialogBatchFile("svtools/qa/unit/data/svtools-dialogs-test.txt");
}

CPPUNIT_TEST_SUITE_REGISTRATION(SvtoolsDialogsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
