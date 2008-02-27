#ifndef CPPUNIT_TESTRESULT_H
#define CPPUNIT_TESTRESULT_H

#include <cppunit/Portability.h>

#if CPPUNIT_NEED_DLL_DECL
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4251 )  // X needs to have dll-interface to be used by clients of class Z
#endif
#endif

#include <cppunit/result/SynchronizedObject.h>
#include <vector>
#include <string>
#include <deque>
//!io #include <iostream>

#include "cppunit/result/optionhelper.hxx"
#include "cppunit/TestFailure.h"

class Outputter;
namespace CppUnit {

class Exception;
class Test;
// class TestFailure;
class TestListener;

#if CPPUNIT_NEED_DLL_DECL
  template class CPPUNIT_API std::deque<TestListener *>;
#endif

/*! \brief Manages TestListener.
 * \ingroup TrackingTestExecution
 *
 * A single instance of this class is used when running the test. It is usually
 * created by the test runner (TestRunner).
 *
 * This class shouldn't have to be inherited from. Use a TestListener
 * or one of its subclasses to be informed of the ongoing tests.
 * Use a Outputter to receive a test summary once it has finished
 *
 * TestResult supplies a template method 'setSynchronizationObject()'
 * so that subclasses can provide mutual exclusion in the face of multiple
 * threads.  This can be useful when tests execute in one thread and
 * they fill a subclass of TestResult which effects change in another
 * thread.  To have mutual exclusion, override setSynchronizationObject()
 * and make sure that you create an instance of ExclusiveZone at the
 * beginning of each method.
 *
 * \see Test, TestListener, TestResultCollector, Outputter.
 */
class CPPUNIT_API TestResult : protected SynchronizedObject
{
protected:
    OptionHelper m_aOptionHelper;

public:
  TestResult( GetOpt & _aOptions, SynchronizationObject *syncObject = 0 );
  virtual ~TestResult();

  virtual void addListener( TestListener *listener );
  virtual void removeListener( TestListener *listener );

  virtual void reset();
  virtual void stop();

  virtual bool shouldStop() const;

  virtual void startTest( Test *test );
  virtual void addError( Test *test, Exception *e, ErrorType::num eType=ErrorType::ET_ERROR);
  virtual void addFailure( Test *test, Exception *e );
  virtual void endTest( Test *test );

    // LLA: additionals
  virtual void addInfo(Test *test, const char *sInfo);

  virtual void enterNode(const char* Node);
  virtual void leaveNode(const char* Node);
  virtual std::string getNodeName();

  // if true, execution is allowed.
  virtual bool isAllowedToExecute(std::string const & sName);
  bool isOnlyShowJobs() {return m_aOptionHelper.isOnlyShowJobs();}
  bool isOptionWhereAmI();

  virtual void print(Outputter &);
  void setExitValue(int _nValue) {m_nExitValue = _nValue;}
  int getExitValue() {return m_nExitValue;}

protected:
  void addFailure( const TestFailure &failure );

protected:
  typedef std::deque<TestListener *> TestListeners;
  TestListeners m_listeners;
  bool m_stop;

    // this vector is used to expand the test name with a current node name
    std::vector<std::string> m_aCurrentNodeNames;

    //# std::vector<std::string> m_aNodes;

private:
  TestResult( const TestResult &other );
  TestResult &operator =( const TestResult &other );
  int m_nExitValue;
};


} // namespace CppUnit


#if CPPUNIT_NEED_DLL_DECL
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#endif

#endif // CPPUNIT_TESTRESULT_H


