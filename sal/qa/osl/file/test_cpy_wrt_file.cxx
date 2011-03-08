/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sal.hxx"

#include <testshl/simpleheader.hxx>
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
