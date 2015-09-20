/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



//------------------------------------------------------------------------
//------------------------------------------------------------------------

#ifndef _OSL_SECURITY_CONST_H_
#define _OSL_SECURITY_CONST_H_

#if ( defined WNT )                     // Windows
//#define UNICODE
#include <tools/prewin.h>
// #include <windows.h>
#include <io.h>
#include <tools/postwin.h>
#endif

//------------------------------------------------------------------------
//------------------------------------------------------------------------
#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <osl/file.hxx>
#include <osl/security.hxx>

#include <stdlib.h>
#include <stdio.h>

#if ( defined UNX ) || ( defined OS2 )
#include <unistd.h>
#include <pwd.h>
#endif

#include "gtest/gtest.h"
// LLA: #include <testshl2/cmdlinebits.hxx>


#define BUFSIZE 1024
const char pTestString[17] = "Sun Microsystems";


#define OSLTEST_DECLARE_USTRING( str_name, str_value ) \
    ::rtl::OUString a##str_name = rtl::OUString::createFromAscii( str_value )

//------------------------------------------------------------------------
// condition names
//------------------------------------------------------------------------
OSLTEST_DECLARE_USTRING( TestSec,  "testsecurity" );
OSLTEST_DECLARE_USTRING( NullURL,  "" );

::rtl::OUString aLogonUser( aNullURL ), aLogonPasswd( aNullURL ), aFileServer( aNullURL ), aStringForward( aNullURL );
::rtl::OUString strUserName( aNullURL ) , strComputerName( aNullURL ) , strHomeDirectory( aNullURL );
::rtl::OUString strConfigDirectory( aNullURL ), strUserID( aNullURL );

sal_Bool isAdmin = sal_False;

#endif /* _OSL_SECURITY_CONST_H_ */
