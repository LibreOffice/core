#include <stdlib.h>
#include "cppunit/TestSuite.h"
// #include "cppunit/TestResult.h"
#include <cppunit/tagvalues.hxx>
#include <cppunit/autoregister/callbackfunc_fktptr.h>
#include <cppunit/externcallbackfunc.hxx>

namespace CppUnit {

/// Default constructor
TestSuite::TestSuite( std::string const& name )
        : m_name( name )
{
}


/// Destructor
TestSuite::~TestSuite()
{
    deleteContents();
}


/// Deletes all tests in the suite.
void
TestSuite::deleteContents()
{
    for ( std::vector<Test *>::iterator it = m_tests.begin();
          it != m_tests.end();
          ++it)
        delete *it;
    m_tests.clear();
}


/// Runs the tests and collects their result in a TestResult.

void
TestSuite::run( hTestResult result )
{
    // (pTestResult_EnterNode)(result, getName().c_str());
    (pCallbackFunc)(0 /* NULL */, TAG_TYPE, RESULT_ENTER_NODE, TAG_RESULT_PTR, result, TAG_NODENAME, getName().c_str(), TAG_DONE);
    for ( std::vector<Test *>::iterator it = m_tests.begin();
          it != m_tests.end();
          ++it )
    {
        //# if ( result->shouldStop() )
        //#     break;
        // if ( pTestResult_ShouldStop(result) )
        if ( (pCallbackFunc)(0 /* NULL */, TAG_TYPE, RESULT_SHOULD_STOP, TAG_RESULT_PTR, result, TAG_DONE) )
        {
            break;
        }
        Test *test = *it;
        test->run( result );
    }
    // (pTestResult_LeaveNode)(result, getName().c_str());
    (pCallbackFunc)(0 /* NULL */, TAG_TYPE, RESULT_LEAVE_NODE, TAG_RESULT_PTR, result, TAG_NODENAME, getName().c_str(), TAG_DONE);
}


/// Counts the number of test cases that will be run by this test.
int
TestSuite::countTestCases() const
{
    int count = 0;

    for ( std::vector<Test *>::const_iterator it = m_tests.begin();
          it != m_tests.end();
          ++it )
        count += (*it)->countTestCases();

    return count;
}


/// Adds a test to the suite.
void
TestSuite::addTest( Test *test )
{
    m_tests.push_back( test );
}


/// Returns a string representation of the test suite.
std::string
TestSuite::toString() const
{
    return "suite " + getName();
}


/// Returns the name of the test suite.
std::string
TestSuite::getName() const
{
    return m_name;
}


const std::vector<Test *> &
TestSuite::getTests() const
{
    return m_tests;
}


} // namespace CppUnit

// The following sets variables for GNU EMACS
// Local Variables:
// tab-width:4
// End:
