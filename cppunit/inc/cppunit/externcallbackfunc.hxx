/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: externcallbackfunc.hxx,v $
 * $Revision: 1.2 $
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

#ifndef _externcallbackfunc_hxx
#define _externcallbackfunc_hxx

#include <cppunit/autoregister/callbackfunc_fktptr.h>

// these pointers have they origin in 'tresregister.cxx'
extern FktPtr_CallbackDispatch pCallbackFunc;

//# extern FktPtr_TestResult_startTest  pTestResult_StartTest;
//# extern FktPtr_TestResult_addFailure pTestResult_AddFailure;
//# extern FktPtr_TestResult_addError   pTestResult_AddError;
//# extern FktPtr_TestResult_endTest    pTestResult_EndTest;
//# extern FktPtr_TestResult_addInfo    pTestResult_AddInfo;

// these pointers have they origin in 'tresregister.cxx'
// extern FktPtr_TestResult_Starter  pTestResult_Starter;

//# extern FktPtr_TestResult_startTest  pTestResult_StartTest;
//# extern FktPtr_TestResult_addFailure pTestResult_AddFailure;
//# extern FktPtr_TestResult_addError   pTestResult_AddError;
//# extern FktPtr_TestResult_endTest    pTestResult_EndTest;
//# extern FktPtr_TestResult_addInfo    pTestResult_AddInfo;
//#
//# extern FktPtr_TestResult_enterNode  pTestResult_EnterNode;
//# extern FktPtr_TestResult_leaveNode  pTestResult_LeaveNode;
//#

#endif
