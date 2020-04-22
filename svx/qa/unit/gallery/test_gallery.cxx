/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include <svx/gallery1.hxx>
#include <unotools/tempfile.hxx>
#include <comphelper/DirectoryHelper.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

class GalleryObjTest : public CppUnit::TestFixture
{
public:
    void TestCreateTheme();
    void TestDeleteTheme();
    void TestSetThemeName();
    void TestThemeURLCase();

    CPPUNIT_TEST_SUITE(GalleryObjTest);

    CPPUNIT_TEST(TestCreateTheme);
    CPPUNIT_TEST(TestDeleteTheme);
    CPPUNIT_TEST(TestSetThemeName);
    CPPUNIT_TEST(TestThemeURLCase);

    CPPUNIT_TEST_SUITE_END();
};

// Create and Dereference a theme, check that file exists
void GalleryObjTest::TestCreateTheme()
{
    // Create theme
    std::unique_ptr<utl::TempFile> pTempDir;
    pTempDir.reset(new utl::TempFile(nullptr, true));
    CPPUNIT_ASSERT_MESSAGE("Could not create valid temporary directory", pTempDir->IsValid());
    pTempDir->EnableKillingFile();
    const OUString aGalleryURL = pTempDir->GetURL();

    // Check if directory exists
    CPPUNIT_ASSERT_MESSAGE("Could not create temporary directory",
                           comphelper::DirectoryHelper::dirExists(aGalleryURL));

    std::unique_ptr<Gallery> pGallery(new Gallery(aGalleryURL));
    CPPUNIT_ASSERT_MESSAGE("Could not create gallery instance", (pGallery != nullptr));
    const OUString myThemeName = "addytesttheme";
    CPPUNIT_ASSERT_MESSAGE("Could not create theme", pGallery->CreateTheme(myThemeName));
    CPPUNIT_ASSERT_MESSAGE("Could not find theme", pGallery->HasTheme(myThemeName));

    // check if files exist
    CPPUNIT_ASSERT_MESSAGE(
        "Could not find .thm file inside it",
        comphelper::DirectoryHelper::fileExists(aGalleryURL + "/" + myThemeName + ".thm"));
    CPPUNIT_ASSERT_MESSAGE(
        "Could not find .sdv file inside it",
        comphelper::DirectoryHelper::fileExists(aGalleryURL + "/" + myThemeName + ".sdv"));
}

//  Create and Delete Theme, check the file doesn't exist
void GalleryObjTest::TestDeleteTheme()
{
    // Create theme
    std::unique_ptr<utl::TempFile> pTempDir;
    pTempDir.reset(new utl::TempFile(nullptr, true));
    CPPUNIT_ASSERT_MESSAGE("Could not create valid temporary directory", pTempDir->IsValid());
    pTempDir->EnableKillingFile();
    const OUString aGalleryURL = pTempDir->GetURL();

    std::unique_ptr<Gallery> pGallery(new Gallery(aGalleryURL));
    CPPUNIT_ASSERT_MESSAGE("Could not create gallery instance", (pGallery != nullptr));
    const OUString myThemeName = "addytesttheme";
    CPPUNIT_ASSERT_MESSAGE("Could not create theme", pGallery->CreateTheme(myThemeName));
    CPPUNIT_ASSERT_MESSAGE("Could not find theme", pGallery->HasTheme(myThemeName));

    // Delete Theme
    CPPUNIT_ASSERT_MESSAGE("Could not remove theme", pGallery->RemoveTheme(myThemeName));
    CPPUNIT_ASSERT_MESSAGE("Could not remove theme, theme found even after trying to remove",
                           !pGallery->HasTheme(myThemeName));

    // Check that files do not exist
    CPPUNIT_ASSERT_MESSAGE(
        "Found .thm file inside it after deletion",
        !comphelper::DirectoryHelper::fileExists(aGalleryURL + "/" + myThemeName + ".thm"));
    CPPUNIT_ASSERT_MESSAGE(
        "Found .sdv file inside it after deletion",
        !comphelper::DirectoryHelper::fileExists(aGalleryURL + "/" + myThemeName + ".sdv"));
    CPPUNIT_ASSERT_MESSAGE(
        "Found .sdg file inside it after deletion",
        !comphelper::DirectoryHelper::fileExists(aGalleryURL + "/" + myThemeName + ".sdg"));
    CPPUNIT_ASSERT_MESSAGE(
        "Found .str file inside it after deletion",
        !comphelper::DirectoryHelper::fileExists(aGalleryURL + "/" + myThemeName + ".str"));
}

// Create theme, set name, assert the name is expected
void GalleryObjTest::TestSetThemeName()
{
    // Create theme
    std::unique_ptr<utl::TempFile> pTempDir;
    pTempDir.reset(new utl::TempFile(nullptr, true));
    CPPUNIT_ASSERT_MESSAGE("Could not create valid temporary directory", pTempDir->IsValid());
    pTempDir->EnableKillingFile();
    const OUString aGalleryURL = pTempDir->GetURL();

    std::unique_ptr<Gallery> pGallery(new Gallery(aGalleryURL));
    CPPUNIT_ASSERT_MESSAGE("Could not create gallery instance", (pGallery != nullptr));
    const OUString myThemeName = "addytesttheme";
    CPPUNIT_ASSERT_MESSAGE("Could not create theme", pGallery->CreateTheme(myThemeName));
    CPPUNIT_ASSERT_MESSAGE("Could not find theme", pGallery->HasTheme(myThemeName));

    // Rename theme
    const OUString myNewThemeName = "addytestthemenew";
    pGallery->RenameTheme(myThemeName, myNewThemeName);
    CPPUNIT_ASSERT_MESSAGE("Could not rename theme because old theme name still exists",
                           !pGallery->HasTheme(myThemeName));
    CPPUNIT_ASSERT_MESSAGE("Could not find new renamed theme", pGallery->HasTheme(myNewThemeName));

    // Check that files are not renamed
    CPPUNIT_ASSERT_MESSAGE(
        "Could not find .thm file inside it",
        comphelper::DirectoryHelper::fileExists(aGalleryURL + "/" + myThemeName + ".thm"));
    CPPUNIT_ASSERT_MESSAGE(
        "Could not find .sdv file inside it",
        comphelper::DirectoryHelper::fileExists(aGalleryURL + "/" + myThemeName + ".sdv"));
}

void GalleryObjTest::TestThemeURLCase()
{
    // Create theme
    std::unique_ptr<utl::TempFile> pTempDir;
    pTempDir.reset(new utl::TempFile(nullptr, true));
    CPPUNIT_ASSERT_MESSAGE("Could not create valid temporary directory", pTempDir->IsValid());
    pTempDir->EnableKillingFile();
    const OUString aGalleryURL = pTempDir->GetURL();

    // Check if directory exists
    CPPUNIT_ASSERT_MESSAGE("Could not create temporary directory",
                           comphelper::DirectoryHelper::dirExists(aGalleryURL));

    std::unique_ptr<Gallery> pGallery(new Gallery(aGalleryURL));
    CPPUNIT_ASSERT_MESSAGE("Could not create gallery instance", (pGallery != nullptr));

    // Mixed Case Theme Name
    const OUString myThemeName = "AddyTestTheme";

    CPPUNIT_ASSERT_MESSAGE("Could not create theme", pGallery->CreateTheme(myThemeName));
    CPPUNIT_ASSERT_MESSAGE("Could not find theme", pGallery->HasTheme(myThemeName));

#if defined(LINUX)
    CPPUNIT_ASSERT_MESSAGE("[LINUX] Could not find .thm in lowercase",
                           comphelper::DirectoryHelper::fileExists(
                               aGalleryURL + "/" + myThemeName.toAsciiLowerCase() + ".thm"));
    CPPUNIT_ASSERT_MESSAGE("[LINUX] Could not find .sdv in lowercase",
                           comphelper::DirectoryHelper::fileExists(
                               aGalleryURL + "/" + myThemeName.toAsciiLowerCase() + ".sdv"));
#else
    CPPUNIT_ASSERT_MESSAGE(
        "[WINDOWS] Could not find .thm in mixed case",
        comphelper::DirectoryHelper::fileExists(aGalleryURL + "/" + myThemeName + ".thm"));
    CPPUNIT_ASSERT_MESSAGE(
        "[WINDOWS] Could not find .sdv in mixed case",
        comphelper::DirectoryHelper::fileExists(aGalleryURL + "/" + myThemeName + ".sdv"));
#endif
}

CPPUNIT_TEST_SUITE_REGISTRATION(GalleryObjTest);

CPPUNIT_PLUGIN_IMPLEMENT();
