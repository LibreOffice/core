/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile$
 * $Revision$
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
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

#ifndef _registerfunc_h
#define _registerfunc_h

#include <sal/types.h>
#include <testshl/autoregister/htestresult.h>
#include <cppunit/callbackfunc_fktptr.h>
#include <testshl/autoregister/testfunc.h>

struct CallbackStructure;

// this function is used to register one function into a Container in the testshl tool.
extern "C" void SAL_CALL registerFunction(FktPtr aFunc, const char* aFuncName);

// this function is the register function for auto registering

extern "C" void SAL_CALL registerAllTestFunction(CallbackStructure* );
typedef void (* FktRegAllPtr)(CallbackStructure*);


#endif
