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
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

namespace basegfx3d
{

class b3dhommatrix : public CppUnit::TestFixture
{
public:

    // insert your test code here.
    // this is only demonstration code
    void EmptyMethod()
    {
          // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b3dhommatrix);
    CPPUNIT_TEST(EmptyMethod);
    CPPUNIT_TEST_SUITE_END();
}; // class b3dhommatrix

class b3dpoint : public CppUnit::TestFixture
{
public:

    // insert your test code here.
    // this is only demonstration code
    void EmptyMethod()
    {
          // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b3dpoint);
    CPPUNIT_TEST(EmptyMethod);
    CPPUNIT_TEST_SUITE_END();
}; // class b3dpoint

class b3drange : public CppUnit::TestFixture
{
public:

    // insert your test code here.
    void EmptyMethod()
    {
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b3drange);
    CPPUNIT_TEST(EmptyMethod);
    CPPUNIT_TEST_SUITE_END();
}; // class b3drange

class b3dtuple : public CppUnit::TestFixture
{
public:

    // insert your test code here.
    // this is only demonstration code
    void EmptyMethod()
    {
          // CPPUNIT_ASSERT_MESSAGE("a message", 1 == 1);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b3dtuple);
    CPPUNIT_TEST(EmptyMethod);
    CPPUNIT_TEST_SUITE_END();
}; // class b3dtuple

class b3dvector : public CppUnit::TestFixture
{
public:

    // insert your test code here.
    void EmptyMethod()
    {
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(b3dvector);
    CPPUNIT_TEST(EmptyMethod);
    CPPUNIT_TEST_SUITE_END();
}; // class b3dvector

CPPUNIT_TEST_SUITE_REGISTRATION(basegfx3d::b3dhommatrix);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx3d::b3dpoint);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx3d::b3drange);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx3d::b3dtuple);
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx3d::b3dvector);
} // namespace basegfx3d

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
// NOADDITIONAL;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
