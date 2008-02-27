#include "cppunit/Exception.h"
#include "cppunit/Test.h"
#include "cppunit/TestFailure.h"

namespace CppUnit {

/// Constructs a TestFailure with the given test and exception.
TestFailure::TestFailure( Test *failed,
                          Exception *_thrownException,
                          ErrorType::num eError ) :
    m_failedTest( failed ),
    m_thrownException( _thrownException ),
    m_eError( eError )
{
}

/// Deletes the owned exception.
TestFailure::~TestFailure()
{
  delete m_thrownException;
}

/// Gets the failed test.
Test *
TestFailure::failedTest() const
{
  return m_failedTest;
}


/// Gets the thrown exception. Never \c NULL.
Exception *
TestFailure::thrownException() const
{
  return m_thrownException;
}


/// Gets the failure location.
SourceLine
TestFailure::sourceLine() const
{
  return m_thrownException->sourceLine();
}


/// Indicates if the failure is a failed assertion or an error.
bool
TestFailure::isError() const
{
    if (m_eError == ErrorType::ET_ERROR ||
        m_eError == ErrorType::ET_SIGNAL)
        return true;
    return false;
}

ErrorType::num
TestFailure::getErrorType() const
{
  return m_eError;
}


/// Gets the name of the failed test.
std::string
TestFailure::failedTestName() const
{
  return m_failedTest->getName();
}


/// Returns a short description of the failure.
std::string
TestFailure::toString() const
{
  return m_failedTest->toString() + ": " + m_thrownException->what();
}


TestFailure *
TestFailure::clone() const
{
  return new TestFailure( m_failedTest, m_thrownException->clone(), m_eError );
}

} // namespace CppUnit
