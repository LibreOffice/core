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



#ifndef _OSL_MODULE_CONST_H_
#define _OSL_MODULE_CONST_H_


#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <osl/module.hxx>
#include <osl/file.hxx>

#include <testshl/simpleheader.hxx>

#if ( defined UNX ) || ( defined OS2 )  //Unix
#   include <unistd.h>
#endif
#if ( defined WNT )                     // Windows
#include <tools/prewin.h>
// #    include <windows.h>
#   include <io.h>
#include <tools/postwin.h>
#endif

#   define FILE_PREFIX          "file:///"

//Korea characters
::rtl::OUString aKname(
    RTL_CONSTASCII_STRINGPARAM(
        "/\xEC\x95\x88\xEB\x85\x95\xED\x95\x98\xEC\x84\xB8\xEC\x9A\x94"),
    RTL_TEXTENCODING_ISO_8859_1);
    // zero-extend the individual byte-sized characters one-to-one to individual
    // sal_Unicode-sized characters; not sure whether this is what was
    // intended...

//------------------------------------------------------------------------
// function pointer type.
//------------------------------------------------------------------------
typedef sal_Bool (* FuncPtr )( sal_Bool );


#endif /* _OSL_MODULE_CONST_H_ */
