/*************************************************************************
 *
 *  $RCSfile: osl_Socket_tests.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-01-05 21:23:33 $
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
 *  Copyright 2000 by Sun Microsystems,
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

//------------------------------------------------------------------------
// include files
//------------------------------------------------------------------------

#ifndef _OSL_SOCKET_CONST_H_
#include <osl_Socket_Const.h>
#endif

#include <cppunit/simpleheader.hxx>

#ifndef _OSL_SOCKET_HXX_
#include <osl/socket.hxx>
#endif
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
                volatile int dummy = 0;
                CPPUNIT_ASSERT_MESSAGE("getLocalHostname failed", aResult == osl_Socket_Ok);
            }

// -----------------------------------------------------------------------------

/*
#if defined UNX
        void getHostname_003()
        {
            struct hostent *pQualifiedHostByName;
            struct hostent *pHostByName;

            struct hostent  aHostByName, aQualifiedHostByName;
            sal_Char        pHostBuffer[ 2000 ];
            sal_Char          pQualifiedHostBuffer[ 2560 ];
            int     nErrorNo;

            pHostBuffer[0] = '\0';
            pQualifiedHostBuffer[0] = '\0';

            gethostbyname_r ("sceri.PRC.Sun.COM", &aQualifiedHostByName, pQualifiedHostBuffer, sizeof(pQualifiedHostBuffer), &pQualifiedHostByName, &nErrorNo);
            gethostbyname_r ("longshot.PRC.Sun.COM", &aHostByName, pHostBuffer, sizeof(pHostBuffer), &pHostByName, &nErrorNo);

            if ( pQualifiedHostByName )
                printf( "# getHostname_003: QualifiedHostByName!\n" );
            if ( pHostByName )
                printf( "# getHostname_003: HostByName!\n" );
        }
#endif
*/
        CPPUNIT_TEST_SUITE( tests );
        CPPUNIT_TEST( test_001 );
        //CPPUNIT_TEST( getHostname_003 );
        CPPUNIT_TEST_SUITE_END();

    };

    CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(osl_Socket::tests, "osl_SocketTest");
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

NOADDITIONAL;

//#endif
