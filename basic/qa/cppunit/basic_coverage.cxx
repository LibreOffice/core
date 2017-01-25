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
#include <basic/sbmod.hxx>
#include <basic/sbmeth.hxx>
#include <i18nlangtag/languagetag.hxx>
#include <unotools/syslocaleoptions.hxx>

namespace
{

class Coverage : public test::BootstrapFixture
{
private:
    int  m_nb_tests_ok;
    OUString m_sCurrentTest;
    void process_directory(const OUString& sDirName);
    void run_test(const OUString& sFileName);
    void test_failed();
    void test_success();
    std::vector< OUString > get_subdirnames( const OUString& sDirName );

public:
    Coverage();
    virtual ~Coverage() override;

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
    , m_nb_tests_ok(0)
{
}

Coverage::~Coverage()
{
    fprintf(stderr,"basic coverage Summary : pass:%d\n", m_nb_tests_ok );
}

void Coverage::test_failed()
{
    CPPUNIT_FAIL(
        OUStringToOString(m_sCurrentTest, RTL_TEXTENCODING_UTF8).getStr());
}

void Coverage::test_success()
{
    m_nb_tests_ok += 1;
    fprintf(stderr,"%s,PASS\n", OUStringToOString( m_sCurrentTest, RTL_TEXTENCODING_UTF8 ).getStr() );
}

void Coverage::run_test(const OUString& sFileURL)
{
    m_sCurrentTest = sFileURL;
    bool bResult = false;
    MacroSnippet testMacro;
    testMacro.LoadSourceFromFile( sFileURL );
    testMacro.Compile();
    if( !testMacro.HasError() )
    {
        SbxVariableRef pResult = testMacro.Run();
        if( pResult.is() && pResult->GetInteger() == 1 )
        {
            bResult = true;
        }
    }
    if(bResult)
    {
        test_success();
    }
    else
    {
        test_failed();
    }
}

std::vector< OUString > Coverage::get_subdirnames( const OUString& sDirName )
{
    std::vector< OUString > sSubDirNames;
    osl::Directory aDir(sDirName);
    osl::DirectoryItem aItem;
    osl::FileStatus aFileStatus(osl_FileStatus_Mask_FileURL|osl_FileStatus_Mask_Type);

    if(osl::FileBase::E_None == aDir.open())
    {
        while (aDir.getNextItem(aItem) == osl::FileBase::E_None)
        {
            aItem.getFileStatus(aFileStatus);
            if(aFileStatus.isDirectory())
                sSubDirNames.push_back( aFileStatus.getFileURL() );
        }
    }
    return sSubDirNames;
}
void Coverage::process_directory(const OUString& sDirName)
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
                run_test(aFileStatus.getFileURL());
            }
        }
    }
    else
    {
    }
    fprintf(stderr,"end process directory\n");
}

void Coverage::Coverage_Iterator()
{
    OUString sDirName = m_directories.getURLFromSrc("/basic/qa/basic_coverage/");

    CPPUNIT_ASSERT(!sDirName.isEmpty());
    process_directory(sDirName); // any files in the root test dir are run in test harness default locale ( en-US )
    std::vector< OUString > sLangDirs = get_subdirnames( sDirName );

    for ( std::vector< OUString >::iterator it = sLangDirs.begin(), it_end = sLangDirs.end(); it != it_end; ++it )
    {
        OUString sDir( *it );
        sal_Int32 nSlash = (*it).lastIndexOf('/');
        if ( nSlash != -1 )
        {
            OUString sLangISO = sDir.copy( nSlash + 1 );
            LanguageTag aLocale( sLangISO );
            if ( aLocale.isValidBcp47() )
            {
                SvtSysLocaleOptions aLocalOptions;
                // set locale for test dir
                aLocalOptions.SetLocaleConfigString( sLangISO );
                process_directory(sDir);
            }
        }
    }
}

  CPPUNIT_TEST_SUITE_REGISTRATION(Coverage);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
