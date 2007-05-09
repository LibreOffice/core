/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: EnvStack.tester.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:42:13 $
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

#ifndef INCLUDED_cppu_test_EnvStack_tester_hxx
#define INCLUDED_cppu_test_EnvStack_tester_hxx

#include "uno/environment.hxx"


#ifdef CPPU_test_EnvStack_tester_IMPL
# define CPPU_test_EnvStack_tester_EXPORT SAL_DLLPUBLIC_EXPORT

#elif defined(CPPU_test_EnvStack_Test_LIB)
# define CPPU_test_EnvStack_tester_EXPORT extern

#elif defined(__GNUC__) && defined(HAVE_GCC_VISIBILITY_FEATURE)
# define CPPU_test_EnvStack_tester_EXPORT extern __attribute__ ((weak))

#elif defined(__SUNPRO_CC)
# define CPPU_test_EnvStack_tester_EXPORT extern
  extern rtl::OUString g_commentStack;
  extern com::sun::star::uno::Environment g_env;

# pragma weak  g_commentStack
# pragma weak  g_env

#else
# define CPPU_test_EnvStack_tester_EXPORT SAL_DLLPUBLIC_IMPORT

#endif

CPPU_test_EnvStack_tester_EXPORT rtl::OUString    g_commentStack;
CPPU_test_EnvStack_tester_EXPORT com::sun::star::uno::Environment g_env;



#endif
