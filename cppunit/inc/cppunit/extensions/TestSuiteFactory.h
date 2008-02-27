#ifndef CPPUNIT_EXTENSIONS_TESTSUITEFACTORY_H
#define CPPUNIT_EXTENSIONS_TESTSUITEFACTORY_H

#include <cppunit/extensions/TestFactory.h>

namespace CppUnit {

  class Test;

  /*! \brief TestFactory for TestFixture that implements a static suite() method.
   * \see AutoRegisterSuite.
   */
  template<typename TestCaseType>
  class TestSuiteFactory : public TestFactory
  {
  public:
    virtual Test *makeTest()
    {
      return TestCaseType::suite();
    }
  };

}  // namespace CppUnit

#endif  // CPPUNIT_EXTENSIONS_TESTSUITEFACTORY_H
