/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: autoregisterhelper.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:27:37 $
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


#include <stdio.h>
#include <stdlib.h>
#include "autoregisterhelper.hxx"

#include <rtl/ustring.hxx>

#include <cppunit/autoregister/registerfunc.h>
#include <cppunit/autoregister/callbackstructure.h>
#include <cppunit/result/callbackfunc.h>
#include <cppunit/autoregister/testfunc.h>
#include "filehelper.hxx"

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

