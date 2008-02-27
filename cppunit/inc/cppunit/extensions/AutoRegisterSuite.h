#ifndef CPPUNIT_EXTENSIONS_AUTOREGISTERSUITE_H
#define CPPUNIT_EXTENSIONS_AUTOREGISTERSUITE_H

#include <stdio.h>
#include <string>
#include <cppunit/extensions/TestSuiteFactory.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

namespace CppUnit {

  /** Automatically register the test suite of the specified type.
   *
   * You should not use this class directly. Instead, use the following macros:
   * - CPPUNIT_TEST_SUITE_REGISTRATION()
   * - CPPUNIT_TEST_SUITE_NAMED_REGISTRATION()
   *
   * This object will register the test returned by TestCaseType::suite()
   * when constructed to the test registry.
   *
   * This object is intented to be used as a static variable.
   *
   *
   * \param TestCaseType Type of the test case which suite is registered.
   * \see CPPUNIT_TEST_SUITE_REGISTRATION, CPPUNIT_TEST_SUITE_NAMED_REGISTRATION
   * \see CppUnit::TestFactoryRegistry.
   */
  template<typename TestCaseType>
  class AutoRegisterSuite
  {
    public:
      /** Auto-register the suite factory in the global registry.
       */
      AutoRegisterSuite()
          {
              TestFactory *factory = new TestSuiteFactory<TestCaseType>();
              TestFactoryRegistry::getRegistry().registerFactory( factory );
      }

      /** Auto-register the suite factory in the specified registry.
       * \param name Name of the registry.
       */
      AutoRegisterSuite( const std::string &name )
          {
              // printf("AutoRegisterSuite %s\n", name.c_str());
              TestFactory *factory = new TestSuiteFactory<TestCaseType>();
              TestFactoryRegistry::getRegistry( name ).registerFactory( factory );
          }
  };

} // namespace CppUnit


#endif  // CPPUNIT_EXTENSIONS_AUTOREGISTERSUITE_H
