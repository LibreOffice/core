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


#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include "stringhelper.hxx"

namespace testOfHelperFunctions
{
    class test_valueequal : public CppUnit::TestFixture
    {
    public:
        void valueequal_001();

        CPPUNIT_TEST_SUITE( test_valueequal );
        CPPUNIT_TEST( valueequal_001 );
        CPPUNIT_TEST_SUITE_END( );
    };

    void test_valueequal::valueequal_001( )
    {
        rtl::OString sValue;
        rtl::OUString suValue("This is only a test of some helper functions");
        sValue <<= suValue;
        printf("'%s'\n", sValue.getStr());
    }

} // namespace testOfHelperFunctions

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION( testOfHelperFunctions::test_valueequal );

// -----------------------------------------------------------------------------
// This is only allowed to be in one file!
// NOADDITIONAL;

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
