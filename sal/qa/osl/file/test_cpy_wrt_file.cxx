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
        err = FileBase::createTempFile(nullptr, nullptr, &aTmpFile);
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
        err = tmp_file.write(static_cast<void*>(buffer), sizeof(buffer), written);
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

// register test suites
CPPUNIT_TEST_SUITE_REGISTRATION(test_osl_writeFile);

CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
