/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>

#include <cppunit/plugin/TestPlugIn.h>

#if (defined UNX)

#include <stdio.h>

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <osl/file.hxx>
#include <rtl/ustring.hxx>
#include <sal/types.h>

#include <setallowedpaths.hxx>

#include "../../../osl/unx/file_impl.hxx"

using namespace osl;

namespace osl::qa {

static bool isForbidden(OUString const & path, sal_uInt32 flags) {
    OString utf8;
    auto const ok = path.convertToString(&utf8, RTL_TEXTENCODING_UTF8, RTL_UNICODETOTEXT_FLAGS_UNDEFINED_ERROR | RTL_UNICODETOTEXT_FLAGS_INVALID_ERROR);
    CPPUNIT_ASSERT(ok);
    return ::isForbidden(utf8, flags);
}

}

namespace osl_Forbidden
{

    class Forbidden : public CppUnit::TestFixture
    {
        OUString maScratchBad;
        OUString maScratchGood;
    public:
        void setUp() override
        {
            // create a directory to play in
            createTestDirectory(aTmpName3);
            OUString aBadURL = aTmpName3 + "/bad";
            OUString aGoodURL = aTmpName3 + "/good";
            createTestDirectory(aBadURL);
            createTestDirectory(aGoodURL);
            File::getSystemPathFromFileURL(aBadURL, maScratchBad);
            File::getSystemPathFromFileURL(aGoodURL, maScratchGood);
        }

        void tearDown() override
        {
            setAllowedPaths(u"");
            OUString aBadURL = aTmpName3 + "/bad";
            OUString aGoodURL = aTmpName3 + "/good";
            deleteTestDirectory(aBadURL);
            deleteTestDirectory(aGoodURL);
            deleteTestDirectory(aTmpName3);
        }

        void forbidden()
        {
            setAllowedPaths(maScratchGood);

            // check some corner cases first
            CPPUNIT_ASSERT_EQUAL_MESSAGE("read bad should be forbidden",
                                         true, osl::qa::isForbidden(".", osl_File_OpenFlag_Read));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("read bad should be forbidden",
                                         true, osl::qa::isForbidden("", osl_File_OpenFlag_Read));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("read bad should be forbidden",
                                         true, osl::qa::isForbidden("a", osl_File_OpenFlag_Read));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("read bad should be forbidden",
                                         true, osl::qa::isForbidden("/", osl_File_OpenFlag_Read));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("read from non-existent should be allowed",
                                         false, osl::qa::isForbidden(maScratchGood + "/notthere/file", osl_File_OpenFlag_Read));

            CPPUNIT_ASSERT_EQUAL_MESSAGE("read bad should be forbidden",
                                         true, osl::qa::isForbidden(maScratchBad, osl_File_OpenFlag_Read));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("read from good should be allowed",
                                         false, osl::qa::isForbidden(maScratchGood, osl_File_OpenFlag_Read));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write to good should be forbidden",
                                         true, osl::qa::isForbidden(maScratchGood, osl_File_OpenFlag_Write));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("create in good should be forbidden",
                                         true, osl::qa::isForbidden(maScratchGood, osl_File_OpenFlag_Create));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("exec from good should be forbidden",
                                         true, osl::qa::isForbidden(maScratchGood, 0x80));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write to non-existent should be forbidden",
                                         true, osl::qa::isForbidden(maScratchBad + "/notthere", osl_File_OpenFlag_Write));

            setAllowedPaths(Concat2View("w:" + maScratchGood + ":x:" + maScratchBad));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("read bad should be forbidden",
                                         true, osl::qa::isForbidden(maScratchBad, osl_File_OpenFlag_Read));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("read from good should be allowed", // w implies 'r'
                                         false, osl::qa::isForbidden(maScratchGood, osl_File_OpenFlag_Read));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write to good should be allowed",
                                         false, osl::qa::isForbidden(maScratchGood, osl_File_OpenFlag_Write));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("exec from good should be forbidden",
                                         true, osl::qa::isForbidden(maScratchGood, 0x80));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("exec from bad should be allowed",
                                         false, osl::qa::isForbidden(maScratchBad, 0x80));

            setAllowedPaths(Concat2View(":r:" + maScratchBad));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write to non-existent should be forbidden",
                                         true, osl::qa::isForbidden(maScratchGood + "/notthere", osl_File_OpenFlag_Write));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write to non-existent should be forbidden 2",
                                         true, osl::qa::isForbidden(maScratchGood + "/notthere/file", osl_File_OpenFlag_Write));

            setAllowedPaths(Concat2View(":r:" + maScratchBad + ":w:" + maScratchGood + "/notthere"));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write to non-existent should be allowed",
                                         false, osl::qa::isForbidden(maScratchGood + "/notthere", osl_File_OpenFlag_Write));
            CPPUNIT_ASSERT_EQUAL_MESSAGE("write to non-existent should be allowed 2",
                                         false, osl::qa::isForbidden(maScratchGood + "/notthere/file", osl_File_OpenFlag_Write));
        }

/*
        void open()
        {
            setAllowedPaths(maScratchGood);
            File testFile(maScratchBad + "/open");
            auto nError1 = testFile.open(osl_File_OpenFlag_Read | osl_File_OpenFlag_Write);
            CPPUNIT_ASSERT_EQUAL_MESSAGE("disabled path allowed", osl::FileBase::E_ACCES, nError1);
            deleteTestFile(testFile.getURL());
        }

        void copy()
        {
            setAllowedPaths("w:" + maScratchGood);
            File testGood(maScratchGood + "/good");
            File testGoodTo(maScratchGood + "/good_to");
            File testBad(maScratchBad + "/bad");

            auto nError1 = testGood.open(osl_File_OpenFlag_Create);
            CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, nError1);

            auto nErrorCopy = File::copy(maScratchGood + "/good", maScratchGood + "/good_to");
            CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None, nErrorCopy);

            auto nErrorCopyBad = File::copy(maScratchGood + "/good_to", maScratchBad + "/bad");
            CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_ACCES, nErrorCopyBad);

            deleteTestFile(maScratchGood + "/good_to");
            deleteTestFile(maScratchGood + "/good");
        }

        void nextTests()
        {
            // more entry points to test
#if 0
            auto nError1 = File::move(aTmpName4, aCanURL1);
            auto nError2 = File::remove(aTmpName4);
            auto nError3 = File::setAttributes(aTmpName6, osl_File_Attribute_ReadOnly);
            bool bOk = osl_getSystemTime(pTV_current);
            CPPUNIT_ASSERT(bOk);
            auto nError4 = File::setTime(aTmpName6, *pTV_current, *pTV_current, *pTV_current);
            CPPUNIT_ASSERT_EQUAL_MESSAGE(errorToStr(nError2).getStr(), osl::FileBase::E_None, nError2);
#endif
        }
*/
        CPPUNIT_TEST_SUITE(Forbidden);
        CPPUNIT_TEST(forbidden);
//        CPPUNIT_TEST(open);
//        CPPUNIT_TEST(copy);
//        CPPUNIT_TEST(nextTests);
        CPPUNIT_TEST_SUITE_END();

    //TODO: The following parts are mainly copied from sal/qa/osl/file/osl_File.cxx and
    // sal/qa/osl/file/osl_File_Const.h (but the latter cannot be included directly, as its use of
    // static C++ objects would cause issues with the use of gb_CppunitTest_use_library_objects in
    // this test's sal/CppunitTest_sal_osl_file_details.mk):
    private:
        static OUString getTempDirectoryURL_()
        {
            OUString aDir;
            CPPUNIT_ASSERT_EQUAL_MESSAGE("couldn't get system temp URL",
                osl::FileBase::E_None, osl::FileBase::getTempDirURL(aDir));
            // This resolves symlinks in the temp path if any
            CPPUNIT_ASSERT_EQUAL(osl::FileBase::E_None,
                                 osl::FileBase::getAbsoluteFileURL(aDir, aDir, aDir));
            return aDir;
        }

        OUString aTempDirectoryURL;

        OUString aTmpName3;

        /** simple version to judge if a file name or directory name is a URL or a system path, just to see if it
            is start with "file:///";.
        */
        static bool isURL(const OUString& pathname)
        {
            return pathname.startsWith(u"file:///");
        }

        /** create a temp test directory using OUString name of full qualified URL or system path.
        */
        static void createTestDirectory(const OUString& dirname)
        {
            OUString aPathURL   = dirname.copy(0);
            osl::FileBase::RC nError;

            if (!isURL(dirname))
                osl::FileBase::getFileURLFromSystemPath(dirname, aPathURL); // convert if not full qualified URL
            nError = Directory::create(aPathURL);
            if ((nError != osl::FileBase::E_None) && (nError != osl::FileBase::E_EXIST))
                printf("createTestDirectory failed: %d!\n", int(nError));
        }

        /** delete a temp test directory using OUString name of full qualified URL or system path.
        */
        static void deleteTestDirectory(const OUString& dirname)
        {
            OUString aPathURL = dirname.copy(0);
            if (!isURL(dirname))
                osl::FileBase::getFileURLFromSystemPath(dirname, aPathURL); // convert if not full qualified URL

            Directory testDir(aPathURL);
            if (testDir.isOpen())
                testDir.close();  // close if still open.

            osl::FileBase::RC nError = Directory::remove(aPathURL);

            OString strError = "In deleteTestDirectory function: remove Directory " +
                OUStringToOString(aPathURL, RTL_TEXTENCODING_ASCII_US) + " -> result: " + OString::number(nError);
            CPPUNIT_ASSERT_MESSAGE(strError.getStr(), (osl::FileBase::E_None == nError) || (nError == osl::FileBase::E_NOENT));
        }

    public:
        Forbidden():
            aTempDirectoryURL(getTempDirectoryURL_()),
            aTmpName3( aTempDirectoryURL + "/tmpdir" )
        {}
    };

    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Forbidden::Forbidden, "osl_Forbidden");

    CPPUNIT_REGISTRY_ADD_TO_DEFAULT("osl_Forbidden");
}
#endif

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
