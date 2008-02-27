#include <cppunit/Exception.h>
#include <cppunit/NotEqualException.h>
#include <cppunit/Test.h>
#include <cppunit/TestFailure.h>
#include <cppunit/result/TextTestResult.h>
// #include <TextTestResult.h>
//!io #include <iostream>

namespace CppUnit {


TextTestResult::TextTestResult(GetOpt& _aOptions)
        :TestResult(_aOptions),
         m_aResulter(this)
{
    addListener( &m_aResulter );
}


void
TextTestResult::addFailure( const TestFailure &failure )
{
    TestResult::addFailure( failure );
    // std::cerr << ( failure.isError() ? "E" : "F" );
    if (failure.isError())
        fprintf(stderr, "E");
    else
        fprintf(stderr, "F");
}


void
TextTestResult::startTest( Test *test )
{
    TestResult::startTest (test);
    // std::cerr << ".";
    fprintf(stderr, ".");
}

void TextTestResult::endTest( Test *test )
{
    TestResult::endTest (test);
}


void
TextTestResult::printFailures( std::ostream &stream )
{
    TestResultCollector::TestFailures::const_iterator itFailure = m_aResulter.failures().begin();
    int failureNumber = 1;
    while ( itFailure != m_aResulter.failures().end() )
    {
        stream  <<  std::endl;
        TestFailure *pFailure= (*itFailure++)->getTestFailure();
        printFailure( pFailure, failureNumber++, stream );
  }
}


void
TextTestResult::printFailure( TestFailure *failure,
                              int failureNumber,
                              std::ostream &stream )
{
  printFailureListMark( failureNumber, stream );
  stream << ' ';
  printFailureTestName( failure, stream );
  stream << ' ';
  printFailureType( failure, stream );
  stream << ' ';
  printFailureLocation( failure->sourceLine(), stream );
  stream << std::endl;
  printFailureDetail( failure->thrownException(), stream );
  stream << std::endl;
}


void
TextTestResult::printFailureListMark( int failureNumber,
                                      std::ostream &stream )
{
  stream << failureNumber << ")";
}


void
TextTestResult::printFailureTestName( TestFailure *failure,
                                      std::ostream &stream )
{
    Test* pTest = failure->failedTest();
    stream << "test: " << pTest->getName();
}


void
TextTestResult::printFailureType( TestFailure *failure,
                                  std::ostream &stream )
{
  stream << "("
         << (failure->isError() ? "E" : "F")
         << ")";
}


void
TextTestResult::printFailureLocation( SourceLine sourceLine,
                                      std::ostream &stream )
{
  if ( !sourceLine.isValid() )
    return;

  stream << "line: " << sourceLine.lineNumber()
         << ' ' << sourceLine.fileName();
}


void
TextTestResult::printFailureDetail( Exception *thrownException,
                                    std::ostream &stream )
{
  if ( thrownException->isInstanceOf( NotEqualException::type() ) )
  {
    NotEqualException *e = (NotEqualException*)thrownException;
    stream << "expected: " << e->expectedValue() << std::endl
           << "but was:  " << e->actualValue();
    if ( !e->additionalMessage().empty() )
    {
      stream  << std::endl;
      stream  <<  "additional message:"  <<  std::endl
              <<  e->additionalMessage();
    }
  }
  else
  {
    stream << " \"" << thrownException->what() << "\"";
  }
}


void
TextTestResult::print( std::ostream& stream )
{
  printHeader( stream );
  stream << std::endl;
  printFailures( stream );
}


void
TextTestResult::printHeader( std::ostream &stream )
{
    if (m_aResulter.wasSuccessful ())
        stream << std::endl << "OK (" << m_aResulter.runTests () << " tests)"
               << std::endl;
    else
    {
        stream << std::endl;
        printFailureWarning( stream );
        printStatistics( stream );
    }
}


void
TextTestResult::printFailureWarning( std::ostream &stream )
{
  stream  << "!!!FAILURES!!!" << std::endl;
}


void
TextTestResult::printStatistics( std::ostream &stream )
{
  stream  << "Test Results:" << std::endl;

  stream  <<  "Run:  "  <<  m_aResulter.runTests()
          <<  "   Failures: "  <<  m_aResulter.testFailures()
          <<  "   Errors: "  <<  m_aResulter.testErrors()
          <<  std::endl;
}


std::ostream &
operator <<( std::ostream &stream,
             TextTestResult &result )
{
  result.print (stream); return stream;
}


} // namespace CppUnit
