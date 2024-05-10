/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/
#include <test/bootstrapfixture.hxx>

#include <tools/urlobj.hxx>
#include <sfx2/app.hxx>
#include <unotools/tempfile.hxx>
#include <comphelper/DirectoryHelper.hxx>

#include <svx/gallery1.hxx>
#include <svx/galtheme.hxx>
#include <gallerystoragelocations.hxx>
#include <galobj.hxx>

#include <cppunit/TestAssert.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

class GalleryObjTest : public test::BootstrapFixture
{
public:
    void TestCreateTheme();
    void TestDeleteTheme();
    void TestSetThemeName();
    void TestThemeURLCase();
    void TestThemeCount();
    void TestGalleryThemeEntry();
    void TestInsertGalleryObject();
    void TestRemoveGalleryObject();
    void TestChangePositionGalleryObject();
    void TestGetThemeNameFromGalleryTheme();

    CPPUNIT_TEST_SUITE(GalleryObjTest);

    CPPUNIT_TEST(TestCreateTheme);
    CPPUNIT_TEST(TestDeleteTheme);
    CPPUNIT_TEST(TestSetThemeName);
    CPPUNIT_TEST(TestThemeURLCase);
    CPPUNIT_TEST(TestThemeCount);
    CPPUNIT_TEST(TestGalleryThemeEntry);
    CPPUNIT_TEST(TestInsertGalleryObject);
    CPPUNIT_TEST(TestRemoveGalleryObject);
    CPPUNIT_TEST(TestChangePositionGalleryObject);
    CPPUNIT_TEST(TestGetThemeNameFromGalleryTheme);

    CPPUNIT_TEST_SUITE_END();
};

// Create and Dereference a theme, check that file exists
void GalleryObjTest::TestCreateTheme()
{
    // Create theme
    std::unique_ptr<utl::TempFileNamed> pTempDir;
    pTempDir.reset(new utl::TempFileNamed(nullptr, true));
    CPPUNIT_ASSERT_MESSAGE("Could not create valid temporary directory", pTempDir->IsValid());
    pTempDir->EnableKillingFile();
    const OUString aGalleryURL = pTempDir->GetURL();

    // Check if directory exists
    CPPUNIT_ASSERT_MESSAGE("Could not create temporary directory",
                           comphelper::DirectoryHelper::dirExists(aGalleryURL));

    std::unique_ptr<Gallery> pGallery(new Gallery(aGalleryURL));
    CPPUNIT_ASSERT_MESSAGE("Could not create gallery instance", (pGallery != nullptr));
    static constexpr OUString myThemeName = u"addytesttheme"_ustr;
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
    std::unique_ptr<utl::TempFileNamed> pTempDir;
    pTempDir.reset(new utl::TempFileNamed(nullptr, true));
    CPPUNIT_ASSERT_MESSAGE("Could not create valid temporary directory", pTempDir->IsValid());
    pTempDir->EnableKillingFile();
    const OUString aGalleryURL = pTempDir->GetURL();

    std::unique_ptr<Gallery> pGallery(new Gallery(aGalleryURL));
    CPPUNIT_ASSERT_MESSAGE("Could not create gallery instance", (pGallery != nullptr));
    static constexpr OUString myThemeName = u"addytesttheme"_ustr;
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
    std::unique_ptr<utl::TempFileNamed> pTempDir;
    pTempDir.reset(new utl::TempFileNamed(nullptr, true));
    CPPUNIT_ASSERT_MESSAGE("Could not create valid temporary directory", pTempDir->IsValid());
    pTempDir->EnableKillingFile();
    const OUString aGalleryURL = pTempDir->GetURL();

    std::unique_ptr<Gallery> pGallery(new Gallery(aGalleryURL));
    CPPUNIT_ASSERT_MESSAGE("Could not create gallery instance", (pGallery != nullptr));
    static constexpr OUString myThemeName = u"addytesttheme"_ustr;
    CPPUNIT_ASSERT_MESSAGE("Could not create theme", pGallery->CreateTheme(myThemeName));
    CPPUNIT_ASSERT_MESSAGE("Could not find theme", pGallery->HasTheme(myThemeName));

    // Rename theme
    static constexpr OUString myNewThemeName = u"addytestthemenew"_ustr;
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
    std::unique_ptr<utl::TempFileNamed> pTempDir;
    pTempDir.reset(new utl::TempFileNamed(nullptr, true));
    CPPUNIT_ASSERT_MESSAGE("Could not create valid temporary directory", pTempDir->IsValid());
    pTempDir->EnableKillingFile();
    const OUString aGalleryURL = pTempDir->GetURL();

    // Check if directory exists
    CPPUNIT_ASSERT_MESSAGE("Could not create temporary directory",
                           comphelper::DirectoryHelper::dirExists(aGalleryURL));

    std::unique_ptr<Gallery> pGallery(new Gallery(aGalleryURL));
    CPPUNIT_ASSERT_MESSAGE("Could not create gallery instance", (pGallery != nullptr));

    // Mixed Case Theme Name
    constexpr OUString myThemeName = u"AddyTestTheme"_ustr;

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

void GalleryObjTest::TestThemeCount()
{
    std::unique_ptr<utl::TempFileNamed> pTempDir;
    pTempDir.reset(new utl::TempFileNamed(nullptr, true));
    CPPUNIT_ASSERT_MESSAGE("Could not create valid temporary directory", pTempDir->IsValid());
    pTempDir->EnableKillingFile();
    const OUString aGalleryURL = pTempDir->GetURL();

    // Check if directory exists
    CPPUNIT_ASSERT_MESSAGE("Could not create temporary directory",
                           comphelper::DirectoryHelper::dirExists(aGalleryURL));

    std::unique_ptr<Gallery> pGallery(new Gallery(aGalleryURL));
    CPPUNIT_ASSERT_MESSAGE("Could not create gallery instance", (pGallery != nullptr));

    // Loop through and test theme count in each pass.
    sal_uInt32 nLimit = 10;
    for (sal_uInt32 i = 1; i <= nLimit; i++)
    {
        OUString myThemeName = "addytesttheme" + OUString::number(i);
        // Create theme
        CPPUNIT_ASSERT_MESSAGE("Could not create theme", pGallery->CreateTheme(myThemeName));
        CPPUNIT_ASSERT_MESSAGE("Could not find theme", pGallery->HasTheme(myThemeName));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Inconsistent theme count",
                                     static_cast<sal_uInt32>(pGallery->GetThemeCount()), i);
    }
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Inconsistent theme count",
                                 static_cast<sal_uInt32>(pGallery->GetThemeCount()), nLimit);
    for (sal_uInt32 i = nLimit; i > 0; i--)
    {
        OUString myThemeName = "addytesttheme" + OUString::number(i);
        // Delete Theme
        CPPUNIT_ASSERT_MESSAGE("Could not remove theme", pGallery->RemoveTheme(myThemeName));
        CPPUNIT_ASSERT_MESSAGE("Could not remove theme, theme found even after trying to remove",
                               !pGallery->HasTheme(myThemeName));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Inconsistent theme count",
                                     static_cast<sal_uInt32>(pGallery->GetThemeCount()), i - 1);
    }
}

void GalleryObjTest::TestGalleryThemeEntry()
{
    // Create theme
    std::unique_ptr<utl::TempFileNamed> pTempDir;
    pTempDir.reset(new utl::TempFileNamed(nullptr, true));
    CPPUNIT_ASSERT_MESSAGE("Could not create valid temporary directory", pTempDir->IsValid());
    pTempDir->EnableKillingFile();
    const OUString aGalleryURL = pTempDir->GetURL();

    // Check if directory exists
    CPPUNIT_ASSERT_MESSAGE("Could not create temporary directory",
                           comphelper::DirectoryHelper::dirExists(aGalleryURL));

    std::unique_ptr<Gallery> pGallery(new Gallery(aGalleryURL));
    CPPUNIT_ASSERT_MESSAGE("Could not create gallery instance", (pGallery != nullptr));
    constexpr OUString myThemeName = u"addytesttheme"_ustr;
    CPPUNIT_ASSERT_MESSAGE("Could not create theme", pGallery->CreateTheme(myThemeName));
    CPPUNIT_ASSERT_MESSAGE("Could not find theme", pGallery->HasTheme(myThemeName));

    // Get Theme Entry Object
    const GalleryThemeEntry* mpThemeEntry = pGallery->GetThemeInfo(myThemeName);

    // Check Theme Name
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Theme name doesn't match", myThemeName,
                                 mpThemeEntry->GetThemeName());

    // Check URLs
    GalleryStorageLocations& rGalleryStorageLocations = mpThemeEntry->getGalleryStorageLocations();
    INetURLObject aURL(aGalleryURL);
    aURL.Append(myThemeName);
    INetURLObject aThemeURL(aURL), aSdvURL(aURL), aSdgURL(aURL), aStrURL(aURL);
    aThemeURL.setExtension(u"thm");
    aSdvURL.setExtension(u"sdv");
    aSdgURL.setExtension(u"sdg");
    aStrURL.setExtension(u"str");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Theme URL doesn't match",
                                 rGalleryStorageLocations.GetThmURL().GetMainURL(
                                     INetURLObject::DecodeMechanism::Unambiguous),
                                 aThemeURL.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sdv URL doesn't match",
                                 rGalleryStorageLocations.GetSdvURL().GetMainURL(
                                     INetURLObject::DecodeMechanism::Unambiguous),
                                 aSdvURL.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sdg URL doesn't match",
                                 rGalleryStorageLocations.GetSdgURL().GetMainURL(
                                     INetURLObject::DecodeMechanism::Unambiguous),
                                 aSdgURL.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Str URL doesn't match",
                                 rGalleryStorageLocations.GetStrURL().GetMainURL(
                                     INetURLObject::DecodeMechanism::Unambiguous),
                                 aStrURL.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous));
}

void GalleryObjTest::TestInsertGalleryObject()
{
    // Create theme
    std::unique_ptr<utl::TempFileNamed> pTempDir;
    pTempDir.reset(new utl::TempFileNamed(nullptr, true));
    CPPUNIT_ASSERT_MESSAGE("Could not create valid temporary directory", pTempDir->IsValid());
    pTempDir->EnableKillingFile();
    const OUString aGalleryURL = pTempDir->GetURL();

    // Check if directory exists
    CPPUNIT_ASSERT_MESSAGE("Could not create temporary directory",
                           comphelper::DirectoryHelper::dirExists(aGalleryURL));

    std::unique_ptr<Gallery> pGallery(new Gallery(aGalleryURL));
    CPPUNIT_ASSERT_MESSAGE("Could not create gallery instance", (pGallery != nullptr));
    static constexpr OUString myThemeName = u"addytesttheme"_ustr;
    CPPUNIT_ASSERT_MESSAGE("Could not create theme", pGallery->CreateTheme(myThemeName));
    CPPUNIT_ASSERT_MESSAGE("Could not find theme", pGallery->HasTheme(myThemeName));

    // Create Sfx Instance
    SfxListener aListener;
    SfxApplication::GetOrCreate();

    // Insert Objects Into Theme
    GalleryTheme* pGalleryTheme = pGallery->AcquireTheme(myThemeName, aListener);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Object count inconsistent", sal_uInt32(0),
                                 pGalleryTheme->GetObjectCount());

    std::vector<OUString> imageList{ u"galtest1.png"_ustr, u"galtest2.png"_ustr,
                                     u"galtest3.jpg"_ustr };

    for (sal_uInt32 i = 0; i < static_cast<sal_uInt32>(imageList.size()); i++)
    {
        OUString imageNameFromList(imageList[i]);
        OUString aURL(m_directories.getURLFromSrc(u"/svx/qa/unit/gallery/data/")
                      + imageNameFromList);
        CPPUNIT_ASSERT_MESSAGE("Could not insert object into theme",
                               pGalleryTheme->InsertURL(INetURLObject(aURL)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Inconsistent object Count", pGalleryTheme->GetObjectCount(),
                                     i + 1);
        std::unique_ptr<SgaObject> pObj = pGalleryTheme->AcquireObject(i);
        CPPUNIT_ASSERT_MESSAGE("Acquired Object Invalid", pObj->IsValid());
    }
    pGallery->ReleaseTheme(pGalleryTheme, aListener);
}

void GalleryObjTest::TestRemoveGalleryObject()
{
    // Create theme
    std::unique_ptr<utl::TempFileNamed> pTempDir;
    pTempDir.reset(new utl::TempFileNamed(nullptr, true));
    CPPUNIT_ASSERT_MESSAGE("Could not create valid temporary directory", pTempDir->IsValid());
    pTempDir->EnableKillingFile();
    const OUString aGalleryURL = pTempDir->GetURL();

    // Check if directory exists
    CPPUNIT_ASSERT_MESSAGE("Could not create temporary directory",
                           comphelper::DirectoryHelper::dirExists(aGalleryURL));

    std::unique_ptr<Gallery> pGallery(new Gallery(aGalleryURL));
    CPPUNIT_ASSERT_MESSAGE("Could not create gallery instance", (pGallery != nullptr));
    static constexpr OUString myThemeName = u"addytesttheme"_ustr;
    CPPUNIT_ASSERT_MESSAGE("Could not create theme", pGallery->CreateTheme(myThemeName));
    CPPUNIT_ASSERT_MESSAGE("Could not find theme", pGallery->HasTheme(myThemeName));

    // Create Sfx Instance
    SfxListener aListener;
    SfxApplication::GetOrCreate();

    // Insert Objects Into Theme
    GalleryTheme* pGalleryTheme = pGallery->AcquireTheme(myThemeName, aListener);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Object count inconsistent", sal_uInt32(0),
                                 pGalleryTheme->GetObjectCount());

    std::vector<OUString> imageList{ u"galtest1.png"_ustr, u"galtest2.png"_ustr,
                                     u"galtest3.jpg"_ustr };

    for (sal_uInt32 i = 0; i < static_cast<sal_uInt32>(imageList.size()); i++)
    {
        OUString imageNameFromList(imageList[i]);
        OUString aURL(m_directories.getURLFromSrc(u"/svx/qa/unit/gallery/data/")
                      + imageNameFromList);
        CPPUNIT_ASSERT_MESSAGE("Could not insert object into theme",
                               pGalleryTheme->InsertURL(INetURLObject(aURL)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Inconsistent object Count", pGalleryTheme->GetObjectCount(),
                                     i + 1);
        std::unique_ptr<SgaObject> pObj = pGalleryTheme->AcquireObject(i);
        CPPUNIT_ASSERT_MESSAGE("Acquired Object Invalid", pObj->IsValid());
    }

    for (sal_uInt32 i = static_cast<sal_uInt32>(imageList.size()); i > 0; i--)
    {
        std::unique_ptr<SgaObject> pObj = pGalleryTheme->AcquireObject(i - 1);
        CPPUNIT_ASSERT_MESSAGE("Acquired Object Invalid", pObj->IsValid());
        pGalleryTheme->RemoveObject(i - 1);
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Inconsistent object Count", pGalleryTheme->GetObjectCount(),
                                     i - 1);
    }

    pGallery->ReleaseTheme(pGalleryTheme, aListener);
}

void GalleryObjTest::TestChangePositionGalleryObject()
{
    // Create theme
    std::unique_ptr<utl::TempFileNamed> pTempDir;
    pTempDir.reset(new utl::TempFileNamed(nullptr, true));
    CPPUNIT_ASSERT_MESSAGE("Could not create valid temporary directory", pTempDir->IsValid());
    pTempDir->EnableKillingFile();
    const OUString aGalleryURL = pTempDir->GetURL();

    // Check if directory exists
    CPPUNIT_ASSERT_MESSAGE("Could not create temporary directory",
                           comphelper::DirectoryHelper::dirExists(aGalleryURL));

    std::unique_ptr<Gallery> pGallery(new Gallery(aGalleryURL));
    CPPUNIT_ASSERT_MESSAGE("Could not create gallery instance", (pGallery != nullptr));
    static constexpr OUString myThemeName = u"addytesttheme"_ustr;
    CPPUNIT_ASSERT_MESSAGE("Could not create theme", pGallery->CreateTheme(myThemeName));
    CPPUNIT_ASSERT_MESSAGE("Could not find theme", pGallery->HasTheme(myThemeName));

    // Create Sfx Instance
    SfxListener aListener;
    SfxApplication::GetOrCreate();

    // Insert Objects Into Theme
    GalleryTheme* pGalleryTheme = pGallery->AcquireTheme(myThemeName, aListener);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Object count inconsistent", sal_uInt32(0),
                                 pGalleryTheme->GetObjectCount());

    OUString imageList[] = { u"galtest1.png"_ustr, u"galtest2.png"_ustr, u"galtest3.jpg"_ustr };

    for (sal_uInt32 i = 0; i < (sizeof(imageList) / sizeof(imageList[0])); i++)
    {
        OUString imageNameFromList(imageList[i]);
        OUString aURL(m_directories.getURLFromSrc(u"/svx/qa/unit/gallery/data/")
                      + imageNameFromList);
        CPPUNIT_ASSERT_MESSAGE("Could not insert object into theme",
                               pGalleryTheme->InsertURL(INetURLObject(aURL)));
        CPPUNIT_ASSERT_EQUAL_MESSAGE("Inconsistent object Count", pGalleryTheme->GetObjectCount(),
                                     i + 1);
        std::unique_ptr<SgaObject> pObj = pGalleryTheme->AcquireObject(i);
        CPPUNIT_ASSERT_MESSAGE("Acquired Object Invalid", pObj->IsValid());
    }

    CPPUNIT_ASSERT(pGalleryTheme->ChangeObjectPos(1, 3));
    std::unique_ptr<SgaObject> pObj = pGalleryTheme->AcquireObject(0);
    INetURLObject aURL = pObj->GetURL();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Failure to change object position", imageList[0],
                                 aURL.GetLastName());

    pObj = pGalleryTheme->AcquireObject(1);
    aURL = pObj->GetURL();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Failure to change object position", imageList[2],
                                 aURL.GetLastName());

    pObj = pGalleryTheme->AcquireObject(2);
    aURL = pObj->GetURL();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Failure to change object position", imageList[1],
                                 aURL.GetLastName());

    pGallery->ReleaseTheme(pGalleryTheme, aListener);
}

void GalleryObjTest::TestGetThemeNameFromGalleryTheme()
{
    // Create theme
    std::unique_ptr<utl::TempFileNamed> pTempDir;
    pTempDir.reset(new utl::TempFileNamed(nullptr, true));
    CPPUNIT_ASSERT_MESSAGE("Could not create valid temporary directory", pTempDir->IsValid());
    pTempDir->EnableKillingFile();
    const OUString aGalleryURL = pTempDir->GetURL();

    // Check if directory exists
    CPPUNIT_ASSERT_MESSAGE("Could not create temporary directory",
                           comphelper::DirectoryHelper::dirExists(aGalleryURL));

    std::unique_ptr<Gallery> pGallery(new Gallery(aGalleryURL));
    CPPUNIT_ASSERT_MESSAGE("Could not create gallery instance", (pGallery != nullptr));
    constexpr OUString myThemeName = u"addytesttheme"_ustr;
    CPPUNIT_ASSERT_MESSAGE("Could not create theme", pGallery->CreateTheme(myThemeName));
    CPPUNIT_ASSERT_MESSAGE("Could not find theme", pGallery->HasTheme(myThemeName));

    // Create Sfx Instance
    SfxListener aListener;
    SfxApplication::GetOrCreate();

    GalleryTheme* pGalleryTheme = pGallery->AcquireTheme(myThemeName, aListener);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Object count inconsistent", sal_uInt32(0),
                                 pGalleryTheme->GetObjectCount());

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Theme name not matching", myThemeName, pGalleryTheme->GetName());

    pGallery->ReleaseTheme(pGalleryTheme, aListener);
}

CPPUNIT_TEST_SUITE_REGISTRATION(GalleryObjTest);

CPPUNIT_PLUGIN_IMPLEMENT();
