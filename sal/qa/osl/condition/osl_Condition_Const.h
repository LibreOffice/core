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

#ifndef _OSL_CONDITION_CONST_H_
#define _OSL_CONDITION_CONST_H_

#include <sal/types.h>
#include <rtl/ustring.hxx>

#ifndef _OSL_THREAD_HXX_
#include <osl/thread.hxx>
#endif
#include <osl/mutex.hxx>
#include <osl/pipe.hxx>

#ifndef _OSL_SEMAPHOR_HXX_
#include <osl/semaphor.hxx>
#endif

#ifndef _OSL_CONDITION_HXX_
#include <osl/conditn.hxx>
#endif
#include <osl/time.h>

#ifdef UNX
#include <unistd.h>
#endif

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/plugin/TestPlugIn.h>

#define OSLTEST_DECLARE_USTRING( str_name, str_value ) \
    ::rtl::OUString a##str_name = rtl::OUString::createFromAscii( str_value )

//------------------------------------------------------------------------
// condition names
//------------------------------------------------------------------------
OSLTEST_DECLARE_USTRING( TestCon,  "testcondition" );

const char pTestString[17] = "Sun Microsystems";


#endif /* _OSL_CONDITION_CONST_H_ */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
