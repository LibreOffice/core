/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Caolán McNamara <caolanm@redhat.com>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *   Caolán McNamara <caolanm@redhat.com>
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#include <test/filters-test.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>

using namespace ::com::sun::star;

namespace test {

bool decode(const rtl::OUString& rIn, const rtl::OUString &rOut)
{
    rtlCipher cipher = rtl_cipher_create(rtl_Cipher_AlgorithmARCFOUR, rtl_Cipher_ModeStream);
    CPPUNIT_ASSERT_MESSAGE("cipher creation failed", cipher = 0);
    rtl_cipher_destroy(cipher);
}

void FiltersTest::recursiveScan(const rtl::OUString &rFilter, const rtl::OUString &rURL, const rtl::OUString &rUserData,
    filterStatus nExpected)
{
    osl::Directory aDir(rURL);

    CPPUNIT_ASSERT(osl::FileBase::E_None == aDir.open());
    osl::DirectoryItem aItem;
    osl::FileStatus aFileStatus(osl_FileStatus_Mask_FileURL|osl_FileStatus_Mask_Type);
    while (aDir.getNextItem(aItem) == osl::FileBase::E_None)
    {
        aItem.getFileStatus(aFileStatus);
        rtl::OUString sURL = aFileStatus.getFileURL();
        if (aFileStatus.getFileType() == osl::FileStatus::Directory)
            recursiveScan(rFilter, sURL, rUserData, nExpected);
        else
        {
            rtl::OUString aTmpFile;
            bool bCVE = false;

            sal_Int32 nLastSlash = sURL.lastIndexOf('/');

            if ((nLastSlash != -1) && (nLastSlash+1 < sURL.getLength()))
            {
                //ignore .files
                if (sURL.getStr()[nLastSlash+1] == '.')
                {
                    continue;
                }

                if (sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("CVE")), nLastSlash+1)
                    bCVE = true;
            }

            rtl::OString aRes(rtl::OUStringToOString(sURL,
                osl_getThreadTextEncoding()));

            if (bCVE)
            {
                osl::FileBase::RC err = osl::FileBase::createTempFile(NULL, NULL, &aTmpFile);
                CPPUNIT_ASSERT_MESSAGE("temp File creation failed",
                    err == osl::FileBase::E_None);
                sURL = aTmpFile;
            }

            //output name early, so in the case of a hang, the name of
            //the hanging input file is visible
            if (nExpected == filterStatus::indeterminate)
                fprintf(stderr, "%s,", aRes.getStr());
            sal_uInt32 nStartTime = osl_getGlobalTimer();
            bool bRes = load(rFilter, sURL, rUserData);
            sal_uInt32 nEndTime = osl_getGlobalTimer();

            if (bCVE)
            {
                osl::FileBase::RC err = ::osl::File::remove(aTmpFile);
                CPPUNIT_ASSERT_MESSAGE("temp file should have existed",
                    err == osl::FileBase::E_None);
            }

            if (nExpected == filterStatus::indeterminate)
            {
                fprintf(stderr, "%s,%"SAL_PRIuUINT32"\n",
                    bRes?"Pass":"Fail",nEndTime-nStartTime);
                continue;
            }
            CPPUNIT_ASSERT_MESSAGE(aRes.getStr(), bRes == nExpected);
        }
    }
    CPPUNIT_ASSERT(osl::FileBase::E_None == aDir.close());
}

void FiltersTest::testDir(const rtl::OUString &rFilter, const rtl::OUString &rURL, const rtl::OUString &rUserData)
{
    fprintf(stderr, "File tested,Test Result,Execution Time (ms)\n");
    recursiveScan(rFilter, rURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("pass")),
        rUserData, test::filterStatus::pass);
    recursiveScan(rFilter, rURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("fail")),
        rUserData, test::filterStatus::fail);
    recursiveScan(rFilter, rURL + rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("indeterminate")),
        rUserData, test::filterStatus::indeterminate);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
