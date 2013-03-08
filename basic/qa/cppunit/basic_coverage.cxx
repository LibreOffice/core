/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "basictest.hxx"
#include <osl/file.hxx>
#include "basic/sbmod.hxx"
#include "basic/sbmeth.hxx"


namespace
{

class Coverage : public test::BootstrapFixture
{
private:
    int  m_nb_tests;
    int  m_nb_tests_ok;
    int  m_nb_tests_skipped;
    OUString m_sCurrentTest;
    void process_directory(OUString sDirName);
    void process_file(OUString sFileName);
    void run_test(OUString sFileName, OUString sCode);
    void test_start(OUString /* sFileName */);
    void test_failed(void);
    void test_success(void);
    void print_summary() {};

public:
    Coverage();
    ~Coverage();

    void Coverage_Iterator();

    // Adds code needed to register the test suite
    CPPUNIT_TEST_SUITE(Coverage);

    // Declares the method as a test to call
    CPPUNIT_TEST(Coverage_Iterator);

    // End of test suite definition
    CPPUNIT_TEST_SUITE_END();
};

Coverage::Coverage()
    : BootstrapFixture(true, false)
    , m_nb_tests(0)
    , m_nb_tests_ok(0)
    , m_nb_tests_skipped(0)
{
}

Coverage::~Coverage()
{
    fprintf(stderr,"basic coverage Summary : skipped:%d pass:%d\n", m_nb_tests_skipped, m_nb_tests_ok );
}

void Coverage::test_start(OUString sFileName)
{
    m_nb_tests += 1;
    m_sCurrentTest = sFileName;
}

void Coverage::test_failed()
{
    CPPUNIT_FAIL(
        OUStringToOString(m_sCurrentTest, RTL_TEXTENCODING_UTF8).getStr());
}

void Coverage::test_success()
{
    m_nb_tests_ok += 1;
    fprintf(stderr,"%s,PASS\n", rtl::OUStringToOString( m_sCurrentTest, RTL_TEXTENCODING_UTF8 ).getStr() );
}

void Coverage::run_test(OUString /*sFileName*/, OUString sCode)
{
    bool result = false;
    MacroSnippet testMacro( sCode );
    testMacro.Compile();
    if( !testMacro.HasError() )
    {
        SbxVariableRef pResult = testMacro.Run();
        if( pResult && pResult->GetInteger() == 1 )
        {
            result = true;
        }
    }
    if(result)
    {
        test_success();
    }
    else
    {
        test_failed();
    }
}

void Coverage::process_file(OUString sFileName)
{
    osl::File aFile(sFileName);

    test_start(sFileName);
    if(osl::FileBase::E_None == aFile.open(osl_File_OpenFlag_Read))
    {
        sal_uInt64 size;
        sal_uInt64 size_read;
        if(osl::FileBase::E_None == aFile.getSize(size))
        {
            void* buffer = calloc(1, size+1);
            CPPUNIT_ASSERT(buffer);
            if(osl::FileBase::E_None == aFile.read( buffer, size, size_read))
            {
                if(size == size_read)
                {
                    OUString sCode((sal_Char*)buffer, size, RTL_TEXTENCODING_UTF8);
                    run_test(sFileName, sCode);
                    return;
                }
            }
        }
    }
    test_failed();
}

void Coverage::process_directory(OUString sDirName)
{
    osl::Directory aDir(sDirName);
    osl::DirectoryItem aItem;
    osl::FileStatus aFileStatus(osl_FileStatus_Mask_FileURL|osl_FileStatus_Mask_Type);

    if(osl::FileBase::E_None == aDir.open())
    {
        while (aDir.getNextItem(aItem) == osl::FileBase::E_None)
        {
            aItem.getFileStatus(aFileStatus);
            if(aFileStatus.isRegular())
            {
                process_file(aFileStatus.getFileURL());
            }
        }
    }
    else
    {
    }
    fprintf(stderr,"end process directory\n");
}

void Coverage::Coverage_Iterator(void)
{
    OUString sDirName = getURLFromSrc("/basic/qa/basic_coverage/");

    CPPUNIT_ASSERT(!sDirName.isEmpty());
    process_directory(sDirName);
}

  CPPUNIT_TEST_SUITE_REGISTRATION(Coverage);

}
CPPUNIT_PLUGIN_IMPLEMENT();



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
