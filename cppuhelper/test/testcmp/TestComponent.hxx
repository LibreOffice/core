/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestComponent.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:27:47 $
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

#ifndef INCLUDED_cppuhelper_test_testcmp_TestComponent_hxx
#define INCLUDED_cppuhelper_test_testcmp_TestComponent_hxx

#include <sal/types.h>
#include <rtl/ustring.hxx>

#ifdef CPPUHELPER_TEST_COMPONENT_IMPL
# define CPPUHELPER_TEST_COMPONENT_EXPORT SAL_DLLPUBLIC_EXPORT

#elif defined(CPPUHELPER_TEST_COMPONENT_LIB)
# define CPPUHELPER_TEST_COMPONENT_EXPORT extern

#elif defined(__GNUC__) && defined(HAVE_GCC_VISIBILITY_FEATURE)
# define CPPUHELPER_TEST_COMPONENT_EXPORT extern __attribute__ ((weak))

#elif defined(__SUNPRO_CC)
# define CPPUHELPER_TEST_COMPONENT_EXPORT extern
  extern rtl::OUString g_envDcp;
# pragma weak  g_envDcp

#else
# define CPPUHELPER_TEST_COMPONENT_EXPORT SAL_DLLPUBLIC_IMPORT

#endif

CPPUHELPER_TEST_COMPONENT_EXPORT rtl::OUString g_envDcp;


#endif
