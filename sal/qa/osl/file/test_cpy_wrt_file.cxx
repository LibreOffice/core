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

#include <sal/types.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <rtl/ustring.hxx>

using namespace osl;

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;

class test_osl_writeFile : public CppUnit::TestFixture
{
public:
    void wrt_file()
    {
        FileBase::RC err;

        //create a tempfile
        rtl::OUString aTmpFile;
        err = FileBase::createTempFile(NULL, NULL, &aTmpFile);
        CPPUNIT_ASSERT_MESSAGE("temp File creation failed", err == osl::FileBase::E_None);

        //now attempt to open with Create flag an existing file, should get E_EXIST
        File tmp_file(aTmpFile);
        err = tmp_file.open(osl_File_OpenFlag_Write | osl_File_OpenFlag_Create);

        rtl::OString sErrorMsg = "Expected that '";
        sErrorMsg += rtl::OUStringToOString(aTmpFile, RTL_TEXTENCODING_ASCII_US);
        sErrorMsg += "' would exist!";
        CPPUNIT_ASSERT_MESSAGE(sErrorMsg.getStr(), err == FileBase::E_EXIST);

        char buffer[1];
        sal_uInt64 written = 0;
        err = tmp_file.write((void*)buffer, sizeof(buffer), written);
        CPPUNIT_ASSERT_MESSAGE("write on unconnected file should fail",
            err != osl::FileBase::E_None && written == 0);

        err = tmp_file.sync();
        CPPUNIT_ASSERT_MESSAGE("sync on unconnected file should fail", err != FileBase::E_None);
        err = tmp_file.close();
        CPPUNIT_ASSERT_MESSAGE("close on unconnected file should fail", err != FileBase::E_None);

        err = ::osl::File::remove(aTmpFile);
        CPPUNIT_ASSERT_MESSAGE("temp file should have existed", err == FileBase::E_None);
    }

    CPPUNIT_TEST_SUITE(test_osl_writeFile);
    CPPUNIT_TEST(wrt_file);
    CPPUNIT_TEST_SUITE_END();
};

//#####################################
// register test suites
CPPUNIT_TEST_SUITE_REGISTRATION(test_osl_writeFile);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
