#ifndef CPPUNIT_TESTRESULTCOLLECTOR_H
#define CPPUNIT_TESTRESULTCOLLECTOR_H

#include <cppunit/Portability.h>

#if CPPUNIT_NEED_DLL_DECL
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4251 )  // X needs to have dll-interface to be used by clients of class Z
#endif
#endif

#include <cppunit/result/TestResult.h>
#include <cppunit/result/TestSucessListener.h>
#include <deque>
#include <vector>

namespace CppUnit
{


#if CPPUNIT_NEED_DLL_DECL
  template class CPPUNIT_API std::deque<TestFailure *>;
  template class CPPUNIT_API std::deque<Test *>;
#endif


/*! \brief Collects test result.
 * \ingroup WritingTestResult
 * \ingroup BrowsingCollectedTestResult
 *
 * A TestResultCollector is a TestListener which collects the results of executing
 * a test case. It is an instance of the Collecting Parameter pattern.
 *
 * The test framework distinguishes between failures and errors.
 * A failure is anticipated and checked for with assertions. Errors are
 * unanticipated problems signified by exceptions that are not generated
 * by the framework.
 * \see TestListener, TestFailure.
 */

class OneStringContainer
{
    std::string m_sName;
protected:
    OneStringContainer() {}
public:
    OneStringContainer(std::string const& _sName)
            :m_sName(_sName){}
    std::string getString() const {return m_sName;}
    virtual ~OneStringContainer(){}
};

// -----------------------------------------------------------------------------
class TestEnvelope : public OneStringContainer
{
    Test* m_pTest;
public:
    TestEnvelope():m_pTest(NULL){}

    TestEnvelope(Test* _pTest, std::string const& _sName)
            : OneStringContainer(_sName),
              m_pTest(_pTest)
        {}

    Test* getTest() {return m_pTest;}
    virtual ~TestEnvelope(){}

};

// -----------------------------------------------------------------------------
class TestInfo : public TestEnvelope
{
public:
    TestInfo(Test* _pTest, std::string const& _sName)
            :TestEnvelope(_pTest, _sName)
        {}
};

// -----------------------------------------------------------------------------
class TestFailureEnvelope : public OneStringContainer
{
    TestFailure* m_pTestFailure;
public:
    TestFailureEnvelope():m_pTestFailure(NULL){}

    TestFailureEnvelope(TestFailure* _pTestFailure, std::string const& _sName)
            :OneStringContainer(_sName),
             m_pTestFailure(_pTestFailure)
        {}

    TestFailure* getTestFailure() {return m_pTestFailure;}
    virtual ~TestFailureEnvelope(){}

};
// -----------------------------------------------------------------------------

class CPPUNIT_API TestResultCollector : public TestSucessListener
{
    TestResult* m_pResult;
public:
    typedef std::deque<TestFailureEnvelope *> TestFailures;
    typedef std::deque<TestEnvelope *> Tests;
    typedef std::vector<TestInfo *> TestInfos;


  /*! Constructs a TestResultCollector object.
   */
  TestResultCollector( TestResult *_pResult, SynchronizationObject *syncObject = 0 );

  /// Destructor.
  virtual ~TestResultCollector();

  void startTest( Test *test );
  void endTest( Test *test );

  void addFailure( const TestFailure &failure );

  virtual void reset();

  virtual int runTests() const;
  virtual int testErrors() const;
  virtual int testFailures() const;
  virtual int testFailuresTotal() const;

  virtual const TestFailures& failures() const;
  virtual const Tests &tests() const;
  virtual std::string getInfo(Test*);

  virtual void addInfo(Test *test, const char *sInfo);

    // virtual void enterNode(const char* Node);
    // virtual void leaveNode(const char* Node);

protected:
  Tests        m_tests;
  TestFailures m_failures;
  TestInfos    m_aInfos;

  int m_testErrors;

    // this vector is used to expand the test name with a current node name
    // std::vector<std::string> m_aCurrentNodeNames;
    // std::string getNodeName();
private:
  /// Prevents the use of the copy constructor.
  TestResultCollector( const TestResultCollector &copy );

  /// Prevents the use of the copy operator.
  void operator =( const TestResultCollector &copy );
};



} //  namespace CppUnit


#if CPPUNIT_NEED_DLL_DECL
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#endif


#endif  // CPPUNIT_TESTRESULTCOLLECTOR_H
