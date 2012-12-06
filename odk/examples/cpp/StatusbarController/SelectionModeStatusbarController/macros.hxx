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

#ifndef GENERAL_MACROS_HXX
#define GENERAL_MACROS_HXX

#define STRINGIFY(x) _STRINGIFY(x)
#define _STRINGIFY(x) #x

#define C2U( constAsciiStr ) \
    ( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( constAsciiStr ) ) )

#define U2C( ouString ) \
    ( ::rtl::OUStringToOString( ouString, RTL_TEXTENCODING_ASCII_US ).getStr() )

#define GET_STR( OUSTR ) \
    ::rtl::OUStringToOString( OUSTR, RTL_TEXTENCODING_ASCII_US ).getStr()


#if OSL_DEBUG_LEVEL > 0
#define DISPLAY_EXCEPTION_ENSURE( ClassName, MethodName, e )    \
    ::rtl::OString sMessage( #ClassName "::" #MethodName ": caught an exception!\n" ); \
    sMessage += ::rtl::OString( e.Message.getStr(), e.Message.getLength(), RTL_TEXTENCODING_UTF8 ); \
    OSL_ENSURE( sal_False, sMessage.getStr() )
#else
#define DISPLAY_EXCEPTION_ENSURE( ClassName, MethodName, e ) ((void)e)
#endif

#if OSL_DEBUG_LEVEL > 0
#define DISPLAY_EXCEPTION( ClassName, MethodName, e )    \
    ::rtl::OString sMessage( #ClassName "::" #MethodName ": caught an exception!\n" ); \
    sMessage += ::rtl::OString( e.Message.getStr(), e.Message.getLength(), RTL_TEXTENCODING_UTF8 ); \
    OSL_TRACE( sMessage.getStr() )
#else
#define DISPLAY_EXCEPTION( ClassName, MethodName, e ) ((void)e)
#endif

#endif
