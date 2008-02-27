#include <cppunit/TestFailure.h>
#include <cppunit/result/TestResultCollector.h>


namespace CppUnit
{


TestResultCollector::TestResultCollector( TestResult* _pResult, SynchronizationObject *syncObject )
        : TestSucessListener( syncObject ),
          m_pResult(_pResult),
          m_testErrors(0)
{
  reset();
}


TestResultCollector::~TestResultCollector()
{
    TestFailures::iterator itFailure = m_failures.begin();
    while ( itFailure != m_failures.end() )
    {
        TestFailureEnvelope *pEnvelope = *itFailure++;
        TestFailure *pFailure = pEnvelope->getTestFailure();
        delete pFailure;
    }
}


void
TestResultCollector::reset()
{
  TestSucessListener::reset();

  ExclusiveZone zone( m_syncObject );
  m_testErrors = 0;
  m_tests.clear();
  m_failures.clear();
}


void
TestResultCollector::startTest( Test *test )
{
  ExclusiveZone zone (m_syncObject);

  m_tests.push_back( new TestEnvelope(test, m_pResult->getNodeName()) );
}

void
TestResultCollector::endTest( Test * )
{
    // ExclusiveZone zone (m_syncObject);
    // Nothing!
}


void
TestResultCollector::addFailure( const TestFailure &failure )
{
  TestSucessListener::addFailure( failure );

  ExclusiveZone zone( m_syncObject );
  if ( failure.isError() )
    ++m_testErrors;
  m_failures.push_back( new TestFailureEnvelope(failure.clone(), m_pResult->getNodeName()) );
}


/// Gets the number of run tests.
int
TestResultCollector::runTests() const
{
  ExclusiveZone zone( m_syncObject );
  return m_tests.size();
}


/// Gets the number of detected errors (uncaught exception).
int
TestResultCollector::testErrors() const
{
  ExclusiveZone zone( m_syncObject );
  return m_testErrors;
}


/// Gets the number of detected failures (failed assertion).
int
TestResultCollector::testFailures() const
{
  ExclusiveZone zone( m_syncObject );
  return m_failures.size() - m_testErrors;
}


/// Gets the total number of detected failures.
int
TestResultCollector::testFailuresTotal() const
{
  ExclusiveZone zone( m_syncObject );
  return m_failures.size();
}


/// Returns a the list failures (random access collection).
const TestResultCollector::TestFailures &
TestResultCollector::failures() const
{
  ExclusiveZone zone( m_syncObject );
  return m_failures;
}


const TestResultCollector::Tests &
TestResultCollector::tests() const
{
  ExclusiveZone zone( m_syncObject );
  return m_tests;
}

void TestResultCollector::addInfo(Test *_pTest, const char* _sInfo)
{
    ExclusiveZone zone( m_syncObject );
    m_aInfos.push_back(new TestInfo(_pTest, _sInfo));
}

std::string TestResultCollector::getInfo(Test *_pTest)
{
    for (TestInfos::const_iterator it = m_aInfos.begin();
         it != m_aInfos.end();
         ++it)
    {
        TestInfo *pInfo   = *it;
        Test *pTest       = pInfo->getTest();
        if (pTest == _pTest)
        {
            std::string sInfo = pInfo->getString();
            return sInfo;
        }
    }
    return std::string();
}


} //  namespace CppUnit

