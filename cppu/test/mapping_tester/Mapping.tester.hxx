/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Mapping.tester.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:48:55 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
