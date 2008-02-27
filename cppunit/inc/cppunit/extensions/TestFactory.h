#ifndef CPPUNIT_EXTENSIONS_TESTFACTORY_H
#define CPPUNIT_EXTENSIONS_TESTFACTORY_H

#include <cppunit/Portability.h>

namespace CppUnit {

class Test;

/*! \brief Abstract Test factory.
 */
class CPPUNIT_API TestFactory
{
public:
  virtual ~TestFactory() {}

  /*! Makes a new test.
   * \return A new Test.
   */
  virtual Test* makeTest() = 0;
};

}  // namespace CppUnit

#endif  // CPPUNIT_EXTENSIONS_TESTFACTORY_H
