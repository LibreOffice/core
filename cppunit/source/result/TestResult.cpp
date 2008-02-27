#include <cppunit/TestFailure.h>
#include <cppunit/result/TestListener.h>
#include <cppunit/result/TestResult.h>
#include <algorithm>
#include "cppunit/result/outputter.hxx"
#include <cppunit/Test.h>
#include <cppunit/cmdlinebits.hxx>

namespace CppUnit {

/// Construct a TestResult
TestResult::TestResult( GetOpt &_pOptions, SynchronizationObject *syncObject )
    : SynchronizedObject( syncObject ),
      m_aOptionHelper(_pOptions),
      m_nExitValue(0)
{
  reset();
}


/// Destroys a test result
TestResult::~TestResult()
{
}


/** Resets the result for a new run.
 *
 * Clear the previous run result.
 */
void
TestResult::reset()
{
  ExclusiveZone zone( m_syncObject );
  m_stop = false;
}


/** Adds an error to the list of errors.
 *  The passed in exception
 *  caused the error
 */
void
TestResult::addError( Test *test,
                      Exception *e, ErrorType::num _eType )
{
  TestFailure aTestFailure( test, e, _eType );
  addFailure( aTestFailure );
}


/** Adds a failure to the list of failures. The passed in exception
 * caused the failure.
 */
void
TestResult::addFailure( Test *test, Exception *e )
{
  TestFailure aTestFailure( test, e, ErrorType::ET_FAILURE );
  addFailure( aTestFailure );
}


/** Called to add a failure to the list of failures.
 */
void
TestResult::addFailure( const TestFailure &failure )
{
  ExclusiveZone zone( m_syncObject );

  // LLA:
  // this set the global returnvalue, due to the fact, there occurs a failure, we have to return a non zero value
  // at the moment this seams to be a good place.
  setExitValue(1);

  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end();
        ++it )
  {
      TestListener *pListener = *it;
      pListener->addFailure( failure );
  }
}


/// Informs the result that a test will be started.
void
TestResult::startTest( Test *test )
{
  ExclusiveZone zone( m_syncObject );
  if (m_aOptionHelper.isVerbose())
  {
      std::string aStr;
      if (test)
      {
          aStr = getNodeName();
          aStr += ".";
          aStr += test->getName();
      }
      // fprintf(stderr, "Start test: %s\n", aStr.c_str());
      t_print( T_VERBOSE, "Start test: %s\n", aStr.c_str());
  }

  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end();
        ++it )
  {
      TestListener *pListener = *it;
      pListener->startTest( test );
  }
}


/// Informs the result that a test was completed.
void
TestResult::endTest( Test *test )
{
  ExclusiveZone zone( m_syncObject );
  for ( TestListeners::iterator it = m_listeners.begin();
        it != m_listeners.end();
        ++it )
  {
      TestListener *pListener = *it;
      pListener->endTest( test );
  }
}


/// Returns whether testing should be stopped
bool
TestResult::shouldStop() const
{
  ExclusiveZone zone( m_syncObject );
  return m_stop;
}


/// Stop testing
void
TestResult::stop()
{
  ExclusiveZone zone( m_syncObject );
  m_stop = true;
}


void
TestResult::addListener( TestListener *listener )
{
  ExclusiveZone zone( m_syncObject );
  m_listeners.push_back( listener );
}


void
TestResult::removeListener ( TestListener *listener )
{
  ExclusiveZone zone( m_syncObject );
#if defined(_MSC_VER) && (_MSC_VER >=1400)
  m_listeners.erase( remove( m_listeners.begin(),
#else
  m_listeners.erase( std::remove( m_listeners.begin(),
#endif
                                  m_listeners.end(),
                                  listener ),
                     m_listeners.end());
}

void
TestResult::addInfo(Test *test, const char* _aStr)
{
    ExclusiveZone zone( m_syncObject );
    for ( TestListeners::iterator it = m_listeners.begin();
          it != m_listeners.end();
          ++it )
    {
        TestListener *pListener = *it;
        pListener->addInfo( test, _aStr );
    }
}

// old: void
// old: TestResult::enterNode(const char* _aStr)
// old: {
// old:         ExclusiveZone zone( m_syncObject );
// old:         for ( TestListeners::iterator it = m_listeners.begin();
// old:                   it != m_listeners.end();
// old:                   ++it )
// old:         {
// old:                 TestListener *pListener = *it;
// old:                 pListener->enterNode( _aStr );
// old:         }
// old: }
// old:
// old: void
// old: TestResult::leaveNode(const char* _aStr)
// old: {
// old:         ExclusiveZone zone( m_syncObject );
// old:
// old:         for ( TestListeners::iterator it = m_listeners.begin();
// old:                   it != m_listeners.end();
// old:                   ++it )
// old:         {
// old:                 TestListener *pListener = *it;
// old:                 pListener->leaveNode( _aStr );
// old:         }
// old: }

void TestResult::enterNode(const char* _sNode)
{
    ExclusiveZone zone( m_syncObject );
    m_aCurrentNodeNames.push_back(std::string(_sNode));
}

void TestResult::leaveNode(const char* /*_sNode*/)
{
    ExclusiveZone zone( m_syncObject );
    std::string sBack = m_aCurrentNodeNames.back();
    m_aCurrentNodeNames.pop_back();

    // due to a -Wall warning, comment out.
    // if (sBack != std::string(_sNode))
    // {
    //     volatile int dummy = 0;
    //     // problem?!
    // }
}

std::string TestResult::getNodeName()
{
    std::string sName;
    for (std::vector<std::string>::const_iterator it = m_aCurrentNodeNames.begin();
         it != m_aCurrentNodeNames.end();
         ++it)
    {
        if (sName.size() != 0)
        {
            sName += ".";
        }
        sName += *it;
    }
    return sName;
}

// -----------------------------------------------------------------------------
bool TestResult::isAllowedToExecute(std::string const& _sName)
{
    return m_aOptionHelper.isAllowedToExecute(getNodeName(), _sName);
}
// -----------------------------------------------------------------------------
bool TestResult::isOptionWhereAmI()
{
    return m_aOptionHelper.isOptionWhereAmI();
}

// -----------------------------------------------------------------------------
void TestResult::print(Outputter&)
{
}

} // namespace CppUnit
