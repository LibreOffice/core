/*************************************************************************
 *
 *  $RCSfile: test_cpy_wrt_file.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-03 08:57:22 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include <cppunit/simpleheader.hxx>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <rtl/ustring.hxx>

using namespace osl;
using namespace rtl;

//########################################
#ifdef UNX
#   define COPY_SOURCE_PATH "/home/tr109510/ucbhelper.cxx"
#   define COPY_DEST_PATH "/mnt/mercury08/ucbhelper.cxx"
#else /* if WNT */
#   define COPY_SOURCE_PATH "d:\\msvcr70.dll"
#   define COPY_DEST_PATH "x:\\tra\\msvcr70.dll"
#endif

class test_osl_copyFile : public CppUnit::TestFixture
{
public:
    void cp_file()
    {
        rtl::OUString src_url;
        FileBase::getFileURLFromSystemPath(rtl::OUString::createFromAscii(COPY_SOURCE_PATH), src_url);

        rtl::OUString dest_url;
        FileBase::getFileURLFromSystemPath(rtl::OUString::createFromAscii(COPY_DEST_PATH), dest_url);

        FileBase::RC err = File::copy(src_url, dest_url);
        CPPUNIT_ASSERT_MESSAGE("Copy didn't recognized disk full", err != FileBase::E_None);
    }

    CPPUNIT_TEST_SUITE(test_osl_copyFile);
    CPPUNIT_TEST(cp_file);
    CPPUNIT_TEST_SUITE_END();
};

//########################################
#ifdef UNX
#   define WRITE_DEST_PATH "/mnt/mercury08/muell.tmp"
#else /* if WNT */
#   define WRITE_DEST_PATH "d:\\tmp_data.tmp"
#endif

class test_osl_writeFile : public CppUnit::TestFixture
{
public:
    void wrt_file()
    {
        rtl::OUString dest_url;
        FileBase::getFileURLFromSystemPath(rtl::OUString::createFromAscii(WRITE_DEST_PATH), dest_url);

        File tmp_file(dest_url);
        rtl::OUString suErrorMsg = rtl::OUString::createFromAscii("File creation failed: ")+ dest_url;
        FileBase::RC err = tmp_file.open(osl_File_OpenFlag_Write | osl_File_OpenFlag_Create);

        CPPUNIT_ASSERT_MESSAGE( suErrorMsg, err == FileBase::E_None || err == FileBase::E_EXIST );

        char buffer[50000];
        sal_uInt64 written = 0;
        err = tmp_file.write((void*)buffer, sizeof(buffer), written);

        err = tmp_file.sync();

        CPPUNIT_ASSERT_MESSAGE("Write didn't recognized disk full", err != FileBase::E_None);

        tmp_file.close();
    }

    CPPUNIT_TEST_SUITE(test_osl_writeFile);
    CPPUNIT_TEST(wrt_file);
    CPPUNIT_TEST_SUITE_END();
};

//#####################################
// register test suites
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test_osl_writeFile, "test_osl_writeFile");
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(test_osl_copyFile,  "test_osl_copyFile");

NOADDITIONAL;

