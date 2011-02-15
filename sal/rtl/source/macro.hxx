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
#elif defined AIX
#define THIS_OS "AIX"
#elif defined OPENBSD
#define THIS_OS "OpenBSD"
#elif defined DRAGONFLY
#define THIS_OS "DragonFly"
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





/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
