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
#include <svx/svdmodel.hxx>
#include <svx/svxdlg.hxx>
#include <vcl/abstdlg.hxx>

using namespace ::com::sun::star;

/// Test opening a dialog in cui
class CuiDialogsTest : public ScreenshotTest
{
private:
    std::unique_ptr<SdrModel> mxModel;
    std::unique_ptr<SfxItemSet> mxAttr;
    SvxAbstractDialogFactory* mpFact;

    void initialize();

    /// helper method to populate KnownDialogs, called in setUp(). Needs to be
    /// written and has to add entries to KnownDialogs
    virtual void registerKnownDialogsByID(mapType& rKnownDialogs) override;

    /// dialog creation for known dialogs by ID. Has to be implemented for
    /// each registered known dialog
    virtual VclPtr<VclAbstractDialog> createDialogByID(sal_uInt32 nID) override;

public:
    CuiDialogsTest();

    // try to open a dialog
    void openAnyDialog();

    CPPUNIT_TEST_SUITE(CuiDialogsTest);
    CPPUNIT_TEST(openAnyDialog);
    CPPUNIT_TEST_SUITE_END();
};

CuiDialogsTest::CuiDialogsTest() {}

void CuiDialogsTest::initialize()
{
    mpFact = SvxAbstractDialogFactory::Create();
    mxModel.reset(new SdrModel(nullptr, nullptr, true));
    mxModel->GetItemPool().FreezeIdRanges();
    mxAttr.reset(new SfxItemSet(mxModel->GetItemPool()));
}

void CuiDialogsTest::registerKnownDialogsByID(mapType& rKnownDialogs)
{
    // fill map of known dialogs
    rKnownDialogs["cui/ui/formatcellsdialog.ui"] = 0;
    rKnownDialogs["cui/ui/textdialog.ui"] = 1;
}

VclPtr<VclAbstractDialog> CuiDialogsTest::createDialogByID(sal_uInt32 nID)
{
    VclPtr<VclAbstractDialog> pReturnDialog;

    switch (nID)
    {
        case 0: // "cui/ui/formatcellsdialog.ui"
        {
            pReturnDialog = mpFact->CreateSvxFormatCellsDialog(nullptr, mxAttr.get(), *mxModel);
            break;
        }

        case 1: // "cui/ui/textdialog.ui"
        {
            pReturnDialog = mpFact->CreateTextTabDialog(nullptr, mxAttr.get(), nullptr);
            break;
        }

        default:
            break;
    }

    return pReturnDialog;
}

void CuiDialogsTest::openAnyDialog()
{
    initialize();

    /// process input file containing the UXMLDescriptions of the dialogs to dump
    processDialogBatchFile(u"cui/qa/unit/data/cui-dialogs-test.txt");
}

CPPUNIT_TEST_SUITE_REGISTRATION(CuiDialogsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
