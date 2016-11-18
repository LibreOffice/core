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
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <sfx2/app.hxx>
#include <vcl/abstdlg.hxx>

#include <swabstdlg.hxx>

using namespace ::com::sun::star;

extern "C" { using Fn = SwAbstractDialogFactory * (*)(); }
    // sw/source/ui/dialog/swuiexp.cxx

/// Test opening a dialog in sw
class SwDialogsTest : public ScreenshotTest
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
    SwDialogsTest();
    virtual ~SwDialogsTest() override;

    void setUp() override;

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
    // Make sure the swui library's global pSwResMgr is initialized
    // (alternatively to dynamically loading the library, SwCreateDialogFactory
    // could be declared in an include file and this CppunitTest link against
    // the swui library):
    OUString url("${LO_LIB_DIR}/" SVLIBRARY("swui"));
    rtl::Bootstrap::expandMacros(url); //TODO: detect failure
    CPPUNIT_ASSERT(libSwui_.load(url, SAL_LOADMODULE_GLOBAL));
    auto fn = reinterpret_cast<Fn>(
        libSwui_.getFunctionSymbol("SwCreateDialogFactory"));
    CPPUNIT_ASSERT(fn != nullptr);
    (*fn)();
}

void SwDialogsTest::registerKnownDialogsByID(mapType& /*rKnownDialogs*/)
{
    // fill map of known dialogs
}

VclPtr<VclAbstractDialog> SwDialogsTest::createDialogByID(sal_uInt32 /*nID*/)
{
    return nullptr;
}

void SwDialogsTest::openAnyDialog()
{
    /// process input file containing the UXMLDescriptions of the dialogs to dump
    processDialogBatchFile("sw/qa/unit/data/sw-dialogs-test.txt");
}

CPPUNIT_TEST_SUITE_REGISTRATION(SwDialogsTest);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
