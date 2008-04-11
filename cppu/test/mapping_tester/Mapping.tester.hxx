/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Mapping.tester.hxx,v $
 * $Revision: 1.3 $
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

#ifndef INCLUDED_Mapping_tester_hxx
#define INCLUDED_Mapping_tester_hxx


#include "../ObjectFactory/callee.hxx"


#ifdef CPPU_TEST_MAPPING_TESTER_TESTS_IMPL
# define CPPU_TEST_MAPPING_TESTER_TESTS_EXPORT SAL_DLLPUBLIC_EXPORT

#elif defined(CPPU_TEST_MAPPING_TESTER_TESTS_LIB)
# define CPPU_TEST_MAPPING_TESTER_TESTS_EXPORT extern

#elif defined(__GNUC__) && defined(HAVE_GCC_VISIBILITY_FEATURE)
# define CPPU_TEST_MAPPING_TESTER_TESTS_EXPORT extern __attribute__ ((weak))

#elif defined(__SUNPRO_CC)
# define CPPU_TEST_MAPPING_TESTER_TESTS_EXPORT extern
  extern rtl::OUString g_from_envDcp;
  extern rtl::OUString g_to_envDcp;
  extern rtl::OUString g_ref;
  extern rtl::OUString g_custom;
  extern rtl::OUString g_result;
  extern int           g_check;
# pragma weak  g_from_envDcp
# pragma weak  g_to_envDcp
# pragma weak  g_ref
# pragma weak  g_custom
# pragma weak  g_result
# pragma weak  g_check

#else
# define CPPU_TEST_MAPPING_TESTER_TESTS_EXPORT SAL_DLLPUBLIC_IMPORT

#endif

CPPU_TEST_MAPPING_TESTER_TESTS_EXPORT rtl::OUString g_from_envDcp;
CPPU_TEST_MAPPING_TESTER_TESTS_EXPORT rtl::OUString g_to_envDcp;
CPPU_TEST_MAPPING_TESTER_TESTS_EXPORT rtl::OUString g_ref;
CPPU_TEST_MAPPING_TESTER_TESTS_EXPORT rtl::OUString g_custom;
CPPU_TEST_MAPPING_TESTER_TESTS_EXPORT rtl::OUString g_result;
CPPU_TEST_MAPPING_TESTER_TESTS_EXPORT int           g_check;


#define D_CALLEE extern "C" void CALLEE(rtl_uString *  pMethod_name)


#endif
