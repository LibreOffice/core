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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
