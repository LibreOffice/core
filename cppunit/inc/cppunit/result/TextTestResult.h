#ifndef CPPUNIT_TEXTTESTRESULT_H
#define CPPUNIT_TEXTTESTRESULT_H

#include <cppunit/result/TestResult.h>
#include <cppunit/result/TestResultCollector.h>
#include <ostream>

class GetOpt;
namespace CppUnit {

class SourceLine;
class Exception;
class Test;

/*! \brief Holds printable test result (DEPRECATED).
 * \ingroup TrackingTestExecution
 *
 * deprecated Use class TextTestProgressListener and TextOutputter instead.
 */
class CPPUNIT_API TextTestResult : public TestResult
/*                                     public TestResultCollector*/
{
    TestResultCollector m_aResulter;
public:
  TextTestResult(GetOpt& _aOptions);

  virtual void addFailure( const TestFailure &failure );
  virtual void startTest( Test *test );
  virtual void endTest( Test *test );

  virtual void print( std::ostream &stream );
protected:

  virtual void printFailures( std::ostream &stream );
  virtual void printHeader( std::ostream &stream );

  virtual void printFailure( TestFailure *failure,
                             int failureNumber,
                             std::ostream &stream );
  virtual void printFailureListMark( int failureNumber,
                                     std::ostream &stream );
  virtual void printFailureTestName( TestFailure *failure,
                                     std::ostream &stream );
  virtual void printFailureType( TestFailure *failure,
                                 std::ostream &stream );
  virtual void printFailureLocation( SourceLine sourceLine,
                                     std::ostream &stream );
  virtual void printFailureDetail( Exception *thrownException,
                                   std::ostream &stream );
  virtual void printFailureWarning( std::ostream &stream );
  virtual void printStatistics( std::ostream &stream );
};

/** insertion operator for easy output */
std::ostream &operator <<( std::ostream &stream,
                           TextTestResult &result );

} // namespace CppUnit

#endif // CPPUNIT_TEXTTESTRESULT_H


