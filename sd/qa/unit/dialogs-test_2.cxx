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
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/drawing/XDrawPagesSupplier.hpp>
#include <com/sun/star/presentation/AnimationEffect.hpp>
#include <com/sun/star/presentation/ClickAction.hpp>

#include <sfx2/app.hxx>
#include <sfx2/docfilt.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxmodelfactory.hxx>
#include <svl/stritem.hxx>
#include <svl/aeitem.hxx>
#include <editeng/colritem.hxx>
#include <svx/svdtext.hxx>
#include <svx/svdotext.hxx>
#include <vcl/sound.hxx>

#include "drawdoc.hxx"
#include "DrawDocShell.hxx"

#include <osl/process.h>
#include <osl/thread.h>
#include <osl/file.hxx>

#include "sdabstdlg.hxx"
#include <vcl/pngwrite.hxx>
#include "unomodel.hxx"
#include "ViewShell.hxx"
#include "Window.hxx"
#include "drawview.hxx"
#include "DrawViewShell.hxx"
#include <app.hrc>
#include "stlsheet.hxx"
#include "sdattr.hrc"
#include "strings.hrc"
#include "sdresid.hxx"
#include "sdattr.hxx"
#include "prltempl.hrc"

#include <comphelper/servicehelper.hxx>
#include <com/sun/star/frame/Desktop.hpp>
#include <com/sun/star/frame/XDesktop2.hpp>
#include <comphelper/processfactory.hxx>
#include <unotest/macros_test.hxx>

using namespace ::com::sun::star;

/// Test opening a dialog in sd
class SdDialogsTest2 : public ScreenshotTest
{
private:
    /// helper method to populate KnownDialogs, called in setUp(). Needs to be
    /// written and has to add entries to KnownDialogs
    virtual void registerKnownDialogsByID(mapType& rKnownDialogs) override;

    /// dialog creation for known dialogs by ID. Has to be implemented for
    /// each registered known dialog
    virtual VclPtr<VclAbstractDialog> createDialogByID(sal_uInt32 nID) override;

public:
    SdDialogsTest2();
    virtual ~SdDialogsTest2() override;

    // try to open a dialog
    void openAnyDialog();

    CPPUNIT_TEST_SUITE(SdDialogsTest2);
    CPPUNIT_TEST(openAnyDialog);
    CPPUNIT_TEST_SUITE_END();
};

SdDialogsTest2::SdDialogsTest2()
{
}

SdDialogsTest2::~SdDialogsTest2()
{
}

void SdDialogsTest2::registerKnownDialogsByID(mapType& /* rKnownDialogs */)
{
    // fill map of known dialogs
}

VclPtr<VclAbstractDialog> SdDialogsTest2::createDialogByID(sal_uInt32 /* nID */)
{
    return nullptr;
}

void SdDialogsTest2::openAnyDialog()
{

    processDialogBatchFile("sd/qa/unit/data/dialogs-test_2.txt");
}

CPPUNIT_TEST_SUITE_REGISTRATION(SdDialogsTest2);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
