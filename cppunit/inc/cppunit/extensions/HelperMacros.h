// //////////////////////////////////////////////////////////////////////////
// Header file HelperMacros.h
// (c)Copyright 2000, Baptiste Lepilleur.
// Created: 2001/04/15
// //////////////////////////////////////////////////////////////////////////
#ifndef CPPUNIT_EXTENSIONS_HELPERMACROS_H
#define CPPUNIT_EXTENSIONS_HELPERMACROS_H

#include <cppunit/Portability.h>
#include <cppunit/extensions/AutoRegisterSuite.h>
#include <cppunit/extensions/TestSuiteBuilder.h>
#include <string>

namespace CppUnit
{
  class TestFixture;

  /*! \brief Abstract TestFixture factory.
   */
  class TestFixtureFactory
  {
  public:
    //! Creates a new TestFixture instance.
    virtual CppUnit::TestFixture *makeFixture() =0;
  };
} // namespace CppUnit


// The macro __CPPUNIT_SUITE_CTOR_ARGS expand to an expression used to construct
// the TestSuiteBuilder with macro CPPUNIT_TEST_SUITE.
//
// The name of the suite is obtained using RTTI if CPPUNIT_USE_TYPEINFO_NAME
// is defined, otherwise it is extracted from the macro parameter
//
// This macro is for cppunit internal and should not be use otherwise.
#if CPPUNIT_USE_TYPEINFO_NAME
#  define __CPPUNIT_SUITE_CTOR_ARGS( ATestFixtureType )
#else
#  define __CPPUNIT_SUITE_CTOR_ARGS( ATestFixtureType ) (std::string(#ATestFixtureType))
#endif


/*! \addtogroup WritingTestFixture Writing test fixture
 */
/** @{
 */


/** \file
 * Macros intended to ease the definition of test suites.
 *
 * The macros
 * CPPUNIT_TEST_SUITE(), CPPUNIT_TEST(), and CPPUNIT_TEST_SUITE_END()
 * are designed to facilitate easy creation of a test suite.
 * For example,
 *
 * \code
 * #include <cppunit/extensions/HelperMacros.h>
 * class MyTest : public CppUnit::TestFixture {
 *   CPPUNIT_TEST_SUITE( MyTest );
 *   CPPUNIT_TEST( testEquality );
 *   CPPUNIT_TEST( testSetName );
 *   CPPUNIT_TEST_SUITE_END();
 * public:
 *   void testEquality();
 *   void testSetName();
 * };
 * \endcode
 *
 * The effect of these macros is to define two methods in the
 * class MyTest.  The first method is an auxiliary function
 * named registerTests that you will not need to call directly.
 * The second function
 * \code static CppUnit::TestSuite *suite()\endcode
 * returns a pointer to the suite of tests defined by the CPPUNIT_TEST()
 * macros.
 *
 * Rather than invoking suite() directly,
 * the macro CPPUNIT_TEST_SUITE_REGISTRATION() is
 * used to create a static variable that automatically
 * registers its test suite in a global registry.
 * The registry yields a Test instance containing all the
 * registered suites.
 * \code
 * CPPUNIT_TEST_SUITE_REGISTRATION( MyTest );
 * CppUnit::Test* tp =
 *   CppUnit::TestFactoryRegistry::getRegistry().makeTest();
 * \endcode
 *
 * The test suite macros can even be used with templated test classes.
 * For example:
 *
 * \code
 * template<typename CharType>
 * class StringTest : public CppUnit::TestFixture {
 *   CPPUNIT_TEST_SUITE( StringTest );
 *   CPPUNIT_TEST( testAppend );
 *   CPPUNIT_TEST_SUITE_END();
 * public:
 *   ...
 * };
 * \endcode
 *
 * You need to add in an implementation file:
 *
 * \code
 * CPPUNIT_TEST_SUITE_REGISTRATION( StringTest<char> );
 * CPPUNIT_TEST_SUITE_REGISTRATION( StringTest<wchar_t> );
 * \endcode
 */


/*! \brief Begin test suite
 *
 * This macro starts the declaration of a new test suite.
 * Use CPPUNIT_TEST_SUB_SUITE() instead, if you wish to include the
 * test suite of the parent class.
 *
 * \param ATestFixtureType Type of the test case class. This type \b MUST
 *                         be derived from TestFixture.
 * \see CPPUNIT_TEST_SUB_SUITE, CPPUNIT_TEST, CPPUNIT_TEST_SUITE_END,
 * \see CPPUNIT_TEST_SUITE_REGISTRATION, CPPUNIT_TEST_EXCEPTION, CPPUNIT_TEST_FAIL.
 */
#define CPPUNIT_TEST_SUITE( ATestFixtureType )                            \
  private:                                                                \
    typedef ATestFixtureType __ThisTestFixtureType;                       \
    class ThisTestFixtureFactory : public CppUnit::TestFixtureFactory     \
    {                                                                     \
      virtual CppUnit::TestFixture *makeFixture()                         \
      {                                                                   \
        return new ATestFixtureType();                                    \
      }                                                                   \
    };                                                                    \
  public:                                                                 \
    static CppUnit::TestSuite *suite()                                    \
    {                                                                     \
      CppUnit::TestSuiteBuilder<__ThisTestFixtureType>                    \
          builder __CPPUNIT_SUITE_CTOR_ARGS( ATestFixtureType );          \
      ThisTestFixtureFactory factory;                                     \
      __ThisTestFixtureType::registerTests( builder.suite(), &factory );  \
      return builder.takeSuite();                                         \
    }                                                                     \
    static void                                                           \
    registerTests( CppUnit::TestSuite *suite,                             \
                   CppUnit::TestFixtureFactory *factory )                 \
    {                                                                     \
      CppUnit::TestSuiteBuilder<__ThisTestFixtureType> builder( suite );


/*! \brief Begin test suite (includes parent suite)
 *
 * This macro may only be used in a class whose parent class
 * defines a test suite using CPPUNIT_TEST_SUITE() or CPPUNIT_TEST_SUB_SUITE().
 *
 * This macro begins the declaration of a test suite, in the same
 * manner as CPPUNIT_TEST_SUITE().  In addition, the test suite of the
 * parent is automatically inserted in the test suite being
 * defined.
 *
 * Here is an example:
 *
 * \code
 * #include <cppunit/extensions/HelperMacros.h>
 * class MySubTest : public MyTest {
 *   CPPUNIT_TEST_SUB_SUITE( MySubTest, MyTest );
 *   CPPUNIT_TEST( testAdd );
 *   CPPUNIT_TEST( testSub );
 *   CPPUNIT_TEST_SUITE_END();
 * public:
 *   void testAdd();
 *   void testSub();
 * };
 * \endcode
 *
 * \param ATestFixtureType Type of the test case class. This type \b MUST
 *                         be derived from TestFixture.
 * \param ASuperClass   Type of the parent class.
 * \see CPPUNIT_TEST_SUITE.
 */
#define CPPUNIT_TEST_SUB_SUITE( ATestFixtureType, ASuperClass )  \
  private:                                                       \
    typedef ASuperClass __ThisSuperClassType;                    \
    CPPUNIT_TEST_SUITE( ATestFixtureType );                      \
      __ThisSuperClassType::registerTests( suite, factory )


/*! \brief Add a method to the suite.
 * \param testMethod Name of the method of the test case to add to the
 *                   suite. The signature of the method must be of
 *                   type: void testMethod();
 * \see  CPPUNIT_TEST_SUITE.
 */
#define CPPUNIT_TEST( testMethod )                                           \
      builder.addTestCaller( #testMethod,                                    \
                             &__ThisTestFixtureType::testMethod ,            \
                             (__ThisTestFixtureType*)factory->makeFixture() )


/*! \brief Add a test which fail if the specified exception is not caught.
 *
 * Example:
 * \code
 * #include <cppunit/extensions/HelperMacros.h>
 * #include <vector>
 * class MyTest : public CppUnit::TestFixture {
 *   CPPUNIT_TEST_SUITE( MyTest );
 *   CPPUNIT_TEST_EXCEPTION( testVectorAtThrow, std::invalid_argument );
 *   CPPUNIT_TEST_SUITE_END();
 * public:
 *   void testVectorAtThrow()
 *   {
 *     std::vector<int> v;
 *     v.at( 1 );     // must throw exception std::invalid_argument
 *   }
 * };
 * \endcode
 *
 * \param testMethod Name of the method of the test case to add to the suite.
 * \param ExceptionType Type of the exception that must be thrown by the test
 *                      method.
 */
#define CPPUNIT_TEST_EXCEPTION( testMethod, ExceptionType )                   \
      builder.addTestCallerForException( #testMethod,                         \
                             &__ThisTestFixtureType::testMethod ,             \
                             (__ThisTestFixtureType*)factory->makeFixture(),  \
                             (ExceptionType *)NULL );

/*! \brief Adds a test case which is excepted to fail.
 *
 * The added test case expect an assertion to fail. You usually used that type
 * of test case when testing custom assertion macros.
 *
 * \code
 * CPPUNIT_TEST_FAIL( testAssertFalseFail );
 *
 * void testAssertFalseFail()
 * {
 *   CPPUNIT_ASSERT( false );
 * }
 * \endcode
 * \see CreatingNewAssertions.
 */
#define CPPUNIT_TEST_FAIL( testMethod ) \
              CPPUNIT_TEST_EXCEPTION( testMethod, CppUnit::Exception )

/*! \brief End declaration of the test suite.
 *
 * After this macro, member access is set to "private".
 *
 * \see  CPPUNIT_TEST_SUITE.
 * \see  CPPUNIT_TEST_SUITE_REGISTRATION.
 */
#define CPPUNIT_TEST_SUITE_END()                                          \
      builder.takeSuite();                                                \
    }                                                                     \
  private: /* dummy typedef so that the macro can still end with ';'*/    \
    typedef ThisTestFixtureFactory __ThisTestFixtureFactory

/** @}
 */

#define __CPPUNIT_CONCATENATE_DIRECT( s1, s2 ) s1##s2
#define __CPPUNIT_CONCATENATE( s1, s2 ) __CPPUNIT_CONCATENATE_DIRECT( s1, s2 )

/** Decorates the specified string with the line number to obtain a unique name;
 * @param str String to decorate.
 */
#define __CPPUNIT_MAKE_UNIQUE_NAME( str ) __CPPUNIT_CONCATENATE( str, __LINE__ )


/** Adds the specified fixture suite to the unnamed registry.
 * \ingroup CreatingTestSuite
 *
 * This macro declares a static variable whose construction
 * causes a test suite factory to be inserted in a global registry
 * of such factories.  The registry is available by calling
 * the static function CppUnit::TestFactoryRegistry::getRegistry().
 *
 * \param ATestFixtureType Type of the test case class.
 * \warning This macro should be used only once per line of code (the line
 *          number is used to name a hidden static variable).
 * \see CPPUNIT_TEST_SUITE_NAMED_REGISTRATION
 * \see CPPUNIT_TEST_SUITE, CppUnit::AutoRegisterSuite,
 *      CppUnit::TestFactoryRegistry.
 */

/* LLA:
 #define CPPUNIT_TEST_SUITE_REGISTRATION( ATestFixtureType )      \
   static CppUnit::AutoRegisterSuite< ATestFixtureType >          \
              __CPPUNIT_MAKE_UNIQUE_NAME(__autoRegisterSuite )
*/

/** Adds the specified fixture suite to the specified registry suite.
 * \ingroup CreatingTestSuite
 *
 * This macro declares a static variable whose construction
 * causes a test suite factory to be inserted in the global registry
 * suite of the specified name. The registry is available by calling
 * the static function CppUnit::TestFactoryRegistry::getRegistry().
 *
 * For the suite name, use a string returned by a static function rather
 * than a hardcoded string. That way, you can know what are the name of
 * named registry and you don't risk mistyping the registry name.
 *
 * \code
 * // MySuites.h
 * namespace MySuites {
 *   std::string math() {
 *     return "Math";
 *   }
 * }
 *
 * // ComplexNumberTest.cpp
 * #include "MySuites.h"
 *
 * CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ComplexNumberTest, MySuites::math() );
 * \endcode
 *
 * \param ATestFixtureType Type of the test case class.
 * \param suiteName Name of the global registry suite the test suite is
 *                  registered into.
 * \warning This macro should be used only once per line of code (the line
 *          number is used to name a hidden static variable).
 * \see CPPUNIT_TEST_SUITE_REGISTRATION
 * \see CPPUNIT_TEST_SUITE, CppUnit::AutoRegisterSuite,
 *      CppUnit::TestFactoryRegistry..
 */
#define CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ATestFixtureType, suiteName ) \
  static CppUnit::AutoRegisterSuite< ATestFixtureType >                      \
             __CPPUNIT_MAKE_UNIQUE_NAME(__autoRegisterSuite )( suiteName )


#endif  // CPPUNIT_EXTENSIONS_HELPERMACROS_H
