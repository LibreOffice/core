/*************************************************************************
*
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
*
* Copyright 2000, 2011 Oracle and/or its affiliates.
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

#ifndef INCLUDED_SAL_PRECPPUNIT_HXX
#define INCLUDED_SAL_PRECPPUNIT_HXX

#include "sal/config.h"
#include "sal/types.h"

// Overwrite the way CppUnit test plug-ins export the test function, adapting it
// to OOo's symbol visibility framework:
#define CPPUNIT_PLUGIN_EXPORT extern "C" SAL_DLLPUBLIC_EXPORT

//std::auto_ptr is deprecated in c++0x, but we're stuck with them in cppunit
//So push and pop -Wdeprecated-declarations for gcc
#define SAL_CPPUNIT_TEST_SUITE(X) \
SAL_WNODEPRECATED_DECLARATIONS_PUSH \
CPPUNIT_TEST_SUITE(X)

#define SAL_CPPUNIT_TEST_SUITE_END() \
CPPUNIT_TEST_SUITE_END() \
; \
SAL_WNODEPRECATED_DECLARATIONS_POP \
typedef int SalCppUnitDummyTypedefForSemiColonEnding__ \

#endif
