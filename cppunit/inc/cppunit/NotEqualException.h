#ifndef NOTEQUALEXCEPTION_H
#define NOTEQUALEXCEPTION_H

#include <cppunit/Exception.h>


namespace CppUnit {

/*! \brief Exception thrown by failed equality assertions.
 * \ingroup BrowsingCollectedTestResult
 */
class CPPUNIT_API NotEqualException : public Exception
{
public:
  /*! Constructs the exception.
   * \param expected Text that represents the expected value.
   * \param actual Text that represents the actual value.
   * \param sourceLine Location of the assertion.
   * \param additionalMessage Additionnal information provided to further qualify
   *                          the inequality.
   */
  NotEqualException( std::string const& expected,
                     std::string const& actual,
                     SourceLine const& sourceLine = SourceLine(),
                     std::string const& additionalMessage = "" );

#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
  NotEqualException( std::string const& expected,
                     std::string const& actual,
                     long lineNumber,
                     std::string const& fileName );
#endif

  NotEqualException( const NotEqualException &other );


  virtual ~NotEqualException() throw();

  std::string expectedValue() const;

  std::string actualValue() const;

  std::string additionalMessage() const;

  /*! Copy operator.
   * @param other Object to copy.
   * @return Reference on this object.
   */
  NotEqualException &operator =( const NotEqualException &other );

  Exception *clone() const;

  bool isInstanceOf( const Type &type ) const;

  static Type type();

private:
  std::string m_expected;
  std::string m_actual;
  std::string m_additionalMessage;
};

}  // namespace CppUnit

#endif  // NOTEQUALEXCEPTION_H
