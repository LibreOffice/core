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


#ifndef CHART_MACROS_HXX
#define CHART_MACROS_HXX

#include <typeinfo>

/// creates a unicode-string from an ASCII string
#define C2U(constAsciiStr) (::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( constAsciiStr ) ))

/** shows an error-box for an exception ex
    else-branch necessary to avoid warning
*/
#if OSL_DEBUG_LEVEL > 0
#define ASSERT_EXCEPTION(ex)                   \
  OSL_ENSURE( false, ::rtl::OUStringToOString( \
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Exception caught. Type: " )) +\
    ::rtl::OUString::createFromAscii( typeid( ex ).name()) +\
    ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ", Message: " )) +\
    ex.Message, RTL_TEXTENCODING_ASCII_US ).getStr())
#else
//avoid compilation warnings
#define ASSERT_EXCEPTION(ex) (void)(ex)
#endif

#define U2C(ouString) (::rtl::OUStringToOString(ouString,RTL_TEXTENCODING_ASCII_US).getStr())

// CHART_MACROS_HXX
#endif
