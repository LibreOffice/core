/*************************************************************************
 *
 *  $RCSfile: osl_old_testprofile.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-05-03 09:01:24 $
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

// LLA:
// this file is converted to use with testshl2
// original was placed in sal/test/textenc.cxx


// -----------------------------------------------------------------------------
#include <stdio.h>
#include <osl/profile.h>

#include <cppunit/simpleheader.hxx>

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
    if (hProfile = osl_openProfile( ustrProfileName, 0 ))
    {
        if (! osl_writeProfileBool( hProfile, "testsection", "testbool", 1 ))
            printf( "### cannot write into init file!\n" );

        osl_closeProfile( hProfile );
    }

    // unsuccessful write
    if (hProfile = osl_openProfile( ustrProfileName2, 0 ))
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
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( osl_Profile::oldtests, "osl_Profile" );

// -----------------------------------------------------------------------------
NOADDITIONAL;
