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
#include <vcl/IconThemeInfo.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace vcl;

class IconThemeInfoTest : public CppUnit::TestFixture
{
    void
    UpperCaseDisplayNameIsReturnedForNonDefaultId();

    void
    ImagesZipIsNotValid();

    void
    ImagesColibreZipIsValid();

    void
    ThemeIdIsDetectedFromFileNameWithUnderscore();

    void
    ExceptionIsThrownWhenIdCannotBeDetermined1();

    void
    ExceptionIsThrownWhenIdCannotBeDetermined2();

    // Adds code needed to register the test suite
    CPPUNIT_TEST_SUITE(IconThemeInfoTest);
    CPPUNIT_TEST(UpperCaseDisplayNameIsReturnedForNonDefaultId);
    CPPUNIT_TEST(ThemeIdIsDetectedFromFileNameWithUnderscore);
    CPPUNIT_TEST(ImagesZipIsNotValid);
    CPPUNIT_TEST(ImagesColibreZipIsValid);
    CPPUNIT_TEST(ExceptionIsThrownWhenIdCannotBeDetermined1);
    CPPUNIT_TEST(ExceptionIsThrownWhenIdCannotBeDetermined2);

    // End of test suite definition
    CPPUNIT_TEST_SUITE_END();
};

void
IconThemeInfoTest::UpperCaseDisplayNameIsReturnedForNonDefaultId()
{
    OUString displayName = vcl::IconThemeInfo::ThemeIdToDisplayName(u"katze"_ustr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("theme id is properly uppercased", u"Katze"_ustr, displayName);
}

void
IconThemeInfoTest::ImagesZipIsNotValid()
{
    bool valid = vcl::IconThemeInfo::UrlCanBeParsed(u"file://images.zip");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("images.zip is not a valid theme name", false, valid);
}

void
IconThemeInfoTest::ImagesColibreZipIsValid()
{
    bool valid = vcl::IconThemeInfo::UrlCanBeParsed(u"file://images_colibre.zip");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("images_colibre.zip is a valid theme name", true, valid);
}

void
IconThemeInfoTest::ThemeIdIsDetectedFromFileNameWithUnderscore()
{
    OUString sname = vcl::IconThemeInfo::FileNameToThemeId(u"images_colibre.zip");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("'colibre' theme id is returned for 'images_colibre.zip'", u"colibre"_ustr, sname);
}

void
IconThemeInfoTest::ExceptionIsThrownWhenIdCannotBeDetermined1()
{
    bool thrown = false;
    try {
        vcl::IconThemeInfo::FileNameToThemeId(u"images_colibre");
    }
    catch (std::runtime_error&) {
        thrown = true;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Exception was thrown",true, thrown);
}

void
IconThemeInfoTest::ExceptionIsThrownWhenIdCannotBeDetermined2()
{
    bool thrown = false;
    try {
        vcl::IconThemeInfo::FileNameToThemeId(u"image_colibre.zip");
    }
    catch (std::runtime_error&) {
        thrown = true;
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Exception was thrown", true, thrown);
}

// Put the test suite in the registry
CPPUNIT_TEST_SUITE_REGISTRATION(IconThemeInfoTest);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
