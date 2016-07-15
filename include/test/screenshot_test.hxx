/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_TEST_SCREENSHOT_TEST_HXX
#define INCLUDED_TEST_SCREENSHOT_TEST_HXX

#include <sal/config.h>
#include <test/bootstrapfixture.hxx>
#include <unotest/macros_test.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <osl/file.hxx>
#include <vcl/dialog.hxx>

class VclAbstractDialog;


class OOO_DLLPUBLIC_TEST ScreenshotTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    ScreenshotTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    /// version for AbstractDialogs, the ones created in AbstractDialogFactories
    void dumpDialogToPath(VclAbstractDialog& rDialog);

    /// version for pure vcl-based dialogs
    void dumpDialogToPath(Dialog& rDialog);

    /// fallback version for dialogs for which only the UXMLDescription is known.
    /// This should be used with care - no active layouting will be done, only the
    /// VclBuilder will be activated for layouting. Result can thus vary drastically
    /// compared to the active dialog (can be compared with dialog previewer)
    void dumpDialogToPath(const OString& rUIXMLDescription);

private:
    void implSaveScreenshot(const Bitmap& rScreenshot, const OString& rScreenshotId);
    void saveScreenshot(VclAbstractDialog& rDialog);
    void saveScreenshot(Dialog& rDialog);

    OUString m_aScreenshotDirectory;
};

#endif // INCLUDED_TEST_SCREENSHOT_TEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
