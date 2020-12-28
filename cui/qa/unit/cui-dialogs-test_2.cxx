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
#include <vcl/abstdlg.hxx>

using namespace ::com::sun::star;

/// Test opening a dialog in cui
class CuiDialogsTest2 : public ScreenshotTest
{
private:
    /// helper method to populate KnownDialogs, called in setUp(). Needs to be
    /// written and has to add entries to KnownDialogs
    virtual void registerKnownDialogsByID(mapType& rKnownDialogs) override;

    /// dialog creation for known dialogs by ID. Has to be implemented for
    /// each registered known dialog
    virtual VclPtr<VclAbstractDialog> createDialogByID(sal_uInt32 nID) override;

public:
    CuiDialogsTest2();

    // try to open a dialog
    void openAnyDialog();

    CPPUNIT_TEST_SUITE(CuiDialogsTest2);
    CPPUNIT_TEST(openAnyDialog);
    CPPUNIT_TEST_SUITE_END();
};

CuiDialogsTest2::CuiDialogsTest2() {}

void CuiDialogsTest2::registerKnownDialogsByID(mapType& /*rKnownDialogs*/)
{
    // leave empty should be in CuiDialogTest if any
}

VclPtr<VclAbstractDialog> CuiDialogsTest2::createDialogByID(sal_uInt32 /*nID*/) { return nullptr; }

void CuiDialogsTest2::openAnyDialog()
{
    /// process input file containing the UXMLDescriptions of the dialogs to dump
    processDialogBatchFile(u"cui/qa/unit/data/cui-dialogs-test_2.txt");
}

CPPUNIT_TEST_SUITE_REGISTRATION(CuiDialogsTest2);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
