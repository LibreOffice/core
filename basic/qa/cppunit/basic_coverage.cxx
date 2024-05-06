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
#include <i18nlangtag/languagetag.hxx>
#include <unotools/syslocaleoptions.hxx>

namespace
{

class Coverage : public test::BootstrapFixture
{
private:
    void process_directory(const OUString& sDirName);
    std::vector< OUString > get_subdirnames( const OUString& sDirName );

public:
    Coverage();

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
{
}

std::vector< OUString > Coverage::get_subdirnames( const OUString& sDirName )
{
    std::vector< OUString > sSubDirNames;
    osl::Directory aDir(sDirName);
    osl::DirectoryItem aItem;
    osl::FileStatus aFileStatus(osl_FileStatus_Mask_FileURL|osl_FileStatus_Mask_Type);

    if(aDir.open() == osl::FileBase::E_None)
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
    OUString sMacroUtilsURL = m_directories.getURLFromSrc(u"basic/qa/cppunit/_test_asserts.bas");

    if(aDir.open() == osl::FileBase::E_None)
    {
        while (aDir.getNextItem(aItem) == osl::FileBase::E_None)
        {
            aItem.getFileStatus(aFileStatus);
            if(aFileStatus.isRegular())
            {
                OUString sFileURL = aFileStatus.getFileURL();
                if (sFileURL.endsWith(".bas"))
                {
                    MacroSnippet testMacro;
                    testMacro.LoadSourceFromFile(u"TestUtil"_ustr, sMacroUtilsURL);
                    testMacro.LoadSourceFromFile(u"TestModule"_ustr, sFileURL);
                    SbxVariableRef pReturn = testMacro.Run();
                    CPPUNIT_ASSERT_MESSAGE("No return variable huh?", pReturn.is());
                    fprintf(stderr, "macro result for %s\n", OUStringToOString(sFileURL,RTL_TEXTENCODING_UTF8).getStr());
                    fprintf(stderr, "macro returned:\n%s\n",
                            OUStringToOString(pReturn->GetOUString(), RTL_TEXTENCODING_UTF8).getStr());
                    CPPUNIT_ASSERT_EQUAL_MESSAGE("Result not as expected", u"OK"_ustr,
                                                 pReturn->GetOUString());
                }
            }
        }
    }
    fprintf(stderr,"end process directory\n");
}

void Coverage::Coverage_Iterator()
{
    OUString sDirName = m_directories.getURLFromSrc(u"basic/qa/basic_coverage");

    CPPUNIT_ASSERT(!sDirName.isEmpty());
    process_directory(sDirName); // any files in the root test dir are run in test harness default locale ( en-US )
    std::vector< OUString > sLangDirs = get_subdirnames( sDirName );

    for (auto const& langDir : sLangDirs)
    {
        sal_Int32 nSlash = langDir.lastIndexOf('/');
        if ( nSlash != -1 )
        {
            OUString sLangISO = langDir.copy( nSlash + 1 );
            LanguageTag aLocale( sLangISO );
            if ( aLocale.isValidBcp47() )
            {
                SvtSysLocaleOptions aLocalOptions;
                // set locale for test dir
                aLocalOptions.SetLocaleConfigString( sLangISO );
                process_directory(langDir);
            }
        }
    }
}

  CPPUNIT_TEST_SUITE_REGISTRATION(Coverage);

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
