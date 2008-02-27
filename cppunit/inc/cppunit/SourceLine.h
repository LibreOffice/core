#ifndef CPPUNIT_SOURCELINE_H
#define CPPUNIT_SOURCELINE_H

#include <cppunit/Portability.h>
#include <string>

/*! \brief Constructs a SourceLine object initialized with the location where the macro is expanded.
 * \ingroup CreatingNewAssertions
 * \relates CppUnit::SourceLine
 * Used to write your own assertion macros.
 * \see Asserter for example of usage.
 */
#define CPPUNIT_SOURCELINE() ::CppUnit::SourceLine( __FILE__, __LINE__ )


namespace CppUnit
{

/*! \brief Represents a source line location.
 * \ingroup CreatingNewAssertions
 * \ingroup BrowsingCollectedTestResult
 *
 * Used to capture the failure location in assertion.
 *
 * Use the CPPUNIT_SOURCELINE() macro to construct that object. Typically used when
 * writing an assertion macro in association with Asserter.
 *
 * \see Asserter.
 */
class CPPUNIT_API SourceLine
{
public:
  SourceLine();

  SourceLine( const std::string &fileName,
              int lineNumber );

  /// Destructor.
  virtual ~SourceLine();

  bool isValid() const;

  int lineNumber() const;

  std::string fileName() const;

  bool operator ==( const SourceLine &other ) const;
  bool operator !=( const SourceLine &other ) const;

private:
  std::string m_fileName;
  int m_lineNumber;
};


} // namespace CppUnit



#endif  // CPPUNIT_SOURCELINE_H
