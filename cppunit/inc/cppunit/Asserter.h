#ifndef CPPUNIT_ASSERTER_H
#define CPPUNIT_ASSERTER_H

#include <cppunit/Portability.h>
#include <cppunit/SourceLine.h>
#include <string>
#include <rtl/ustring.hxx>

namespace CppUnit
{

/*! \brief A set of functions to help writing assertion macros.
 * \ingroup CreatingNewAssertions
 *
 * Here is an example of assertion, a simplified version of the
 * actual assertion implemented in examples/cppunittest/XmlUniformiser.h:
 * \code
 * #include <cppunit/SourceLine.h>
 * #include <cppunit/TestAssert.h>
 *
 * void
 * checkXmlEqual( std::string expectedXml,
 *                std::string actualXml,
 *                CppUnit::SourceLine sourceLine )
 * {
 *   std::string expected = XmlUniformiser( expectedXml ).stripped();
 *   std::string actual = XmlUniformiser( actualXml ).stripped();
 *
 *   if ( expected == actual )
 *     return;
 *
 *   ::CppUnit::Asserter::failNotEqual( expected,
 *                                      actual,
 *                                      sourceLine );
 * }
 *
 * /// Asserts that two XML strings are equivalent.
 * #define CPPUNITTEST_ASSERT_XML_EQUAL( expected, actual ) \
 *     checkXmlEqual( expected, actual,                     \
 *                    CPPUNIT_SOURCELINE() )
 * \endcode
 */
namespace Asserter
{

  /*! Throws a Exception with the specified message and location.
   */
  void CPPUNIT_API fail( std::string const& message,
                         SourceLine const& sourceLine = SourceLine() );

  /*! Throws a Exception with the specified message and location.
   * \param shouldFail if \c true then the exception is thrown. Otherwise
   *                   nothing happen.
   * \param message Message explaining the assertion failiure.
   * \param sourceLine Location of the assertion.
   */
  void CPPUNIT_API failIf( bool shouldFail,
                           std::string const& message,
                           SourceLine const& sourceLine = SourceLine() );


    // LLA: test, if it's possible to use a DEFINE twice ;-)
  void CPPUNIT_API failIf( bool shouldFail,
                           rtl::OUString const& message,
                           SourceLine const& sourceLine = SourceLine() );

  /*! Throws a NotEqualException with the specified message and location.
   * \param expected Text describing the expected value.
   * \param actual Text describing the actual value.
   * \param additionalMessage Additional message. Usually used to report
   *                          where the "difference" is located.
   * \param sourceLine Location of the assertion.
   */
  void CPPUNIT_API failNotEqual( std::string const& expected,
                                 std::string const& actual,
                                 SourceLine const& sourceLine = SourceLine(),
                                 std::string const& additionalMessage ="" );

  /*! Throws a NotEqualException with the specified message and location.
   * \param shouldFail if \c true then the exception is thrown. Otherwise
   *                   nothing happen.
   * \param expected Text describing the expected value.
   * \param actual Text describing the actual value.
   * \param additionalMessage Additional message. Usually used to report
   *                          where the "difference" is located.
   * \param sourceLine Location of the assertion.
   */
  void CPPUNIT_API failNotEqualIf( bool shouldFail,
                                   std::string const& expected,
                                   std::string const& actual,
                                   SourceLine const& sourceLine = SourceLine(),
                                   std::string const& additionalMessage ="" );


  /*! Throws an Exception with the specified message and location.
   * \param message Message explaining the assertion failure.
   * \param sourceLine Location of the assertion.
   */
  void CPPUNIT_API failStub( std::string const& message,
                             SourceLine const& sourceLine = SourceLine() );

} // namespace Asserter
} // namespace CppUnit


#endif  // CPPUNIT_ASSERTER_H
