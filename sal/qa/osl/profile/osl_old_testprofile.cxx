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

// LLA:
// this file is converted to use with testshl2
// original was placed in sal/test/textenc.cxx


// -----------------------------------------------------------------------------
#include <stdio.h>
#include <osl/profile.h>
#include <sal/cppunit.h>


//==================================================================================================
// -----------------------------------------------------------------------------
namespace osl_Profile
{
    class oldtests : public CppUnit::TestFixture
    {
    public:
        void test_profile();

        CPPUNIT_TEST_SUITE( oldtests );
        CPPUNIT_TEST( test_profile );
        CPPUNIT_TEST_SUITE_END( );
    };

void oldtests::test_profile(void)
{
    oslProfile hProfile;
    rtl_uString* ustrProfileName=0;
    rtl_uString* ustrProfileName2=0;

    rtl_uString_newFromAscii(&ustrProfileName,"//./tmp/soffice.ini");
    rtl_uString_newFromAscii(&ustrProfileName2,"//./tmp/not_existing_path/soffice.ini");


    // successful write
    hProfile = osl_openProfile( ustrProfileName, 0 );
    if (hProfile != 0)
    {
        if (! osl_writeProfileBool( hProfile, "testsection", "testbool", 1 ))
            printf( "### cannot write into init file!\n" );

        osl_closeProfile( hProfile );
    }

    // unsuccessful write
    hProfile = osl_openProfile( ustrProfileName2, 0 );
    if (hProfile != 0)
    {
        if (osl_writeProfileBool( hProfile, "testsection", "testbool", 1 ))
            printf( "### unexpected success writing into test2.ini!\n" );

        osl_closeProfile( hProfile );
    }

    rtl_uString_release(ustrProfileName);
    rtl_uString_release(ustrProfileName2);
}

} // namespace osl_Profile

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION( osl_Profile::oldtests );

// -----------------------------------------------------------------------------
CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
