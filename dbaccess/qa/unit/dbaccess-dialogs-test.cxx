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

/// Test opening a dialog in dbaccess
class DbaccessDialogsTest : public ScreenshotTest
{
private:
    /// helper method to populate KnownDialogs, called in setUp(). Needs to be
    /// written and has to add entries to KnownDialogs
    virtual void registerKnownDialogsByID(mapType& rKnownDialogs) override;

    /// dialog creation for known dialogs by ID. Has to be implemented for
    /// each registered known dialog
    virtual VclAbstractDialog* createDialogByID(sal_uInt32 nID) override;

public:
    DbaccessDialogsTest();
    virtual ~DbaccessDialogsTest();

    virtual void setUp() override;

    // try to open a dialog
    void openAnyDialog();

    CPPUNIT_TEST_SUITE(DbaccessDialogsTest);
    CPPUNIT_TEST(openAnyDialog);
    CPPUNIT_TEST_SUITE_END();
};

DbaccessDialogsTest::DbaccessDialogsTest()
{
}

DbaccessDialogsTest::~DbaccessDialogsTest()
{
}

void DbaccessDialogsTest::setUp()
{
    ScreenshotTest::setUp();
}

void DbaccessDialogsTest::registerKnownDialogsByID(mapType& /*rKnownDialogs*/)
{
    // fill map of known dilogs
}

VclAbstractDialog* DbaccessDialogsTest::createDialogByID(sal_uInt32 /*nID*/)
{
    return nullptr;
}

void DbaccessDialogsTest::openAnyDialog()
{
    /// process input file containing the UXMLDescriptions of the dialogs to dump
    processDialogBatchFile("dbaccess/qa/unit/data/dbaccess-dialogs-test.txt");
}

CPPUNIT_TEST_SUITE_REGISTRATION(DbaccessDialogsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
