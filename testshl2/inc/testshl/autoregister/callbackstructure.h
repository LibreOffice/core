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

#ifndef _callbackstructure_h
#define _callbackstructure_h

#include <sal/types.h>
#include <cppunit/callbackfunc_fktptr.h>
#include <testshl/autoregister/registerfunc.h>
#include <testshl/autoregister/registertestfunction.h>
#include <testshl/cmdlinebits.hxx>

// this structure will filled by the testshl tool and hand to the testdll

struct CallbackStructure
{
    // a version number, if we want to change/expand this struct
    sal_Int32 nVersion;
    sal_Int64 nMagic;

    FktRegFuncPtr                aRegisterFunction;

    FktPtr_CallbackDispatch      aCallbackDispatch;

    //# FktPtr_TestResult_startTest  aStartTest;
    //# FktPtr_TestResult_addFailure aAddFailure;
    //# FktPtr_TestResult_addError   aAddError;
    //# FktPtr_TestResult_endTest    aEndTest;
    //# FktPtr_TestResult_shouldStop aShouldStop;
    //#
    //# FktPtr_TestResult_addInfo    aAddInfo;
    //# FktPtr_TestResult_enterNode  aEnterNode;
    //# FktPtr_TestResult_leaveNode  aLeaveNode;

    const char* psForward;
    CmdLineBits nBits;

    // void* pJobList;

    sal_Int64 nMagic2;

    CallbackStructure()
            :nVersion(1),
             nMagic(SAL_CONST_INT64(0x0123456789abcdef)),
             aRegisterFunction(NULL),
             aCallbackDispatch(NULL),
             /* aStartTest(NULL),
             aAddFailure(NULL),
             aAddError(NULL),
             aEndTest(NULL),
             aShouldStop(NULL),
             aAddInfo(NULL),
             aEnterNode(NULL),
             aLeaveNode(NULL), */
             psForward(NULL),
             nBits(0),
             /* pJobList(NULL), */
             nMagic2(0)
        {}
};

#endif
