#include <cppunit/SourceLine.h>


namespace CppUnit
{

SourceLine::SourceLine() :
    m_lineNumber( -1 )
{
}


SourceLine::SourceLine( const std::string &file,
                        int line ) :
    m_fileName( file ),
    m_lineNumber( line )
{
}


SourceLine::~SourceLine()
{
}


bool
SourceLine::isValid() const
{
  return !m_fileName.empty();
}


int
SourceLine::lineNumber() const
{
  return m_lineNumber;
}


std::string
SourceLine::fileName() const
{
  return m_fileName;
}


bool
SourceLine::operator ==( const SourceLine &other ) const
{
  return m_fileName == other.m_fileName  &&
         m_lineNumber == other.m_lineNumber;
}


bool
SourceLine::operator !=( const SourceLine &other ) const
{
  return !( *this == other );
}


} // namespace CppUnit
