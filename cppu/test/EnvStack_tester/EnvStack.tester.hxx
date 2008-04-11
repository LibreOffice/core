/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: EnvStack.tester.hxx,v $
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
