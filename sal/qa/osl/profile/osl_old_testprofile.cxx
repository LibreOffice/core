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
#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"
#include "cppunit/plugin/TestPlugIn.h"

#include <stdio.h>
#include <osl/profile.h>



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


CPPUNIT_TEST_SUITE_REGISTRATION( osl_Profile::oldtests );


CPPUNIT_PLUGIN_IMPLEMENT();

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
