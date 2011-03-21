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

//------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------
#include <osl_Socket_Const.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <osl/socket.hxx>
//------------------------------------------------------------------------
// helper functions
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// tests cases begins here
//------------------------------------------------------------------------

namespace osl_Socket
{

    class tests : public CppUnit::TestFixture
    {
    public:

        void test_001()
            {
                // _osl_getFullQualifiedDomainName( );
                oslSocketResult aResult;
                rtl::OUString suHostname = osl::SocketAddr::getLocalHostname(&aResult);
                CPPUNIT_ASSERT_MESSAGE("getLocalHostname failed", aResult == osl_Socket_Ok);
            }

        CPPUNIT_TEST_SUITE( tests );
        CPPUNIT_TEST( test_001 );
        CPPUNIT_TEST_SUITE_END();
    };

    CPPUNIT_TEST_SUITE_REGISTRATION(osl_Socket::tests);
}


// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.

/*#if (defined LINUX)

void RegisterAdditionalFunctions( FktRegFuncPtr _pFunc )
{
    // for cover lines in _osl_getFullQualifiedDomainName( )
    // STAR_OVERRIDE_DOMAINNAME is more an internal HACK for 5.2, which should remove from sal
    setenv( "STAR_OVERRIDE_DOMAINNAME", "PRC.Sun.COM", 0 );
}

#else*/

CPPUNIT_PLUGIN_IMPLEMENT();

//#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
