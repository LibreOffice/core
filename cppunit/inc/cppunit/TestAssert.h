#ifndef CPPUNIT_TESTASSERT_H
#define CPPUNIT_TESTASSERT_H

#include <cppunit/Portability.h>
#include <cppunit/Exception.h>
#include <cppunit/Asserter.h>

#include <cppunit/checkboom.hxx>

namespace CppUnit {



  /*! \brief Traits used by CPPUNIT_ASSERT_EQUAL().
   *
   * Here is an example of specialization of that traits:
   *
   * \code
   * template<>
   * struct assertion_traits<std::string>   // specialization for the std::string type
   * {
   *   static bool equal( const std::string& x, const std::string& y )
   *   {
   *     return x == y;
   *   }
   *
   *   static std::string toString( const std::string& x )
   *   {
   *     std::string text = '"' + x + '"';    // adds quote around the string to see whitespace
   *     OStringStream ost;
   *     ost << text;
   *     return ost.str();
   *   }
   * };
   * \endcode
   */


template <class T>
struct assertion_traits
{
    static bool equal( const T& x, const T& y )
    {
        return x == y;
    }

    static std::string toString( const T& x )
    {
        OStringStream ost;
        ost << x;
        return ost.str();
    }
};

namespace TestAssert
{
#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
      void CPPUNIT_API assertImplementation( bool         condition,
                                             std::string  conditionExpression = "",
                                             long lineNumber,
                                             std::string  fileName );

      void CPPUNIT_API assertNotEqualImplementation( std::string expected,
                                                     std::string actual,
                                                     long lineNumber,
                                                     std::string fileName );


      template <class T>
      void assertEquals( const T& expected,
                         const T& actual,
                         long lineNumber,
                         std::string fileName )
      {
        if ( !assertion_traits<T>::equal(expected,actual) ) // lazy toString conversion...
        {
          assertNotEqualImplementation( assertion_traits<T>::toString(expected),
                                        assertion_traits<T>::toString(actual),
                                        lineNumber,
                                        fileName );
        }
      }

      void CPPUNIT_API assertEquals( double expected,
                                     double actual,
                                     double delta,
                                     long lineNumber,
                                     std::string fileName );

#else   //                  using SourceLine

      template <class T>
      void assertEquals( const T& expected,
                         const T& actual,
                         SourceLine sourceLine,
                         const std::string &message ="" )
      {
        if ( !assertion_traits<T>::equal(expected,actual) ) // lazy toString conversion...
        {
          Asserter::failNotEqual( assertion_traits<T>::toString(expected),
                                  assertion_traits<T>::toString(actual),
                                  sourceLine,
                                  message );
        }
      }

  void CPPUNIT_API assertDoubleEquals( double expected,
                                       double actual,
                                       double delta,
                                       SourceLine sourceLine );

#endif
}

/* A set of macros which allow us to get the line number
 * and file name at the point of an error.
 * Just goes to show that preprocessors do have some
 * redeeming qualities.
 */
#if CPPUNIT_HAVE_CPP_SOURCE_ANNOTATION
/** Assertions that a condition is \c true.
 * \ingroup Assertions
 */
    // LLA: due to the fact, that side effects are not wished, we create a local scope,
    //      assign the condition to a local variable and check this variable again and again.

#define CPPUNIT_ASSERT(condition)                       \
   {   bool __bLocalCondition = condition;                     \
       CheckBoom((__bLocalCondition), (#condition));           \
       (::CppUnit::Asserter::failIf( !(__bLocalCondition),     \
                                    (#condition),       \
                                    CPPUNIT_SOURCELINE() ) ); }
#else
#define CPPUNIT_ASSERT(condition)                       \
   { bool __bLocalCondition = condition;                       \
   CheckBoom((__bLocalCondition), "");                         \
    (::CppUnit::Asserter::failIf( !(__bLocalCondition),        \
                                 "",                    \
                                 CPPUNIT_SOURCELINE() ) ); }
#endif

/** Assertion with a user specified message.
 * \ingroup Assertions
 * \param message Message reported in diagnostic if \a condition evaluates
 *                to \c false.
 * \param condition If this condition evaluates to \c false then the
 *                  test failed.
 */
#define CPPUNIT_ASSERT_MESSAGE(message,condition)       \
   { bool __bLocalCondition = (condition);                     \
   CheckBoom((__bLocalCondition), (message));                  \
    (::CppUnit::Asserter::failIf( !(__bLocalCondition),        \
                                 (message),             \
                                 CPPUNIT_SOURCELINE() ) ); }

#define CPPUNIT_ASSERT_STUB() \
   CheckBoom((1 == 0), ("This is only a stub."));          \
   (::CppUnit::Asserter::failStub("This is only a stub.",  \
                                  CPPUNIT_SOURCELINE() ) )


/** Fails with the specified message.
 * \ingroup Assertions
 * \param message Message reported in diagnostic.
 */
#define CPPUNIT_FAIL( message )                            \
    ( ::CppUnit::Asserter::fail( message,                    \
                                 CPPUNIT_SOURCELINE() ) )

#ifdef CPPUNIT_ENABLE_SOURCELINE_DEPRECATED
/// Generalized macro for primitive value comparisons
#define CPPUNIT_ASSERT_EQUAL(expected,actual)                    \
    ( ::CppUnit::TestAssert::assertEquals( (expected),             \
                                           (actual),               \
                                           __LINE__, __FILE__ ) )
#else
/** Asserts that two values are equals.
 * \ingroup Assertions
 *
 * Equality and string representation can be defined with
 * an appropriate CppUnit::assertion_traits class.
 *
 * A diagnostic is printed if actual and expected values disagree.
 *
 * Requirement for \a expected and \a actual parameters:
 * - They are exactly of the same type
 * - They are serializable into a std::strstream using operator <<.
 * - They can be compared using operator ==.
 *
 * The last two requirements (serialization and comparison) can be
 * removed by specializing the CppUnit::assertion_traits.
 */
#define CPPUNIT_ASSERT_EQUAL(expected,actual)                     \
    ( ::CppUnit::TestAssert::assertEquals( (expected),              \
                                           (actual),                \
                                           CPPUNIT_SOURCELINE() ) )

/** Asserts that two values are equals, provides additional messafe on failure.
 * \ingroup Assertions
 *
 * Equality and string representation can be defined with
 * an appropriate assertion_traits class.
 *
 * A diagnostic is printed if actual and expected values disagree.
 * The message is printed in addition to the expected and actual value
 * to provide additional information.
 *
 * Requirement for \a expected and \a actual parameters:
 * - They are exactly of the same type
 * - They are serializable into a std::strstream using operator <<.
 * - They can be compared using operator ==.
 *
 * The last two requirements (serialization and comparison) can be
 * removed by specializing the CppUnit::assertion_traits.
 */
#define CPPUNIT_ASSERT_EQUAL_MESSAGE(message,expected,actual)     \
    ( ::CppUnit::TestAssert::assertEquals( (expected),              \
                                           (actual),                \
                                           CPPUNIT_SOURCELINE(),    \
                                           (message) ) )
#endif

/*! \brief Macro for primitive value comparisons
 * \ingroup Assertions
 */
#define CPPUNIT_ASSERT_DOUBLES_EQUAL(expected,actual,delta)       \
    ( ::CppUnit::TestAssert::assertDoubleEquals( (expected),        \
                                                 (actual),          \
                                                 (delta),           \
                                                 CPPUNIT_SOURCELINE() ) )

// Backwards compatibility

#if CPPUNIT_ENABLE_NAKED_ASSERT

#undef assert
#define assert(c)                 CPPUNIT_ASSERT(c)
#define assertEqual(e,a)          CPPUNIT_ASSERT_EQUAL(e,a)
#define assertDoublesEqual(e,a,d) CPPUNIT_ASSERT_DOUBLES_EQUAL(e,a,d)
#define assertLongsEqual(e,a)     CPPUNIT_ASSERT_EQUAL(e,a)

#endif

} // namespace CppUnit

#endif  // CPPUNIT_TESTASSERT_H
