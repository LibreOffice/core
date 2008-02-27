/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: callbackstructure.h,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: lla $ $Date: 2008-02-27 16:18:40 $
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

#ifndef _callbackstructure_h
#define _callbackstructure_h

#include <sal/types.h>
#include <cppunit/autoregister/callbackfunc_fktptr.h>
#include <cppunit/autoregister/registerfunc.h>
#include <cppunit/autoregister/registertestfunction.h>
#include <cppunit/cmdlinebits.hxx>

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
