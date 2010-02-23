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
#include "precompiled_testshl2.hxx"
/*
 * LLA: 20040527
 */
#include <cppunit/simpleheader.hxx>

namespace skeleton
{
//------------------------------------------------------------------------
// testing constructors
//------------------------------------------------------------------------

    class  ctor : public CppUnit::TestFixture
    {
    public:
        ctor()
            {
                printf("ctor: called\n");
            }

        void ctor_test()
            {
                printf("ctor test: called\n");
            }
        void ctor_test2()
            {
                printf("ctor test2: called\n");
            }
        void ctor_test3()
            {
                printf("ctor test3: called\n");
            }

        CPPUNIT_TEST_SUITE(ctor);
        CPPUNIT_TEST(ctor_test);
        CPPUNIT_TEST(ctor_test2);
        CPPUNIT_TEST(ctor_test3);
        CPPUNIT_TEST_SUITE_END();
    };

    // -----------------------------------------------------------------------------
    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(skeleton::ctor, "skeleton");
} // unotest

// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.

NOADDITIONAL;
