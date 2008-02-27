#include <cppunit/NotEqualException.h>

namespace CppUnit {


NotEqualException::NotEqualException( std::string const& _expected,
                                      std::string const& _actual,
                                      SourceLine const& _sourceLine ,
                                      std::string const& _additionalMessage ) :
    Exception( "Expected: " + _expected +
                   ", but was: " + _actual +
                   "." + _additionalMessage ,
               _sourceLine),
    m_expected( _expected ),
    m_actual( _actual ),
    m_additionalMessage( _additionalMessage )
{
}


#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
/*!
 * \deprecated Use other constructor instead.
 */
NotEqualException::NotEqualException( std::string const& expected,
                                      std::string const& actual,
                                      long lineNumber,
                                      std::string const& fileName ) :
    Exception( "Expected: " + expected + ", but was: " + actual,
               lineNumber,
               fileName ),
    m_expected( expected ),
    m_actual( actual )
{
}
#endif


NotEqualException::NotEqualException( const NotEqualException &other ) :
    Exception( other ),
    m_expected( other.m_expected ),
    m_actual( other.m_actual ),
    m_additionalMessage( other.m_additionalMessage )
{
}


NotEqualException::~NotEqualException() throw()
{
}


NotEqualException &
NotEqualException::operator =( const NotEqualException &other )
{
  Exception::operator =( other );

  if ( &other != this )
  {
    m_expected = other.m_expected;
    m_actual = other.m_actual;
    m_additionalMessage = other.m_additionalMessage;
  }
  return *this;
}


Exception *
NotEqualException::clone() const
{
  return new NotEqualException( *this );
}


bool
NotEqualException::isInstanceOf( const Type &exceptionType ) const
{
  return exceptionType == type()  ||
         Exception::isInstanceOf( exceptionType );
}


Exception::Type
NotEqualException::type()
{
  return Type( "CppUnit::NotEqualException" );
}


std::string
NotEqualException::expectedValue() const
{
  return m_expected;
}


std::string
NotEqualException::actualValue() const
{
  return m_actual;
}


std::string
NotEqualException::additionalMessage() const
{
  return m_additionalMessage;
}


}  //  namespace CppUnit
