/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/types.h>

#include <rtl/ustring.hxx>
#include <vcl/IconThemeScanner.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

using rtl::OUString;

class IconThemeScannerTest : public CppUnit::TestFixture
{
private:

    void
    StyleNameIsDetectedFromFileNameWithUnderscore();

    void
    StyleNameIsDetectedFromFileNameWithoutUnderscore();

    void
    AddingThemesByPathWorks();

    // Adds code needed to register the test suite
    CPPUNIT_TEST_SUITE(IconThemeScannerTest);
    CPPUNIT_TEST(StyleNameIsDetectedFromFileNameWithUnderscore);
    CPPUNIT_TEST(StyleNameIsDetectedFromFileNameWithoutUnderscore);
    CPPUNIT_TEST(AddingThemesByPathWorks);
    // End of test suite definition
    CPPUNIT_TEST_SUITE_END();
};

void
IconThemeScannerTest::StyleNameIsDetectedFromFileNameWithUnderscore()
{
    OUString fname("images_oxygen.zip");
    OUString sname = vcl::IconThemeScanner::FileNameToIconTheme(fname);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'oxygen' style name is returned for 'images_oxygen.zip'", OUString("oxygen"), sname);
}

void
IconThemeScannerTest::StyleNameIsDetectedFromFileNameWithoutUnderscore()
{
    OUString fname("default.zip");
    OUString sname = vcl::IconThemeScanner::FileNameToIconTheme(fname);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'default' style name is returned for 'default.zip'", OUString("default"), sname);
}

void
IconThemeScannerTest::AddingThemesByPathWorks()
{
    vcl::IconThemeScanner Scanner;
    OUString theme("file:://images_katze.zip");
    Scanner.AddIconThemeByPath(theme);
    bool found = false;
    found = Scanner.IconThemeIsInstalled("katze");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'katze' icon theme is found from url", found, true);
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(IconThemeScannerTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
