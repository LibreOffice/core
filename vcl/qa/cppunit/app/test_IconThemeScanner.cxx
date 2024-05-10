/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdexcept>

#include <rtl/ustring.hxx>
#include <IconThemeScanner.hxx>
#include <vcl/IconThemeInfo.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class IconThemeScannerTest : public CppUnit::TestFixture
{
    void
    AddedThemeIsFoundById();

    void
    AddedThemeInfoIsReturned();

    void
    ExceptionIsThrownIfInvalidInfoIsRequested();

    // Adds code needed to register the test suite
    CPPUNIT_TEST_SUITE(IconThemeScannerTest);
    CPPUNIT_TEST(AddedThemeIsFoundById);
    CPPUNIT_TEST(AddedThemeInfoIsReturned);
    CPPUNIT_TEST(ExceptionIsThrownIfInvalidInfoIsRequested);

    // End of test suite definition
    CPPUNIT_TEST_SUITE_END();
};

void
IconThemeScannerTest::AddedThemeIsFoundById()
{
    vcl::IconThemeScanner scanner;
    scanner.AddIconThemeByPath(u"file:://images_katze.zip"_ustr);
    OUString id = vcl::IconThemeInfo::FileNameToThemeId(u"images_katze.zip");
    bool found = scanner.IconThemeIsInstalled(id);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("icon theme could be added by url", true, found);
}

void
IconThemeScannerTest::AddedThemeInfoIsReturned()
{
    vcl::IconThemeScanner scanner;
    OUString theme(u"file:://images_katze.zip"_ustr);
    scanner.AddIconThemeByPath(theme);
    OUString id = vcl::IconThemeInfo::FileNameToThemeId(u"images_katze.zip");
    const vcl::IconThemeInfo& info = scanner.GetIconThemeInfo(id);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'katze' icon theme is found from id", theme, info.GetUrlToFile());
}

void
IconThemeScannerTest::ExceptionIsThrownIfInvalidInfoIsRequested()
{
    vcl::IconThemeScanner scanner;
    scanner.AddIconThemeByPath(u"file:://images_katze.zip"_ustr);
    bool thrown = false;
    try
    {
        scanner.GetIconThemeInfo(u"hund"_ustr);
    }
    catch (const std::runtime_error&)
    {
        thrown = true;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Exception is thrown if invalid theme info is requested", true, thrown);
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(IconThemeScannerTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
