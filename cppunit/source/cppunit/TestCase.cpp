#include <stdlib.h>
#include <cppunit/Portability.h>
#include <typeinfo>
#include <stdexcept>

#include "cppunit/TestCase.h"
#include "cppunit/Exception.h"
// #include "cppunit/TestResult.h"

#include <cppunit/autoregister/callbackfunc_fktptr.h>
#include <cppunit/tagvalues.hxx>
#include <cppunit/externcallbackfunc.hxx>

namespace CppUnit {

/// Create a default TestResult
//# CppUnit::TestResult*
//# TestCase::defaultResult()
//# {
//#   return new TestResult;
//# }


/// Run the test and catch any exceptions that are triggered by it
//# void
//# TestCase::run( TestResult *result )
//# {
//#   result->startTest(this);
//#
//#   try {
//#       setUp();
//#
//#       try {
//#         runTest();
//#       }
//#       catch ( Exception &e ) {
//#         Exception *copy = e.clone();
//#         result->addFailure( this, copy );
//#       }
//#       catch ( std::exception &e ) {
//#         result->addError( this, new Exception( e.what() ) );
//#       }
//#       catch (...) {
//#         Exception *e = new Exception( "caught unknown exception" );
//#         result->addError( this, e );
//#       }
//#
//#       try {
//#         tearDown();
//#       }
//#       catch (...) {
//#         result->addError( this, new Exception( "tearDown() failed" ) );
//#       }
//#   }
//#   catch (...) {
//#       result->addError( this, new Exception( "setUp() failed" ) );
//#   }
//#
//#   result->endTest( this );
//# }
//#

void
TestCase::run( hTestResult result )
{
    Test* pTest = this;

    // if (signalCheck(pTest->getName()) == HAS_SIGNAL)
    sal_Int32 nValue = (pCallbackFunc)(0 /* NULL */, TAG_TYPE, EXECUTION_CHECK, TAG_RESULT_PTR, result, TAG_NODENAME, pTest->getName().c_str(), TAG_DONE);
    if (nValue == DO_NOT_EXECUTE)
    {
        return;
    }

    // (pTestResult_StartTest)(result, pTest);
    (pCallbackFunc)(0 /* NULL */, TAG_TYPE, RESULT_START, TAG_RESULT_PTR, result, TAG_TEST_PTR, pTest, TAG_DONE);

    try
    {
        setUp();

        try
        {
            runTest();
            // (pTestResult_AddInfo)(result, pTest, "PASSED");
            (pCallbackFunc)(0 /* NULL */, TAG_TYPE, RESULT_ADD_INFO, TAG_RESULT_PTR, result, TAG_TEST_PTR, pTest, TAG_INFO, "PASSED", TAG_DONE);
        }
        catch ( StubException &)
        {
            (pCallbackFunc)(0 /* NULL */, TAG_TYPE, RESULT_ADD_INFO, TAG_RESULT_PTR, result, TAG_TEST_PTR, pTest, TAG_INFO, "STUB", TAG_DONE );
        }
        catch ( Exception &e )
        {
            Exception *copy = e.clone();
            // (pTestResult_AddFailure)( result, pTest, copy );
            (pCallbackFunc)(0 /* NULL */, TAG_TYPE, RESULT_ADD_FAILURE, TAG_RESULT_PTR, result, TAG_TEST_PTR, pTest, TAG_EXCEPTION, copy, TAG_DONE );
        }
        catch ( std::exception &e )
        {
            // (pTestResult_AddError)( result, pTest, new Exception( e.what() ) );
            (pCallbackFunc)( 0 /* NULL */, TAG_TYPE, RESULT_ADD_ERROR, TAG_RESULT_PTR, result, TAG_TEST_PTR, pTest, TAG_EXCEPTION, new Exception( e.what() ), TAG_DONE );
        }
        catch (...)
        {
            Exception *e = new Exception( "caught unknown exception" );
            // (pTestResult_AddError)( result, pTest, e );
            (pCallbackFunc)( 0 /* NULL */, TAG_TYPE, RESULT_ADD_ERROR, TAG_RESULT_PTR, result, TAG_TEST_PTR, pTest, TAG_EXCEPTION, e, TAG_DONE );
        }

        try
        {
            tearDown();
        }
        catch (...)
        {
            // (pTestResult_AddError)( result, pTest, new Exception( "tearDown() failed" ) );
            (pCallbackFunc)( 0 /* NULL */, TAG_TYPE, RESULT_ADD_ERROR, TAG_RESULT_PTR, result, TAG_TEST_PTR, pTest, TAG_EXCEPTION, new Exception( "tearDown() failed" ), TAG_DONE );
        }
    }
    catch (...)
    {
        // (pTestResult_AddError)( result, pTest, new Exception( "setUp() failed" ) );
        (pCallbackFunc)( 0 /* NULL */, TAG_TYPE, RESULT_ADD_ERROR, TAG_RESULT_PTR, result, TAG_TEST_PTR, pTest, TAG_EXCEPTION, new Exception( "setUp() failed" ), TAG_DONE );
    }

    // (pTestResult_EndTest)( result, pTest );
    (pCallbackFunc)( 0 /* NULL */, TAG_TYPE, RESULT_END, TAG_RESULT_PTR, result, TAG_TEST_PTR, pTest, TAG_DONE );
}

/// A default run method
//# TestResult *
//# TestCase::run()
//# {
//#   TestResult *result = defaultResult();
//#
//#   run (result);
//#   return result;
//# }


/// All the work for runTest is deferred to subclasses
void
TestCase::runTest()
{
}


/** Constructs a test case.
 *  \param name the name of the TestCase.
 **/
TestCase::TestCase( std::string const& name )
    : m_name(name)
{
}


/** Constructs a test case for a suite.
 *  This TestCase is intended for use by the TestCaller and should not
 *  be used by a test case for which run() is called.
 **/
TestCase::TestCase()
    : m_name( "" )
{
}


/// Destructs a test case
TestCase::~TestCase()
{
}


/// Returns a count of all the tests executed
int
TestCase::countTestCases() const
{
  return 1;
}


/// Returns the name of the test case
std::string
TestCase::getName() const
{
  return m_name;
}


/// Returns the name of the test case instance
std::string
TestCase::toString() const
{
  std::string className;

#if CPPUNIT_USE_TYPEINFO_NAME
  const std::type_info& thisClass = typeid( *this );
  className = thisClass.name();
#else
  className = "TestCase";
#endif

  return className + "." + getName();
}


} // namespace CppUnit

// The following sets variables for GNU EMACS
// Local Variables:
// tab-width:4
// End:
