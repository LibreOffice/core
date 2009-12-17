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

#ifndef _registertestfunction_h
#define _registertestfunction_h

#include <sal/types.h>
#include <testshl/autoregister/testfunc.h>

#ifdef __cplusplus
extern "C" {
#endif

// function ptr, to register a function
typedef void (* FktRegFuncPtr)(FktPtr aFunc, const char* aFuncName);

// this function is used in the testdll to register the given function
// into the testshl
extern "C" void SAL_CALL RegisterTestFunctions(FktRegFuncPtr aFunc);

#ifdef __cplusplus
}
#endif

#endif


