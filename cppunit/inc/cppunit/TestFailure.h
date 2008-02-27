#ifndef CPPUNIT_TESTFAILURE_H    // -*- C++ -*-
#define CPPUNIT_TESTFAILURE_H

#include <cppunit/Portability.h>
#include <string>

namespace ErrorType
{
    enum num
    {
        ET_FAILURE = 1,
        ET_ERROR = 2,
        ET_SIGNAL = 4
    };
}

namespace CppUnit {

class Exception;
class SourceLine;
class Test;


/*! \brief Record of a failed Test execution.
 * \ingroup BrowsingCollectedTestResult
 *
 * A TestFailure collects a failed test together with
 * the caught exception.
 *
 * TestFailure assumes lifetime control for any exception
 * passed to it.
 */

class CPPUNIT_API TestFailure
{
public:
  TestFailure( Test *failedTest,
               Exception *thrownException,
               ErrorType::num eError );

  virtual ~TestFailure ();

  virtual Test *failedTest() const;

  virtual Exception *thrownException() const;

  virtual SourceLine sourceLine() const;

  virtual bool isError() const;
  virtual ErrorType::num getErrorType() const;

  virtual std::string failedTestName() const;

  virtual std::string toString() const;

  virtual TestFailure *clone() const;

protected:
  Test *m_failedTest;
  Exception *m_thrownException;
  ErrorType::num m_eError;

private:
  TestFailure( const TestFailure &other );
  TestFailure &operator =( const TestFailure& other );
};


} // namespace CppUnit

#endif // CPPUNIT_TESTFAILURE_H
