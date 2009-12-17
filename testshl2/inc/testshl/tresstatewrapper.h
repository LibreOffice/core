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

#ifndef TESTSTATEWRAPPER_H
#define TESTSTATEWRAPPER_H

#ifdef _RTL_TRES_H_
#error "This is old code, please remove the include of rtl/tres.h"
#endif

#include <sal/types.h>
#include "testshl/autoregister/registertestfunction.h"

#ifdef __cplusplus
extern "C" {
#endif

// typedef void* hTestResult;
typedef hTestResult rtlTestResult;

// -----------------------------------------------------------------------------
void c_rtl_tres_state_start(hTestResult _aResult, const sal_Char* _pName);
void c_rtl_tres_state_end(hTestResult _aResult, const sal_Char* _pName);

// -----------------------------------------------------------------------------
// the old test state function, which here convert all values for using with the
// new testshl tool

sal_Bool SAL_CALL c_rtl_tres_state(
    hTestResult pResult,
    sal_Bool bSuccess,
    char const * pTestMsg,
    char const * pFuncName);

#ifdef __cplusplus
}
#endif

#endif
