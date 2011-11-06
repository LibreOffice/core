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
