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
#include <map>

class VclAbstractDialog;
typedef std::map< OString, sal_uInt32 > mapType;

class OOO_DLLPUBLIC_TEST ScreenshotTest : public test::BootstrapFixture, public unotest::MacrosTest
{
private:
    /// the target directory for screenshots
    OUString    m_aScreenshotDirectory;

    /// the set of known dialogs and their ID for usage in createDialogByID
    mapType     maKnownDialogs;

private:
    /// helpers
    void implSaveScreenshot(const Bitmap& rScreenshot, const OString& rScreenshotId);
    void saveScreenshot(VclAbstractDialog& rDialog);
    void saveScreenshot(Dialog& rDialog);

    /// helper method to populate maKnownDialogs, called in setUp(). Needs to be
    /// written and has to add entries to maKnownDialogs
    virtual void registerKnownDialogsByID(mapType& rKnownDialogs) = 0;

    /// dialog creation for known dialogs by ID. Has to be implemented for
    /// each registered known dialog
    virtual VclAbstractDialog* createDialogByID(sal_uInt32 nID) = 0;

public:
    ScreenshotTest();
    virtual ~ScreenshotTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    /// Dialog creation for known dialogs by Name (path and UIXMLDescription, *.ui file).
    /// This uses maKnownDialogs to check if known, and if so, calls createDialogByID
    /// with the ID from the map
    VclAbstractDialog* createDialogByName(const OString& rName);

    /// version for AbstractDialogs, the ones created in AbstractDialogFactories
    void dumpDialogToPath(VclAbstractDialog& rDialog);

    /// version for pure vcl-based dialogs
    void dumpDialogToPath(Dialog& rDialog);

    /// fallback version for dialogs for which only the UXMLDescription is known.
    /// This should be used with care - no active layouting will be done, only the
    /// VclBuilder will be activated for layouting. Result can thus vary drastically
    /// compared to the active dialog (can be compared with dialog previewer)
    void dumpDialogToPath(const OString& rUIXMLDescription);

    /// const access to known dialogs
    const mapType& getKnownDialogs() const { return maKnownDialogs; }
};

#endif // INCLUDED_TEST_SCREENSHOT_TEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
