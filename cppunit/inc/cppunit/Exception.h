#ifndef CPPUNIT_EXCEPTION_H
#define CPPUNIT_EXCEPTION_H

#include <cppunit/Portability.h>
#include <cppunit/SourceLine.h>
#include <exception>
#include <string>

namespace CppUnit {

/*! \brief Exceptions thrown by failed assertions.
 * \ingroup BrowsingCollectedTestResult
 *
 * Exception is an exception that serves
 * descriptive strings through its what() method
 */
class CPPUNIT_API Exception : public std::exception
{
public:

    class Type
    {
    public:
        Type( std::string const& _type ) : m_type ( _type ) {}

        bool operator ==( const Type &other ) const
        {
        return m_type == other.m_type;
        }
    private:
        const std::string m_type;
    };


    Exception( std::string  const& message = "",
           SourceLine const& sourceLine = SourceLine() );

#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
    Exception( std::string  const& message,
           long lineNumber,
           std::string const& fileName );
#endif

    Exception (const Exception& other);

    virtual ~Exception () throw();

    Exception& operator= (const Exception& other);

    const char *what() const throw ();

    SourceLine sourceLine() const;

#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
    long lineNumber() const;
    std::string fileName() const;

    static const std::string UNKNOWNFILENAME;
    static const long UNKNOWNLINENUMBER;
#endif

    virtual Exception *clone() const;

    virtual bool isInstanceOf( const Type &type ) const;

    static Type type();

private:
    // VC++ does not recognize call to parent class when prefixed
    // with a namespace. This is a workaround.
    typedef std::exception SuperClass;

    std::string m_message;
    SourceLine m_sourceLine;
};

// -----------------------------------------------------------------------------

class StubException : public Exception
{
public:
    StubException( std::string const&  _message = "",
                   SourceLine const& _sourceLine = SourceLine() )
            :Exception(_message, _sourceLine) {}


#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
    StubException( std::string const& message,
                   long lineNumber,
                   std::string const& fileName )
            :Exception(message, lineNumber, fileName) {}
#endif
};

// -----------------------------------------------------------------------------

class SignalException : public Exception
{
public:
    SignalException( std::string const& _message = "",
                     SourceLine const& _sourceLine = SourceLine() )
            :Exception(_message, _sourceLine) {}


#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
    SignalException( std::string const& message,
                   long lineNumber,
                   std::string const& fileName )
            :Exception(message, lineNumber, fileName) {}
#endif
};

} // namespace CppUnit

#endif // CPPUNIT_EXCEPTION_H

