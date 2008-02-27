#ifndef CPPUNIT_EXTENSIONS_TESTSUITEBUILDER_H
#define CPPUNIT_EXTENSIONS_TESTSUITEBUILDER_H

#include <cppunit/Portability.h>
#include <memory>
#include <cppunit/TestSuite.h>
#include <cppunit/TestCaller.h>

#if CPPUNIT_USE_TYPEINFO_NAME
#  include <cppunit/extensions/TypeInfoHelper.h>
#endif

namespace CppUnit {

  /*! \brief Helper to add tests to a TestSuite.
   * \ingroup WritingTestFixture
   *
   * All tests added to the TestSuite are prefixed by TestSuite name. The resulting
   * TestCase name has the following pattern:
   *
   * MyTestSuiteName.myTestName
   */
  template<typename Fixture>
  class TestSuiteBuilder
  {
    public:
      typedef void (Fixture::*TestMethod)();

#if CPPUNIT_USE_TYPEINFO_NAME
      TestSuiteBuilder() :
          m_suite( new TestSuite(
              TypeInfoHelper::getClassName( typeid(Fixture) )  ) )
      {
      }
#endif

      TestSuiteBuilder( TestSuite *_suite ) : m_suite( _suite )
      {
      }

      TestSuiteBuilder(std::string const& name) : m_suite( new TestSuite(name) )
      {
      }

      TestSuite *suite() const
      {
        return m_suite.get();
      }

      TestSuite *takeSuite()
      {
        return m_suite.release();
      }

      void addTest( Test *test )
      {
        m_suite->addTest( test );
      }

      void addTestCaller( std::string const& methodName,
                          TestMethod testMethod )
      {
          Test *test =
              new TestCaller<Fixture>( makeTestName( methodName ),
                                       testMethod );
          addTest( test );
      }

      void addTestCaller( std::string const& methodName,
                          TestMethod testMethod,
                          Fixture *fixture )
      {
          Test *test =
              new TestCaller<Fixture>( makeTestName( methodName ),
                                       testMethod,
                                       fixture);
          addTest( test );
      }

      template<typename ExceptionType>
      void addTestCallerForException( std::string const& methodName,
                                      TestMethod testMethod,
                                      Fixture *fixture,
                                      ExceptionType *dummyPointer )
      {
          Test *test = new TestCaller<Fixture,ExceptionType>(
                                       makeTestName( methodName ),
                                       testMethod,
                                       fixture);
          addTest( test );
      }


      std::string makeTestName( const std::string &methodName )
      {
          // return m_suite->getName() + "." + methodName;
          return methodName;
      }

    private:
      std::auto_ptr<TestSuite> m_suite;
  };

}  // namespace CppUnit

#endif  // CPPUNIT_EXTENSIONS_TESTSUITEBUILDER_H
