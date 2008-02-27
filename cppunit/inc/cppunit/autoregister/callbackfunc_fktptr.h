/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: callbackfunc_fktptr.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:18:28 $
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

#ifndef _callbackfunc_fktptr_h
#define _callbackfunc_fktptr_h

#include <sal/types.h>

#ifdef __cplusplus
extern "C" {
#endif

// these typedefs are used for the TestCase, to call the TestResult functions.
// The TestResult is only in the testshl tool, not in the test libraries.

typedef void* hTest;
typedef void* hException;

typedef sal_Int32 (* FktPtr_CallbackDispatch)(int x, ...);

//# typedef void (* FktPtr_TestResult_startTest)(hTestResult, hTest );
//# typedef void (* FktPtr_TestResult_addFailure)( hTestResult, hTest , hException pException );
//# typedef void (* FktPtr_TestResult_addError)( hTestResult, hTest , hException pException );
//# typedef void (* FktPtr_TestResult_endTest)( hTestResult, hTest );
//# typedef bool (* FktPtr_TestResult_shouldStop)(hTestResult);
//#
//# // Additional functions
//# typedef void (* FktPtr_TestResult_addInfo)( hTestResult, hTest, const char* );
//# typedef void (* FktPtr_TestResult_enterNode)( hTestResult, const char* );
//# typedef void (* FktPtr_TestResult_leaveNode)( hTestResult, const char* );

#ifdef __cplusplus
}
#endif

#endif
