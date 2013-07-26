/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <unotest/filters-test.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <rtl/cipher.h>

#include "cppunit/TestAssert.h"

namespace test {

void decode(const OUString& rIn, const OUString &rOut)
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
    const OUString &rFilter, const OUString &rURL,
    const OUString &rUserData, unsigned int nFilterFlags,
    unsigned int nClipboardID, unsigned int nFilterVersion)
{
    osl::Directory aDir(rURL);

    CPPUNIT_ASSERT(osl::FileBase::E_None == aDir.open());
    osl::DirectoryItem aItem;
    osl::FileStatus aFileStatus(osl_FileStatus_Mask_FileURL|osl_FileStatus_Mask_Type);
    while (aDir.getNextItem(aItem) == osl::FileBase::E_None)
    {
        aItem.getFileStatus(aFileStatus);
        OUString sURL = aFileStatus.getFileURL();
        if (aFileStatus.getFileType() == osl::FileStatus::Directory)
        {
            recursiveScan(nExpected, rFilter, sURL, rUserData,
                nFilterFlags, nClipboardID, nFilterVersion);
        }
        else
        {
            OUString sTmpFile;
            bool bEncrypted = false;

            sal_Int32 nLastSlash = sURL.lastIndexOf('/');

            if ((nLastSlash != -1) && (nLastSlash+1 < sURL.getLength()))
            {
                //ignore .files
                if (sURL.getStr()[nLastSlash+1] == '.')
                    continue;

                if (
                    (sURL.match("BID", nLastSlash+1)) ||
                    (sURL.match("CVE", nLastSlash+1)) ||
                    (sURL.match("EDB", nLastSlash+1))
                   )
                {
                    bEncrypted = true;
                }
            }

            OString aRes(OUStringToOString(sURL,
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

void FiltersTest::testDir(const OUString &rFilter,
    const OUString &rURL, const OUString &rUserData,
    unsigned int nFilterFlags, unsigned int nClipboardID,
    unsigned int nFilterVersion)
{
    fprintf(stderr, "File tested,Test Result,Execution Time (ms)\n");
    recursiveScan(test::pass, rFilter,
        rURL + OUString("pass"),
        rUserData, nFilterFlags, nClipboardID, nFilterVersion);
    recursiveScan(test::fail, rFilter,
        rURL + OUString("fail"),
        rUserData, nFilterFlags, nClipboardID, nFilterVersion);
    recursiveScan(test::indeterminate, rFilter,
        rURL + OUString("indeterminate"),
        rUserData, nFilterFlags, nClipboardID, nFilterVersion);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
