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



#ifndef _RTL_MACRO_HXX
#define _RTL_MACRO_HXX

#include <rtl/bootstrap.h>
#include <rtl/ustring.hxx>
#include <osl/endian.h>

#if defined WIN32
#define THIS_OS "Windows"
#elif defined OS2
#define THIS_OS "OS2"
#elif defined SOLARIS
#define THIS_OS "Solaris"
#elif defined LINUX
#ifdef __FreeBSD_kernel__
#define THIS_OS "kFreeBSD"
#else
#define THIS_OS "Linux"
#endif
#elif defined MACOSX
#define THIS_OS "MacOSX"
#elif defined NETBSD
#define THIS_OS "NetBSD"
#elif defined FREEBSD
#define THIS_OS "FreeBSD"
#endif

#if ! defined THIS_OS
#error "unknown OS -- insert your OS identifier above"
this is inserted for the case that the preprocessor ignores error
#endif

#if defined X86_64
#    define THIS_ARCH "X86_64"
#elif defined INTEL
#    define THIS_ARCH "x86"
#elif defined POWERPC64
#    define THIS_ARCH "PowerPC_64"
#elif defined POWERPC
#    define THIS_ARCH "PowerPC"
#elif defined S390X
#    define THIS_ARCH "S390x"
#elif defined S390
#    define THIS_ARCH "S390"
#elif defined SPARC
#if defined IS_LP64
#    define THIS_ARCH "SPARC64"
#else
#    define THIS_ARCH "SPARC"
#endif
#elif defined MIPS
#    ifdef OSL_BIGENDIAN
#        define THIS_ARCH "MIPS_EB"
#    else
#        define THIS_ARCH "MIPS_EL"
#    endif
#elif defined ARM
#    ifdef __ARM_EABI__
#        define THIS_ARCH "ARM_EABI"
#    else
#        define THIS_ARCH "ARM_OABI"
#    endif
#elif defined IA64
#    define THIS_ARCH "IA64"
#elif defined M68K
#    define THIS_ARCH "M68K"
#elif defined HPPA
#    define THIS_ARCH "HPPA"
#elif defined AXP
#    define THIS_ARCH "ALPHA"
#endif

#if ! defined THIS_ARCH
#error "unknown ARCH -- insert your ARCH identifier above"
this is inserted for the case that the preprocessor ignores error
#endif

#endif





