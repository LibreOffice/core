/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: t_osl_getVolInfo.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 09:09:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include <cppunit/simpleheader.hxx>
#include <osl/file.h>
#include <rtl/ustring.hxx>

#ifdef WNT
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

//------------------------------
//
//------------------------------

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

//------------------------------
//
//------------------------------

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
        rtl::OUString path = rtl::OUString::createFromAscii(TEST_PATH_1);
        rtl::OUString path_url;
        osl_getFileURLFromSystemPath(path.pData, &path_url.pData);
        test_getVolumeInformation(path_url);

        path = rtl::OUString::createFromAscii(TEST_PATH_2);
        osl_getFileURLFromSystemPath(path.pData, &path_url.pData);
        test_getVolumeInformation(path_url);

        path = rtl::OUString::createFromAscii(TEST_PATH_3);
        osl_getFileURLFromSystemPath(path.pData, &path_url.pData);
        test_getVolumeInformation(path_url);

        path = rtl::OUString::createFromAscii(TEST_PATH_4);
        osl_getFileURLFromSystemPath(path.pData, &path_url.pData);
        test_getVolumeInformation(path_url);

        path = rtl::OUString::createFromAscii(TEST_PATH_5);
        osl_getFileURLFromSystemPath(path.pData, &path_url.pData);
        test_getVolumeInformation(path_url);

        path = rtl::OUString::createFromAscii(TEST_PATH_6);
        osl_getFileURLFromSystemPath(path.pData, &path_url.pData);
        test_getVolumeInformation(path_url);
    }

    CPPUNIT_TEST_SUITE( TestClass_osl_getVolumeInformation );
    CPPUNIT_TEST( test_osl_getVolumeInformation );
    CPPUNIT_TEST_SUITE_END( );
};

CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( TestClass_osl_getVolumeInformation, "Test osl_getVolumeInformation");

NOADDITIONAL;

