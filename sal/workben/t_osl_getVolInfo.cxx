/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <sal/types.h>
#include <cppunit/simpleheader.hxx>
#include <osl/file.h>
#include <rtl/ustring.hxx>

#ifdef _WIN32
#   define  TEST_PATH_1 "c:\\"
#   define  TEST_PATH_2 "c:\\mnt\\MSDN"
#   define  TEST_PATH_3 "c:\\Program Files"
#   define  TEST_PATH_4 "\\\\Tra-1\\mnt\\c"
#   define  TEST_PATH_5 "\\\\Tra-1\\mnt"
#   define  TEST_PATH_6 "\\\\Tra-1\\mnt\\c\\"
#else // UNX
#   define  TEST_PATH_1 "/net/athene/export/home/tra"
#   define  TEST_PATH_2 "/net/athene/export/home/tra/"
#   define  TEST_PATH_3 "/"
#   define  TEST_PATH_4 "."
#   define  TEST_PATH_5 "/net/athene/export/home/tra/projects"
#   define  TEST_PATH_6 "/blah"
#endif

void test_getVolumeInformation(const rtl::OUString& path_url)
    {
        oslVolumeInfo vi;
        memset((void*)&vi, 0, sizeof(vi));
        vi.uStructSize   = sizeof(vi);
        vi.pDeviceHandle = NULL;

        oslFileError err = osl_getVolumeInformation(
            path_url.pData,
            &vi,
            osl_VolumeInfo_Mask_Attributes |
            osl_VolumeInfo_Mask_TotalSpace |
            osl_VolumeInfo_Mask_UsedSpace |
            osl_VolumeInfo_Mask_FreeSpace |
            osl_VolumeInfo_Mask_MaxNameLength |
            osl_VolumeInfo_Mask_MaxPathLength |
            osl_VolumeInfo_Mask_FileSystemName |
            osl_VolumeInfo_Mask_DeviceHandle);

        CPPUNIT_ASSERT_MESSAGE
        (
            "osl_getVolumeInformation failed",
            err == osl_File_E_None
        );
    }

class TestClass_osl_getVolumeInformation : public CppUnit::TestFixture
{
public:

    /*-------------------------------------
        Start a process and join with this
        process specify a timeout so that
        osl_joinProcessWithTimeout returns
        osl_Process_E_TimedOut
     -------------------------------------*/

    void test_osl_getVolumeInformation()
    {
        rtl::OUString path = rtl::OUString(TEST_PATH_1);
        rtl::OUString path_url;
        osl_getFileURLFromSystemPath(path.pData, &path_url.pData);
        test_getVolumeInformation(path_url);

        path = rtl::OUString(TEST_PATH_2);
        osl_getFileURLFromSystemPath(path.pData, &path_url.pData);
        test_getVolumeInformation(path_url);

        path = rtl::OUString(TEST_PATH_3);
        osl_getFileURLFromSystemPath(path.pData, &path_url.pData);
        test_getVolumeInformation(path_url);

        path = rtl::OUString(TEST_PATH_4);
        osl_getFileURLFromSystemPath(path.pData, &path_url.pData);
        test_getVolumeInformation(path_url);

        path = rtl::OUString(TEST_PATH_5);
        osl_getFileURLFromSystemPath(path.pData, &path_url.pData);
        test_getVolumeInformation(path_url);

        path = rtl::OUString(TEST_PATH_6);
        osl_getFileURLFromSystemPath(path.pData, &path_url.pData);
        test_getVolumeInformation(path_url);
    }

    CPPUNIT_TEST_SUITE( TestClass_osl_getVolumeInformation );
    CPPUNIT_TEST( test_osl_getVolumeInformation );
    CPPUNIT_TEST_SUITE_END( );
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TestClass_osl_getVolumeInformation, "Test osl_getVolumeInformation");

NOADDITIONAL;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
