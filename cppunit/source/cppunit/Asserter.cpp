#include <cppunit/Asserter.h>
#include <cppunit/NotEqualException.h>
#include <cppunit/stringhelper.hxx>

namespace CppUnit
{


namespace Asserter
{

void
fail( std::string const& message,
      SourceLine const& sourceLine )
{
    throw Exception( message, sourceLine );
}

void
failStub( std::string const& message,
          SourceLine const& sourceLine )
{
    throw StubException( message, sourceLine );
}


void
failIf( bool shouldFail,
        std::string const& message,
        SourceLine const& location )
{
  if ( shouldFail )
  {
      fail( message, location );
  }
}

// -----------------------------------------------------------------------------
void
failIf( bool shouldFail,
        rtl::OUString const& _suMessage,
        SourceLine const& location )
{
  if ( shouldFail )
  {
      rtl::OString sMessage;
      sMessage <<= _suMessage;
      std::string message;
      message = sMessage.getStr();
      fail( message, location );
  }
}
// -----------------------------------------------------------------------------

void
failNotEqual( std::string const& expected,
              std::string const& actual,
              SourceLine const& sourceLine,
              std::string const& additionalMessage )
{
    throw NotEqualException( expected,
                             actual,
                             sourceLine,
                             additionalMessage );
}


void
failNotEqualIf( bool shouldFail,
                std::string const& expected,
                std::string const& actual,
                SourceLine const& sourceLine,
                std::string const& additionalMessage )
{
  if ( shouldFail )
    failNotEqual( expected, actual, sourceLine, additionalMessage );
}


} // namespace Asserter
} // namespace CppUnit
