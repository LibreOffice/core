/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <test/bootstrapfixture.hxx>

#include <o3tl/cppunittraitshelper.hxx>
#include <unotest/directories.hxx>
#include <svl/lockfilecommon.hxx>
#include <svl/documentlockfile.hxx>
#include <svl/msodocumentlockfile.hxx>
#include <unotools/useroptions.hxx>
#include <tools/stream.hxx>
#include <rtl/strbuf.hxx>
#include <osl/security.hxx>
#include <osl/socket.hxx>
#include <unotools/bootstrap.hxx>

namespace
{
class LockfileTest : public test::BootstrapFixture
{
    OUString generateTestURL(const OUString& sFileName) const;

public:
    void testLOLockFileURL();
    void testLOLockFileContent();
    void testLOLockFileRT();
    void testLOLockFileUnicodeUsername();
    void testLOLockFileOverwrite();
    void testWordLockFileURL();
    void testExcelLockFileURL();
    void testPowerPointLockFileURL();
    void testWordLockFileContent();
    void testExcelLockFileContent();
    void testPowerPointLockFileContent();
    void testWordLockFileRT();
    void testExcelLockFileRT();
    void testPowerPointLockFileRT();
    void testMSOLockFileLongUserName();
    void testMSOLockFileUnicodeUsername();
    void testMSOLockFileOverwrite();

private:
    CPPUNIT_TEST_SUITE(LockfileTest);
    CPPUNIT_TEST(testLOLockFileURL);
    CPPUNIT_TEST(testLOLockFileContent);
    CPPUNIT_TEST(testLOLockFileRT);
    CPPUNIT_TEST(testLOLockFileUnicodeUsername);
    CPPUNIT_TEST(testLOLockFileOverwrite);
    CPPUNIT_TEST(testWordLockFileURL);
    CPPUNIT_TEST(testExcelLockFileURL);
    CPPUNIT_TEST(testPowerPointLockFileURL);
    CPPUNIT_TEST(testWordLockFileContent);
    CPPUNIT_TEST(testExcelLockFileContent);
    CPPUNIT_TEST(testPowerPointLockFileContent);
    CPPUNIT_TEST(testWordLockFileRT);
    CPPUNIT_TEST(testExcelLockFileRT);
    CPPUNIT_TEST(testPowerPointLockFileRT);
    CPPUNIT_TEST(testMSOLockFileLongUserName);
    CPPUNIT_TEST(testMSOLockFileUnicodeUsername);
    CPPUNIT_TEST(testMSOLockFileOverwrite);
    CPPUNIT_TEST_SUITE_END();
};

OUString readLockFile(const OUString& aSource)
{
    SvFileStream aFileStream(aSource, StreamMode::READ);
    std::size_t nSize = aFileStream.remainingSize();
    std::unique_ptr<sal_Int8[]> pBuffer(new sal_Int8[nSize]);
    aFileStream.ReadBytes(pBuffer.get(), nSize);

    css::uno::Sequence<sal_Int8> aData(pBuffer.get(), nSize);
    OStringBuffer aResult(static_cast<int>(nSize));
    for (sal_Int8 nByte : aData)
    {
        aResult.append(static_cast<char>(nByte));
    }
    return OStringToOUString(aResult.makeStringAndClear(), RTL_TEXTENCODING_UTF8);
}

OUString GetLockFileName(const svt::GenDocumentLockFile& rLockFile)
{
    INetURLObject aDocURL = svt::LockFileCommon::ResolveLinks(INetURLObject(rLockFile.GetURL()));
    return aDocURL.GetLastName();
}

OUString LockfileTest::generateTestURL(const OUString& sFileName) const
{
    return m_directories.getURLFromWorkdir("/CppunitTest/svl_lockfiles.test.user/") + sFileName;
}

void LockfileTest::testLOLockFileURL()
{
    // Test the generated file name for LibreOffice lock files
    OUString aTestODT = generateTestURL("testLOLockFileURL.odt");

    svt::DocumentLockFile aLockFile(aTestODT);
    CPPUNIT_ASSERT_EQUAL(OUString(".~lock.testLOLockFileURL.odt%23"), GetLockFileName(aLockFile));
}

void LockfileTest::testLOLockFileContent()
{
    // Test the lockfile generated for the specified ODT document
    OUString aTestODT = generateTestURL("testLOLockFileContent.odt");

    // Set user name
    SvtUserOptions aUserOpt;
    aUserOpt.SetToken(UserOptToken::FirstName, "LockFile");
    aUserOpt.SetToken(UserOptToken::LastName, "Test");

    // Write the lock file and check the content
    svt::DocumentLockFile aLockFile(aTestODT);
    aLockFile.CreateOwnLockFile();
    OUString sLockFileContent(readLockFile(aLockFile.GetURL()));
    aLockFile.RemoveFileDirectly();

    // User name
    sal_Int32 nFirstChar = 0;
    sal_Int32 nNextComma = sLockFileContent.indexOf(',', nFirstChar);
    OUString sUserName;
    sUserName += aUserOpt.GetFirstName() + " ";
    sUserName += aUserOpt.GetLastName();
    CPPUNIT_ASSERT_EQUAL(sUserName, sLockFileContent.copy(nFirstChar, nNextComma - nFirstChar));

    // System user name
    nFirstChar = nNextComma + 1;
    nNextComma = sLockFileContent.indexOf(',', nFirstChar);
    ::osl::Security aSecurity;
    OUString sSysUserName;
    aSecurity.getUserName(sSysUserName);
    CPPUNIT_ASSERT_EQUAL(sSysUserName, sLockFileContent.copy(nFirstChar, nNextComma - nFirstChar));

    // Local host
    nFirstChar = nNextComma + 1;
    nNextComma = sLockFileContent.indexOf(',', nFirstChar);
    CPPUNIT_ASSERT_EQUAL(::osl::SocketAddr::getLocalHostname(),
                         sLockFileContent.copy(nFirstChar, nNextComma - nFirstChar));

    // Skip date and time because it changes after the lock file was created
    nFirstChar = nNextComma + 1;
    nNextComma = sLockFileContent.indexOf(',', nFirstChar);

    // user url
    nFirstChar = nNextComma + 1;
    OUString aUserInstDir;
    ::utl::Bootstrap::locateUserInstallation(aUserInstDir);
    CPPUNIT_ASSERT_EQUAL(
        aUserInstDir,
        sLockFileContent.copy(nFirstChar, sLockFileContent.getLength() - nFirstChar - 1));
}

void LockfileTest::testLOLockFileRT()
{
    // Test the lockfile generated for the specified ODT document
    OUString aTestODT = generateTestURL("testLOLockFileRT.odt");

    // Set user name
    SvtUserOptions aUserOpt;
    aUserOpt.SetToken(UserOptToken::FirstName, "LockFile");
    aUserOpt.SetToken(UserOptToken::LastName, "Test");

    // Write the lock file and read it back
    svt::DocumentLockFile aLockFile(aTestODT);
    LockFileEntry aOrigEntry = svt::LockFileCommon::GenerateOwnEntry();
    aLockFile.CreateOwnLockFile();
    LockFileEntry aRTEntry = aLockFile.GetLockData();

    // Check whether the lock file attributes are the same
    CPPUNIT_ASSERT_EQUAL(aOrigEntry[LockFileComponent::OOOUSERNAME],
                         aRTEntry[LockFileComponent::OOOUSERNAME]);
    CPPUNIT_ASSERT_EQUAL(aOrigEntry[LockFileComponent::SYSUSERNAME],
                         aRTEntry[LockFileComponent::SYSUSERNAME]);
    CPPUNIT_ASSERT_EQUAL(aOrigEntry[LockFileComponent::LOCALHOST],
                         aRTEntry[LockFileComponent::LOCALHOST]);
    CPPUNIT_ASSERT_EQUAL(aOrigEntry[LockFileComponent::USERURL],
                         aRTEntry[LockFileComponent::USERURL]);
    // LockFileComponent::EDITTIME can change

    aLockFile.RemoveFileDirectly();
}

void LockfileTest::testLOLockFileUnicodeUsername()
{
    // Test the lockfile generated for the specified ODT document
    OUString aTestODT = generateTestURL("testLOLockFileUnicodeUsername.odt");

    // Set user name
    SvtUserOptions aUserOpt;
    sal_Unicode vFirstName[] = { 2351, 2676, 3117, 5279 };
    aUserOpt.SetToken(UserOptToken::FirstName, OUString(vFirstName, 4));
    sal_Unicode vLastName[] = { 671, 1245, 1422, 1822 };
    aUserOpt.SetToken(UserOptToken::LastName, OUString(vLastName, 4));

    // Write the lock file and read it back
    svt::DocumentLockFile aLockFile(aTestODT);
    LockFileEntry aOrigEntry = svt::LockFileCommon::GenerateOwnEntry();
    aLockFile.CreateOwnLockFile();
    LockFileEntry aRTEntry = aLockFile.GetLockData();

    // Check whether the lock file attributes are the same
    CPPUNIT_ASSERT_EQUAL(aOrigEntry[LockFileComponent::OOOUSERNAME],
                         aRTEntry[LockFileComponent::OOOUSERNAME]);
    CPPUNIT_ASSERT_EQUAL(OUString(aUserOpt.GetFirstName() + " " + aUserOpt.GetLastName()),
                         aOrigEntry[LockFileComponent::OOOUSERNAME]);

    aLockFile.RemoveFileDirectly();
}

void LockfileTest::testLOLockFileOverwrite()
{
    OUString aTestODT = generateTestURL("testLOLockFileOverwrite.odt");

    // Set user name
    SvtUserOptions aUserOpt;
    aUserOpt.SetToken(UserOptToken::FirstName, "LockFile");
    aUserOpt.SetToken(UserOptToken::LastName, "Test");

    // Write the lock file and read it back
    svt::DocumentLockFile aLockFile(aTestODT);
    aLockFile.CreateOwnLockFile();

    // Change user name
    aUserOpt.SetToken(UserOptToken::FirstName, "LockFile2");
    aUserOpt.SetToken(UserOptToken::LastName, "Test");

    // Overwrite lockfile
    svt::DocumentLockFile aLockFile2(aTestODT);
    LockFileEntry aOrigEntry = svt::LockFileCommon::GenerateOwnEntry();
    aLockFile2.OverwriteOwnLockFile();

    LockFileEntry aRTEntry = aLockFile.GetLockData();

    // Check whether the lock file attributes are the same
    CPPUNIT_ASSERT_EQUAL(aOrigEntry[LockFileComponent::OOOUSERNAME],
                         aRTEntry[LockFileComponent::OOOUSERNAME]);
    CPPUNIT_ASSERT_EQUAL(aOrigEntry[LockFileComponent::SYSUSERNAME],
                         aRTEntry[LockFileComponent::SYSUSERNAME]);
    CPPUNIT_ASSERT_EQUAL(aOrigEntry[LockFileComponent::LOCALHOST],
                         aRTEntry[LockFileComponent::LOCALHOST]);
    CPPUNIT_ASSERT_EQUAL(aOrigEntry[LockFileComponent::USERURL],
                         aRTEntry[LockFileComponent::USERURL]);

    aLockFile2.RemoveFileDirectly();
}

void LockfileTest::testWordLockFileURL()
{
    // Test the generated file name for Word lock files

    // Word specific file format
    {
        OUString aTestFile = generateTestURL("testWordLockFileURL.docx");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$stWordLockFileURL.docx"), GetLockFileName(aLockFile));
    }

    // Eight character file name (cuts two characters)
    {
        OUString aTestFile = generateTestURL("12345678.docx");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$345678.docx"), GetLockFileName(aLockFile));
    }

    // Seven character file name (cuts one character)
    {
        OUString aTestFile = generateTestURL("1234567.docx");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$234567.docx"), GetLockFileName(aLockFile));
    }

    // Six character file name (cuts no character)
    {
        OUString aTestFile = generateTestURL("123456.docx");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$123456.docx"), GetLockFileName(aLockFile));
    }

    // One character file name
    {
        OUString aTestFile = generateTestURL("1.docx");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$1.docx"), GetLockFileName(aLockFile));
    }

    // Test for ODT format
    {
        OUString aTestFile = generateTestURL("12345678.odt");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$345678.odt"), GetLockFileName(aLockFile));
    }

    // Test for DOC format
    {
        OUString aTestFile = generateTestURL("12345678.doc");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$345678.doc"), GetLockFileName(aLockFile));
    }

    // Test for RTF format
    {
        OUString aTestFile = generateTestURL("12345678.rtf");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$345678.rtf"), GetLockFileName(aLockFile));
    }
}

void LockfileTest::testExcelLockFileURL()
{
    // Test the generated file name for Excel lock files
    {
        OUString aTestFile = generateTestURL("testExcelLockFileURL.xlsx");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$testExcelLockFileURL.xlsx"), GetLockFileName(aLockFile));
    }

    // Eight character file name
    {
        OUString aTestFile = generateTestURL("12345678.xlsx");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$12345678.xlsx"), GetLockFileName(aLockFile));
    }

    // One character file name
    {
        OUString aTestFile = generateTestURL("1.xlsx");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$1.xlsx"), GetLockFileName(aLockFile));
    }

    // Test for ODS format
    {
        OUString aTestFile = generateTestURL("12345678.ods");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$12345678.ods"), GetLockFileName(aLockFile));
    }
}

void LockfileTest::testPowerPointLockFileURL()
{
    // Test the generated file name for PowerPoint lock files
    {
        OUString aTestFile = generateTestURL("testPowerPointLockFileURL.pptx");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$testPowerPointLockFileURL.pptx"),
                             GetLockFileName(aLockFile));
    }

    // Eight character file name
    {
        OUString aTestFile = generateTestURL("12345678.pptx");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$12345678.pptx"), GetLockFileName(aLockFile));
    }

    // One character file name
    {
        OUString aTestFile = generateTestURL("1.pptx");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$1.pptx"), GetLockFileName(aLockFile));
    }

    // Test for PPT format
    {
        OUString aTestFile = generateTestURL("12345678.ppt");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$12345678.ppt"), GetLockFileName(aLockFile));
    }

    // Test for ODP format
    {
        OUString aTestFile = generateTestURL("/12345678.odp");
        svt::MSODocumentLockFile aLockFile(aTestFile);
        CPPUNIT_ASSERT_EQUAL(OUString("~$12345678.odp"), GetLockFileName(aLockFile));
    }
}

void LockfileTest::testWordLockFileContent()
{
    // Test the lockfile generated for the specified DOCX document
    OUString aTestFile = generateTestURL("testWordLockFileContent.docx");

    // Set user name
    SvtUserOptions aUserOpt;
    aUserOpt.SetToken(UserOptToken::FirstName, "LockFile");
    aUserOpt.SetToken(UserOptToken::LastName, "Test");

    // Write the lock file and check the content
    svt::MSODocumentLockFile aLockFile(aTestFile);
    aLockFile.CreateOwnLockFile();
    OUString sLockFileContent(readLockFile(aLockFile.GetURL()));
    aLockFile.RemoveFileDirectly();

    // First character is the size of the user name
    OUString sUserName;
    sUserName += aUserOpt.GetFirstName() + " ";
    sUserName += aUserOpt.GetLastName();
    int nIndex = 0;
    CPPUNIT_ASSERT_EQUAL(sUserName.getLength(), static_cast<sal_Int32>(sLockFileContent[nIndex]));

    // Then we have the user name
    CPPUNIT_ASSERT_EQUAL(sUserName, sLockFileContent.copy(1, sUserName.getLength()));

    // We have some filling 0 bytes after the user name
    for (nIndex = sUserName.getLength() + 1; nIndex < MSO_USERNAME_MAX_LENGTH + 2; ++nIndex)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), static_cast<sal_Int32>(sLockFileContent[nIndex]));
    }

    // Then we have the user name's length again
    CPPUNIT_ASSERT_EQUAL(sUserName.getLength(), static_cast<sal_Int32>(sLockFileContent[nIndex]));

    // Then we have the user name again with 16 bit coding
    for (int i = 0; i < sUserName.getLength(); ++i)
    {
        CPPUNIT_ASSERT_EQUAL(
            sUserName[i],
            static_cast<sal_Unicode>(static_cast<sal_Int16>(sLockFileContent[55 + i * 2])
                                     + static_cast<sal_Int16>(sLockFileContent[55 + i * 2 + 1])));
    }

    // We have some filling 0 bytes after the user name
    for (nIndex += sUserName.getLength() * 2 + 1; nIndex < MSO_WORD_LOCKFILE_SIZE; ++nIndex)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0), static_cast<sal_Int32>(sLockFileContent[nIndex]));
    }

    // We have a fixed size lock file
    CPPUNIT_ASSERT_EQUAL(sal_Int32(MSO_WORD_LOCKFILE_SIZE), sLockFileContent.getLength());
}

void LockfileTest::testExcelLockFileContent()
{
    // Test the lockfile generated for the specified XLSX document
    OUString aTestFile = generateTestURL("testExcelLockFileContent.xlsx");

    // Set user name
    SvtUserOptions aUserOpt;
    aUserOpt.SetToken(UserOptToken::FirstName, "LockFile");
    aUserOpt.SetToken(UserOptToken::LastName, "Test");

    // Write the lock file and check the content
    svt::MSODocumentLockFile aLockFile(aTestFile);
    aLockFile.CreateOwnLockFile();
    OUString sLockFileContent(readLockFile(aLockFile.GetURL()));
    aLockFile.RemoveFileDirectly();

    // First character is the size of the user name
    OUString sUserName;
    sUserName += aUserOpt.GetFirstName() + " ";
    sUserName += aUserOpt.GetLastName();
    int nIndex = 0;
    CPPUNIT_ASSERT_EQUAL(sUserName.getLength(), static_cast<sal_Int32>(sLockFileContent[nIndex]));

    // Then we have the user name
    CPPUNIT_ASSERT_EQUAL(sUserName, sLockFileContent.copy(1, sUserName.getLength()));

    // We have some filling 0x20 bytes after the user name
    for (nIndex = sUserName.getLength() + 1; nIndex < MSO_USERNAME_MAX_LENGTH + 3; ++nIndex)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x20), static_cast<sal_Int32>(sLockFileContent[nIndex]));
    }

    // Then we have the user name's length again
    CPPUNIT_ASSERT_EQUAL(sUserName.getLength(), static_cast<sal_Int32>(sLockFileContent[nIndex]));

    // Then we have the user name again with 16 bit coding
    for (int i = 0; i < sUserName.getLength(); ++i)
    {
        CPPUNIT_ASSERT_EQUAL(
            sUserName[i],
            static_cast<sal_Unicode>(static_cast<sal_Int16>(sLockFileContent[56 + i * 2])
                                     + static_cast<sal_Int16>(sLockFileContent[56 + i * 2 + 1])));
    }

    // We have some filling 0 and 0x20 bytes after the user name
    for (nIndex += sUserName.getLength() * 2 + 2; nIndex < MSO_EXCEL_AND_POWERPOINT_LOCKFILE_SIZE;
         nIndex += 2)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x20), static_cast<sal_Int32>(sLockFileContent[nIndex]));
        if (nIndex + 1 < MSO_EXCEL_AND_POWERPOINT_LOCKFILE_SIZE)
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                                 static_cast<sal_Int32>(sLockFileContent[nIndex + 1]));
    }

    // We have a fixed size lock file
    CPPUNIT_ASSERT_EQUAL(sal_Int32(MSO_EXCEL_AND_POWERPOINT_LOCKFILE_SIZE),
                         sLockFileContent.getLength());
}

void LockfileTest::testPowerPointLockFileContent()
{
    // Test the lockfile generated for the specified PPTX document
    OUString aTestFile = generateTestURL("testPowerPointLockFileContent.pptx");

    // Set user name
    SvtUserOptions aUserOpt;
    aUserOpt.SetToken(UserOptToken::FirstName, "LockFile");
    aUserOpt.SetToken(UserOptToken::LastName, "Test");

    // Write the lock file and check the content
    svt::MSODocumentLockFile aLockFile(aTestFile);
    aLockFile.CreateOwnLockFile();
    OUString sLockFileContent(readLockFile(aLockFile.GetURL()));
    aLockFile.RemoveFileDirectly();

    // First character is the size of the user name
    OUString sUserName;
    sUserName += aUserOpt.GetFirstName() + " ";
    sUserName += aUserOpt.GetLastName();
    int nIndex = 0;
    CPPUNIT_ASSERT_EQUAL(sUserName.getLength(), static_cast<sal_Int32>(sLockFileContent[nIndex]));

    // Then we have the user name
    CPPUNIT_ASSERT_EQUAL(sUserName, sLockFileContent.copy(1, sUserName.getLength()));

    // We have some filling bytes after the user name
    nIndex = sUserName.getLength() + 1;
    CPPUNIT_ASSERT_EQUAL(sal_Int32(0), static_cast<sal_Int32>(sLockFileContent[nIndex]));
    for (nIndex += 1; nIndex < MSO_USERNAME_MAX_LENGTH + 3; ++nIndex)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x20), static_cast<sal_Int32>(sLockFileContent[nIndex]));
    }

    // Then we have the user name's length again
    CPPUNIT_ASSERT_EQUAL(sUserName.getLength(), static_cast<sal_Int32>(sLockFileContent[nIndex]));

    // Then we have the user name again with 16 bit coding
    for (int i = 0; i < sUserName.getLength(); ++i)
    {
        CPPUNIT_ASSERT_EQUAL(
            sUserName[i],
            static_cast<sal_Unicode>(static_cast<sal_Int16>(sLockFileContent[56 + i * 2])
                                     + static_cast<sal_Int16>(sLockFileContent[56 + i * 2 + 1])));
    }

    // We have some filling 0 and 0x20 bytes after the user name
    for (nIndex += sUserName.getLength() * 2 + 2; nIndex < MSO_EXCEL_AND_POWERPOINT_LOCKFILE_SIZE;
         nIndex += 2)
    {
        CPPUNIT_ASSERT_EQUAL(sal_Int32(0x20), static_cast<sal_Int32>(sLockFileContent[nIndex]));
        if (nIndex + 1 < MSO_EXCEL_AND_POWERPOINT_LOCKFILE_SIZE)
            CPPUNIT_ASSERT_EQUAL(sal_Int32(0),
                                 static_cast<sal_Int32>(sLockFileContent[nIndex + 1]));
    }

    // We have a fixed size lock file
    CPPUNIT_ASSERT_EQUAL(sal_Int32(MSO_EXCEL_AND_POWERPOINT_LOCKFILE_SIZE),
                         sLockFileContent.getLength());
}

void LockfileTest::testWordLockFileRT()
{
    OUString aTestODT = generateTestURL("testWordLockFileRT.docx");

    // Set user name
    SvtUserOptions aUserOpt;
    aUserOpt.SetToken(UserOptToken::FirstName, "LockFile");
    aUserOpt.SetToken(UserOptToken::LastName, "Test");

    // Write the lock file and read it back
    svt::MSODocumentLockFile aLockFile(aTestODT);
    LockFileEntry aOrigEntry = svt::LockFileCommon::GenerateOwnEntry();
    aLockFile.CreateOwnLockFile();
    LockFileEntry aRTEntry = aLockFile.GetLockData();
    aLockFile.RemoveFileDirectly();

    // Check whether the lock file attributes are the same
    CPPUNIT_ASSERT_EQUAL(aOrigEntry[LockFileComponent::OOOUSERNAME],
                         aRTEntry[LockFileComponent::OOOUSERNAME]);
}

void LockfileTest::testExcelLockFileRT()
{
    OUString aTestODT = generateTestURL("testExcelLockFileRT.xlsx");

    // Set user name
    SvtUserOptions aUserOpt;
    aUserOpt.SetToken(UserOptToken::FirstName, "LockFile");
    aUserOpt.SetToken(UserOptToken::LastName, "Test");

    // Write the lock file and read it back
    svt::MSODocumentLockFile aLockFile(aTestODT);
    LockFileEntry aOrigEntry = svt::LockFileCommon::GenerateOwnEntry();
    aLockFile.CreateOwnLockFile();
    LockFileEntry aRTEntry = aLockFile.GetLockData();
    aLockFile.RemoveFileDirectly();

    // Check whether the lock file attributes are the same
    CPPUNIT_ASSERT_EQUAL(aOrigEntry[LockFileComponent::OOOUSERNAME],
                         aRTEntry[LockFileComponent::OOOUSERNAME]);
}

void LockfileTest::testPowerPointLockFileRT()
{
    OUString aTestODT = generateTestURL("testPowerPointLockFileRT.pptx");

    // Set user name
    SvtUserOptions aUserOpt;
    aUserOpt.SetToken(UserOptToken::FirstName, "LockFile");
    aUserOpt.SetToken(UserOptToken::LastName, "Test");

    // Write the lock file and read it back
    svt::MSODocumentLockFile aLockFile(aTestODT);
    LockFileEntry aOrigEntry = svt::LockFileCommon::GenerateOwnEntry();
    aLockFile.CreateOwnLockFile();
    LockFileEntry aRTEntry = aLockFile.GetLockData();
    aLockFile.RemoveFileDirectly();

    // Check whether the lock file attributes are the same
    CPPUNIT_ASSERT_EQUAL(aOrigEntry[LockFileComponent::OOOUSERNAME],
                         aRTEntry[LockFileComponent::OOOUSERNAME]);
}

void LockfileTest::testMSOLockFileLongUserName()
{
    OUString aTestODT = generateTestURL("testMSOLockFileLongUserName.docx");

    // Set user name
    SvtUserOptions aUserOpt;
    aUserOpt.SetToken(UserOptToken::FirstName,
                      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");
    aUserOpt.SetToken(UserOptToken::LastName,
                      "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx");

    // Write the lock file and read it back
    svt::MSODocumentLockFile aLockFile(aTestODT);
    LockFileEntry aOrigEntry = svt::LockFileCommon::GenerateOwnEntry();
    aLockFile.CreateOwnLockFile();
    LockFileEntry aRTEntry = aLockFile.GetLockData();

    // Check whether the user name was cut to the maximum length
    CPPUNIT_ASSERT_EQUAL(
        aOrigEntry[LockFileComponent::OOOUSERNAME].copy(0, MSO_USERNAME_MAX_LENGTH),
        aRTEntry[LockFileComponent::OOOUSERNAME]);

    aLockFile.RemoveFileDirectly();
}

void LockfileTest::testMSOLockFileUnicodeUsername()
{
    // Test the lockfile generated for the specified ODT document
    OUString aTestODT = generateTestURL("testMSOLockFileUnicodeUsername.docx");

    // Set user name
    SvtUserOptions aUserOpt;
    sal_Unicode vFirstName[] = { 2351, 2676, 3117, 5279 };
    aUserOpt.SetToken(UserOptToken::FirstName, OUString(vFirstName, 4));
    sal_Unicode vLastName[] = { 671, 1245, 1422, 1822 };
    aUserOpt.SetToken(UserOptToken::LastName, OUString(vLastName, 4));

    // Write the lock file and read it back
    svt::DocumentLockFile aLockFile(aTestODT);
    LockFileEntry aOrigEntry = svt::LockFileCommon::GenerateOwnEntry();
    aLockFile.CreateOwnLockFile();
    LockFileEntry aRTEntry = aLockFile.GetLockData();

    // Check whether the user name is the same
    CPPUNIT_ASSERT_EQUAL(aOrigEntry[LockFileComponent::OOOUSERNAME],
                         aRTEntry[LockFileComponent::OOOUSERNAME]);
    CPPUNIT_ASSERT_EQUAL(OUString(aUserOpt.GetFirstName() + " " + aUserOpt.GetLastName()),
                         aOrigEntry[LockFileComponent::OOOUSERNAME]);

    aLockFile.RemoveFileDirectly();
}

void LockfileTest::testMSOLockFileOverwrite()
{
    OUString aTestODT = generateTestURL("testMSOLockFileOverwrite.docx");

    // Set user name
    SvtUserOptions aUserOpt;
    aUserOpt.SetToken(UserOptToken::FirstName, "LockFile");
    aUserOpt.SetToken(UserOptToken::LastName, "Test");

    // Write the lock file and read it back
    svt::MSODocumentLockFile aLockFile(aTestODT);
    aLockFile.CreateOwnLockFile();

    // Change user name
    aUserOpt.SetToken(UserOptToken::FirstName, "LockFile2");
    aUserOpt.SetToken(UserOptToken::LastName, "Test");

    // Overwrite lockfile
    svt::MSODocumentLockFile aLockFile2(aTestODT);
    LockFileEntry aOrigEntry = svt::LockFileCommon::GenerateOwnEntry();
    aLockFile2.OverwriteOwnLockFile();

    LockFileEntry aRTEntry = aLockFile.GetLockData();

    // Check whether the lock file attributes are the same
    CPPUNIT_ASSERT_EQUAL(aOrigEntry[LockFileComponent::OOOUSERNAME],
                         aRTEntry[LockFileComponent::OOOUSERNAME]);

    aLockFile2.RemoveFileDirectly();
}

CPPUNIT_TEST_SUITE_REGISTRATION(LockfileTest);
} // namespace

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
