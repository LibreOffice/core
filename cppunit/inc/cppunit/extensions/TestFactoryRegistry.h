#ifndef CPPUNIT_EXTENSIONS_TESTFACTORYREGISTRY_H
#define CPPUNIT_EXTENSIONS_TESTFACTORYREGISTRY_H

#include <cppunit/Portability.h>

#if CPPUNIT_NEED_DLL_DECL
#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable: 4251 )  // X needs to have dll-interface to be used by clients of class Z
#endif
#endif

#include <cppunit/extensions/TestFactory.h>
#include <map>
#include <string>

#include <cppunit/autoregister/htestresult.h>

namespace CppUnit {

class TestSuite;

#if CPPUNIT_NEED_DLL_DECL
  template class CPPUNIT_API std::map<std::string, TestFactory *>;
#endif


/*! \brief Registry for TestFactory.
 * \ingroup CreatingTestSuite
 *
 * Notes that the registry assumes lifetime control for any registered test.
 *
 * To register tests, use the macros:
 * - CPPUNIT_TEST_SUITE_REGISTRATION(): to add tests in the unnamed registry.
 * - CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(): to add tests in a named registry.
 *
 * Example 1: retreiving a suite that contains all the test registered with
 * CPPUNIT_TEST_SUITE_REGISTRATION().
 * \code
 * CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
 * CppUnit::TestSuite *suite = registry.makeTest();
 * \endcode
 *
 * Example 2: retreiving a suite that contains all the test registered with
 * \link CPPUNIT_TEST_SUITE_NAMED_REGISTRATION() CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ..., "Math" )\endlink.
 * \code
 * CppUnit::TestFactoryRegistry &mathRegistry = CppUnit::TestFactoryRegistry::getRegistry( "Math" );
 * CppUnit::TestSuite *mathSuite = mathRegistry.makeTest();
 * \endcode
 *
 * Example 3: creating a test suite hierarchy composed of unnamed registration and
 * named registration:
 * - All Tests
 *   - tests registered with CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ..., "Graph" )
 *   - tests registered with CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ..., "Math" )
 *   - tests registered with CPPUNIT_TEST_SUITE_REGISTRATION
 *
 * \code
 * CppUnit::TestSuite *rootSuite = new CppUnit::TestSuite( "All tests" );
 * rootSuite->addTest( CppUnit::TestFactoryRegistry::getRegistry( "Graph" ).makeTest() );
 * rootSuite->addTest( CppUnit::TestFactoryRegistry::getRegistry( "Math" ).makeTest() );
 * CppUnit::TestFactoryRegistry::getRegistry().addTestToSuite( rootSuite );
 * \endcode
 *
 * The same result can be obtained with:
 * \code
 * CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
 * registry.registerFactory( CppUnit::TestFactoryRegistry::getRegistry( "Graph" ) );
 * registry.registerFactory( CppUnit::TestFactoryRegistry::getRegistry( "Math" ) );
 * CppUnit::TestSuite *suite = registry.makeTest();
 * \endcode
 *
 * Since a TestFactoryRegistry is a TestFactory, the named registries can be
 * registered in the unnamed registry, creating the hierarchy links.
 *
 * \see TestSuiteFactory, AutoRegisterSuite
 * \see CPPUNIT_TEST_SUITE_REGISTRATION, CPPUNIT_TEST_SUITE_NAMED_REGISTRATION
 */
class CPPUNIT_API TestFactoryRegistry : public TestFactory
{
public:
  /** Constructs the registry with the specified name.
   * \param name Name of the registry. It is the name of TestSuite returned by
   *             makeTest().
   */
  TestFactoryRegistry( std::string const& name = "All Tests" );

  /// Destructor.
  virtual ~TestFactoryRegistry();

  /** Returns a new TestSuite that contains the registered test.
   * \return A new TestSuite which contains all the test added using
   * registerFactory(TestFactory *).
   */
  virtual Test *makeTest();

  /** Returns unnamed the registry.
   * TestSuite registered using CPPUNIT_TEST_SUITE_REGISTRATION() are registered
   * in this registry.
   * \return Registry which name is "All Tests".
   */
  static TestFactoryRegistry &getRegistry();

  /** Returns a named registry.
   * TestSuite registered using CPPUNIT_TEST_SUITE_NAMED_REGISTRATION() are registered
   * in the registry of the same name.
   * \param name Name of the registry to return.
   * \return Registry. If the registry does not exist, it is created with the
   *         specified name.
   */
  static TestFactoryRegistry &getRegistry( const std::string &name );
  static void testRegistries(hTestResult _pResult);

  /** Adds the registered tests to the specified suite.
   * \param suite Suite the tests are added to.
   */
  void addTestToSuite( TestSuite *suite );

  /** Adds the specified TestFactory with a specific name (DEPRECATED).
   * \param name Name associated to the factory.
   * \param factory Factory to register.
   * \deprecated Use registerFactory( TestFactory *) instead.
   */
protected:

  void registerFactory( const std::string &name,
                        TestFactory *factory );
public:

  /** Adds the specified TestFactory to the registry.
   *
   * \param factory Factory to register.
   */
  void registerFactory( TestFactory *factory );

private:
  TestFactoryRegistry( const TestFactoryRegistry &copy );
  void operator =( const TestFactoryRegistry &copy );

private:
  typedef std::map<std::string, TestFactory *> Factories;
  Factories m_factories;

  std::string m_name;
};


}  // namespace CppUnit


#if CPPUNIT_NEED_DLL_DECL
#ifdef _MSC_VER
#pragma warning( pop )
#endif
#endif


#endif  // CPPUNIT_EXTENSIONS_TESTFACTORYREGISTRY_H
