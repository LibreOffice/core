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

class VclAbstractDialog;


class OOO_DLLPUBLIC_TEST ScreenshotTest : public test::BootstrapFixture, public unotest::MacrosTest
{
public:
    ScreenshotTest();

    virtual void setUp() override;
    virtual void tearDown() override;

    void dumpDialogToPath( VclAbstractDialog& rDialog );

private:
    void saveScreenshot( VclAbstractDialog& rDialog );
    OUString m_aScreenshotDirectory;
};

#endif // INCLUDED_TEST_SCREENSHOT_TEST_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
