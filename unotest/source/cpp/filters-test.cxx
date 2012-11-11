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

#include <unotest/filters-test.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <rtl/cipher.h>

#include "cppunit/TestAssert.h"

namespace test {

void decode(const rtl::OUString& rIn, const rtl::OUString &rOut)
{
    rtlCipher cipher = rtl_cipher_create(rtl_Cipher_AlgorithmARCFOUR, rtl_Cipher_ModeStream);
    CPPUNIT_ASSERT_MESSAGE("cipher creation failed", cipher != 0);

    //mcrypt --bare -a arcfour -o hex -k 435645 -s 3
    const sal_uInt8 aKey[3] = {'C', 'V', 'E'};

    rtlCipherError result = rtl_cipher_init(cipher, rtl_Cipher_DirectionDecode, aKey, SAL_N_ELEMENTS(aKey), 0, 0);

    CPPUNIT_ASSERT_MESSAGE("cipher init failed", result == rtl_Cipher_E_None);

    osl::File aIn(rIn);
    CPPUNIT_ASSERT(osl::FileBase::E_None == aIn.open(osl_File_OpenFlag_Read));

    osl::File aOut(rOut);
    CPPUNIT_ASSERT(osl::FileBase::E_None == aOut.open(osl_File_OpenFlag_Write));

    sal_uInt8 in[8192];
    sal_uInt8 out[8192];
    sal_uInt64 nBytesRead, nBytesWritten;
    while(1)
    {
        CPPUNIT_ASSERT(osl::FileBase::E_None == aIn.read(in, sizeof(in), nBytesRead));
        if (!nBytesRead)
            break;
        CPPUNIT_ASSERT(rtl_Cipher_E_None == rtl_cipher_decode(cipher, in, nBytesRead, out, sizeof(out)));
        CPPUNIT_ASSERT(osl::FileBase::E_None == aOut.write(out, nBytesRead, nBytesWritten));
        CPPUNIT_ASSERT(nBytesRead == nBytesWritten);
    }

    rtl_cipher_destroy(cipher);
}

void FiltersTest::recursiveScan(filterStatus nExpected,
    const rtl::OUString &rFilter, const rtl::OUString &rURL,
    const rtl::OUString &rUserData, unsigned int nFilterFlags,
    unsigned int nClipboardID, unsigned int nFilterVersion)
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
        {
            recursiveScan(nExpected, rFilter, sURL, rUserData,
                nFilterFlags, nClipboardID, nFilterVersion);
        }
        else
        {
            rtl::OUString sTmpFile;
            bool bEncrypted = false;

            sal_Int32 nLastSlash = sURL.lastIndexOf('/');

            if ((nLastSlash != -1) && (nLastSlash+1 < sURL.getLength()))
            {
                //ignore .files
                if (sURL.getStr()[nLastSlash+1] == '.')
                    continue;

                if (
                    (sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("BID"), nLastSlash+1)) ||
                    (sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("CVE"), nLastSlash+1)) ||
                    (sURL.matchAsciiL(RTL_CONSTASCII_STRINGPARAM("EDB"), nLastSlash+1))
                   )
                {
                    bEncrypted = true;
                }
            }

            rtl::OString aRes(rtl::OUStringToOString(sURL,
                osl_getThreadTextEncoding()));

            if (bEncrypted)
            {
                CPPUNIT_ASSERT(osl::FileBase::E_None == osl::FileBase::createTempFile(NULL, NULL, &sTmpFile));
                decode(sURL, sTmpFile);
                sURL = sTmpFile;
            }

            //output name early, so in the case of a hang, the name of
            //the hanging input file is visible
            fprintf(stderr, "%s,", aRes.getStr());
            sal_uInt32 nStartTime = osl_getGlobalTimer();
            bool bRes = load(rFilter, sURL, rUserData, nFilterFlags,
                nClipboardID, nFilterVersion);
            sal_uInt32 nEndTime = osl_getGlobalTimer();

            if (bEncrypted)
                CPPUNIT_ASSERT(osl::FileBase::E_None == osl::File::remove(sTmpFile));

            fprintf(stderr, "%s,%" SAL_PRIuUINT32"\n",
                bRes?"Pass":"Fail",nEndTime-nStartTime);
            if (nExpected == test::indeterminate)
                continue;
            CPPUNIT_ASSERT_MESSAGE(aRes.getStr(), bRes == (nExpected == test::pass));
        }
    }
    CPPUNIT_ASSERT(osl::FileBase::E_None == aDir.close());
}

void FiltersTest::testDir(const rtl::OUString &rFilter,
    const rtl::OUString &rURL, const rtl::OUString &rUserData,
    unsigned int nFilterFlags, unsigned int nClipboardID,
    unsigned int nFilterVersion)
{
    fprintf(stderr, "File tested,Test Result,Execution Time (ms)\n");
    recursiveScan(test::pass, rFilter,
        rURL + rtl::OUString("pass"),
        rUserData, nFilterFlags, nClipboardID, nFilterVersion);
    recursiveScan(test::fail, rFilter,
        rURL + rtl::OUString("fail"),
        rUserData, nFilterFlags, nClipboardID, nFilterVersion);
    recursiveScan(test::indeterminate, rFilter,
        rURL + rtl::OUString("indeterminate"),
        rUserData, nFilterFlags, nClipboardID, nFilterVersion);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
