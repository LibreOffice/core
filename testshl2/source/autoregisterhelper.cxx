/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: autoregisterhelper.cxx,v $
 * $Revision: 1.12 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_testshl2.hxx"


#include <stdio.h>
#include <stdlib.h>
#include "testshl/autoregisterhelper.hxx"

#include <rtl/ustring.hxx>

#include <testshl/autoregister/registerfunc.h>
#include <testshl/autoregister/callbackstructure.h>
#include <testshl/result/callbackfunc.h>
#include <testshl/autoregister/testfunc.h>
#include "testshl/filehelper.hxx"

FunctionList m_Functions;
// osl::Mutex   m_Mutex;

extern "C" void SAL_CALL registerFunc(FktPtr _pFunc, const char*)
{
    m_Functions.push_back(_pFunc);
}

// -----------------------------------------------------------------------------
AutomaticRegisterHelper::AutomaticRegisterHelper(rtl::OUString const& _sDLLName, GetOpt & _aOptions /*, JobList * _pJobList*/)
        :DynamicLibraryHelper(_sDLLName, _aOptions),
         m_bLoadLibraryOK(false)
{
    // try to get the entry pointer
    FktRegAllPtr pFunc = (FktRegAllPtr) m_pModule->getFunctionSymbol(
        rtl::OUString::createFromAscii( "registerAllTestFunction" ) );

    if (pFunc)
    {
        m_bLoadLibraryOK = true;
        // FktRegFuncPtr pRegisterFunc = &DynamicLibraryHelper::registerFunc;
        // pFunc(pRegisterFunc);
        // osl::Guard aGuard(m_Mutex);
        FktRegFuncPtr pRegisterFunc = &registerFunc;

        CallbackStructure aCallback;
        aCallback.aRegisterFunction = pRegisterFunc;

        aCallback.aCallbackDispatch  = &CallbackDispatch;

        // special parameter for API testing
        if (_aOptions.hasOpt("-forward"))
        {
            aCallback.psForward = _aOptions.getOpt("-forward").getStr();
        }

        // aCallback.pJobList = _pJobList;

        //# aCallback.aStartTest  = &TestResult_startTest;
        //# aCallback.aAddFailure = &TestResult_addFailure;
        //# aCallback.aAddError   = &TestResult_addError;
        //# aCallback.aEndTest    = &TestResult_endTest;
        //# aCallback.aShouldStop = &TestResult_shouldStop;
        //# aCallback.aAddInfo    = &TestResult_addInfo;
        //# aCallback.aEnterNode  = &TestResult_enterNode;
        //# aCallback.aLeaveNode  = &TestResult_leaveNode;

        aCallback.nBits       = FileHelper::createFlags(_aOptions);

        pFunc(&aCallback);

        if (aCallback.nMagic == aCallback.nMagic2)
        {
            // ok internal simple test done.
            m_aFunctionList = m_Functions;
        }
        else
        {
            // ERROR, the function seams not to be what we thing it's to be.
            fprintf(stderr, "error: Internal check failed. Structure inconsistent, Value Magic2 != Magic.\nPlease recompile your test libraries.");
            exit(-1);
        }
    }
    else
    {
        fprintf(stderr, "warning: Function 'registerAllTestFunction' not found.\n");
        fprintf(stderr, "If you think, you are right, build testshl2 completly new.\n");
    }
}

void AutomaticRegisterHelper::CallAll(hTestResult _hResult) const
{
    // can't load the module, break the tests.
    if (m_bLoadLibraryOK == false)
    {
        return;
    }

    for (FunctionList::const_iterator it = m_aFunctionList.begin();
         it != m_aFunctionList.end();
         ++it)
    {
        FktPtr pFunc = *it;
        if (pFunc)
        {
            (pFunc)(_hResult);
        }
    }
}

AutomaticRegisterHelper::~AutomaticRegisterHelper()
{
    if (getOptions().hasOpt("-verbose"))
    {
        fprintf(stderr, "Dtor AutomaticRegisterHelper.\n");
    }
}
