
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

void SAL_CALL registerFunc(FktPtr _pFunc, const char* _sFuncName)
{
    // printf("Function register call for func(%s) successful.\n", _sFuncName);
    m_Functions.push_back(_pFunc);
}

// -----------------------------------------------------------------------------
AutomaticRegisterHelper::AutomaticRegisterHelper(rtl::OUString const& _sDLLName, GetOpt & _aOptions /*, JobList * _pJobList*/)
        :DynamicLibraryHelper(_sDLLName, _aOptions)
{
    // try to get the entry pointer
    FktRegAllPtr pFunc = (FktRegAllPtr) m_pModule->getSymbol( rtl::OUString::createFromAscii( "registerAllTestFunction" ) );

    if (pFunc)
    {
        // FktRegFuncPtr pRegisterFunc = &DynamicLibraryHelper::registerFunc;
        // pFunc(pRegisterFunc);
        // osl::Guard aGuard(m_Mutex);
        FktRegFuncPtr pRegisterFunc = &registerFunc;

        CallbackStructure aCallback;
        aCallback.aRegisterFunction = pRegisterFunc;

        aCallback.aCallbackDispatch  = &CallbackDispatch;

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
            fprintf(stderr, "error: Internal test failed. Value Magic2 != Magic\n");
            exit(-1);
        }
    }
    else
    {
        fprintf(stderr, "warning: Function 'registerAllTestFunction' not found.\n");
    }
}

void AutomaticRegisterHelper::CallAll(hTestResult _hResult)
{
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

