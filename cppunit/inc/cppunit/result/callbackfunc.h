#ifndef _callbackfunc_h
#define _callbackfunc_h

#ifdef __cplusplus
extern "C" {
#endif

    // this is the entry point from the DLL back to the executable.
    sal_Int32 CallbackDispatch(int x, ...);

//# void TestResult_startTest(hTestResult _pResult, hTest _pTest);
//# void TestResult_addFailure( hTestResult _pResult, hTest _pTest, hException _pException );
//# void TestResult_addError( hTestResult _pResult, hTest _pTest, hException _pException );
//# void TestResult_endTest( hTestResult _pResult, hTest _pTest );
//# bool TestResult_shouldStop(hTestResult _pResult);
//# void TestResult_addInfo( hTestResult _pResult, hTest _pTest, const char* _sInfo );
//#
//# void TestResult_enterNode( hTestResult _pResult, const char* _sInfo );
//# void TestResult_leaveNode( hTestResult _pResult, const char* _sInfo );

#ifdef __cplusplus
}
#endif

#endif
