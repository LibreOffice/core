#ifndef CPPUNIT_TESTSUITE_H    // -*- C++ -*-
#define CPPUNIT_TESTSUITE_H

#include <cppunit/Portability.h>

#if CPPUNIT_NEED_DLL_DECL
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4251 )  // X needs to have dll-interface to be used by clients of class Z
#endif
#endif

#include <cppunit/Test.h>
#include <vector>
#include <string>
#include <cppunit/autoregister/htestresult.h>

namespace CppUnit {

//# class TestResult;

#if CPPUNIT_NEED_DLL_DECL
  template class CPPUNIT_API std::vector<Test *>;
#endif


/*! \brief A Composite of Tests.
 * \ingroup CreatingTestSuite
 *
 * It runs a collection of test cases. Here is an example.
 * \code
 * CppUnit::TestSuite *suite= new CppUnit::TestSuite();
 * suite->addTest(new CppUnit::TestCaller<MathTest> (
 *                  "testAdd", testAdd));
 * suite->addTest(new CppUnit::TestCaller<MathTest> (
 *                  "testDivideByZero", testDivideByZero));
 * \endcode
 * Note that TestSuites assume lifetime
 * control for any tests added to them.
 *
 * \see Test
 * \see TestCaller
 */


class CPPUNIT_API TestSuite : public Test
{
public:
  TestSuite( std::string const& name = "" );
  ~TestSuite();

  void run( hTestResult pResult );
  int countTestCases() const;
  std::string getName() const;
  std::string toString() const;

  void addTest( Test *test );
  const std::vector<Test *> &getTests() const;

  virtual void deleteContents();

private:
  TestSuite( const TestSuite &other );
  TestSuite &operator =( const TestSuite &other );

private:
  std::vector<Test *> m_tests;
  const std::string m_name;
};


} // namespace CppUnit


#if CPPUNIT_NEED_DLL_DECL
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#endif

#endif // CPPUNIT_TESTSUITE_H
