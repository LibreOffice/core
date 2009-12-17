#ifndef CPPUNIT_TESTLISTENER_H    // -*- C++ -*-
#define CPPUNIT_TESTLISTENER_H

#include <cppunit/Portability.h>


namespace CppUnit {

class Exception;
class Test;
class TestFailure;


/*! \brief Listener for test progress and result.
 * \ingroup TrackingTestExecution
 *
 * Implementing the Observer pattern a TestListener may be registered
 * to a TestResult to obtain information on the testing progress. Use
 * specialized sub classes of TestListener for text output
 * (TextTestProgressListener). Do not use the Listener for the test
 * result output, use a subclass of Outputter instead.
 *
 * The test framework distinguishes between failures and errors.
 * A failure is anticipated and checked for with assertions. Errors are
 * unanticipated problems signified by exceptions that are not generated
 * by the framework.
 *
 * \see TestResult
 */
class CPPUNIT_API TestListener
{
public:
  virtual ~TestListener() {}

  /// Called when just before a TestCase is run.
    virtual void startTest( Test *test ) =0 ;

  /*! Called when a failure occurs while running a test.
   * \see TestFailure.
   * \warning \a failure is a temporary object that is destroyed after the
   *          method call. Use TestFailure::clone() to create a duplicate.
   */
    virtual void addFailure( const TestFailure &failure ) =0;

  /// Called just after a TestCase was run (even if a failure occured).
    virtual void endTest( Test *test ) =0;

    // additional info
    virtual void addInfo(Test *test, const char*) =0;

    // info in which node we are
    // helper functions to create tree structures
    // old: virtual void enterNode( const char* ) =0;
    // old: virtual void leaveNode( const char* ) =0;
};


} // namespace CppUnit

#endif // CPPUNIT_TESTLISTENER_H


