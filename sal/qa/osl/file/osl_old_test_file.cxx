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

#include <osl/file.h>
#include <rtl/ustring.hxx>

#ifdef SAL_UNX
#define TEST_VOLUME ""
#elif defined _WIN32
#define TEST_VOLUME "Z:/"
#endif

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <utility>

namespace osl_test_file
{

class oldtestfile : public CppUnit::TestFixture
{
public:
    void test_file_001();
    void test_file_002();
    void test_file_004();

    CPPUNIT_TEST_SUITE( oldtestfile );
    CPPUNIT_TEST( test_file_001 );
    CPPUNIT_TEST( test_file_002 );
    CPPUNIT_TEST( test_file_004 );
    CPPUNIT_TEST_SUITE_END( );
};

const std::pair<OUString, OUString> aSource1[] = {
    { u"a", u"file:///" TEST_VOLUME "bla/a" },
    ///TODO: check if last slash must be omitted in resolved path.
//    { u"a/", u"file:///" TEST_VOLUME "bla/a" },
    { u"../a", u"file:///" TEST_VOLUME "a" },
    { u"a/..", u"file:///" TEST_VOLUME "bla/" },
    { u"a/../b", u"file:///" TEST_VOLUME "bla/b" },
    { u"..", u"file:///" TEST_VOLUME "" },
    { u"a/b/c/d", u"file:///" TEST_VOLUME "bla/a/b/c/d" },
    { u"a/./c", u"file:///" TEST_VOLUME "bla/a/c" },
    { u"a/././c", u"file:///" TEST_VOLUME "bla/a/c" },
    { u"file:///" TEST_VOLUME "bla1/blub", u"file:///" TEST_VOLUME "bla1/blub" },
};

const std::pair<OUString, OUString> aSource2[] = {
    { u"a", u"file:///" TEST_VOLUME "bla/blubs/schnubbel/a" },
    ///TODO: check if last slash must be omitted in resolved path.
//    { u"a/", u"file:///" TEST_VOLUME "bla/blubs/schnubbel/a" },
    { u"../a", u"file:///" TEST_VOLUME "bla/blubs/a" },
    { u"../../a", u"file:///" TEST_VOLUME "bla/a" },
    { u"../../../a", u"file:///" TEST_VOLUME "a" },
    { u"../../../a/b/c/d", u"file:///" TEST_VOLUME "a/b/c/d" },
};

void oldtestfile::test_file_001()
{
    OUString base1( "file:///" TEST_VOLUME "bla" );
    for (const auto& [rel, expected] : aSource1)
    {
        OUString target;
        oslFileError e = osl_getAbsoluteFileURL( base1.pData, rel.pData , &target.pData );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("failure #1",  osl_File_E_None, e );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("failure #1.1", expected, target);
    }
}

void oldtestfile::test_file_002()
{
    OUString base2( "file:///" TEST_VOLUME "bla/blubs/schnubbel" );
    for (const auto& [rel, expected] : aSource2)
    {
        OUString target;
        oslFileError e = osl_getAbsoluteFileURL( base2.pData, rel.pData , &target.pData );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("failure #2",  osl_File_E_None, e );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("failure #2.1", expected, target);
    }
}

void oldtestfile::test_file_004()
{
    OUString base4( "file:///" TEST_VOLUME "bla/" );
    for (const auto& [rel, expected] : aSource1)
    {
        OUString target;
        oslFileError e = osl_getAbsoluteFileURL( base4.pData, rel.pData , &target.pData );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("failure #10", osl_File_E_None, e );
        CPPUNIT_ASSERT_EQUAL_MESSAGE("failure #10.1", expected, target);
    }
}

} // namespace osl_test_file

CPPUNIT_TEST_SUITE_REGISTRATION( osl_test_file::oldtestfile);

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
