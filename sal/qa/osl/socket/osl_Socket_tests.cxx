/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */




#include <sal/types.h>
#include <osl_Socket_Const.h>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>
#include <osl/socket.hxx>








namespace osl_Socket
{

    class tests : public CppUnit::TestFixture
    {
    public:

        void test_001()
            {
                
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







/*#if (defined LINUX)

void RegisterAdditionalFunctions( FktRegFuncPtr _pFunc )
{
    
    
    setenv( "STAR_OVERRIDE_DOMAINNAME", "PRC.Sun.COM", 0 );
}

#else*/

CPPUNIT_PLUGIN_IMPLEMENT();



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
