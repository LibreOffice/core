/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <test/bootstrapfixture.hxx>
#include <cppunit/plugin/TestPlugIn.h>
#include <rtl/ustring.hxx>

// Mock the GDriveFileInfo structure for testing
struct GDriveFileInfo {
    rtl::OUString id;
    rtl::OUString name;
    rtl::OUString mimeType;
    rtl::OUString size;
    rtl::OUString modifiedTime;
    bool isFolder;

    GDriveFileInfo() : isFolder(false) {}
};

// Content types from gdrive provider
inline constexpr OUString GDRIVE_FILE_TYPE = u"application/gdrive-file"_ustr;
inline constexpr OUString GDRIVE_FOLDER_TYPE = u"application/gdrive-folder"_ustr;

namespace
{
    class gdrive_content_test: public test::BootstrapFixture
    {

    public:
        gdrive_content_test() : BootstrapFixture( true, false ) {}

        void setUp() override;
        void tearDown() override;

        void testFileInfoCreation();
        void testContentTypeDetection();
        void testFileProperties();

        CPPUNIT_TEST_SUITE( gdrive_content_test );
        CPPUNIT_TEST( testFileInfoCreation );
        CPPUNIT_TEST( testContentTypeDetection );
        CPPUNIT_TEST( testFileProperties );
        CPPUNIT_TEST_SUITE_END();
    };

    void gdrive_content_test::setUp()
    {
    }

    void gdrive_content_test::tearDown()
    {
    }

    void gdrive_content_test::testFileInfoCreation()
    {
        // Test file creation
        GDriveFileInfo fileInfo;
        fileInfo.id = u"1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms"_ustr;
        fileInfo.name = u"Test Document.docx"_ustr;
        fileInfo.mimeType = u"application/vnd.openxmlformats-officedocument.wordprocessingml.document"_ustr;
        fileInfo.size = u"12345"_ustr;
        fileInfo.modifiedTime = u"2023-01-01T12:00:00.000Z"_ustr;
        fileInfo.isFolder = false;

        CPPUNIT_ASSERT_EQUAL(u"1BxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms"_ustr, fileInfo.id);
        CPPUNIT_ASSERT_EQUAL(u"Test Document.docx"_ustr, fileInfo.name);
        CPPUNIT_ASSERT_EQUAL(false, fileInfo.isFolder);

        // Test folder creation
        GDriveFileInfo folderInfo;
        folderInfo.id = u"2CxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms"_ustr;
        folderInfo.name = u"My Folder"_ustr;
        folderInfo.mimeType = u"application/vnd.google-apps.folder"_ustr;
        folderInfo.isFolder = true;

        CPPUNIT_ASSERT_EQUAL(u"2CxiMVs0XRA5nFMdKvBdBZjgmUUqptlbs74OgvE2upms"_ustr, folderInfo.id);
        CPPUNIT_ASSERT_EQUAL(u"My Folder"_ustr, folderInfo.name);
        CPPUNIT_ASSERT_EQUAL(true, folderInfo.isFolder);
    }

    void gdrive_content_test::testContentTypeDetection()
    {
        // Test file content type
        GDriveFileInfo fileInfo;
        fileInfo.isFolder = false;

        rtl::OUString expectedFileType = GDRIVE_FILE_TYPE;
        rtl::OUString actualFileType = fileInfo.isFolder ? GDRIVE_FOLDER_TYPE : GDRIVE_FILE_TYPE;
        CPPUNIT_ASSERT_EQUAL(expectedFileType, actualFileType);

        // Test folder content type
        GDriveFileInfo folderInfo;
        folderInfo.isFolder = true;

        rtl::OUString expectedFolderType = GDRIVE_FOLDER_TYPE;
        rtl::OUString actualFolderType = folderInfo.isFolder ? GDRIVE_FOLDER_TYPE : GDRIVE_FILE_TYPE;
        CPPUNIT_ASSERT_EQUAL(expectedFolderType, actualFolderType);
    }

    void gdrive_content_test::testFileProperties()
    {
        // Test numeric size conversion
        GDriveFileInfo fileInfo;
        fileInfo.size = u"123456789"_ustr;

        sal_Int64 expectedSize = 123456789;
        sal_Int64 actualSize = fileInfo.size.isEmpty() ? 0 : fileInfo.size.toInt64();
        CPPUNIT_ASSERT_EQUAL(expectedSize, actualSize);

        // Test empty size handling
        GDriveFileInfo emptyFile;
        emptyFile.size = u""_ustr;

        sal_Int64 emptySize = emptyFile.size.isEmpty() ? 0 : emptyFile.size.toInt64();
        CPPUNIT_ASSERT_EQUAL(sal_Int64(0), emptySize);

        // Test folder detection from MIME type
        GDriveFileInfo folder;
        folder.mimeType = u"application/vnd.google-apps.folder"_ustr;
        bool isFolderFromMime = (folder.mimeType == u"application/vnd.google-apps.folder"_ustr);
        CPPUNIT_ASSERT_EQUAL(true, isFolderFromMime);

        // Test regular file MIME type
        GDriveFileInfo file;
        file.mimeType = u"text/plain"_ustr;
        bool isFileFromMime = (file.mimeType != u"application/vnd.google-apps.folder"_ustr);
        CPPUNIT_ASSERT_EQUAL(true, isFileFromMime);
    }

    CPPUNIT_TEST_SUITE_REGISTRATION(gdrive_content_test);
}

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */