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
#include <rtl/bootstrap.hxx>
#include <osl/module.hxx>
#include <tools/svlibrary.h>
#include <vcl/abstdlg.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/text/DialogFactoryService.hpp>
#include <swdll.hxx>

class SwAbstractDialogFactory;

using namespace ::com::sun::star;

extern "C" { using Fn = SwAbstractDialogFactory * (*)(); }
    // sw/source/ui/dialog/swuiexp.cxx

/// Test opening a dialog in sw
class SwDialogsTest2 : public ScreenshotTest
{
private:
    osl::Module libSwui_;

    /// helper method to populate KnownDialogs, called in setUp(). Needs to be
    /// written and has to add entries to KnownDialogs
    virtual void registerKnownDialogsByID(mapType& rKnownDialogs) override;

    /// dialog creation for known dialogs by ID. Has to be implemented for
    /// each registered known dialog
    virtual VclPtr<VclAbstractDialog> createDialogByID(sal_uInt32 nID) override;

public:
    SwDialogsTest2();

    void setUp() override;

    // try to open a dialog
    void openAnyDialog();

    CPPUNIT_TEST_SUITE(SwDialogsTest2);
    CPPUNIT_TEST(openAnyDialog);
    CPPUNIT_TEST_SUITE_END();
};

SwDialogsTest2::SwDialogsTest2()
{
}

void SwDialogsTest2::setUp()
{
    ScreenshotTest::setUp();
    SwGlobals::ensure();
    // Make sure the swui library's global pSwResMgr is initialized
    auto xService = css::text::DialogFactoryService::create(comphelper::getProcessComponentContext());
    CPPUNIT_ASSERT(xService.is());
    // get a factory instance
    SwAbstractDialogFactory* pFactory = reinterpret_cast<SwAbstractDialogFactory*>(xService->getSomething({}));
    CPPUNIT_ASSERT(pFactory != nullptr);
}

void SwDialogsTest2::registerKnownDialogsByID(mapType& /*rKnownDialogs*/)
{
    // fill map of known dialogs
}

VclPtr<VclAbstractDialog> SwDialogsTest2::createDialogByID(sal_uInt32 /*nID*/)
{
    return nullptr;
}

void SwDialogsTest2::openAnyDialog()
{
    /// process input file containing the UXMLDescriptions of the dialogs to dump
    processDialogBatchFile(u"sw/qa/unit/data/sw-dialogs-test_2.txt");
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwDialogsTest2);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
